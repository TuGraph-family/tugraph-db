#pragma once
#include "util.h"
#include "status.h"
namespace eraft {

// ErrStepLocalMsg is returned when try to step a local raft message
const Error ErrStepLocalMsg("raft: cannot step raft local message");

// ErrStepPeerNotFound is returned when try to step a response message
// but there is no peer found in raft.trk for that node.
const Error ErrStepPeerNotFound("raft: cannot step as peer not found");

enum SnapshotStatus {
    SnapshotFinish = 1,
    SnapshotFailure = 2
};

// MustSync returns true if the hard state and count of Raft entries indicate
// that a synchronous write to persistent storage is required.
inline bool MustSync(const raftpb::HardState& st, const raftpb::HardState& prevst, size_t entsnum) {
	// Persistent state on all servers:
	// (Updated on stable storage before responding to RPCs)
	// currentTerm
	// votedFor
	// log entries[]
	return entsnum != 0 || st.vote() != prevst.vote() || st.term() != prevst.term();
}

// ProgressType indicates the type of replica a Progress corresponds to.
enum ProgressType {
    // ProgressTypePeer accompanies a Progress for a regular peer replica.
    ProgressTypePeer = 0,
    // ProgressTypeLearner accompanies a Progress for a learner replica.
    ProgressTypeLearner
};

struct Peer {
	uint64_t id_ = 0;
	std::string context_;
};

// RawNode is a thread-unsafe Node.
// The methods of this struct correspond to the methods of Node and are described
// more fully there.
struct RawNode {
    std::shared_ptr<detail::raft> raft_;
    bool asyncStorageWrites_ = false;

    // Mutable fields.
    std::shared_ptr<SoftState> prevSoftSt_;
    raftpb::HardState prevHardSt_;
    std::vector<raftpb::Message> stepsOnAdvance_;

    // Tick advances the internal logical clock by a single tick.
    void Tick() {
        raft_->tick_();
    }

    // TickQuiesced advances the internal logical clock by a single tick without
    // performing any other state machine processing. It allows the caller to avoid
    // periodic heartbeats and elections when all of the peers in a Raft group are
    // known to be at the same state. Expected usage is to periodically invoke Tick
    // or TickQuiesced depending on whether the group is "active" or "quiesced".
    //
    // WARNING: Be very careful about using this method as it subverts the Raft
    // state machine. You should probably be using Tick instead.
    //
    // DEPRECATED: This method will be removed in a future release.
    void TickQuiesced() {
        raft_->electionElapsed_++;
    }

    // Campaign causes this RawNode to transition to candidate state.
    Error Campaign() {
        raftpb::Message m;
        m.set_type(raftpb::MsgHup);
        return raft_->Step(std::move(m));
    }

    // Propose proposes data be appended to the raft log.
    Error Propose(std::string data) {
        raftpb::Message m;
        m.set_type(raftpb::MsgProp);
        m.set_from(raft_->id_);
        m.add_entries()->set_data(std::move(data));
        return raft_->Step(std::move(m));
    }
    // ProposeConfChange proposes a config change. See (Node).ProposeConfChange for
    // details.
    Error ProposeConfChange(raftpb::ConfChangeI* cc) {
        raftpb::Message m;
        Error err;
        std::tie(m, err) = raftpb::detail::confChangeToMsg(cc);
        if (err != nullptr) {
            return err;
        }
        return raft_->Step(m);
    }

    // ApplyConfChange applies a config change to the local node. The app must call
    // this when it applies a configuration change, except when it decides to reject
    // the configuration change, in which case no call must take place.
    std::shared_ptr<raftpb::ConfState> ApplyConfChange(const raftpb::ConfChangeI& cc) {
        return std::make_shared<raftpb::ConfState>(raft_->applyConfChange(cc.AsV2()));
    }

    // Step advances the state machine using the given message.
    Error Step(raftpb::Message m) {
        // Ignore unexpected local messages receiving over network.
        if (IsLocalMsg(m.type()) && !IsLocalMsgTarget(m.from())) {
            return ErrStepLocalMsg;
        }
        if (IsResponseMsg(m.type()) && !IsLocalMsgTarget(m.from()) && raft_->trk_.progress_.data().count(m.from()) == 0) {
            return ErrStepPeerNotFound;
        }
        return raft_->Step(std::move(m));
    }

