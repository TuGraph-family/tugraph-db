#include <gtest/gtest.h>
#include "log_unstable.h"
namespace eraft {

TEST(unstable, TestUnstableMaybeFirstIndex) {
    struct Test {
        std::vector<raftpb::Entry> entries;
        uint64_t offset;
        raftpb::Snapshot* snap;
        bool wok;
        uint64_t windex;
    };
    std::vector<Test> tests = {
            // no snapshot
            {
                {EntryHelper{.index = 5,.term = 1}.Done()}, 5, nullptr,
                false, 0
            },
            {
                {}, 0, nullptr,
                false, 0,
            },
            // has snapshot
            {
                {EntryHelper{.index = 5,.term = 1}.Done()}, 5, new raftpb::Snapshot(SnapshotHelper{.index = 4,.term = 1}.Done()),
                true, 5,
            },
            {
                {}, 5, new raftpb::Snapshot(SnapshotHelper{.index = 4,.term = 1}.Done()),
                true, 5,
            }
    };

    for (auto& tt : tests) {
        Unstable u;
        u.entries_ = tt.entries;
        u.offset_ = tt.offset;
        u.snapshot_.reset(tt.snap);
        uint64_t index;
        bool ok;
		std::tie(index, ok) = u.maybeFirstIndex();
        EXPECT_EQ(ok, tt.wok);
        EXPECT_EQ(index, tt.windex);
	}
}

TEST(unstable, TestMaybeLastIndex) {
    struct Test {
        std::vector<raftpb::Entry> entries;
        uint64_t offset;
        raftpb::Snapshot* snap;
        bool wok;
        uint64_t windex;
    };

    std::vector<Test> tests = {
            // last in entries
            {
                    {EntryHelper{.index = 5,.term = 1}.Done()}, 5, nullptr,
                    true, 5
            },
            {
                    {EntryHelper{.index = 5,.term = 1}.Done()}, 5, new raftpb::Snapshot(SnapshotHelper{.index = 4,.term = 1}.Done()),
                    true, 5
            },
            // last in snapshot
            {
                    {}, 5, new raftpb::Snapshot(SnapshotHelper{.index = 4,.term = 1}.Done()),
                    true, 4
            },
            // empty unstable
            {
                    {}, 0, nullptr,
                    false, 0
            }
    };

    for (auto& tt : tests) {
        Unstable u;
        u.entries_ = std::move(tt.entries);
        u.offset_ = tt.offset;
        u.snapshot_.reset(tt.snap);
        uint64_t index;
        bool ok;
		std::tie(index, ok) = u.maybeLastIndex();
        EXPECT_EQ(ok, tt.wok);
        EXPECT_EQ(index, tt.windex);
	}
}

TEST(unstable, TestUnstableMaybeTerm) {
    struct Test {
        std::vector<raftpb::Entry> entries;
        uint64_t offset;
        raftpb::Snapshot* snap;
        uint64_t index;
        bool wok;
        uint64_t wterm;
    };

    std::vector<Test> tests = {
		// term from entries
		{
                {EntryHelper{.index = 5,.term = 1}.Done()}, 5, nullptr,
			5,
			true, 1,
		},
		{
                {EntryHelper{.index = 5,.term = 1}.Done()}, 5, nullptr,
			6,
			false, 0,
		},
		{
                {EntryHelper{.index = 5,.term = 1}.Done()}, 5, nullptr,
			4,
			false, 0,
		},
		{
                {EntryHelper{.index = 5,.term = 1}.Done()}, 5, new raftpb::Snapshot(SnapshotHelper{.index = 4,.term = 1}.Done()),
			5,
			true, 1,
		},
		{
                {EntryHelper{.index = 5,.term = 1}.Done()}, 5, new raftpb::Snapshot(SnapshotHelper{.index = 4,.term = 1}.Done()),
			6,
			false, 0,
		},
		// term from snapshot
		{
                {EntryHelper{.index = 5,.term = 1}.Done()}, 5, new raftpb::Snapshot(SnapshotHelper{.index = 4,.term = 1}.Done()),
			4,
			true, 1,
		},
		{
                {EntryHelper{.index = 5,.term = 1}.Done()}, 5, new raftpb::Snapshot(SnapshotHelper{.index = 4,.term = 1}.Done()),
			3,
			false, 0,
		},
		{
                {}, 5, new raftpb::Snapshot(SnapshotHelper{.index = 4,.term = 1}.Done()),
			5,
			false, 0,
		},
		{
                {}, 5, new raftpb::Snapshot(SnapshotHelper{.index = 4,.term = 1}.Done()),
			4,
			true, 1,
		},
		{
                {}, 0, nullptr,
			5,
			false, 0,
		}
	};

    for (auto& tt : tests) {
        Unstable u;
        u.entries_ = std::move(tt.entries);
        u.offset_ = tt.offset;
        u.snapshot_.reset(tt.snap);
        uint64_t term;
        bool ok;
		std::tie(term, ok) = u.maybeTerm(tt.index);
        EXPECT_EQ(ok, tt.wok);
        EXPECT_EQ(term, tt.wterm);
	}
}

TEST(unstable, TestUnstableRestore) {
    Unstable u;
    u.entries_ = {EntryHelper{.index = 5,.term = 1}.Done()};
    u.offset_ = 5;
    u.offsetInProgress_ = 6;
    u.snapshot_ = std::make_shared<raftpb::Snapshot>(SnapshotHelper{.index = 4,.term = 1}.Done());
    u.snapshotInProgress_ = true;
    auto s = SnapshotHelper{.index = 6,.term = 2}.Done();
	u.restore(s);

	EXPECT_EQ(s.metadata().index()+1, u.offset_);
    EXPECT_EQ(s.metadata().index()+1, u.offsetInProgress_);
    EXPECT_EQ(0, u.entries_.size());
    EXPECT_TRUE(google::protobuf::util::MessageDifferencer::Equals(s, *u.snapshot_));
    EXPECT_FALSE(u.snapshotInProgress_);
}

TEST(unstable, TestUnstableNextEntries) {
    struct Test {
        std::vector<raftpb::Entry> entries;
        uint64_t offset;
        uint64_t offsetInProgress;
        std::vector<raftpb::Entry> wentries;
    };
    std::vector<Test> tests = {
		// nothing in progress
		{
			{EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6,.term = 1}.Done()}, 5, 5,
			{EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done()},
		},
		// partially in progress
		{
			{EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done()}, 5, 6,
			{EntryHelper{.index = 6, .term = 1}.Done()},
		},
		// everything in progress
		{
			{EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done()}, 5, 7,
                {}, // nil, not empty slice
		},
	};

