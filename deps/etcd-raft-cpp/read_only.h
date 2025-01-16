#pragma once
#include "util.h"
namespace eraft {

// ReadState provides state for read only query.
// It's caller's responsibility to call ReadIndex first before getting
// this state from ready, it's also caller's duty to differentiate if this
// state is what it requests through RequestCtx, eg. given a unique id as
// RequestCtx
struct ReadState {
    uint64_t index_ = 0;
	std::string requestCtx_;
    ReadState(uint64_t i, std::string ctx) : index_(i), requestCtx_(std::move(ctx)) {}
    bool operator==(const ReadState& rs) const {
        return index_ == rs.index_ && requestCtx_ == rs.requestCtx_;
    }
    bool operator!=(const ReadState& rs) const {
        return index_ != rs.index_ || requestCtx_ != rs.requestCtx_;
    }
};

enum ReadOnlyOption {
    // ReadOnlySafe guarantees the linearizability of the read only request by
    // communicating with the quorum. It is the default and suggested option.
    ReadOnlySafe = 0,
    // ReadOnlyLeaseBased ensures linearizability of the read only request by
    // relying on the leader lease. It can be affected by clock drift.
    // If the clock drift is unbounded, leader might keep the lease longer than it
    // should (clock can move backward/pause without any bound). ReadIndex is not safe
    // in that case.
    ReadOnlyLeaseBased
};

namespace detail {
struct readIndexStatus {
    raftpb::Message req_;
    uint64_t index_ = 0;
    // NB: this never records 'false', but it's more convenient to use this
    // instead of a map[uint64]struct{} due to the API of quorum.VoteResult. If
    // this becomes performance sensitive enough (doubtful), quorum.VoteResult
    // can change to an API that is closer to that of CommittedIndex.
    std::unordered_map<uint64_t, bool> acks_;
};
}

struct ReadOnly {
    ReadOnlyOption option_ = ReadOnlySafe;
    std::unordered_map<std::string, std::shared_ptr<detail::readIndexStatus>> pendingReadIndex_;
    std::vector<std::string> readIndexQueue_;

    // addRequest adds a read only request into readonly struct.
    // `index` is the commit index of the raft state machine when it received
    // the read only request.
    // `m` is the original read only request message from the local or remote node.
    void addRequest(uint64_t index, raftpb::Message m) {
        const auto& s = m.entries(0).data();
        if (pendingReadIndex_.count(s)) {
            return;
        }
        readIndexQueue_.push_back(s);
        auto ris = std::make_shared<detail::readIndexStatus>();
        ris->index_ = index;
        ris->req_ = std::move(m);
        pendingReadIndex_[s] = ris;
    }

    // recvAck notifies the readonly struct that the raft state machine received
    // an acknowledgment of the heartbeat that attached with the read only request
    // context.
    std::unordered_map<uint64_t, bool> recvAck(uint64_t id, const std::string& context) {
        auto iter = pendingReadIndex_.find(context);
        if (iter == pendingReadIndex_.end()) {
            return {};
        }
        iter->second->acks_[id] = true;
        return iter->second->acks_;
    }

    // advance advances the read only request queue kept by the readonly struct.
    // It dequeues the requests until it finds the read only request that has
    // the same context as the given `m`.
    std::vector<std::shared_ptr<detail::readIndexStatus>> advance(const raftpb::Message& m) {
        int64_t i = 0;
        bool found = false;

        const auto& ctx = m.context();
        std::vector<std::shared_ptr<detail::readIndexStatus>> rss;

        for (auto& okctx : readIndexQueue_) {
            i++;
            auto iter = pendingReadIndex_.find(okctx);
            if (iter == pendingReadIndex_.end()) {
                ERAFT_FATAL("cannot find corresponding read state from pending map");
            }
            rss.push_back(iter->second);
            if (okctx == ctx) {
                found = true;
                break;
            }
        }

        if (found) {
            readIndexQueue_.erase(readIndexQueue_.begin(), readIndexQueue_.begin() + i);
            for (auto& rs : rss) {
                pendingReadIndex_.erase(rs->req_.entries(0).data());
            }
            return rss;
        }

        return {};
    }

    // lastPendingRequestCtx returns the context of the last pending read only
    // request in readonly struct.
    const std::string& lastPendingRequestCtx() {
        if (readIndexQueue_.empty()) {
            static std::string emptyContext;
            return emptyContext;
        }
        return readIndexQueue_[readIndexQueue_.size() - 1];
    }
};

namespace detail {
inline std::shared_ptr<ReadOnly> newReadOnly(ReadOnlyOption option) {
    auto ret = std::make_shared<ReadOnly>();
    ret->option_ = option;
    return ret;
}
}

}