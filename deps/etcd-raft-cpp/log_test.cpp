#include "log.h"
#include <gtest/gtest.h>

namespace eraft {

static uint64_t mustTerm(const std::pair<uint64_t, Error>& ret) {
    EXPECT_TRUE(ret.second == nullptr);
    return ret.first;
}

TEST(log, TestFindConflict) {
    std::vector<raftpb::Entry> previousEnts = {EntryHelper{.index = 1,.term = 1}.Done(),
                                               EntryHelper{.index = 2,.term = 2}.Done(),
                                               EntryHelper{.index = 3,.term = 3}.Done()};
    struct Test {
        std::vector<raftpb::Entry> ents;
        uint64_t wconflict;
        Test(std::vector<raftpb::Entry> _ents, uint64_t _wconflict) : ents(std::move(_ents)), wconflict(_wconflict) {}
    };
    std::vector<Test> tests = {
		// no conflict, empty ent
		{{}, 0},
		// no conflict
		{{EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done(), EntryHelper{.index = 3,.term = 3}.Done()}, 0},
		{{EntryHelper{.index = 2,.term = 2}.Done(), EntryHelper{.index = 3,.term = 3}.Done()}, 0},
		{{EntryHelper{.index = 3,.term = 3}.Done()}, 0},
		// no conflict, but has new entries
		{{EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done(), EntryHelper{.index = 3,.term = 3}.Done(), EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 4}.Done()}, 4},
		{{EntryHelper{.index = 2,.term = 2}.Done(), EntryHelper{.index = 3,.term = 3}.Done(), EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 4}.Done()}, 4},
		{{EntryHelper{.index = 3,.term = 3}.Done(), EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 4}.Done()}, 4},
		{{EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 4}.Done()}, 4},
		// conflicts with existing entries
		{{EntryHelper{.index = 1,.term = 4}.Done(), EntryHelper{.index = 2,.term = 4}.Done()}, 1},
		{{EntryHelper{.index = 2,.term = 1}.Done(), EntryHelper{.index = 3,.term = 3}.Done(), EntryHelper{.index = 4,.term = 4}.Done()}, 2},
		{{EntryHelper{.index = 3,.term = 1}.Done(), EntryHelper{.index = 4,.term = 2}.Done(), EntryHelper{.index = 5,.term = 4}.Done(), EntryHelper{.index = 6,.term = 4}.Done()}, 3},
	};
    for (auto& tt : tests) {
		auto raftLog = detail::newLog(NewMemoryStorage());
		raftLog->append(previousEnts);
		auto gconflict = raftLog->findConflict(tt.ents);
        EXPECT_EQ(gconflict, tt.wconflict);
	}
}

TEST(log, TestFindConflictByTerm) {
	auto ents = [](uint64_t fromIndex, std::vector<uint64_t> terms) ->std::vector<raftpb::Entry> {
        std::vector<raftpb::Entry> e;
        for (size_t i = 0; i < terms.size(); i++) {
            e.push_back(EntryHelper{.index = fromIndex + i, .term = terms[i]}.Done());
        }
		return e;
	};
    struct Test {
        std::vector<raftpb::Entry> ents; // ents[0] contains the (index, term) of the snapshot
        uint64_t index;
        uint64_t term;
        uint64_t want;
    };
    std::vector<Test> tests = {
		// Log starts from index 1.
		{ents(0, {0, 2, 2, 5, 5, 5}), 100, 2, 100}, // ErrUnavailable
		{ents(0, {0, 2, 2, 5, 5, 5}), 5, 6, 5},
		{ents(0, {0, 2, 2, 5, 5, 5}), 5, 5, 5},
		{ents(0, {0, 2, 2, 5, 5, 5}), 5, 4, 2},
		{ents(0, {0, 2, 2, 5, 5, 5}), 5, 2, 2},
		{ents(0, {0, 2, 2, 5, 5, 5}), 5, 1, 0},
		{ents(0, {0, 2, 2, 5, 5, 5}), 1, 2, 1},
		{ents(0, {0, 2, 2, 5, 5, 5}), 1, 1, 0},
		{ents(0, {0, 2, 2, 5, 5, 5}), 0, 0, 0},
		// Log with compacted entries.
		{ents(10, {3, 3, 3, 4, 4, 4}), 30, 3, 30}, // ErrUnavailable
		{ents(10, {3, 3, 3, 4, 4, 4}), 14, 9, 14},
		{ents(10, {3, 3, 3, 4, 4, 4}), 14, 4, 14},
		{ents(10, {3, 3, 3, 4, 4, 4}), 14, 3, 12},
		{ents(10, {3, 3, 3, 4, 4, 4}), 14, 2, 9},
		{ents(10, {3, 3, 3, 4, 4, 4}), 11, 5, 11},
		{ents(10, {3, 3, 3, 4, 4, 4}), 10, 5, 10},
		{ents(10, {3, 3, 3, 4, 4, 4}), 10, 3, 10},
		{ents(10, {3, 3, 3, 4, 4, 4}), 10, 2, 9},
		{ents(10, {3, 3, 3, 4, 4, 4}), 9, 2, 9}, // ErrCompacted
		{ents(10, {3, 3, 3, 4, 4, 4}), 4, 2, 4}, // ErrCompacted
		{ents(10, {3, 3, 3, 4, 4, 4}), 0, 0, 0}, // ErrCompacted
	};
    for (auto& tt : tests) {
        auto st = NewMemoryStorage();
        EXPECT_FALSE(tt.ents.empty());
        st->ApplySnapshot(SnapshotHelper{.index = tt.ents[0].index(), .term = tt.ents[0].term()}.Done());
        auto l = detail::newLog(st);
        l->append({tt.ents.begin() + 1, tt.ents.end()});

        uint64_t index, term;
        std::tie(index, term) = l->findConflictByTerm(tt.index, tt.term);
        EXPECT_EQ(tt.want, index);
        uint64_t wantTerm;
        Error err;
        std::tie(wantTerm, err) = l->term(index);
        wantTerm = l->zeroTermOnOutOfBounds({wantTerm, err});
        EXPECT_EQ(wantTerm, term);
	}
}

