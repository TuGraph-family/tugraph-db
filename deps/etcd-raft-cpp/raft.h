#pragma once
#include "util.h"
#include "read_only.h"
#include "tracker/tracker.h"
#include "quorum/quorum.h"
#include "log.h"
#include "confchange/restore.h"
#include "raftpb/confstate.h"
#include "raftpb/confchange.h"
namespace eraft {

// Possible values for StateType.
enum StateType {
    StateFollower = 0,
    StateCandidate,
    StateLeader,
    StatePreCandidate,
    numStates
};

inline std::string ToString(StateType st) {
    switch (st) {
        case StateType::StateFollower:
            return "StateFollower";
        case StateType::StateCandidate:
            return "StateCandidate";
        case StateType::StateLeader:
            return "StateLeader";
        case StateType::StatePreCandidate:
            return "StatePreCandidate";
        default:
            ERAFT_FATAL("unknown state type");
            return "UnknownStateType";
    };
}

// CampaignType represents the type of campaigning
// the reason we use the type of string instead of uint64
// is because it's simpler to compare and fill in raft entries
typedef std::string CampaignType;

// Possible values for CampaignType
// campaignPreElection represents the first phase of a normal election when
// Config.PreVote is true.
const CampaignType campaignPreElection = "CampaignPreElection";
// campaignElection represents a normal (time-based) election (the second phase
// of the election when Config.PreVote is true).
const CampaignType campaignElection = "CampaignElection";
// campaignTransfer represents the type of leader transfer
const CampaignType campaignTransfer = "CampaignTransfer";

// ErrProposalDropped is returned when the proposal is ignored by some cases,
// so that the proposer can be notified and fail fast.
const Error ErrProposalDropped("raft proposal dropped");

// SoftState provides state that is useful for logging and debugging.
// The state is volatile and does not need to be persisted to the WAL.
struct SoftState {
    uint64_t lead_ = 0; // must use atomic operations to access; keep 64-bit aligned.
    StateType raftState_ = StateFollower;
    SoftState() = default;
    SoftState(uint64_t lead, StateType raftState) : lead_(lead), raftState_(raftState) {}
    bool operator==(const SoftState& state) const {
        return lead_ == state.lead_ && raftState_ == state.raftState_;
    }
    bool operator!=(const SoftState& state) const {
        return lead_ != state.lead_ || raftState_ != state.raftState_;
    }
};

// Ready encapsulates the entries and messages that are ready to read,
// be saved to stable storage, committed or sent to other peers.
// All fields in Ready are read-only.
struct Ready {
	// The current volatile state of a Node.
	// SoftState will be nil if there is no update.
	// It is not required to consume or store SoftState.
	std::shared_ptr<SoftState> softState_;

	// The current state of a Node to be saved to stable storage BEFORE
	// Messages are sent.
	// HardState will be equal to empty state if there is no update.
	raftpb::HardState hardState_;

	// ReadStates can be used for node to serve linearizable read requests locally
	// when its applied index is greater than the index in ReadState.
	// Note that the readState will be returned when raft receives msgReadIndex.
	// The returned is only valid for the request that requested to read.
    std::vector<ReadState> readStates_;

	// Entries specifies entries to be saved to stable storage BEFORE
	// Messages are sent.
    std::vector<raftpb::Entry> entries_;

	// Snapshot specifies the snapshot to be saved to stable storage.
    raftpb::Snapshot snapshot_;

	// CommittedEntries specifies entries to be committed to a
	// store/state-machine. These have previously been committed to stable
	// store.
    std::vector<raftpb::Entry> committedEntries_;

	// Messages specifies outbound messages to be sent AFTER Entries are
	// committed to stable storage.
	// If it contains a MsgSnap message, the application MUST report back to raft
	// when the snapshot has been received or has failed by calling ReportSnapshot.
    std::vector<raftpb::Message> messages_;

	// MustSync indicates whether the HardState and Entries must be synchronously
	// written to disk or if an asynchronous write is permissible.
	bool mustSync_ = false;

    bool operator==(const Ready& r) const {
        if ((!softState_ && r.softState_) || (softState_ && !r.softState_)) {
            return false;
        }
        if (softState_ && r.softState_ && *softState_ != *r.softState_) {
            return false;
        }
        if (!google::protobuf::util::MessageDifferencer::Equals(hardState_, r.hardState_)) {
            return false;
        }
        if (readStates_.size() != r.readStates_.size()) {
            return false;
        }
        for (size_t i = 0; i < readStates_.size(); i++) {
            if (readStates_.at(i) != r.readStates_.at(i)) {
                return false;
            }
        }
        if (!VectorEquals(entries_, r.entries_)) {
            return false;
        }
        if (!google::protobuf::util::MessageDifferencer::Equals(snapshot_, r.snapshot_)) {
            return false;
        }
        if (!VectorEquals(committedEntries_, r.committedEntries_)) {
            return false;
        }
        if (!VectorEquals(messages_, r.messages_)) {
            return false;
        }
        if (mustSync_ != r.mustSync_) {
            return false;
        }
        return true;
    }

    bool containsUpdates() const {
        return softState_ != nullptr || !IsEmptyHardState(hardState_) ||
               !IsEmptySnap(snapshot_) || !entries_.empty() ||
               !committedEntries_.empty() || !messages_.empty() || !readStates_.empty();
    }

    // appliedCursor extracts from the Ready the highest index the client has
    // applied (once the Ready is confirmed via Advance). If no information is
    // contained in the Ready, returns zero.
    uint64_t appliedCursor() const {
        auto n = committedEntries_.size();
        if (n > 0) {
            return committedEntries_[n-1].index();
        }
        auto index = snapshot_.metadata().index();
        if (index > 0) {
            return index;
        }
        return 0;
    }
};

// Config contains the parameters to start a raft.
struct Config {
	// ID is the identity of the local raft. ID cannot be 0.
	uint64_t id_ = 0;

	// ElectionTick is the number of Node.Tick invocations that must pass between
	// elections. That is, if a follower does not receive any message from the
	// leader of current term before ElectionTick has elapsed, it will become
	// candidate and start an election. ElectionTick must be greater than
	// HeartbeatTick. We suggest ElectionTick = 10 * HeartbeatTick to avoid
	// unnecessary leader switching.
	int64_t electionTick_ = 0;
	// HeartbeatTick is the number of Node.Tick invocations that must pass between
	// heartbeats. That is, a leader sends heartbeat messages to maintain its
	// leadership every HeartbeatTick ticks.
	int64_t heartbeatTick_ = 0;

	// Storage is the storage for raft. raft generates entries and states to be
	// stored in storage. raft reads the persisted entries and states out of
	// Storage when it needs. raft reads out the previous state and configuration
	// out of storage when restarting.
	std::shared_ptr<Storage> storage_;
	// Applied is the last applied index. It should only be set when restarting
	// raft. raft will not return entries to the application smaller or equal to
	// Applied. If Applied is unset when restarting, raft might return previous
	// applied entries. This is a very application dependent configuration.
	uint64_t applied_ = 0;

    // AsyncStorageWrites configures the raft node to write to its local storage
    // (raft log and state machine) using a request/response message passing
    // interface instead of the default Ready/Advance function call interface.
    // Local storage messages can be pipelined and processed asynchronously
    // (with respect to Ready iteration), facilitating reduced interference
    // between Raft proposals and increased batching of log appends and state
    // machine application. As a result, use of asynchronous storage writes can
    // reduce end-to-end commit latency and increase maximum throughput.
    //
    // When true, the Ready.Message slice will include MsgStorageAppend and
    // MsgStorageApply messages. The messages will target a LocalAppendThread
    // and a LocalApplyThread, respectively. Messages to the same target must be
    // reliably processed in order. In other words, they can't be dropped (like
    // messages over the network) and those targeted at the same thread can't be
    // reordered. Messages to different targets can be processed in any order.
    //
    // MsgStorageAppend carries Raft log entries to append, election votes /
    // term changes / updated commit indexes to persist, and snapshots to apply.
    // All writes performed in service of a MsgStorageAppend must be durable
    // before response messages are delivered. However, if the MsgStorageAppend
    // carries no response messages, durability is not required. The message
    // assumes the role of the Entries, HardState, and Snapshot fields in Ready.
    //
    // MsgStorageApply carries committed entries to apply. Writes performed in
    // service of a MsgStorageApply need not be durable before response messages
    // are delivered. The message assumes the role of the CommittedEntries field
    // in Ready.
    //
    // Local messages each carry one or more response messages which should be
    // delivered after the corresponding storage write has been completed. These
    // responses may target the same node or may target other nodes. The storage
    // threads are not responsible for understanding the response messages, only
    // for delivering them to the correct target after performing the storage
    // write.
    bool AsyncStorageWrites_ = false;

	// MaxSizePerMsg limits the max byte size of each append message. Smaller
	// value lowers the raft recovery cost(initial probing and message lost
	// during normal operation). On the other side, it might affect the
	// throughput during normal replication. Note: math.MaxUint64 for unlimited,
	// 0 for at most one entry per message.
	uint64_t maxSizePerMsg_ = 0;
    // MaxCommittedSizePerReady limits the size of the committed entries which
    // can be applying at the same time.
    //
    // Despite its name (preserved for compatibility), this quota applies across
    // Ready structs to encompass all outstanding entries in unacknowledged
    // MsgStorageApply messages when AsyncStorageWrites is enabled.
	uint64_t maxCommittedSizePerReady_ = 0;
	// MaxUncommittedEntriesSize limits the aggregate byte size of the
	// uncommitted entries that may be appended to a leader's log. Once this
	// limit is exceeded, proposals will begin to return ErrProposalDropped
	// errors. Note: 0 for no limit.
	uint64_t maxUncommittedEntriesSize_ = 0;
	// MaxInflightMsgs limits the max number of in-flight append messages during
	// optimistic replication phase. The application transportation layer usually
	// has its own sending buffer over TCP/UDP. Setting MaxInflightMsgs to avoid
	// overflowing that sending buffer. TODO (xiangli): feedback to application to
	// limit the proposal rate?
	int64_t maxInflightMsgs_ = 0;

    // MaxInflightBytes limits the number of in-flight bytes in append messages.
    // Complements MaxInflightMsgs. Ignored if zero.
    //
    // This effectively bounds the bandwidth-delay product. Note that especially
    // in high-latency deployments setting this too low can lead to a dramatic
    // reduction in throughput. For example, with a peer that has a round-trip
    // latency of 100ms to the leader and this setting is set to 1 MB, there is a
    // throughput limit of 10 MB/s for this group. With RTT of 400ms, this drops
    // to 2.5 MB/s. See Little's law to understand the maths behind.
    uint64_t maxInflightBytes_ = 0;

	// CheckQuorum specifies if the leader should check quorum activity. Leader
	// steps down when quorum is not active for an electionTimeout.
	bool checkQuorum_ = false;

	// PreVote enables the Pre-Vote algorithm described in raft thesis section
	// 9.6. This prevents disruption when a node that has been partitioned away
	// rejoins the cluster.
	bool preVote_ = false;

	// ReadOnlyOption specifies how the read only request is processed.
	//
	// ReadOnlySafe guarantees the linearizability of the read only request by
	// communicating with the quorum. It is the default and suggested option.
	//
	// ReadOnlyLeaseBased ensures linearizability of the read only request by
	// relying on the leader lease. It can be affected by clock drift.
	// If the clock drift is unbounded, leader might keep the lease longer than it
	// should (clock can move backward/pause without any bound). ReadIndex is not safe
	// in that case.
	// CheckQuorum MUST be enabled if ReadOnlyOption is ReadOnlyLeaseBased.
	ReadOnlyOption readOnlyOption_ = ReadOnlySafe;

	// DisableProposalForwarding set to true means that followers will drop
	// proposals, rather than forwarding them to the leader. One use case for
	// this feature would be in a situation where the Raft leader is used to
	// compute the data of a proposal, for example, adding a timestamp from a
	// hybrid logical clock to data in a monotonically increasing way. Forwarding
	// should be disabled to prevent a follower with an inaccurate hybrid
	// logical clock from assigning the timestamp and then forwarding the data
	// to the leader.
	bool disableProposalForwarding_ = false;


