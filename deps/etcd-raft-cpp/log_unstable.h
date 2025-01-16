#pragma once
#include "util.h"
#include "raftpb/raft.pb.h"
namespace eraft {

// unstable contains "unstable" log entries and snapshot state that has
// not yet been written to Storage. The type serves two roles. First, it
// holds on to new log entries and an optional snapshot until they are
// handed to a Ready struct for persistence. Second, it continues to
// hold on to this state after it has been handed off to provide raftLog
// with a view of the in-progress log entries and snapshot until their
// writes have been stabilized and are guaranteed to be reflected in
// queries of Storage. After this point, the corresponding log entries
// and/or snapshot can be cleared from unstable.
//
// unstable.entries[i] has raft log position i+unstable.offset.
// Note that unstable.offset may be less than the highest log
// position in storage; this means that the next write to storage
// might need to truncate the log before persisting unstable.entries.
struct Unstable {
	// the incoming unstable snapshot, if any.
    std::shared_ptr<raftpb::Snapshot> snapshot_ = nullptr;
	// all entries that have not yet been written to storage.
    std::vector<raftpb::Entry> entries_;
	// entries[i] has raft log position i+offset.
	uint64_t offset_ = 0;

	// if true, snapshot is being written to storage.
	bool snapshotInProgress_ = false;
	// entries[:offsetInProgress-offset] are being written to storage.
	// Like offset, offsetInProgress is exclusive, meaning that it
	// contains the index following the largest in-progress entry.
	// Invariant: offset <= offsetInProgress
	uint64_t offsetInProgress_ = 0;

    // maybeFirstIndex returns the index of the first possible entry in entries
    // if it has a snapshot.
    std::pair<uint64_t, bool> maybeFirstIndex() {
        if (snapshot_) {
            return {snapshot_->metadata().index() + 1, true};
        }
        return {0, false};
    }

    // maybeLastIndex returns the last index if it has at least one
    // unstable entry or snapshot.
    std::pair<uint64_t, bool> maybeLastIndex() {
        if (!entries_.empty()) {
            return {offset_ + entries_.size() - 1, true};
        }
        if (snapshot_) {
            return {snapshot_->metadata().index(), true};
        }
        return {0, false};
    }

    // maybeTerm returns the term of the entry at index i, if there
    // is any.
    std::pair<uint64_t, bool> maybeTerm(uint64_t i) {
        if (i < offset_) {
            if (snapshot_ != nullptr && snapshot_->metadata().index() == i) {
                return {snapshot_->metadata().term(), true};
            }
            return {0, false};
        }
        uint64_t last = 0;
        bool ok = false;
        std::tie(last, ok) = maybeLastIndex();
        if (!ok) {
            return {0, false};
        }
        if (i > last) {
            return {0, false};
        }

        return {entries_[i - offset_].term(), true};
    }

    // nextEntries returns the unstable entries that are not already in the process
    // of being written to storage.
    std::vector<raftpb::Entry> nextEntries() {
        auto inProgress = offsetInProgress_ - offset_;
        if (entries_.size() == inProgress) {
            return {};
        }
        // TODO(wangzhiyong)
        return {entries_.begin() + inProgress, entries_.end()};
    }

    // nextSnapshot returns the unstable snapshot, if one exists that is not already
    // in the process of being written to storage.
    std::shared_ptr<raftpb::Snapshot> nextSnapshot() {
        if (snapshot_ == nullptr || snapshotInProgress_) {
            return nullptr;
        }
        return snapshot_;
    }

    // acceptInProgress marks all entries and the snapshot, if any, in the unstable
    // as having begun the process of being written to storage. The entries/snapshot
    // will no longer be returned from nextEntries/nextSnapshot. However, new
    // entries/snapshots added after a call to acceptInProgress will be returned
    // from those methods, until the next call to acceptInProgress.
    void acceptInProgress() {
        if (!entries_.empty()) {
            // NOTE: +1 because offsetInProgress is exclusive, like offset.
            offsetInProgress_ = entries_[entries_.size()-1].index() + 1;
        }
        if (snapshot_ != nullptr) {
            snapshotInProgress_ = true;
        }
    }