    for (auto& tt : tests) {
        Unstable u;
        u.entries_ = tt.entries;
        u.offset_ = tt.offset;
        u.offsetInProgress_ = tt.offsetInProgress;
        auto res = u.nextEntries();
        EXPECT_TRUE(VectorEquals(tt.wentries, res));
    }
}

TEST(unstable, TestUnstableNextSnapshot) {
    auto s = std::make_shared<raftpb::Snapshot>(SnapshotHelper{.index = 4,.term = 1}.Done());
    struct Test {
        std::shared_ptr<raftpb::Snapshot> snapshot;
        bool snapshotInProgress;
        std::shared_ptr<raftpb::Snapshot> wsnapshot;
    };
    std::vector<Test> tests = {
		// snapshot not unstable
		{
                nullptr, false,
                nullptr,
		},
		// snapshot not in progress
		{
			s, false,
			s,
		},
		// snapshot in progress
		{
			s, true,
                nullptr,
		},
	};
    for(auto& tt : tests) {
        Unstable u;
        u.snapshot_ = tt.snapshot;
        u.snapshotInProgress_ = tt.snapshotInProgress;
        auto res = u.nextSnapshot();
        if (!tt.wsnapshot) {
            EXPECT_TRUE(!res);
        } else {
            EXPECT_TRUE(google::protobuf::util::MessageDifferencer::Equals(*tt.wsnapshot, *res));
        }
	}
}