    // applyUnstableEntries returns whether entries are allowed to be applied once
    // they are known to be committed but before they have been written locally to
    // stable storage.
    bool applyUnstableEntries() const {
        return !asyncStorageWrites_;
    }

    bool needStorageAppendMsg(const std::shared_ptr<detail::raft>& raft, const Ready& rd) {
        // Return true if log entries, hard state, or a snapshot need to be written
        // to stable storage. Also return true if any messages are contingent on all
        // prior MsgStorageAppend being processed.
        return !rd.entries_.empty() ||
               !IsEmptyHardState(rd.hardState_) ||
               !IsEmptySnap(rd.snapshot_) ||
               !raft->msgsAfterAppend_.empty();
    }

    bool needStorageAppendRespMsg(const std::shared_ptr<detail::raft>& raft, const Ready& rd) {
        // Return true if raft needs to hear about stabilized entries or an applied
        // snapshot. See the comment in newStorageAppendRespMsg, which explains why
        // we check hasNextOrInProgressUnstableEnts instead of len(rd.Entries) > 0.
        return raft->raftLog_->hasNextOrInProgressUnstableEnts() ||
               !IsEmptySnap(rd.snapshot_);
    }

    // newStorageAppendRespMsg creates the message that should be returned to node
    // after the unstable log entries, hard state, and snapshot in the current Ready
    // (along with those in all prior Ready structs) have been saved to stable
    // storage.
    raftpb::Message newStorageAppendRespMsg(const std::shared_ptr<detail::raft>& raft, const Ready& rd) {
        raftpb::Message m;
        m.set_type(raftpb::MsgStorageAppendResp);
        m.set_to(raft->id_);
        m.set_from(LocalAppendThread);
        // Dropped after term change, see below.
        m.set_term(raft->term_);
        if (raft->raftLog_->hasNextOrInProgressUnstableEnts()) {
            // If the raft log has unstable entries, attach the last index and term of the
            // append to the response message. This (index, term) tuple will be handed back
            // and consulted when the stability of those log entries is signaled to the
            // unstable. If the (index, term) match the unstable log by the time the
            // response is received (unstable.stableTo), the unstable log can be truncated.
            //
            // However, with just this logic, there would be an ABA problem[^1] that could
            // lead to the unstable log and the stable log getting out of sync temporarily
            // and leading to an inconsistent view. Consider the following example with 5
            // nodes, A B C D E:
            //
            //  1. A is the leader.
            //  2. A proposes some log entries but only B receives these entries.
            //  3. B gets the Ready and the entries are appended asynchronously.
            //  4. A crashes and C becomes leader after getting a vote from D and E.
            //  5. C proposes some log entries and B receives these entries, overwriting the
            //     previous unstable log entries that are in the process of being appended.
            //     The entries have a larger term than the previous entries but the same
            //     indexes. It begins appending these new entries asynchronously.
            //  6. C crashes and A restarts and becomes leader again after getting the vote
            //     from D and E.
            //  7. B receives the entries from A which are the same as the ones from step 2,
            //     overwriting the previous unstable log entries that are in the process of
            //     being appended from step 5. The entries have the original terms and
            //     indexes from step 2. Recall that log entries retain their original term
            //     numbers when a leader replicates entries from previous terms. It begins
            //     appending these new entries asynchronously.
            //  8. The asynchronous log appends from the first Ready complete and stableTo
            //     is called.
            //  9. However, the log entries from the second Ready are still in the
            //     asynchronous append pipeline and will overwrite (in stable storage) the
            //     entries from the first Ready at some future point. We can't truncate the
            //     unstable log yet or a future read from Storage might see the entries from
            //     step 5 before they have been replaced by the entries from step 7.
            //     Instead, we must wait until we are sure that the entries are stable and
            //     that no in-progress appends might overwrite them before removing entries
            //     from the unstable log.
            //
            // To prevent these kinds of problems, we also attach the current term to the
            // MsgStorageAppendResp (above). If the term has changed by the time the
            // MsgStorageAppendResp if returned, the response is ignored and the unstable
            // log is not truncated. The unstable log is only truncated when the term has
            // remained unchanged from the time that the MsgStorageAppend was sent to the
            // time that the MsgStorageAppendResp is received, indicating that no-one else
            // is in the process of truncating the stable log.
            //
            // However, this replaces a correctness problem with a liveness problem. If we
            // only attempted to truncate the unstable log when appending new entries but
            // also occasionally dropped these responses, then quiescence of new log entries
            // could lead to the unstable log never being truncated.
            //
            // To combat this, we attempt to truncate the log on all MsgStorageAppendResp
            // messages where the unstable log is not empty, not just those associated with
            // entry appends. This includes MsgStorageAppendResp messages associated with an
            // updated HardState, which occur after a term change.
            //
            // In other words, we set Index and LogTerm in a block that looks like:
            //
            //  if r.raftLog.hasNextOrInProgressUnstableEnts() { ... }
            //
            // not like:
            //
            //  if len(rd.Entries) > 0 { ... }
            //
            // To do so, we attach r.raftLog.lastIndex() and r.raftLog.lastTerm(), not the
            // (index, term) of the last entry in rd.Entries. If rd.Entries is not empty,
            // these will be the same. However, if rd.Entries is empty, we still want to
            // attest that this (index, term) is correct at the current term, in case the
            // MsgStorageAppend that contained the last entry in the unstable slice carried
            // an earlier term and was dropped.
            //
            // A MsgStorageAppend with a new term is emitted on each term change. This is
            // the same condition that causes MsgStorageAppendResp messages with earlier
            // terms to be ignored. As a result, we are guaranteed that, assuming a bounded
            // number of term changes, there will eventually be a MsgStorageAppendResp
            // message that is not ignored. This means that entries in the unstable log
            // which have been appended to stable storage will eventually be truncated and
            // dropped from memory.
            //
            // [^1]: https://en.wikipedia.org/wiki/ABA_problem
            m.set_index(raft->raftLog_->lastIndex());
            m.set_logterm(raft->raftLog_->lastTerm());
        };
        if (!IsEmptySnap(rd.snapshot_)) {
            *m.mutable_snapshot() = rd.snapshot_;
        }
        return m;
    }