    // stableTo marks entries up to the entry with the specified (index, term) as
    // being successfully written to stable storage.
    //
    // The method should only be called when the caller can attest that the entries
    // can not be overwritten by an in-progress log append. See the related comment
    // in newStorageAppendRespMsg.
    void stableTo(uint64_t i, uint64_t t) {
        uint64_t gt = 0;
        bool ok = false;
        std::tie(gt, ok) = maybeTerm(i);
        if (!ok) {
            // Unstable entry missing. Ignore.
            ERAFT_INFO("entry at index %d missing from unstable log; ignoring", i);
            return;
        }
        if (i < offset_) {
            // Index matched unstable snapshot, not unstable entry. Ignore.
            ERAFT_INFO("entry at index %d matched unstable snapshot; ignoring", i);
            return;
        }
        if (gt != t) {
            // Term mismatch between unstable entry and specified entry. Ignore.
            // This is possible if part or all of the unstable log was replaced
            // between that time that a set of entries started to be written to
            // stable storage and when they finished.
            ERAFT_INFO("entry at (index,term)=(%d,%d) mismatched with entry at (%d,%d) in unstable log; ignoring", i, t, i, gt);
            return;
        }
        auto num = i + 1 - offset_;
        entries_.erase(entries_.begin(), entries_.begin() + num);
        offset_ = i + 1;
        offsetInProgress_ = std::max(offsetInProgress_, offset_);
    }

    void stableSnapTo(uint64_t i) {
        if (snapshot_ != nullptr && snapshot_->metadata().index() == i) {
            snapshot_ = nullptr;
            snapshotInProgress_ = false;
        }
    }

    void restore(raftpb::Snapshot s) {
        offset_ = s.metadata().index() + 1;
        offsetInProgress_ = offset_;
        entries_.clear();
        snapshot_ = std::make_shared<raftpb::Snapshot>(std::move(s));
        snapshotInProgress_ = false;
    }

    // u.offset <= lo <= hi <= u.offset+len(u.entries)
    void mustCheckOutOfBounds(uint64_t lo, uint64_t hi) {
        if (lo > hi) {
            ERAFT_FATAL("invalid unstable.slice %d > %d", lo, hi);
        }
        auto upper = offset_ + entries_.size();
        if (lo < offset_ || hi > upper) {
            ERAFT_FATAL("unstable.slice[%d,%d) out of bound [%d,%d]", lo, hi, offset_, upper);
        }
    }

    // slice returns the entries from the unstable log with indexes in the range
    // [lo, hi). The entire range must be stored in the unstable log or the method
    // will panic. The returned slice can be appended to, but the entries in it must
    // not be changed because they are still shared with unstable.
    //
    // TODO(pavelkalinnikov): this, and similar []pb.Entry slices, may bubble up all
    // the way to the application code through Ready struct. Protect other slices
    // similarly, and document how the client can use them.
    std::vector<raftpb::Entry> slice(uint64_t lo, uint64_t hi) {
        mustCheckOutOfBounds(lo, hi);
        return {entries_.begin() + (lo - offset_), entries_.begin() + (hi - offset_)};
    }

    void truncateAndAppend(std::vector<raftpb::Entry> ents) {
        // TODO(nvanbenschoten): rename this variable to firstAppIndex.
        auto fromIndex = ents[0].index();
        if (fromIndex == offset_ + entries_.size()) {
            // fromIndex is the next index in the u.entries, so append directly.
            entries_.insert(entries_.end(), std::make_move_iterator(ents.begin()), std::make_move_iterator(ents.end()));
        } else if (fromIndex <= offset_) {
            ERAFT_INFO("replace the unstable entries from index %d", fromIndex);
            // The log is being truncated to before our current offset
            // portion, so set the offset and replace the entries.
            entries_ = std::move(ents);
            offset_ = fromIndex;
            offsetInProgress_ = offset_;
        } else {
            // Truncate to fromIndex (exclusive), and append the new entries.
            ERAFT_INFO("truncate the unstable entries before index %d", fromIndex);
            auto keep = slice(offset_, fromIndex);
            entries_ = std::move(keep);
            entries_.insert(entries_.end(), std::make_move_iterator(ents.begin()), std::make_move_iterator(ents.end()));
            // Only in-progress entries before fromIndex are still considered to be
            // in-progress.
            offsetInProgress_ = std::min(offsetInProgress_, fromIndex);
        }
    }

};

}