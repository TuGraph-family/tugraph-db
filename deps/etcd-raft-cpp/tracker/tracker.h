#pragma once
#include "../util.h"
#include "progress.h"
#include "../quorum/joint.h"
#include "../raftpb/raft.pb.h"

namespace tracker {
// Config reflects the configuration tracked in a ProgressTracker.
struct Config {
    quorum::JointConfig voters_;
	// AutoLeave is true if the configuration is joint and a transition to the
	// incoming configuration should be carried out automatically by Raft when
	// this is possible. If false, the configuration will be joint until the
	// application initiates the transition manually.
    bool autoLeave_ = false;
	// Learners is a set of IDs corresponding to the learners active in the
	// current configuration.
	//
	// Invariant: Learners and Voters does not intersect, i.e. if a peer is in
	// either half of the joint config, it can't be a learner; if it is a
	// learner it can't be in either half of the joint config. This invariant
	// simplifies the implementation since it allows peers to have clarity about
	// its current role without taking into account joint consensus.
	std::unordered_set<uint64_t> learners_;
	// When we turn a voter into a learner during a joint consensus transition,
	// we cannot add the learner directly when entering the joint state. This is
	// because this would violate the invariant that the intersection of
	// voters and learners is empty. For example, assume a Voter is removed and
	// immediately re-added as a learner (or in other words, it is demoted):
	//
	// Initially, the configuration will be
	//
	//   voters:   {1 2 3}
	//   learners: {}
	//
	// and we want to demote 3. Entering the joint configuration, we naively get
	//
	//   voters:   {1 2} & {1 2 3}
	//   learners: {3}
	//
	// but this violates the invariant (3 is both voter and learner). Instead,
	// we get
	//
	//   voters:   {1 2} & {1 2 3}
	//   learners: {}
	//   next_learners: {3}
	//
	// Where 3 is now still purely a voter, but we are remembering the intention
	// to make it a learner upon transitioning into the final configuration:
	//
	//   voters:   {1 2}
	//   learners: {3}
	//   next_learners: {}
	//
	// Note that next_learners is not used while adding a learner that is not
	// also a voter in the joint config. In this case, the learner is added
	// right away when entering the joint configuration, so that it is caught up
	// as soon as possible.
	std::unordered_set<uint64_t> learnersNext_;

    bool operator==(const Config& c) const {
        return voters_ == c.voters_ &&
               autoLeave_ == c.autoLeave_ &&
               learners_ == c.learners_ &&
               learnersNext_ == c.learnersNext_;
    }

    std::string String() {
        std::string buf;
        buf.append(format("voters=%s", voters_.String().c_str()));
        if (!learners_.empty()) {
            buf.append(format(" learners=%s", quorum::MajorityConfig(learners_).String().c_str()));
        }
        if (!learnersNext_.empty()) {
            buf.append(format(" learners_next=%s", quorum::MajorityConfig(learnersNext_).String().c_str()));
        }
        if (autoLeave_) {
            buf.append(" autoleave");
        }
        return buf;
    }

    // Clone returns a copy of the Config that shares no memory with the original.
    Config Clone() {
        return *this;
    }
};

namespace detail {
struct matchAckIndexer : public quorum::AckedIndexer {
    std::unordered_map<uint64_t, std::shared_ptr<Progress>> data_;

    explicit matchAckIndexer(decltype(data_) d) : data_(std::move(d)) {};

    std::pair<quorum::Index, bool> AckedIndex(uint64_t id) const override {
        auto iter = data_.find(id);
        if (iter == data_.end()) {
            return {quorum::Index(0), false};
        }
        return {quorum::Index(iter->second->match_), true};
    }
};
}

// ProgressTracker tracks the currently active configuration and the information
// known about the nodes and learners in it. In particular, it tracks the match
// index for each peer which in turn allows reasoning about the committed index.
struct ProgressTracker {
	Config config_;

    ProgressMap progress_;

	std::unordered_map<uint64_t, bool> votes_;

	size_t maxInflight_ = 0;

    uint64_t maxInflightBytes_ = 0;