TEST(unstable, TestUnstableAcceptInProgress) {
    struct Test {
        std::vector<raftpb::Entry> entries;
        std::shared_ptr<raftpb::Snapshot> snapshot;
        uint64_t offsetInProgress;
        bool snapshotInProgress;
        uint64_t woffsetInProgress;
        bool wsnapshotInProgress;
    };
    std::vector<Test> tests = {
		{
                {}, nullptr,
			5,     // no entries
			false, // snapshot not already in progress
			5, false,
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done()}, nullptr,
			5,     // entries not in progress
			false, // snapshot not already in progress
			6, false,
		},
		{
			{EntryHelper{.index = 5,.term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done()}, nullptr,
			5,     // entries not in progress
			false, // snapshot not already in progress
			7, false,
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6,.term = 1}.Done()}, nullptr,
			6,     // in-progress to the first entry
			false, // snapshot not already in progress
			7, false,
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done()}, nullptr,
			7,     // in-progress to the second entry
			false, // snapshot not already in progress
			7, false,
		},
		// with snapshot
		{
			{}, std::make_shared<raftpb::Snapshot>(SnapshotHelper{.index = 4,.term = 1}.Done()),
			5,     // no entries
			false, // snapshot not already in progress
			5, true,
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done()}, std::make_shared<raftpb::Snapshot>(SnapshotHelper{.index = 4,.term = 1}.Done()),
			5,     // entries not in progress
			false, // snapshot not already in progress
			6, true,
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done()}, std::make_shared<raftpb::Snapshot>(SnapshotHelper{.index = 4,.term = 1}.Done()),
			5,     // entries not in progress
			false, // snapshot not already in progress
			7, true,
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done()}, std::make_shared<raftpb::Snapshot>(SnapshotHelper{.index = 4,.term = 1}.Done()),
			6,     // in-progress to the first entry
			false, // snapshot not already in progress
			7, true,
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done()}, std::make_shared<raftpb::Snapshot>(SnapshotHelper{.index = 4,.term = 1}.Done()),
			7,     // in-progress to the second entry
			false, // snapshot not already in progress
			7, true,
		},
		{
			{}, std::make_shared<raftpb::Snapshot>(SnapshotHelper{.index = 4,.term = 1}.Done()),
			5,    // entries not in progress
			true, // snapshot already in progress
			5, true,
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done()}, std::make_shared<raftpb::Snapshot>(SnapshotHelper{.index = 4,.term = 1}.Done()),
			5,    // entries not in progress
			true, // snapshot already in progress
			6, true,
		},
		{
			{EntryHelper{.index = 5,.term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done()}, std::make_shared<raftpb::Snapshot>(SnapshotHelper{.index = 4,.term = 1}.Done()),
			5,    // entries not in progress
			true, // snapshot already in progress
			7, true,
		},
		{
			{EntryHelper{.index = 5,.term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done()}, std::make_shared<raftpb::Snapshot>(SnapshotHelper{.index = 4,.term = 1}.Done()),
			6,    // in-progress to the first entry
			true, // snapshot already in progress
			7, true,
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done()}, std::make_shared<raftpb::Snapshot>(SnapshotHelper{.index = 4,.term = 1}.Done()),
			7,    // in-progress to the second entry
			true, // snapshot already in progress
			7, true,
		},
	};
    for (auto& tt : tests) {
        Unstable u;
        u.entries_ = tt.entries;
        u.snapshot_ = tt.snapshot;
        u.offsetInProgress_ = tt.offsetInProgress;
        u.snapshotInProgress_ = tt.snapshotInProgress;
        u.acceptInProgress();
        EXPECT_EQ(tt.woffsetInProgress, u.offsetInProgress_);
        EXPECT_EQ(tt.wsnapshotInProgress, u.snapshotInProgress_);
	}
}

