#pragma once
#include "util.h"
#include "storage.h"
#include "log_unstable.h"
namespace eraft {

struct RaftLog {
	// storage contains all stable entries since the last snapshot.
    std::shared_ptr<Storage> storage_;

	// unstable contains all unstable entries and snapshot.
	// they will be saved into storage.
	Unstable unstable_;

	// committed is the highest log position that is known to be in
	// stable storage on a quorum of nodes.
	uint64_t committed_ = 0;
	// applying is the highest log position that the application has
	// been instructed to apply to its state machine. Some of these
	// entries may be in the process of applying and have not yet
	// reached applied.
	// Use: The field is incremented when accepting a Ready struct.
	// Invariant: applied <= applying && applying <= committed
	uint64_t applying_ = 0;
	// applied is the highest log position that the application has
	// successfully applied to its state machine.
	// Use: The field is incremented when advancing after the committed
	// entries in a Ready struct have been applied (either synchronously
	// or asynchronously).
	// Invariant: applied <= committed
	uint64_t applied_ = 0;

	// maxApplyingEntsSize limits the outstanding byte size of the messages
	// returned from calls to nextCommittedEnts that have not been acknowledged
	// by a call to appliedTo.
	uint64_t maxApplyingEntsSize_ = 0;
	// applyingEntsSize is the current outstanding byte size of the messages
	// returned from calls to nextCommittedEnts that have not been acknowledged
	// by a call to appliedTo.
	uint64_t applyingEntsSize_ = 0;
	// applyingEntsPaused is true when entry application has been paused until
	// enough progress is acknowledged.
	bool applyingEntsPaused_ = false;

    std::string String() const {
        return format("committed=%d, applied=%d, applying=%d, unstable.offset=%d, unstable.offsetInProgress=%d, len(unstable.Entries)=%d",
                           committed_, applied_, applying_, unstable_.offset_, unstable_.offsetInProgress_, unstable_.entries_.size());
    }

    uint64_t firstIndex() {
        uint64_t i = 0;
        bool ok = false;
        std::tie(i, ok) = unstable_.maybeFirstIndex();
        if (ok) {
            return i;
        }
        uint64_t index = 0;
        Error err;
        std::tie(index, err) = storage_->FirstIndex();
        if (err != nullptr) {
            ERAFT_FATAL(err.String().c_str());
        }
        return index;
    }

    uint64_t lastIndex() {
        uint64_t i = 0;
        bool ok = false;
        std::tie(i, ok) = unstable_.maybeLastIndex();
        if (ok) {
            return i;
        }
        Error err;
        std::tie(i, err) = storage_->LastIndex();
        if (err != nullptr) {
            ERAFT_FATAL(err.String().c_str());
        }
        return i;
    }

    std::pair<uint64_t, Error> term(uint64_t i) {
        // Check the unstable log first, even before computing the valid term range,
        // which may need to access stable Storage. If we find the entry's term in
        // the unstable log, we know it was in the valid range.
        uint64_t t = 0;
        bool ok = false;
        std::tie(t, ok) = unstable_.maybeTerm(i);
        if (ok) {
            return {t, Error()};
        }

        // The valid term range is [firstIndex-1, lastIndex]. Even though the entry at
        // firstIndex-1 is compacted away, its term is available for matching purposes
        // when doing log appends.
        if (i+1 < firstIndex()) {
            return {0, ErrCompacted};
        }
        if (i > lastIndex()) {
            return {0, ErrUnavailable};
        }

        Error err;
        std::tie(t, err) = storage_->Term(i);
        if (err == nullptr) {
            return {t, Error()};
        }
        if (err == ErrCompacted || err == ErrUnavailable) {
            return {0, err};
        }
        ERAFT_FATAL(err.String().c_str());
        return {0, {}};
    }

    bool matchTerm(uint64_t i, uint64_t te) {
        uint64_t t = 0;
        Error err;
        std::tie(t, err) = term(i);
        if (err != nullptr) {
            return false;
        }
        return t == te;
    }

    uint64_t zeroTermOnOutOfBounds(std::pair<uint64_t, Error> term) {
        if (term.second == nullptr) {
            return term.first;
        }
        if (term.second == ErrCompacted || term.second == ErrUnavailable) {
            return 0;
        }
        ERAFT_FATAL("unexpected error (%s)", term.second.String().c_str());
        return 0;
    }