    // ConfState returns a ConfState representing the active configuration.
    raftpb::ConfState ConfState() {
        raftpb::ConfState cs;
        auto tmp = config_.voters_[0].Slice();
        *cs.mutable_voters() = {tmp.begin(), tmp.end()};
        tmp = config_.voters_[1].Slice();
        *cs.mutable_voters_outgoing() = {tmp.begin(), tmp.end()};
        tmp = quorum::MajorityConfig(config_.learners_).Slice();
        *cs.mutable_learners() = {tmp.begin(), tmp.end()};
        tmp = quorum::MajorityConfig(config_.learnersNext_).Slice();
        *cs.mutable_learners_next() = {tmp.begin(), tmp.end()};
        cs.set_auto_leave(config_.autoLeave_);
        return cs;
    }

    // IsSingleton returns true if (and only if) there is only one voting member
    // (i.e. the leader) in the current configuration.
    bool IsSingleton() {
        return config_.voters_[0].data().size() == 1 && config_.voters_[1].data().size() == 0;
    }

    // Committed returns the largest log index known to be committed based on what
    // the voting members of the group have acknowledged.
    uint64_t Committed() {
        return config_.voters_.CommittedIndex(detail::matchAckIndexer(progress_.data())).data();
    }

    // Visit invokes the supplied closure for all tracked progresses in stable order.
    void Visit(const std::function<void(uint64_t id, std::shared_ptr<Progress>&)>& f) {
        auto n = progress_.data().size();
        // TODO(wangzhiyong)
        std::vector<uint64_t> ids(n);
        for (auto& item : progress_.data()) {
            n--;
            ids[n] = item.first;
        }
        std::sort(ids.begin(), ids.end(), std::less<uint64_t>());
        for(auto id : ids) {
            f(id, progress_[id]);
        }
    }

    // QuorumActive returns true if the quorum is active from the view of the local
    // raft state machine. Otherwise, it returns false.
    bool QuorumActive() {
        std::unordered_map<uint64_t, bool> votes;
        Visit([this, &votes](uint64_t id, std::shared_ptr<Progress> pr) {
            if (pr->isLearner_) {
                return;
            }
            votes[id] = pr->recentActive_;
        });

        return config_.voters_.GetVoteResult(votes) == quorum::VoteResult::VoteWon;
    }

    // VoterNodes returns a sorted slice of voters.
    std::vector<uint64_t> VoterNodes() const {
        auto m = config_.voters_.IDs();
        std::vector<uint64_t> nodes{m.begin(), m.end()};
        std::sort(nodes.begin(), nodes.end(), std::less<uint64_t>());
        return nodes;
    }

    // LearnerNodes returns a sorted slice of learners.
    std::vector<uint64_t> LearnerNodes() {
        if (config_.learners_.empty()) {
            return {};
        }
        std::vector<uint64_t> nodes {config_.learners_.begin(), config_.learners_.end()};
        std::sort(nodes.begin(), nodes.end(), std::less<uint64_t>());
        return nodes;
    }

    // ResetVotes prepares for a new round of vote counting via recordVote.
    void ResetVotes() {
        votes_.clear();
    }

    // RecordVote records that the node with the given id voted for this Raft
    // instance if v == true (and declined it otherwise).
    void RecordVote(uint64_t id, bool v) {
        if (!votes_.count(id)) {
            votes_[id] = v;
        }
    }

    // TallyVotes returns the number of granted and rejected Votes, and whether the
    // election outcome is known.
    std::tuple<size_t, size_t, quorum::VoteResult> TallyVotes() {
        // Make sure to populate granted/rejected correctly even if the Votes slice
        // contains members no longer part of the configuration. This doesn't really
        // matter in the way the numbers are used (they're informational), but might
        // as well get it right.
        size_t granted = 0, rejected = 0;
        for (auto& item : progress_.data()) {
            auto id = item.first;
            auto& pr = item.second;
            if (pr->isLearner_) {
                continue;
            }

            if (!votes_.count(id)) {
                continue;
            }
            if (votes_.at(id)) {
                granted++;
            } else {
                rejected++;
            }
        }
        return std::make_tuple(granted, rejected, config_.voters_.GetVoteResult(votes_));
    }

};

// MakeProgressTracker initializes a ProgressTracker.
inline ProgressTracker MakeProgressTracker(size_t maxInflight, uint64_t maxBytes) {
    ProgressTracker p;
    p.maxInflight_ = maxInflight;
    p.maxInflightBytes_ = maxBytes;
    return p;
}

}