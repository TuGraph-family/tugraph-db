#include <gtest/gtest.h>
#include "storage.h"
namespace eraft {

TEST(storage, TestStorageTerm) {
    std::vector<raftpb::Entry> ents = {EntryHelper{.index = 3,.term = 3}.Done(), EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 5}.Done()};
    struct Test {
        uint64_t i = 0;
        Error werr;
        uint64_t wterm = 0;
        bool wpanic = false;

        Test(uint64_t arg_i, Error arg_werr, uint64_t arg_wterm, bool arg_wpanic)
            : i(arg_i), werr(std::move(arg_werr)), wterm(arg_wterm), wpanic(arg_wpanic) {}
    };

    std::vector<Test> tests = {
            {2, ErrCompacted, 0, false},
            {3, Error(nullptr), 3, false},
            {4, Error(nullptr), 4, false},
            {5, Error(nullptr), 5, false},
            {6, ErrUnavailable, 0, false},
    };
    for (auto& tt : tests) {
        MemoryStorage s(ents);
        uint64_t term;
        Error err;
        std::tie(term, err) = s.Term(tt.i);
        EXPECT_EQ(err, tt.werr);
        EXPECT_EQ(term, tt.wterm);
    }
}

TEST(storage, TestStorageEntries) {
    std::vector<raftpb::Entry> ents = {EntryHelper{.index = 3,.term = 3}.Done(), EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 5}.Done(), EntryHelper{.index = 6,.term = 6}.Done()};
    struct Test {
        uint64_t lo;
        uint64_t hi;
        uint64_t maxsize;
        Error werr;
        std::vector<raftpb::Entry> wentries;
        Test(uint64_t arg_lo, uint64_t arg_hi, uint64_t arg_maxsize, Error err, std::vector<raftpb::Entry> entries) :
                lo(arg_lo), hi(arg_hi), maxsize(arg_maxsize), werr(std::move(err)), wentries(std::move(entries)) {}
    };

    std::vector<Test> tests = {
            {2, 6, std::numeric_limits<uint64_t>::max(), ErrCompacted, {}},
            {3, 4, std::numeric_limits<uint64_t>::max(), ErrCompacted, {}},
            {4, 5, std::numeric_limits<uint64_t>::max(), Error(nullptr), {EntryHelper{.index = 4,.term = 4}.Done()}},
            {4, 6, std::numeric_limits<uint64_t>::max(), Error(nullptr), {EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 5}.Done()}},
            {4, 7, std::numeric_limits<uint64_t>::max(), Error(nullptr), {EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 5}.Done(), EntryHelper{.index = 6,.term = 6}.Done()}},
            // even if maxsize is zero, the first entry should be returned
            {4, 7, 0, Error(nullptr), {EntryHelper{.index = 4,.term = 4}.Done()}},
            // limit to 2
            {4, 7, ents[1].ByteSizeLong() + ents[2].ByteSizeLong(), Error(nullptr), {EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 5}.Done()}},
            // limit to 2
            {4, 7, ents[1].ByteSizeLong() + ents[2].ByteSizeLong() + ents[3].ByteSizeLong()/2, Error(nullptr), {EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 5}.Done()}},
            {4, 7, ents[1].ByteSizeLong() + ents[2].ByteSizeLong() + ents[3].ByteSizeLong() - 1, Error(nullptr), {EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 5}.Done()}},
            // all
            {4, 7, ents[1].ByteSizeLong() + ents[2].ByteSizeLong() + ents[3].ByteSizeLong(), Error(nullptr), {EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 5}.Done(), EntryHelper{.index = 6,.term = 6}.Done()}},
    };


    for(auto& tt : tests) {
        MemoryStorage s(ents);
        std::vector<raftpb::Entry> entries;
        Error err;
		std::tie(entries, err) = s.Entries(tt.lo, tt.hi, tt.maxsize);
        EXPECT_EQ(err, tt.werr);
        EXPECT_TRUE(VectorEquals(entries, tt.wentries));
	}
}

