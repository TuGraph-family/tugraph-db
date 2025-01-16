#pragma once
#include "state.h"
#include "inflights.h"

namespace tracker {
// Progress represents a follower’s progress in the view of the leader. Leader
// maintains progresses of all followers, and sends entries to the follower
// based on its progress.
//
// NB(tbg): Progress is basically a state machine whose transitions are mostly
// strewn around `*raft.raft`. Additionally, some fields are only used when in a
// certain State. All of this isn't ideal.
struct Progress {
	uint64_t match_ = 0;
    uint64_t next_ = 0;
	// State defines how the leader should interact with the follower.
	//
	// When in StateProbe, leader sends at most one replication message
	// per heartbeat interval. It also probes actual progress of the follower.
	//
	// When in StateReplicate, leader optimistically increases next
	// to the latest entry sent after sending replication message. This is
	// an optimized state for fast replicating log entries to the follower.
	//
	// When in StateSnapshot, leader should have sent out snapshot
	// before and stops sending any replication message.
    StateType state_ = StateType::StateProbe;

    // PendingSnapshot is used in StateSnapshot and tracks the last index of the
    // leader at the time at which it realized a snapshot was necessary. This
    // matches the index in the MsgSnap message emitted from raft.
    //
    // While there is a pending snapshot, replication to the follower is paused.
    // The follower will transition back to StateReplicate if the leader
    // receives an MsgAppResp from it that reconnects the follower to the
    // leader's log (such an MsgAppResp is emitted when the follower applies a
    // snapshot). It may be surprising that PendingSnapshot is not taken into
    // account here, but consider that complex systems may delegate the sending
    // of snapshots to alternative datasources (i.e. not the leader). In such
    // setups, it is difficult to manufacture a snapshot at a particular index
    // requested by raft and the actual index may be ahead or behind. This
    // should be okay, as long as the snapshot allows replication to resume.
    //
    // The follower will transition to StateProbe if ReportSnapshot is called on
    // the leader; if SnapshotFinish is passed then PendingSnapshot becomes the
    // basis for the next attempt to append. In practice, the first mechanism is
    // the one that is relevant in most cases. However, if this MsgAppResp is
    // lost (fallible network) then the second mechanism ensures that in this
    // case the follower does not erroneously remain in StateSnapshot.
    uint64_t pendingSnapshot_ = 0;

	// RecentActive is true if the progress is recently active. Receiving any messages
	// from the corresponding follower indicates the progress is active.
	// RecentActive can be reset to false after an election timeout.
    // This is always true on the leader.
    bool recentActive_ = false;

    // MsgAppFlowPaused is used when the MsgApp flow to a node is throttled. This
    // happens in StateProbe, or StateReplicate with saturated Inflights. In both
    // cases, we need to continue sending MsgApp once in a while to guarantee
    // progress, but we only do so when MsgAppFlowPaused is false (it is reset on
    // receiving a heartbeat response), to not overflow the receiver. See
    // IsPaused().
    bool msgAppFlowPaused_ = false;

	// Inflights is a sliding window for the inflight messages.
	// Each inflight message contains one or more log entries.
	// The max number of entries per message is defined in raft config as MaxSizePerMsg.
	// Thus inflight effectively limits both the number of inflight messages
	// and the bandwidth each Progress can use.
	// When inflights is Full, no more message should be sent.
	// When a leader sends out a message, the index of the last
	// entry should be added to inflights. The index MUST be added
	// into inflights in order.
	// When a leader receives a reply, the previous inflights should
	// be freed by calling inflights.FreeLE with the index of the last
	// received entry.
	std::shared_ptr<Inflights> inflights_;

	// IsLearner is true if this progress is tracked for a learner.
	bool isLearner_ = false;

    bool operator==(const Progress& p) const {
        if (match_ != p.match_ ||
            next_ != p.next_ ||
            state_ != p.state_ ||
            pendingSnapshot_ != p.pendingSnapshot_ ||
            recentActive_ != p.recentActive_ ||
            msgAppFlowPaused_ != p.msgAppFlowPaused_ || isLearner_ != p.isLearner_) {
            return false;
        }
        if ((!inflights_ && p.inflights_) || (inflights_ && !p.inflights_)) {
            return false;
        }
        if (inflights_ && p.inflights_) {
            if (!(*inflights_ == *p.inflights_)) {
                return false;
            }
        }
        return true;
    }

    // ResetState moves the Progress into the specified State, resetting MsgAppFlowPaused,
    // PendingSnapshot, and Inflights.
    void ResetState(StateType s) {
        msgAppFlowPaused_ = false;
        pendingSnapshot_ = 0;
        state_ = s;
        inflights_->reset();
    }

    // BecomeProbe transitions into StateProbe. Next is reset to Match+1 or,
    // optionally and if larger, the index of the pending snapshot.
    void BecomeProbe() {
        // If the original state is StateSnapshot, progress knows that
        // the pending snapshot has been sent to this peer successfully, then
        // probes from pendingSnapshot + 1.
        if (state_ == StateType::StateSnapshot) {
            auto pendingSnapshot = pendingSnapshot_;
            ResetState(StateType::StateProbe);
            next_ = std::max(match_ + 1, pendingSnapshot + 1);
        } else {
            ResetState(StateType::StateProbe);
            next_ = match_ + 1;
        }
    }

    // BecomeReplicate transitions into StateReplicate, resetting Next to Match+1.
    void BecomeReplicate() {
        ResetState(StateType::StateReplicate);
        next_ = match_ + 1;
    }

    // BecomeSnapshot moves the Progress to StateSnapshot with the specified pending
    // snapshot index.
    void BecomeSnapshot(uint64_t snapshoti) {
        ResetState(StateType::StateSnapshot);
        pendingSnapshot_ = snapshoti;
    }