TEST(log, TestIsUpToDate) {
    std::vector<raftpb::Entry> previousEnts = {EntryHelper{.index = 1,.term = 1}.Done(),
                                               EntryHelper{.index = 2,.term = 2}.Done(),
                                               EntryHelper{.index = 3,.term = 3}.Done()};
	auto raftLog = detail::newLog(NewMemoryStorage());
	raftLog->append(previousEnts);
    struct Test {
        uint64_t lastIndex;
        uint64_t term;
        bool wUpToDate;
        Test(uint64_t _lastIndex, uint64_t _term, bool _wUpToDate)
            : lastIndex(_lastIndex),term(_term),wUpToDate(_wUpToDate) {}
    };
    std::vector<Test> tests = {
		// greater term, ignore lastIndex
		{raftLog->lastIndex() - 1, 4, true},
		{raftLog->lastIndex(), 4, true},
		{raftLog->lastIndex() + 1, 4, true},
		// smaller term, ignore lastIndex
		{raftLog->lastIndex() - 1, 2, false},
		{raftLog->lastIndex(), 2, false},
		{raftLog->lastIndex() + 1, 2, false},
		// equal term, equal or lager lastIndex wins
		{raftLog->lastIndex() - 1, 3, false},
		{raftLog->lastIndex(), 3, true},
		{raftLog->lastIndex() + 1, 3, true},
	};
    for (auto& tt : tests) {
		auto gUpToDate = raftLog->isUpToDate(tt.lastIndex, tt.term);
        EXPECT_EQ(gUpToDate, tt.wUpToDate);
	}
}

TEST(log, TestAppend) {
    std::vector<raftpb::Entry> previousEnts = {EntryHelper{.index = 1,.term = 1}.Done(),
                                               EntryHelper{.index = 2,.term = 2}.Done()};
    struct Test {
        std::vector<raftpb::Entry> ents;
        uint64_t windex;
        std::vector<raftpb::Entry> wents;
        uint64_t wunstable;
        Test(std::vector<raftpb::Entry> _ents,
             uint64_t _windex,
             std::vector<raftpb::Entry> _wents,
             uint64_t _wunstable)
             : ents(std::move(_ents)), windex(_windex), wents(std::move(_wents)), wunstable(_wunstable) {}
    };
    std::vector<Test> tests = {
		{
            {},
			2,
            {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done()},
			3,
		},
		{
                {EntryHelper{.index = 3,.term = 2}.Done()},
			3,
                {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done(), EntryHelper{.index = 3,.term = 2}.Done()},
			3,
		},
		// conflicts with index 1
		{
                {EntryHelper{.index = 1,.term = 2}.Done()},
			1,
            {EntryHelper{.index = 1,.term = 2}.Done()},
			1,
		},
		// conflicts with index 2
		{
                {EntryHelper{.index = 2,.term = 3}.Done(), EntryHelper{.index = 3,.term = 3}.Done()},
			3,
                {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 3}.Done(), EntryHelper{.index = 3,.term = 3}.Done()},
			2,
		}
	};

    for (auto& tt : tests) {
		auto storage = NewMemoryStorage();
		storage->Append(previousEnts);
		auto raftLog = detail::newLog(storage);

		auto index = raftLog->append(tt.ents);
        EXPECT_EQ(index, tt.windex);
        std::vector<raftpb::Entry> g;
        Error err;
		std::tie(g, err) = raftLog->entries(1, noLimit);
        EXPECT_EQ(err, nullptr);
        EXPECT_TRUE(VectorEquals(g, tt.wents));
        EXPECT_EQ(raftLog->unstable_.offset_, tt.wunstable);
	}
}