TEST(storage, TestStorageLastIndex) {
    std::vector<raftpb::Entry> ents = {EntryHelper{.index = 3,.term = 3}.Done(),EntryHelper{.index = 4,.term = 4}.Done(),EntryHelper{.index = 5,.term = 5}.Done()};
	MemoryStorage s(ents);
    uint64_t last;
    Error err;
	std::tie(last, err) = s.LastIndex();
    EXPECT_EQ(err, nullptr);
    EXPECT_EQ(last, 5);

	s.Append(std::vector<raftpb::Entry>{EntryHelper{.index = 6,.term = 5}.Done()});
	std::tie(last, err) = s.LastIndex();
    EXPECT_EQ(err, nullptr);
    EXPECT_EQ(last, 6);
}

TEST(storage, TestStorageFirstIndex) {
    std::vector<raftpb::Entry> ents = {EntryHelper{.index = 3,.term = 3}.Done(),EntryHelper{.index = 4,.term = 4}.Done(),EntryHelper{.index = 5,.term = 5}.Done()};
    MemoryStorage s(ents);
    uint64_t first;
    Error err;
	std::tie(first, err) = s.FirstIndex();
    EXPECT_EQ(err, nullptr);
    EXPECT_EQ(first, 4);
	s.Compact(4);
    std::tie(first, err) = s.FirstIndex();
    EXPECT_EQ(err, nullptr);
    EXPECT_EQ(first, 5);
}

TEST(storage, TestStorageCompact) {
    std::vector<raftpb::Entry> ents = {EntryHelper{.index = 3,.term = 3}.Done(),EntryHelper{.index = 4,.term = 4}.Done(),EntryHelper{.index = 5,.term = 5}.Done()};
    struct Test {
        uint64_t i = 0;
        Error werr;
        uint64_t windex = 0;
        uint64_t wterm = 0;
        int wlen = 0;
        Test(uint64_t arg_i, Error err, uint64_t index, uint64_t term, int len)
            : i(arg_i), werr(std::move(err)), windex(index), wterm(term), wlen(len) {
        }
    };
    std::vector<Test> tests = {
            {2, ErrCompacted, 3, 3, 3},
            {3, ErrCompacted, 3, 3, 3},
            {4, Error(nullptr), 4, 4, 2},
            {5, Error(nullptr), 5, 5, 1}
    };

    for (auto& tt : tests) {
        MemoryStorage s(ents);
		auto err = s.Compact(tt.i);
        EXPECT_EQ(err, tt.werr);
        EXPECT_EQ(s.ents_[0].index(), tt.windex);
        EXPECT_EQ(s.ents_[0].term(), tt.wterm);
        EXPECT_EQ(s.ents_.size(), tt.wlen);
	}
}

TEST(storage, TestStorageCreateSnapshot) {
    std::vector<raftpb::Entry> ents = {EntryHelper{.index = 3,.term = 3}.Done(),EntryHelper{.index = 4,.term = 4}.Done(),EntryHelper{.index = 5,.term = 5}.Done()};
    raftpb::ConfState cs;
    std::vector<uint64_t> voters {1,2,3};
    *cs.mutable_voters() = {voters.begin(), voters.end()};
	std::string data("data");

    struct Test {
        uint64_t i = 0;
        Error werr;
        raftpb::Snapshot wsnap;
        Test(uint64_t arg_i, Error err, raftpb::Snapshot snap)
            : i(arg_i), werr(std::move(err)), wsnap(std::move(snap)) {}
    };
    std::vector<Test> tests = {
            {4, Error(nullptr), SnapshotHelper{.data = data, .index = 4, .term = 4, .cs = cs}.Done()},
            {5, Error(nullptr), SnapshotHelper{.data = data, .index = 5, .term = 5, .cs = cs}.Done()},
    };

    for (auto& tt : tests) {
		MemoryStorage s{ents};
        raftpb::Snapshot snap;
        Error err;
		std::tie(snap, err) = s.CreateSnapshot(tt.i, &cs, data);
        EXPECT_EQ(err, tt.werr);
        EXPECT_TRUE(google::protobuf::util::MessageDifferencer::Equals(snap, tt.wsnap));
	}
}