TEST(unstable, TestUnstableStableTo) {
    struct Test {
        std::vector<raftpb::Entry> entries;
        uint64_t offset;
        uint64_t offsetInProgress;
        std::shared_ptr<raftpb::Snapshot> snap;
        uint64_t index;
        uint64_t term;
        uint64_t woffset;
        uint64_t woffsetInProgress;
        int wlen;
    };
    std::vector<Test> tests = {
		{
			{}, 0, 0, nullptr,
			5, 1,
			0, 0, 0,
		},
		{
			{EntryHelper{.index = 5,.term = 1}.Done()}, 5, 6, nullptr,
			5, 1, // stable to the first entry
			6, 6, 0,
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done()}, 5, 6, nullptr,
			5, 1, // stable to the first entry
			6, 6, 1,
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6,.term = 1}.Done()}, 5, 7, nullptr,
			5, 1, // stable to the first entry and in-progress ahead
			6, 7, 1,
		},
		{
			{EntryHelper{.index = 6, .term = 2}.Done()}, 6, 7, nullptr,
			6, 1, // stable to the first entry and term mismatch
			6, 7, 1,
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done()}, 5, 6, nullptr,
			4, 1, // stable to old entry
			5, 6, 1,
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done()}, 5, 6, nullptr,
			4, 2, // stable to old entry
			5, 6, 1,
		},
		// with snapshot
		{
			{EntryHelper{.index = 5, .term = 1}.Done()}, 5, 6, std::make_shared<raftpb::Snapshot>(SnapshotHelper{.index = 4,.term = 1}.Done()),
			5, 1, // stable to the first entry
			6, 6, 0,
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done()}, 5, 6, std::make_shared<raftpb::Snapshot>(SnapshotHelper{.index = 4,.term = 1}.Done()),
			5, 1, // stable to the first entry
			6, 6, 1,
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done()}, 5, 7, std::make_shared<raftpb::Snapshot>(SnapshotHelper{.index = 4,.term = 1}.Done()),
			5, 1, // stable to the first entry and in-progress ahead
			6, 7, 1,
		},
		{
			{EntryHelper{.index = 6, .term = 2}.Done()}, 6, 7, std::make_shared<raftpb::Snapshot>(SnapshotHelper{.index = 5,.term = 1}.Done()),
			6, 1, // stable to the first entry and term mismatch
			6, 7, 1,
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done()}, 5, 6, std::make_shared<raftpb::Snapshot>(SnapshotHelper{.index = 4,.term = 1}.Done()),
			4, 1, // stable to snapshot
			5, 6, 1,
		},
		{
			{EntryHelper{.index = 5, .term = 2}.Done()}, 5, 6, std::make_shared<raftpb::Snapshot>(SnapshotHelper{.index = 4,.term = 2}.Done()),
			4, 1, // stable to old entry
			5, 6, 1,
		},
	};
    for (auto& tt : tests) {
        Unstable u;
        u.entries_ = tt.entries;
        u.offset_ = tt.offset;
        u.offsetInProgress_ = tt.offsetInProgress;
        u.snapshot_ = tt.snap;
        u.stableTo(tt.index, tt.term);
        EXPECT_EQ(tt.woffset, u.offset_);
        EXPECT_EQ(tt.woffsetInProgress, u.offsetInProgress_);
        EXPECT_EQ(tt.wlen, u.entries_.size());
	}
}

