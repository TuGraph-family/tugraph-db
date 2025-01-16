#pragma once

#include "../util.h"

namespace tracker {
using namespace eraft;
// Inflights limits the number of MsgApp (represented by the largest index
// contained within) sent to followers but not yet acknowledged by them. Callers
// use Full() to check whether more messages can be sent, call Add() whenever
// they are sending a new append, and release "quota" via FreeLE() whenever an
// ack is received.

namespace detail {
// inflight describes an in-flight MsgApp message.
struct inflight {
    uint64_t index_ = 0; // the index of the last entry inside the message
    uint64_t bytes_ = 0; // the total byte size of the entries in the message
    inflight() = default;

    inflight(uint64_t index, uint64_t bytes) : index_(index), bytes_(bytes) {}

    bool operator==(const inflight& i) const {
        return index_ == i.index_ && bytes_ == i.bytes_;
    }
};
}

struct Inflights {
    // the starting index in the buffer
    size_t start_ = 0;

    size_t count_ = 0;   // number of inflight messages in the buffer
    uint64_t bytes_ = 0; // number of inflight bytes

    size_t size_ = 0;       // the max number of inflight messages
    uint64_t maxBytes_ = 0; // the max total byte size of inflight messages

    // buffer is a ring buffer containing info about all in-flight messages.
    std::vector<detail::inflight> buffer_;

    // Clone returns an *Inflights that is identical to but shares no memory with
    // the receiver.
    std::shared_ptr<Inflights> Clone() {
        return std::make_shared<Inflights>(*this);
    }

    // reset frees all inflights.
    void reset() {
        count_ = 0;
        start_ = 0;
        bytes_ = 0;
    }

    // Full returns true if no more messages can be sent at the moment.
    bool Full() const {
        return count_ == size_ || (maxBytes_ != 0 && bytes_ >= maxBytes_);
    }

    // Count returns the number of inflight messages.
    size_t Count() const { return count_; }

    // grow the inflight buffer by doubling up to inflights.size. We grow on demand
    // instead of preallocating to inflights.size to handle systems which have
    // thousands of Raft groups per process.
    void grow() {
        auto newSize = buffer_.size() * 2;
        if (newSize == 0) {
            newSize = 1;
        } else if (newSize > size_) {
            newSize = size_;
        }
        buffer_.resize(newSize);
    }

    // Add notifies the Inflights that a new message with the given index and byte
    // size is being dispatched. Full() must be called prior to Add() to verify that
    // there is room for one more message, and consecutive calls to Add() must
    // provide a monotonic sequence of indexes.
    void Add(uint64_t index, uint64_t bytes) {
        if (Full()) {
            ERAFT_FATAL("cannot add into a Full inflights");
        }
        auto next = start_ + count_;
        if (next >= size_) {
            next -= size_;
        }
        if (next >= buffer_.size()) {
            grow();
        }
        buffer_[next] = {index,bytes};
        count_++;
        bytes_ += bytes;
    }

    // FreeLE frees the inflights smaller or equal to the given `to` flight.
    void FreeLE(uint64_t to) {
        if (count_ == 0 || to < buffer_[start_].index_) {
            // out of the left side of the window
            return;
        }

        auto idx = start_;
        size_t i = 0;
        uint64_t bytes = 0;
        for (i = 0; i < count_; i++) {
            if (to < buffer_[idx].index_) { // found the first large inflight
                break;
            }
            bytes += buffer_[idx].bytes_;

            // increase index and maybe rotate
            if (++idx >= size_) {
                idx -= size_;
            }
        }
        // free i inflights and set new start index
        count_ -= i;
        bytes_ -= bytes;
        start_ = idx;
        if (count_ == 0) {
            // inflights is empty, reset the start index so that we don't grow the
            // buffer unnecessarily.
            start_ = 0;
        }
    }

    bool operator==(const Inflights& right) const {
        return start_ == right.start_ &&
               count_ == right.count_ &&
               bytes_ == right.bytes_ &&
               size_ == right.size_ &&
               maxBytes_ == right.maxBytes_ &&
               buffer_ == right.buffer_;
    }
};

// NewInflights sets up an Inflights that allows up to size inflight messages,
// with the total byte size up to maxBytes. If maxBytes is 0 then there is no
// byte size limit. The maxBytes limit is soft, i.e. we accept a single message
// that brings it from size < maxBytes to size >= maxBytes.
inline std::shared_ptr<Inflights> NewInflights(size_t size, uint64_t maxBytes) {
    std::shared_ptr<Inflights> ret(new Inflights);
    ret->size_ = size;
    ret->maxBytes_ = maxBytes;
    return ret;
}

}