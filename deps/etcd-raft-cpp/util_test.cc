#include <gtest/gtest.h>
#include "describle.h"
namespace eraft {

auto testFormatter = [](const std::string &data) {
    std::string str(data);
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
};

TEST(util, TestDescribeEntry) {
    raftpb::Entry entry;
    entry.set_term(1);
    entry.set_index(2);
    entry.set_type(raftpb::EntryNormal);
    std::string s;
    s.append("hello").append(1, 0x00).append("world");
    entry.set_data(std::move(s));

    auto defaultFormatted = DescribeEntry(entry, nullptr);
    EXPECT_EQ(defaultFormatted, "1/2 EntryNormal \"hello\\x00world\"");

    auto customFormatted = DescribeEntry(entry, testFormatter);
    EXPECT_EQ(customFormatted, "1/2 EntryNormal HELLO\x00WORLD");
}

TEST(util, TestLimitSize) {
    std::vector<raftpb::Entry> ents = {EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 5}.Done(), EntryHelper{.index = 6,.term = 6}.Done()};
    auto prefix = [&](int size)->std::vector<raftpb::Entry>{
        return {ents.begin(), ents.begin() + size};
    };
    struct Test {
        uint64_t maxSize;
        std::vector<raftpb::Entry> want;
    };
    std::vector<Test> tests = {
		{std::numeric_limits<uint64_t>::max(), prefix(ents.size())}, // all entries are returned
		// Even if maxSize is zero, the first entry should be returned.
		{0, prefix(1)},
		// Limit to 2.
		{ents[0].ByteSizeLong() + ents[1].ByteSizeLong(), prefix(2)},
		{ents[0].ByteSizeLong() + ents[1].ByteSizeLong() + ents[2].ByteSizeLong()/2, prefix(2)},
		{ents[0].ByteSizeLong() + ents[1].ByteSizeLong() + ents[2].ByteSizeLong() - 1, prefix(2)},
		// All.
		{ents[0].ByteSizeLong() + ents[1].ByteSizeLong() + ents[2].ByteSizeLong(), prefix(3)},
	};
    for (auto& tt : tests) {
        auto got = ents;
        limitSize(got, tt.maxSize);
        EXPECT_TRUE(VectorEquals(tt.want, got));
        auto size = entsSize(got);
        EXPECT_TRUE(got.size() == 1 || size <= tt.maxSize);
	}
}

TEST(util, TestIsLocalMsg) {
    struct Temp {
        raftpb::MessageType msgt;
        bool isLocal;
    };
    std::vector<Temp> tests =
    {
		{raftpb::MsgHup, true},
		{raftpb::MsgBeat, true},
		{raftpb::MsgUnreachable, true},
		{raftpb::MsgSnapStatus, true},
		{raftpb::MsgCheckQuorum, true},
		{raftpb::MsgTransferLeader, false},
		{raftpb::MsgProp, false},
		{raftpb::MsgApp, false},
		{raftpb::MsgAppResp, false},
		{raftpb::MsgVote, false},
		{raftpb::MsgVoteResp, false},
		{raftpb::MsgSnap, false},
		{raftpb::MsgHeartbeat, false},
		{raftpb::MsgHeartbeatResp, false},
		{raftpb::MsgTimeoutNow, false},
		{raftpb::MsgReadIndex, false},
		{raftpb::MsgReadIndexResp, false},
		{raftpb::MsgPreVote, false},
		{raftpb::MsgPreVoteResp, false},
        {raftpb::MsgStorageAppend, true},
        {raftpb::MsgStorageAppendResp, true},
        {raftpb::MsgStorageApply, true},
        {raftpb::MsgStorageApplyResp, true},
	};

    for (auto& tt : tests) {
        EXPECT_EQ(IsLocalMsg(tt.msgt), tt.isLocal);
    }
}

TEST(util, TestIsResponseMsg) {
    struct Test {
        raftpb::MessageType msgt;
        bool isResponse;
    };
    std::vector<Test> tests = {
		{raftpb::MsgHup, false},
		{raftpb::MsgBeat, false},
		{raftpb::MsgUnreachable, true},
		{raftpb::MsgSnapStatus, false},
		{raftpb::MsgCheckQuorum, false},
		{raftpb::MsgTransferLeader, false},
		{raftpb::MsgProp, false},
		{raftpb::MsgApp, false},
		{raftpb::MsgAppResp, true},
		{raftpb::MsgVote, false},
		{raftpb::MsgVoteResp, true},
		{raftpb::MsgSnap, false},
		{raftpb::MsgHeartbeat, false},
		{raftpb::MsgHeartbeatResp, true},
		{raftpb::MsgTimeoutNow, false},
		{raftpb::MsgReadIndex, false},
		{raftpb::MsgReadIndexResp, true},
		{raftpb::MsgPreVote, false},
		{raftpb::MsgPreVoteResp, true},
		{raftpb::MsgStorageAppend, false},
		{raftpb::MsgStorageAppendResp, true},
		{raftpb::MsgStorageApply, false},
		{raftpb::MsgStorageApplyResp, true},
	};
    for (auto& tt : tests) {
        EXPECT_EQ(IsResponseMsg(tt.msgt), tt.isResponse);
	}
}


// TestPayloadSizeOfEmptyEntry ensures that payloadSize of empty entry is always zero.
// This property is important because new leaders append an empty entry to their log,
// and we don't want this to count towards the uncommitted log quota.
TEST(util, TestPayloadSizeOfEmptyEntry) {
    raftpb::Entry e;
    EXPECT_EQ(payloadSize(e), 0);
}

}