    // newStorageAppendMsg creates the message that should be sent to the local
    // append thread to instruct it to append log entries, write an updated hard
    // state, and apply a snapshot. The message also carries a set of responses
    // that should be delivered after the rest of the message is processed. Used
    // with AsyncStorageWrites.
    raftpb::Message newStorageAppendMsg(std::shared_ptr<detail::raft> r, const Ready& rd) {
        raftpb::Message m;
        m.set_type(raftpb::MsgStorageAppend);
        m.set_to(LocalAppendThread);
        m.set_from(r->id_);
        *m.mutable_entries() = {rd.entries_.begin(), rd.entries_.end()};

        if (!IsEmptyHardState(rd.hardState_)) {
            // If the Ready includes a HardState update, assign each of its fields
            // to the corresponding fields in the Message. This allows clients to
            // reconstruct the HardState and save it to stable storage.
            //
            // If the Ready does not include a HardState update, make sure to not
            // assign a value to any of the fields so that a HardState reconstructed
            // from them will be empty (return true from raft.IsEmptyHardState).
            m.set_term(rd.hardState_.term());
            m.set_vote(rd.hardState_.vote());
            m.set_commit(rd.hardState_.commit());
        };
        if (!IsEmptySnap(rd.snapshot_)) {
            *m.mutable_snapshot() = rd.snapshot_;
        }
        // Attach all messages in msgsAfterAppend as responses to be delivered after
        // the message is processed, along with a self-directed MsgStorageAppendResp
        // to acknowledge the entry stability.
        //
        // NB: it is important for performance that MsgStorageAppendResp message be
        // handled after self-directed MsgAppResp messages on the leader (which will
        // be contained in msgsAfterAppend). This ordering allows the MsgAppResp
        // handling to use a fast-path in r.raftLog.term() before the newly appended
        // entries are removed from the unstable log.
        *m.mutable_responses() = {raft_->msgsAfterAppend_.begin(), raft_->msgsAfterAppend_.end()};
        if (needStorageAppendRespMsg(r, rd)) {
            *m.mutable_responses()->Add() = newStorageAppendRespMsg(r, rd);
        }
        return m;
    }

    bool needStorageApplyMsg(const Ready& rd) const {
        return !rd.committedEntries_.empty();
    }

    bool needStorageApplyRespMsg(const Ready& rd) const {
        return needStorageApplyMsg(rd);
    };