    // findConflict finds the index of the conflict.
    // It returns the first pair of conflicting entries between the existing
    // entries and the given entries, if there are any.
    // If there is no conflicting entries, and the existing entries contains
    // all the given entries, zero will be returned.
    // If there is no conflicting entries, but the given entries contains new
    // entries, the index of the first new entry will be returned.
    // An entry is considered to be conflicting if it has the same index but
    // a different term.
    // The index of the given entries MUST be continuously increasing.
    uint64_t findConflict(const std::vector<raftpb::Entry>& ents) {
        for (auto& ne : ents) {
            if (!matchTerm(ne.index(), ne.term())) {
                if (ne.index() <= lastIndex()) {
                    ERAFT_INFO("found conflict at index %d [existing term: %d, conflicting term: %d]",
                        ne.index(), zeroTermOnOutOfBounds(term(ne.index())), ne.term());
                }
                return ne.index();
            }
        }
        return 0;
    }

    uint64_t append(std::vector<raftpb::Entry> ents) {
        if (ents.size() == 0) {
            return lastIndex();
        }
        if (ents[0].index() - 1 < committed_) {
            ERAFT_FATAL("after(%d) is out of range [committed(%d)]", ents[0].index() - 1, committed_);
        }
        unstable_.truncateAndAppend(std::move(ents));
        return  lastIndex();
    }

    void commitTo(uint64_t tocommit) {
        // never decrease commit
        if (committed_ < tocommit) {
            if (lastIndex() < tocommit) {
                ERAFT_FATAL("tocommit(%d) is out of range [lastIndex(%d)]. Was the raft log corrupted, truncated, or lost?", tocommit, lastIndex());
            }
            committed_ = tocommit;
        }
    }

    // maybeAppend returns (0, false) if the entries cannot be appended. Otherwise,
    // it returns (last index of new entries, true).
    std::pair<uint64_t, bool> maybeAppend(uint64_t index, uint64_t logTerm, uint64_t committed, std::vector<raftpb::Entry> ents) {
        if (!matchTerm(index, logTerm)) {
            return {0, false};
        }

        auto lastnewi = index + ents.size();
        auto ci = findConflict(ents);

        if (ci == 0) {
        } else if (ci <= committed_) {
            ERAFT_FATAL("entry %d conflict with committed entry [committed(%d)]", ci, committed_);
        } else {
            auto offset = index + 1;
            if (ci - offset > ents.size()) {
                ERAFT_FATAL("index, %d, is out of range [%d]", ci - offset, ents.size());
            }
            append({ents.begin() + int(ci - offset), ents.end()});
        }

        commitTo(std::min(committed, lastnewi));
        return {lastnewi, true};
    }

    // findConflictByTerm returns a best guess on where this log ends matching
    // another log, given that the only information known about the other log is the
    // (index, term) of its single entry.
    //
    // Specifically, the first returned value is the max guessIndex <= index, such
    // that term(guessIndex) <= term or term(guessIndex) is not known (because this
    // index is compacted or not yet stored).
    //
    // The second returned value is the term(guessIndex), or 0 if it is unknown.
    //
    // This function is used by a follower and leader to resolve log conflicts after
    // an unsuccessful append to a follower, and ultimately restore the steady flow
    // of appends.
    std::pair<uint64_t, uint64_t> findConflictByTerm(uint64_t index, uint64_t _term) {
        for (; index > 0; index--) {
            // If there is an error (likely ErrCompacted or ErrUnavailable), we don't
            // know whether it's a match or not, so assume a possible match and return
            // the index, with 0 term indicating an unknown term.
            Error err;
            uint64_t ourTerm = 0;
            std::tie(ourTerm, err) = term(index);
            if (err != nullptr) {
                return {index, 0};
            } else if (ourTerm <= _term) {
                return {index, ourTerm};
            }
        }
        return {0, 0};
    }

    // nextUnstableEnts returns all entries that are available to be written to the
    // local stable log and are not already in-progress.
    std::vector<raftpb::Entry> nextUnstableEnts() {
        return unstable_.nextEntries();
    }

    // hasNextUnstableEnts returns if there are any entries that are available to be
    // written to the local stable log and are not already in-progress.
    bool hasNextUnstableEnts() {
        return nextUnstableEnts().size() > 0;
    }

    // hasNextOrInProgressUnstableEnts returns if there are any entries that are
    // available to be written to the local stable log or in the process of being
    // written to the local stable log.
    bool hasNextOrInProgressUnstableEnts() const {
        return unstable_.entries_.size() > 0;
    }