// TestLogMaybeAppend ensures:
// If the given (index, term) matches with the existing log:
// 	1. If an existing entry conflicts with a new one (same index
// 	but different terms), delete the existing entry and all that
// 	follow it
// 	2.Append any new entries not already in the log
// If the given (index, term) does not match with the existing log:
// 	return false
TEST(log, TestLogMaybeAppend) {
    std::vector<raftpb::Entry> previousEnts = {EntryHelper{.index = 1, .term = 1}.Done(),
                                               EntryHelper{.index = 2, .term = 2}.Done(),
                                               EntryHelper{.index = 3, .term = 3}.Done()};
	uint64_t lastindex = 3;
	uint64_t lastterm = 3;
	uint64_t commit = 1;
    struct Test {
        uint64_t logTerm;
        uint64_t index;
        uint64_t committed;
        std::vector<raftpb::Entry> ents;

        uint64_t wlasti;
        bool wappend;
        uint64_t wcommit;
        bool wpanic;
    };
    std::vector<Test> tests = {
		// not match: term is different
		{
			lastterm - 1, lastindex, lastindex, {EntryHelper{.index = lastindex + 1, .term = 4}.Done()},
			0, false, commit, false,
		},
		// not match: index out of bound
		{
			lastterm, lastindex + 1, lastindex, {EntryHelper{.index = lastindex + 2, .term = 4}.Done()},
			0, false, commit, false,
		},
		// match with the last existing entry
		{
			lastterm, lastindex, lastindex, {},
			lastindex, true, lastindex, false,
		},
		{
			lastterm, lastindex, lastindex + 1, {},
			lastindex, true, lastindex, false, // do not increase commit higher than lastnewi
		},
		{
			lastterm, lastindex, lastindex - 1, {},
			lastindex, true, lastindex - 1, false, // commit up to the commit in the message
		},
		{
			lastterm, lastindex, 0, {},
			lastindex, true, commit, false, // commit do not decrease
		},
		{
			0, 0, lastindex, {},
			0, true, commit, false, // commit do not decrease
		},
		{
			lastterm, lastindex, lastindex, {EntryHelper{.index = lastindex + 1, .term = 4}.Done()},
			lastindex + 1, true, lastindex, false,
		},
		{
			lastterm, lastindex, lastindex + 1, {EntryHelper{.index = lastindex + 1, .term = 4}.Done()},
			lastindex + 1, true, lastindex + 1, false,
		},
		{
			lastterm, lastindex, lastindex + 2, {EntryHelper{.index = lastindex + 1, .term = 4}.Done()},
			lastindex + 1, true, lastindex + 1, false, // do not increase commit higher than lastnewi
		},
		{
			lastterm, lastindex, lastindex + 2, {EntryHelper{.index = lastindex + 1, .term = 4}.Done(), EntryHelper{.index = lastindex + 2, .term = 4}.Done()},
			lastindex + 2, true, lastindex + 2, false,
		},
		// match with the entry in the middle
		{
			lastterm - 1, lastindex - 1, lastindex, {EntryHelper{.index = lastindex, .term = 4}.Done()},
			lastindex, true, lastindex, false,
		},
		{
			lastterm - 2, lastindex - 2, lastindex, {EntryHelper{.index = lastindex-1, .term = 4}.Done()},
			lastindex - 1, true, lastindex - 1, false,
		},
		{
			lastterm - 3, lastindex - 3, lastindex, {EntryHelper{.index = lastindex-2, .term = 4}.Done()},
			lastindex - 2, true, lastindex - 2, true, // conflict with existing committed entry
		},
		{
			lastterm - 2, lastindex - 2, lastindex, {EntryHelper{.index = lastindex-1, .term = 4}.Done(),EntryHelper{.index = lastindex, .term = 4}.Done()},
			lastindex, true, lastindex, false,
		},
	};
    for (auto& tt : tests) {
		auto raftLog = detail::newLog(NewMemoryStorage());
		raftLog->append(previousEnts);
		raftLog->committed_ = commit;
		try {
            uint64_t glasti;
            bool gappend;
			std::tie(glasti, gappend) = raftLog->maybeAppend(tt.index, tt.logTerm, tt.committed, tt.ents);
            EXPECT_EQ(glasti, tt.wlasti);
            EXPECT_EQ(gappend, tt.wappend);
            EXPECT_EQ(raftLog->committed_, tt.wcommit);
			if (gappend && !tt.ents.empty()) {
                std::vector<raftpb::Entry>gents;
                Error err;
				std::tie(gents, err) = raftLog->slice(raftLog->lastIndex()-tt.ents.size()+1, raftLog->lastIndex()+1, noLimit);
                EXPECT_EQ(err, nullptr);
                EXPECT_TRUE(VectorEquals(tt.ents, gents));
			}
		} catch (std::exception& e) {
            EXPECT_EQ(typeid(e), typeid(PanicException));
            EXPECT_TRUE(tt.wpanic);
        }
	}
}

// TestCompactionSideEffects ensures that all the log related functionality works correctly after
// a compaction.
TEST(log, TestCompactionSideEffects) {
	uint64_t i = 0;
	// Populate the log with 1000 entries; 750 in stable storage and 250 in unstable.
    uint64_t lastIndex = 1000;
    uint64_t unstableIndex = 750;
	auto lastTerm = lastIndex;
	auto storage = NewMemoryStorage();
	for (i = 1; i <= unstableIndex; i++) {
		storage->Append({EntryHelper{.index = i,.term = i}.Done()});
	}
	auto raftLog = detail::newLog(storage);
	for (i = unstableIndex; i < lastIndex; i++) {
		raftLog->append({EntryHelper{.index = i+1, .term = i+1}.Done()});
	}

	auto ok = raftLog->maybeCommit(lastIndex, lastTerm);
    EXPECT_TRUE(ok);
	raftLog->appliedTo(raftLog->committed_, 0);

	uint64_t offset = 500;
	storage->Compact(offset);

    EXPECT_EQ(raftLog->lastIndex(), lastIndex);
    for (uint64_t j = offset; j <= raftLog->lastIndex(); j++) {
        EXPECT_EQ(mustTerm(raftLog->term(j)), j);
	}

	for (uint64_t j = offset; j <= raftLog->lastIndex(); j++) {
        EXPECT_TRUE(raftLog->matchTerm(j, j));
	}

	auto unstableEnts = raftLog->nextUnstableEnts();
    EXPECT_EQ(unstableEnts.size(), 250);
    EXPECT_EQ(unstableEnts[0].index(), 751);

	auto prev = raftLog->lastIndex();
	raftLog->append({EntryHelper{.index = raftLog->lastIndex() + 1, .term = raftLog->lastIndex() + 1}.Done()});
    EXPECT_EQ(raftLog->lastIndex(), prev+1);

    std::vector<raftpb::Entry> ents;
    Error err;
	std::tie(ents, err) = raftLog->entries(raftLog->lastIndex(), noLimit);
    EXPECT_EQ(err, nullptr);
    EXPECT_EQ(ents.size(), 1);
}