    // newStorageApplyRespMsg creates the message that should be returned to node
    // after the committed entries in the current Ready (along with those in all
    // prior Ready structs) have been applied to the local state machine.
    raftpb::Message newStorageApplyRespMsg(const std::shared_ptr<detail::raft>& raft, std::vector<raftpb::Entry> ents) {
        raftpb::Message m;
        m.set_type(raftpb::MsgStorageApplyResp);
        m.set_to(raft->id_);
        m.set_from(LocalApplyThread);
        m.set_term(0); // committed entries don't apply under a specific term
        *m.mutable_entries() = {std::make_move_iterator(ents.begin()), std::make_move_iterator(ents.end())};
        return m;
    }

    // newStorageApplyMsg creates the message that should be sent to the local
    // apply thread to instruct it to apply committed log entries. The message
    // also carries a response that should be delivered after the rest of the
    // message is processed. Used with AsyncStorageWrites.
    raftpb::Message newStorageApplyMsg(const std::shared_ptr<detail::raft>& raft, const Ready& rd) {
        const auto& ents = rd.committedEntries_;
        raftpb::Message m;
        m.set_type(raftpb::MsgStorageApply);
        m.set_to(LocalApplyThread);
        m.set_from(raft_->id_);
        m.set_term(0); // committed entries don't apply under a specific term
        *m.mutable_entries() = {ents.begin(), ents.end()};
        *m.mutable_responses()->Add() = newStorageApplyRespMsg(raft, ents);
        return m;
    }

    // readyWithoutAccept returns a Ready. This is a read-only operation, i.e. there
    // is no obligation that the Ready must be handled.
    Ready readyWithoutAccept() {
        Ready rd;
        rd.entries_ = raft_->raftLog_->nextUnstableEnts();
        rd.committedEntries_ = raft_->raftLog_->nextCommittedEnts(applyUnstableEntries());
        rd.messages_ = raft_->msgs_;
        auto softSt = raft_->softState();
        if (softSt != *prevSoftSt_) {
            rd.softState_ = std::make_shared<SoftState>(softSt);
        }
        auto hardSt = raft_->hardState();
        if (!isHardStateEqual(hardSt, prevHardSt_)) {
            rd.hardState_ = hardSt;
        }

        if (raft_->raftLog_->hasNextUnstableSnapshot()) {
            rd.snapshot_ = *raft_->raftLog_->nextUnstableSnapshot();
        }
        if (!raft_->readStates_.empty()) {
            rd.readStates_ = raft_->readStates_;
        }
        rd.mustSync_ = MustSync(raft_->hardState(), prevHardSt_, rd.entries_.size());

        if (asyncStorageWrites_) {
            // If async storage writes are enabled, enqueue messages to
            // local storage threads, where applicable.
            if (needStorageAppendMsg(raft_, rd)) {
                auto m = newStorageAppendMsg(raft_, rd);
                rd.messages_.emplace_back(std::move(m));
            }
            if (needStorageApplyMsg(rd)) {
                auto m = newStorageApplyMsg(raft_, rd);
                rd.messages_.emplace_back(std::move(m));
            }
        } else {
            // If async storage writes are disabled, immediately enqueue
            // msgsAfterAppend to be sent out. The Ready struct contract
            // mandates that Messages cannot be sent until after Entries
            // are written to stable storage.
            for (auto& m : raft_->msgsAfterAppend_) {
                if (m.to() != raft_->id_) {
                    rd.messages_.emplace_back(m);
                }
            }
        }

        return rd;
    }

    // acceptReady is called when the consumer of the RawNode has decided to go
    // ahead and handle a Ready. Nothing must alter the state of the RawNode between
    // this call and the prior call to Ready().
    void acceptReady(const Ready& rd) {
        if (rd.softState_ != nullptr) {
            prevSoftSt_ = rd.softState_;
        }
        if (!IsEmptyHardState(rd.hardState_)) {
            prevHardSt_ = rd.hardState_;
        }
        if (!rd.readStates_.empty()) {
            raft_->readStates_.clear();
        }
        if (!asyncStorageWrites_) {
            if (!stepsOnAdvance_.empty()) {
                ERAFT_FATAL("two accepted Ready structs without call to Advance");
            }
            for (auto& m : raft_->msgsAfterAppend_) {
                if (m.to() == raft_->id_) {
                    stepsOnAdvance_.emplace_back(m);
                }
            }
            if (needStorageAppendRespMsg(raft_, rd)) {
                auto m = newStorageAppendRespMsg(raft_, rd);
                stepsOnAdvance_.emplace_back(m);
            }
            if (needStorageApplyRespMsg(rd)) {
                auto m = newStorageApplyRespMsg(raft_, rd.committedEntries_);
                stepsOnAdvance_.emplace_back(m);
            }
        }
        raft_->msgs_.clear();
        raft_->msgsAfterAppend_.clear();
        raft_->raftLog_->acceptUnstable();
        if (!rd.committedEntries_.empty()) {
            const auto& ents = rd.committedEntries_;
            auto index = ents[ents.size()-1].index();
            raft_->raftLog_->acceptApplying(index, entsSize(ents), applyUnstableEntries());
        }
    }