    // hasNextOrInProgressSnapshot returns if there is pending snapshot waiting for
    // applying or in the process of being applied.
    bool hasNextOrInProgressSnapshot() const {
        return unstable_.snapshot_ != nullptr;
    }

    // maxAppliableIndex returns the maximum committed index that can be applied.
    // If allowUnstable is true, committed entries from the unstable log can be
    // applied; otherwise, only entries known to reside locally on stable storage
    // can be applied.
    uint64_t maxAppliableIndex(bool allowUnstable) const {
        auto hi = committed_;
        if (!allowUnstable) {
            hi = std::min(hi, unstable_.offset_-1);
        }
        return hi;
    }

    // l.firstIndex <= lo <= hi <= l.firstIndex + len(l.entries)
    Error mustCheckOutOfBounds(uint64_t lo, uint64_t hi) {
        if (lo > hi) {
            ERAFT_FATAL("invalid slice %d > %d", lo, hi);
        }
        auto fi = firstIndex();
        if (lo < fi) {
            return ErrCompacted;
        }

        auto length = lastIndex() + 1 - fi;
        if (hi > fi+length) {
            ERAFT_FATAL("slice[%d,%d) out of bound [%d,%d]", lo, hi, fi, lastIndex());
        }
        return Error();
    }

    // scan visits all log entries in the [lo, hi) range, returning them via the
    // given callback. The callback can be invoked multiple times, with consecutive
    // sub-ranges of the requested range. Returns up to pageSize bytes worth of
    // entries at a time. May return more if a single entry size exceeds the limit.
    //
    // The entries in [lo, hi) must exist, otherwise scan() eventually returns an
    // error (possibly after passing some entries through the callback).
    //
    // If the callback returns an error, scan terminates and returns this error
    // immediately. This can be used to stop the scan early ("break" the loop).
    Error scan(uint64_t lo, uint64_t hi, int pageSize, const std::function<Error(const std::vector<raftpb::Entry>&)>& v) {
        while (lo < hi) {
            std::vector<raftpb::Entry> ents;
            Error err;
            std::tie(ents, err) = slice(lo, hi, pageSize);
            if (err != nullptr) {
                return err;
            } else if (ents.empty()) {
                return Error(format("got 0 entries in [%d, %d)", lo, hi));
            }
            err = v(ents);
            if (err != nullptr) {
                return err;
            }
            lo += ents.size();
        }
        return nullptr;
    }

    // slice returns a slice of log entries from lo through hi-1, inclusive.
    std::pair<std::vector<raftpb::Entry>, Error> slice(uint64_t lo, uint64_t hi, uint64_t maxSize) {
        auto err = mustCheckOutOfBounds(lo, hi);
        if (err != nullptr) {
            return {{}, err};
        }
        if (lo == hi) {
            return {{}, nullptr};
        }
        if (lo >= unstable_.offset_) {
            auto ents = unstable_.slice(lo, hi);
            limitSize(ents, maxSize);
            // NB: use the full slice expression to protect the unstable slice from
            // appends to the returned ents slice.
            return {std::move(ents), nullptr};
        }

        auto cut = std::min(hi, unstable_.offset_);
        std::vector<raftpb::Entry> ents;
        std::tie(ents, err) = storage_->Entries(lo, cut, maxSize);
        if (err == ErrCompacted) {
            return {{}, err};
        } else if (err == ErrUnavailable) {
            ERAFT_FATAL("entries[%d:%d) is unavailable from storage", lo, cut);
        } else if (err != nullptr) {
            ERAFT_FATAL("%s", err.String().c_str()); // TODO(pavelkalinnikov): handle errors uniformly
        }
        if (hi <= unstable_.offset_) {
            return {std::move(ents), nullptr};
        }

        // Fast path to check if ents has reached the size limitation. Either the
        // returned slice is shorter than requested (which means the next entry would
        // bring it over the limit), or a single entry reaches the limit.
        if (ents.size() < cut-lo) {
            return {std::move(ents), nullptr};
        }
        // Slow path computes the actual total size, so that unstable entries are cut
        // optimally before being copied to ents slice.
        auto size = entsSize(ents);
        if (size >= maxSize) {
            return {std::move(ents), nullptr};
        }

        auto unstable = unstable_.slice(unstable_.offset_, hi);
        limitSize(unstable, maxSize-size);
        // Total size of unstable may exceed maxSize-size only if len(unstable) == 1.
        // If this happens, ignore this extra entry.
        if (unstable.size() == 1 && size+entsSize(unstable) > maxSize) {
            return {ents, nullptr};
        }
        // Otherwise, total size of unstable does not exceed maxSize-size, so total
        // size of ents+unstable does not exceed maxSize. Simply concatenate them.
        ents.insert(ents.end(), unstable.begin(), unstable.end());
        return {std::move(ents), nullptr};
    }

