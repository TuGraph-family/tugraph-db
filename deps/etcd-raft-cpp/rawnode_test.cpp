#include <gtest/gtest.h>
#include "rawnode.h"

namespace eraft {
using namespace detail;
// TestRawNodeStep ensures that RawNode.Step ignore local message.
extern Config newTestConfig(uint64_t id, int election, int heartbeat, std::shared_ptr<Storage> storage);
using testMemoryStorageOptions = std::function<void(std::shared_ptr<MemoryStorage>&)>;
extern testMemoryStorageOptions withPeers(const std::vector<uint64_t>& peers);
extern std::shared_ptr<MemoryStorage> newTestMemoryStorage(const std::vector<testMemoryStorageOptions>& opts);

// newTestRawNode sets up a RawNode with the given peers. The configuration will
// not be reflected in the Storage.
std::shared_ptr<RawNode> newTestRawNode(uint64_t id, int election, int heartbeat, std::shared_ptr<Storage> storage) {
    auto cfg = newTestConfig(id, election, heartbeat, std::move(storage));
    Error err;
    std::shared_ptr<RawNode> rn;
    std::tie(rn, err) = NewRawNode(cfg);
    EXPECT_EQ(err, nullptr);
    return rn;
}

TEST(raft, TestRawNodeStep) {
    for (int i = static_cast<int>(raftpb::MessageType_MIN);
             i <= static_cast<int>(raftpb::MessageType_MAX); i++) {
        auto s = NewMemoryStorage();
        raftpb::HardState hs;
        hs.set_term(1);
        hs.set_commit(1);
        s->SetHardState(hs);
        s->Append({EntryHelper{.index = 1,.term = 1}.Done()});
        raftpb::Snapshot sp;
        std::vector<uint64_t> voters = {1};
        *sp.mutable_metadata()->mutable_conf_state()->mutable_voters() = {voters.begin(), voters.end()};
        sp.mutable_metadata()->set_index(1);
        sp.mutable_metadata()->set_term(1);
        auto err = s->ApplySnapshot(sp);
        EXPECT_EQ(err, nullptr);
        // Append an empty entry to make sure the non-local messages (like
        // vote requests) are ignored and don't trigger assertions.
        std::shared_ptr<RawNode> rawNode;
        std::tie(rawNode, err) = NewRawNode(newTestConfig(1, 10, 1, s));
        EXPECT_EQ(err, nullptr);
        raftpb::Message msgt;
        msgt.set_type(static_cast<raftpb::MessageType>(i));
        err = rawNode->Step(msgt);
        // LocalMsg should be ignored.
        if (IsLocalMsg(static_cast<raftpb::MessageType>(i))) {
            EXPECT_EQ(err, ErrStepLocalMsg);
        }
	}
}


// TestNodeStepUnblock from node_test.go has no equivalent in rawNode because there is
// no goroutine in RawNode.

// TestRawNodeProposeAndConfChange tests the configuration change mechanism. Each
// test case sends a configuration change which is either simple or joint, verifies
// that it applies and that the resulting ConfState matches expectations, and for
// joint configurations makes sure that they are exited successfully.
std::shared_ptr<raftpb::ConfChangeWrap> NewConfChange(raftpb::ConfChangeType type, int node_id) {
    raftpb::ConfChange cc;
    cc.set_type(type);
    cc.set_node_id(node_id);
    return std::make_shared<raftpb::ConfChangeWrap>(cc);
}

raftpb::ConfChangeSingle NewConfChangeSingle(raftpb::ConfChangeType type, int node_id) {
    raftpb::ConfChangeSingle ccs;
    ccs.set_type(type);
    ccs.set_node_id(node_id);
    return ccs;
}

raftpb::ConfState NewConfState(const std::vector<uint64_t>& voters,
                                 const std::vector<uint64_t>& learners,
                                 const std::vector<uint64_t>& voters_outgoing,
                                 const std::vector<uint64_t>& learners_next,
                                 bool auto_leave) {
    raftpb::ConfState cs;
    *cs.mutable_voters() = {voters.begin(), voters.end()};
    *cs.mutable_learners() = {learners.begin(), learners.end()};
    *cs.mutable_voters_outgoing() = {voters_outgoing.begin(), voters_outgoing.end()};
    *cs.mutable_learners_next() = {learners_next.begin(), learners_next.end()};
    cs.set_auto_leave(auto_leave);
    return cs;
}

std::shared_ptr<raftpb::ConfChangeV2Wrap> NewConfChangeV2(const std::vector<raftpb::ConfChangeSingle>& ccss) {
    raftpb::ConfChangeV2 ccv2;
    *ccv2.mutable_changes() = {ccss.begin(), ccss.end()};
    return std::make_shared<raftpb::ConfChangeV2Wrap>(ccv2);
}

std::shared_ptr<raftpb::ConfChangeV2Wrap> NewConfChangeV2(const std::vector<raftpb::ConfChangeSingle>& ccss, raftpb::ConfChangeTransition cct) {
    raftpb::ConfChangeV2 ccv2;
    *ccv2.mutable_changes() = {ccss.begin(), ccss.end()};
    ccv2.set_transition(cct);
    return std::make_shared<raftpb::ConfChangeV2Wrap>(ccv2);
}

// TestNodeStepUnblock from node_test.go has no equivalent in rawNode because there is
// no goroutine in RawNode.

// TestRawNodeProposeAndConfChange tests the configuration change mechanism. Each
// test case sends a configuration change which is either simple or joint, verifies
// that it applies and that the resulting ConfState matches expectations, and for
// joint configurations makes sure that they are exited successfully.
TEST(raft, TestRawNodeProposeAndConfChange) {
    struct Test {
        std::shared_ptr<raftpb::ConfChangeI> cc;
        raftpb::ConfState exp;
        std::shared_ptr<raftpb::ConfState> exp2;
    };
    std::vector<Test> testCases =
    {
		// V1 config change.
		{
            NewConfChange(raftpb::ConfChangeAddNode, 2),
            NewConfState({1,2},{},{},{},false),
			nullptr,
		},
		// Proposing the same as a V2 change works just the same, without entering
		// a joint config.
		{
            NewConfChangeV2({NewConfChangeSingle(raftpb::ConfChangeAddNode, 2)}),
            NewConfState({1,2},{},{},{},false),
            nullptr,
		},
		// Ditto if we add it as a learner instead.
		{
            NewConfChangeV2({NewConfChangeSingle(raftpb::ConfChangeAddLearnerNode, 2)}),
            NewConfState({1},{2},{},{},false),
            nullptr,
		},
		// We can ask explicitly for joint consensus if we want it.
		{
            NewConfChangeV2({NewConfChangeSingle(raftpb::ConfChangeAddLearnerNode, 2)},
                            raftpb::ConfChangeTransitionJointExplicit),
            NewConfState({1},{1},{2},{},false),
            std::make_shared<raftpb::ConfState>(NewConfState({1},{2},{},{},false)),
		},
		// Ditto, but with implicit transition (the harness checks this).
		{
            NewConfChangeV2({NewConfChangeSingle(raftpb::ConfChangeAddLearnerNode, 2)},
                            raftpb::ConfChangeTransitionJointImplicit),
            NewConfState({1},{1},{2},{},true),
            std::make_shared<raftpb::ConfState>(NewConfState({1},{2},{},{},false)),
		},
		// Add a new node and demote n1. This exercises the interesting case in
		// which we really need joint config changes and also need LearnersNext.
		{
            NewConfChangeV2({NewConfChangeSingle(raftpb::ConfChangeAddLearnerNode, 2),
                             NewConfChangeSingle(raftpb::ConfChangeAddLearnerNode, 1),
                             NewConfChangeSingle(raftpb::ConfChangeAddLearnerNode, 3)}),
            NewConfState({2},{1},{3},{1},true),
            std::make_shared<raftpb::ConfState>(NewConfState({2},{1,3},{},{},false)),
		},
		// Ditto explicit.
		{
            NewConfChangeV2({NewConfChangeSingle(raftpb::ConfChangeAddNode, 2),
                             NewConfChangeSingle(raftpb::ConfChangeAddLearnerNode, 1),
                             NewConfChangeSingle(raftpb::ConfChangeAddLearnerNode, 3)},
                            raftpb::ConfChangeTransitionJointExplicit),
            NewConfState({2},{1},{3},{1},false),
            std::make_shared<raftpb::ConfState>(NewConfState({2},{1,3},{},{},false)),
		},
		// Ditto implicit.
		{
            NewConfChangeV2({NewConfChangeSingle(raftpb::ConfChangeAddNode, 2),
                             NewConfChangeSingle(raftpb::ConfChangeAddLearnerNode, 1),
                             NewConfChangeSingle(raftpb::ConfChangeAddLearnerNode, 3)},
                            raftpb::ConfChangeTransitionJointImplicit),
            NewConfState({2},{1},{3},{1},true),
            std::make_shared<raftpb::ConfState>(NewConfState({2},{1,3},{},{},false)),
		}
	};
    for (auto& tc : testCases) {
			auto s = newTestMemoryStorage({withPeers({1})});
        std::shared_ptr<RawNode> rawNode;
        Error err;
        std::tie(rawNode, err) = NewRawNode(newTestConfig(1, 10, 1, s));
        EXPECT_EQ(err, nullptr);
			rawNode->Campaign();
			auto proposed = false;
            uint64_t lastIndex;
            std::string ccdata;
			// Propose the ConfChange, wait until it applies, save the resulting
			// ConfState.
            std::shared_ptr<raftpb::ConfState> cs;
			while(cs == nullptr) {
				auto rd = rawNode->GetReady();
				s->Append(rd.entries_);
                for (auto& ent : rd.committedEntries_) {
					std::unique_ptr<raftpb::ConfChangeI> cc;
					if (ent.type() == raftpb::EntryConfChange) {
						raftpb::ConfChange ccc;
                        EXPECT_TRUE(ccc.ParseFromString(ent.data()));
                        cc.reset(new raftpb::ConfChangeWrap(ccc));
					} else if (ent.type() == raftpb::EntryConfChangeV2) {
						raftpb::ConfChangeV2 ccc;
                        EXPECT_TRUE(ccc.ParseFromString(ent.data()));
                        cc.reset(new raftpb::ConfChangeV2Wrap(ccc));
					}
					if (cc != nullptr) {
						cs = rawNode->ApplyConfChange(*cc);
					}
				}
				rawNode->Advance(rd);
				// Once we are the leader, propose a command and a ConfChange.
				if (!proposed && rd.softState_->lead_ == rawNode->raft_->id_) {
                    err = rawNode->Propose("somedata");
                    EXPECT_EQ(err, nullptr);
                    raftpb::ConfChange ccv1;
                    bool ok;
                    std::tie(ccv1, ok) = tc.cc->AsV1();
                    if (ok) {
                        EXPECT_TRUE(ccv1.SerializeToString(&ccdata));
                        auto ccw = raftpb::ConfChangeWrap(ccv1);
						rawNode->ProposeConfChange(&ccw);
					} else {
						auto ccv2 = tc.cc->AsV2();
                        EXPECT_TRUE(ccv2.SerializeToString(&ccdata));
                        auto ccv2w = raftpb::ConfChangeV2Wrap(ccv2);
						rawNode->ProposeConfChange(&ccv2w);
					}
					proposed = true;
				}
			}

			// Check that the last index is exactly the conf change we put in,
			// down to the bits. Note that this comes from the Storage, which
			// will not reflect any unstable entries that we'll only be presented
			// with in the next Ready.
			std::tie(lastIndex, err) = s->LastIndex();
            EXPECT_EQ(err, nullptr);
            std::vector<raftpb::Entry> entries;
			std::tie(entries, err) = s->Entries(lastIndex-1, lastIndex+1, noLimit);
            EXPECT_EQ(err, nullptr);
            EXPECT_EQ(entries.size(), 2);
            EXPECT_EQ(entries[0].data(), "somedata");
			auto typ = raftpb::EntryConfChange;
            bool ok;
            std::tie(std::ignore, ok) = tc.cc->AsV1();
            if (!ok) {
                typ = raftpb::EntryConfChangeV2;
            }
            EXPECT_EQ(entries[1].type(), typ);
            EXPECT_EQ(entries[1].data(), ccdata);
            EXPECT_TRUE(google::protobuf::util::MessageDifferencer::Equals(tc.exp, *cs));

			uint64_t maybePlusOne = 0;
            bool autoLeave = false;
            std::tie(autoLeave, ok) = raftpb::ConfChangeV2Wrap(tc.cc->AsV2()).EnterJoint();
			if (ok && autoLeave) {
				// If this is an auto-leaving joint conf change, it will have
				// appended the entry that auto-leaves, so add one to the last
				// index that forms the basis of our expectations on
				// pendingConfIndex. (Recall that lastIndex was taken from stable
				// storage, but this auto-leaving entry isn't on stable storage
				// yet).
				maybePlusOne = 1;
			}
            EXPECT_EQ(lastIndex+maybePlusOne, rawNode->raft_->pendingConfIndex_);

			// Move the RawNode along. If the ConfChange was simple, nothing else
			// should happen. Otherwise, we're in a joint state, which is either
			// left automatically or not. If not, we add the proposal that leaves
			// it manually.
			auto rd = rawNode->GetReady();
			std::string context;
			if (!tc.exp.auto_leave()) {
                EXPECT_TRUE(rd.entries_.empty());
                rawNode->Advance(rd);
				if (tc.exp2 == nullptr) {
					return;
				}
				context = "manual";
                raftpb::ConfChangeV2 ccv2;
                ccv2.set_context(context);
                auto ccv2Wrap = raftpb::ConfChangeV2Wrap(ccv2);
                err = rawNode->ProposeConfChange(&ccv2Wrap);
                EXPECT_EQ(err, nullptr);
				rd = rawNode->GetReady();
			}

			// Check that the right ConfChange comes out.
            EXPECT_FALSE(rd.entries_.size() != 1 || rd.entries_[0].type() != raftpb::EntryConfChangeV2);
			raftpb::ConfChangeV2 cc;
            EXPECT_TRUE(cc.ParseFromString(rd.entries_[0].data()));
            raftpb::ConfChangeV2 ccv2;
            ccv2.set_context(context);
            EXPECT_TRUE(google::protobuf::util::MessageDifferencer::Equals(cc, ccv2));
			// Lie and pretend the ConfChange applied. It won't do so because now
			// we require the joint quorum and we're only running one node.
			cs = rawNode->ApplyConfChange(raftpb::ConfChangeV2Wrap(cc));
            EXPECT_TRUE(google::protobuf::util::MessageDifferencer::Equals(*tc.exp2, *cs));
            rawNode->Advance(rd);
	}
}

// TestRawNodeJointAutoLeave tests the configuration change auto leave even leader
// lost leadership.
TEST(raft, TestRawNodeJointAutoLeave) {
    raftpb::ConfChangeV2 testCc;
    testCc.set_transition(raftpb::ConfChangeTransitionJointImplicit);
    {
        auto ccs = testCc.mutable_changes()->Add();
        ccs->set_node_id(2);
        ccs->set_type(raftpb::ConfChangeAddLearnerNode);
    }
    raftpb::ConfState expCs;
    expCs.set_auto_leave(true);
    std::vector<uint64_t> voters = {1};
    std::vector<uint64_t> votersOutgoing = {1};
    std::vector<uint64_t> learners = {2};
    *expCs.mutable_voters() = {voters.begin(), voters.end()};
    *expCs.mutable_voters_outgoing() = {votersOutgoing.begin(), votersOutgoing.end()};
    *expCs.mutable_learners() = {learners.begin(), learners.end()};
    raftpb::ConfState exp2Cs;
    *exp2Cs.mutable_voters() = {voters.begin(), voters.end()};
    *exp2Cs.mutable_learners() = {learners.begin(), learners.end()};

	auto s = newTestMemoryStorage({withPeers({1})});
    std::shared_ptr<RawNode> rawNode;
    Error err;
	std::tie(rawNode, err) = NewRawNode(newTestConfig(1, 10, 1, s));
    EXPECT_EQ(err, nullptr);

	rawNode->Campaign();
	bool proposed = false;
    uint64_t lastIndex = 0;
    std::string ccdata;
	// Propose the ConfChange, wait until it applies, save the resulting
	// ConfState.
	std::shared_ptr<raftpb::ConfState> cs;
	while (cs == nullptr) {
		auto rd = rawNode->GetReady();
		s->Append(rd.entries_);
        for (auto& ent : rd.committedEntries_) {
			std::unique_ptr<raftpb::ConfChangeI> cc;
			if (ent.type() == raftpb::EntryConfChangeV2) {
				raftpb::ConfChangeV2 ccc;
                EXPECT_TRUE(ccc.ParseFromString(ent.data()));
                cc.reset(new raftpb::ConfChangeV2Wrap(ccc));
			}
			if (cc != nullptr) {
				// Force it step down.
                raftpb::Message m;
                m.set_type(raftpb::MsgHeartbeatResp);
                m.set_from(1);
                m.set_term(rawNode->raft_->term_ + 1);
				rawNode->Step(m);
				cs = rawNode->ApplyConfChange(*cc);
			}
		}
		rawNode->Advance(rd);
		// Once we are the leader, propose a command and a ConfChange.
		if (!proposed && rd.softState_->lead_ == rawNode->raft_->id_) {
            EXPECT_EQ(rawNode->Propose("somedata"), nullptr);
            ccdata = testCc.SerializeAsString();
            raftpb::ConfChangeV2Wrap testCcWrap(testCc);
			rawNode->ProposeConfChange(&testCcWrap);
			proposed = true;
		}
	}

	// Check that the last index is exactly the conf change we put in,
	// down to the bits. Note that this comes from the Storage, which
	// will not reflect any unstable entries that we'll only be presented
	// with in the next Ready.
	std::tie(lastIndex, err) = s->LastIndex();
    EXPECT_EQ(err, nullptr);
    std::vector<raftpb::Entry> entries;
	std::tie(entries, err) = s->Entries(lastIndex-1, lastIndex+1, noLimit);
    EXPECT_EQ(err, nullptr);
    EXPECT_EQ(entries.size(), 2);
    EXPECT_EQ(entries[0].data(), "somedata");
    EXPECT_EQ(entries[1].type(), raftpb::EntryConfChangeV2);
    EXPECT_EQ(entries[1].data(), ccdata);
    EXPECT_TRUE(google::protobuf::util::MessageDifferencer::Equals(expCs, *cs));
    EXPECT_EQ(rawNode->raft_->pendingConfIndex_, 0);

	// Move the RawNode along. It should not leave joint because it's follower.
	auto rd = rawNode->readyWithoutAccept();
	// Check that the right ConfChange comes out.
    EXPECT_EQ(rd.entries_.size(), 0);

	// Make it leader again. It should leave joint automatically after moving apply index.
	rawNode->Campaign();
	rd = rawNode->GetReady();
	s->Append(rd.entries_);
	rawNode->Advance(rd);
	rd = rawNode->GetReady();
	s->Append(rd.entries_);
	rawNode->Advance(rd);
	rd = rawNode->GetReady();
	s->Append(rd.entries_);
	rawNode->Advance(rd);
	rd = rawNode->GetReady();
	s->Append(rd.entries_);
	// Check that the right ConfChange comes out.
    EXPECT_FALSE(rd.entries_.size() != 1 || rd.entries_[0].type() != raftpb::EntryConfChangeV2);
    raftpb::ConfChangeV2 cc;
    EXPECT_TRUE(cc.ParseFromString(rd.entries_[0].data()));
    EXPECT_TRUE(google::protobuf::util::MessageDifferencer::Equals(cc, raftpb::ConfChangeV2()));
	// Lie and pretend the ConfChange applied. It won't do so because now
	// we require the joint quorum and we're only running one node.
	cs = rawNode->ApplyConfChange(raftpb::ConfChangeV2Wrap(cc));
    EXPECT_TRUE(google::protobuf::util::MessageDifferencer::Equals(exp2Cs, *cs));
}

// TestRawNodeProposeAddDuplicateNode ensures that two proposes to add the same node should
// not affect the later propose to add new node.
TEST(raft, TestRawNodeProposeAddDuplicateNode) {
	auto s = newTestMemoryStorage({withPeers({1})});
    std::shared_ptr<RawNode> rawNode;
    Error err;
	std::tie(rawNode, err) = NewRawNode(newTestConfig(1, 10, 1, s));
    EXPECT_EQ(err, nullptr);
	auto rd = rawNode->GetReady();
	s->Append(rd.entries_);
	rawNode->Advance(rd);

	rawNode->Campaign();
	while(true) {
		rd = rawNode->GetReady();
		s->Append(rd.entries_);
		if (rd.softState_->lead_ == rawNode->raft_->id_) {
			rawNode->Advance(rd);
			break;
		}
		rawNode->Advance(rd);
	}

	auto proposeConfChangeAndApply = [&](const raftpb::ConfChange& cc) {
        auto ccw = raftpb::ConfChangeWrap(cc);
		rawNode->ProposeConfChange(&ccw);
		rd = rawNode->GetReady();
		s->Append(rd.entries_);
		for (auto& entry : rd.committedEntries_) {
			if (entry.type() == raftpb::EntryConfChange) {
				raftpb::ConfChange cc1;
                cc1.ParseFromString(entry.data());
				rawNode->ApplyConfChange(raftpb::ConfChangeWrap(cc1));
			}
		}
		rawNode->Advance(rd);
	};

    raftpb::ConfChange cc1;
    cc1.set_node_id(1);
    cc1.set_type(raftpb::ConfChangeAddNode);
    auto ccdata1 = cc1.SerializeAsString();
	proposeConfChangeAndApply(cc1);

	// try to add the same node again
	proposeConfChangeAndApply(cc1);

	// the new node join should be ok
    raftpb::ConfChange cc2;
    cc2.set_node_id(2);
    cc2.set_type(raftpb::ConfChangeAddNode);
    auto ccdata2 = cc2.SerializeAsString();
	proposeConfChangeAndApply(cc2);

    uint64_t lastIndex;
	std::tie(lastIndex, err) = s->LastIndex();
    EXPECT_EQ(err, nullptr);

	// the last three entries should be: ConfChange cc1, cc1, cc2
    std::vector<raftpb::Entry> entries;
	std::tie(entries, err) = s->Entries(lastIndex-2, lastIndex+1, noLimit);
    EXPECT_EQ(err, nullptr);
    EXPECT_EQ(entries.size(), 3);
    EXPECT_EQ(entries[0].data(), ccdata1);
    EXPECT_EQ(entries[2].data(), ccdata2);
}

// TestRawNodeReadIndex ensures that Rawnode.ReadIndex sends the MsgReadIndex message
// to the underlying raft. It also ensures that ReadState can be read out.
TEST(raft, TestRawNodeReadIndex) {
    std::vector<raftpb::Message> msgs;
    auto appendStep = [&](raft&, raftpb::Message m) -> Error {
        msgs.emplace_back(std::move(m));
		return {};
	};
    auto equal = [](const std::vector<ReadState>& a, const std::vector<ReadState>& b){
        if (a.size() != b.size()) {
            return false;
        }
        for (size_t i = 0; i < a.size(); i++) {
            if (a[i].index_ != b[i].index_ || a[i].requestCtx_ != b[i].requestCtx_) {
                return false;
            }
        }
        return true;
    };
    std::vector<ReadState> wrs = {{1, "somedata"}};

	auto s = newTestMemoryStorage({withPeers({1})});
	auto c = newTestConfig(1, 10, 1, s);
    std::shared_ptr<RawNode> rawNode;
	Error err;
    std::tie(rawNode, err) = NewRawNode(c);
    EXPECT_EQ(err, nullptr);

	rawNode->raft_->readStates_ = wrs;
	// ensure the ReadStates can be read out
	auto hasReady = rawNode->HasReady();
    EXPECT_TRUE(hasReady);
	auto rd = rawNode->GetReady();
    EXPECT_TRUE(equal(rd.readStates_, wrs));
	s->Append(rd.entries_);
	rawNode->Advance(rd);
	// ensure raft.readStates is reset after advance
    EXPECT_TRUE(rawNode->raft_->readStates_.empty());

    std::string wrequestCtx("somedata2");
	rawNode->Campaign();
    while (true) {
		rd = rawNode->GetReady();
		s->Append(rd.entries_);

		if (rd.softState_->lead_ == rawNode->raft_->id_) {
			rawNode->Advance(rd);

			// Once we are the leader, issue a ReadIndex request
			rawNode->raft_->step_ = appendStep;
			rawNode->ReadIndex(wrequestCtx);
			break;
		}
		rawNode->Advance(rd);
	}
	// ensure that MsgReadIndex message is sent to the underlying raft
    EXPECT_EQ(msgs.size(), 1);
    EXPECT_EQ(msgs[0].type(), raftpb::MsgReadIndex);
    EXPECT_EQ(msgs[0].entries(0).data(), wrequestCtx);
}

// TestBlockProposal from node_test.go has no equivalent in rawNode because there is
// no leader check in RawNode.

// TestNodeTick from node_test.go has no equivalent in rawNode because
// it reaches into the raft object which is not exposed.

// TestNodeStop from node_test.go has no equivalent in rawNode because there is
// no goroutine in RawNode.

// TestRawNodeStart ensures that a node can be started correctly. Note that RawNode
// requires the application to bootstrap the state, i.e. it does not accept peers
// and will not create faux configuration change entries.
TEST(raft, TestRawNodeStart) {
    std::vector<raftpb::Entry> entries = {{EntryHelper{.index = 2,.term = 1}.Done(), EntryHelper{.index = 3,.term = 1,.data = "foo"}.Done()}};
    Ready want;
    want.softState_ = std::make_shared<SoftState>();
    want.softState_->lead_ = 1;
    want.softState_->raftState_ = StateLeader;

    want.hardState_.set_term(1);
    want.hardState_.set_commit(3);
    want.hardState_.set_vote(1);

    want.committedEntries_ = entries;
    want.mustSync_ = false;


	auto storage = NewMemoryStorage();
	storage->ents_[0].set_index(1);

	// TODO(tbg): this is a first prototype of what bootstrapping could look
	// like (without the annoying faux ConfChanges). We want to persist a
	// ConfState at some index and make sure that this index can't be reached
	// from log position 1, so that followers are forced to pick up the
	// ConfState in order to move away from log position 1 (unless they got
	// bootstrapped in the same way already). Failing to do so would mean that
	// followers diverge from the bootstrapped nodes and don't learn about the
	// initial config.
	//
	// NB: this is exactly what CockroachDB does. The Raft log really begins at
	// index 10, so empty followers (at index 1) always need a snapshot first.
	auto bootstrap = [](const std::shared_ptr<MemoryStorage>& storage, const raftpb::ConfState& cs)->Error {
		if (cs.voters().empty()){
			return Error("no voters specified");
		}
        uint64_t fi;
        Error err;
		std::tie(fi, err) = storage->FirstIndex();
		if (err != nullptr) {
			return err;
		}
		if (fi < 2) {
			return Error("FirstIndex >= 2 is prerequisite for bootstrap");
		}
        std::tie(std::ignore, err) = storage->Entries(fi, fi, std::numeric_limits<uint64_t>::max());
        if (err == nullptr) {
			// TODO(tbg): match exact error
			return Error("should not have been able to load first index");
		}
        uint64_t li;
		std::tie(li, err) = storage->LastIndex();
		if (err != nullptr) {
			return err;
		}
        std::tie(std::ignore, err) = storage->Entries(li, li, std::numeric_limits<uint64_t>::max());
        if (err == nullptr) {
			return Error("should not have been able to load last index");
		};
        raftpb::HardState hs;
        raftpb::ConfState ics;
		std::tie(hs, ics, err) = storage->InitialState();
		if (err != nullptr) {
			return err;
		}
		if (!IsEmptyHardState(hs)) {
			return Error("HardState not empty");
		}
		if (!ics.voters().empty()) {
			return Error("ConfState not empty");
		}
        raftpb::SnapshotMetadata meta;
        meta.set_index(1);
        meta.set_term(0);
        *meta.mutable_conf_state() = cs;
        raftpb::Snapshot snap;
        *snap.mutable_metadata() = meta;
		return storage->ApplySnapshot(snap);
	};
    std::vector<uint64_t> voters = {1};
    raftpb::ConfState cs;
    *cs.mutable_voters() = {voters.begin(), voters.end()};
    EXPECT_EQ(bootstrap(storage, cs), nullptr);
    std::shared_ptr<RawNode> rawNode;
    Error err;
	std::tie(rawNode, err) = NewRawNode(newTestConfig(1, 10, 1, storage));
    EXPECT_EQ(err, nullptr);
    EXPECT_FALSE(rawNode->HasReady());
	rawNode->Campaign();
    auto rd = rawNode->GetReady();
    storage->Append(rd.entries_);
    rawNode->Advance(rd);
	rawNode->Propose("foo");
    EXPECT_TRUE(rawNode->HasReady());
	rd = rawNode->GetReady();
    EXPECT_TRUE(VectorEquals(entries, rd.entries_));
	storage->Append(rd.entries_);
	rawNode->Advance(rd);

    EXPECT_TRUE(rawNode->HasReady());
    rd = rawNode->GetReady();
    EXPECT_EQ(rd.entries_.size(), 0);
    EXPECT_FALSE(rd.mustSync_);
    rawNode->Advance(rd);

	rd.softState_ = nullptr;
    want.softState_ = nullptr;
    EXPECT_EQ(rd, want);
    EXPECT_FALSE(rawNode->HasReady());
}

TEST(raft, TestRawNodeRestart) {
    std::vector<raftpb::Entry> entries = {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1,.data = "foo"}.Done()};
    raftpb::HardState st;
    st.set_term(1);
    st.set_commit(1);