TEST(log, TestHasNextCommittedEnts) {
    auto snap = SnapshotHelper{.index = 3, .term = 1}.Done();
	std::vector<raftpb::Entry> ents = {EntryHelper{.index = 4,.term = 1}.Done(),
                                       EntryHelper{.index = 5,.term = 1}.Done(),
                                       EntryHelper{.index = 6,.term = 1}.Done()};
	struct Test {
        uint64_t applied;
        uint64_t applying;
        bool allowUnstable;
        bool paused;
        bool snap;
        bool whasNext;
    };
    std::vector<Test> tests = {
		{3, 3, true, false, false, true},
		{3, 4, true, false, false, true},
		{3, 5, true, false, false, false},
		{4, 4, true, false, false, true},
		{4, 5, true, false, false, false},
		{5, 5, true, false, false, false},
		// Don't allow unstable entries.
		{3, 3, false, false, false, true},
		{3, 4, false, false, false, false},
		{3, 5, false, false, false, false},
		{4, 4, false, false, false, false},
		{4, 5, false, false, false, false},
		{5, 5, false, false, false, false},
		// Paused.
		{3, 3, true, true, false, false},
		// With snapshot.
		{3, 3, true, false, true, false},
	};
    for (auto& tt : tests) {
        auto storage = NewMemoryStorage();
        EXPECT_EQ(storage->ApplySnapshot(snap), nullptr);
        EXPECT_EQ(storage->Append({ents.begin(), ents.begin()+1}), nullptr);

        auto raftLog = detail::newLog(storage);
        raftLog->append(ents);
        raftLog->stableTo(4, 1);
        raftLog->maybeCommit(5, 1);
        raftLog->appliedTo(tt.applied, 0);
        raftLog->acceptApplying(tt.applying, 0, tt.allowUnstable);
        raftLog->applyingEntsPaused_ = tt.paused;
        if (tt.snap) {
            auto newSnap = snap;
            newSnap.mutable_metadata()->set_index(newSnap.metadata().index()+1);
            raftLog->restore(newSnap);
        }
        EXPECT_EQ(tt.whasNext, raftLog->hasNextCommittedEnts(tt.allowUnstable));
	}
}

TEST(log, TestNextCommittedEnts) {
    auto snap = SnapshotHelper{.index = 3,.term = 1}.Done();
    std::vector<raftpb::Entry> ents = {EntryHelper{.index = 4,.term = 1}.Done(),
                                       EntryHelper{.index = 5,.term = 1}.Done(),
                                       EntryHelper{.index = 6,.term = 1}.Done()};
    struct Test {
        uint64_t applied;
        uint64_t applying;
        bool allowUnstable;
        bool paused;
        bool snap;
        std::vector<raftpb::Entry> wents;
    };
    std::vector<Test> tests = {
		{3, 3, true, false, false, {ents.begin(), ents.begin()+2}},
		{3, 4, true, false, false, {ents.begin()+1, ents.begin()+2}},
		{3, 5, true, false, false, {}},
		{4, 4, true, false, false, {ents.begin()+1, ents.begin()+2}},
		{4, 5, true, false, false, {}},
		{5, 5, true, false, false, {}},
		// Don't allow unstable entries.
		{3, 3, false, false, false, {ents.begin(), ents.begin()+1}},
		{3, 4, false, false, false, {}},
		{3, 5, false, false, false, {}},
		{4, 4, false, false, false, {}},
		{4, 5, false, false, false, {}},
		{5, 5, false, false, false, {}},
		// Paused.
		{3, 3, true, true, false, {}},
		// With snapshot.
		{3, 3, true, false, true, {}},
	};
    for (auto& tt : tests) {
        auto storage = NewMemoryStorage();
        EXPECT_EQ(storage->ApplySnapshot(snap), nullptr);
        EXPECT_EQ(storage->Append({ents.begin(), ents.begin()+1}), nullptr);

        auto raftLog = detail::newLog(storage);
        raftLog->append(ents);
        raftLog->stableTo(4, 1);
        raftLog->maybeCommit(5, 1);
        raftLog->appliedTo(tt.applied, 0 /* size */);
        raftLog->acceptApplying(tt.applying, 0 /* size */, tt.allowUnstable);
        raftLog->applyingEntsPaused_ = tt.paused;
        if (tt.snap) {
            auto newSnap = snap;
            newSnap.mutable_metadata()->set_index(newSnap.metadata().index()+1);
            raftLog->restore(newSnap);
        }
        EXPECT_TRUE(VectorEquals(tt.wents, raftLog->nextCommittedEnts(tt.allowUnstable)));
	}
}

