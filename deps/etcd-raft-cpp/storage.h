#pragma once
#include "util.h"
#include "raftpb/raft.pb.h"

namespace eraft {
// ErrCompacted is returned by Storage.Entries/Compact when a requested
// index is unavailable because it predates the last snapshot.
const Error ErrCompacted(Error::Compacted, "requested index is unavailable due to compaction");

// ErrSnapOutOfDate is returned by Storage.CreateSnapshot when a requested
// index is older than the existing snapshot.
const Error ErrSnapOutOfDate(Error::SnapOutOfDate, "requested index is older than the existing snapshot");

// ErrUnavailable is returned by Storage interface when the requested log entries
// are unavailable.
const Error ErrUnavailable(Error::Unavailable, "requested entry at index is unavailable");

// ErrSnapshotTemporarilyUnavailable is returned by the Storage interface when the required
// snapshot is temporarily unavailable.
const Error ErrSnapshotTemporarilyUnavailable(Error::SnapshotTemporarilyUnavailable, "snapshot is temporarily unavailable");

const Error errBreak("break");

// Storage is an interface that may be implemented by the application
// to retrieve log entries from storage.
//
// If any Storage method returns an error, the raft instance will
// become inoperable and refuse to participate in elections; the
// application is responsible for cleanup and recovery in this case.
struct Storage {
	// TODO(tbg): split this into two interfaces, LogStorage and StateStorage.

	// InitialState returns the saved HardState and ConfState information.
    virtual std::tuple<raftpb::HardState, raftpb::ConfState, Error> InitialState() = 0;

    // Entries returns a slice of consecutive log entries in the range [lo, hi),
    // starting from lo. The maxSize limits the total size of the log entries
    // returned, but Entries returns at least one entry if any.
    //
    // The caller of Entries owns the returned slice, and may append to it. The
    // individual entries in the slice must not be mutated, neither by the Storage
    // implementation nor the caller. Note that raft may forward these entries
    // back to the application via Ready struct, so the corresponding handler must
    // not mutate entries either (see comments in Ready struct).
    //
    // Since the caller may append to the returned slice, Storage implementation
    // must protect its state from corruption that such appends may cause. For
    // example, common ways to do so are:
    //  - allocate the slice before returning it (safest option),
    //  - return a slice protected by Go full slice expression, which causes
    //  copying on appends (see MemoryStorage).
    //
    // Returns ErrCompacted if entry lo has been compacted, or ErrUnavailable if
    // encountered an unavailable entry in [lo, hi).
    virtual std::pair<std::vector<raftpb::Entry>, Error> Entries(uint64_t lo, uint64_t hi, uint64_t maxSize) = 0;
	// Term returns the term of entry i, which must be in the range
	// [FirstIndex()-1, LastIndex()]. The term of the entry before
	// FirstIndex is retained for matching purposes even though the
	// rest of that entry may not be available.
    virtual std::pair<uint64_t, Error> Term(uint64_t i) = 0;
	// LastIndex returns the index of the last entry in the log.
    virtual std::pair<uint64_t, Error> LastIndex() = 0;
	// FirstIndex returns the index of the first log entry that is
	// possibly available via Entries (older entries have been incorporated
	// into the latest Snapshot; if storage only contains the dummy entry the
	// first log entry is not available).
    virtual std::pair<uint64_t, Error> FirstIndex() = 0;
	// Snapshot returns the most recent snapshot.
	// If snapshot is temporarily unavailable, it should return ErrSnapshotTemporarilyUnavailable,
	// so raft state machine could know that Storage needs some time to prepare
	// snapshot and call Snapshot later.
    virtual std::pair<raftpb::Snapshot, Error> Snapshot() = 0;

    virtual ~Storage() = default;
};

namespace detail {
struct inMemStorageCallStats {
    int initialState = 0;
    int firstIndex = 0;
    int lastIndex = 0;
    int entries = 0;
    int term = 0;
    int snapshot = 0;
};
}

// MemoryStorage implements the Storage interface backed by an
// in-memory array.
struct MemoryStorage : public Storage {
#ifndef RAFT_TEST
private:
#else
public:
#endif
	// Protects access to all fields. Most methods of MemoryStorage are
	// run on the raft goroutine, but Append() is run on an application
	// goroutine.
	std::mutex mutex_;

    raftpb::HardState hardState_;
    raftpb::Snapshot snapshot_;
	// ents[i] has raft log position i+snapshot.Metadata.Index
	std::vector<raftpb::Entry> ents_;

    detail::inMemStorageCallStats callStats_;

    uint64_t lastIndex() {
        return ents_[0].index() + ents_.size() - 1;
    }

public:
    MemoryStorage() = default;
    explicit MemoryStorage(std::vector<raftpb::Entry> ents) : ents_(std::move(ents)) {}

    // InitialState implements the Storage interface.
    std::tuple<raftpb::HardState, raftpb::ConfState, Error> InitialState() override {
        callStats_.initialState++;
        return std::make_tuple(hardState_, snapshot_.metadata().conf_state(), Error());
    }

    // SetHardState saves the current HardState.
    Error SetHardState(raftpb::HardState st) {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        hardState_ = std::move(st);
        return nullptr;
    }