    Ready want;
    want.committedEntries_ = {entries.begin(), entries.begin() + st.commit()};
    want.mustSync_ = false;

	auto storage = newTestMemoryStorage({withPeers({1})});
	storage->SetHardState(st);
	storage->Append(entries);
    std::shared_ptr<RawNode> rawNode;
    Error err;
	std::tie(rawNode, err) = NewRawNode(newTestConfig(1, 10, 1, storage));
    EXPECT_EQ(err, nullptr);
	auto rd = rawNode->GetReady();
    EXPECT_EQ(rd, want);
	rawNode->Advance(rd);
    EXPECT_FALSE(rawNode->HasReady());
}

TEST(raft, TestRawNodeRestartFromSnapshot) {
    raftpb::Snapshot snap;
    snap.mutable_metadata()->set_index(2);
    snap.mutable_metadata()->set_term(1);
    raftpb::ConfState cs;
    std::vector<uint64_t> voters = {1,2};
    *cs.mutable_voters() = {voters.begin(), voters.end()};
    *snap.mutable_metadata()->mutable_conf_state() = cs;

    std::vector<raftpb::Entry> entries = {EntryHelper{.index = 3,.term = 1,.data = "foo"}.Done()};
    raftpb::HardState st;
    st.set_term(1);
    st.set_commit(3);

    Ready want;
    want.committedEntries_ = entries;
    want.mustSync_ = false;

	auto s = NewMemoryStorage();
	s->SetHardState(st);
	s->ApplySnapshot(snap);
	s->Append(entries);
    std::shared_ptr<RawNode> rawNode;
    Error err;
	std::tie(rawNode, err) = NewRawNode(newTestConfig(1, 10, 1, s));
    EXPECT_EQ(err, nullptr);
    auto rd = rawNode->GetReady();
    EXPECT_EQ(rd, want);
    rawNode->Advance(rd);
    EXPECT_FALSE(rawNode->HasReady());
}

