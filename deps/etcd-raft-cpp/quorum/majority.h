#pragma once
#include <cstdint>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include "quorum.h"
#include "../util.h"

namespace quorum {
using namespace eraft;
// MajorityConfig is a set of IDs that uses majority quorums to make decisions.
struct MajorityConfig {
    std::unordered_set<uint64_t> data_;
    MajorityConfig() = default;
    bool operator==(const MajorityConfig& m) const {
        return data_ == m.data_;
    }
    explicit MajorityConfig(std::unordered_set<uint64_t> d) : data_(std::move(d)) {};
    const std::unordered_set<uint64_t>& data() const {return data_;}
    std::unordered_set<uint64_t>& data() {return data_;}
    std::string String() {
        std::vector<uint64_t> sl;
        sl = {data_.begin(), data_.end()};
        std::sort(sl.begin(), sl.end(), std::less<uint64_t>());
        std::string buf;
        buf.push_back('(');
        for (size_t i = 0; i < sl.size(); i++) {
            if (i > 0) {
                buf.push_back(' ');
            }
            buf.append(std::to_string(sl[i]));
        }
        buf.push_back(')');
        return buf;
    }
    // Describe returns a (multi-line) representation of the commit indexes for the
    // given lookuper.
    std::string Describe(const AckedIndexer& l) {
        if (data_.empty()) {
            return "<empty majority quorum>";
        }
        struct tup {
            uint64_t id = 0;
            Index idx;
            bool ok = false; // idx found?
            size_t bar = 0;  // length of bar displayed for this tup
            tup(uint64_t i, Index index, bool o) : id(i), idx(index), ok(o) {}
        };

        // Below, populate .bar so that the i-th largest commit index has bar i (we
        // plot this as sort of a progress bar). The actual code is a bit more
        // complicated and also makes sure that equal index => equal bar.

        auto n = data_.size();
        std::vector<tup> info;
        for (auto id : data_) {
            auto ret = l.AckedIndex(id);
            info.emplace_back(id, ret.first, ret.second);
        }

        // Sort by index
        std::sort(info.begin(), info.end(), [](const tup& i, const tup& j) {
            if (i.idx == j.idx) {
                return i.id < j.id;
            }
            return i.idx < j.idx;
        });

        // Populate .bar.
        for (size_t i = 0; i < info.size(); i++) {
            if (i > 0 && info[i-1].idx < info[i].idx) {
                info[i].bar = i;
            }
        }

        // Sort by ID.
        std::sort(info.begin(), info.end(), [](const tup& i, const tup& j) {
            return i.id < j.id;
        });

        std::string buf;

        // Print.
        buf.append(n, ' ').append("    idx\n");
        for (const auto& t : info) {
            auto bar = t.bar;
            if (!t.ok) {
                buf.append("?").append(n, ' ');
            } else {
                buf.append(bar, 'x').append(1, '>').append(n-bar, ' ');
            }
            buf.append(format(" %5d    (id=%d)\n", t.idx, t.id));
        }
        return buf;
    }
    // Slice returns the MajorityConfig as a sorted slice.
    std::vector<uint64_t> Slice() {
        std::vector<uint64_t> sl;
        sl = {data_.begin(), data_.end()};
        std::sort(sl.begin(), sl.end(), std::less<uint64_t>());
        return sl;
    }
    // CommittedIndex computes the committed index from those supplied via the
    // provided AckedIndexer (for the active config).
    Index CommittedIndex(const AckedIndexer& l) {
        auto n = data_.size();
        if (n == 0) {
            // This plays well with joint quorums which, when one half is the zero
            // MajorityConfig, should behave like the other half.
            return Index(std::numeric_limits<uint64_t>::max());
        }

        // Use an on-stack slice to collect the committed indexes when n <= 7
        // (otherwise we alloc). The alternative is to stash a slice on
        // MajorityConfig, but this impairs usability (as is, MajorityConfig is just
        // a map, and that's nice). The assumption is that running with a
        // replication factor of >7 is rare, and in cases in which it happens
        // performance is a lesser concern (additionally the performance
        // implications of an allocation here are far from drastic).
        // TODO(wangzhiyong)
        std::vector<uint64_t> srt(n);
        {
            // Fill the slice with the indexes observed. Any unused slots will be
            // left as zero; these correspond to voters that may report in, but
            // haven't yet. We fill from the right (since the zeroes will end up on
            // the left after sorting below anyway).
            auto i = n - 1;
            for (auto id : data_) {
                Index idx;
                bool ok;
                std::tie(idx, ok) = l.AckedIndex(id);
                if (ok) {
                    srt[i] = idx.data();
                    i--;
                }
            }
        }

        // Sort by index. Use a bespoke algorithm (copied from the stdlib's sort
        // package) to keep srt on the stack.
        // TODO wangzhiyong
        std::sort(srt.begin(), srt.end(), std::less<uint64_t>());

        // The smallest index into the array for which the value is acked by a
        // quorum. In other words, from the end of the slice, move n/2+1 to the
        // left (accounting for zero-indexing).
        auto pos = n - (n/2 + 1);
        return Index(srt[pos]);
    }
    // VoteResult takes a mapping of voters to yes/no (true/false) votes and returns
    // a result indicating whether the vote is pending (i.e. neither a quorum of
    // yes/no has been reached), won (a quorum of yes has been reached), or lost (a
    // quorum of no has been reached).
    VoteResult GetVoteResult(std::unordered_map<uint64_t, bool> votes) {
        if (data_.empty()) {
            // By convention, the elections on an empty config win. This comes in
            // handy with joint quorums because it'll make a half-populated joint
            // quorum behave like a majority quorum.
            return VoteResult::VoteWon;
        }

        size_t votedCnt = 0; //vote counts for yes.
        size_t missing = 0;
        for (auto id : data_) {
            if (!votes.count(id)) {
                missing++;
                continue;
            } else {
                if (votes[id]) {
                    votedCnt++;
                }
            }
        }

        auto q = data_.size()/2 + 1;
        if (votedCnt >= q) {
            return VoteResult::VoteWon;
        }
        if (votedCnt+missing >= q) {
            return VoteResult::VotePending;
        }
        return VoteResult::VoteLost;
    }
};
}