#include "raft.h"
#include <gtest/gtest.h>

namespace eraft {
using namespace detail;

using testMemoryStorageOptions = std::function<void(std::shared_ptr<MemoryStorage>&)>;
extern std::shared_ptr<MemoryStorage> newTestMemoryStorage(const std::vector<testMemoryStorageOptions>& opts);
extern testMemoryStorageOptions withPeers(const std::vector<uint64_t>& peers);
extern std::shared_ptr<raft> newTestRaft(uint64_t id, int election, int heartbeat, std::shared_ptr<Storage> storage);
extern std::vector<raftpb::Message> readMessages(const std::shared_ptr<raft>& r);

// TestMsgAppFlowControlFull ensures:
// 1. msgApp can fill the sending window until full
// 2. when the window is full, no more msgApp can be sent.

TEST(raft, TestMsgAppFlowControlFull) {
	auto r = newTestRaft(1, 5, 1, newTestMemoryStorage({withPeers({1, 2})}));
	r->becomeCandidate();
	r->becomeLeader();

	auto& pr2 = r->trk_.progress_[2];
	// force the progress to be in replicate state
	pr2->BecomeReplicate();
	// fill in the inflights window
    for (size_t i = 0; i < r->trk_.maxInflight_; i++) {
		r->Step(MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp,.entries = {EntryHelper{.data = "somedata"}.Done()}}.Done());
		auto ms = readMessages(r);
        EXPECT_FALSE(ms.size() != 1 || ms[0].type() != raftpb::MsgApp);
	}

	// ensure 1
    EXPECT_TRUE(pr2->IsPaused());

	// ensure 2
    for (size_t i = 0; i < 10; i++) {
		r->Step(MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "somedata"}.Done()}}.Done());
		auto ms = readMessages(r);
        EXPECT_EQ(ms.size(), 0);
	}
}

// TestMsgAppFlowControlMoveForward ensures msgAppResp can move
// forward the sending window correctly:
// 1. valid msgAppResp.index moves the windows to pass all smaller or equal index.
// 2. out-of-dated msgAppResp has no effect on the sliding window.
TEST(raft, TestMsgAppFlowControlMoveForward) {
	auto r = newTestRaft(1, 5, 1, newTestMemoryStorage({withPeers({1, 2})}));
	r->becomeCandidate();
	r->becomeLeader();

	auto& pr2 = r->trk_.progress_[2];
	// force the progress to be in replicate state
	pr2->BecomeReplicate();
	// fill in the inflights window
    for (size_t i = 0; i < r->trk_.maxInflight_; i++) {
        r->Step(MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "somedata"}.Done()}}.Done());
		readMessages(r);
	}

	// 1 is noop, 2 is the first proposal we just sent.
	// so we start with 2.
    for (size_t tt = 2; tt < r->trk_.maxInflight_; tt++) {
		// move forward the window
        raftpb::Message m;
        m.set_from(2);
        m.set_to(1);
        m.set_type(raftpb::MsgAppResp);
        m.set_index(tt);
		r->Step(m);
		readMessages(r);

		// fill in the inflights window again
        r->Step(MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "somedata"}.Done()}}.Done());
		auto ms = readMessages(r);
        EXPECT_FALSE(ms.size() != 1 || ms[0].type() != raftpb::MsgApp);

		// ensure 1
        EXPECT_TRUE(pr2->IsPaused());

		// ensure 2
        for (size_t i = 0; i < tt; i++) {
            raftpb::Message m;
            m.set_from(2);
            m.set_to(1);
            m.set_type(raftpb::MsgAppResp);
            m.set_index(i);
            r->Step(m);
            EXPECT_TRUE(pr2->IsPaused());
        }
	}
}

// TestMsgAppFlowControlRecvHeartbeat ensures a heartbeat response
// frees one slot if the window is full.
TEST(raft, TestMsgAppFlowControlRecvHeartbeat) {
	auto r = newTestRaft(1, 5, 1, newTestMemoryStorage({withPeers({1, 2})}));
	r->becomeCandidate();
	r->becomeLeader();

	auto pr2 = r->trk_.progress_[2];
	// force the progress to be in replicate state
	pr2->BecomeReplicate();
	// fill in the inflights window
    for (size_t i = 0; i < r->trk_.maxInflight_; i++) {
        r->Step(MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp,.entries = {EntryHelper{.data = "somedata"}.Done()}}.Done());
		readMessages(r);
	}
    for (size_t tt = 1; tt < 5; tt++) {
		// recv tt msgHeartbeatResp and expect one free slot
        for (size_t i = 0; i < tt; i++) {
            EXPECT_TRUE(pr2->IsPaused());
			// Unpauses the progress, sends an empty MsgApp, and pauses it again.
            raftpb::Message m;
            m.set_from(2);
            m.set_to(1);
            m.set_type(raftpb::MsgHeartbeatResp);
			r->Step(m);
			auto ms = readMessages(r);
            EXPECT_FALSE(ms.size() != 1 || ms[0].type() != raftpb::MsgApp || ms[0].entries().size() != 0);
		}

		// No more appends are sent if there are no heartbeats.
        for (size_t i = 0; i < 10; i++) {
            EXPECT_TRUE(pr2->IsPaused());
            r->Step(MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp,.entries = {EntryHelper{.data = "somedata"}.Done()}}.Done());
			auto ms = readMessages(r);
            EXPECT_TRUE(ms.empty());
		}

		// clear all pending messages.
        raftpb::Message m;
        m.set_from(2);
        m.set_to(1);
        m.set_type(raftpb::MsgHeartbeatResp);
		r->Step(m);
		readMessages(r);
	}
}

}