TEST(log, TestAcceptApplying) {
	uint64_t maxSize = 100;
    auto snap = SnapshotHelper{.index = 3,.term = 1}.Done();
    std::vector<raftpb::Entry> ents = {EntryHelper{.index = 4,.term = 1}.Done(),
                                       EntryHelper{.index = 5, .term = 1}.Done(),
                                       EntryHelper{.index = 6,.term = 1}.Done()};
    struct Test {
        uint64_t index;
        bool allowUnstable;
        uint64_t size;
        bool wpaused;
    };
    std::vector<Test> tests = {
		{3, true, maxSize - 1, true},
		{3, true, maxSize, true},
		{3, true, maxSize + 1, true},
		{4, true, maxSize - 1, true},
		{4, true, maxSize, true},
		{4, true, maxSize + 1, true},
		{5, true, maxSize - 1, false},
		{5, true, maxSize, true},
		{5, true, maxSize + 1, true},
		// Don't allow unstable entries.
		{3, false, maxSize - 1, true},
		{3, false, maxSize, true},
		{3, false, maxSize + 1, true},
		{4, false, maxSize - 1, false},
		{4, false, maxSize, true},
		{4, false, maxSize + 1, true},
		{5, false, maxSize - 1, false},
		{5, false, maxSize, true},
		{5, false, maxSize + 1, true},
	};
    for (auto& tt : tests) {
        auto storage = NewMemoryStorage();
        EXPECT_EQ(storage->ApplySnapshot(snap), nullptr);
        EXPECT_EQ(storage->Append({ents.begin(), ents.begin()+1}), nullptr);

        auto raftLog = detail::newLogWithSize(storage, maxSize);
        raftLog->append(ents);
        raftLog->stableTo(4, 1);
        raftLog->maybeCommit(5, 1);
        raftLog->appliedTo(3, 0 /* size */);

        raftLog->acceptApplying(tt.index, tt.size, tt.allowUnstable);
        EXPECT_EQ(tt.wpaused, raftLog->applyingEntsPaused_);
	}
}

TEST(log, TestAppliedTo) {
	uint64_t maxSize = 100;
	uint64_t overshoot = 5;
    auto snap = SnapshotHelper{.index = 3,.term = 1}.Done();
    std::vector<raftpb::Entry> ents = {EntryHelper{.index = 4, .term = 1}.Done(),
                                       EntryHelper{.index = 5,.term = 1}.Done(),
                                       EntryHelper{.index = 6,.term = 1}.Done()};
    struct Test {
        uint64_t index;
        uint64_t size;
        uint64_t wapplyingSize;
        bool wpaused;
    };
    std::vector<Test> tests = {
		// Apply some of in-progress entries (applying = 5 below).
		{4, overshoot - 1, maxSize + 1, true},
		{4, overshoot, maxSize, true},
		{4, overshoot + 1, maxSize - 1, false},
		// Apply all of in-progress entries.
		{5, overshoot - 1, maxSize + 1, true},
		{5, overshoot, maxSize, true},
		{5, overshoot + 1, maxSize - 1, false},
		// Apply all of outstanding bytes.
		{4, maxSize + overshoot, 0, false},
		// Apply more than outstanding bytes.
		// Incorrect accounting doesn't underflow applyingSize.
		{4, maxSize + overshoot + 1, 0, false},
	};
    for (auto& tt : tests) {
        auto storage = NewMemoryStorage();
        EXPECT_EQ(storage->ApplySnapshot(snap), nullptr);
        EXPECT_EQ(storage->Append({ents.begin(), ents.begin()+1}), nullptr);

        auto raftLog = detail::newLogWithSize(storage, maxSize);
        raftLog->append(ents);
        raftLog->stableTo(4, 1);
        raftLog->maybeCommit(5, 1);
        raftLog->appliedTo(3, 0 /* size */);
        raftLog->acceptApplying(5, maxSize+overshoot, false /* allowUnstable */);

        raftLog->appliedTo(tt.index, tt.size);
        EXPECT_EQ(tt.index, raftLog->applied_);
        EXPECT_EQ(5, raftLog->applying_);
        EXPECT_EQ(tt.wapplyingSize, raftLog->applyingEntsSize_);
        EXPECT_EQ(tt.wpaused, raftLog->applyingEntsPaused_);
	}
}

// TestNextUnstableEnts ensures unstableEntries returns the unstable part of the
// entries correctly.
TEST(log, TestNextUnstableEnts) {
    std::vector<raftpb::Entry> previousEnts = {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done()};
    struct Test {
        uint64_t unstable;
        std::vector<raftpb::Entry> wents;
    };
    std::vector<Test> tests = {
		{3, {}},
		{1, previousEnts},
	};
    for (auto& tt : tests) {
        // append stable entries to storage
        auto storage = NewMemoryStorage();
        EXPECT_EQ(storage->Append({previousEnts.begin(), previousEnts.begin() + tt.unstable-1}), nullptr);

        // append unstable entries to raftlog
        auto raftLog = detail::newLog(storage);
        raftLog->append({previousEnts.begin() + tt.unstable-1, previousEnts.end()});

        auto ents = raftLog->nextUnstableEnts();
        if (!ents.empty()) {
            raftLog->stableTo(ents[ents.size()-1].index(), ents[ents.size()-1].term());
        }
        EXPECT_TRUE(VectorEquals(tt.wents, ents));
        EXPECT_EQ(previousEnts[previousEnts.size()-1].index()+1, raftLog->unstable_.offset_);
	}
}