// TestNodeAdvance from node_test.go has no equivalent in rawNode because there is
// no dependency check between Ready() and Advance()
TEST(raft, TestRawNodeStatus) {
	auto s = newTestMemoryStorage({withPeers({1})});
    std::shared_ptr<RawNode> rn;
    Error err;
	std::tie(rn, err) = NewRawNode(newTestConfig(1, 10, 1, s));
    EXPECT_EQ(err, nullptr);
    auto status = rn->GetStatus();
    EXPECT_TRUE(status.progress_.empty());
    err = rn->Campaign();
    EXPECT_EQ(err, nullptr);
    auto rd = rn->GetReady();
    s->Append(rd.entries_);
    rn->Advance(rd);
	status = rn->GetStatus();
    EXPECT_EQ(status.basicStatus_.softState_.lead_, 1);
    EXPECT_EQ(status.basicStatus_.softState_.raftState_, StateLeader);
    const auto& exp = *rn->raft_->trk_.progress_[1];
    auto act = status.progress_.at(1);
    EXPECT_EQ(exp, act);
    tracker::Config expCfg;
    expCfg.voters_.data_[0].data_ = {1};
    EXPECT_EQ(expCfg, status.config_);
}

// TestRawNodeCommitPaginationAfterRestart is the RawNode version of
// TestNodeCommitPaginationAfterRestart. The anomaly here was even worse as the
// Raft group would forget to apply entries:
//
//   - node learns that index 11 is committed
//   - nextCommittedEnts returns index 1..10 in CommittedEntries (but index 10
//     already exceeds maxBytes), which isn't noticed internally by Raft
//   - Commit index gets bumped to 10
//   - the node persists the HardState, but crashes before applying the entries
//   - upon restart, the storage returns the same entries, but `slice` takes a
//     different code path and removes the last entry.
//   - Raft does not emit a HardState, but when the app calls Advance(), it bumps
//     its internal applied index cursor to 10 (when it should be 9)
//   - the next Ready asks the app to apply index 11 (omitting index 10), losing a
//     write.
TEST(raft, TestRawNodeCommitPaginationAfterRestart) {
    auto s = newTestMemoryStorage({withPeers({1})});
    raftpb::HardState persistedHardState;
    persistedHardState.set_term(1);
    persistedHardState.set_vote(1);
    persistedHardState.set_commit(10);
	s->hardState_ = persistedHardState;
    s->ents_.resize(10);
	uint64_t size;
    for (size_t i = 0; i < s->ents_.size(); i++) {
        s->ents_[i].set_term(1);
        s->ents_[i].set_index(i+1);
        s->ents_[i].set_type(raftpb::EntryNormal);
        s->ents_[i].set_data("a");
        size += s->ents_[i].ByteSizeLong();
    }

	auto cfg = newTestConfig(1, 10, 1, s);
	// Set a MaxSizePerMsg that would suggest to Raft that the last committed entry should
	// not be included in the initial rd.CommittedEntries. However, our storage will ignore
	// this and *will* return it (which is how the Commit index ended up being 10 initially).
	cfg.maxSizePerMsg_ = size - s->ents_[s->ents_.size()-1].ByteSizeLong() - 1;

    s->ents_.emplace_back();
    s->ents_.back().set_term(1);
    s->ents_.back().set_index(11);
    s->ents_.back().set_type(raftpb::EntryNormal);
    s->ents_.back().set_data("boom");

    std::shared_ptr<RawNode> rawNode;
    Error err;
	std::tie(rawNode, err) = NewRawNode(cfg);
    EXPECT_EQ(err, nullptr);

    for (size_t highestApplied = 0; highestApplied != 11; ) {
		auto rd = rawNode->GetReady();
		auto n = rd.committedEntries_.size();
        EXPECT_TRUE( n != 0);
        if (highestApplied != 0 && highestApplied+1 != rd.committedEntries_[0].index()) {
            EXPECT_FALSE(true);
        }
		highestApplied = rd.committedEntries_[n-1].index();
		rawNode->Advance(rd);
        raftpb::Message msg;
        msg.set_type(raftpb::MsgHeartbeat);
        msg.set_to(1);
        msg.set_from(2); // illegal, but we get away with it
        msg.set_term(1);
        msg.set_commit(11);
		rawNode->Step(msg);
	}
}