    // DisableConfChangeValidation turns off propose-time verification of
    // configuration changes against the currently active configuration of the
    // raft instance. These checks are generally sensible (cannot leave a joint
    // config unless in a joint config, et cetera) but they have false positives
    // because the active configuration may not be the most recent
    // configuration. This is because configurations are activated during log
    // application, and even the leader can trail log application by an
    // unbounded number of entries.
    // Symmetrically, the mechanism has false negatives - because the check may
    // not run against the "actual" config that will be the predecessor of the
    // newly proposed one, the check may pass but the new config may be invalid
    // when it is being applied. In other words, the checks are best-effort.
    //
    // Users should *not* use this option unless they have a reliable mechanism
    // (above raft) that serializes and verifies configuration changes. If an
    // invalid configuration change enters the log and gets applied, a panic
    // will result.
    //
    // This option may be removed once false positives are no longer possible.
    // See: https://github.com/etcd-io/raft/issues/80
    bool disableConfChangeValidation_ = false;

    // StepDownOnRemoval makes the leader step down when it is removed from the
    // group or demoted to a learner.
    //
    // This behavior will become unconditional in the future. See:
    // https://github.com/etcd-io/raft/issues/83
    bool stepDownOnRemoval_ = false;

    Error validate() {
        if (id_ == None) {
            return Error("cannot use none as id");
        }
        if (IsLocalMsgTarget(id_)) {
            return Error("cannot use local target as id");
        }
        if (heartbeatTick_ <= 0) {
            return Error("heartbeat tick must be greater than 0");
        }

        if (electionTick_ <= heartbeatTick_) {
            return Error("election tick must be greater than heartbeat tick");
        }

        if (storage_ == nullptr) {
            return Error("storage cannot be nil");
        }

        if (maxUncommittedEntriesSize_ == 0) {
            maxUncommittedEntriesSize_ = noLimit;
        }

        // default MaxCommittedSizePerReady to MaxSizePerMsg because they were
        // previously the same parameter.
        if (maxCommittedSizePerReady_ == 0) {
            maxCommittedSizePerReady_ = maxSizePerMsg_;
        }

        if (maxInflightMsgs_ <= 0) {
            return Error("max inflight messages must be greater than 0");
        }

        if (maxInflightBytes_ == 0) {
            maxInflightBytes_ = noLimit;
        } else if (maxInflightBytes_ < maxSizePerMsg_) {
            return Error("max inflight bytes must be >= max message size");
        }

        if (readOnlyOption_ == ReadOnlyLeaseBased && !checkQuorum_) {
            return Error("CheckQuorum must be enabled when ReadOnlyOption is ReadOnlyLeaseBased");
        }

        return {};
    };
};

namespace detail {
struct raft {
    uint64_t id_ = 0;

    uint64_t term_ = 0;
    uint64_t vote_ = 0;

    std::vector<ReadState> readStates_;

    // the log
    std::shared_ptr<RaftLog> raftLog_;

    uint64_t maxMsgSize_ = 0;
    uint64_t maxUncommittedSize_ = 0;
    // TODO(tbg): rename to trk.
    tracker::ProgressTracker trk_;

    StateType state_ = StateFollower;

    // isLearner is true if the local raft node is a learner.
    bool isLearner_ = false;

    // msgs contains the list of messages that should be sent out immediately to
    // other nodes.
    //
    // Messages in this list must target other nodes.
    std::vector<raftpb::Message> msgs_;

    // msgsAfterAppend contains the list of messages that should be sent after
    // the accumulated unstable state (e.g. term, vote, []entry, and snapshot)
    // has been persisted to durable storage. This includes waiting for any
    // unstable state that is already in the process of being persisted (i.e.
    // has already been handed out in a prior Ready struct) to complete.
    //
    // Messages in this list may target other nodes or may target this node.
    //
    // Messages in this list have the type MsgAppResp, MsgVoteResp, or
    // MsgPreVoteResp. See the comment in raft.send for details.
    std::vector<raftpb::Message> msgsAfterAppend_;

    // the leader id
    uint64_t lead_ = 0;
    // leadTransferee is id of the leader transfer target when its value is not zero.
    // Follow the procedure defined in raft thesis 3.10.
    uint64_t leadTransferee_ = 0;
    // Only one conf change may be pending (in the log, but not yet
    // applied) at a time. This is enforced via pendingConfIndex, which
    // is set to a value >= the log index of the latest pending
    // configuration change (if any). Config changes are only allowed to
    // be proposed if the leader's applied index is greater than this
    // value.
    uint64_t pendingConfIndex_ = 0;
    // disableConfChangeValidation is Config.DisableConfChangeValidation,
    // see there for details.
    bool disableConfChangeValidation_ = false;
    // an estimate of the size of the uncommitted tail of the Raft log. Used to
    // prevent unbounded log growth. Only maintained by the leader. Reset on
    // term changes.
    uint64_t uncommittedSize_ = 0;

    std::shared_ptr<ReadOnly> readOnly_;

    // number of ticks since it reached last electionTimeout when it is leader
    // or candidate.
    // number of ticks since it reached last electionTimeout or received a
    // valid message from current leader when it is a follower.
    int64_t electionElapsed_ = 0;

    // number of ticks since it reached last heartbeatTimeout.
    // only leader keeps heartbeatElapsed.
    int64_t heartbeatElapsed_ = 0;

    bool checkQuorum_ = false;
    bool preVote_ = false;

    int64_t heartbeatTimeout_ = 0;
    int64_t electionTimeout_ = 0;
    // randomizedElectionTimeout is a random number between
    // [electiontimeout, 2 * electiontimeout - 1]. It gets reset
    // when raft changes its state to follower or candidate.
    int64_t randomizedElectionTimeout_ = 0;
    bool disableProposalForwarding_ = false;
    bool stepDownOnRemoval_ = false;

    std::function<void()> tick_;
    std::function<Error(raft & , raftpb::Message)> step_;

    // pendingReadIndexMessages is used to store messages of type MsgReadIndex
    // that can't be answered as new leader didn't committed any log in
    // current term. Those will be handled as fast as first log is committed in
    // current term.
    std::vector<raftpb::Message> pendingReadIndexMessages_;

    bool hasLeader();

    SoftState softState();

    raftpb::HardState hardState();

    void send(raftpb::Message m);

    void sendAppend(uint64_t to);

    bool maybeSendAppend(uint64_t to, bool sendIfEmpty);

    void sendHeartbeat(uint64_t to, std::string ctx);

    void bcastAppend();

    void bcastHeartbeat();

    void bcastHeartbeatWithCtx(const std::string& ctx);

    void appliedTo(uint64_t index, uint64_t size);

    void appliedSnap(const raftpb::Snapshot& snap);

    bool maybeCommit();

    void reset(uint64_t term);

    bool appendEntry(std::vector<raftpb::Entry> es);

    void tickElection();

    void tickHeartbeat();

    void becomeFollower(uint64_t term, uint64_t lead);

    void becomeCandidate();

    void becomePreCandidate();

    void becomeLeader();

    bool hasUnappliedConfChanges();

    void hup(const CampaignType& t);

    void campaign(const CampaignType& t);

    std::tuple<size_t, size_t, quorum::VoteResult> poll(uint64_t id, raftpb::MessageType t, bool v);

    Error Step(raftpb::Message m);

    void handleAppendEntries(raftpb::Message m);

    void handleHeartbeat(raftpb::Message m);

    void handleSnapshot(raftpb::Message m);

    bool restore(raftpb::Snapshot s);

    bool promotable();

    raftpb::ConfState applyConfChange(raftpb::ConfChangeV2 cc);

    raftpb::ConfState switchToConfig(tracker::Config cfg, tracker::ProgressMap prs);

    void loadState(const raftpb::HardState& state);

    bool pastElectionTimeout() const;

    void resetRandomizedElectionTimeout();

    void sendTimeoutNow(uint64_t to);

    void abortLeaderTransfer();

    bool committedEntryInCurrentTerm();

    raftpb::Message responseToReadIndexReq(raftpb::Message req, uint64_t readIndex);

    bool increaseUncommittedSize(const std::vector<raftpb::Entry>& ents);