TEST(storage, TestStorageAppend) {
    std::vector<raftpb::Entry> ents = {EntryHelper{.index = 3,.term = 3}.Done(),EntryHelper{.index = 4,.term = 4}.Done(),EntryHelper{.index = 5,.term = 5}.Done()};
    struct Test {
        std::vector<raftpb::Entry> entries;
        Error werr;
        std::vector<raftpb::Entry> wentries;
        Test(std::vector<raftpb::Entry> _entries, Error _werr, std::vector<raftpb::Entry> _wentries)
            : entries(std::move(_entries)), werr(std::move(_werr)), wentries(std::move(_wentries)) {}
    };
    std::vector<Test> tests = {
            {
                    {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done()},
                    Error(nullptr),
                    {EntryHelper{.index = 3,.term = 3}.Done(), EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 5}.Done()}
            },
            {
                    {EntryHelper{.index = 3,.term = 3}.Done(), EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 5}.Done()},
                    Error(nullptr),
                    {EntryHelper{.index = 3,.term = 3}.Done(), EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 5}.Done()}
            },
            {
                    {EntryHelper{.index = 3,.term = 3}.Done(), EntryHelper{.index = 4,.term = 6}.Done(), EntryHelper{.index = 5,.term = 6}.Done()},
                    Error(nullptr),
                    {EntryHelper{.index = 3,.term = 3}.Done(), EntryHelper{.index = 4,.term = 6}.Done(), EntryHelper{.index = 5,.term = 6}.Done()}
            },
            {
                    {EntryHelper{.index = 3,.term = 3}.Done(), EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 5}.Done(), EntryHelper{.index = 6,.term = 5}.Done()},
                    Error(nullptr),
                    {EntryHelper{.index = 3,.term = 3}.Done(), EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 5}.Done(), EntryHelper{.index = 6,.term = 5}.Done()}
            },
            // Truncate incoming entries, truncate the existing entries and append.
            {
                    {EntryHelper{.index = 2,.term = 3}.Done(), EntryHelper{.index = 3,.term = 3}.Done(), EntryHelper{.index = 4,.term = 5}.Done()},
                    Error(nullptr),
                    {EntryHelper{.index = 3,.term = 3}.Done(), EntryHelper{.index = 4,.term = 5}.Done()}
            },
            // Truncate the existing entries and append.
            {
                    {EntryHelper{.index = 4,.term = 5}.Done()},
                    Error(nullptr),
                    {EntryHelper{.index = 3,.term = 3}.Done(), EntryHelper{.index = 4,.term = 5}.Done()}
            },
            // Direct append.
            {
                    {EntryHelper{.index = 6,.term = 5}.Done()},
                    Error(nullptr),
                    {EntryHelper{.index = 3,.term = 3}.Done(),EntryHelper{.index = 4,.term = 4}.Done(),EntryHelper{.index = 5,.term = 5}.Done(),EntryHelper{.index = 6,.term = 5}.Done()}
            }
    };

    for (auto& tt : tests) {
		MemoryStorage s(ents);
		auto err = s.Append(tt.entries);
        EXPECT_EQ(err, tt.werr);
        EXPECT_TRUE(VectorEquals(s.ents_, tt.wentries));
	}
}

TEST(storage, TestStorageApplySnapshot) {
    raftpb::ConfState cs;
    *cs.mutable_voters()->Add() = 1;
    *cs.mutable_voters()->Add() = 2;
    *cs.mutable_voters()->Add() = 3;


    std::vector<raftpb::Snapshot> tests;
    tests.emplace_back();
    tests.back().set_data("data");
    tests.back().mutable_metadata()->set_index(4);
    tests.back().mutable_metadata()->set_term(4);
    tests.back().mutable_metadata()->mutable_conf_state()->CopyFrom(cs);

    tests.emplace_back();
    tests.back().set_data("data");
    tests.back().mutable_metadata()->set_index(3);
    tests.back().mutable_metadata()->set_term(3);
    tests.back().mutable_metadata()->mutable_conf_state()->CopyFrom(cs);

	auto s = NewMemoryStorage();

	auto err = s->ApplySnapshot(tests[0]);
    EXPECT_EQ(err, nullptr);

	// ApplySnapshot fails due to ErrSnapOutOfDate.
	err = s->ApplySnapshot(tests[1]);
    EXPECT_EQ(err, ErrSnapOutOfDate);
}


}