    // Entries implements the Storage interface.
    std::pair<std::vector<raftpb::Entry>, Error> Entries(uint64_t lo, uint64_t hi, uint64_t maxSize) override {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        callStats_.entries++;
        auto offset = ents_[0].index();
        if (lo <= offset) {
            return {{}, ErrCompacted};
        }
        if (hi > lastIndex()+1) {
            ERAFT_FATAL("entries' hi(%d) is out of bound lastindex(%d)", hi, lastIndex());
        }
        // only contains dummy entries.
        if (ents_.size() == 1) {
            return {{}, ErrUnavailable};
        }
        std::vector<raftpb::Entry> ret(ents_.begin() + (int64_t)(lo - offset), ents_.begin() + (int64_t)(hi - offset));
        limitSize(ret, maxSize);
        return {std::move(ret), nullptr};
    }

    // Term implements the Storage interface.
    std::pair<uint64_t, Error> Term(uint64_t i) override {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        callStats_.term++;
        auto offset = ents_[0].index();
        if (i < offset) {
            return {0, ErrCompacted};
        }
        if (i-offset >= ents_.size()) {
            return {0, ErrUnavailable};
        }
        return {ents_[i - offset].term(), nullptr};
    }

    // LastIndex implements the Storage interface.
    std::pair<uint64_t, Error> LastIndex() override {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        callStats_.lastIndex++;
        return {lastIndex(), nullptr};
    }

    uint64_t firstIndex() {
        return ents_[0].index() + 1;
    }

    // FirstIndex implements the Storage interface.
    std::pair<uint64_t, Error> FirstIndex() override {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        callStats_.firstIndex++;
        return {firstIndex(), nullptr};
    }

    // Snapshot implements the Storage interface.
    std::pair<raftpb::Snapshot, Error> Snapshot() override {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        callStats_.snapshot++;
        return {snapshot_, nullptr};
    }

    // ApplySnapshot overwrites the contents of this Storage object with
    // those of the given snapshot.
    Error ApplySnapshot(raftpb::Snapshot snap) {
        std::lock_guard<std::mutex> lockGuard(mutex_);

        //handle check for old snapshot being applied
        auto msIndex = snapshot_.metadata().index();
        auto snapIndex = snap.metadata().index();
        if (msIndex >= snapIndex) {
            return ErrSnapOutOfDate;
        }

        snapshot_ = std::move(snap);
        ents_.clear();
        ents_.emplace_back();
        ents_.back().set_term(snapshot_.metadata().term());
        ents_.back().set_index(snapshot_.metadata().index());
        return nullptr;
    }

    // CreateSnapshot makes a snapshot which can be retrieved with Snapshot() and
    // can be used to reconstruct the state at that point.
    // If any configuration changes have been made since the last compaction,
    // the result of the last ApplyConfChange must be passed in.
    std::pair<raftpb::Snapshot, Error> CreateSnapshot(uint64_t i, raftpb::ConfState* cs, std::string data) {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        if (i <= snapshot_.metadata().index()) {
            return {{}, ErrSnapOutOfDate};
        }

        auto offset = ents_[0].index();
        if (i > lastIndex()) {
            ERAFT_FATAL("snapshot %d is out of bound lastindex(%d)", i, lastIndex());
        }

        snapshot_.mutable_metadata()->set_index(i);
        snapshot_.mutable_metadata()->set_term(ents_[i - offset].term());
        if (cs != nullptr) {
            *snapshot_.mutable_metadata()->mutable_conf_state() = *cs;
        }
        snapshot_.set_data(std::move(data));
        return {snapshot_, nullptr};
    }

    // Compact discards all log entries prior to compactIndex.
    // It is the application's responsibility to not attempt to compact an index
    // greater than raftLog.applied.
    Error Compact(uint64_t compactIndex) {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        auto offset = ents_[0].index();
        if (compactIndex <= offset) {
            return ErrCompacted;
        }
        if (compactIndex > lastIndex()) {
            ERAFT_FATAL("compact %d is out of bound lastindex(%d)", compactIndex, lastIndex());
        }

        auto i = compactIndex - offset;
        ents_.erase(ents_.begin(), ents_.begin() + (int64_t)i);
        return nullptr;
    }

    // Append the new entries to storage.
    // TODO (xiangli): ensure the entries are continuous and
    // entries[0].Index > ms.entries[0].Index
    Error Append(std::vector<raftpb::Entry> entries) {
        if (entries.empty()) {
            return nullptr;
        }
        std::lock_guard<std::mutex> lockGuard(mutex_);

        auto first = firstIndex();
        auto last = entries[0].index() + entries.size() - 1;

        // shortcut if there is no new entry.
        if (last < first) {
            return nullptr;
        }
        // truncate compacted entries
        if (first > entries[0].index()) {
            entries.erase(entries.begin(), entries.begin() + (int64_t)(first-entries[0].index()));
        }

        auto offset = entries[0].index() - ents_[0].index();
        if (ents_.size() > offset) {
            ents_.erase(ents_.begin() + (int64_t)offset, ents_.end());
            ents_.insert(ents_.end(), entries.begin(), entries.end());
        } else if (ents_.size() == offset) {
            ents_.insert(ents_.end(), entries.begin(), entries.end());
        } else {
            ERAFT_FATAL("missing log entry [last: %d, append at: %d]", lastIndex(), entries[0].index());
        }
        return nullptr;
    }
};

// NewMemoryStorage creates an empty MemoryStorage.
inline std::shared_ptr<MemoryStorage> NewMemoryStorage() {
    return std::make_shared<MemoryStorage>(std::vector<raftpb::Entry>{{}});
}

}