    void reduceUncommittedSize(uint64_t s);
};

std::shared_ptr<raft> newRaft(Config& c);

Error stepLeader(raft& r, raftpb::Message m);

Error stepCandidate(raft& r, raftpb::Message m);

Error stepFollower(raft& r, raftpb::Message m);

void releasePendingReadIndexMessages(raft& r);

void sendMsgReadIndexResponse(raft& r, raftpb::Message m);

inline std::shared_ptr<raft> newRaft(Config& c) {
    auto err = c.validate();
    if (err != nullptr) {
        ERAFT_FATAL(err.String().c_str());
    }
    auto raftlog = detail::newLogWithSize(c.storage_, c.maxCommittedSizePerReady_);
    raftpb::HardState hs;
    raftpb::ConfState cs;
    std::tie(hs, cs, err) = c.storage_->InitialState();
    if (err != nullptr) {
        ERAFT_FATAL(err.String().c_str()); // TODO(bdarnell)
    }
    std::shared_ptr<raft> r(new raft());
    r->id_ = c.id_;
    r->lead_ = None;
    r->isLearner_ = false;
    r->raftLog_ = raftlog;
    r->maxMsgSize_ = c.maxSizePerMsg_;
    r->maxUncommittedSize_ = c.maxUncommittedEntriesSize_;
    r->trk_ = tracker::MakeProgressTracker(c.maxInflightMsgs_, c.maxInflightBytes_);
    r->electionTimeout_ = c.electionTick_;
    r->heartbeatTimeout_ = c.heartbeatTick_;
    r->checkQuorum_ = c.checkQuorum_;
    r->preVote_ = c.preVote_;
    r->readOnly_ = detail::newReadOnly(c.readOnlyOption_);
    r->disableProposalForwarding_ = c.disableProposalForwarding_;
    r->disableConfChangeValidation_ = c.disableConfChangeValidation_;
    r->stepDownOnRemoval_ = c.stepDownOnRemoval_;
    tracker::Config cfg;
    tracker::ProgressMap prs;
    std::tie(cfg, prs, err) = confchange::Restore({r->trk_, raftlog->lastIndex()}, cs);
    if (err != nullptr) {
        ERAFT_FATAL(err.String().c_str());
    }
    err = raftpb::Equivalent(cs, r->switchToConfig(cfg, prs));
    if (err != nullptr) {
        ERAFT_FATAL(err.String().c_str());
    }

    if (!IsEmptyHardState(hs)) {
        r->loadState(hs);
    }
    if (c.applied_ > 0) {
        raftlog->appliedTo(c.applied_, 0);
    }
    r->becomeFollower(r->term_, None);

    std::vector<std::string> nodesStrs;
    for (auto n: r->trk_.VoterNodes()) {
        nodesStrs.push_back(format("%x", n));
    }

    ERAFT_INFO("newRaft %x [peers: [%s], term: %llu, commit: %llu, applied: %llu, lastindex: %llu, lastterm: %llu]",
             r->id_, Join(nodesStrs, ",").c_str(), r->term_,
             r->raftLog_->committed_, r->raftLog_->applied_, r->raftLog_->lastIndex(),
             r->raftLog_->lastTerm());
    return r;
}

inline bool raft::hasLeader() { return lead_ != None; }

inline SoftState raft::softState() {
    return {lead_, state_};
}

inline raftpb::HardState raft::hardState() {
    raftpb::HardState hs;
    hs.set_term(term_);
    hs.set_vote(vote_);
    hs.set_commit(raftLog_->committed_);
    return hs;
}

// send schedules persisting state to a stable storage and AFTER that
// sending the message (as part of next Ready message processing).
inline void raft::send(raftpb::Message m) {
    if (m.from() == None) {
        m.set_from(id_);
    }
    if (m.type() == raftpb::MsgVote || m.type() == raftpb::MsgVoteResp || m.type() == raftpb::MsgPreVote ||
        m.type() == raftpb::MsgPreVoteResp) {
        if (m.term() == 0) {
            // All {pre-,}campaign messages need to have the term set when
            // sending.
            // - MsgVote: m.Term is the term the node is campaigning for,
            //   non-zero as we increment the term when campaigning.
            // - MsgVoteResp: m.Term is the new r.Term if the MsgVote was
            //   granted, non-zero for the same reason MsgVote is
            // - MsgPreVote: m.Term is the term the node will campaign,
            //   non-zero as we use m.Term to indicate the next term we'll be
            //   campaigning for
            // - MsgPreVoteResp: m.Term is the term received in the original
            //   MsgPreVote if the pre-vote was granted, non-zero for the
            //   same reasons MsgPreVote is
            ERAFT_FATAL("term should be set when sending %s", raftpb::MessageType_Name(m.type()).c_str());
        }
    } else {
        if (m.term() != 0) {
            ERAFT_FATAL("term should not be set when sending %s (was %d)", raftpb::MessageType_Name(m.type()).c_str(),
                      m.term());
        }
        // do not attach term to MsgProp, MsgReadIndex
        // proposals are a way to forward to the leader and
        // should be treated as local message.
        // MsgReadIndex is also forwarded to leader.
        if (m.type() != raftpb::MsgProp && m.type() != raftpb::MsgReadIndex) {
            m.set_term(term_);
        }
    }
    if (m.type() == raftpb::MsgAppResp || m.type() == raftpb::MsgVoteResp || m.type() == raftpb::MsgPreVoteResp) {
        // If async storage writes are enabled, messages added to the msgs slice
        // are allowed to be sent out before unstable state (e.g. log entry
        // writes and election votes) have been durably synced to the local
        // disk.
        //
        // For most message types, this is not an issue. However, response
        // messages that relate to "voting" on either leader election or log
        // appends require durability before they can be sent. It would be
        // incorrect to publish a vote in an election before that vote has been
        // synced to stable storage locally. Similarly, it would be incorrect to
        // acknowledge a log append to the leader before that entry has been
        // synced to stable storage locally.
        //
        // Per the Raft thesis, section 3.8 Persisted state and server restarts:
        //
        // > Raft servers must persist enough information to stable storage to
        // > survive server restarts safely. In particular, each server persists
        // > its current term and vote; this is necessary to prevent the server
        // > from voting twice in the same term or replacing log entries from a
        // > newer leader with those from a deposed leader. Each server also
        // > persists new log entries before they are counted towards the entries’
        // > commitment; this prevents committed entries from being lost or
        // > “uncommitted” when servers restart
        //
        // To enforce this durability requirement, these response messages are
        // queued to be sent out as soon as the current collection of unstable
        // state (the state that the response message was predicated upon) has
        // been durably persisted. This unstable state may have already been
        // passed to a Ready struct whose persistence is in progress or may be
        // waiting for the next Ready struct to begin being written to Storage.
        // These messages must wait for all of this state to be durable before
        // being published.
        //
        // Rejected responses (m.Reject == true) present an interesting case
        // where the durability requirement is less unambiguous. A rejection may
        // be predicated upon unstable state. For instance, a node may reject a
        // vote for one peer because it has already begun syncing its vote for
        // another peer. Or it may reject a vote from one peer because it has
        // unstable log entries that indicate that the peer is behind on its
        // log. In these cases, it is likely safe to send out the rejection
        // response immediately without compromising safety in the presence of a
        // server restart. However, because these rejections are rare and
        // because the safety of such behavior has not been formally verified,
        // we err on the side of safety and omit a `&& !m.Reject` condition
        // above.
        msgsAfterAppend_.push_back(std::move(m));
    } else {
        if (m.to() == id_) {
            ERAFT_FATAL("message should not be self-addressed when sending %s",
                      raftpb::MessageType_Name(m.type()).c_str());
        }
        msgs_.push_back(std::move(m));
    }
}

// sendAppend sends an append RPC with new entries (if any) and the
// current commit index to the given peer.
inline void raft::sendAppend(uint64_t to) {
    maybeSendAppend(to, true);
}

// maybeSendAppend sends an append RPC with new entries to the given peer,
// if necessary. Returns true if a message was sent. The sendIfEmpty
// argument controls whether messages with no entries will be sent
// ("empty" messages are useful to convey updated Commit indexes, but
// are undesirable when we're sending multiple messages in a batch).
inline bool raft::maybeSendAppend(uint64_t to, bool sendIfEmpty) {
    auto& pr = trk_.progress_[to];
    if (pr->IsPaused()) {
        return false;
    }

    auto lastIndex = pr->next_-1;
    auto nextIndex = pr->next_;
    uint64_t lastTerm = 0;
    Error err, errt, erre;
    std::tie(lastTerm, errt) = raftLog_->term(lastIndex);

    std::vector<raftpb::Entry> ents;
    // In a throttled StateReplicate only send empty MsgApp, to ensure progress.
    // Otherwise, if we had a full Inflights and all inflight messages were in
    // fact dropped, replication to that follower would stall. Instead, an empty
    // MsgApp will eventually reach the follower (heartbeats responses prompt the
    // leader to send an append), allowing it to be acked or rejected, both of
    // which will clear out Inflights.
    if (pr->state_ != tracker::StateType::StateReplicate || !pr->inflights_->Full()) {
        std::tie(ents, erre) = raftLog_->entries(nextIndex, maxMsgSize_);
    }

    if (ents.empty() && !sendIfEmpty) {
        return false;
    }

    if (errt != nullptr || erre != nullptr) { // send snapshot if we failed to get term or entries
        if (!pr->recentActive_) {
            ERAFT_DEBUG("ignore sending snapshot to %x since it is not recently active", to);
            return false;
        }

        raftpb::Snapshot snapshot;
        std::tie(snapshot, err) = raftLog_->snapshot();
        if (err != nullptr) {
            if (err == ErrSnapshotTemporarilyUnavailable) {
                ERAFT_DEBUG("%x failed to send snapshot to %x because snapshot is temporarily unavailable", id_, to);
                return false;
            }
            ERAFT_FATAL(err.String().c_str()) // TODO(bdarnell)
        }
        if (IsEmptySnap(snapshot)) {
            ERAFT_FATAL("need non-empty snapshot");
        }
        auto sindex = snapshot.metadata().index();
        auto sterm = snapshot.metadata().term();
        ERAFT_DEBUG("%x [firstindex: %d, commit: %d] sent snapshot[index: %d, term: %d] to %x [%s]",
                  id_, raftLog_->firstIndex(), raftLog_->committed_, sindex, sterm, to, pr->String().c_str());
        pr->BecomeSnapshot(sindex);
        ERAFT_DEBUG("%x paused sending replication messages to %x [%s]", id_, to, pr->String().c_str());
        raftpb::Message msg;
        msg.set_to(to);
        msg.set_type(raftpb::MsgSnap);
        *msg.mutable_snapshot() = snapshot;
        send(std::move(msg));
        return true;
    }

    // Send the actual MsgApp otherwise, and update the progress accordingly.
    err = pr->UpdateOnEntriesSend(ents.size(), payloadsSize(ents), nextIndex);
    if (err != nullptr) {
        ERAFT_FATAL("%x: %s", id_, err.String().c_str());
    }
    raftpb::Message msg;
    msg.set_to(to);
    msg.set_type(raftpb::MsgApp);
    msg.set_index(lastIndex);
    msg.set_logterm(lastTerm);
    *msg.mutable_entries() = {std::make_move_iterator(ents.begin()), std::make_move_iterator(ents.end())};
    msg.set_commit(raftLog_->committed_);
    send(std::move(msg));
    return true;
}

// sendHeartbeat sends a heartbeat RPC to the given peer.
inline void raft::sendHeartbeat(uint64_t to, std::string ctx) {
    // Attach the commit as min(to.matched, r.committed).
    // When the leader sends out heartbeat message,
    // the receiver(follower) might not be matched with the leader
    // or it might not have all the committed entries.
    // The leader MUST NOT forward the follower's commit to
    // an unmatched index.
    auto commit = std::min(trk_.progress_[to]->match_, raftLog_->committed_);
    raftpb::Message m;
    m.set_to(to);
    m.set_type(raftpb::MsgHeartbeat);
    m.set_commit(commit);
    m.set_context(std::move(ctx));

    send(std::move(m));
}

// bcastAppend sends RPC, with entries to all peers that are not up-to-date
// according to the progress recorded in r.prs.
inline void raft::bcastAppend() {
    trk_.Visit([this](uint64_t id, std::shared_ptr<tracker::Progress>&) {
        if (id == id_) {
            return;
        }
        sendAppend(id);
    });
}

// bcastHeartbeat sends RPC, without entries to all the peers.
inline void raft::bcastHeartbeat() {
    const auto& lastCtx = readOnly_->lastPendingRequestCtx();
    bcastHeartbeatWithCtx(lastCtx);
}

inline void raft::bcastHeartbeatWithCtx(const std::string& ctx) {
    trk_.Visit([this, ctx](uint64_t id, std::shared_ptr<tracker::Progress>&) {
        if (id == id_) {
            return;
        }
        sendHeartbeat(id, ctx);
    });
}

inline void raft::appliedTo(uint64_t index, uint64_t size) {
    auto oldApplied = raftLog_->applied_;
    auto newApplied = std::max(index, oldApplied);
    raftLog_->appliedTo(newApplied, size);

    if (trk_.config_.autoLeave_ && newApplied >= pendingConfIndex_ && state_ == StateLeader) {
        // If the current (and most recent, at least for this leader's term)
        // configuration should be auto-left, initiate that now. We use a
        // nil Data which unmarshals into an empty ConfChangeV2 and has the
        // benefit that appendEntry can never refuse it based on its size
        // (which registers as zero).
        raftpb::Message m;
        Error err;
        std::tie(m, err) = raftpb::detail::confChangeToMsg(nullptr);
        if (err != nullptr) {
            ERAFT_FATAL(err.String().c_str());
        }
        // NB: this proposal can't be dropped due to size, but can be
        // dropped if a leadership transfer is in progress. We'll keep
        // checking this condition on each applied entry, so either the
        // leadership transfer will succeed and the new leader will leave
        // the joint configuration, or the leadership transfer will fail,
        // and we will propose the config change on the next advance.
        err = Step(m);
        if (err != nullptr) {
            ERAFT_DEBUG("not initiating automatic transition out of joint configuration %s: %s",
                      trk_.config_.String().c_str(), err.String().c_str())
        } else {
            ERAFT_INFO("initiating automatic transition out of joint configuration %s", trk_.config_.String().c_str());
        }
    }
}

inline void raft::appliedSnap(const raftpb::Snapshot& snap) {
    auto index = snap.metadata().index();
    raftLog_->stableSnapTo(index);
    appliedTo(index, 0 /* size */);
}

// maybeCommit attempts to advance the commit index. Returns true if
// the commit index changed (in which case the caller should call
// r.bcastAppend).
inline bool raft::maybeCommit() {
    auto mci = trk_.Committed();
    return raftLog_->maybeCommit(mci, term_);
}

inline void raft::reset(uint64_t term) {
    if (term_ != term) {
        term_ = term;
        vote_ = None;
    }
    lead_ = None;

    electionElapsed_ = 0;
    heartbeatElapsed_ = 0;
    resetRandomizedElectionTimeout();

    abortLeaderTransfer();

    trk_.ResetVotes();
    trk_.Visit([this](uint64_t id, std::shared_ptr<tracker::Progress>& pr) {
        auto new_pr = new tracker::Progress();
        new_pr->match_ = 0;
        new_pr->next_ = raftLog_->lastIndex() + 1;
        new_pr->inflights_ = tracker::NewInflights(trk_.maxInflight_, trk_.maxInflightBytes_);
        new_pr->isLearner_ = pr->isLearner_;
        pr.reset(new_pr);
        if (id == id_) {
            pr->match_ = raftLog_->lastIndex();
        }
    });

    pendingConfIndex_ = 0;
    uncommittedSize_ = 0;
    readOnly_ = detail::newReadOnly(readOnly_->option_);
}

inline bool raft::appendEntry(std::vector<raftpb::Entry> es) {
    auto li = raftLog_->lastIndex();
    for (size_t i = 0; i < es.size(); i++) {
        es[i].set_term(term_);
        es[i].set_index(li + 1 + i);
    }
    // Track the size of this uncommitted proposal.
    if (!increaseUncommittedSize(es)) {
        ERAFT_WARN("%x appending new entries to log would exceed uncommitted entry size limit; dropping proposal", id_);
        // Drop the proposal.
        return false;
    }
    // use latest "last" index after truncate/append
    li = raftLog_->append(std::move(es));
    // The leader needs to self-ack the entries just appended once they have
    // been durably persisted (since it doesn't send an MsgApp to itself). This
    // response message will be added to msgsAfterAppend and delivered back to
    // this node after these entries have been written to stable storage. When
    // handled, this is roughly equivalent to:
    //
    //  r.prs.Progress[r.id].MaybeUpdate(e.Index)
    //  if r.maybeCommit() {
    //  	r.bcastAppend()
    //  }
    raftpb::Message msg;
    msg.set_to(id_);
    msg.set_type(raftpb::MsgAppResp);
    msg.set_index(li);
    send(std::move(msg));
    return true;
}

// tickElection is run by followers and candidates after r.electionTimeout.
inline void raft::tickElection() {
    electionElapsed_++;

    if (promotable() && pastElectionTimeout()) {
        electionElapsed_ = 0;
        raftpb::Message m;
        m.set_from(id_);
        m.set_type(raftpb::MsgHup);
        auto err = Step(std::move(m));
        if (err != nullptr) {
            ERAFT_DEBUG("error occurred during election: %s", err.String().c_str());
        }
    }
}

// tickHeartbeat is run by leaders to send a MsgBeat after r.heartbeatTimeout.
inline void raft::tickHeartbeat() {
    heartbeatElapsed_++;
    electionElapsed_++;

    if (electionElapsed_ >= electionTimeout_) {
        electionElapsed_ = 0;
        if (checkQuorum_) {
            raftpb::Message m;
            m.set_from(id_);
            m.set_type(raftpb::MsgCheckQuorum);
            auto err = Step(std::move(m));
            if (err != nullptr) {
                ERAFT_DEBUG("error occurred during checking sending heartbeat: %s", err.String().c_str());
            }
        }
        // If current leader cannot transfer leadership in electionTimeout, it becomes leader again.
        if (state_ == StateLeader && leadTransferee_ != None) {
            abortLeaderTransfer();
        }
    }

    if (state_ != StateLeader) {
        return;
    }

    if (heartbeatElapsed_ >= heartbeatTimeout_) {
        heartbeatElapsed_ = 0;
        raftpb::Message m;
        m.set_from(id_);
        m.set_type(raftpb::MsgBeat);
        auto err = Step(std::move(m));
        if (err != nullptr) {
            ERAFT_DEBUG("error occurred during checking sending heartbeat: %s", err.String().c_str());
        }
    }
}

inline void raft::becomeFollower(uint64_t term, uint64_t lead) {
    step_ = stepFollower;
    reset(term);
    tick_ = std::bind(&raft::tickElection, this);
    lead_ = lead;
    state_ = StateFollower;
    ERAFT_INFO("%x became follower at term %d", id_, term_);
}

inline void raft::becomeCandidate() {
    // TODO(xiangli) remove the panic when the raft implementation is stable
    if (state_ == StateLeader) {
        ERAFT_FATAL("invalid transition [leader -> candidate]");
    }
    step_ = stepCandidate;
    reset(term_ + 1);
    tick_ = std::bind(&raft::tickElection, this);
    vote_ = id_;
    state_ = StateCandidate;
    ERAFT_INFO("%x became candidate at term %d", id_, term_);
}

inline void raft::becomePreCandidate() {
    // TODO(xiangli) remove the panic when the raft implementation is stable
    if (state_ == StateLeader) {
        ERAFT_FATAL("invalid transition [leader -> pre-candidate]");
    }
    // Becoming a pre-candidate changes our step functions and state,
    // but doesn't change anything else. In particular it does not increase
    // r.Term or change r.Vote.
    step_ = stepCandidate;
    trk_.ResetVotes();
    tick_ = std::bind(&raft::tickElection, this);
    lead_ = None;
    state_ = StatePreCandidate;
    ERAFT_INFO("%x became pre-candidate at term %d", id_, term_);
}

inline void raft::becomeLeader() {
    // TODO(xiangli) remove the panic when the raft implementation is stable
    if (state_ == StateFollower) {
        ERAFT_FATAL("invalid transition [follower -> leader]");
    }
    step_ = stepLeader;
    reset(term_);
    tick_ = std::bind(&raft::tickHeartbeat, this);
    lead_ = id_;
    state_ = StateLeader;
    // Followers enter replicate mode when they've been successfully probed
    // (perhaps after having received a snapshot as a result). The leader is
    // trivially in this state. Note that r.reset() has initialized this
    // progress with the last index already.
    auto pr = trk_.progress_[id_];
    pr->BecomeReplicate();
    // The leader always has RecentActive == true; MsgCheckQuorum makes sure to
    // preserve this.
    pr->recentActive_ = true;

    // Conservatively set the pendingConfIndex to the last index in the
    // log. There may or may not be a pending config change, but it's
    // safe to delay any future proposals until we commit all our
    // pending log entries, and scanning the entire tail of the log
    // could be expensive.
    pendingConfIndex_ = raftLog_->lastIndex();

    raftpb::Entry emptyEnt;
    if (!appendEntry({emptyEnt})) {
        // This won't happen because we just called reset() above.
        ERAFT_FATAL("empty entry was dropped");
    }
    // The payloadSize of an empty entry is 0 (see TestPayloadSizeOfEmptyEntry),
    // so the preceding log append does not count against the uncommitted log
    // quota of the new leader. In other words, after the call to appendEntry,
    // r.uncommittedSize is still 0.
    ERAFT_INFO("%x became leader at term %d", id_, term_);
}

inline void raft::hup(const CampaignType& t) {
    if (state_ == StateLeader) {
        ERAFT_DEBUG("%x ignoring MsgHup because already leader", id_)
        return;
    }

    if (!promotable()) {
        ERAFT_WARN("%x is unpromotable and can not campaign", id_);
        return;
    }

    if (hasUnappliedConfChanges()) {
        ERAFT_WARN("%x cannot campaign at term %d since there are still pending configuration changes to apply", id_, term_);
        return;
    }

    ERAFT_INFO("%x is starting a new election at term %d", id_, term_);
    campaign(t);
}

inline bool raft::hasUnappliedConfChanges() {
	if (raftLog_->applied_ >= raftLog_->committed_) { // in fact applied == committed
		return false;
	}
	auto found = false;
	// Scan all unapplied committed entries to find a config change. Paginate the
	// scan, to avoid a potentially unlimited memory spike.
    auto lo = raftLog_->applied_ + 1;
	auto hi = raftLog_->committed_ + 1;
	// Reuse the maxApplyingEntsSize limit because it is used for similar purposes
	// (limiting the read of unapplied committed entries) when raft sends entries
	// via the Ready struct for application.
	// TODO(pavelkalinnikov): find a way to budget memory/bandwidth for this scan
	// outside the raft package.
	auto pageSize = raftLog_->maxApplyingEntsSize_;
    auto err = raftLog_->scan(lo, hi, pageSize, [&](const std::vector<raftpb::Entry>& ents)->Error{
        for (size_t i = 0; i < ents.size(); i++) {
            if (ents[i].type() == raftpb::EntryConfChange || ents[i].type() == raftpb::EntryConfChangeV2) {
                found = true;
                return errBreak;
            };
        }
        return nullptr;
    });
    if (err != nullptr && err != errBreak) {
        ERAFT_FATAL("error scanning unapplied entries [%d, %d): %s", lo, hi, err.String().c_str());
    }
	return found;
}

// campaign transitions the raft instance to candidate state. This must only be
// called after verifying that this is a legitimate transition.
inline void raft::campaign(const CampaignType& t) {
    if (!promotable()) {
        // This path should not be hit (callers are supposed to check), but
        // better safe than sorry.
        ERAFT_WARN("%x is unpromotable; campaign() should have been called", id_);
    }
    uint64_t term;
    raftpb::MessageType voteMsg;
    if (t == campaignPreElection) {
        becomePreCandidate();
        voteMsg = raftpb::MsgPreVote;
        // PreVote RPCs are sent for the next term before we've incremented r.Term.
        term = term_ + 1;
    } else {
        becomeCandidate();
        voteMsg = raftpb::MsgVote;
        term = term_;
    }
    std::vector<uint64_t> ids;
    {
        auto idMap = trk_.config_.voters_.IDs();
        ids = {idMap.begin(), idMap.end()};
        std::sort(ids.begin(), ids.end(), std::less<uint64_t>());
    }
    for (auto id: ids) {
        if (id == id_) {
            // The candidate votes for itself and should account for this self
            // vote once the vote has been durably persisted (since it doesn't
            // send a MsgVote to itself). This response message will be added to
            // msgsAfterAppend and delivered back to this node after the vote
            // has been written to stable storage.
            raftpb::Message msg;
            msg.set_to(id);
            msg.set_term(term);
            msg.set_type(voteRespMsgType(voteMsg));
            send(std::move(msg));
            continue;
        }
        ERAFT_INFO("%x [logterm: %d, index: %d] sent %s request to %x at term %d",
                 id_, raftLog_->lastTerm(), raftLog_->lastIndex(), MessageType_Name(voteMsg).c_str(), id, term_);

        std::string ctx;
        if (t == campaignTransfer) {
            ctx = t;
        }
        raftpb::Message m;
        m.set_term(term);
        m.set_to(id);
        m.set_type(voteMsg);
        m.set_index(raftLog_->lastIndex());
        m.set_logterm(raftLog_->lastTerm());
        m.set_context(std::move(ctx));

        send(std::move(m));
    }
}

inline std::tuple<size_t, size_t, quorum::VoteResult>
raft::poll(uint64_t id, raftpb::MessageType t, bool v) {
    if (v) {
        ERAFT_INFO("%x received %s from %x at term %d", id_, MessageType_Name(t).c_str(), id, term_);
    } else {
        ERAFT_INFO("%x received %s rejection from %x at term %d", id_, MessageType_Name(t).c_str(), id, term_)
    }
    trk_.RecordVote(id, v);
    return trk_.TallyVotes();
}

inline Error raft::Step(raftpb::Message m) {
    // Handle the message term, which may result in our stepping down to a follower.

    if (m.term() == 0) {
        // local message
    } else if (m.term() > term_) {
        if (m.type() == raftpb::MsgVote || m.type() == raftpb::MsgPreVote) {
            auto force = m.context() == campaignTransfer;
            auto inLease = checkQuorum_ && lead_ != None && electionElapsed_ < electionTimeout_;
            if (!force && inLease) {
                // If a server receives a RequestVote request within the minimum election timeout
                // of hearing from a current leader, it does not update its term or grant its vote
                ERAFT_INFO(
                        "%x [logterm: %d, index: %d, vote: %x] ignored %s from %x [logterm: %d, index: %d] at term %d: lease is not expired (remaining ticks: %d)",
                        id_, raftLog_->lastTerm(), raftLog_->lastIndex(), vote_, MessageType_Name(m.type()).c_str(),
                        m.from(), m.logterm(),
                        m.index(), term_, electionTimeout_ - electionElapsed_)
                return {};
            }
        }

        if (m.type() == raftpb::MsgPreVote) {
            // Never change our term in response to a PreVote
        } else if (m.type() == raftpb::MsgPreVoteResp && !m.reject()) {
            // We send pre-vote requests with a term in our future. If the
            // pre-vote is granted, we will increment our term when we get a
            // quorum. If it is not, the term comes from the node that
            // rejected our vote so we should become a follower at the new
            // term.
        } else {
            ERAFT_INFO("%x [term: %d] received a %s message with higher term from %x [term: %d]",
                     id_, term_, MessageType_Name(m.type()).c_str(), m.from(), m.term());
            if (m.type() == raftpb::MsgApp || m.type() == raftpb::MsgHeartbeat || m.type() == raftpb::MsgSnap) {
                becomeFollower(m.term(), m.from());
            } else {
                becomeFollower(m.term(), None);
            }
        }
    } else if (m.term() < term_) {
        if ((checkQuorum_ || preVote_) && (m.type() == raftpb::MsgHeartbeat || m.type() == raftpb::MsgApp)) {
            // We have received messages from a leader at a lower term. It is possible
            // that these messages were simply delayed in the network, but this could
            // also mean that this node has advanced its term number during a network
            // partition, and it is now unable to either win an election or to rejoin
            // the majority on the old term. If checkQuorum is false, this will be
            // handled by incrementing term numbers in response to MsgVote with a
            // higher term, but if checkQuorum is true we may not advance the term on
            // MsgVote and must generate other messages to advance the term. The net
            // result of these two features is to minimize the disruption caused by
            // nodes that have been removed from the cluster's configuration: a
            // removed node will send MsgVotes (or MsgPreVotes) which will be ignored,
            // but it will not receive MsgApp or MsgHeartbeat, so it will not create
            // disruptive term increases, by notifying leader of this node's activeness.
            // The above comments also true for Pre-Vote
            //
            // When follower gets isolated, it soon starts an election ending
            // up with a higher term than leader, although it won't receive enough
            // votes to win the election. When it regains connectivity, this response
            // with "pb.MsgAppResp" of higher term would force leader to step down.
            // However, this disruption is inevitable to free this stuck node with
            // fresh election. This can be prevented with Pre-Vote phase.
            raftpb::Message msg;
            msg.set_to(m.from());
            msg.set_type(raftpb::MsgAppResp);
            send(std::move(msg));
        } else if (m.type() == raftpb::MsgPreVote) {
            // Before Pre-Vote enable, there may have candidate with higher term,
            // but less log. After update to Pre-Vote, the cluster may deadlock if
            // we drop messages with a lower term.
            ERAFT_INFO("%x [logterm: %d, index: %d, vote: %x] rejected %s from %x [logterm: %d, index: %d] at term %d",
                     id_, raftLog_->lastTerm(), raftLog_->lastIndex(), vote_, MessageType_Name(m.type()).c_str(),
                     m.from(), m.logterm(), m.index(),
                     term_);
            raftpb::Message msg;
            msg.set_to(m.from());
            msg.set_term(term_);
            msg.set_type(raftpb::MsgPreVoteResp);
            msg.set_reject(true);
            send(std::move(msg));

        } else if (m.type() == raftpb::MsgStorageAppendResp) {
            if (m.index() != 0) {
                // Don't consider the appended log entries to be stable because
                // they may have been overwritten in the unstable log during a
                // later term. See the comment in newStorageAppendResp for more
                // about this race.
                ERAFT_INFO("%x [term: %d] ignored entry appends from a %s message with lower term [term: %d]",
                         id_, term_, MessageType_Name(m.type()).c_str(), m.term());
            }
            if (!IsEmptySnap(m.snapshot())) {
                // Even if the snapshot applied under a different term, its
                // application is still valid. Snapshots carry committed
                // (term-independent) state.
                appliedSnap(m.snapshot());
            }
        } else {
            // ignore other cases
            ERAFT_INFO("%x [term: %d] ignored a %s message with lower term from %x [term: %d]",
                     id_, term_, MessageType_Name(m.type()).c_str(), m.from(), m.term());
        }
        return {};
    }

    switch (m.type()) {
        case raftpb::MsgHup: {
            if (preVote_) {
                hup(campaignPreElection);
            } else {
                hup(campaignElection);
            }
            break;
        }

        case raftpb::MsgStorageAppendResp: {
            if (m.index() != 0) {
                raftLog_->stableTo(m.index(), m.logterm());
            }
            if (!IsEmptySnap(m.snapshot())) {
                appliedSnap(m.snapshot());
            }
            break;
        }
        case raftpb::MsgStorageApplyResp: {
            if (!m.entries().empty()) {
                auto index = m.entries(m.entries().size() - 1).index();
                appliedTo(index, entsSize(m.entries()));
                reduceUncommittedSize(payloadsSize(m.entries()));
            }
            break;
        }
        case raftpb::MsgVote:
        case raftpb::MsgPreVote: {
            // We can vote if this is a repeat of a vote we've already cast...
            auto canVote = vote_ == m.from() ||
                           // ...we haven't voted and we don't think there's a leader yet in this term...
                           (vote_ == None && lead_ == None) ||
                           // ...or this is a PreVote for a future term...
                           (m.type() == raftpb::MsgPreVote && m.term() > term_);
            // ...and we believe the candidate is up to date.
            if (canVote && raftLog_->isUpToDate(m.index(), m.logterm())) {
                // Note: it turns out that that learners must be allowed to cast votes.
                // This seems counter- intuitive but is necessary in the situation in which
                // a learner has been promoted (i.e. is now a voter) but has not learned
                // about this yet.
                // For example, consider a group in which id=1 is a learner and id=2 and
                // id=3 are voters. A configuration change promoting 1 can be committed on
                // the quorum `{2,3}` without the config change being appended to the
                // learner's log. If the leader (say 2) fails, there are de facto two
                // voters remaining. Only 3 can win an election (due to its log containing
                // all committed entries), but to do so it will need 1 to vote. But 1
                // considers itself a learner and will continue to do so until 3 has
                // stepped up as leader, replicates the conf change to 1, and 1 applies it.
                // Ultimately, by receiving a request to vote, the learner realizes that
                // the candidate believes it to be a voter, and that it should act
                // accordingly. The candidate's config may be stale, too; but in that case
                // it won't win the election, at least in the absence of the bug discussed
                // in:
                // https://github.com/etcd-io/etcd/issues/7625#issuecomment-488798263.
                ERAFT_INFO("%x [logterm: %d, index: %d, vote: %x] cast %s for %x [logterm: %d, index: %d] at term %d",
                         id_, raftLog_->lastTerm(), raftLog_->lastIndex(), vote_, MessageType_Name(m.type()).c_str(),
                         m.from(), m.logterm(), m.index(), term_);
                // When responding to Msg{Pre,}Vote messages we include the term
                // from the message, not the local term. To see why, consider the
                // case where a single node was previously partitioned away and
                // it's local term is now out of date. If we include the local term
                // (recall that for pre-votes we don't update the local term), the
                // (pre-)campaigning node on the other end will proceed to ignore
                // the message (it ignores all out of date messages).
                // The term in the original message and current local term are the
                // same in the case of regular votes, but different for pre-votes.
                raftpb::Message msg;
                msg.set_to(m.from());
                msg.set_term(m.term());
                msg.set_type(voteRespMsgType(m.type()));
                send(std::move(msg));
                if (m.type() == raftpb::MsgVote) {
                    // Only record real votes.
                    electionElapsed_ = 0;
                    vote_ = m.from();
                }
            } else {
                ERAFT_INFO(
                        "%x [logterm: %d, index: %d, vote: %x] rejected %s from %x [logterm: %d, index: %d] at term %d",
                        id_, raftLog_->lastTerm(), raftLog_->lastIndex(), vote_, MessageType_Name(m.type()).c_str(),
                        m.from(), m.logterm(), m.index(), term_);
                raftpb::Message msg;
                msg.set_to(m.from());
                msg.set_term(term_);
                msg.set_type(voteRespMsgType(m.type()));
                msg.set_reject(true);
                send(std::move(msg));
            }
            break;
        }
        default: {
            auto err = step_(*this, std::move(m));
            if (err != nullptr) {
                return err;
            }
        }
    }
    return {};
}

inline Error stepLeader(raft& r, raftpb::Message m) {
    // These message types do not require any progress for m.From.
    switch (m.type()) {
        case raftpb::MsgBeat: {
            r.bcastHeartbeat();
            return {};
        }
        case raftpb::MsgCheckQuorum: {
            if (!r.trk_.QuorumActive()) {
                ERAFT_WARN("%x stepped down to follower since quorum is not active", r.id_);
                r.becomeFollower(r.term_, None);
            }
            // Mark everyone (but ourselves) as inactive in preparation for the next
            // CheckQuorum.
            r.trk_.Visit([&r](uint64_t id, std::shared_ptr<tracker::Progress>& pr) {
                if (id != r.id_) {
                    pr->recentActive_ = false;
                }
            });
            return {};
        }
        case raftpb::MsgProp: {
            if (m.entries().empty()) {
                ERAFT_FATAL("%x stepped empty MsgProp", r.id_);
            }
            if (r.trk_.progress_.data().count(r.id_) == 0) {
                // If we are not currently a member of the range (i.e. this node
                // was removed from the configuration while serving as leader),
                // drop any new proposals.
                return ErrProposalDropped;
            }
            if (r.leadTransferee_ != None) {
                ERAFT_DEBUG("%x [term %d] transfer leadership to %x is in progress; dropping proposal", r.id_, r.term_,
                          r.leadTransferee_);
                return ErrProposalDropped;
            }

            for (int i = 0; i < m.entries().size(); i++) {
                auto& e = m.entries(i);
                std::unique_ptr<raftpb::ConfChangeI> cc;
                if (e.type() == raftpb::EntryConfChange) {
                    raftpb::ConfChange ccc;
                    if (!ccc.ParseFromString(e.data())) {
                        ERAFT_FATAL("ConfChange ParseFromString error");
                    }
                    cc.reset(new raftpb::ConfChangeWrap(std::move(ccc)));
                } else if (e.type() == raftpb::EntryConfChangeV2) {
                    raftpb::ConfChangeV2 ccc;
                    if (!ccc.ParseFromString(e.data())) {
                        ERAFT_FATAL("ConfChangeV2 ParseFromString error");
                    }
                    cc.reset(new raftpb::ConfChangeV2Wrap(std::move(ccc)));
                }
                if (cc != nullptr) {
                    auto alreadyPending = r.pendingConfIndex_ > r.raftLog_->applied_;
                    auto alreadyJoint = !r.trk_.config_.voters_[1].data().empty();
                    auto wantsLeaveJoint = cc->AsV2().changes().empty();

                    std::string failedCheck;
                    if (alreadyPending) {
                        failedCheck = format("possible unapplied conf change at index %d (applied to %d)",
                                         r.pendingConfIndex_, r.raftLog_->applied_);
                    } else if (alreadyJoint && !wantsLeaveJoint) {
                        failedCheck = "must transition out of joint config first";
                    } else if (!alreadyJoint && wantsLeaveJoint) {
                        failedCheck = "not in joint state; refusing empty conf change";
                    }

                    if (!failedCheck.empty() && !r.disableConfChangeValidation_) {
                        ERAFT_INFO("%x ignoring conf change [%s] at config %s: %s", r.id_, cc->String().c_str(),
                                 r.trk_.config_.String().c_str(), failedCheck.c_str());
                        raftpb::Entry ent;
                        ent.set_type(raftpb::EntryNormal);
                        *m.mutable_entries((int) i) = ent;
                    } else {
                        r.pendingConfIndex_ = r.raftLog_->lastIndex() + i + 1;
                    }
                }
            }
            // TODO(wangzhiyong)
            if (!r.appendEntry({m.entries().begin(), m.entries().end()})) {
                return ErrProposalDropped;
            }
            r.bcastAppend();
            return {};
        }
        case raftpb::MsgReadIndex: {
            // only one voting member (the leader) in the cluster
            if (r.trk_.IsSingleton()) {
                auto resp = r.responseToReadIndexReq(m, r.raftLog_->committed_);
                if (resp.to() != None) {
                    r.send(std::move(resp));
                }
                return {};
            }

            // Postpone read only request when this leader has not committed
            // any log entry at its term.
            if (!r.committedEntryInCurrentTerm()) {
                r.pendingReadIndexMessages_.push_back(std::move(m));
                return {};
            }

            sendMsgReadIndexResponse(r, std::move(m));

            return {};
        }
        case raftpb::MsgForgetLeader:
            return nullptr; // noop on leader
        default: {
            break;
        }
    }

    // All other message types require a progress for m.From (pr).
    auto iter = r.trk_.progress_.data().find(m.from());
    if (iter == r.trk_.progress_.data().end()) {
        ERAFT_DEBUG("%x no progress available for %x", r.id_, m.from());
        return {};
    }
    auto& pr = iter->second;
    switch (m.type()) {
        case raftpb::MsgAppResp: {
            // NB: this code path is also hit from (*raft).advance, where the leader steps
            // an MsgAppResp to acknowledge the appended entries in the last Ready.
            pr->recentActive_ = true;

            if (m.reject()) {
                // RejectHint is the suggested next base entry for appending (i.e.
                // we try to append entry RejectHint+1 next), and LogTerm is the
                // term that the follower has at index RejectHint. Older versions
                // of this library did not populate LogTerm for rejections and it
                // is zero for followers with an empty log.
                //
                // Under normal circumstances, the leader's log is longer than the
                // follower's and the follower's log is a prefix of the leader's
                // (i.e. there is no divergent uncommitted suffix of the log on the
                // follower). In that case, the first probe reveals where the
                // follower's log ends (RejectHint=follower's last index) and the
                // subsequent probe succeeds.
                //
                // However, when networks are partitioned or systems overloaded,
                // large divergent log tails can occur. The naive attempt, probing
                // entry by entry in decreasing order, will be the product of the
                // length of the diverging tails and the network round-trip latency,
                // which can easily result in hours of time spent probing and can
                // even cause outright outages. The probes are thus optimized as
                // described below.
                ERAFT_DEBUG("%x received MsgAppResp(rejected, hint: (index %d, term %d)) from %x for index %d",
                          r.id_, m.rejecthint(), m.logterm(), m.from(), m.index());
                auto nextProbeIdx = m.rejecthint();
                if (m.logterm() > 0) {
                    // If the follower has an uncommitted log tail, we would end up
                    // probing one by one until we hit the common prefix.
                    //
                    // For example, if the leader has:
                    //
                    //   idx        1 2 3 4 5 6 7 8 9
                    //              -----------------
                    //   term (L)   1 3 3 3 5 5 5 5 5
                    //   term (F)   1 1 1 1 2 2
                    //
                    // Then, after sending an append anchored at (idx=9,term=5) we
                    // would receive a RejectHint of 6 and LogTerm of 2. Without the
                    // code below, we would try an append at index 6, which would
                    // fail again.
                    //
                    // However, looking only at what the leader knows about its own
                    // log and the rejection hint, it is clear that a probe at index
                    // 6, 5, 4, 3, and 2 must fail as well:
                    //
                    // For all of these indexes, the leader's log term is larger than
                    // the rejection's log term. If a probe at one of these indexes
                    // succeeded, its log term at that index would match the leader's,
                    // i.e. 3 or 5 in this example. But the follower already told the
                    // leader that it is still at term 2 at index 6, and since the
                    // log term only ever goes up (within a log), this is a contradiction.
                    //
                    // At index 1, however, the leader can draw no such conclusion,
                    // as its term 1 is not larger than the term 2 from the
                    // follower's rejection. We thus probe at 1, which will succeed
                    // in this example. In general, with this approach we probe at
                    // most once per term found in the leader's log.
                    //
                    // There is a similar mechanism on the follower (implemented in
                    // handleAppendEntries via a call to findConflictByTerm) that is
                    // useful if the follower has a large divergent uncommitted log
                    // tail[1], as in this example:
                    //
                    //   idx        1 2 3 4 5 6 7 8 9
                    //              -----------------
                    //   term (L)   1 3 3 3 3 3 3 3 7
                    //   term (F)   1 3 3 4 4 5 5 5 6
                    //
                    // Naively, the leader would probe at idx=9, receive a rejection
                    // revealing the log term of 6 at the follower. Since the leader's
                    // term at the previous index is already smaller than 6, the leader-
                    // side optimization discussed above is ineffective. The leader thus
                    // probes at index 8 and, naively, receives a rejection for the same
                    // index and log term 5. Again, the leader optimization does not improve
                    // over linear probing as term 5 is above the leader's term 3 for that
                    // and many preceding indexes; the leader would have to probe linearly
                    // until it would finally hit index 3, where the probe would succeed.
                    //
                    // Instead, we apply a similar optimization on the follower. When the
                    // follower receives the probe at index 8 (log term 3), it concludes
                    // that all of the leader's log preceding that index has log terms of
                    // 3 or below. The largest index in the follower's log with a log term
                    // of 3 or below is index 3. The follower will thus return a rejection
                    // for index=3, log term=3 instead. The leader's next probe will then
                    // succeed at that index.
                    //
                    // [1]: more precisely, if the log terms in the large uncommitted
                    // tail on the follower are larger than the leader's. At first,
                    // it may seem unintuitive that a follower could even have such
                    // a large tail, but it can happen:
                    //
                    // 1. Leader appends (but does not commit) entries 2 and 3, crashes.
                    //   idx        1 2 3 4 5 6 7 8 9
                    //              -----------------
                    //   term (L)   1 2 2     [crashes]
                    //   term (F)   1
                    //   term (F)   1
                    //
                    // 2. a follower becomes leader and appends entries at term 3.
                    //              -----------------
                    //   term (x)   1 2 2     [down]
                    //   term (F)   1 3 3 3 3
                    //   term (F)   1
                    //
                    // 3. term 3 leader goes down, term 2 leader returns as term 4
                    //    leader. It commits the log & entries at term 4.
                    //
                    //              -----------------
                    //   term (L)   1 2 2 2
                    //   term (x)   1 3 3 3 3 [down]
                    //   term (F)   1
                    //              -----------------
                    //   term (L)   1 2 2 2 4 4 4
                    //   term (F)   1 3 3 3 3 [gets probed]
                    //   term (F)   1 2 2 2 4 4 4
                    //
                    // 4. the leader will now probe the returning follower at index
                    //    7, the rejection points it at the end of the follower's log
                    //    which is at a higher log term than the actually committed
                    //    log.
                    std::tie(nextProbeIdx, std::ignore) = r.raftLog_->findConflictByTerm(m.rejecthint(), m.logterm());
                }
                if (pr->MaybeDecrTo(m.index(), nextProbeIdx)) {
                    ERAFT_DEBUG("%x decreased progress of %x to [%s]", r.id_, m.from(), pr->String().c_str());
                    if (pr->state_ == tracker::StateType::StateReplicate) {
                        pr->BecomeProbe();
                    }
                    r.sendAppend(m.from());
                }
            } else {
                auto oldPaused = pr->IsPaused();
                // We want to update our tracking if the response updates our
                // matched index or if the response can move a probing peer back
                // into StateReplicate (see heartbeat_rep_recovers_from_probing.txt
                // for an example of the latter case).
                // NB: the same does not make sense for StateSnapshot - if `m.Index`
                // equals pr.Match we know we don't m.Index+1 in our log, so moving
                // back to replicating state is not useful; besides pr.PendingSnapshot
                // would prevent it.
                if (pr->MaybeUpdate(m.index()) || (pr->match_ == m.index() && pr->state_ == tracker::StateType::StateProbe)) {
                    if (pr->state_ == tracker::StateType::StateProbe) {
                        pr->BecomeReplicate();
                    } else if (pr->state_ == tracker::StateType::StateSnapshot && pr->match_ + 1 >= r.raftLog_->firstIndex()) {
                        // Note that we don't take into account PendingSnapshot to
                        // enter this branch. No matter at which index a snapshot
                        // was actually applied, as long as this allows catching up
                        // the follower from the log, we will accept it. This gives
                        // systems more flexibility in how they implement snapshots;
                        // see the comments on PendingSnapshot.
                        ERAFT_DEBUG("%x recovered from needing snapshot, resumed sending replication messages to %x [%s]",
                                  r.id_,
                                  m.from(), pr->String().c_str());
                        // Transition back to replicating state via probing state
                        // (which takes the snapshot into account). If we didn't
                        // move to replicating state, that would only happen with
                        // the next round of appends (but there may not be a next
                        // round for a while, exposing an inconsistent RaftStatus).
                        pr->BecomeProbe();
                        pr->BecomeReplicate();
                    } else if (pr->state_ == tracker::StateType::StateReplicate) {
                        pr->inflights_->FreeLE(m.index());
                    }

                    if (r.maybeCommit()) {
                        // committed index has progressed for the term, so it is safe
                        // to respond to pending read index requests
                        releasePendingReadIndexMessages(r);
                        r.bcastAppend();
                    } else if (oldPaused) {
                        // If we were paused before, this node may be missing the
                        // latest commit index, so send it.
                        r.sendAppend(m.from());
                    }
                    // We've updated flow control information above, which may
                    // allow us to send multiple (size-limited) in-flight messages
                    // at once (such as when transitioning from probe to
                    // replicate, or when freeTo() covers multiple messages). If
                    // we have more entries to send, send as many messages as we
                    // can (without sending empty messages for the commit index)
                    // TODO(wangzhiyong)
                    if (r.id_ != m.from()) {
                        while (r.maybeSendAppend(m.from(), false)) {
                        }
                    }
                    // Transfer leadership is in progress.
                    if (m.from() == r.leadTransferee_ && pr->match_ == r.raftLog_->lastIndex()) {
                        ERAFT_INFO("%x sent MsgTimeoutNow to %x after received MsgAppResp", r.id_, m.from());
                        r.sendTimeoutNow(m.from());
                    }
                }
            }
            break;
        }
        case raftpb::MsgHeartbeatResp: {
            pr->recentActive_ = true;
            pr->msgAppFlowPaused_ = false;

            // NB: if the follower is paused (full Inflights), this will still send an
            // empty append, allowing it to recover from situations in which all the
            // messages that filled up Inflights in the first place were dropped. Note
            // also that the outgoing heartbeat already communicated the commit index.
            //
            // If the follower is fully caught up but also in StateProbe (as can happen
            // if ReportUnreachable was called), we also want to send an append (it will
            // be empty) to allow the follower to transition back to StateReplicate once
            // it responds.
            //
            // Note that StateSnapshot typically satisfies pr.Match < lastIndex, but
            // `pr.Paused()` is always true for StateSnapshot, so sendAppend is a
            // no-op.
            if (pr->match_ < r.raftLog_->lastIndex() || pr->state_ == tracker::StateType::StateProbe) {
                r.sendAppend(m.from());
            }

            if (r.readOnly_->option_ != ReadOnlySafe || m.context().empty()) {
                return {};
            }

            if (r.trk_.config_.voters_.GetVoteResult(r.readOnly_->recvAck(m.from(), m.context())) !=
                quorum::VoteResult::VoteWon) {
                return {};
            }

            auto rss = r.readOnly_->advance(m);
            for (auto& rs: rss) {
                auto resp = r.responseToReadIndexReq(rs->req_, rs->index_);
                if (resp.to() != None) {
                    r.send(std::move(resp));
                }
            }
            break;
        }
        case raftpb::MsgSnapStatus: {
            if (pr->state_ != tracker::StateType::StateSnapshot) {
                return {};
            }
            if (!m.reject()) {
                pr->BecomeProbe();
                ERAFT_DEBUG("%x snapshot succeeded, resumed sending replication messages to %x [%s]", r.id_, m.from(),
                          pr->String().c_str());
            } else {
                // NB: the order here matters or we'll be probing erroneously from
                // the snapshot index, but the snapshot never applied.
                pr->pendingSnapshot_ = 0;
                pr->BecomeProbe();
                ERAFT_DEBUG("%x snapshot failed, resumed sending replication messages to %x [%s]", r.id_, m.from(),
                          pr->String().c_str());
            }
            // If snapshot finish, wait for the MsgAppResp from the remote node before sending
            // out the next MsgApp.
            // If snapshot failure, wait for a heartbeat interval before next try
            pr->msgAppFlowPaused_ = true;
            break;
        }
        case raftpb::MsgUnreachable: {
            // During optimistic replication, if the remote becomes unreachable,
            // there is huge probability that a MsgApp is lost.
            if (pr->state_ == tracker::StateType::StateReplicate) {
                pr->BecomeProbe();
            }
            ERAFT_DEBUG("%x failed to send message to %x because it is unreachable [%s]", r.id_, m.from(),
                      pr->String().c_str());
            break;
        }
        case raftpb::MsgTransferLeader: {
            if (pr->isLearner_) {
                ERAFT_DEBUG("%x is learner. Ignored transferring leadership", r.id_);
                return {};
            }
            auto leadTransferee = m.from();
            auto lastLeadTransferee = r.leadTransferee_;
            if (lastLeadTransferee != None) {
                if (lastLeadTransferee == leadTransferee) {
                    ERAFT_INFO("%x [term %d] transfer leadership to %x is in progress, ignores request to same node %x",
                             r.id_, r.term_, leadTransferee, leadTransferee);
                    return {};
                }
                r.abortLeaderTransfer();
                ERAFT_INFO("%x [term %d] abort previous transferring leadership to %x", r.id_, r.term_,
                         lastLeadTransferee);
            }
            if (leadTransferee == r.id_) {
                ERAFT_DEBUG("%x is already leader. Ignored transferring leadership to self", r.id_);
                return {};
            }
            // Transfer leadership to third party.
            ERAFT_INFO("%x [term %d] starts to transfer leadership to %x", r.id_, r.term_, leadTransferee);
            // Transfer leadership should be finished in one electionTimeout, so reset r.electionElapsed.
            r.electionElapsed_ = 0;
            r.leadTransferee_ = leadTransferee;
            if (pr->match_ == r.raftLog_->lastIndex()) {
                r.sendTimeoutNow(leadTransferee);
                ERAFT_INFO("%x sends MsgTimeoutNow to %x immediately as %x already has up-to-date log", r.id_,
                         leadTransferee, leadTransferee);
            } else {
                r.sendAppend(leadTransferee);
            }
            break;
        }
        default: {
            break;
        }
    }
    return {};
}

// stepCandidate is shared by StateCandidate and StatePreCandidate; the difference is
// whether they respond to MsgVoteResp or MsgPreVoteResp.
inline Error stepCandidate(raft& r, raftpb::Message m) {
    switch (m.type()) {
        case raftpb::MsgProp: {
            ERAFT_INFO("%x no leader at term %d; dropping proposal", r.id_, r.term_);
            return ErrProposalDropped;
        }
        case raftpb::MsgApp: {
            r.becomeFollower(m.term(), m.from()); // always m.Term == r.Term
            r.handleAppendEntries(m);
            break;
        }
        case raftpb::MsgHeartbeat: {
            r.becomeFollower(m.term(), m.from()); // always m.Term == r.Term
            r.handleHeartbeat(m);
            break;
        }
        case raftpb::MsgSnap: {
            r.becomeFollower(m.term(), m.from()); // always m.Term == r.Term
            r.handleSnapshot(m);
            break;
        }
        case raftpb::MsgPreVoteResp:
        case raftpb::MsgVoteResp: {
            // Only handle vote responses corresponding to our candidacy (while in
            // StateCandidate, we may get stale MsgPreVoteResp messages in this term from
            // our pre-candidate state).
            bool match = false;
            if ((r.state_ == StatePreCandidate && m.type() == raftpb::MsgPreVoteResp) ||
                (r.state_ != StatePreCandidate && m.type() == raftpb::MsgVoteResp)) {
                match = true;
            }
            if (!match) {
                break;
            }
            size_t gr = 0;
            size_t rj = 0;
            quorum::VoteResult res;
            std::tie(gr, rj, res) = r.poll(m.from(), m.type(), !m.reject());
            ERAFT_INFO("%x has received %d %s votes and %d vote rejections", r.id_, gr,
                     MessageType_Name(m.type()).c_str(), rj);
            switch (res) {
                case quorum::VoteResult::VoteWon: {
                    if (r.state_ == StatePreCandidate) {
                        r.campaign(campaignElection);
                    } else {
                        r.becomeLeader();
                        r.bcastAppend();
                    }
                    break;
                }
                case quorum::VoteResult::VoteLost: {
                    // pb.MsgPreVoteResp contains future term of pre-candidate
                    // m.Term > r.Term; reuse r.Term
                    r.becomeFollower(r.term_, None);
                    break;
                }
                default: {
                    break;
                }
            }
            break;
        }
        case raftpb::MsgTimeoutNow: {
            ERAFT_DEBUG("%x [term %d state %v] ignored MsgTimeoutNow from %x", r.id_, r.term_, r.state_, m.from());
            break;
        }
        default: {
            break;
        }
    }
    return {};
}

inline Error stepFollower(raft& r, raftpb::Message m) {
    switch (m.type()) {
        case raftpb::MsgProp: {
            if (r.lead_ == None) {
                ERAFT_INFO("%x no leader at term %d; dropping proposal", r.id_, r.term_);
                return ErrProposalDropped;
            } else if (r.disableProposalForwarding_) {
                ERAFT_INFO("%x not forwarding to leader %x at term %d; dropping proposal", r.id_, r.lead_, r.term_);
                return ErrProposalDropped;
            }
            m.set_to(r.lead_);
            r.send(std::move(m));
            break;
        }
        case raftpb::MsgApp: {
            r.electionElapsed_ = 0;
            r.lead_ = m.from();
            r.handleAppendEntries(std::move(m));
            break;
        }
        case raftpb::MsgHeartbeat: {
            r.electionElapsed_ = 0;
            r.lead_ = m.from();
            r.handleHeartbeat(std::move(m));
            break;
        }
        case raftpb::MsgSnap: {
            r.electionElapsed_ = 0;
            r.lead_ = m.from();
            r.handleSnapshot(std::move(m));
            break;
        }
        case raftpb::MsgTransferLeader: {
            if (r.lead_ == None) {
                ERAFT_INFO("%x no leader at term %d; dropping leader transfer msg", r.id_, r.term_);
                return {};
            }
            m.set_to(r.lead_);
            r.send(std::move(m));
            break;
        }
        case raftpb::MsgForgetLeader: {
            if (r.readOnly_->option_ == ReadOnlyLeaseBased) {
                ERAFT_ERROR("ignoring MsgForgetLeader due to ReadOnlyLeaseBased")
                return nullptr;
            }
            if (r.lead_ != None) {
                ERAFT_INFO("%x forgetting leader %x at term %d", r.id_, r.lead_, r.term_);
                r.lead_ = None;
            }
            break;
        }
        case raftpb::MsgTimeoutNow: {
            ERAFT_INFO("%x [term %d] received MsgTimeoutNow from %x and starts an election to get leadership.", r.id_,
                     r.term_, m.from());
            // Leadership transfers never use pre-vote even if r.preVote is true; we
            // know we are not recovering from a partition so there is no need for the
            // extra round trip.
            r.hup(campaignTransfer);
            break;
        }
        case raftpb::MsgReadIndex: {
            if (r.lead_ == None) {
                ERAFT_INFO("%x no leader at term %d; dropping index reading msg", r.id_, r.term_);
                return {};
            }
            m.set_to(r.lead_);
            r.send(std::move(m));
            break;
        }
        case raftpb::MsgReadIndexResp: {
            if (m.entries().size() != 1) {
                ERAFT_ERROR("%x invalid format of MsgReadIndexResp from %x, entries count: %d", r.id_, m.from(),
                          m.entries().size());
                return {};
            }
            r.readStates_.emplace_back(m.index(), m.entries(0).data());
            break;
        }
        default: {

        }
    }
    return {};
};

inline void raft::handleAppendEntries(raftpb::Message m) {
	if (m.index() < raftLog_->committed_) {
        raftpb::Message msg;
        msg.set_to(m.from());
        msg.set_type(raftpb::MsgAppResp);
        msg.set_index(raftLog_->committed_);
		send(std::move(msg));
		return;
	}
    uint64_t mlastIndex = 0;
    bool ok = false;
    std::tie(mlastIndex, ok) = raftLog_->maybeAppend(m.index(), m.logterm(), m.commit(), {m.entries().begin(), m.entries().end()});
	if (ok) {
        raftpb::Message msg;
        msg.set_to(m.from());
        msg.set_type(raftpb::MsgAppResp);
        msg.set_index(mlastIndex);
		send(std::move(msg));
		return;
	}
	ERAFT_DEBUG("%x [logterm: %d, index: %d] rejected MsgApp [logterm: %d, index: %d] from %x",
		id_, raftLog_->zeroTermOnOutOfBounds(raftLog_->term(m.index())), m.index(), m.logterm(), m.index(), m.from());

	// Our log does not match the leader's at index m.Index. Return a hint to the
	// leader - a guess on the maximal (index, term) at which the logs match. Do
	// this by searching through the follower's log for the maximum (index, term)
	// pair with a term <= the MsgApp's LogTerm and an index <= the MsgApp's
	// Index. This can help skip all indexes in the follower's uncommitted tail
	// with terms greater than the MsgApp's LogTerm.
	//
	// See the other caller for findConflictByTerm (in stepLeader) for a much more
	// detailed explanation of this mechanism.

	// NB: m.Index >= raftLog.committed by now (see the early return above), and
	// raftLog.lastIndex() >= raftLog.committed by invariant, so min of the two is
	// also >= raftLog.committed. Hence, the findConflictByTerm argument is within
	// the valid interval, which then will return a valid (index, term) pair with
	// a non-zero term (unless the log is empty). However, it is safe to send a zero
	// LogTerm in this response in any case, so we don't verify it here.
	auto hintIndex = std::min(m.index(), raftLog_->lastIndex());
    uint64_t hintTerm = 0;
	std::tie(hintIndex, hintTerm) = raftLog_->findConflictByTerm(hintIndex, m.logterm());
    raftpb::Message msg;
    msg.set_to(m.from());
    msg.set_type(raftpb::MsgAppResp);
    msg.set_index(m.index());
    msg.set_reject(true);
    msg.set_rejecthint(hintIndex);
    msg.set_logterm(hintTerm);
	send(std::move(msg));
}

inline void raft::handleHeartbeat(raftpb::Message m) {
    raftLog_->commitTo(m.commit());
    raftpb::Message msg;
    msg.set_to(m.from());
    msg.set_type(raftpb::MsgHeartbeatResp);
    msg.set_context(std::move(*m.mutable_context()));
    send(std::move(msg));
}

inline void raft::handleSnapshot(raftpb::Message m) {
    // MsgSnap messages should always carry a non-nil Snapshot, but err on the
    // side of safety and treat a nil Snapshot as a zero-valued Snapshot.
    // TODO(wangzhiyong)
    raftpb::Snapshot s;
    if (!IsEmptySnap(m.snapshot())) {
        s = m.snapshot();
    }
    auto sindex = s.metadata().index();
    auto sterm = s.metadata().term();

    if (restore(std::move(s))) {
        ERAFT_INFO("%x [commit: %d] restored snapshot [index: %d, term: %d]",
                 id_, raftLog_->committed_, sindex, sterm);
        raftpb::Message msg;
        msg.set_to(m.from());
        msg.set_type(raftpb::MsgAppResp);
        msg.set_index(raftLog_->lastIndex());
        send(std::move(msg));
    } else {
        ERAFT_INFO("%x [commit: %d] ignored snapshot [index: %d, term: %d]",
                 id_, raftLog_->committed_, sindex, sterm);
        raftpb::Message msg;
        msg.set_to(m.from());
        msg.set_type(raftpb::MsgAppResp);
        msg.set_index(raftLog_->committed_);
        send(std::move(msg));
    }
}

// restore recovers the state machine from a snapshot. It restores the log and the
// configuration of state machine. If this method returns false, the snapshot was
// ignored, either because it was obsolete or because of an error.
inline bool raft::restore(raftpb::Snapshot s) {
    if (s.metadata().index() <= raftLog_->committed_) {
        return false;
    }
    if (state_ != StateFollower) {
        // This is defense-in-depth: if the leader somehow ended up applying a
        // snapshot, it could move into a new term without moving into a
        // follower state. This should never fire, but if it did, we'd have
        // prevented damage by returning early, so log only a loud warning.
        //
        // At the time of writing, the instance is guaranteed to be in follower
        // state when this method is called.
        ERAFT_WARN("%x attempted to restore snapshot as leader; should never happen", id_);
        becomeFollower(term_ + 1, None);
        return false;
    }

    // More defense-in-depth: throw away snapshot if recipient is not in the
    // config. This shouldn't ever happen (at the time of writing) but lots of
    // code here and there assumes that r.id is in the progress tracker.
    auto found = false;
    auto cs = s.metadata().conf_state();

    for (auto& set: std::vector<std::vector<uint64_t>>{{cs.voters().begin(),          cs.voters().end()},
                                                       {cs.learners().begin(),        cs.learners().end()},
                                                       {cs.voters_outgoing().begin(), cs.voters_outgoing().end()}}) {
        // `LearnersNext` doesn't need to be checked. According to the rules, if a peer in
        // `LearnersNext`, it has to be in `VotersOutgoing`.
        for (auto id: set) {
            if (id == id_) {
                found = true;
                break;
            }
        }
        if (found) {
            break;
        }
    }
    if (!found) {
        ERAFT_WARN("%x attempted to restore snapshot but it is not in the ConfState [%s]; should never happen", id_,
                 cs.ShortDebugString().c_str());
        return false;
    }

    // Now go ahead and actually restore.
    auto snapshot_index = s.metadata().index();
    auto snapshot_term = s.metadata().term();
    if (raftLog_->matchTerm(snapshot_index, snapshot_term)) {
        ERAFT_INFO("%x [commit: %d, lastindex: %d, lastterm: %d] fast-forwarded commit to snapshot [index: %d, term: %d]",
                 id_, raftLog_->committed_, raftLog_->lastIndex(), raftLog_->lastTerm(), s.metadata().index(),
                 s.metadata().term());
        raftLog_->commitTo(s.metadata().index());
        return false;
    }

    raftLog_->restore(std::move(s));

    // Reset the configuration and add the (potentially updated) peers in anew.
    trk_ = tracker::MakeProgressTracker(trk_.maxInflight_, trk_.maxInflightBytes_);
    tracker::Config cfg;
    tracker::ProgressMap prs;
    Error err;
    std::tie(cfg, prs, err) = confchange::Restore({trk_, raftLog_->lastIndex()}, cs);
    if (err != nullptr) {
        // This should never happen. Either there's a bug in our config change
        // handling or the client corrupted the conf change.
        ERAFT_FATAL("unable to restore config [%s]: %s", cs.SerializeAsString().c_str(), err.String().c_str());
    }
    err = raftpb::Equivalent(cs, switchToConfig(cfg, prs));
    if (err != nullptr) {
        ERAFT_FATAL(err.String().c_str());
    }
    auto& pr = trk_.progress_[id_];
    pr->MaybeUpdate(pr->next_ - 1); // TODO(tbg): this is untested and likely unneeded

    ERAFT_INFO("%x [commit: %d, lastindex: %d, lastterm: %d] restored snapshot [index: %d, term: %d]",
             id_, raftLog_->committed_, raftLog_->lastIndex(), raftLog_->lastTerm(), snapshot_index, snapshot_term);
    return true;
}

// promotable indicates whether state machine can be promoted to leader,
// which is true when its own id is in progress list.
inline bool raft::promotable() {
    if (trk_.progress_.data().count(id_) == 0) {
        return false;
    }
    auto& pr = trk_.progress_[id_];
    return !pr->isLearner_ && !raftLog_->hasNextOrInProgressSnapshot();
}

inline raftpb::ConfState raft::applyConfChange(raftpb::ConfChangeV2 cc) {
    tracker::Config cfg;
    tracker::ProgressMap prs;
    Error err;
    raftpb::ConfChangeV2Wrap ccv2_wrap(std::move(cc));
    std::tie(cfg, prs, err) = [this, &ccv2_wrap]() {
        confchange::Changer changer{trk_, raftLog_->lastIndex()};
        if (ccv2_wrap.LeaveJoint()) {
            return changer.LeaveJoint();
        }
        bool autoLeave = false;
        bool ok = false;
        std::tie(autoLeave, ok) = ccv2_wrap.EnterJoint();
        if (ok) {
            return changer.EnterJoint(autoLeave, {ccv2_wrap.data().changes().begin(), ccv2_wrap.data().changes().end()});
        }
        return changer.Simple({ccv2_wrap.data().changes().begin(), ccv2_wrap.data().changes().end()});
    }();

    if (err != nullptr) {
        // TODO(tbg): return the error to the caller.
        ERAFT_FATAL(err.String().c_str());
    }

    return switchToConfig(cfg, prs);
}

// switchToConfig reconfigures this node to use the provided configuration. It
// updates the in-memory state and, when necessary, carries out additional
// actions such as reacting to the removal of nodes or changed quorum
// requirements.
//
// The inputs usually result from restoring a ConfState or applying a ConfChange.
inline raftpb::ConfState raft::switchToConfig(tracker::Config cfg, tracker::ProgressMap prs) {
    trk_.config_ = std::move(cfg);
    trk_.progress_ = std::move(prs);

    ERAFT_INFO("%x switched to configuration %s", id_, trk_.config_.String().c_str())
    auto cs = trk_.ConfState();
    auto iter = trk_.progress_.data().find(id_);
    bool ok = iter != trk_.progress_.data().end();
    // Update whether the node itself is a learner, resetting to false when the
    // node is removed.
    isLearner_ = ok && iter->second->isLearner_;

    if ((!ok || isLearner_) && state_ == StateLeader) {
        // This node is leader and was removed or demoted, step down if requested.
        //
        // We prevent demotions at the time writing but hypothetically we handle
        // them the same way as removing the leader.
        //
        // TODO(tbg): ask follower with largest Match to TimeoutNow (to avoid
        // interruption). This might still drop some proposals but it's better than
        // nothing.
        if (stepDownOnRemoval_) {
            becomeFollower(term_, None);
        }
        return cs;
    }

    // The remaining steps only make sense if this node is the leader and there
    // are other nodes.
    if (state_ != StateLeader || cs.voters().empty()) {
        return cs;
    }

    if (maybeCommit()) {
        // If the configuration change means that more entries are committed now,
        // broadcast/append to everyone in the updated config.
        bcastAppend();
    } else {
        // Otherwise, still probe the newly added replicas; there's no reason to
        // let them wait out a heartbeat interval (or the next incoming
        // proposal).
        trk_.Visit([this](uint64_t id, std::shared_ptr<tracker::Progress>&) {
            if (id == id_) {
                return;
            }
            maybeSendAppend(id, false /* sendIfEmpty */);
        });
    }
    // If the leadTransferee was removed or demoted, abort the leadership transfer.
    if (trk_.config_.voters_.IDs().find(leadTransferee_) == trk_.config_.voters_.IDs().end() && leadTransferee_ != 0) {
        abortLeaderTransfer();
    }

    return cs;
}

inline void raft::loadState(const raftpb::HardState& state) {
    if (state.commit() < raftLog_->committed_ || state.commit() > raftLog_->lastIndex()) {
        ERAFT_FATAL("%x state.commit %d is out of range [%d, %d]", id_, state.commit(), raftLog_->committed_,
                  raftLog_->lastIndex());
    }
    raftLog_->committed_ = state.commit();
    term_ = state.term();
    vote_ = state.vote();
}

// pastElectionTimeout returns true if r.electionElapsed is greater
// than or equal to the randomized election timeout in
// [electiontimeout, 2 * electiontimeout - 1].
inline bool raft::pastElectionTimeout() const {
    return electionElapsed_ >= randomizedElectionTimeout_;
}

inline void raft::resetRandomizedElectionTimeout() {
    randomizedElectionTimeout_ = electionTimeout_ + random(electionTimeout_);
}

inline void raft::sendTimeoutNow(uint64_t to) {
    raftpb::Message m;
    m.set_to(to);
    m.set_type(raftpb::MsgTimeoutNow);
    send(std::move(m));
}

inline void raft::abortLeaderTransfer() {
    leadTransferee_ = None;
}

// committedEntryInCurrentTerm return true if the peer has committed an entry in its term.
inline bool raft::committedEntryInCurrentTerm() {
    // NB: r.Term is never 0 on a leader, so if zeroTermOnOutOfBounds returns 0,
    // we won't see it as a match with r.Term.
    return raftLog_->zeroTermOnOutOfBounds(raftLog_->term(raftLog_->committed_)) == term_;
}

// responseToReadIndexReq constructs a response for `req`. If `req` comes from the peer
// itself, a blank value will be returned.
inline raftpb::Message raft::responseToReadIndexReq(raftpb::Message req, uint64_t readIndex) {
    if (req.from() == None || req.from() == id_) {
        readStates_.emplace_back(readIndex, req.entries(0).data());
        return {};
    }
    raftpb::Message ret;
    ret.set_type(raftpb::MsgReadIndexResp);
    ret.set_to(req.from());
    ret.set_index(readIndex);
    *ret.mutable_entries() = std::move(*req.mutable_entries());
    return ret;
}

// increaseUncommittedSize computes the size of the proposed entries and
// determines whether they would push leader over its maxUncommittedSize limit.
// If the new entries would exceed the limit, the method returns false. If not,
// the increase in uncommitted entry size is recorded and the method returns
// true.
//
// Empty payloads are never refused. This is used both for appending an empty
// entry at a new leader's term, as well as leaving a joint configuration.
inline bool raft::increaseUncommittedSize(const std::vector<raftpb::Entry>& ents) {
    auto s = payloadsSize(ents);
    if (uncommittedSize_ > 0 && s > 0 && uncommittedSize_ + s > maxUncommittedSize_) {
        // If the uncommitted tail of the Raft log is empty, allow any size
        // proposal. Otherwise, limit the size of the uncommitted tail of the
        // log and drop any proposal that would push the size over the limit.
        // Note the added requirement s>0 which is used to make sure that
        // appending single empty entries to the log always succeeds, used both
        // for replicating a new leader's initial empty entry, and for
        // auto-leaving joint configurations.
        return false;
    }
    uncommittedSize_ += s;
    return true;
}

// reduceUncommittedSize accounts for the newly committed entries by decreasing
// the uncommitted entry size limit.
inline void raft::reduceUncommittedSize(uint64_t s) {
    if (s > uncommittedSize_) {
        // uncommittedSize may underestimate the size of the uncommitted Raft
        // log tail but will never overestimate it. Saturate at 0 instead of
        // allowing overflow.
        uncommittedSize_ = 0;
    } else {
        uncommittedSize_ -= s;
    }
}

inline void releasePendingReadIndexMessages(raft& r) {
    if (r.pendingReadIndexMessages_.empty()) {
        // Fast path for the common case to avoid a call to storage.LastIndex()
        // via committedEntryInCurrentTerm.
        return;
    }
    if (!r.committedEntryInCurrentTerm()) {
        ERAFT_ERROR("pending MsgReadIndex should be released only after first commit in current term");
        return;
    }

    auto msgs = std::move(r.pendingReadIndexMessages_);
    r.pendingReadIndexMessages_.clear();

    for (auto& m: msgs) {
        sendMsgReadIndexResponse(r, std::move(m));
    }
}

inline void sendMsgReadIndexResponse(raft& r, raftpb::Message m) {
    // thinking: use an internally defined context instead of the user given context.
    // We can express this in terms of the term and index instead of a user-supplied value.
    // This would allow multiple reads to piggyback on the same message.
    switch (r.readOnly_->option_) {
        // If more than the local vote is needed, go through a full broadcast.
        case ReadOnlySafe: {
            auto ctx = m.entries(0).data();
            r.readOnly_->addRequest(r.raftLog_->committed_, std::move(m));
            // The local node automatically acks the request.
            r.readOnly_->recvAck(r.id_, ctx);
            r.bcastHeartbeatWithCtx(ctx);
            break;
        }
        case ReadOnlyLeaseBased: {
            auto resp = r.responseToReadIndexReq(std::move(m), r.raftLog_->committed_);
            if (resp.to() != None) {
                r.send(std::move(resp));
            }
            break;
        }
    }
}
}

};