TEST(unstable, TestUnstableTruncateAndAppend) {
    struct Test {
        std::vector<raftpb::Entry> entries;
        uint64_t offset;
        uint64_t offsetInProgress;
        std::shared_ptr<raftpb::Snapshot> snap;
        std::vector<raftpb::Entry> toappend;
        uint64_t woffset;
        uint64_t woffsetInProgress;
        std::vector<raftpb::Entry> wentries;
    };
    std::vector<Test> tests = {
		// append to the end
		{
			{EntryHelper{.index = 5, .term = 1}.Done()}, 5, 5, nullptr,
			{EntryHelper{.index = 6, .term = 1}.Done(), EntryHelper{.index = 7, .term = 1}.Done()},
			5, 5, {EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done(), EntryHelper{.index = 7, .term = 1}.Done()},
		},
		{
			{EntryHelper{.index = 5,.term = 1}.Done()}, 5, 6, nullptr,
			{EntryHelper{.index = 6, .term = 1}.Done(), EntryHelper{.index = 7, .term = 1}.Done()},
			5, 6, {EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done(), EntryHelper{.index = 7, .term = 1}.Done()},
		},
		// replace the unstable entries
		{
			{EntryHelper{.index = 5, .term = 1}.Done()}, 5, 5, nullptr,
			{EntryHelper{.index = 5, .term = 2}.Done(), EntryHelper{.index = 6, .term = 2}.Done()},
			5, 5, {EntryHelper{.index = 5, .term = 2}.Done(), EntryHelper{.index = 6, .term = 2}.Done()},
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done()}, 5, 5, nullptr,
			{EntryHelper{.index = 4, .term = 2}.Done(), EntryHelper{.index = 5, .term = 2}.Done(), EntryHelper{.index = 6, .term = 2}.Done()},
			4, 4, {EntryHelper{.index = 4, .term = 2}.Done(), EntryHelper{.index = 5, .term = 2}.Done(), EntryHelper{.index = 6, .term = 2}.Done()},
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done()}, 5, 6, nullptr,
			{EntryHelper{.index = 5, .term = 2}.Done(), EntryHelper{.index = 6, .term = 2}.Done()},
			5, 5, {EntryHelper{.index = 5, .term = 2}.Done(), EntryHelper{.index = 6, .term = 2}.Done()},
		},
		// truncate the existing entries and append
		{
			{EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done(), EntryHelper{.index = 7, .term = 1}.Done()}, 5, 5, nullptr,
			{EntryHelper{.index = 6, .term = 2}.Done()},
			5, 5, {EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6, .term = 2}.Done()},
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done(), EntryHelper{.index = 7, .term = 1}.Done()}, 5, 5, nullptr,
			{EntryHelper{.index = 7, .term = 2}.Done(), EntryHelper{.index = 8, .term = 2}.Done()},
			5, 5, {EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done(), EntryHelper{.index = 7, .term = 2}.Done(), EntryHelper{.index = 8, .term = 2}.Done()},
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done(), EntryHelper{.index = 7, .term = 1}.Done()}, 5, 6, nullptr,
			{EntryHelper{.index = 6, .term = 2}.Done()},
			5, 6, {EntryHelper{.index = 5,.term = 1}.Done(), EntryHelper{.index = 6, .term = 2}.Done()},
		},
		{
			{EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6, .term = 1}.Done(), EntryHelper{.index = 7, .term = 1}.Done()}, 5, 7, nullptr,
			{EntryHelper{.index = 6, .term = 2}.Done()},
			5, 6, {EntryHelper{.index = 5, .term = 1}.Done(), EntryHelper{.index = 6, .term = 2}.Done()},
		},
	};
    for (auto& tt : tests) {
        Unstable u;
        u.entries_ = tt.entries;
        u.offset_ = tt.offset;
        u.offsetInProgress_ = tt.offsetInProgress;
        u.snapshot_ = tt.snap;
        u.truncateAndAppend(tt.toappend);
        EXPECT_EQ(tt.woffset, u.offset_);
        EXPECT_EQ(tt.woffsetInProgress, u.offsetInProgress_);
        EXPECT_TRUE(VectorEquals(tt.wentries, u.entries_));
	}
}

}