// TestRawNodeBoundedLogGrowthWithPartition tests a scenario where a leader is
// partitioned from a quorum of nodes. It verifies that the leader's log is
// protected from unbounded growth even as new entries continue to be proposed.
// This protection is provided by the MaxUncommittedEntriesSize configuration.
TEST(raft, TestRawNodeBoundedLogGrowthWithPartition) {
	const size_t maxEntries = 16;
	std::string data = "testdata";
    raftpb::Entry testEntry = EntryHelper{.data = data}.Done();
	auto maxEntrySize = maxEntries * payloadSize(testEntry);

	auto s = newTestMemoryStorage({withPeers({1})});
	auto cfg = newTestConfig(1, 10, 1, s);
	cfg.maxUncommittedEntriesSize_ = maxEntrySize;
    std::shared_ptr<RawNode> rawNode;
    Error err;
	std::tie(rawNode, err) = NewRawNode(cfg);
    EXPECT_EQ(err, nullptr);

	// Become the leader and apply empty entry.
	rawNode->Campaign();
	while(true) {
		auto rd = rawNode->GetReady();
		s->Append(rd.entries_);
		rawNode->Advance(rd);
		if (!rd.committedEntries_.empty()) {
			break;
		}
	}

	// Simulate a network partition while we make our proposals by never
	// committing anything. These proposals should not cause the leader's
	// log to grow indefinitely.
	for (auto i = 0; i < 1024; i++) {
		rawNode->Propose(data);
	}

	// Check the size of leader's uncommitted log tail. It should not exceed the
	// MaxUncommittedEntriesSize limit.
	auto checkUncommitted = [&rawNode](uint64_t exp) {
        EXPECT_EQ(rawNode->raft_->uncommittedSize_, exp);
	};
	checkUncommitted(maxEntrySize);

	// Recover from the partition. The uncommitted tail of the Raft log should
	// disappear as entries are committed.
	auto rd = rawNode->GetReady();
    EXPECT_EQ(rd.entries_.size(), maxEntries);
	s->Append(rd.entries_);
	rawNode->Advance(rd);

	// Entries are appended, but not applied.
	checkUncommitted(maxEntrySize);

	rd = rawNode->GetReady();
    EXPECT_TRUE(rd.entries_.empty());
    EXPECT_EQ(rd.committedEntries_.size(), maxEntries);
	rawNode->Advance(rd);

	checkUncommitted(0);
}