TEST(log, TestCommitTo) {
    std::vector<raftpb::Entry> previousEnts = {
            EntryHelper{.index = 1,.term = 1}.Done(),
            EntryHelper{.index = 2,.term = 2}.Done(),
            EntryHelper{.index = 3,.term = 3}.Done()};
	uint64_t commit = 2;
    struct Test {
        uint64_t commit;
        uint64_t wcommit;
        bool wpanic;
    };
    std::vector<Test> tests = {
            {3, 3, false},
            {1, 2, false}, // never decrease
            {4, 0, true},  // commit out of range -> panic
    };
    for (auto& tt : tests) {
        try {
            auto raftLog = detail::newLog(NewMemoryStorage());
            raftLog->append(previousEnts);
            raftLog->committed_ = commit;
            raftLog->commitTo(tt.commit);
            EXPECT_EQ(raftLog->committed_, tt.wcommit);
        } catch (std::exception& e) {
            EXPECT_EQ(typeid(e), typeid(PanicException));
            EXPECT_TRUE(tt.wpanic);
        }
	}
}

TEST(log, TestStableTo) {
    struct Test {
        uint64_t stablei;
        uint64_t stablet;
        uint64_t wunstable;
    };
    std::vector<Test> tests = {
            {1, 1, 2},
            {2, 2, 3},
            {2, 1, 1}, // bad term
            {3, 1, 1}, // bad index
    };
    for (auto& tt : tests) {
		auto raftLog = detail::newLog(NewMemoryStorage());
		raftLog->append({EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done()});
		raftLog->stableTo(tt.stablei, tt.stablet);
        EXPECT_EQ(raftLog->unstable_.offset_, tt.wunstable);
	}
}

TEST(log, TestStableToWithSnap) {
	uint64_t snapi = 5;
    uint64_t snapt = 2;
    struct Test {
        uint64_t stablei;
        uint64_t stablet;
        std::vector<raftpb::Entry> newEnts;
        uint64_t wunstable;
    };
    std::vector<Test> tests = {
            {snapi + 1, snapt, {}, snapi + 1},
            {snapi, snapt, {}, snapi + 1},
            {snapi - 1, snapt, {}, snapi + 1},

            {snapi + 1, snapt + 1, {}, snapi + 1},
            {snapi, snapt + 1, {}, snapi + 1},
            {snapi - 1, snapt + 1, {}, snapi + 1},

            {snapi + 1, snapt, {EntryHelper{.index = snapi + 1, .term = snapt}.Done()}, snapi + 2},
            {snapi, snapt, {EntryHelper{.index = snapi + 1, .term = snapt}.Done()}, snapi + 1},
            {snapi - 1, snapt, {EntryHelper{.index = snapi + 1, .term = snapt}.Done()}, snapi + 1},

            {snapi + 1, snapt + 1, {EntryHelper{.index = snapi + 1, .term = snapt}.Done()}, snapi + 1},
            {snapi, snapt + 1, {EntryHelper{.index = snapi + 1, .term = snapt}.Done()}, snapi + 1},
            {snapi - 1, snapt + 1, {EntryHelper{.index = snapi + 1, .term = snapt}.Done()}, snapi + 1},
    };
    for (auto& tt : tests) {
		auto s = NewMemoryStorage();
		s->ApplySnapshot(SnapshotHelper{.index = snapi, .term = snapt}.Done());
		auto raftLog = detail::newLog(s);
		raftLog->append(tt.newEnts);
		raftLog->stableTo(tt.stablei, tt.stablet);
        EXPECT_EQ(raftLog->unstable_.offset_, tt.wunstable);
	}
}

// TestCompaction ensures that the number of log entries is correct after compactions.
TEST(log, TestCompaction) {
    struct Test {
        uint64_t lastIndex;
        std::vector<uint64_t> compact;
        std::vector<int> wleft;
        bool wallow;
    };

    std::vector<Test> tests = {
        // out of upper bound
        {1000, {1001}, {-1}, false},
        {1000, {300, 500, 800, 900}, {700, 500, 200, 100}, true},
        // out of lower bound
        {1000, {300, 299}, {700, -1}, false},
    };
    for (auto& tt : tests) {
        try {
            auto storage = NewMemoryStorage();
            for (uint64_t i = 1; i <= tt.lastIndex; i++) {
                storage->Append({EntryHelper{.index = i, .term = 0}.Done()});
            }
            auto raftLog = detail::newLog(storage);
            raftLog->maybeCommit(tt.lastIndex, 0);

            raftLog->appliedTo(raftLog->committed_, 0);
            for (uint64_t j = 0; j < tt.compact.size(); j++) {
                auto err = storage->Compact(tt.compact[j]);
                if (err != nullptr) {
                    EXPECT_FALSE(tt.wallow);
                    continue;
                }
                EXPECT_EQ(raftLog->allEntries().size(), tt.wleft[j]);
            }
        } catch (const std::exception& e) {
            EXPECT_EQ(typeid(e), typeid(PanicException));
            EXPECT_FALSE(tt.wallow);
        }
	}
}

TEST(log, TestLogRestore) {
	uint64_t index = 1000;
	uint64_t term = 1000;
	auto storage = NewMemoryStorage();
	storage->ApplySnapshot(SnapshotHelper{.index = index,.term = term}.Done());
	auto raftLog = detail::newLog(storage);

    EXPECT_EQ(raftLog->allEntries().size(), 0);
    EXPECT_EQ(raftLog->firstIndex(), index+1);
    EXPECT_EQ(raftLog->committed_, index);
    EXPECT_EQ(raftLog->unstable_.offset_, index+1);
    raftLog->term(index);
    EXPECT_EQ(mustTerm(raftLog->term(index)), term);
}