    // nextCommittedEnts returns all the available entries for execution.
    // Entries can be committed even when the local raft instance has not durably
    // appended them to the local raft log yet. If allowUnstable is true, committed
    // entries from the unstable log may be returned; otherwise, only entries known
    // to reside locally on stable storage will be returned.
    std::vector<raftpb::Entry> nextCommittedEnts(bool allowUnstable) {
        if (applyingEntsPaused_) {
            // Entry application outstanding size limit reached.
            return {};
        }
        if (hasNextOrInProgressSnapshot()) {
            // See comment in hasNextCommittedEnts.
            return {};
        }
        // [lo, hi)
        uint64_t lo = applying_+1;
        uint64_t hi = maxAppliableIndex(allowUnstable)+1;
        if (lo >= hi) {
            // Nothing to apply.
            return {};
        }
        auto maxSize = maxApplyingEntsSize_ - applyingEntsSize_;
        if (maxSize <= 0) {
            ERAFT_FATAL("applying entry size (%d-%d)=%d not positive",
                maxApplyingEntsSize_, applyingEntsSize_, maxSize);
        }
        std::vector<raftpb::Entry> ents;
        Error err;
        std::tie(ents, err) = slice(lo, hi, maxSize);
        if (err != nullptr) {
            ERAFT_FATAL("unexpected error when getting unapplied entries (%s)", err.String().c_str());
        }
        return ents;
    }


    // hasNextCommittedEnts returns if there is any available entries for execution.
    // This is a fast check without heavy raftLog.slice() in nextCommittedEnts().
    bool hasNextCommittedEnts(bool allowUnstable) {
        if (applyingEntsPaused_) {
            // Entry application outstanding size limit reached.
            return false;
        }
        if (hasNextOrInProgressSnapshot()) {
            // If we have a snapshot to apply, don't also return any committed
            // entries. Doing so raises questions about what should be applied
            // first.
            return false;
        }
        // [lo, hi)
        auto lo = applying_+1;
        auto hi = maxAppliableIndex(allowUnstable)+1;
        return lo < hi;
    }

    // nextUnstableSnapshot returns the snapshot, if present, that is available to
    // be applied to the local storage and is not already in-progress.
    std::shared_ptr<raftpb::Snapshot> nextUnstableSnapshot() {
        return unstable_.nextSnapshot();
    }

    // hasNextUnstableSnapshot returns if there is a snapshot that is available to
    // be applied to the local storage and is not already in-progress.
    bool hasNextUnstableSnapshot() {
        return unstable_.nextSnapshot() != nullptr;
    }

    std::pair<raftpb::Snapshot, Error> snapshot() {
        if (unstable_.snapshot_ != nullptr) {
                return {*unstable_.snapshot_, {}};
        }
        return storage_->Snapshot();
    }

    void appliedTo(uint64_t i, uint64_t size) {
        if (committed_ < i || i < applied_) {
            ERAFT_FATAL("applied(%d) is out of range [prevApplied(%d), committed(%d)]", i, applied_, committed_);
        }
        applied_ = i;
        applying_ = std::max(applying_, i);
        if (applyingEntsSize_ > size) {
            applyingEntsSize_ -= size;
        } else {
            // Defense against underflow.
            applyingEntsSize_ = 0;
        }
        applyingEntsPaused_ = applyingEntsSize_ >= maxApplyingEntsSize_;
    }

    void acceptApplying(uint64_t i, uint64_t size, bool allowUnstable) {
        if (committed_ < i) {
            ERAFT_FATAL("applying(%d) is out of range [prevApplying(%d), committed(%d)]", i, applying_, committed_);
        }
        applying_ = i;
        applyingEntsSize_ += size;
        // Determine whether to pause entry application until some progress is
        // acknowledged. We pause in two cases:
        // 1. the outstanding entry size equals or exceeds the maximum size.
        // 2. the outstanding entry size does not equal or exceed the maximum size,
        //    but we determine that the next entry in the log will push us over the
        //    limit. We determine this by comparing the last entry returned from
        //    raftLog.nextCommittedEnts to the maximum entry that the method was
        //    allowed to return had there been no size limit. If these indexes are
        //    not equal, then the returned entries slice must have been truncated to
        //    adhere to the memory limit.
        applyingEntsPaused_ = applyingEntsSize_ >= maxApplyingEntsSize_ ||
            i < maxAppliableIndex(allowUnstable);
    };

