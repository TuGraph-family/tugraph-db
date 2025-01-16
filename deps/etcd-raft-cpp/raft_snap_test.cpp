#include "raft.h"
#include <gtest/gtest.h>

namespace eraft {
using namespace detail;

using testMemoryStorageOptions = std::function<void(std::shared_ptr<MemoryStorage>&)>;
extern std::shared_ptr<MemoryStorage> newTestMemoryStorage(const std::vector<testMemoryStorageOptions>& opts);
extern testMemoryStorageOptions withPeers(const std::vector<uint64_t>& peers);
extern std::shared_ptr<raft> newTestRaft(uint64_t id, int election, int heartbeat, std::shared_ptr<Storage> storage);
extern std::vector<raftpb::Message> readMessages(const std::shared_ptr<raft>& r);

static raftpb::Snapshot newTestingSnap() {
    raftpb::Snapshot testingSnap;
    auto meta = testingSnap.mutable_metadata();
    meta->set_index(11); // magic number
    meta->set_term(11); // magic number
    std::vector<uint64_t> voters = {1,2};
    *meta->mutable_conf_state()->mutable_voters() = {voters.begin(), voters.end()};
    return testingSnap;
}

TEST(raft, TestSendingSnapshotSetPendingSnapshot) {
	auto storage = newTestMemoryStorage({withPeers({1})});
	auto sm = newTestRaft(1, 10, 1, storage);
	sm->restore(newTestingSnap());

	sm->becomeCandidate();
	sm->becomeLeader();

	// force set the next of node 2, so that
	// node 2 needs a snapshot
	sm->trk_.progress_[2]->next_ = sm->raftLog_->firstIndex();
    raftpb::Message msg;
    msg.set_from(2);
    msg.set_to(1);
    msg.set_type(raftpb::MsgAppResp);
    msg.set_index(sm->trk_.progress_[2]->next_ - 1);
    msg.set_reject(true);
    sm->Step(msg);
    EXPECT_EQ(sm->trk_.progress_[2]->pendingSnapshot_, 11);
}

TEST(raft, TestPendingSnapshotPauseReplication) {
	auto storage = newTestMemoryStorage({withPeers({1, 2})});
	auto sm = newTestRaft(1, 10, 1, storage);
	sm->restore(newTestingSnap());

	sm->becomeCandidate();
	sm->becomeLeader();

	sm->trk_.progress_[2]->BecomeSnapshot(11);
	sm->Step(MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "somedata"}.Done()}}.Done());
	auto msgs = readMessages(sm);
    EXPECT_TRUE(msgs.empty());
}

TEST(raft, TestSnapshotFailure) {
	auto storage = newTestMemoryStorage({withPeers({1, 2})});
	auto sm = newTestRaft(1, 10, 1, storage);
	sm->restore(newTestingSnap());

	sm->becomeCandidate();
	sm->becomeLeader();

	sm->trk_.progress_[2]->next_ = 1;
	sm->trk_.progress_[2]->BecomeSnapshot(11);
    raftpb::Message msg;
    msg.set_from(2);
    msg.set_to(1);
    msg.set_type(raftpb::MsgSnapStatus);
    msg.set_reject(true);
	sm->Step(msg);
    EXPECT_EQ(sm->trk_.progress_[2]->pendingSnapshot_, 0);
    EXPECT_EQ(sm->trk_.progress_[2]->next_, 1);
    EXPECT_TRUE(sm->trk_.progress_[2]->msgAppFlowPaused_);
}

TEST(raft, TestSnapshotSucceed) {
	auto storage = newTestMemoryStorage({withPeers({1, 2})});
	auto sm = newTestRaft(1, 10, 1, storage);
	sm->restore(newTestingSnap());

	sm->becomeCandidate();
	sm->becomeLeader();

	sm->trk_.progress_[2]->next_ = 1;
	sm->trk_.progress_[2]->BecomeSnapshot(11);

    raftpb::Message msg;
    msg.set_from(2);
    msg.set_to(1);
    msg.set_type(raftpb::MsgSnapStatus);
    msg.set_reject(false);
	sm->Step(msg);
    EXPECT_EQ(sm->trk_.progress_[2]->pendingSnapshot_, 0);
    EXPECT_EQ(sm->trk_.progress_[2]->next_, 12);
    EXPECT_TRUE(sm->trk_.progress_[2]->msgAppFlowPaused_);
}

TEST(raft, TestSnapshotAbort) {
	auto storage = newTestMemoryStorage({withPeers({1, 2})});
	auto sm = newTestRaft(1, 10, 1, storage);
	sm->restore(newTestingSnap());

	sm->becomeCandidate();
	sm->becomeLeader();

	sm->trk_.progress_[2]->next_ = 1;
	sm->trk_.progress_[2]->BecomeSnapshot(11);

	// A successful msgAppResp that has a higher/equal index than the
	// pending snapshot should abort the pending snapshot.
    raftpb::Message msg;
    msg.set_from(2);
    msg.set_to(1);
    msg.set_type(raftpb::MsgAppResp);
    msg.set_index(11);
	sm->Step(msg);
    EXPECT_EQ(sm->trk_.progress_[2]->pendingSnapshot_, 0);
	// The follower entered StateReplicate and the leader send an append
	// and optimistically updated the progress (so we see 13 instead of 12).
	// There is something to append because the leader appended an empty entry
	// to the log at index 12 when it assumed leadership.
    EXPECT_EQ(sm->trk_.progress_[2]->next_, 13);
    auto n = sm->trk_.progress_[2]->inflights_->Count();
    EXPECT_EQ(n, 1);
}

}