    // UpdateOnEntriesSend updates the progress on the given number of consecutive
    // entries being sent in a MsgApp, with the given total bytes size, appended at
    // and after the given log index.
    Error UpdateOnEntriesSend(int entries, uint64_t bytes, uint64_t nextIndex) {
        switch (state_) {
            case StateType::StateReplicate: {
                if (entries > 0) {
                    auto last = nextIndex + entries - 1;
                    OptimisticUpdate(last);
                    inflights_->Add(last, bytes);
                }
                // If this message overflows the in-flights tracker, or it was already full,
                // consider this message being a probe, so that the flow is paused.
                msgAppFlowPaused_ = inflights_->Full();
                break;
            }
            case StateType::StateProbe: {
                // TODO(pavelkalinnikov): this condition captures the previous behaviour,
                // but we should set MsgAppFlowPaused unconditionally for simplicity, because any
                // MsgApp in StateProbe is a probe, not only non-empty ones.
                if (entries > 0) {
                    msgAppFlowPaused_ = true;
                }
                break;
            }
        default:
            return Error(format("sending append in unhandled state %s", ToString(state_).c_str()));
        }
        return {};
    }

    // MaybeUpdate is called when an MsgAppResp arrives from the follower, with the
    // index acked by it. The method returns false if the given n index comes from
    // an outdated message. Otherwise it updates the progress and returns true.
    bool MaybeUpdate(uint64_t n) {
        bool updated = false;
        if (match_ < n) {
            match_ = n;
            updated = true;
            msgAppFlowPaused_ = false;
        }
        next_ = std::max(next_, n + 1);
        return updated;
    }

    // OptimisticUpdate signals that appends all the way up to and including index n
    // are in-flight. As a result, Next is increased to n+1.
    void OptimisticUpdate(uint64_t n) { next_ = n + 1; }

    // MaybeDecrTo adjusts the Progress to the receipt of a MsgApp rejection. The
    // arguments are the index of the append message rejected by the follower, and
    // the hint that we want to decrease to.
    //
    // Rejections can happen spuriously as messages are sent out of order or
    // duplicated. In such cases, the rejection pertains to an index that the
    // Progress already knows were previously acknowledged, and false is returned
    // without changing the Progress.
    //
    // If the rejection is genuine, Next is lowered sensibly, and the Progress is
    // cleared for sending log entries.
    bool MaybeDecrTo(uint64_t rejected, uint64_t matchHint) {
        if (state_ == StateType::StateReplicate) {
            // The rejection must be stale if the progress has matched and "rejected"
            // is smaller than "match".
            if (rejected <= match_) {
                return false;
            }
            // Directly decrease next to match + 1.
            //
            // TODO(tbg): why not use matchHint if it's larger?
            next_ = match_ + 1;
            return true;
        }

        // The rejection must be stale if "rejected" does not match next - 1. This
        // is because non-replicating followers are probed one entry at a time.
        if (next_ - 1 != rejected) {
            return false;
        }

        next_ = std::max(std::min(rejected, matchHint + 1), (size_t)1);
        msgAppFlowPaused_ = false;
        return true;
    }

    // IsPaused returns whether sending log entries to this node has been throttled.
    // This is done when a node has rejected recent MsgApps, is currently waiting
    // for a snapshot, or has reached the MaxInflightMsgs limit. In normal
    // operation, this is false. A throttled node will be contacted less frequently
    // until it has reached a state in which it's able to accept a steady stream of
    // log entries again.
    bool IsPaused() {
        switch (state_) {
            case StateType::StateProbe:
                return msgAppFlowPaused_;
            case StateType::StateReplicate:
                return msgAppFlowPaused_;
            case StateType::StateSnapshot:
                return true;
            default: {
                ERAFT_FATAL("unexpected state");
                return false;
            }
        }
    }

    std::string String() {
        std::string buf;
        buf.append(format("%s match=%d next=%d", ToString(state_).c_str(), match_, next_));
        if (isLearner_) {
            buf.append(" learner");
        }
        if (IsPaused()) {
            buf.append(" paused");
        }
        if (pendingSnapshot_ > 0) {
            buf.append(format(" pendingSnap=%d", pendingSnapshot_));
        }
        if (!recentActive_) {
            buf.append(" inactive");
        }
        auto n = inflights_->Count();
        if (n > 0) {
            buf.append(format(" inflight=%d", n));
            if (inflights_->Full()) {
                buf.append("[full]");
            }
        }
        return buf;
    }
};

// ProgressMap is a map of *Progress.
struct ProgressMap {
    std::unordered_map<uint64_t, std::shared_ptr<Progress>> data_;
    const std::unordered_map<uint64_t, std::shared_ptr<Progress>>&
    data() const {return data_;}
    std::unordered_map<uint64_t, std::shared_ptr<Progress>>&
    data() {return data_;}
    // TODO(wangzhiyong): return nullptr if not exist ?
    std::shared_ptr<Progress>& operator[](size_t index) {return data_.at(index);};
    const std::shared_ptr<Progress>& operator[](size_t index) const {return data_.at(index);};
    // String prints the ProgressMap in sorted key order, one Progress per line.
    std::string String() {
        std::vector<uint64_t> ids;
        for (const auto& item : data_) {
            ids.push_back(item.first);
        }
        std::sort(ids.begin(), ids.end(), std::less<uint64_t>());
        std::string buf;
        for (auto id : ids) {
            buf.append(format("%d: %s\n", id, data_.at(id)->String().c_str()));
        }
        return buf;
    }
};

}