    // GetReady returns the outstanding work that the application needs to handle. This
    // includes appending and applying entries or a snapshot, updating the HardState,
    // and sending messages. The returned Ready() *must* be handled and subsequently
    // passed back via Advance().
    Ready GetReady() {
        auto rd = readyWithoutAccept();
        acceptReady(rd);
        return rd;
    }

    // HasReady called when RawNode user need to check if any Ready pending.
    // Checking logic in this method should be consistent with Ready.containsUpdates().
    bool HasReady() {
        // TODO(nvanbenschoten): order these cases in terms of cost and frequency.
        auto softSt = raft_->softState();
        if (softSt != *prevSoftSt_) {
            return true;
        }
        auto hardSt = raft_->hardState();
        if (!IsEmptyHardState(hardSt) && !isHardStateEqual(hardSt, prevHardSt_)) {
            return true;
        }
        if (raft_->raftLog_->hasNextUnstableSnapshot()) {
            return true;
        }
        if (!raft_->msgs_.empty() || !raft_->msgsAfterAppend_.empty()) {
            return true;
        }
        if (raft_->raftLog_->hasNextUnstableEnts() || raft_->raftLog_->hasNextCommittedEnts(applyUnstableEntries())) {
            return true;
        }
        if (!raft_->readStates_.empty()) {
            return true;
        }
        return false;
    }

    // Advance notifies the RawNode that the application has applied and saved progress in the
    // last Ready results.
    //
    // NOTE: Advance must not be called when using AsyncStorageWrites. Response messages from
    // the local append and apply threads take its place.
    void Advance(const Ready& /*rd*/) {
        // The actions performed by this function are encoded into stepsOnAdvance in
        // acceptReady. In earlier versions of this library, they were computed from
        // the provided Ready struct. Retain the unused parameter for compatibility.
        if (asyncStorageWrites_) {
            ERAFT_FATAL("Advance must not be called when using AsyncStorageWrites")
        };
        for (size_t i = 0; i < stepsOnAdvance_.size(); i++) {
            raft_->Step(std::move(stepsOnAdvance_[i]));
        }
        stepsOnAdvance_.clear();
    }

    // GetStatus returns the current status of the given group. This allocates, see
    // BasicStatus and WithProgress for allocation-friendlier choices.
    Status GetStatus() {
        auto status = detail::getStatus(*raft_);
        return status;
    }

    // GetBasicStatus returns a BasicStatus. Notably this does not contain the
    // Progress map; see WithProgress for an allocation-free way to inspect it.
    BasicStatus GetBasicStatus() {
        return detail::getBasicStatus(*raft_);
    }

    // WithProgress is a helper to introspect the Progress for this node and its
    // peers.
    void WithProgress(std::function<void(uint64_t, ProgressType, tracker::Progress)>& visitor) {
        raft_->trk_.Visit([visitor](uint64_t id, std::shared_ptr<tracker::Progress>& pr) {
            auto typ = ProgressTypePeer;
            if (pr->isLearner_) {
                typ = ProgressTypeLearner;
            }
            auto p = *pr;
            p.inflights_ = nullptr;
            visitor(id, typ, p);
        });
    }

    // ReportUnreachable reports the given node is not reachable for the last send.
    void ReportUnreachable(uint64_t id) {
        raftpb::Message m;
        m.set_type(raftpb::MsgUnreachable);
        m.set_from(id);
        raft_->Step(std::move(m));
    }

    // ReportSnapshot reports the status of the sent snapshot.
    void ReportSnapshot(uint64_t id, SnapshotStatus status) {
        auto rej = status == SnapshotFailure;
        raftpb::Message m;
        m.set_type(raftpb::MsgSnapStatus);
        m.set_from(id);
        m.set_reject(rej);
        raft_->Step(m);
    }