    void stableTo(uint64_t i, uint64_t t) {
        unstable_.stableTo(i, t);
    }

    void stableSnapTo(uint64_t i) {
        unstable_.stableSnapTo(i);
    }

    // acceptUnstable indicates that the application has started persisting the
    // unstable entries in storage, and that the current unstable entries are thus
    // to be marked as being in-progress, to avoid returning them with future calls
    // to Ready().
    void acceptUnstable() {
        unstable_.acceptInProgress();
    }

    uint64_t lastTerm() {
        uint64_t t = 0;
        Error err;
        std::tie(t, err) = term(lastIndex());
        if (err != nullptr) {
            ERAFT_FATAL("unexpected error when getting the last term (%s)", err.String().c_str());
        }
        return t;
    }

    std::pair<std::vector<raftpb::Entry>, Error> entries(uint64_t i, uint64_t maxSize){
        if (i > lastIndex()) {
            return {{}, {}};
        }
        return slice(i, lastIndex()+1, maxSize);
    }

    // allEntries returns all entries in the log.
    std::vector<raftpb::Entry> allEntries() {
        std::vector<raftpb::Entry> ents;
        Error err;
        std::tie(ents, err) = entries(firstIndex(), noLimit);
        if (err == nullptr) {
            return ents;
        }
        if (err == ErrCompacted) { // try again if there was a racing compaction
            return allEntries();
        }
        ERAFT_FATAL(err.String().c_str());
        return {};
    }

    // isUpToDate determines if the given (lastIndex,term) log is more up-to-date
    // by comparing the index and term of the last entries in the existing logs.
    // If the logs have last entries with different terms, then the log with the
    // later term is more up-to-date. If the logs end with the same term, then
    // whichever log has the larger lastIndex is more up-to-date. If the logs are
    // the same, the given log is up-to-date.
    bool isUpToDate(uint64_t lasti, uint64_t term) {
        return term > lastTerm() || (term == lastTerm() && lasti >= lastIndex());
    }

    bool maybeCommit(uint64_t maxIndex, uint64_t te) {
        // NB: term should never be 0 on a commit because the leader campaigns at
        // least at term 1. But if it is 0 for some reason, we don't want to consider
        // this a term match in case zeroTermOnOutOfBounds returns 0.
        if (maxIndex > committed_ && te != 0 && zeroTermOnOutOfBounds(term(maxIndex)) == te) {
            commitTo(maxIndex);
            return true;
        }
        return false;
    }

    void restore(raftpb::Snapshot s) {
        ERAFT_INFO("log [] starts to restore snapshot [index: %d, term: %d]", s.metadata().index(), s.metadata().term());
        committed_ = s.metadata().index();
        unstable_.restore(s);
    }

};

namespace detail {
// newLogWithSize returns a log using the given storage and max
// message size.
inline std::shared_ptr<RaftLog> newLogWithSize(const std::shared_ptr<Storage>& storage, uint64_t maxApplyingEntsSize) {
    if (storage == nullptr) {
        ERAFT_FATAL("storage must not be nil");
    }
    auto log = std::make_shared<RaftLog>();
    log->storage_ = storage;
    log->maxApplyingEntsSize_ = maxApplyingEntsSize;
    uint64_t firstIndex = 0;
    Error err;
    std::tie(firstIndex, err) = storage->FirstIndex();
    if (err != nullptr) {
        ERAFT_FATAL(err.String().c_str());
    }
    uint64_t lastIndex = 0;
    std::tie(lastIndex, err) = storage->LastIndex();
    if (err != nullptr) {
        ERAFT_FATAL(err.String().c_str());
    }
    log->unstable_.offset_ = lastIndex + 1;
    log->unstable_.offsetInProgress_ = lastIndex + 1;
    // Initialize our committed and applied pointers to the time of the last compaction.
    log->committed_ = firstIndex - 1;
    log->applying_ = firstIndex - 1;
    log->applied_ = firstIndex - 1;

    return log;
}

// newLog returns log using the given storage and default options. It
// recovers the log to the state that it just commits and applies the
// latest snapshot.
inline std::shared_ptr<RaftLog> newLog(const std::shared_ptr<Storage>& storage) {
    return newLogWithSize(storage, noLimit);
}
}

}