TEST(raft, TestRawNodeConsumeReady) {
	// Check that readyWithoutAccept() does not call acceptReady (which resets
	// the messages) but Ready() does.
	auto s = newTestMemoryStorage({withPeers({1})});
	auto rn = newTestRawNode(1, 3, 1, s);
    raftpb::Message m1, m2;
    m1.set_context("foo");
    m2.set_context("bar");

	// Inject first message, make sure it's visible via readyWithoutAccept.
    rn->raft_->msgs_.push_back(m1);
	auto rd = rn->readyWithoutAccept();
    EXPECT_FALSE(rd.messages_.size() != 1 || !google::protobuf::util::MessageDifferencer::Equals(rd.messages_[0], m1));
	// Now call Ready() which should move the message into the Ready (as opposed
	// to leaving it in both places).
	rd = rn->GetReady();
    EXPECT_TRUE(rn->raft_->msgs_.empty());
    EXPECT_FALSE(rd.messages_.size() != 1 || !google::protobuf::util::MessageDifferencer::Equals(rd.messages_[0], m1));
	// Add a message to raft to make sure that Advance() doesn't drop it.
    rn->raft_->msgs_.push_back(m2);
	rn->Advance(rd);
    EXPECT_FALSE(rn->raft_->msgs_.size() != 1 || !google::protobuf::util::MessageDifferencer::Equals(rn->raft_->msgs_[0], m2));
}

}