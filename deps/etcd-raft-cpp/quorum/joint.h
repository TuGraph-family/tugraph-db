#pragma once
#include "majority.h"

namespace quorum {
using namespace eraft;
// JointConfig is a configuration of two groups of (possibly overlapping)
// majority configurations. Decisions require the support of both majorities.
struct JointConfig {
    std::array<MajorityConfig, 2> data_;
    JointConfig() = default;
    bool operator==(const JointConfig& j) const {
        return data_ == j.data_;
    }
    // TODO(wangzhiyong): return nullptr if not exist
    MajorityConfig& operator[](size_t index) {
        return data_.at(index);
    }
    const MajorityConfig& operator[](size_t index) const{
        return data_.at(index);
    }
    std::string String() {
        if (!data_[1].data().empty()) {
            return data_[0].String() + "&&" + data_[1].String();
        }
        return data_[0].String();
    }
    // IDs returns a newly initialized map representing the set of voters present
    // in the joint configuration.
    std::unordered_set<uint64_t> IDs() const {
        std::unordered_set<uint64_t> m;
        for (const auto& cc : data_) {
            for (auto id : cc.data()) {
                m.emplace(id);
            }
        }
        return m;
    }

    // Describe returns a (multi-line) representation of the commit indexes for the
    // given lookuper.
    std::string Describe(const AckedIndexer& l) {
        return MajorityConfig(IDs()).Describe(l);
    }

    // CommittedIndex returns the largest committed index for the given joint
    // quorum. An index is jointly committed if it is committed in both constituent
    // majorities.
    Index CommittedIndex(const AckedIndexer& l) {
        auto idx0 = data_[0].CommittedIndex(l);
        auto idx1 = data_[1].CommittedIndex(l);
        if (idx0 < idx1) {
            return idx0;
        }
        return idx1;
    }

    // GetVoteResult takes a mapping of voters to yes/no (true/false) votes and returns
    // a result indicating whether the vote is pending, lost, or won. A joint quorum
    // requires both majority quorums to vote in favor.
    VoteResult GetVoteResult(const std::unordered_map<uint64_t, bool>& votes) {
        auto r1 = data_[0].GetVoteResult(votes);
        auto r2 = data_[1].GetVoteResult(votes);

        if (r1 == r2) {
            // If they agree, return the agreed state.
            return r1;
        }
        if (r1 == VoteResult::VoteLost || r2 == VoteResult::VoteLost) {
            // If either config has lost, loss is the only possible outcome.
            return VoteResult::VoteLost;
        }
        // One side won, the other one is pending, so the whole outcome is.
        return VoteResult::VotePending;
    }

};

}