TEST(log, TestIsOutOfBounds) {
	uint64_t offset = 100;
    uint64_t num = 100;
	auto storage = NewMemoryStorage();
	storage->ApplySnapshot(SnapshotHelper{.index = offset}.Done());
	auto l = detail::newLog(storage);
	for (uint64_t i = 1; i <= num; i++) {
        l->append({EntryHelper{.index = i + offset}.Done()});
	}

	auto first = offset + 1;
    struct Test {
        uint64_t lo;
        uint64_t hi;
        bool wpanic;
        bool wErrCompacted;
    };
    std::vector<Test> tests = {
        {
                first - 2, first + 1,
                false,
                true,
        },
        {
                first - 1, first + 1,
                false,
                true,
        },
        {
                first, first,
                false,
                false,
        },
        {
                first + num/2, first + num/2,
                false,
                false,
        },
        {
                first + num - 1, first + num - 1,
                false,
                false,
        },
        {
                first + num, first + num,
                false,
                false,
        },
        {
                first + num, first + num + 1,
                true,
                false,
        },
        {
                first + num + 1, first + num + 1,
                true,
                false,
        }
    };

    for (auto& tt : tests) {
        try {
            auto err = l->mustCheckOutOfBounds(tt.lo, tt.hi);
            EXPECT_FALSE(tt.wpanic);
            EXPECT_FALSE(tt.wErrCompacted && err != ErrCompacted);
            EXPECT_FALSE(!tt.wErrCompacted && err != nullptr);
        } catch (const std::exception& e) {
            EXPECT_EQ(typeid(e), typeid(PanicException));
            EXPECT_TRUE(tt.wpanic);
        }
	}
}


TEST(log, TestTerm) {
	uint64_t offset = 100;
	uint64_t num = 100;

	auto storage = NewMemoryStorage();
	storage->ApplySnapshot(SnapshotHelper{.index = offset, .term = 1}.Done());
	auto l = detail::newLog(storage);
	for (size_t i = 1; i < num; i++) {
		l->append({EntryHelper{.index = offset + i, .term = i}.Done()});
	}
    struct Test {
        uint64_t idx;
        uint64_t term;
        Error err;
    };
    std::vector<Test> tests = {
		{offset - 1, 0, ErrCompacted},
		{offset, 1, nullptr},
		{offset + num/2, num / 2, nullptr},
		{offset + num - 1, num - 1, nullptr},
		{offset + num, 0, ErrUnavailable},
	};
    for (auto& tt : tests) {
        uint64_t term;
        Error err;
        std::tie(term, err) = l->term(tt.idx);
        EXPECT_EQ(tt.term, term);
        EXPECT_EQ(tt.err, err);
	}
}

TEST(log, TestTermWithUnstableSnapshot) {
	uint64_t storagesnapi = 100;
	uint64_t unstablesnapi = storagesnapi + 5;

	auto storage = NewMemoryStorage();
	storage->ApplySnapshot(SnapshotHelper{.index = storagesnapi, .term = 1}.Done());
	auto l = detail::newLog(storage);
	l->restore(SnapshotHelper{.index = unstablesnapi, .term = 1}.Done());
    struct Test {
        uint64_t idx;
        uint64_t term;
        Error err;
    };
    std::vector<Test> tests = {
		// cannot get term from storage
		{storagesnapi, 0, ErrCompacted},
		// cannot get term from the gap between storage ents and unstable snapshot
		{storagesnapi + 1, 0, ErrCompacted},
		{unstablesnapi - 1, 0, ErrCompacted},
		// get term from unstable snapshot index
		{unstablesnapi, 1, nullptr},
		// the log beyond the unstable snapshot is empty
		{unstablesnapi + 1, 0, ErrUnavailable},
	};
    for (auto& tt : tests) {
        uint64_t term;
        Error err;
        std::tie(term, err) = l->term(tt.idx);
        EXPECT_EQ(tt.term, term);
        EXPECT_EQ(tt.err, err);
	}
}

