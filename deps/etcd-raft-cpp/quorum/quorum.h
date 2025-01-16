#pragma once

#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>

namespace quorum {
using namespace eraft;
// Index is a Raft log position.
struct Index {
    uint64_t data_ = 0;
    Index() = default;
    explicit Index(uint64_t d) : data_(d) {}
    bool operator== (const Index& r) const {return data_ == r.data_;}
    bool operator< (const Index& r) const {return data_ < r.data_;};
    uint64_t data() const {return data_;}
    std::string String() const {
        if (data_ == std::numeric_limits<uint64_t>::max()) {
            return "∞";
        }
        return std::to_string(data_);
    }
};

// AckedIndexer allows looking up a commit index for a given ID of a voter
// from a corresponding MajorityConfig.
struct AckedIndexer {
	virtual std::pair<Index, bool> AckedIndex(uint64_t voterID) const = 0;
};

namespace detail {
struct mapAckIndexer : public AckedIndexer {
    std::unordered_map<uint64_t, Index> data_;

    std::pair<Index, bool> AckedIndex(uint64_t voterID) const override {
        auto iter = data_.find(voterID);
        if (iter == data_.end()) {
            return {Index(0), false};
        } else {
            return {iter->second, true};
        }
    }
};
}

enum class VoteResult {
    // VotePending indicates that the decision of the vote depends on future
    // votes, i.e. neither "yes" or "no" has reached quorum yet.
    VotePending = 1,
    // VoteLost indicates that the quorum has voted "no".
    VoteLost,
    // VoteWon indicates that the quorum has voted "yes".
    VoteWon
};

}