    // TransferLeader tries to transfer leadership to the given transferee.
    void TransferLeader(uint64_t transferee) {
        raftpb::Message m;
        m.set_type(raftpb::MsgTransferLeader);
        m.set_from(transferee);
        raft_->Step(m);
    }

    // ReadIndex requests a read state. The read state will be set in ready.
    // Read State has a read index. Once the application advances further than the read
    // index, any linearizable read requests issued before the read request can be
    // processed safely. The read state will have the same rctx attached.
    void ReadIndex(std::string rctx) {
        raftpb::Message m;
        m.set_type(raftpb::MsgReadIndex);
        m.add_entries()->set_data(std::move(rctx));
        raft_->Step(m);
    }
    // Bootstrap initializes the RawNode for first use by appending configuration
    // changes for the supplied peers. This method returns an error if the Storage
    // is nonempty.
    //
    // It is recommended that instead of calling this method, applications bootstrap
    // their state manually by setting up a Storage that has a first index > 1 and
    // which stores the desired ConfState as its InitialState.
    Error Bootstrap(const std::vector<Peer>& peers) {
        if (peers.empty()) {
            return Error("must provide at least one peer to Bootstrap");
        }
        uint64_t lastIndex = 0;
        Error err;
        std::tie(lastIndex, err) = raft_->raftLog_->storage_->LastIndex();
        if (err != nullptr) {
            return err;
        }

        if (lastIndex != 0) {
            return Error("can't bootstrap a nonempty Storage");
        }

        // We've faked out initial entries above, but nothing has been
        // persisted. Start with an empty HardState (thus the first Ready will
        // emit a HardState update for the app to persist).
        prevHardSt_= {};

        // TODO(tbg): remove StartNode and give the application the right tools to
        // bootstrap the initial membership in a cleaner way.
        raft_->becomeFollower(1, None);
        std::vector<raftpb::Entry> ents(peers.size());
        for (size_t i = 0; i < peers.size(); i++) {
            raftpb::ConfChange cc;
            cc.set_type(raftpb::ConfChangeAddNode);
            cc.set_node_id(peers[i].id_);
            cc.set_context(peers[i].context_);

            raftpb::Entry ent;
            ent.set_type(raftpb::EntryConfChange);
            ent.set_term(1);
            ent.set_index(i + 1);
            ent.set_data(cc.SerializeAsString());
            ents[i] = std::move(ent);
        }
        raft_->raftLog_->append(ents);

        // Now apply them, mainly so that the application can call Campaign
        // immediately after StartNode in tests. Note that these nodes will
        // be added to raft twice: here and when the application's Ready
        // loop calls ApplyConfChange. The calls to addNode must come after
        // all calls to raftLog.append so progress.next is set after these
        // bootstrapping entries (it is an error if we try to append these
        // entries since they have already been committed).
        // We do not set raftLog.applied so the application will be able
        // to observe all conf changes via Ready.CommittedEntries.
        //
        // TODO(bdarnell): These entries are still unstable; do we need to preserve
        // the invariant that committed < unstable?
        raft_->raftLog_->committed_ = ents.size();
        for (auto& peer : peers) {
            raftpb::ConfChange cc;
            cc.set_node_id(peer.id_);
            cc.set_type(raftpb::ConfChangeAddNode);
            raft_->applyConfChange(raftpb::ConfChangeWrap(cc).AsV2());
        }
        return {};
    }

    // ForgetLeader forgets a follower's current leader, changing it to None.
    // See (Node).ForgetLeader for details.
    Error ForgetLeader() {
        raftpb::Message msg;
        msg.set_type(raftpb::MsgForgetLeader);
        return raft_->Step(std::move(msg));
    }
};

// NewRawNode instantiates a RawNode from the given configuration.
//
// See Bootstrap() for bootstrapping an initial state; this replaces the former
// 'peers' argument to this method (with identical behavior). However, It is
// recommended that instead of calling Bootstrap, applications bootstrap their
// state manually by setting up a Storage that has a first index > 1 and which
// stores the desired ConfState as its InitialState.
inline std::pair<std::shared_ptr<RawNode>, Error> NewRawNode(Config config) {
	auto r = detail::newRaft(config);
    auto rn = std::make_shared<RawNode>();
    rn->asyncStorageWrites_ = config.AsyncStorageWrites_;
    rn->raft_ = r;
	rn->prevSoftSt_ = std::make_shared<SoftState>(r->softState());
	rn->prevHardSt_ = r->hardState();
	return {rn, {}};
}

}