TEST(log, TestSlice) {
	uint64_t offset = 100;
	uint64_t num = 100;
	uint64_t last = offset + num;
	uint64_t half = offset + num/2;
    auto halfe = EntryHelper{.index = half, .term = half}.Done();

	auto entries = [](uint64_t from, uint64_t to)->std::vector<raftpb::Entry> {
        std::vector<raftpb::Entry> res;
		for (auto i = from; i < to; i++) {
            res.push_back(EntryHelper{.index = i, .term = i}.Done());
		}
		return res;
	};

	auto storage = NewMemoryStorage();
    EXPECT_EQ(storage->ApplySnapshot(SnapshotHelper{.index = offset}.Done()), nullptr);
    EXPECT_EQ(storage->Append(entries(offset+1, half)), nullptr);
	auto l = detail::newLog(storage);
	l->append(entries(half, last));

    struct Test {
        uint64_t lo = 0;
        uint64_t hi = 0;
        uint64_t lim = 0;
        std::vector<raftpb::Entry> w = {};
        bool wpanic = false;

    };
    std::vector<Test> tests = {
		// ErrCompacted.
		{offset - 1, offset + 1, noLimit},
		{offset, offset + 1, noLimit},
		// panics
		{half, half - 1, noLimit, {}, true}, // lo and hi inversion
		{last, last + 1, noLimit, {}, true}, // hi is out of bounds

		// No limit.
		{offset + 1, offset + 1, noLimit, {}},
		{offset + 1, half - 1, noLimit, entries(offset+1, half-1)},
		{offset + 1, half, noLimit, entries(offset+1, half)},
		{offset + 1, half + 1, noLimit, entries(offset+1, half+1)},
		{offset + 1, last, noLimit, entries(offset+1, last)},
		{half - 1, half, noLimit, entries(half-1, half)},
		{half - 1, half + 1, noLimit, entries(half-1, half+1)},
		{half - 1, last, noLimit, entries(half-1, last)},
		{half, half + 1, noLimit, entries(half, half+1)},
		{half, last, noLimit, entries(half, last)},
		{last - 1, last, noLimit, entries(last-1, last)},

		// At least one entry is always returned.
		{offset + 1, last, 0, entries(offset+1, offset+2)},
		{half - 1, half + 1, 0, entries(half-1, half)},
		{half, last, 0, entries(half, half+1)},
		{half + 1, last, 0, entries(half+1, half+2)},
		// Low limit.
		{offset + 1, last, halfe.ByteSizeLong() - 1, entries(offset+1, offset+2)},
		{half - 1, half + 1, halfe.ByteSizeLong() - 1, entries(half-1, half)},
		{half, last, halfe.ByteSizeLong() - 1, entries(half, half+1)},
		// Just enough for one limit.
		{offset + 1, last, halfe.ByteSizeLong(), entries(offset+1, offset+2)},
		{half - 1, half + 1, halfe.ByteSizeLong(), entries(half-1, half)},
		{half, last, halfe.ByteSizeLong(), entries(half, half+1)},
		// Not enough for two limit.
		{offset + 1, last, halfe.ByteSizeLong() + 1, entries(offset+1, offset+2)},
		{half - 1, half + 1, halfe.ByteSizeLong() + 1, entries(half-1, half)},
		{half, last, halfe.ByteSizeLong() + 1, entries(half, half+1)},
		// Enough for two limit.
        // TODO(wangzhiyong)
		//{offset + 1, last, halfe.ByteSizeLong() * 2, entries(offset+1, offset+3)},
		{half - 2, half + 1, halfe.ByteSizeLong() * 2, entries(half-2, half)},
		{half - 1, half + 1, halfe.ByteSizeLong() * 2, entries(half-1, half+1)},
		{half, last, halfe.ByteSizeLong() * 2, entries(half, half+2)},
		// Not enough for three.
		{half - 2, half + 1, halfe.ByteSizeLong()*3 - 1, entries(half-2, half)},
		// Enough for three.
		{half - 1, half + 2, halfe.ByteSizeLong() * 3, entries(half-1, half+2)},
	};
    for (auto& tt : tests) {
        std::vector<raftpb::Entry> g;
        Error err;
        try {
            std::tie(g, err) = l->slice(tt.lo, tt.hi, tt.lim);
        } catch (const std::exception& e) {
            EXPECT_EQ(typeid(e), typeid(PanicException));
            EXPECT_TRUE(tt.wpanic);
        }
        EXPECT_FALSE(tt.lo <= offset && err != ErrCompacted);
        EXPECT_FALSE(tt.lo > offset && err != nullptr);
        EXPECT_TRUE(VectorEquals(tt.w, g));
	}
}

TEST(log, TestScan) {
	uint64_t offset = 47;
	uint64_t num = 20;
	auto last = offset + num;
	auto half = offset + num/2;
	auto entries = [](uint64_t from, uint64_t to)->std::vector<raftpb::Entry> {
        std::vector<raftpb::Entry> res;
		for (auto i = from; i < to; i++) {
            res.push_back(EntryHelper{.index = i,.term = i}.Done());
		}
		return res;
	};
	auto entrySize = entsSize(entries(half, half+1));

	auto storage = NewMemoryStorage();
    EXPECT_EQ(storage->ApplySnapshot(SnapshotHelper{.index = offset}.Done()), nullptr);
    EXPECT_EQ(storage->Append(entries(offset+1, half)), nullptr);
	auto l = detail::newLog(storage);
	l->append(entries(half, last));

	// Test that scan() returns the same entries as slice(), on all inputs.
	for (auto pageSize : std::vector<uint64_t>{0, 1, 10, 100, entrySize, entrySize + 1}) {
		for (auto lo = offset + 1; lo < last; lo++) {
			for (auto hi = lo; hi <= last; hi++) {
                std::vector<raftpb::Entry> got;
                l->scan(lo, hi, pageSize, [&](const std::vector<raftpb::Entry>& e)->Error{
                    got.insert(got.end(), e.begin(), e.end());
                    EXPECT_TRUE(e.size() == 1 || entsSize(e) <= pageSize);
                    return nullptr;
                });
                std::vector<raftpb::Entry> want;
                Error err;
				std::tie(want, err) = l->slice(lo, hi, noLimit);
                EXPECT_EQ(err, nullptr);
                EXPECT_TRUE(VectorEquals(want, got));
			}
		}
	}

	// Test that the callback error is propagated to the caller.
	int iters = 0;
    EXPECT_EQ(errBreak, l->scan(offset+1, half, 0, [&](const std::vector<raftpb::Entry>&)->Error{
        iters++;
        if (iters == 2) {
            return errBreak;
        }
        return nullptr;
    }));
    EXPECT_EQ(2, iters);

	// Test that we max out the limit, and not just always return a single entry.
	// NB: this test works only because the requested range length is even.
    EXPECT_EQ(l->scan(offset+1, offset+11, entrySize*2, [&](const std::vector<raftpb::Entry>& ents)->Error{
        EXPECT_EQ(ents.size(), 2);
        EXPECT_EQ(entrySize*2, entsSize(ents));
        return nullptr;
    }), nullptr);
}

}
