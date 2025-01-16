#include <gtest/gtest.h>
#include <utility>
#include "tracker/state.h"
#include "raft.h"

namespace eraft {
using namespace detail;

std::string ltoa(std::shared_ptr<RaftLog> l) {
    char str[1024] = {0};
    sprintf(str, "committed: %lu\n", l->committed_);
    sprintf(str, "applied:  %lu\n", l->applied_);
    auto ents = l->allEntries();
    for (size_t i = 0; i < ents.size(); i++) {
        sprintf(str, "#%lu: %s\n", i, ents[i].ShortDebugString().c_str());
    }
	return str;
}
void advanceMessagesAfterAppend(const std::shared_ptr<raft>& r);
// nextEnts returns the appliable entries and updates the applied index
std::vector<raftpb::Entry> nextEnts(const std::shared_ptr<raft>& r, const std::shared_ptr<MemoryStorage>& s) {
    // Append unstable entries.
    s->Append(r->raftLog_->nextUnstableEnts());
    r->raftLog_->stableTo(r->raftLog_->lastIndex(), r->raftLog_->lastTerm());

    // Run post-append steps.
    advanceMessagesAfterAppend(r);

    // Return committed entries.
    auto ents = r->raftLog_->nextCommittedEnts(true);
    r->raftLog_->appliedTo(r->raftLog_->committed_, 0 /* size */);
    return ents;
}

void mustAppendEntry(std::shared_ptr<raft> r, const std::vector<raftpb::Entry>& ents) {
	if (!r->appendEntry(ents)) {
		ERAFT_FATAL("entry unexpectedly dropped");
	}
}

Config newTestConfig(uint64_t id, int election, int heartbeat, std::shared_ptr<Storage> storage) {
    Config config;
    config.id_ = id;
    config.electionTick_ = election;
    config.heartbeatTick_ = heartbeat;
    config.storage_ = std::move(storage);
    config.maxSizePerMsg_ = noLimit;
    config.maxInflightMsgs_ = 256;
	return config;
}

std::shared_ptr<raft> newTestRaft(uint64_t id, int election, int heartbeat, std::shared_ptr<Storage> storage) {
    auto config = newTestConfig(id, election, heartbeat, std::move(storage));
	return newRaft(config);
}

using testMemoryStorageOptions = std::function<void(std::shared_ptr<MemoryStorage>&)>;

std::shared_ptr<MemoryStorage> newTestMemoryStorage(const std::vector<testMemoryStorageOptions>& opts) {
	auto ms = NewMemoryStorage();
    for (auto& o : opts) {
		o(ms);
	}
	return ms;
}

std::shared_ptr<raft> newTestLearnerRaft(uint64_t id, int election, int heartbeat, std::shared_ptr<Storage> storage) {
	auto cfg = newTestConfig(id, election, heartbeat, std::move(storage));
	return newRaft(cfg);
}

std::vector<raftpb::Message> readMessages(const std::shared_ptr<raft>& r) {
    advanceMessagesAfterAppend(r);
    auto msgs = r->msgs_;
    r->msgs_.clear();
	return msgs;
}

std::vector<raftpb::Message> takeMessagesAfterAppend(const std::shared_ptr<raft>& r) {
	auto msgs = r->msgsAfterAppend_;
    r->msgsAfterAppend_.clear();
	return msgs;
}

Error stepOrSend(const std::shared_ptr<raft>& r, std::vector<raftpb::Message> msgs) {
    for (auto& m : msgs) {
		if (m.to() == r->id_) {
            auto err = r->Step(m);
            if (err != nullptr) {
                return err;
            }
		} else {
            r->msgs_.push_back(m);
		}
	}
	return {};
}

void advanceMessagesAfterAppend(const std::shared_ptr<raft>& r) {
	while(true) {
		auto msgs = takeMessagesAfterAppend(r);
		if (msgs.empty()) {
			break;
		}
		stepOrSend(r, msgs);
	}
}

testMemoryStorageOptions withPeers(const std::vector<uint64_t>& peers) {
    return [peers](std::shared_ptr<MemoryStorage>& ms) {
        *ms->snapshot_.mutable_metadata()->mutable_conf_state()->mutable_voters() = {peers.begin(), peers.end()};
    };
}

testMemoryStorageOptions withLearners(const std::vector<uint64_t>& learners)  {
    return [learners](std::shared_ptr<MemoryStorage>& ms) {
        *ms->snapshot_.mutable_metadata()->mutable_conf_state()->mutable_learners() = {learners.begin(), learners.end()};
    };
}

TEST(raft, TestProgressLeader) {
	auto r = newTestRaft(1, 5, 1, newTestMemoryStorage({withPeers({1, 2})}));
	r->becomeCandidate();
	r->becomeLeader();
	r->trk_.progress_[2]->BecomeReplicate();

    // Send proposals to r1. The first 5 entries should be queued in the unstable log.
    raftpb::Message propMsg;
    propMsg.set_from(1);
    propMsg.set_to(1);
    propMsg.set_type(raftpb::MsgProp);
    propMsg.mutable_entries()->Add()->set_data("foo");
	for (uint64_t i = 0; i < 5; i++) {
        EXPECT_EQ(r->Step(propMsg), nullptr);
	}
    auto m = r->trk_.progress_[1]->match_;
    EXPECT_EQ(m, 0);
    auto ents = r->raftLog_->nextUnstableEnts();
    if ((ents.size() != 6) || ents[0].data().size() > 0 || ents[5].data() != "foo") {
        EXPECT_TRUE(false);
    }
    advanceMessagesAfterAppend(r);
    EXPECT_EQ(r->trk_.progress_[1]->match_, 6);
    EXPECT_EQ(r->trk_.progress_[1]->next_, 7);
}

// TestProgressResumeByHeartbeatResp ensures raft.heartbeat reset progress.paused by heartbeat response.
TEST(raft, TestProgressResumeByHeartbeatResp) {
	auto r = newTestRaft(1, 5, 1, newTestMemoryStorage({withPeers({1, 2})}));
	r->becomeCandidate();
	r->becomeLeader();

	r->trk_.progress_[2]->msgAppFlowPaused_ = true;
    raftpb::Message msg;
    msg.set_from(1);
    msg.set_to(1);
    msg.set_type(raftpb::MsgBeat);
	r->Step(msg);
    EXPECT_TRUE(r->trk_.progress_[2]->msgAppFlowPaused_);

	r->trk_.progress_[2]->BecomeReplicate();
    msg.Clear();
    msg.set_from(2);
    msg.set_to(1);
    msg.set_type(raftpb::MsgHeartbeatResp);
	r->Step(msg);
    EXPECT_FALSE(r->trk_.progress_[2]->msgAppFlowPaused_);
}

TEST(raft, TestProgressPaused) {
	auto r = newTestRaft(1, 5, 1, newTestMemoryStorage({withPeers({1, 2})}));
	r->becomeCandidate();
	r->becomeLeader();
    raftpb::Message msg;
    msg.set_from(1);
    msg.set_to(1);
    msg.set_type(raftpb::MsgProp);
    msg.mutable_entries()->Add()->set_data("somedata");
	r->Step(msg);
    r->Step(msg);
    r->Step(msg);

	auto ms = readMessages(r);
    EXPECT_EQ(ms.size(), 1);
}

TEST(raft, TestProgressFlowControl) {
	auto cfg = newTestConfig(1, 5, 1, newTestMemoryStorage({withPeers({1, 2})}));
	cfg.maxInflightMsgs_ = 3;
	cfg.maxSizePerMsg_ = 2048;
    cfg.maxInflightBytes_ = 9000; // A little over MaxInflightMsgs * MaxSizePerMsg.
	auto r = newRaft(cfg);
	r->becomeCandidate();
	r->becomeLeader();

	// Throw away all the messages relating to the initial election.
	readMessages(r);

	// While node 2 is in probe state, propose a bunch of entries.
	r->trk_.progress_[2]->BecomeProbe();
    std::string blob(1000, 'a');
    std::string large(5000, 'b');

    for (auto i = 0; i < 22; i++) {
        auto newblob = blob;
        if (i >= 10 && i < 16) {
            // Temporarily send large messages.
            newblob = large;
        }
        raftpb::Message msg;
        msg.set_from(1);
        msg.set_to(1);
        msg.set_type(raftpb::MsgProp);
        msg.mutable_entries()->Add()->set_data(newblob);
        r->Step(msg);
    }

	auto ms = readMessages(r);
	// First append has two entries: the empty entry to confirm the
	// election, and the first proposal (only one proposal gets sent
	// because we're in probe state).
    EXPECT_FALSE(ms.size() != 1 || ms[0].type() != raftpb::MsgApp);
    EXPECT_EQ(ms[0].entries().size(), 2);
    EXPECT_FALSE(ms[0].entries(0).data().size() != 0 || ms[0].entries(1).data().size() != 1000);

    auto ackAndVerify = [&r](uint64_t index, std::vector<int> expEntries) {
        raftpb::Message msg;
        msg.set_from(2);
        msg.set_to(1);
        msg.set_type(raftpb::MsgAppResp);
        msg.set_index(index);
        r->Step(msg);
        auto ms = readMessages(r);
        EXPECT_EQ(ms.size(), expEntries.size());
        for (size_t i = 0; i < ms.size(); i++) {
            EXPECT_EQ(ms[i].type(), raftpb::MsgApp);
            EXPECT_EQ(ms[i].entries().size(), expEntries[i]);
        }
        auto last = ms[ms.size()-1].entries();
        if (last.empty()) {
            return index;
        }
        return last[last.size()-1].index();
    };

    // When this append is acked, we change to replicate state and can
    // send multiple messages at once.
    auto index = ackAndVerify(ms[0].entries(1).index(), {2, 2, 2});
    // Ack all three of those messages together and get another 3 messages. The
    // third message contains a single large entry, in contrast to 2 before.
    index = ackAndVerify(index, {2, 1, 1});
    // All subsequent messages contain one large entry, and we cap at 2 messages
    // because it overflows MaxInflightBytes.
    index = ackAndVerify(index, {1, 1});
    index = ackAndVerify(index, {1, 1});
    // Start getting small messages again.
    index = ackAndVerify(index, {1, 2, 2});
    ackAndVerify(index, {2});
}

TEST(raft, TestUncommittedEntryLimit) {
	// Use a relatively large number of entries here to prevent regression of a
	// bug which computed the size before it was fixed. This test would fail
	// with the bug, either because we'd get dropped proposals earlier than we
	// expect them, or because the final tally ends up nonzero. (At the time of
	// writing, the former).
	const auto maxEntries = 1024;
    raftpb::Entry testEntry;
    testEntry.set_data("testdata");
	auto maxEntrySize = maxEntries * payloadSize(testEntry);

    EXPECT_EQ(payloadSize(raftpb::Entry()), 0);

	auto cfg = newTestConfig(1, 5, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	cfg.maxUncommittedEntriesSize_ = maxEntrySize;
	cfg.maxInflightMsgs_ = 2 * 1024; // avoid interference
	auto r = newRaft(cfg);
	r->becomeCandidate();
	r->becomeLeader();
    EXPECT_EQ(r->uncommittedSize_, 0);

	// Set the two followers to the replicate state. Commit to tail of log.
	const auto numFollowers = 2;
	r->trk_.progress_[2]->BecomeReplicate();
	r->trk_.progress_[3]->BecomeReplicate();
	r->uncommittedSize_ = 0;

	// Send proposals to r1. The first 5 entries should be appended to the log.
    raftpb::Message propMsg;
    propMsg.set_from(1);
    propMsg.set_to(1);
    propMsg.set_type(raftpb::MsgProp);
    *propMsg.mutable_entries()->Add() = testEntry;
	//propEnts := make([]pb.Entry, maxEntries)
    std::vector<raftpb::Entry> propEnts(maxEntries);
	for (auto i = 0; i < maxEntries; i++) {
        EXPECT_EQ(r->Step(propMsg), nullptr);
		propEnts[i] = testEntry;
	}

	// Send one more proposal to r1. It should be rejected.
    EXPECT_EQ(r->Step(propMsg), ErrProposalDropped);

	// Read messages and reduce the uncommitted size as if we had committed
	// these entries.
	auto ms = readMessages(r);
    EXPECT_EQ(maxEntries * numFollowers, ms.size());
	r->reduceUncommittedSize(payloadsSize(propEnts));
    EXPECT_EQ(r->uncommittedSize_, 0);

	// Send a single large proposal to r1. Should be accepted even though it
	// pushes us above the limit because we were beneath it before the proposal.
    propEnts.clear();
    propEnts.resize(2*maxEntries, testEntry);
    raftpb::Message propMsgLarge;
    propMsgLarge.set_from(1);
    propMsgLarge.set_to(1);
    propMsgLarge.set_type(raftpb::MsgProp);
    *propMsgLarge.mutable_entries() = {propEnts.begin(), propEnts.end()};
    EXPECT_EQ(r->Step(propMsgLarge), nullptr);

	// Send one more proposal to r1. It should be rejected, again.
    EXPECT_EQ(r->Step(propMsg), ErrProposalDropped);

	// But we can always append an entry with no Data. This is used both for the
	// leader's first empty entry and for auto-transitioning out of joint config
	// states.
    raftpb::Message msg;
    msg.set_from(1);
    msg.set_to(1);
    msg.set_type(raftpb::MsgProp);
    msg.mutable_entries()->Add();
    EXPECT_EQ(r->Step(msg), nullptr);

	// Read messages and reduce the uncommitted size as if we had committed
	// these entries.
	ms = readMessages(r);
    EXPECT_EQ(2 * numFollowers, ms.size());
	r->reduceUncommittedSize(payloadsSize(propEnts));
    EXPECT_EQ(r->uncommittedSize_, 0);
}

void preVoteConfig(Config* c) {
    c->preVote_ = true;
}

struct stateMachine {
    virtual Error Step(raftpb::Message) = 0;
    virtual std::vector<raftpb::Message> readMessages() = 0;
    virtual void advanceMessagesAfterAppend() = 0;
};

struct blackHole : public stateMachine {
    Error Step(raftpb::Message) override {
        return {};
    }
    std::vector<raftpb::Message> readMessages() override {
        return {};
    }
    void advanceMessagesAfterAppend() override {}
};

auto nopStepper = std::make_shared<blackHole>();

struct raftWarp : public stateMachine {
    explicit raftWarp(std::shared_ptr<raft> raft) : raft_(std::move(raft)) {}
    Error Step(raftpb::Message m) override {
        return raft_->Step(std::move(m));
    }
    std::vector<raftpb::Message> readMessages() override {
        return ::eraft::readMessages(raft_);
    }
    void advanceMessagesAfterAppend() override {
        ::eraft::advanceMessagesAfterAppend(raft_);
    }
    std::shared_ptr<raft> raft_;
};

std::shared_ptr<raftWarp> entsWithConfig(const std::function<void(Config*)>& configFunc, std::vector<uint64_t> terms) {
	auto storage = NewMemoryStorage();
    for (size_t i = 0; i < terms.size(); i++) {
        storage->Append({EntryHelper{.index = i + 1, .term = terms[i]}.Done()});
	}
	auto cfg = newTestConfig(1, 5, 1, storage);
	if (configFunc) {
		configFunc(&cfg);
	}
	auto sm = newRaft(cfg);
	sm->reset(terms[terms.size()-1]);
	return std::make_shared<raftWarp>(sm);
}

// votedWithConfig creates a raft state machine with Vote and Term set
// to the given value but no log entries (indicating that it voted in
// the given term but has not received any logs).
std::shared_ptr<raftWarp> votedWithConfig(const std::function<void(Config*)>& configFunc, uint64_t vote, uint64_t term) {
	auto storage = NewMemoryStorage();
    raftpb::HardState hs;
    hs.set_vote(vote);
    hs.set_term(term);
	storage->SetHardState(hs);
	auto cfg = newTestConfig(1, 5, 1, storage);
	if (configFunc){
		configFunc(&cfg);
	}
	auto sm = newRaft(cfg);
	sm->reset(term);
	return std::make_shared<raftWarp>(sm);
}


struct connem {
    uint64_t from;
    uint64_t to;
    bool operator<(const connem& r) const {
        if (from < r.from) {
            return true;
        } else if (from > r.from) {
            return false;
        } else {
            return to < r.to;
        }
    }
};

struct network {
	std::unordered_map<uint64_t, std::shared_ptr<stateMachine>> peers;
    std::unordered_map<uint64_t, std::shared_ptr<MemoryStorage>> storage;
    std::map<connem, float> dropm;
    std::unordered_map<raftpb::MessageType, bool> ignorem;

	// msgHook is called for each message sent. It may inspect the
	// message and return true to send it or false to drop it.
    std::function<bool(const raftpb::Message&)> msgHook;

    std::vector<raftpb::Message>filter(const std::vector<raftpb::Message>& msgs) {
        std::vector<raftpb::Message> mm;
        for (auto& m : msgs) {
            if (ignorem.count(m.type()) > 0 && ignorem.at(m.type())) {
                continue;
            }
            switch (m.type()) {
                case raftpb::MsgHup: {
                    // hups never go over the network, so don't drop them but panic
                    ERAFT_FATAL("unexpected msgHup");
                    break;
                }
                default: {
                    float perc = 0;
                    if (dropm.count(connem{m.from(), m.to()}) > 0) {
                        perc = dropm.at(connem{m.from(), m.to()});
                    }
                    auto n = float(random(std::numeric_limits<int64_t>::max() - 1)) /
                             float(std::numeric_limits<int64_t>::max());
                    if (n < perc) {
                        continue;
                    }
                }
            }
            if (msgHook) {
                if (!msgHook(m)) {
                    continue;
                }
            }
            mm.push_back(m);
        }
        return mm;
    }

    void send(std::vector<raftpb::Message> msgs) {
        while (!msgs.empty()) {
            auto& m = msgs[0];
            auto& p = peers.at(m.to());
            p->Step(m);
            p->advanceMessagesAfterAppend();
            msgs.erase(msgs.begin());
            auto append = filter(p->readMessages());
            msgs.insert(msgs.end(), append.begin(), append.end());
        }
    }

    void drop(uint64_t from, uint64_t to , float perc) {
        dropm[(connem{from, to})] = perc;
    }

    void cut(uint64_t one, uint64_t other) {
        drop(one, other, 2.0); // always drop
        drop(other, one, 2.0); // always drop
    }

    void isolate(uint64_t id) {
        for (size_t i = 0; i < peers.size(); i++) {
            auto nid = i + 1;
            if (nid != id) {
                drop(id, nid, 1.0); // always drop
                drop(nid, id, 1.0); // always drop
            }
        }
    }

    void ignore(raftpb::MessageType t) {
        ignorem[t] = true;
    }

    void recover() {
        dropm.clear();
        ignorem.clear();
    }
};

std::vector<uint64_t> idsBySize(int size) {
    std::vector<uint64_t> ids(size, 0);
    for (auto i = 0; i < size; i++) {
		ids[i] = 1 + i;
	}
	return ids;
}

// newNetworkWithConfig is like newNetwork but calls the given func to
// modify the configuration of any state machines it creates.
std::shared_ptr<network> newNetworkWithConfig(const std::function<void(Config*)>& configFunc, const std::vector<std::shared_ptr<stateMachine>>& peers) {
	auto size = peers.size();
	auto peerAddrs = idsBySize(size);

    std::unordered_map<uint64_t, std::shared_ptr<stateMachine>> npeers;
    std::unordered_map<uint64_t, std::shared_ptr<MemoryStorage>> nstorage(size);

    for (size_t j = 0; j < peers.size(); j++) {
        auto& p = peers[j];
		auto id = peerAddrs[j];
		//switch v := p.(type) {
        if (!p) {
            nstorage[id] = newTestMemoryStorage({withPeers(peerAddrs)});
            auto cfg = newTestConfig(id, 10, 1, nstorage[id]);
            if (configFunc) {
                configFunc(&cfg);
            }
            auto sm  = newRaft(cfg);
            npeers[id] = std::make_shared<raftWarp>(sm);
        } else if (std::dynamic_pointer_cast<raftWarp>(p)) {
            // TODO(tbg): this is all pretty confused. Clean this up.
            std::unordered_map<uint64_t, bool> learners;
            auto rw = std::dynamic_pointer_cast<raftWarp>(p);
            auto v = rw->raft_;
            for (auto &i: v->trk_.config_.learners_) {
                learners[i] = true;
            }
            v->id_ = id;
            v->trk_ = tracker::MakeProgressTracker(v->trk_.maxInflight_, v->trk_.maxInflightBytes_);
            for (size_t i = 0; i < size; i++) {
                auto pr = std::make_shared<tracker::Progress>();
                if (learners.count(peerAddrs[i]) > 0) {
                    pr->isLearner_ = true;
                    v->trk_.config_.learners_.insert(peerAddrs[i]);
                } else {
                    v->trk_.config_.voters_[0].data().insert(peerAddrs[i]);
                }
                v->trk_.progress_.data()[peerAddrs[i]] = pr;
            }
            v->reset(v->term_);
            npeers[id] = rw;
        } else if (std::dynamic_pointer_cast<blackHole>(p)) {
            npeers[id] = std::dynamic_pointer_cast<blackHole>(p);
        } else {
            ERAFT_FATAL("unexpected state machine type");
		}
	}
    auto ret = std::make_shared<network>();
    ret->peers = npeers;
    ret->storage = nstorage;
    return ret;
}

// newNetwork initializes a network from peers.
// A nil node will be replaced with a new *stateMachine.
// A *stateMachine will get its k, id.
// When using stateMachine, the address list is always [1, n].
std::shared_ptr<network> newNetwork(const std::vector<std::shared_ptr<stateMachine>>& peers) {
    return newNetworkWithConfig(nullptr, peers);
}

// setRandomizedElectionTimeout set up the value by caller instead of choosing
// by system, in some test scenario we need to fill in some expected value to
// ensure the certainty
void setRandomizedElectionTimeout(const std::shared_ptr<raft>& r, int64_t v) {
	r->randomizedElectionTimeout_ = v;
}

void testLeaderElection(bool preVote) {
	std::function<void(Config*)> cfg;
	auto candState = StateCandidate;
    uint64_t candTerm =  1;
	if (preVote) {
		cfg = preVoteConfig;
		// In pre-vote mode, an election that fails to complete
		// leaves the node in pre-candidate state without advancing
		// the term.
		candState = StatePreCandidate;
		candTerm = 0;
	}
    struct Test {
        std::shared_ptr<network> network_;
        StateType state;
        uint64_t expTerm;
    };
    std::vector<Test> tests = {
		{newNetworkWithConfig(cfg, {nullptr, nullptr, nullptr}), StateLeader, 1},
		{newNetworkWithConfig(cfg, {nullptr, nullptr, nopStepper}), StateLeader, 1},
		{newNetworkWithConfig(cfg, {nullptr, nopStepper, nopStepper}), candState, candTerm},
		{newNetworkWithConfig(cfg, {nullptr, nopStepper, nopStepper, nullptr}), candState, candTerm},
		{newNetworkWithConfig(cfg, {nullptr, nopStepper, nopStepper, nullptr, nullptr}), StateLeader, 1},

		// three logs further along than 0, but in the same term so rejections
		// are returned instead of the votes being ignored.
		{newNetworkWithConfig(cfg,
                              {nullptr, entsWithConfig(cfg, {1}), entsWithConfig(cfg, {1}), entsWithConfig(cfg, {1, 1}), nullptr}),
			StateFollower, 1},
	};

    for (auto &tt : tests) {
        raftpb::Message m;
        m.set_from(1);
        m.set_to(1);
        m.set_type(raftpb::MsgHup);
		tt.network_->send({m});
		auto sm = std::dynamic_pointer_cast<raftWarp>(tt.network_->peers.at(1))->raft_;
        EXPECT_EQ(sm->state_, tt.state);
        EXPECT_EQ(sm->term_, tt.expTerm);
	}
}

TEST(raft, TestLeaderElection) {
    testLeaderElection(false);
}

TEST(raft, TestLeaderElectionPreVote) {
    testLeaderElection(true);
}

// TestLearnerElectionTimeout verfies that the leader should not start election even
// when times out.
TEST(raft, TestLearnerElectionTimeout) {
	auto n1 = newTestLearnerRaft(1, 10, 1, newTestMemoryStorage({withPeers({1}), withLearners({2})}));
	auto n2 = newTestLearnerRaft(2, 10, 1, newTestMemoryStorage({withPeers({1}), withLearners({2})}));

	n1->becomeFollower(1, None);
	n2->becomeFollower(1, None);

	// n2 is learner. Learner should not start election even when times out.
	setRandomizedElectionTimeout(n2, n2->electionTimeout_);
	for (auto i = 0; i < n2->electionTimeout_; i++) {
		n2->tick_();
	}
    EXPECT_EQ(n2->state_, StateFollower);
}

// TestLearnerPromotion verifies that the learner should not election until
// it is promoted to a normal peer.
TEST(raft, TestLearnerPromotion) {
	auto n1 = newTestLearnerRaft(1, 10, 1, newTestMemoryStorage({withPeers({1}), withLearners({2})}));
	auto n2 = newTestLearnerRaft(2, 10, 1, newTestMemoryStorage({withPeers({1}), withLearners({2})}));

	n1->becomeFollower(1, None);
	n2->becomeFollower(1, None);

	auto nt = newNetwork({std::make_shared<raftWarp>(n1), std::make_shared<raftWarp>(n2)});

    EXPECT_NE(n1->state_, StateLeader);

	// n1 should become leader
	setRandomizedElectionTimeout(n1, n1->electionTimeout_);
	for (int i = 0; i < n1->electionTimeout_; i++) {
		n1->tick_();
	}
    advanceMessagesAfterAppend(n1);

    EXPECT_EQ(n1->state_, StateLeader);
    EXPECT_EQ(n2->state_, StateFollower);

    raftpb::Message msg;
    msg.set_from(1);
    msg.set_to(1);
    msg.set_type(raftpb::MsgBeat);
	nt->send({msg});

    raftpb::ConfChange cc;
    cc.set_node_id(2);
    cc.set_type(raftpb::ConfChangeAddNode);
    raftpb::ConfChangeWrap ccw(std::move(cc));
	n1->applyConfChange(ccw.AsV2());
	n2->applyConfChange(ccw.AsV2());
    EXPECT_FALSE(n2->isLearner_);

	// n2 start election, should become leader
	setRandomizedElectionTimeout(n2, n2->electionTimeout_);
	for (int i = 0; i < n2->electionTimeout_; i++) {
		n2->tick_();
	}
    advanceMessagesAfterAppend(n2);

    msg.Clear();
    msg.set_from(2);
    msg.set_to(2);
    msg.set_type(raftpb::MsgBeat);
	nt->send({msg});

    EXPECT_EQ(n1->state_, StateFollower);
    EXPECT_EQ(n2->state_, StateLeader);
}

// TestLearnerCanVote checks that a learner can vote when it receives a valid Vote request.
// See (*raft).Step for why this is necessary and correct behavior.
TEST(raft, TestLearnerCanVote) {
	auto n2 = newTestLearnerRaft(2, 10, 1, newTestMemoryStorage({withPeers({1}), withLearners({2})}));

	n2->becomeFollower(1, None);

    raftpb::Message msg;
    msg.set_from(1);
    msg.set_to(2);
    msg.set_term(2);
    msg.set_type(raftpb::MsgVote);
    msg.set_logterm(11);
    msg.set_index(11);
	n2->Step(msg);

    auto msgs = readMessages(n2);
    EXPECT_EQ(msgs.size(), 1);
    EXPECT_EQ(msgs[0].type(), raftpb::MsgVoteResp);
    EXPECT_FALSE(msgs[0].reject());
}

// testLeaderCycle verifies that each node in a cluster can campaign
// and be elected in turn. This ensures that elections (including
// pre-vote) work when not starting from a clean slate (as they do in
// TestLeaderElection)
void testLeaderCycle(bool preVote) {
    std::function<void(Config*)> cfg;
	if (preVote) {
		cfg = preVoteConfig;
	}
	auto n = newNetworkWithConfig(cfg, {nullptr, nullptr, nullptr});
	for (uint64_t campaignerID = 1; campaignerID <= 3; campaignerID++) {
        raftpb::Message m;
        m.set_from(campaignerID);
        m.set_to(campaignerID);
        m.set_type(raftpb::MsgHup);
		n->send({m});

        for (auto& pair: n->peers) {
            auto sm = std::dynamic_pointer_cast<raftWarp>(pair.second);
            EXPECT_FALSE(sm->raft_->id_ == campaignerID && sm->raft_->state_ != StateLeader);
            EXPECT_FALSE(sm->raft_->id_ != campaignerID && sm->raft_->state_ != StateFollower);
		}
	}
}

TEST(raft, TestLeaderCycle) {
	testLeaderCycle(false);
}

TEST(raft, TestLeaderCyclePreVote) {
	testLeaderCycle(true);
}

void testLeaderElectionOverwriteNewerLogs(bool preVote) {
    std::function<void(Config*)> cfg;
	if (preVote) {
		cfg = preVoteConfig;
	}
	// This network represents the results of the following sequence of
	// events:
	// - Node 1 won the election in term 1.
	// - Node 1 replicated a log entry to node 2 but died before sending
	//   it to other nodes.
	// - Node 3 won the second election in term 2.
	// - Node 3 wrote an entry to its logs but died without sending it
	//   to any other nodes.
	//
	// At this point, nodes 1, 2, and 3 all have uncommitted entries in
	// their logs and could win an election at term 3. The winner's log
	// entry overwrites the losers'. (TestLeaderSyncFollowerLog tests
	// the case where older log entries are overwritten, so this test
	// focuses on the case where the newer entries are lost).
	auto n = newNetworkWithConfig(cfg,{
		entsWithConfig(cfg, {1}),     // Node 1: Won first election
		entsWithConfig(cfg, {1}),     // Node 2: Got logs from node 1
		entsWithConfig(cfg, {2}),     // Node 3: Won second election
		votedWithConfig(cfg, 3, 2), // Node 4: Voted but didn't get logs
		votedWithConfig(cfg, 3, 2)}); // Node 5: Voted but didn't get logs

	// Node 1 campaigns. The election fails because a quorum of nodes
	// know about the election that already happened at term 2. Node 1's
	// term is pushed ahead to 2.
    raftpb::Message m;
    m.set_from(1);
    m.set_to(1);
    m.set_type(raftpb::MsgHup);
	n->send({m});
    auto sm1 = std::dynamic_pointer_cast<raftWarp>(n->peers[1]);
    EXPECT_EQ(sm1->raft_->state_, StateFollower);
    EXPECT_EQ(sm1->raft_->term_, 2);

	// Node 1 campaigns again with a higher term. This time it succeeds.
    raftpb::Message msg;
    msg.set_from(1);
    msg.set_to(1);
    msg.set_type(raftpb::MsgHup);
	n->send({msg});
    EXPECT_EQ(sm1->raft_->state_, StateLeader);
    EXPECT_EQ(sm1->raft_->term_, 3);

	// Now all nodes agree on a log entry with term 1 at index 1 (and
	// term 3 at index 2).
    for (auto& pair : n->peers) {
        auto sm = std::dynamic_pointer_cast<raftWarp>(pair.second);
		auto entries = sm->raft_->raftLog_->allEntries();
        EXPECT_EQ(entries.size(), 2);
        EXPECT_EQ(entries[0].term(), 1);
        EXPECT_EQ(entries[1].term(), 3);
	}
}

// TestLeaderElectionOverwriteNewerLogs tests a scenario in which a
// newly-elected leader does *not* have the newest (i.e. highest term)
// log entries, and must overwrite higher-term log entries with
// lower-term ones.
TEST(raft, TestLeaderElectionOverwriteNewerLogs) {
    testLeaderElectionOverwriteNewerLogs(false);
}
TEST(raft, TestLeaderElectionOverwriteNewerLogsPreVote) {
    testLeaderElectionOverwriteNewerLogs(true);
}

void testVoteFromAnyState(raftpb::MessageType vt) {
	for (auto n = 0; n < (int)numStates; n++) {
		auto r = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
		r->term_ = 1;
        auto st = (StateType)n;
		switch (st) {
		case StateFollower:
			r->becomeFollower(r->term_, 3);
            break;
		case StatePreCandidate:
			r->becomePreCandidate();
            break;
		case StateCandidate:
			r->becomeCandidate();
            break;
		case StateLeader:
			r->becomeCandidate();
			r->becomeLeader();
            break;
        case numStates:
            // Just to fix the compiler warnings
            break;
        }

        // Note that setting our state above may have advanced r.Term
        // past its initial value.
        auto origTerm = r->term_;
        auto newTerm = r->term_ + 1;

        raftpb::Message msg;
        msg.set_from(2);
        msg.set_to(1);
        msg.set_type(vt);
        msg.set_term(newTerm);
        msg.set_logterm(newTerm);
        msg.set_index(42);
        auto err = r->Step(msg);
        EXPECT_EQ(err, nullptr);
        auto msgs = readMessages(r);
        EXPECT_EQ(msgs.size(),1);

        auto& resp = msgs[0];
        EXPECT_EQ(resp.type(), voteRespMsgType(vt));
        EXPECT_FALSE(resp.reject());

		// If this was a real vote, we reset our state and term.
		if (vt == raftpb::MsgVote) {
            EXPECT_EQ(r->state_, StateFollower);
            EXPECT_EQ(r->term_, newTerm);
            EXPECT_EQ(r->vote_, 2);
		} else {
			// In a prevote, nothing changes.
            EXPECT_EQ(r->state_, st);
            EXPECT_EQ(r->term_, origTerm);
			// if st == StateFollower or StatePreCandidate, r hasn't voted yet.
			// In StateCandidate or StateLeader, it's voted for itself.
            EXPECT_FALSE(r->vote_ != None && r->vote_ != 1);
		}
	}
}

TEST(raft, TestVoteFromAnyState) {
    testVoteFromAnyState(raftpb::MsgVote);
}

TEST(raft, TestPreVoteFromAnyState) {
    testVoteFromAnyState(raftpb::MsgPreVote);
}

TEST(raft, TestLogReplication) {
    struct Test {
        std::shared_ptr<network> net;
        std::vector<raftpb::Message> msgs;
        uint64_t wcommitted;
    };
    std::vector<Test> tests =
	{
		{
			newNetwork({nullptr, nullptr, nullptr}),
            {MessageHelper{.from = 1, .to = 1, .type = raftpb::MsgProp, .entries = {EntryHelper{.data = "somedata"}.Done()}}.Done()},
			2,
		},
		{
			newNetwork({nullptr, nullptr, nullptr}),
            {
                MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "somedata"}.Done()}}.Done(),
                MessageHelper{.from = 1,.to = 2,.type = raftpb::MsgHup}.Done(),
                MessageHelper{.from = 1,.to = 2,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "somedata"}.Done()}}.Done(),
            },
			4,
		}
	};

    for (auto& tt : tests) {
        tt.net->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});
        for (auto& m : tt.msgs) {
			tt.net->send({m});
		}
        for (auto& pair : tt.net->peers) {
            auto sm = std::dynamic_pointer_cast<raftWarp>(pair.second);
            EXPECT_EQ(sm->raft_->raftLog_->committed_, tt.wcommitted);

            std::vector<raftpb::Entry> ents;
            for (auto& e : nextEnts(sm->raft_, tt.net->storage.at(pair.first))) {
				if (!e.data().empty()) {
                    ents.push_back(e);
				}
			}
            std::vector<raftpb::Message> props;
            for (auto& m : tt.msgs) {
				if (m.type() == raftpb::MsgProp) {
                    props.push_back(m);
				}
			}
            for (size_t k = 0; k < props.size(); k++) {
                EXPECT_EQ(ents[k].data(), props[k].entries(0).data());
			}
		}
	}
}

// TestLearnerLogReplication tests that a learner can receive entries from the leader.
TEST(raft, TestLearnerLogReplication) {
	auto n1 = newTestLearnerRaft(1, 10, 1, newTestMemoryStorage({withPeers({1}), withLearners({2})}));
	auto n2 = newTestLearnerRaft(2, 10, 1, newTestMemoryStorage({withPeers({1}), withLearners({2})}));

	auto nt = newNetwork({std::make_shared<raftWarp>(n1), std::make_shared<raftWarp>(n2)});

	n1->becomeFollower(1, None);
	n2->becomeFollower(1, None);

	setRandomizedElectionTimeout(n1, n1->electionTimeout_);
	for (auto i = 0; i < n1->electionTimeout_; i++) {
		n1->tick_();
	}
    advanceMessagesAfterAppend(n1);
	nt->send({MessageHelper{.from = 1, .to = 1, .type = raftpb::MsgBeat}.Done()});

	// n1 is leader and n2 is learner
    EXPECT_EQ(n1->state_, StateLeader);
    EXPECT_TRUE(n2->isLearner_);

	auto nextCommitted = 2;

	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "somedata"}.Done()}}.Done()});
    EXPECT_EQ(n1->raftLog_->committed_, nextCommitted);
    EXPECT_EQ(n1->raftLog_->committed_, n2->raftLog_->committed_);

	auto match = n1->trk_.progress_[2]->match_;
    EXPECT_EQ(match, n2->raftLog_->committed_);
}

TEST(raft, TestSingleNodeCommit) {
    auto s = newTestMemoryStorage({withPeers({1})});
    auto cfg = newTestConfig(1, 10, 1, s);
    auto r = newRaft(cfg);
    auto tt = newNetwork({std::make_shared<raftWarp>(r)});

	tt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	tt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "some data"}.Done()}}.Done()});
	tt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "some data"}.Done()}}.Done()});

    auto sm = std::dynamic_pointer_cast<raftWarp>(tt->peers.at(1));
    EXPECT_EQ(sm->raft_->raftLog_->committed_, 3);
}

// TestCannotCommitWithoutNewTermEntry tests the entries cannot be committed
// when leader changes, no new proposal comes in and ChangeTerm proposal is
// filtered.
TEST(raft, TestCannotCommitWithoutNewTermEntry) {
	auto tt = newNetwork({nullptr, nullptr, nullptr, nullptr, nullptr});
	tt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	// 0 cannot reach 2,3,4
	tt->cut(1, 3);
	tt->cut(1, 4);
	tt->cut(1, 5);


	tt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "some data"}.Done()}}.Done()});
	tt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "some data"}.Done()}}.Done()});

    auto sm = std::dynamic_pointer_cast<raftWarp>(tt->peers.at(1));
    EXPECT_EQ(sm->raft_->raftLog_->committed_, 1);

	// network recovery
	tt->recover();
	// avoid committing ChangeTerm proposal
	tt->ignore(raftpb::MsgApp);

	// elect 2 as the new leader with term 2
	tt->send({MessageHelper{.from = 2,.to = 2,.type = raftpb::MsgHup}.Done()});

	// no log entries from previous term should be committed
    sm = std::dynamic_pointer_cast<raftWarp>(tt->peers.at(2));
    EXPECT_EQ(sm->raft_->raftLog_->committed_, 1);

	tt->recover();
	// send heartbeat; reset wait
	tt->send({MessageHelper{.from = 2,.to = 2,.type = raftpb::MsgBeat}.Done()});
	// append an entry at current term
	tt->send({MessageHelper{.from = 2,.to = 2,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "some data"}.Done()}}.Done()});
	// expect the committed to be advanced
    EXPECT_EQ(sm->raft_->raftLog_->committed_, 5);
}

// TestCommitWithoutNewTermEntry tests the entries could be committed
// when leader changes, no new proposal comes in.
TEST(raft, TestCommitWithoutNewTermEntry) {
	auto tt = newNetwork({nullptr, nullptr, nullptr, nullptr, nullptr});
	tt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	// 0 cannot reach 3,4,5
	tt->cut(1, 3);
	tt->cut(1, 4);
	tt->cut(1, 5);

	tt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "some data"}.Done()}}.Done()});
    tt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "some data"}.Done()}}.Done()});

    auto sm = std::dynamic_pointer_cast<raftWarp>(tt->peers.at(1));
    EXPECT_EQ(sm->raft_->raftLog_->committed_, 1);

	// network recovery
	tt->recover();

	// elect 2 as the new leader with term 2
	// after append a ChangeTerm entry from the current term, all entries
	// should be committed
	tt->send({MessageHelper{.from = 2,.to = 2,.type = raftpb::MsgHup}.Done()});
    EXPECT_EQ(sm->raft_->raftLog_->committed_, 4);
}

TEST(raft, TestDuelingCandidates) {
	auto a = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto b = newTestRaft(2, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto c = newTestRaft(3, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));

	auto nt = newNetwork({std::make_shared<raftWarp>(a), std::make_shared<raftWarp>(b), std::make_shared<raftWarp>(c)});
	nt->cut(1, 3);

	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});
    nt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});

	// 1 becomes leader since it receives votes from 1 and 2
    auto sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));
    EXPECT_EQ(sm->raft_->state_, StateLeader);

	// 3 stays as candidate since it receives a vote from 3 and a rejection from 2
    sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(3));
    EXPECT_EQ(sm->raft_->state_, StateCandidate);

	nt->recover();

	// candidate 3 now increases its term and tries to vote again
	// we expect it to disrupt the leader 1 since it has a higher term
	// 3 will be follower again since both 1 and 2 rejects its vote request since 3 does not have a long enough log

	nt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});
    EXPECT_EQ(sm->raft_->state_, StateFollower);

    struct Test {
        std::shared_ptr<raft> sm;
        StateType state;
        uint64_t term;
        uint64_t lastIndex;
    };
    std::vector<Test> tests = {
		{a, StateFollower, 2, 1},
		{b, StateFollower, 2, 1},
		{c, StateFollower, 2, 0}
	};
    for (size_t i = 0; i < tests.size(); i++) {
        auto& tt = tests[i];
        EXPECT_EQ(tt.sm->state_, tt.state);
        EXPECT_EQ(tt.sm->term_, tt.term);
        EXPECT_EQ(tt.lastIndex, tt.sm->raftLog_->lastIndex());
	}
}

TEST(raft, TestDuelingPreCandidates) {
	auto cfgA = newTestConfig(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto cfgB = newTestConfig(2, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto cfgC = newTestConfig(3, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	cfgA.preVote_ = true;
	cfgB.preVote_ = true;
	cfgC.preVote_ = true;
	auto a = newRaft(cfgA);
	auto b = newRaft(cfgB);
	auto c = newRaft(cfgC);

	auto nt = newNetwork({std::make_shared<raftWarp>(a), std::make_shared<raftWarp>(b), std::make_shared<raftWarp>(c)});
	nt->cut(1, 3);

	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});
    nt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});

	// 1 becomes leader since it receives votes from 1 and 2
    auto sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));
    EXPECT_EQ(sm->raft_->state_, StateLeader);

	// 3 campaigns then reverts to follower when its PreVote is rejected
    sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(3));
    EXPECT_EQ(sm->raft_->state_, StateFollower);

	nt->recover();

	// Candidate 3 now increases its term and tries to vote again.
	// With PreVote, it does not disrupt the leader.
	nt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});


    struct Test {
        std::shared_ptr<raft> sm;
        StateType state;
        uint64_t term;
        uint64_t lastIndex;
    };
    std::vector<Test> tests = {
		{a, StateLeader, 1, 1},
		{b, StateFollower, 1, 1},
		{c, StateFollower, 1, 0}
	};

    for (size_t i = 0; i < tests.size(); i++) {
        auto& tt = tests[i];
        EXPECT_EQ(tt.sm->state_, tt.state);
        EXPECT_EQ(tt.sm->term_, tt.term);
        EXPECT_EQ(tt.lastIndex, tt.sm->raftLog_->lastIndex());
	}
}

TEST(raft, TestCandidateConcede) {
	auto tt = newNetwork({nullptr, nullptr, nullptr});
	tt->isolate(1);

	tt->send({MessageHelper{.from = 3,.to = 1,.type = raftpb::MsgHup}.Done()});
	tt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});

	// heal the partition
	tt->recover();
	// send heartbeat; reset wait
	tt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgBeat}.Done()});

	std::string data("force follower");
	// send a proposal to 3 to flush out a MsgApp to 1
	tt->send({MessageHelper{.from = 3,.to = 3, .type = raftpb::MsgProp, .entries = {EntryHelper{.data = data}.Done()}}.Done()});
	// send heartbeat; flush out commit
	tt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgBeat}.Done()});

    auto a = std::dynamic_pointer_cast<raftWarp>(tt->peers.at(1));
    EXPECT_EQ(a->raft_->state_, StateFollower);
    EXPECT_EQ(a->raft_->term_, 1);

    auto wlog = std::make_shared<RaftLog>();
    auto ms = std::make_shared<MemoryStorage>();
    ms->ents_ = {{}, EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1, .data = data}.Done()};
    wlog->storage_ = ms;
    wlog->committed_ = 2;
    wlog->unstable_.offset_ = 3;
    auto wantLog = ltoa(wlog);
    for (auto& pair : tt->peers) {
        auto sm = std::dynamic_pointer_cast<raftWarp>(pair.second);
        if (sm) {
            auto l = ltoa(sm->raft_->raftLog_);
            EXPECT_EQ(wantLog, l);
        }
	}
}

TEST(raft, TestSingleNodeCandidate) {
	auto tt = newNetwork({nullptr});
	tt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});
    auto sm = std::dynamic_pointer_cast<raftWarp>(tt->peers.at(1));
    EXPECT_EQ(sm->raft_->state_, StateLeader);
}

TEST(raft, TestSingleNodePreCandidate) {
	auto tt = newNetworkWithConfig(preVoteConfig, {nullptr});
	tt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});
    auto sm = std::dynamic_pointer_cast<raftWarp>(tt->peers.at(1));
    EXPECT_EQ(sm->raft_->state_, StateLeader);
}

TEST(raft, TestOldMessages) {
	auto tt = newNetwork({nullptr, nullptr, nullptr});
	// make 0 leader @ term 3
	tt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});
	tt->send({MessageHelper{.from = 2,.to = 2,.type = raftpb::MsgHup}.Done()});
	tt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});
	// pretend we're an old leader trying to make progress; this entry is expected to be ignored.
	tt->send({MessageHelper{.from = 2,.to = 1,.term = 2, .type = raftpb::MsgApp, .entries = {EntryHelper{.index = 3,.term = 2}.Done()}}.Done()});
	// commit a new entry
	tt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "somedata"}.Done()}}.Done()});

    auto wlog = std::make_shared<RaftLog>();
    auto ms = std::make_shared<MemoryStorage>();
    ms->ents_ = {{}, EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done(), EntryHelper{.index = 3,.term = 3}.Done(), EntryHelper{.index = 4,.term = 3,.data = "somedata"}.Done()};
    wlog->storage_ = ms;
    wlog->committed_ = 4;
    wlog->unstable_.offset_ = 5;
    auto base = ltoa(wlog);
    for (auto& pair : tt->peers) {
        auto sm = std::dynamic_pointer_cast<raftWarp>(pair.second);
        if (sm) {
            auto l = ltoa(sm->raft_->raftLog_);
            EXPECT_EQ(base, l);
        }
	}
}

// TestOldMessagesReply - optimization - reply with new term.
TEST(raft, TestProposal) {
    struct Test {
        std::shared_ptr<network> net;
        bool success;
    };
    std::vector<Test> tests = {
		{newNetwork({nullptr, nullptr, nullptr}), true},
		{newNetwork({nullptr, nullptr, nopStepper}), true},
		{newNetwork({nullptr, nopStepper, nopStepper}), false},
		{newNetwork({nullptr, nopStepper, nopStepper, nullptr}), false},
		{newNetwork({nullptr, nopStepper, nopStepper, nullptr, nullptr}), true},
	};

    for (auto& tt : tests) {
		std::string data = "somedata";

		// promote 1 to become leader
		tt.net->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});
        tt.net->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = data}.Done()}}.Done()});
        auto r = std::dynamic_pointer_cast<raftWarp>(tt.net->peers.at(1));
		auto wantLog = detail::newLog(NewMemoryStorage());
		if (tt.success) {
            wantLog = std::make_shared<RaftLog>();
            auto ms = std::make_shared<MemoryStorage>();
            ms->ents_ = {{}, EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1, .data = data}.Done()};
            wantLog->storage_ = ms;
            wantLog->unstable_.offset_ = 3;
		}
		auto base = ltoa(wantLog);
        for (auto& pair : tt.net->peers) {
            auto sm = std::dynamic_pointer_cast<raftWarp>(pair.second);
            if (sm) {
                auto l = ltoa(sm->raft_->raftLog_);
                EXPECT_EQ(base, l);
            }
		}
        EXPECT_EQ(r->raft_->term_, 1);
	}
}

TEST(raft, TestProposalByProxy) {
	std::string data = "somedata";
    std::vector<std::shared_ptr<network>> tests = {
            newNetwork({nullptr, nullptr, nullptr}),
            newNetwork({nullptr, nullptr, nopStepper}),
    };
    for (auto& tt : tests) {
		// promote 0 the leader
		tt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

		// propose via follower
		tt->send({MessageHelper{.from = 2, .to = 2, .type = raftpb::MsgProp, .entries = {EntryHelper{.data = "somedata"}.Done()}}.Done()});

        auto wantLog = std::make_shared<RaftLog>();
        auto ms = std::make_shared<MemoryStorage>();
        ms->ents_ = {{}, EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1, .data = data}.Done()};
        wantLog->storage_ = ms;
        wantLog->committed_ = 2;
        wantLog->unstable_.offset_ = 3;
		auto base = ltoa(wantLog);
        for (auto& pair : tt->peers) {
            auto sm = std::dynamic_pointer_cast<raftWarp>(pair.second);
            if (sm) {
                auto l = ltoa(sm->raft_->raftLog_);
                EXPECT_EQ(base, l);
            }
		}
        auto sm = std::dynamic_pointer_cast<raftWarp>(tt->peers.at(1));
        EXPECT_EQ(sm->raft_->term_, 1);
	}
}

TEST(raft, TestCommit) {
    struct Test {
        std::vector<uint64_t> matches;
        std::vector<raftpb::Entry> logs;
        uint64_t smTerm;
        uint64_t w;
    };
    std::vector<Test> tests = {
		// single
		{{1}, {EntryHelper{.index = 1,.term = 1}.Done()}, 1, 1},
		{{1}, {EntryHelper{.index = 1,.term = 1}.Done()}, 2, 0},
		{{2}, {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done()}, 2, 2},
		{{1}, {EntryHelper{.index = 1,.term = 2}.Done()}, 2, 1},

		// odd
		{{2, 1, 1}, {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done()}, 1, 1},
		{{2, 1, 1}, {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1}.Done()}, 2, 0},
		{{2, 1, 2}, {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done()}, 2, 2},
		{{2, 1, 2}, {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1}.Done()}, 2, 0},

		// even
		{{2, 1, 1, 1}, {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done()}, 1, 1},
		{{2, 1, 1, 1}, {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1}.Done()}, 2, 0},
		{{2, 1, 1, 2}, {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done()}, 1, 1},
		{{2, 1, 1, 2}, {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1}.Done()}, 2, 0},
		{{2, 1, 2, 2}, {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done()}, 2, 2},
		{{2, 1, 2, 2}, {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1}.Done()}, 2, 0},
	};
    for (auto& tt : tests) {
		auto storage = newTestMemoryStorage({withPeers({1})});
		storage->Append(tt.logs);
		storage->hardState_.set_term(tt.smTerm);

		auto sm = newTestRaft(1, 10, 2, storage);
        for (size_t j = 0; j < tt.matches.size(); j++) {
			auto id = j + 1;
			if (id > 1) {
                raftpb::ConfChange cc;
                cc.set_type(raftpb::ConfChangeAddNode);
                cc.set_node_id(id);
                raftpb::ConfChangeWrap ccw(cc);
				sm->applyConfChange(ccw.AsV2());
			}
            auto pr = sm->trk_.progress_.data()[id];
            pr->match_ = tt.matches[j];
            pr->next_ = tt.matches[j]+1;
		}
		sm->maybeCommit();
        EXPECT_EQ(sm->raftLog_->committed_, tt.w);
	}
}

TEST(raft, TestPastElectionTimeout) {
    struct Test {
        int elapse;
        float wprobability;
        bool round;
    };
    std::vector<Test> tests = {
		{5, 0, false},
		{10, 0.1, true},
		{13, 0.4, true},
		{15, 0.6, true},
		{18, 0.9, true},
		{20, 1, false},
	};
    for (auto& tt : tests) {
		auto sm = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1})}));
		sm->electionElapsed_ = tt.elapse;
		int c = 0;
		for (auto j = 0; j < 10000; j++) {
			sm->resetRandomizedElectionTimeout();
			if (sm->pastElectionTimeout()) {
				c++;
			}
		}
        float got = float(c) / float(10000.0);
		if (tt.round) {
			got = std::floor(got*10+0.5) / float(10.0);
		}
        EXPECT_EQ(got, tt.wprobability);
	}
}

// TestStepIgnoreOldTermMsg to ensure that the Step function ignores the message
// from old term and does not pass it to the actual stepX function.
TEST(raft, TestStepIgnoreOldTermMsg) {
	auto called = false;
    auto fakeStep = [&called](raft&, const raftpb::Message&)->Error {
        called = true;
        return {};
    };
	auto sm = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1})}));
	sm->step_ = fakeStep;
	sm->term_ = 2;
    raftpb::Message msg;
    msg.set_type(raftpb::MsgApp);
    msg.set_term(sm->term_ - 1);
	sm->Step(msg);
    EXPECT_FALSE(called);
}

// TestHandleMsgApp ensures:
//  1. Reply false if log doesn’t contain an entry at prevLogIndex whose term matches prevLogTerm.
//  2. If an existing entry conflicts with a new one (same index but different terms),
//     delete the existing entry and all that follow it; append any new entries not already in the log.
//  3. If leaderCommit > commitIndex, set commitIndex = min(leaderCommit, index of last new entry).
TEST(raft, TestHandleMsgApp) {
    struct Test {
        raftpb::Message m;
        uint64_t wIndex;
        uint64_t wCommit;
        bool wReject;
    };

    std::vector<Test> tests;
    std::vector<raftpb::Entry> ents;

    // Ensure 1
    raftpb::Message msg;
    msg.set_type(raftpb::MsgApp);msg.set_term(2);msg.set_logterm(3);msg.set_index(2);msg.set_commit(3);
    tests.push_back({msg, 2,0,true});

    msg.Clear();
    msg.set_type(raftpb::MsgApp);msg.set_term(2);msg.set_logterm(3);msg.set_index(3);msg.set_commit(3);
    tests.push_back({msg, 2,0,true});

    // Ensure 2
    msg.set_type(raftpb::MsgApp);msg.set_term(2);msg.set_logterm(1);msg.set_index(1);msg.set_commit(1);tests.push_back({msg, 2,1,false});
    msg.Clear();

    msg.set_type(raftpb::MsgApp);msg.set_term(0);msg.set_logterm(0);msg.set_index(0);msg.set_commit(1);
    ents = {EntryHelper{.index = 1,.term = 2}.Done()};*msg.mutable_entries() = {ents.begin(), ents.end()};
    tests.push_back({msg, 1,1, false});

    msg.Clear();
    msg.set_type(raftpb::MsgApp);msg.set_term(2);msg.set_logterm(2);msg.set_index(2);msg.set_commit(3);
    ents = {EntryHelper{.index = 3,.term = 2}.Done(), EntryHelper{.index = 4,.term = 2}.Done()};*msg.mutable_entries() = {ents.begin(), ents.end()};
    tests.push_back({msg, 4,3, false});

    msg.Clear();
    msg.set_type(raftpb::MsgApp);msg.set_term(2);msg.set_logterm(2);msg.set_index(2);msg.set_commit(4);
    ents = {EntryHelper{.index = 3,.term = 2}.Done()};*msg.mutable_entries() = {ents.begin(), ents.end()};
    tests.push_back({msg, 3,3, false});

    msg.Clear();
    msg.set_type(raftpb::MsgApp);msg.set_term(2);msg.set_logterm(1);msg.set_index(1);msg.set_commit(4);
    ents = {EntryHelper{.index = 2,.term = 2}.Done()};*msg.mutable_entries() = {ents.begin(), ents.end()};
    tests.push_back({msg, 2, 2, false});

    // Ensure 3
    msg.Clear();
    // match entry 1, commit up to last new entry 1
    msg.set_type(raftpb::MsgApp);msg.set_term(1);msg.set_logterm(1);msg.set_index(1);msg.set_commit(3);
    tests.push_back({msg, 2, 1, false});

    msg.Clear();
    // match entry 1, commit up to last new entry 2
    msg.set_type(raftpb::MsgApp);msg.set_term(1);msg.set_logterm(1);msg.set_index(1);msg.set_commit(3);
    ents = {EntryHelper{.index = 2,.term = 2}.Done()};*msg.mutable_entries() = {ents.begin(), ents.end()};
    tests.push_back({msg, 2, 2, false});

    msg.Clear();
    // match entry 2, commit up to last new entry 2
    msg.set_type(raftpb::MsgApp);msg.set_term(2);msg.set_logterm(2);msg.set_index(2);msg.set_commit(3);
    tests.push_back({msg, 2, 2, false});

    msg.Clear();
    // commit up to log.last()
    msg.set_type(raftpb::MsgApp);msg.set_term(2);msg.set_logterm(2);msg.set_index(2);msg.set_commit(4);
    tests.push_back({msg, 2, 2, false});

    for (auto& tt : tests) {
		auto storage = newTestMemoryStorage({withPeers({1})});
		storage->Append({EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done()});
		auto sm = newTestRaft(1, 10, 1, storage);
		sm->becomeFollower(2, None);

		sm->handleAppendEntries(tt.m);
        EXPECT_EQ(sm->raftLog_->lastIndex(), tt.wIndex);
        EXPECT_EQ(sm->raftLog_->committed_, tt.wCommit);
		auto m = readMessages(sm);
        EXPECT_EQ(m.size(), 1);
        EXPECT_EQ(m[0].reject(), tt.wReject);
	}
}

// TestHandleHeartbeat ensures that the follower commits to the commit in the message.
TEST(raft, TestHandleHeartbeat) {
	uint64_t commit = 2;
    struct Test{
        raftpb::Message m;
        uint64_t wCommit;
    };
    std::vector<Test> tests;
    raftpb::Message msg;
    msg.set_from(2);
    msg.set_to(1);
    msg.set_type(raftpb::MsgHeartbeat);
    msg.set_term(2);
    msg.set_commit(commit + 1);
    tests.push_back({msg, commit + 1});

    msg.Clear();
    // do not decrease commit
    msg.set_from(2);
    msg.set_to(1);
    msg.set_type(raftpb::MsgHeartbeat);
    msg.set_term(2);
    msg.set_commit(commit - 1);
    tests.push_back({msg, commit});


    for (auto& tt : tests) {
		auto storage = newTestMemoryStorage({withPeers({1, 2})});
		storage->Append({EntryHelper{.index = 1,.term = 1}.Done(),EntryHelper{.index = 2,.term = 2}.Done(),EntryHelper{.index = 3,.term = 3}.Done()});
		auto sm = newTestRaft(1, 5, 1, storage);
		sm->becomeFollower(2, 2);
		sm->raftLog_->commitTo(commit);
		sm->handleHeartbeat(tt.m);
        EXPECT_EQ(sm->raftLog_->committed_, tt.wCommit);
		auto m = readMessages(sm);
        EXPECT_EQ(m.size(), 1);
        EXPECT_EQ(m[0].type(), raftpb::MsgHeartbeatResp);
	}
}

// TestRaftFreesReadOnlyMem ensures raft will free read request from
// readOnly readIndexQueue and pendingReadIndex map.
// related issue: https://github.com/etcd-io/etcd/issues/7571
TEST(raft, TestRaftFreesReadOnlyMem) {
	auto sm = newTestRaft(1, 5, 1, newTestMemoryStorage({withPeers({1, 2})}));
	sm->becomeCandidate();
	sm->becomeLeader();
	sm->raftLog_->commitTo(sm->raftLog_->lastIndex());

	std::string ctx = "ctx";

	// leader starts linearizable read request.
	// more info: raft dissertation 6.4, step 2.
    raftpb::Message msg;
    msg.set_from(2);
    msg.set_type(raftpb::MsgReadIndex);
    msg.mutable_entries()->Add()->set_data(ctx);
	sm->Step({msg});
	auto msgs = readMessages(sm);
    EXPECT_EQ(msgs.size(), 1);
    EXPECT_EQ(msgs[0].type(), raftpb::MsgHeartbeat);
    EXPECT_EQ(msgs[0].context(), ctx);
    EXPECT_EQ(sm->readOnly_->readIndexQueue_.size(), 1);
    EXPECT_EQ(sm->readOnly_->pendingReadIndex_.count(ctx), 1);

	// heartbeat responses from majority of followers (1 in this case)
	// acknowledge the authority of the leader.
	// more info: raft dissertation 6.4, step 3.
    msg.Clear();
    msg.set_from(2);
    msg.set_type(raftpb::MsgHeartbeatResp);
    msg.set_context(ctx);
	sm->Step({msg});
    EXPECT_EQ(sm->readOnly_->readIndexQueue_.size(), 0);
    EXPECT_EQ(sm->readOnly_->pendingReadIndex_.size(), 0);
    EXPECT_EQ(sm->readOnly_->pendingReadIndex_.count(ctx), 0);
}

// TestMsgAppRespWaitReset verifies the resume behavior of a leader
// MsgAppResp.
TEST(raft, TestMsgAppRespWaitReset) {
    auto s = newTestMemoryStorage({withPeers({1, 2, 3})});
	auto sm = newTestRaft(1, 5, 1, s);
	sm->becomeCandidate();
	sm->becomeLeader();

    // Run n1 which includes sending a message like the below
    // one to n2, but also appending to its own log.
    nextEnts(sm, s);

	// Node 2 acks the first entry, making it committed.
    raftpb::Message msg;
    msg.set_from(2);
    msg.set_type(raftpb::MsgAppResp);
    msg.set_index(1);
	sm->Step(msg);
    EXPECT_EQ(sm->raftLog_->committed_, 1);
	// Also consume the MsgApp messages that update Commit on the followers.
	readMessages(sm);

	// A new command is now proposed on node 1.
    msg.Clear();
    msg.set_from(1);
    msg.set_type(raftpb::MsgProp);
    msg.mutable_entries()->Add();
    sm->Step(msg);

	// The command is broadcast to all nodes not in the wait state.
	// Node 2 left the wait state due to its MsgAppResp, but node 3 is still waiting.
	auto msgs = readMessages(sm);
    EXPECT_EQ(msgs.size(), 1);
    EXPECT_FALSE(msgs[0].type() != raftpb::MsgApp || msgs[0].to() != 2);
    EXPECT_FALSE(msgs[0].entries().size() != 1 || msgs[0].entries(0).index() != 2);

	// Now Node 3 acks the first entry. This releases the wait and entry 2 is sent.
    msg.Clear();
    msg.set_from(3);
    msg.set_type(raftpb::MsgAppResp);
    msg.set_index(1);
	sm->Step(msg);
	msgs = readMessages(sm);
    EXPECT_EQ(msgs.size(), 1);
    EXPECT_FALSE(msgs[0].type() != raftpb::MsgApp || msgs[0].to() != 3);
    EXPECT_FALSE(msgs[0].entries().size() != 1 || msgs[0].entries(0).index() != 2);
}

void testRecvMsgVote(raftpb::MessageType msgType) {
    struct Test {
        StateType state;
        uint64_t index;
        uint64_t logTerm;
        uint64_t voteFor;
        bool wreject;
    };
    std::vector<Test> tests = {
		{StateFollower, 0, 0, None, true},
		{StateFollower, 0, 1, None, true},
		{StateFollower, 0, 2, None, true},
		{StateFollower, 0, 3, None, false},

		{StateFollower, 1, 0, None, true},
		{StateFollower, 1, 1, None, true},
		{StateFollower, 1, 2, None, true},
		{StateFollower, 1, 3, None, false},

		{StateFollower, 2, 0, None, true},
		{StateFollower, 2, 1, None, true},
		{StateFollower, 2, 2, None, false},
		{StateFollower, 2, 3, None, false},

		{StateFollower, 3, 0, None, true},
		{StateFollower, 3, 1, None, true},
		{StateFollower, 3, 2, None, false},
		{StateFollower, 3, 3, None, false},

		{StateFollower, 3, 2, 2, false},
		{StateFollower, 3, 2, 1, true},

		{StateLeader, 3, 3, 1, true},
		{StatePreCandidate, 3, 3, 1, true},
		{StateCandidate, 3, 3, 1, true},
	};

    for (auto& tt : tests) {
		auto sm = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1})}));
		sm->state_ = tt.state;
		switch (tt.state) {
		case StateFollower:
			sm->step_ = stepFollower;
                break;
        case StateCandidate:
        case StatePreCandidate:
			sm->step_ = stepCandidate;
            break;
		case StateLeader:
			sm->step_ = stepLeader;
            break;
        case numStates:
            EXPECT_FALSE(true);
            break;
        }
		sm->vote_ = tt.voteFor;

        auto log = std::make_shared<RaftLog>();
        auto ms = std::make_shared<MemoryStorage>();
        ms->ents_ = {{}, EntryHelper{.index = 1,.term = 2}.Done(), EntryHelper{.index = 2,.term = 2}.Done()};
        log->storage_ = ms;
        log->unstable_.offset_ = 3;
        sm->raftLog_ = log;

		// raft.Term is greater than or equal to raft.raftLog.lastTerm. In this
		// test we're only testing MsgVote responses when the campaigning node
		// has a different raft log compared to the recipient node.
		// Additionally we're verifying behaviour when the recipient node has
		// already given out its vote for its current term. We're not testing
		// what the recipient node does when receiving a message with a
		// different term number, so we simply initialize both term numbers to
		// be the same.
		auto term = std::max(sm->raftLog_->lastTerm(), tt.logTerm);
		sm->term_ = term;
        raftpb::Message msg;
        msg.set_type(msgType);
        msg.set_term(term);
        msg.set_from(2);
        msg.set_index(tt.index);
        msg.set_logterm(tt.logTerm);
		sm->Step(msg);

		auto msgs = readMessages(sm);
        EXPECT_EQ(msgs.size(), 1);
        EXPECT_EQ(msgs[0].type(), voteRespMsgType(msgType));
        EXPECT_EQ(msgs[0].reject(), tt.wreject);
	}
}

TEST(raft, TestRecvMsgVote) {
    testRecvMsgVote(raftpb::MsgVote);
}

TEST(raft, TestRecvMsgPreVote) {
    testRecvMsgVote(raftpb::MsgPreVote);
}

TEST(raft, TestStateTransition) {
    struct Test {
        StateType from;
        StateType to;
        bool wallow;
        uint64_t wterm;
        uint64_t wlead;
    };
    std::vector<Test> tests = {
		{StateFollower, StateFollower, true, 1, None},
		{StateFollower, StatePreCandidate, true, 0, None},
		{StateFollower, StateCandidate, true, 1, None},
		{StateFollower, StateLeader, false, 0, None},

		{StatePreCandidate, StateFollower, true, 0, None},
		{StatePreCandidate, StatePreCandidate, true, 0, None},
		{StatePreCandidate, StateCandidate, true, 1, None},
		{StatePreCandidate, StateLeader, true, 0, 1},

		{StateCandidate, StateFollower, true, 0, None},
		{StateCandidate, StatePreCandidate, true, 0, None},
		{StateCandidate, StateCandidate, true, 1, None},
		{StateCandidate, StateLeader, true, 0, 1},

		{StateLeader, StateFollower, true, 1, None},
		{StateLeader, StatePreCandidate, false, 0, None},
		{StateLeader, StateCandidate, false, 1, None},
		{StateLeader, StateLeader, true, 0, 1},
	};

    for (auto& tt : tests) {
        try {
            auto sm = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1})}));
            sm->state_ = tt.from;
            switch (tt.to) {
                case StateFollower:
                    sm->becomeFollower(tt.wterm, tt.wlead);
                    break;
                case StatePreCandidate:
                    sm->becomePreCandidate();
                    break;
                case StateCandidate:
                    sm->becomeCandidate();
                    break;
                case StateLeader:
                    sm->becomeLeader();
                    break;
                case numStates:
                    break;
            }
            EXPECT_EQ(sm->term_, tt.wterm);
            EXPECT_EQ(sm->lead_, tt.wlead);
        } catch(const std::exception& e) {
            EXPECT_EQ(typeid(e), typeid(PanicException));
            EXPECT_FALSE(tt.wallow);
        }
	}
}

TEST(raft, TestAllServerStepdown) {
    struct Test {
        StateType state;
        StateType wstate;
        uint64_t wterm;
        uint64_t windex;
    };
    std::vector<Test> tests = {
		{StateFollower, StateFollower, 3, 0},
		{StatePreCandidate, StateFollower, 3, 0},
		{StateCandidate, StateFollower, 3, 0},
		{StateLeader, StateFollower, 3, 1},
	};

    std::vector<raftpb::MessageType> tmsgTypes = {raftpb::MsgVote, raftpb::MsgApp};
	uint64_t tterm = 3;
    for (auto& tt : tests) {
		auto sm = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
		switch (tt.state) {
		case StateFollower:
			sm->becomeFollower(1, None);
            break;
		case StatePreCandidate:
			sm->becomePreCandidate();
            break;
		case StateCandidate:
			sm->becomeCandidate();
            break;
		case StateLeader:
			sm->becomeCandidate();
			sm->becomeLeader();
            break;
        case numStates:
            break;
        }

        for (auto msgType : tmsgTypes) {
            raftpb::Message msg;
            msg.set_from(2);
            msg.set_type(msgType);
            msg.set_term(tterm);
            msg.set_logterm(tterm);
			sm->Step(msg);

            EXPECT_EQ(sm->state_, tt.wstate);
            EXPECT_EQ(sm->term_, tt.wterm);
            EXPECT_EQ(sm->raftLog_->lastIndex(), tt.windex);
            EXPECT_EQ(sm->raftLog_->allEntries().size(), tt.windex);
			uint64_t wlead = 2;
			if (msgType == raftpb::MsgVote) {
				wlead = None;
			}
            EXPECT_EQ(sm->lead_, wlead);
		}
	}
}

// testCandidateResetTerm tests when a candidate receives a
// MsgHeartbeat or MsgApp from leader, "Step" resets the term
// with leader's and reverts back to follower.
void testCandidateResetTerm(raftpb::MessageType mt) {
	auto a = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto b = newTestRaft(2, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto c = newTestRaft(3, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));

	auto nt = newNetwork({std::make_shared<raftWarp>(a), std::make_shared<raftWarp>(b), std::make_shared<raftWarp>(c)});

	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});
    EXPECT_EQ(a->state_, StateLeader);
    EXPECT_EQ(b->state_, StateFollower);
    EXPECT_EQ(c->state_, StateFollower);

	// isolate 3 and increase term in rest
	nt->isolate(3);

	nt->send({MessageHelper{.from = 2,.to = 2,.type = raftpb::MsgHup}.Done()});
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

    EXPECT_EQ(a->state_, StateLeader);
    EXPECT_EQ(b->state_, StateFollower);

	// trigger campaign in isolated c
	c->resetRandomizedElectionTimeout();
	for (auto i = 0; i < c->randomizedElectionTimeout_; i++) {
		c->tick_();
	}
    advanceMessagesAfterAppend(c);

    EXPECT_EQ(c->state_, StateCandidate);

	nt->recover();

	// leader sends to isolated candidate
	// and expects candidate to revert to follower
	nt->send({MessageHelper{.from = 1,.to = 3,.term = a->term_, .type = mt}.Done()});

    EXPECT_EQ(c->state_, StateFollower);

	// follower c term is reset with leader's
    EXPECT_EQ(a->term_, c->term_);
}

TEST(raft, TestCandidateResetTermMsgHeartbeat) {
	testCandidateResetTerm(raftpb::MsgHeartbeat);
}

TEST(raft, TestCandidateResetTermMsgApp) {
	testCandidateResetTerm(raftpb::MsgApp);
}

void testCandidateSelfVoteAfterLostElection(bool preVote) {
	auto sm = newTestRaft(1, 5, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	sm->preVote_ = preVote;;

	// n1 calls an election.
	sm->Step(MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done());
	auto steps = takeMessagesAfterAppend(sm);

	// n1 hears that n2 already won the election before it has had a
	// change to sync its vote to disk and account for its self-vote.
	// Becomes a follower.
    auto m = MessageHelper{.from = 2,.to = 1, .type = raftpb::MsgHeartbeat}.Done();
    m.set_term(sm->term_);
	sm->Step(m);
    EXPECT_EQ(sm->state_, StateFollower);

	// n1 remains a follower even after its self-vote is delivered.
	stepOrSend(sm, steps);
    EXPECT_EQ(sm->state_, StateFollower);

	// Its self-vote does not make its way to its ProgressTracker.
    size_t granted = 0;
	std::tie(granted, std::ignore, std::ignore) = sm->trk_.TallyVotes();
    EXPECT_EQ(granted, 0);
}

// The following three tests exercise the behavior of a (pre-)candidate when its
// own self-vote is delivered back to itself after the peer has already learned
// that it has lost the election. The self-vote should be ignored in these cases.
TEST(raft, TestCandidateSelfVoteAfterLostElection) {
    testCandidateSelfVoteAfterLostElection(false);
}

TEST(raft, TestCandidateSelfVoteAfterLostElectionPreVote) {
    testCandidateSelfVoteAfterLostElection(true);
}

TEST(raft, TestCandidateDeliversPreCandidateSelfVoteAfterBecomingCandidate) {
	auto sm = newTestRaft(1, 5, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	sm->preVote_ = true;

	// n1 calls an election.
	sm->Step(MessageHelper{.from = 1, .to = 1, .type = raftpb::MsgHup}.Done());
    EXPECT_EQ(sm->state_, StatePreCandidate);
	auto steps = takeMessagesAfterAppend(sm);

	// n1 receives pre-candidate votes from both other peers before
	// voting for itself. n1 becomes a candidate.
	// NB: pre-vote messages carry the local term + 1.
    auto m1 = MessageHelper{.from = 2, .to = 1, .type = raftpb::MsgPreVoteResp}.Done();
    m1.set_term(sm->term_ + 1);
	sm->Step(m1);
    auto m2 = MessageHelper{.from = 3, .to = 1, .type = raftpb::MsgPreVoteResp}.Done();
    m2.set_term(sm->term_ + 1);
	sm->Step(m2);
    EXPECT_EQ(sm->state_, StateCandidate);

	// n1 remains a candidate even after its delayed pre-vote self-vote is
	// delivered.
	stepOrSend(sm, steps);
    EXPECT_EQ(sm->state_, StateCandidate);
	steps = takeMessagesAfterAppend(sm);

	// Its pre-vote self-vote does not make its way to its ProgressTracker.
    size_t granted = 0;
	std::tie(granted, std::ignore, std::ignore) = sm->trk_.TallyVotes();
    EXPECT_EQ(granted, 0);

	// A single vote from n2 does not move n1 to the leader.
    auto m3 = MessageHelper{.from = 2, .to = 1, .type = raftpb::MsgVoteResp}.Done();
    m3.set_term(sm->term_);
    sm->Step(m3);
    EXPECT_EQ(sm->state_, StateCandidate);

	// n1 becomes the leader once its self-vote is received because now
	// quorum is reached.
	stepOrSend(sm, steps);
    EXPECT_EQ(sm->state_, StateLeader);
}
TEST(raft, TestLeaderMsgAppSelfAckAfterTermChange) {
	auto sm = newTestRaft(1, 5, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	sm->becomeCandidate();
	sm->becomeLeader();

	// n1 proposes a write.
    auto m = MessageHelper{.from = 1, .to = 1, .type = raftpb::MsgProp}.Done();
    m.mutable_entries()->Add()->set_data("somedata");
	sm->Step(m);
	auto steps = takeMessagesAfterAppend(sm);

	// n1 hears that n2 is the new leader.
    auto m1 = MessageHelper{.from = 2, .to = 1, .type = raftpb::MsgHeartbeat}.Done();
    m1.set_term(sm->term_ + 1);
	sm->Step(m1);
    EXPECT_EQ(sm->state_, StateFollower);

	// n1 advances, ignoring its earlier self-ack of its MsgApp. The
	// corresponding MsgAppResp is ignored because it carries an earlier term.
	stepOrSend(sm, steps);
    EXPECT_EQ(sm->state_, StateFollower);
}

TEST(raft, TestLeaderStepdownWhenQuorumActive) {
	auto sm = newTestRaft(1, 5, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));

	sm->checkQuorum_ = true;

	sm->becomeCandidate();
	sm->becomeLeader();
	for (int i = 0; i < sm->electionTimeout_+1; i++) {
        raftpb::Message msg;
        msg.set_from(2);
        msg.set_type(raftpb::MsgHeartbeatResp);
        msg.set_term(sm->term_);
		sm->Step(msg);
		sm->tick_();
	}

    EXPECT_EQ(sm->state_, StateLeader);
}

TEST(raft, TestLeaderStepdownWhenQuorumLost) {
	auto sm = newTestRaft(1, 5, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));

	sm->checkQuorum_ = true;

	sm->becomeCandidate();
	sm->becomeLeader();

	for (auto i = 0; i < sm->electionTimeout_+1; i++) {
		sm->tick_();
	}
    EXPECT_EQ(sm->state_, StateFollower);
}

TEST(raft, TestLeaderSupersedingWithCheckQuorum) {
	auto a = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto b = newTestRaft(2, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto c = newTestRaft(3, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));

	a->checkQuorum_ = true;
	b->checkQuorum_ = true;
	c->checkQuorum_ = true;

	auto nt = newNetwork({std::make_shared<raftWarp>(a), std::make_shared<raftWarp>(b), std::make_shared<raftWarp>(c)});
	setRandomizedElectionTimeout(b, b->electionTimeout_+1);

	for (auto i = 0; i < b->electionTimeout_; i++) {
		b->tick_();
	}
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

    EXPECT_EQ(a->state_, StateLeader);

    EXPECT_EQ(c->state_, StateFollower);

	nt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});

	// Peer b rejected c's vote since its electionElapsed had not reached to electionTimeout
    EXPECT_EQ(c->state_, StateCandidate);

	// Letting b's electionElapsed reach to electionTimeout
	for (auto i = 0; i < b->electionTimeout_; i++) {
		b->tick_();
	}

	nt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});

    EXPECT_EQ(c->state_, StateLeader);
}

TEST(raft, TestLeaderElectionWithCheckQuorum) {
	auto a = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto b = newTestRaft(2, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto c = newTestRaft(3, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));

	a->checkQuorum_ = true;
	b->checkQuorum_ = true;
	c->checkQuorum_ = true;

	auto nt = newNetwork({std::make_shared<raftWarp>(a), std::make_shared<raftWarp>(b), std::make_shared<raftWarp>(c)});
	setRandomizedElectionTimeout(a, a->electionTimeout_+1);
	setRandomizedElectionTimeout(b, b->electionTimeout_+2);

	// Immediately after creation, votes are cast regardless of the
	// election timeout.
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

    EXPECT_EQ(a->state_, StateLeader);
    EXPECT_EQ(c->state_, StateFollower);

	// need to reset randomizedElectionTimeout larger than electionTimeout again,
	// because the value might be reset to electionTimeout since the last state changes
	setRandomizedElectionTimeout(a, a->electionTimeout_+1);
	setRandomizedElectionTimeout(b, b->electionTimeout_+2);
	for (auto i = 0; i < a->electionTimeout_; i++) {
		a->tick_();
	}
	for (auto i = 0; i < b->electionTimeout_; i++) {
		b->tick_();
	}
	nt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});

    EXPECT_EQ(a->state_, StateFollower);
    EXPECT_EQ(c->state_, StateLeader);
}

// TestFreeStuckCandidateWithCheckQuorum ensures that a candidate with a higher term
// can disrupt the leader even if the leader still "officially" holds the lease, The
// leader is expected to step down and adopt the candidate's term
TEST(raft, TestFreeStuckCandidateWithCheckQuorum) {
	auto a = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto b = newTestRaft(2, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto c = newTestRaft(3, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));

	a->checkQuorum_ = true;
	b->checkQuorum_ = true;
	c->checkQuorum_ = true;

	auto nt = newNetwork({std::make_shared<raftWarp>(a), std::make_shared<raftWarp>(b), std::make_shared<raftWarp>(c)});
	setRandomizedElectionTimeout(b, b->electionTimeout_+1);

	for (auto i = 0; i < b->electionTimeout_; i++) {
		b->tick_();
	}
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	nt->isolate(1);
	nt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});

    EXPECT_EQ(b->state_, StateFollower);
    EXPECT_EQ(c->state_, StateCandidate);
    EXPECT_EQ(c->term_, b->term_+1);

	// Vote again for safety
	nt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});

    EXPECT_EQ(b->state_, StateFollower);
    EXPECT_EQ(c->state_, StateCandidate);
    EXPECT_EQ(c->term_, b->term_+2);

	nt->recover();

    auto msg = MessageHelper{.from = 1,.to = 3,.type = raftpb::MsgHeartbeat}.Done();
    msg.set_term(a->term_);
	nt->send({msg});

	// Disrupt the leader so that the stuck peer is freed
    EXPECT_EQ(a->state_, StateFollower);
    EXPECT_EQ(c->term_, a->term_);

	// Vote again, should become leader this time
	nt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});

    EXPECT_EQ(c->state_, StateLeader);
}

TEST(raft, TestNonPromotableVoterWithCheckQuorum) {
	auto a = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2})}));
	auto b = newTestRaft(2, 10, 1, newTestMemoryStorage({withPeers({1})}));

	a->checkQuorum_ = true;
	b->checkQuorum_ = true;

	auto nt = newNetwork({std::make_shared<raftWarp>(a), std::make_shared<raftWarp>(b)});
	setRandomizedElectionTimeout(b, b->electionTimeout_+1);
	// Need to remove 2 again to make it a non-promotable node since newNetwork overwritten some internal states
    raftpb::ConfChange cc;
    cc.set_type(raftpb::ConfChangeRemoveNode);
    cc.set_node_id(2);
    raftpb::ConfChangeWrap ccw(cc);
	b->applyConfChange(ccw.AsV2());

    EXPECT_FALSE(b->promotable());

	for (auto i = 0; i < b->electionTimeout_; i++) {
		b->tick_();
	}

	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

    EXPECT_EQ(a->state_, StateLeader);
    EXPECT_EQ(b->state_, StateFollower);
    EXPECT_EQ(b->lead_, 1);
}

// TestDisruptiveFollower tests isolated follower,
// with slow network incoming from leader, election times out
// to become a candidate with an increased term. Then, the
// candiate's response to late leader heartbeat forces the leader
// to step down.
TEST(raft, TestDisruptiveFollower) {
	auto n1 = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto n2 = newTestRaft(2, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto n3 = newTestRaft(3, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));

	n1->checkQuorum_ = true;
	n2->checkQuorum_ = true;
	n3->checkQuorum_ = true;

	n1->becomeFollower(1, None);
	n2->becomeFollower(1, None);
	n3->becomeFollower(1, None);

	auto nt = newNetwork({std::make_shared<raftWarp>(n1), std::make_shared<raftWarp>(n2), std::make_shared<raftWarp>(n3)});

	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	// check state
	// n1.state == StateLeader
	// n2.state == StateFollower
	// n3.state == StateFollower
    EXPECT_EQ(n1->state_, StateLeader);
    EXPECT_EQ(n2->state_, StateFollower);
    EXPECT_EQ(n3->state_, StateFollower);

	// etcd server "advanceTicksForElection" on restart;
	// this is to expedite campaign trigger when given larger
	// election timeouts (e.g. multi-datacenter deploy)
	// Or leader messages are being delayed while ticks elapse
	setRandomizedElectionTimeout(n3, n3->electionTimeout_+2);
	for (auto i = 0; i < n3->randomizedElectionTimeout_-1; i++) {
		n3->tick_();
	}

	// ideally, before last election tick elapses,
	// the follower n3 receives "pb.MsgApp" or "pb.MsgHeartbeat"
	// from leader n1, and then resets its "electionElapsed"
	// however, last tick may elapse before receiving any
	// messages from leader, thus triggering campaign
	n3->tick_();

	// n1 is still leader yet
	// while its heartbeat to candidate n3 is being delayed

	// check state
	// n1.state == StateLeader
	// n2.state == StateFollower
	// n3.state == StateCandidate
    EXPECT_EQ(n1->state_, StateLeader);
    EXPECT_EQ(n2->state_, StateFollower);
    EXPECT_EQ(n3->state_, StateCandidate);
	// check term
	// n1.Term == 2
	// n2.Term == 2
	// n3.Term == 3
    EXPECT_EQ(n1->term_, 2);
    EXPECT_EQ(n2->term_, 2);
    EXPECT_EQ(n3->term_, 3);

	// while outgoing vote requests are still queued in n3,
	// leader heartbeat finally arrives at candidate n3
	// however, due to delayed network from leader, leader
	// heartbeat was sent with lower term than candidate's
	nt->send({MessageHelper{.from = 1,.to = 3,.term = n1->term_, .type = raftpb::MsgHeartbeat}.Done()});

	// then candidate n3 responds with "pb.MsgAppResp" of higher term
	// and leader steps down from a message with higher term
	// this is to disrupt the current leader, so that candidate
	// with higher term can be freed with following election

	// check state
	// n1.state == StateFollower
	// n2.state == StateFollower
	// n3.state == StateCandidate
    EXPECT_EQ(n1->state_, StateFollower);
    EXPECT_EQ(n2->state_, StateFollower);
    EXPECT_EQ(n3->state_, StateCandidate);
	// check term
	// n1.Term == 3
	// n2.Term == 2
	// n3.Term == 3
    EXPECT_EQ(n1->term_, 3);
    EXPECT_EQ(n2->term_, 2);
    EXPECT_EQ(n3->term_, 3);
}

// TestDisruptiveFollowerPreVote tests isolated follower,
// with slow network incoming from leader, election times out
// to become a pre-candidate with less log than current leader.
// Then pre-vote phase prevents this isolated node from forcing
// current leader to step down, thus less disruptions.
TEST(raft, TestDisruptiveFollowerPreVote) {
	auto n1 = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto n2 = newTestRaft(2, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto n3 = newTestRaft(3, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));

	n1->checkQuorum_ = true;
	n2->checkQuorum_ = true;
	n3->checkQuorum_ = true;

	n1->becomeFollower(1, None);
	n2->becomeFollower(1, None);
	n3->becomeFollower(1, None);

	auto nt = newNetwork({std::make_shared<raftWarp>(n1), std::make_shared<raftWarp>(n2), std::make_shared<raftWarp>(n3)});

	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	// check state
	// n1.state == StateLeader
	// n2.state == StateFollower
	// n3.state == StateFollower
    EXPECT_EQ(n1->state_, StateLeader);
    EXPECT_EQ(n2->state_, StateFollower);
    EXPECT_EQ(n2->state_, StateFollower);
    EXPECT_EQ(n3->state_, StateFollower);

	nt->isolate(3);
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "somedata"}.Done()}}.Done()});
    nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "somedata"}.Done()}}.Done()});
    nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "somedata"}.Done()}}.Done()});

	n1->preVote_ = true;
	n2->preVote_ = true;
	n3->preVote_ = true;
	nt->recover();
	nt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});

	// check state
	// n1.state == StateLeader
	// n2.state == StateFollower
	// n3.state == StatePreCandidate
    EXPECT_EQ(n1->state_, StateLeader);
    EXPECT_EQ(n2->state_, StateFollower);
    EXPECT_EQ(n3->state_, StatePreCandidate);
	// check term
	// n1.Term == 2
	// n2.Term == 2
	// n3.Term == 2
    EXPECT_EQ(n1->term_, 2);
    EXPECT_EQ(n2->term_, 2);
    EXPECT_EQ(n3->term_, 2);

	// delayed leader heartbeat does not force current leader to step down
    auto msg = MessageHelper{.from = 1,.to = 3,.type = raftpb::MsgHeartbeat}.Done();
    msg.set_term(n1->term_);
	nt->send({msg});
    EXPECT_EQ(n1->state_, StateLeader);
}

TEST(raft, TestReadOnlyOptionSafe) {
	auto a = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto b = newTestRaft(2, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto c = newTestRaft(3, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));

	auto nt = newNetwork({std::make_shared<raftWarp>(a), std::make_shared<raftWarp>(b), std::make_shared<raftWarp>(c)});
	setRandomizedElectionTimeout(b, b->electionTimeout_+1);

	for (auto i = 0; i < b->electionTimeout_; i++) {
		b->tick_();
	}
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

    EXPECT_EQ(a->state_, StateLeader);
    struct Test {
        std::shared_ptr<raft> sm;
        int proposals;
        uint64_t wri;
        std::string wctx;
    };
    std::vector<Test> tests = {
		{a, 10, 11, "ctx1"},
		{b, 10, 21, "ctx2"},
		{c, 10, 31, "ctx3"},
		{a, 10, 41, "ctx4"},
		{b, 10, 51, "ctx5"},
		{c, 10, 61, "ctx6"}
	};

    for (auto& tt : tests) {
		for (auto j = 0; j < tt.proposals; j++) {
			nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {{}}}.Done()});
		}

		nt->send({{MessageHelper{.from = tt.sm->id_,.to = tt.sm->id_,.type = raftpb::MsgReadIndex,.entries = {EntryHelper{.data = tt.wctx}.Done()}}.Done()}});

		auto r = tt.sm;
        EXPECT_NE(r->readStates_.size(), 0);
		auto rs = r->readStates_[0];
        EXPECT_EQ(rs.index_, tt.wri);
        EXPECT_EQ(rs.requestCtx_, tt.wctx);
		r->readStates_.clear();
	}
}

TEST(raft, TestReadOnlyWithLearner) {
    auto s = newTestMemoryStorage({withPeers({1}), withLearners({2})});
	auto a = newTestLearnerRaft(1, 10, 1, s);
	auto b = newTestLearnerRaft(2, 10, 1, newTestMemoryStorage({withPeers({1}), withLearners({2})}));

	auto nt = newNetwork({std::make_shared<raftWarp>(a), std::make_shared<raftWarp>(b)});
	setRandomizedElectionTimeout(b, b->electionTimeout_+1);

	for (auto i = 0; i < b->electionTimeout_; i++) {
		b->tick_();
	}
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});
    EXPECT_EQ(a->state_, StateLeader);
    struct Test {
        std::shared_ptr<raft> sm;
        int proposals;
        uint64_t wri;
        std::string wctx;
    };
    std::vector<Test> tests = {
		{a, 10, 11, "ctx1"},
		{b, 10, 21, "ctx2"},
		{a, 10, 31, "ctx3"},
		{b, 10, 41, "ctx4"}
	};
    for (auto& tt : tests) {
		for (auto j = 0; j < tt.proposals; j++) {
            nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp,.entries = {{}}}.Done()});
            nextEnts(a, s); // append the entries on the leader
		}
        nt->send({{MessageHelper{.from = tt.sm->id_,.to = tt.sm->id_,.type = raftpb::MsgReadIndex,.entries = {EntryHelper{.data = tt.wctx}.Done()}}.Done()}});

		auto r = tt.sm;
        EXPECT_NE(r->readStates_.size(), 0);
		auto rs = r->readStates_[0];
        EXPECT_EQ(rs.index_, tt.wri);
        EXPECT_EQ(rs.requestCtx_, tt.wctx);
		r->readStates_.clear();
	}
}

TEST(raft, TestReadOnlyOptionLease) {
	auto a = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto b = newTestRaft(2, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto c = newTestRaft(3, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	a->readOnly_->option_ = ReadOnlyLeaseBased;
	b->readOnly_->option_ = ReadOnlyLeaseBased;
	c->readOnly_->option_ = ReadOnlyLeaseBased;
	a->checkQuorum_ = true;
	b->checkQuorum_ = true;
	c->checkQuorum_ = true;

	auto nt = newNetwork({std::make_shared<raftWarp>(a), std::make_shared<raftWarp>(b), std::make_shared<raftWarp>(c)});
	setRandomizedElectionTimeout(b, b->electionTimeout_+1);

	for (auto i = 0; i < b->electionTimeout_; i++) {
		b->tick_();
	}
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

    EXPECT_EQ(a->state_, StateLeader);
    struct Test {
        std::shared_ptr<raft> sm;
        int proposals;
        uint64_t wri;
        std::string wctx;
    };
    std::vector<Test> tests = {
		{a, 10, 11, "ctx1"},
		{b, 10, 21, "ctx2"},
		{c, 10, 31, "ctx3"},
		{a, 10, 41, "ctx4"},
		{b, 10, 51, "ctx5"},
		{c, 10, 61, "ctx6"},
	};
    for (auto& tt : tests) {
		for (auto j = 0; j < tt.proposals; j++) {
            nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp,.entries = {{}}}.Done()});
		}
        nt->send({{MessageHelper{.from = tt.sm->id_,.to = tt.sm->id_,.type = raftpb::MsgReadIndex,.entries = {EntryHelper{.data = tt.wctx}.Done()}}.Done()}});

		auto r = tt.sm;
		auto rs = r->readStates_[0];
        EXPECT_EQ(rs.index_, tt.wri);
        EXPECT_EQ(rs.requestCtx_, tt.wctx);
		r->readStates_.clear();
	}
}

// TestReadOnlyForNewLeader ensures that a leader only accepts MsgReadIndex message
// when it commits at least one log entry at it term.
TEST(raft, TestReadOnlyForNewLeader) {
    struct Config {
        uint64_t id;
        uint64_t committed;
        uint64_t applied;
        uint64_t compactIndex;
    };
	std::vector<Config> nodeConfigs = {
		{1, 1, 1, 0},
		{2, 2, 2, 2},
		{3, 2, 2, 2},
	};
	//peers := make([]stateMachine, 0)
    std::vector<std::shared_ptr<stateMachine>> peers;
    for (auto& c : nodeConfigs) {
		auto storage = newTestMemoryStorage({withPeers({1, 2, 3})});
		storage->Append({EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1}.Done()});
        raftpb::HardState hs;
        hs.set_term(1);
        hs.set_commit(c.committed);
		storage->SetHardState(hs);
		if (c.compactIndex != 0) {
			storage->Compact(c.compactIndex);
		}
		auto cfg = newTestConfig(c.id, 10, 1, storage);
		cfg.applied_ = c.applied;
		auto raft = newRaft(cfg);
        peers.push_back(std::make_shared<raftWarp>(raft));
	}
	auto nt = newNetwork(peers);

	// Drop MsgApp to forbid peer a to commit any log entry at its term after it becomes leader.
	nt->ignore(raftpb::MsgApp);
	// Force peer a to become leader.
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

    auto sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));
    EXPECT_EQ(sm->raft_->state_, StateLeader);

	// Ensure peer a drops read only request.
	uint64_t windex = 4;
	std::string wctx = "ctx";
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgReadIndex, .entries = {{EntryHelper{.data = wctx}.Done()}}}.Done()});
    EXPECT_EQ(sm->raft_->readStates_.size(), 0);

	nt->recover();

	// Force peer a to commit a log entry at its term
	for (auto i = 0; i < sm->raft_->heartbeatTimeout_; i++) {
		sm->raft_->tick_();
	}
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {{}}}.Done()});
    EXPECT_EQ(sm->raft_->raftLog_->committed_, 4);
	auto lastLogTerm = sm->raft_->raftLog_->zeroTermOnOutOfBounds(sm->raft_->raftLog_->term(sm->raft_->raftLog_->committed_));
    EXPECT_EQ(lastLogTerm, sm->raft_->term_);
	// Ensure peer a processed postponed read only request after it committed an entry at its term.
    EXPECT_EQ(sm->raft_->readStates_.size(), 1);
	auto rs = sm->raft_->readStates_[0];
    EXPECT_EQ(rs.index_, windex);
    EXPECT_EQ(rs.requestCtx_, wctx);

	// Ensure peer a accepts read only request after it committed an entry at its term.
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgReadIndex, .entries = {{EntryHelper{.data = wctx}.Done()}}}.Done()});
    EXPECT_EQ(sm->raft_->readStates_.size(), 2);
	rs = sm->raft_->readStates_[1];
    EXPECT_EQ(rs.index_, windex);
    EXPECT_EQ(rs.requestCtx_, wctx);
}

TEST(raft, TestLeaderAppResp) {
	// initial progress: match = 0; next = 3
    struct Test {
        uint64_t index;
        bool reject;
        // progress
        uint64_t wmatch;
        uint64_t wnext;
        // message
        int wmsgNum;
        uint64_t windex;
        uint64_t wcommitted;
    };
    std::vector<Test> tests = {
		{3, true, 0, 3, 0, 0, 0},  // stale resp; no replies
		{2, true, 0, 2, 1, 1, 0},  // denied resp; leader does not commit; decrease next and send probing msg
		{2, false, 2, 4, 2, 2, 2}, // accept resp; leader commits; broadcast with commit index
        // Follower is StateProbing at 0, it sends MsgAppResp for 0 (which
        // matches the pr.Match) so it is moved to StateReplicate and as many
        // entries as possible are sent to it (1, 2, and 3). Correspondingly the
        // Next is then 4 (an Entry at 4 does not exist, indicating the follower
        // will be up to date should it process the emitted MsgApp).
        {0, false, 0, 4, 1, 0, 0},
	};
	for (auto& tt : tests) {
		// sm term is 1 after it becomes the leader.
		// thus the last log term must be 1 to be committed.
		auto sm = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
        auto wantLog = std::make_shared<RaftLog>();
        auto ms = std::make_shared<MemoryStorage>();
        ms->ents_ = {{}, EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1}.Done()};
        wantLog->storage_ = ms;
        wantLog->unstable_.offset_ = 3;
        sm->raftLog_ = wantLog;
		sm->becomeCandidate();
		sm->becomeLeader();
		readMessages(sm);
        raftpb::Message msg;
        msg.set_from(2);
        msg.set_type(raftpb::MsgAppResp);
        msg.set_index(tt.index);
        msg.set_term(sm->term_);
        msg.set_reject(tt.reject);
        msg.set_rejecthint(tt.index);
        EXPECT_EQ(sm->Step({msg}), nullptr);

		auto p = sm->trk_.progress_[2];
        EXPECT_EQ(p->match_, tt.wmatch);
        EXPECT_EQ(p->next_, tt.wnext);
		auto msgs = readMessages(sm);
        EXPECT_EQ(msgs.size(), tt.wmsgNum);
        for (auto& m : msgs) {
            EXPECT_EQ(m.index(), tt.windex);
            EXPECT_EQ(m.commit(), tt.wcommitted);
		}
	}
}

// TestBcastBeat is when the leader receives a heartbeat tick, it should
// send a MsgHeartbeat with m.Index = 0, m.LogTerm=0 and empty entries.
TEST(raft, TestBcastBeat) {
    uint64_t offset = 1000;
	// make a state machine with log.offset = 1000
    raftpb::Snapshot s;
    auto meta = s.mutable_metadata();
    meta->set_index(offset);
    meta->set_term(1);
    std::vector<uint64_t> voters = {1,2,3};
    *meta->mutable_conf_state()->mutable_voters() = {voters.begin(), voters.end()};

	auto storage = NewMemoryStorage();
	storage->ApplySnapshot(s);
	auto sm = newTestRaft(1, 10, 1, storage);
	sm->term_ = 1;

	sm->becomeCandidate();
	sm->becomeLeader();
	for (size_t i = 0; i < 10; i++) {
		mustAppendEntry(sm, {EntryHelper{.index = i+1}.Done()});
	}
    advanceMessagesAfterAppend(sm);
	// slow follower
	sm->trk_.progress_[2]->match_ = 5;
    sm->trk_.progress_[2]->next_ = 6;
	// normal follower
    sm->trk_.progress_[3]->match_ = sm->raftLog_->lastIndex();
    sm->trk_.progress_[3]->next_ = sm->raftLog_->lastIndex() + 1;

    raftpb::Message msg;
    msg.set_type(raftpb::MsgBeat);
	sm->Step(msg);
	auto msgs = readMessages(sm);
    EXPECT_EQ(msgs.size(), 2);

    std::unordered_map<uint64_t, uint64_t> wantCommitMap = {
            {2, std::min(sm->raftLog_->committed_, sm->trk_.progress_[2]->match_)},
            {3, std::min(sm->raftLog_->committed_, sm->trk_.progress_[3]->match_)},
	};
    for (auto& m : msgs) {
        EXPECT_EQ(m.type(), raftpb::MsgHeartbeat);
        EXPECT_EQ(m.index(), 0);
        EXPECT_EQ(m.logterm(), 0);
        EXPECT_NE(wantCommitMap.at(m.to()), 0);
        EXPECT_EQ(m.commit(), wantCommitMap.at(m.to()));
        wantCommitMap.erase(m.to());
        EXPECT_EQ(m.entries().size(), 0);
	}
}

// TestRecvMsgBeat tests the output of the state machine when receiving MsgBeat
TEST(raft, TestRecvMsgBeat) {
    struct Test {
        StateType state;
        int wMsg;
    };
    std::vector<Test> tests = {
		{StateLeader, 2},
		// candidate and follower should ignore MsgBeat
		{StateCandidate, 0},
		{StateFollower, 0},
	};
    for (auto& tt : tests) {
		auto sm = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));

        auto log = std::make_shared<RaftLog>();
        auto ms = std::make_shared<MemoryStorage>();
        ms->ents_ = {{}, EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1}.Done()};
        log->storage_ = ms;
		sm->raftLog_ = log;
		sm->term_ = 1;
		sm->state_ = tt.state;
		switch (tt.state) {
		case StateFollower:
			sm->step_ = stepFollower;
            break;
		case StateCandidate:
			sm->step_ = stepCandidate;
            break;
		case StateLeader:
			sm->step_ = stepLeader;
            break;
        default:
            break;
        }
		sm->Step({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgBeat}.Done()});

		auto msgs = readMessages(sm);
        EXPECT_EQ(msgs.size(), tt.wMsg);
        for (auto& m : msgs) {
            EXPECT_EQ(m.type(), raftpb::MsgHeartbeat);
		}
	}
}

TEST(raft, TestLeaderIncreaseNext) {
    std::vector<raftpb::Entry> previousEnts = {EntryHelper{.index = 1,.term = 1}.Done(),EntryHelper{.index = 1,.term = 2}.Done(),EntryHelper{.index = 1,.term = 3}.Done()};
    struct Test {
        tracker::StateType state;
        uint64_t next;
        uint64_t wnext;
    };
    std::vector<Test> tests = {
		// state replicate, optimistically increase next
		// previous entries + noop entry + propose + 1
		{tracker::StateType::StateReplicate, 2, previousEnts.size() + 1 + 1 + 1},
		// state probe, not optimistically increase next
		{tracker::StateType::StateProbe, 2, 2}
	};
    for (auto& tt : tests) {
		auto sm = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2})}));
		sm->raftLog_->append(previousEnts);
		sm->becomeCandidate();
		sm->becomeLeader();
		sm->trk_.progress_[2]->state_ = tt.state;
		sm->trk_.progress_[2]->next_ = tt.next;
		sm->Step({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "somedata"}.Done()}}.Done()});

		auto p = sm->trk_.progress_[2];
        EXPECT_EQ(p->next_, tt.wnext);
	}
}

TEST(raft, TestSendAppendForProgressProbe) {
	auto r = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2})}));
	r->becomeCandidate();
	r->becomeLeader();
	readMessages(r);
	r->trk_.progress_[2]->BecomeProbe();

	// each round is a heartbeat
	for (auto i = 0; i < 3; i++) {
		if (i == 0) {
			// we expect that raft will only send out one msgAPP on the first
			// loop. After that, the follower is paused until a heartbeat response is
			// received.
			mustAppendEntry(r, {EntryHelper{.data = "somedata"}.Done()});
			r->sendAppend(2);
			auto msg = readMessages(r);
            EXPECT_EQ(msg.size(), 1);
            EXPECT_EQ(msg[0].index(), 0);
		}
        EXPECT_TRUE(r->trk_.progress_[2]->msgAppFlowPaused_);
		for (auto j = 0; j < 10; j++) {
			mustAppendEntry(r, {EntryHelper{.data = "somedata"}.Done()});
			r->sendAppend(2);
            EXPECT_EQ(readMessages(r).size(), 0);
		}

		// do a heartbeat
		for (auto j = 0; j < r->heartbeatTimeout_; j++) {
			r->Step({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgBeat}.Done()});
		}
        EXPECT_TRUE(r->trk_.progress_[2]->msgAppFlowPaused_);

		// consume the heartbeat
		auto msg = readMessages(r);
        EXPECT_EQ(msg.size(), 1);
        EXPECT_EQ(msg[0].type(), raftpb::MsgHeartbeat);
	}

	// a heartbeat response will allow another message to be sent
	r->Step({MessageHelper{.from = 2,.to = 1,.type = raftpb::MsgHeartbeatResp}.Done()});
	auto msg = readMessages(r);
    EXPECT_EQ(msg.size(), 1);
    EXPECT_EQ(msg[0].index(), 0);
    EXPECT_TRUE(r->trk_.progress_[2]->msgAppFlowPaused_);
}

TEST(raft, TestSendAppendForProgressReplicate) {
	auto r = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2})}));
	r->becomeCandidate();
	r->becomeLeader();
	readMessages(r);
	r->trk_.progress_[2]->BecomeReplicate();

	for (auto i = 0; i < 10; i++) {
		mustAppendEntry(r, {EntryHelper{.data = "somedata"}.Done()});
		r->sendAppend(2);
		auto msgs = readMessages(r);
        EXPECT_EQ(msgs.size(), 1);
	}
}

TEST(raft, TestSendAppendForProgressSnapshot) {
	auto r = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2})}));
	r->becomeCandidate();
	r->becomeLeader();
	readMessages(r);
	r->trk_.progress_[2]->BecomeSnapshot(10);

	for (auto i = 0; i < 10; i++) {
		mustAppendEntry(r, {EntryHelper{.data = "somdata"}.Done()});
		r->sendAppend(2);
		auto msgs = readMessages(r);
        EXPECT_EQ(msgs.size(), 0);
	}
}

TEST(raft, TestRecvMsgUnreachable) {
    std::vector<raftpb::Entry> previousEnts = {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1}.Done(), EntryHelper{.index = 3,.term = 1}.Done()};
	auto s = newTestMemoryStorage({withPeers({1, 2})});
	s->Append(previousEnts);
	auto r = newTestRaft(1, 10, 1, s);
	r->becomeCandidate();
	r->becomeLeader();
	readMessages(r);
	// set node 2 to state replicate
	r->trk_.progress_[2]->match_ = 3;
	r->trk_.progress_[2]->BecomeReplicate();
	r->trk_.progress_[2]->OptimisticUpdate(5);
	r->Step({MessageHelper{.from = 2,.to = 1,.type = raftpb::MsgUnreachable}.Done()});

    EXPECT_EQ(r->trk_.progress_[2]->state_, tracker::StateType::StateProbe);
    EXPECT_EQ(r->trk_.progress_[2]->match_ + 1, r->trk_.progress_[2]->next_);
}

uint64_t mustTerm(std::pair<uint64_t, Error> ret) {
    if (ret.second != nullptr) {
        EXPECT_TRUE(false);
    }
    return ret.first;
}

TEST(raft, TestRestore) {
    raftpb::Snapshot s;
    auto meta = s.mutable_metadata();
    meta->set_index(11); // magic number
    meta->set_term(11); // magic number
    {
        std::vector<uint64_t> voters = {1, 2, 3};
        *meta->mutable_conf_state()->mutable_voters() = {voters.begin(), voters.end()};
    }

	auto storage = newTestMemoryStorage({withPeers({1, 2})});
	auto sm = newTestRaft(1, 10, 1, storage);
    EXPECT_TRUE(sm->restore(s));
    EXPECT_EQ(sm->raftLog_->lastIndex(), s.metadata().index());
    EXPECT_EQ(mustTerm(sm->raftLog_->term(s.metadata().index())), s.metadata().term());
	auto sg = sm->trk_.VoterNodes();
    const auto& voters = s.metadata().conf_state().voters();
    EXPECT_EQ(sg, std::vector<uint64_t>(voters.begin(), voters.end()));

    EXPECT_FALSE(sm->restore(s));
	// It should not campaign before actually applying data.
	for (auto i = 0; i < sm->randomizedElectionTimeout_; i++) {
		sm->tick_();
	}
    EXPECT_EQ(sm->state_, StateFollower);
}

// TestRestoreWithLearner restores a snapshot which contains learners.
TEST(raft, TestRestoreWithLearner) {
    raftpb::Snapshot s;
    auto meta = s.mutable_metadata();
    meta->set_index(11); // magic number
    meta->set_term(11); // magic number
    {
        std::vector<uint64_t> voters = {1, 2};
        *meta->mutable_conf_state()->mutable_voters() = {voters.begin(), voters.end()};
        std::vector<uint64_t> learners = {3};
        *meta->mutable_conf_state()->mutable_learners() = {learners.begin(), learners.end()};
    }

	auto storage = newTestMemoryStorage({withPeers({1, 2}), withLearners({3})});
	auto sm = newTestLearnerRaft(3, 8, 2, storage);
    EXPECT_TRUE(sm->restore(s));
    EXPECT_EQ(sm->raftLog_->lastIndex(), s.metadata().index());
    EXPECT_EQ(mustTerm(sm->raftLog_->term(s.metadata().index())), s.metadata().term());
	auto sg = sm->trk_.VoterNodes();
    EXPECT_EQ(sg.size(), s.metadata().conf_state().voters().size());
	auto lns = sm->trk_.LearnerNodes();
    EXPECT_EQ(lns.size(), s.metadata().conf_state().learners().size());
    for (auto n : s.metadata().conf_state().voters()) {
        EXPECT_FALSE(sm->trk_.progress_[n]->isLearner_);
	}
    for (auto n : s.metadata().conf_state().learners()) {
        EXPECT_TRUE(sm->trk_.progress_[n]->isLearner_);
	}
    EXPECT_FALSE(sm->restore(s));
}

// TestRestoreWithVotersOutgoing tests if outgoing voter can receive and apply snapshot correctly.
TEST(raft, TestRestoreWithVotersOutgoing) {
    raftpb::Snapshot s;
    auto meta = s.mutable_metadata();
    meta->set_index(11); // magic number
    meta->set_term(11); // magic number
    {
        std::vector<uint64_t> voters = {2,3,4};
        *meta->mutable_conf_state()->mutable_voters() = {voters.begin(), voters.end()};
        std::vector<uint64_t> voters_outgoing = {1,2,3};
        *meta->mutable_conf_state()->mutable_voters_outgoing() = {voters_outgoing.begin(), voters_outgoing.end()};
    }

	auto storage = newTestMemoryStorage({withPeers({1, 2})});
	auto sm = newTestRaft(1, 10, 1, storage);
    EXPECT_TRUE(sm->restore(s));
    EXPECT_EQ(sm->raftLog_->lastIndex(), s.metadata().index());
    EXPECT_EQ(mustTerm(sm->raftLog_->term(s.metadata().index())), s.metadata().term());
	auto sg = sm->trk_.VoterNodes();
    std::vector<uint64_t> ids{1,2,3,4};
    EXPECT_EQ(sg, ids);
    EXPECT_FALSE(sm->restore(s));
	// It should not campaign before actually applying data.
	for (auto i = 0; i < sm->randomizedElectionTimeout_; i++) {
		sm->tick_();
	}
    EXPECT_EQ(sm->state_, StateFollower);
}

// TestRestoreVoterToLearner verifies that a normal peer can be downgraded to a
// learner through a snapshot. At the time of writing, we don't allow
// configuration changes to do this directly, but note that the snapshot may
// compress multiple changes to the configuration into one: the voter could have
// been removed, then readded as a learner and the snapshot reflects both
// changes. In that case, a voter receives a snapshot telling it that it is now
// a learner. In fact, the node has to accept that snapshot, or it is
// permanently cut off from the Raft log.
TEST(raft, TestRestoreVoterToLearner) {
    raftpb::Snapshot s;
    auto meta = s.mutable_metadata();
    meta->set_index(11); // magic number
    meta->set_term(11); // magic number
    {
        std::vector<uint64_t> voters = {1,2};
        *meta->mutable_conf_state()->mutable_voters() = {voters.begin(), voters.end()};
        std::vector<uint64_t> learners = {3};
        *meta->mutable_conf_state()->mutable_learners() = {learners.begin(), learners.end()};
    }


	auto storage = newTestMemoryStorage({withPeers({1, 2, 3})});
	auto sm = newTestRaft(3, 10, 1, storage);
    EXPECT_FALSE(sm->isLearner_);
    EXPECT_TRUE(sm->restore(s));
}

// TestRestoreLearnerPromotion checks that a learner can become to a follower after
// restoring snapshot.
TEST(raft, TestRestoreLearnerPromotion) {
    raftpb::Snapshot s;
    auto meta = s.mutable_metadata();
    meta->set_index(11); // magic number
    meta->set_term(11); // magic number
    {
        std::vector<uint64_t> voters = {1,2,3};
        *meta->mutable_conf_state()->mutable_voters() = {voters.begin(), voters.end()};
    }

	auto storage = newTestMemoryStorage({withPeers({1, 2}), withLearners({3})});
	auto sm = newTestLearnerRaft(3, 10, 1, storage);
    EXPECT_TRUE(sm->isLearner_);
    EXPECT_TRUE(sm->restore(s));
    EXPECT_FALSE(sm->isLearner_);
}

// TestLearnerReceiveSnapshot tests that a learner can receive a snpahost from leader
TEST(raft, TestLearnerReceiveSnapshot) {
	// restore the state machine from a snapshot so it has a compacted log and a snapshot
    raftpb::Snapshot s;
    auto meta = s.mutable_metadata();
    meta->set_index(11); // magic number
    meta->set_term(11); // magic number
    {
        std::vector<uint64_t> voters = {1};
        *meta->mutable_conf_state()->mutable_voters() = {voters.begin(), voters.end()};
        std::vector<uint64_t> learners = {2};
        *meta->mutable_conf_state()->mutable_learners() = {learners.begin(), learners.end()};
    }

	auto store = newTestMemoryStorage({withPeers({1}), withLearners({2})});
	auto n1 = newTestLearnerRaft(1, 10, 1, store);
	auto n2 = newTestLearnerRaft(2, 10, 1, newTestMemoryStorage({withPeers({1}), withLearners({2})}));

	n1->restore(s);
    auto snap = n1->raftLog_->nextUnstableSnapshot();
    store->ApplySnapshot(*snap);
    n1->appliedSnap(*snap);

	auto nt = newNetwork({std::make_shared<raftWarp>(n1), std::make_shared<raftWarp>(n2)});

	setRandomizedElectionTimeout(n1, n1->electionTimeout_);
	for (auto i = 0; i < n1->electionTimeout_; i++) {
		n1->tick_();
	}

	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgBeat}.Done()});
    EXPECT_EQ(n2->raftLog_->committed_, n1->raftLog_->committed_);
}

TEST(raft, TestRestoreIgnoreSnapshot) {
    std::vector<raftpb::Entry> previousEnts = {EntryHelper{.index = 1,.term = 1}.Done(),EntryHelper{.index = 2,.term = 1}.Done(),EntryHelper{.index = 3,.term = 1}.Done()};
	uint64_t commit = 1;
	auto storage = newTestMemoryStorage({withPeers({1, 2})});
	auto sm = newTestRaft(1, 10, 1, storage);
	sm->raftLog_->append(previousEnts);
	sm->raftLog_->commitTo(commit);

    raftpb::Snapshot s;
    auto meta = s.mutable_metadata();
    meta->set_index(commit);
    meta->set_term(1);
    {
        std::vector<uint64_t> voters = {1,2};
        *meta->mutable_conf_state()->mutable_voters() = {voters.begin(), voters.end()};
    }

	// ignore snapshot
    EXPECT_FALSE(sm->restore(s));
    EXPECT_EQ(sm->raftLog_->committed_, commit);

	// ignore snapshot and fast forward commit
    s.mutable_metadata()->set_index(commit + 1);
    EXPECT_FALSE(sm->restore(s));
    EXPECT_EQ(sm->raftLog_->committed_, commit+1);
}

TEST(raft, TestProvideSnap) {
	// restore the state machine from a snapshot so it has a compacted log and a snapshot
    raftpb::Snapshot s;
    auto meta = s.mutable_metadata();
    meta->set_index(11); // magic number
    meta->set_term(11); // magic number
    {
        std::vector<uint64_t> voters = {1,2};
        *meta->mutable_conf_state()->mutable_voters() = {voters.begin(), voters.end()};
    }

	auto storage = newTestMemoryStorage({withPeers({1})});
	auto sm = newTestRaft(1, 10, 1, storage);
	sm->restore(s);

	sm->becomeCandidate();
	sm->becomeLeader();

	// force set the next of node 2, so that node 2 needs a snapshot
	sm->trk_.progress_[2]->next_ = sm->raftLog_->firstIndex();
    raftpb::Message msg;
    msg.set_from(2);
    msg.set_to(1);
    msg.set_type(raftpb::MsgAppResp);
    msg.set_index(sm->trk_.progress_[2]->next_ - 1);
    msg.set_reject(true);
	sm->Step(msg);

	auto msgs = readMessages(sm);
    EXPECT_EQ(msgs.size(), 1);
	auto m = msgs[0];
    EXPECT_EQ(m.type(), raftpb::MsgSnap);
}

TEST(raft, TestIgnoreProvidingSnap) {
	// restore the state machine from a snapshot so it has a compacted log and a snapshot
    raftpb::Snapshot s;
    auto meta = s.mutable_metadata();
    meta->set_index(11); // magic number
    meta->set_term(11); // magic number
    {
        std::vector<uint64_t> voters = {1,2};
        *meta->mutable_conf_state()->mutable_voters() = {voters.begin(), voters.end()};
    }

	auto storage = newTestMemoryStorage({withPeers({1})});
	auto sm = newTestRaft(1, 10, 1, storage);
	sm->restore(s);

	sm->becomeCandidate();
	sm->becomeLeader();

	// force set the next of node 2, so that node 2 needs a snapshot
	// change node 2 to be inactive, expect node 1 ignore sending snapshot to 2
	sm->trk_.progress_[2]->next_ = sm->raftLog_->firstIndex() - 1;
	sm->trk_.progress_[2]->recentActive_ = false;
	sm->Step({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "somedata"}.Done()}}.Done()});

	auto msgs = readMessages(sm);
    EXPECT_EQ(msgs.size(), 0);
}

TEST(raft, TestRestoreFromSnapMsg) {
    raftpb::Snapshot s;
    auto meta = s.mutable_metadata();
    meta->set_index(11); // magic number
    meta->set_term(11); // magic number
    {
        std::vector<uint64_t> voters = {1,2};
        *meta->mutable_conf_state()->mutable_voters() = {voters.begin(), voters.end()};
    }
    raftpb::Message m;
    m.set_type(raftpb::MsgSnap);
    m.set_from(1);
    m.set_term(2);
    *m.mutable_snapshot() = s;

	auto sm = newTestRaft(2, 10, 1, newTestMemoryStorage({withPeers({1, 2})}));
	sm->Step(m);
    EXPECT_EQ(sm->lead_, 1);
}

TEST(raft, TestSlowNodeRestore) {
	auto nt = newNetwork({nullptr, nullptr, nullptr});
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	nt->isolate(3);
	for (auto j = 0; j <= 100; j++) {
		nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {{}}}.Done()});
	}
    auto lead = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));
	nextEnts(lead->raft_, nt->storage[1]);
    raftpb::ConfState cs;
    auto vn = lead->raft_->trk_.VoterNodes();
    *cs.mutable_voters() = {vn.begin(), vn.end()};

    //cs.set_voters()
	nt->storage.at(1)->CreateSnapshot(lead->raft_->raftLog_->applied_, &cs, {});
	nt->storage.at(1)->Compact(lead->raft_->raftLog_->applied_);

	nt->recover();
	// send heartbeats so that the leader can learn everyone is active.
	// node 3 will only be considered as active when node 1 receives a reply from it.
	while(true) {
		nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgBeat}.Done()});
		if (lead->raft_->trk_.progress_[3]->recentActive_) {
			break;
		}
	}

	// trigger a snapshot
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {{}}}.Done()});

    auto follower = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(3));

	// trigger a commit
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {{}}}.Done()});
    EXPECT_EQ(follower->raft_->raftLog_->committed_, lead->raft_->raftLog_->committed_);
}

// TestStepConfig tests that when raft step msgProp in EntryConfChange type,
// it appends the entry to log and sets pendingConf to be true.
TEST(raft, TestStepConfig) {
	// a raft that cannot make progress
	auto r = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2})}));
	r->becomeCandidate();
	r->becomeLeader();
	auto index = r->raftLog_->lastIndex();
	r->Step({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.type = raftpb::EntryConfChange}.Done()}}.Done()});
    EXPECT_EQ(r->raftLog_->lastIndex(), index+1);
    EXPECT_EQ(r->pendingConfIndex_, index+1);
}

// TestStepIgnoreConfig tests that if raft step the second msgProp in
// EntryConfChange type when the first one is uncommitted, the node will set
// the proposal to noop and keep its original state.
TEST(raft, TestStepIgnoreConfig) {
	// a raft that cannot make progress
	auto r = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2})}));
	r->becomeCandidate();
	r->becomeLeader();
    r->Step({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.type = raftpb::EntryConfChange}.Done()}}.Done()});
	auto index = r->raftLog_->lastIndex();
	auto pendingConfIndex = r->pendingConfIndex_;
    r->Step({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.type = raftpb::EntryConfChange}.Done()}}.Done()});
    auto entry = EntryHelper{.index = 3,.term = 1,.data = {}}.Done();
    entry.set_type(raftpb::EntryNormal);
    std::vector<raftpb::Entry> wents = {entry};
    std::vector<raftpb::Entry> ents;
    Error err;
	std::tie(ents, err) = r->raftLog_->entries(index+1, noLimit);
    EXPECT_EQ(err, nullptr);
    EXPECT_TRUE(VectorEquals(ents, wents));
    EXPECT_EQ(r->pendingConfIndex_, pendingConfIndex);
}

// TestNewLeaderPendingConfig tests that new leader sets its pendingConfigIndex
// based on uncommitted entries.
TEST(raft, TestNewLeaderPendingConfig) {
    struct Test {
        bool addEntry;
        uint64_t wpendingIndex;
    };
    std::vector<Test> tests = {
		{false, 0},
		{true, 1},
	};
    for (auto& tt : tests) {
		auto r = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2})}));
		if (tt.addEntry) {
			mustAppendEntry(r, {EntryHelper{.type = raftpb::EntryNormal}.Done()});
		}
		r->becomeCandidate();
		r->becomeLeader();
        EXPECT_EQ(r->pendingConfIndex_, tt.wpendingIndex);
	}
}

// TestAddNode tests that addNode could update nodes correctly.
TEST(raft, TestAddNode) {
	auto r = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1})}));
    raftpb::ConfChange cc;
    cc.set_node_id(2);
    cc.set_type(raftpb::ConfChangeAddNode);
    raftpb::ConfChangeWrap ccw(cc);
	r->applyConfChange(ccw.AsV2());
	auto nodes = r->trk_.VoterNodes();
    std::vector<uint64_t> wnodes = {1,2};
    EXPECT_EQ(nodes, wnodes);
}

// TestAddLearner tests that addLearner could update nodes correctly.
TEST(raft, TestAddLearner) {
	auto r = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1})}));
	// Add new learner peer.
    {
        raftpb::ConfChange cc;
        cc.set_node_id(2);
        cc.set_type(raftpb::ConfChangeAddLearnerNode);
        raftpb::ConfChangeWrap ccw(cc);
        r->applyConfChange(ccw.AsV2());
    }
    EXPECT_FALSE(r->isLearner_);
	auto nodes = r->trk_.LearnerNodes();
	std::vector<uint64_t> wnodes = {2};
    EXPECT_EQ(nodes, wnodes);
    EXPECT_TRUE(r->trk_.progress_[2]->isLearner_);

	// Promote peer to voter.
    {
        raftpb::ConfChange cc;
        cc.set_node_id(2);
        cc.set_type(raftpb::ConfChangeAddNode);
        raftpb::ConfChangeWrap ccw(cc);
        r->applyConfChange(ccw.AsV2());
    }
    EXPECT_FALSE(r->trk_.progress_[2]->isLearner_);

	// Demote r.
    {
        raftpb::ConfChange cc;
        cc.set_node_id(1);
        cc.set_type(raftpb::ConfChangeAddLearnerNode);
        raftpb::ConfChangeWrap ccw(cc);
        r->applyConfChange(ccw.AsV2());
    }
    EXPECT_TRUE(r->trk_.progress_[1]->isLearner_);
    EXPECT_TRUE(r->isLearner_);

	// Promote r again.
    {
        raftpb::ConfChange cc;
        cc.set_node_id(1);
        cc.set_type(raftpb::ConfChangeAddNode);
        raftpb::ConfChangeWrap ccw(cc);
        r->applyConfChange(ccw.AsV2());
    }
    EXPECT_FALSE(r->trk_.progress_[1]->isLearner_);
    EXPECT_FALSE(r->isLearner_);
}

// TestAddNodeCheckQuorum tests that addNode does not trigger a leader election
// immediately when checkQuorum is set.
TEST(raft, TestAddNodeCheckQuorum) {
	auto r = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1})}));
	r->checkQuorum_ = true;

	r->becomeCandidate();
	r->becomeLeader();

	for (auto i = 0; i < r->electionTimeout_-1; i++) {
		r->tick_();
	}
    {
        raftpb::ConfChange cc;
        cc.set_node_id(2);
        cc.set_type(raftpb::ConfChangeAddNode);
        raftpb::ConfChangeWrap ccw(cc);
        r->applyConfChange(ccw.AsV2());
    }

	// This tick will reach electionTimeout, which triggers a quorum check.
	r->tick_();

	// Node 1 should still be the leader after a single tick.
    EXPECT_EQ(r->state_, StateLeader);

	// After another electionTimeout ticks without hearing from node 2,
	// node 1 should step down.
	for (auto i = 0; i < r->electionTimeout_; i++) {
		r->tick_();
	}
    EXPECT_EQ(r->state_, StateFollower);
}

// TestRemoveNode tests that removeNode could update nodes and
// removed list correctly.
TEST(raft, TestRemoveNode) {
	auto r = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2})}));
    {
        raftpb::ConfChange cc;
        cc.set_node_id(2);
        cc.set_type(raftpb::ConfChangeRemoveNode);
        raftpb::ConfChangeWrap ccw(cc);
        r->applyConfChange(ccw.AsV2());
    }
    std::vector<uint64_t> w = {1};
    auto g = r->trk_.VoterNodes();
    EXPECT_EQ(w,g);

    bool panic = false;
	// Removing the remaining voter will panic.
    try {
        raftpb::ConfChange cc;
        cc.set_node_id(1);
        cc.set_type(raftpb::ConfChangeRemoveNode);
        raftpb::ConfChangeWrap ccw(cc);
        r->applyConfChange(ccw.AsV2());
    } catch (const std::exception& e) {
        EXPECT_EQ(typeid(e), typeid(PanicException));
        panic = true;
    }
    EXPECT_TRUE(panic);
}

// TestRemoveLearner tests that removeNode could update nodes and
// removed list correctly.
TEST(raft, TestRemoveLearner) {
	auto r = newTestLearnerRaft(1, 10, 1, newTestMemoryStorage({withPeers({1}), withLearners({2})}));
    {
        raftpb::ConfChange cc;
        cc.set_node_id(2);
        cc.set_type(raftpb::ConfChangeRemoveNode);
        raftpb::ConfChangeWrap ccw(cc);
        r->applyConfChange(ccw.AsV2());
    }
    std::vector<uint64_t> w = {1};
    auto g = r->trk_.VoterNodes();
    EXPECT_EQ(g,w);

	w.clear();
    g = r->trk_.LearnerNodes();
    EXPECT_EQ(g,w);

    bool panic = false;
    try {
        raftpb::ConfChange cc;
        cc.set_node_id(1);
        cc.set_type(raftpb::ConfChangeRemoveNode);
        raftpb::ConfChangeWrap ccw(cc);
        r->applyConfChange(ccw.AsV2());
    } catch (std::exception& e) {
        EXPECT_EQ(typeid(e), typeid(PanicException));
        panic = true;
    }
    EXPECT_TRUE(panic);
}

TEST(raft, TestPromotable) {
	uint64_t id = 1;
    struct Test {
        std::vector<uint64_t> peers;
        bool wp;
    };
    std::vector<Test> tests = {
		{{1}, true},
		{{1, 2, 3}, true},
		{{}, false},
		{{2, 3}, false},
	};
    for (auto& tt : tests) {
		auto r = newTestRaft(id, 5, 1, newTestMemoryStorage({withPeers(tt.peers)}));
        auto g = r->promotable();
        EXPECT_EQ(g, tt.wp);
	}
}

TEST(raft, TestRaftNodes) {
    struct Test {
        std::vector<uint64_t> ids;
        std::vector<uint64_t> wids;
    };
    std::vector<Test> tests = {
		{
			{1, 2, 3},
			{1, 2, 3},
		},
		{
			{3, 2, 1},
			{1, 2, 3},
		}
	};
    for (auto& tt : tests) {
		auto r = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers(tt.ids)}));
        auto g = r->trk_.VoterNodes();
        EXPECT_EQ(g, tt.wids);
	}
}

void testCampaignWhileLeader(bool preVote) {
	auto cfg = newTestConfig(1, 5, 1, newTestMemoryStorage({withPeers({1})}));
	cfg.preVote_ = preVote;
	auto r = newRaft(cfg);
    EXPECT_EQ(r->state_, StateFollower);
	// We don't call campaign() directly because it comes after the check
	// for our current state.
	r->Step({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});
    advanceMessagesAfterAppend(r);
    EXPECT_EQ(r->state_, StateLeader);
	auto term = r->term_;
    r->Step({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});
    advanceMessagesAfterAppend(r);
    EXPECT_EQ(r->state_, StateLeader);
    EXPECT_EQ(r->term_, term);
}

TEST(raft, TestCampaignWhileLeader) {
	testCampaignWhileLeader(false);
}

TEST(raft, TestPreCampaignWhileLeader) {
	testCampaignWhileLeader(true);
}

// TestCommitAfterRemoveNode verifies that pending commands can become
// committed when a config change reduces the quorum requirements.
TEST(raft, TestCommitAfterRemoveNode) {
	// Create a cluster with two nodes.
	auto s = newTestMemoryStorage({withPeers({1, 2})});
	auto r = newTestRaft(1, 5, 1, s);
	r->becomeCandidate();
	r->becomeLeader();

	// Begin to remove the second node.
    raftpb::ConfChange cc;
    cc.set_type(raftpb::ConfChangeRemoveNode);
    cc.set_node_id(2);

    raftpb::Message msg;
    msg.set_type(raftpb::MsgProp);
    auto entry = msg.mutable_entries()->Add();
    entry->set_type(raftpb::EntryConfChange);
    entry->set_data(cc.SerializeAsString());
	r->Step({msg});
	// Stabilize the log and make sure nothing is committed yet.
    auto ents = nextEnts(r, s);
    EXPECT_EQ(ents.size(), 0);
	auto ccIndex = r->raftLog_->lastIndex();

	// While the config change is pending, make another proposal.
    msg.Clear();
    msg.set_type(raftpb::MsgProp);
    entry = msg.mutable_entries()->Add();
    entry->set_type(raftpb::EntryNormal);
    entry->set_data("hello");
	r->Step({msg});

	// Node 2 acknowledges the config change, committing it.
    msg.Clear();
    msg.set_type(raftpb::MsgAppResp);
    msg.set_from(2);
    msg.set_index(ccIndex);
	r->Step({msg});
	ents = nextEnts(r, s);
    EXPECT_EQ(ents.size(), 2);
    EXPECT_FALSE(ents[0].type() != raftpb::EntryNormal || !ents[0].data().empty());
    EXPECT_EQ(ents[1].type(), raftpb::EntryConfChange);

	// Apply the config change. This reduces quorum requirements so the
	// pending command can now commit.
    raftpb::ConfChangeWrap ccw(cc);
	r->applyConfChange(ccw.AsV2());
	ents = nextEnts(r, s);
    EXPECT_FALSE(ents.size() != 1 || ents[0].type() != raftpb::EntryNormal || ents[0].data() != "hello");
}

void checkLeaderTransferState(const std::shared_ptr<raft>& r, StateType state, uint64_t lead) {
    EXPECT_FALSE(r->state_ != state || r->lead_ != lead);
    EXPECT_EQ(r->leadTransferee_, None);
}

// TestLeaderTransferToUpToDateNode verifies transferring should succeed
// if the transferee has the most up-to-date log entries when transfer starts.
TEST(raft, TestLeaderTransferToUpToDateNode) {
	auto nt = newNetwork({nullptr, nullptr, nullptr});
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

    auto lead = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));
    EXPECT_EQ(lead->raft_->lead_, 1);

	// Transfer leadership to 2.
	nt->send({MessageHelper{.from = 2,.to = 1,.type = raftpb::MsgTransferLeader}.Done()});

	checkLeaderTransferState(lead->raft_, StateFollower, 2);

	// After some log replication, transfer leadership back to 1.
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {{}}}.Done()});
    nt->send({MessageHelper{.from = 1,.to = 2,.type = raftpb::MsgTransferLeader}.Done()});
	checkLeaderTransferState(lead->raft_, StateLeader, 1);
}

// TestLeaderTransferToUpToDateNodeFromFollower verifies transferring should succeed
// if the transferee has the most up-to-date log entries when transfer starts.
// Not like TestLeaderTransferToUpToDateNode, where the leader transfer message
// is sent to the leader, in this test case every leader transfer message is sent
// to the follower.
TEST(raft, TestLeaderTransferToUpToDateNodeFromFollower) {
	auto nt = newNetwork({nullptr, nullptr, nullptr});
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

    auto lead = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));
    EXPECT_EQ(lead->raft_->lead_, 1);

	// Transfer leadership to 2.
    nt->send({MessageHelper{.from = 2,.to = 2,.type = raftpb::MsgTransferLeader}.Done()});

	checkLeaderTransferState(lead->raft_, StateFollower, 2);

	// After some log replication, transfer leadership back to 1.
    nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {{}}}.Done()});
    nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgTransferLeader}.Done()});

	checkLeaderTransferState(lead->raft_, StateLeader, 1);
}


// TestLeaderTransferWithCheckQuorum ensures transferring leader still works
// even the current leader is still under its leader lease
TEST(raft, TestLeaderTransferWithCheckQuorum) {
	auto nt = newNetwork({nullptr, nullptr, nullptr});
	for (auto i = 1; i < 4; i++) {
        auto r = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(i));
		r->raft_->checkQuorum_ = true;
		setRandomizedElectionTimeout(r->raft_, r->raft_->electionTimeout_+i);
	}

	// Letting peer 2 electionElapsed reach to timeout so that it can vote for peer 1
    auto f = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(2));
	for (auto i = 0; i < f->raft_->electionTimeout_; i++) {
		f->raft_->tick_();
	}

	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

    auto lead = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));
    EXPECT_EQ(lead->raft_->lead_, 1);

	// Transfer leadership to 2.
    nt->send({MessageHelper{.from = 2,.to = 1,.type = raftpb::MsgTransferLeader}.Done()});

	checkLeaderTransferState(lead->raft_, StateFollower, 2);

	// After some log replication, transfer leadership back to 1.
    nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp,.entries =  {{}}}.Done()});
    nt->send({MessageHelper{.from = 1,.to = 2,.type = raftpb::MsgTransferLeader}.Done()});

	checkLeaderTransferState(lead->raft_, StateLeader, 1);
}

TEST(raft, TestLeaderTransferToSlowFollower) {
	auto nt = newNetwork({nullptr, nullptr, nullptr});
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	nt->isolate(3);
    nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {{}}}.Done()});

	nt->recover();
    auto lead = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));
    EXPECT_EQ(lead->raft_->trk_.progress_[3]->match_, 1);

	// Transfer leadership to 3 when node 3 is lack of log.
    nt->send({MessageHelper{.from = 3,.to = 1,.type = raftpb::MsgTransferLeader}.Done()});

	checkLeaderTransferState(lead->raft_, StateFollower, 3);
}

TEST(raft, TestLeaderTransferAfterSnapshot) {
	auto nt = newNetwork({nullptr, nullptr, nullptr});
    nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	nt->isolate(3);
    nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp,.entries = {{}}}.Done()});
    auto lead = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));
	nextEnts(lead->raft_, nt->storage[1]);

    raftpb::ConfState cs;
    auto vn = lead->raft_->trk_.VoterNodes();
    *cs.mutable_voters() = {vn.begin(), vn.end()};
	nt->storage[1]->CreateSnapshot(lead->raft_->raftLog_->applied_, &cs, {});
	nt->storage[1]->Compact(lead->raft_->raftLog_->applied_);

	nt->recover();
    EXPECT_EQ(lead->raft_->trk_.progress_[3]->match_, 1);

	raftpb::Message filtered;
	// Snapshot needs to be applied before sending MsgAppResp
    nt->msgHook = [&filtered](const raftpb::Message& m){
        if (m.type() != raftpb::MsgAppResp || m.from() != 3 || m.reject()) {
            return true;
        }
        filtered = m;
        return false;
    };
	// Transfer leadership to 3 when node 3 is lack of snapshot.
    nt->send({MessageHelper{.from = 3,.to = 1,.type = raftpb::MsgTransferLeader}.Done()});
    EXPECT_EQ(lead->raft_->state_, StateLeader);
    EXPECT_FALSE(google::protobuf::util::MessageDifferencer::Equals(filtered, raftpb::Message()));

	// Apply snapshot and resume progress
    auto follower = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(3));
    auto snap = follower->raft_->raftLog_->nextUnstableSnapshot();
    nt->storage.at(3)->ApplySnapshot(*snap);
    follower->raft_->appliedSnap(*snap);
	nt->msgHook = nullptr;
	nt->send({filtered});

	checkLeaderTransferState(lead->raft_, StateFollower, 3);
}

TEST(raft, TestLeaderTransferToSelf) {
	auto nt = newNetwork({nullptr, nullptr, nullptr});
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

    auto lead = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));

	// Transfer leadership to self, there will be noop.

	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgTransferLeader}.Done()});
	checkLeaderTransferState(lead->raft_, StateLeader, 1);
}
TEST(raft, TestLeaderTransferToNonExistingNode) {
	auto nt = newNetwork({nullptr, nullptr, nullptr});
    nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

    auto lead = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));
	// Transfer leadership to non-existing node, there will be noop.
    nt->send({MessageHelper{.from = 4,.to = 1,.type = raftpb::MsgTransferLeader}.Done()});
	checkLeaderTransferState(lead->raft_, StateLeader, 1);
}

TEST(raft, TestLeaderTransferTimeout) {
    auto nt = newNetwork({nullptr, nullptr, nullptr});
    nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	nt->isolate(3);

    auto lead = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));

	// Transfer leadership to isolated node, wait for timeout.
    nt->send({MessageHelper{.from = 3,.to = 1,.type = raftpb::MsgTransferLeader}.Done()});
    EXPECT_EQ(lead->raft_->leadTransferee_, 3);
	for (auto i = 0; i < lead->raft_->heartbeatTimeout_; i++) {
		lead->raft_->tick_();
	}
    EXPECT_EQ(lead->raft_->leadTransferee_, 3);

	for (auto i = 0; i < lead->raft_->electionTimeout_-lead->raft_->heartbeatTimeout_; i++) {
		lead->raft_->tick_();
	}

	checkLeaderTransferState(lead->raft_, StateLeader, 1);
}

TEST(raft, TestLeaderTransferIgnoreProposal) {
    auto s = newTestMemoryStorage({withPeers({1, 2, 3})});
    auto r = newTestRaft(1, 10, 1, s);
    auto nt = newNetwork({std::make_shared<raftWarp>(r), nullptr, nullptr});
    nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	nt->isolate(3);

    auto lead = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));
    nextEnts(r, s); // handle empty entry
	// Transfer leadership to isolated node to let transfer pending, then send proposal.
    nt->send({MessageHelper{.from = 3,.to = 1,.type = raftpb::MsgTransferLeader}.Done()});
    EXPECT_EQ(lead->raft_->leadTransferee_, 3);


	nt->send({MessageHelper{.from = 1,.to = 1, .type = raftpb::MsgProp, .entries = {{}}}.Done()});

	auto err = lead->raft_->Step({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {{}}}.Done()});
    EXPECT_EQ(err, ErrProposalDropped);
    EXPECT_EQ(lead->raft_->trk_.progress_[1]->match_, 1);
}

TEST(raft, TestLeaderTransferReceiveHigherTermVote) {
    auto nt = newNetwork({nullptr, nullptr, nullptr});
    nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	nt->isolate(3);

    auto lead = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));

	// Transfer leadership to isolated node to let transfer pending.
    nt->send({MessageHelper{.from = 3,.to = 1,.type = raftpb::MsgTransferLeader}.Done()});
    EXPECT_EQ(lead->raft_->leadTransferee_, 3);

    auto msg = MessageHelper{.from = 2,.to = 2,.type = raftpb::MsgHup}.Done();
    msg.set_index(1);
    msg.set_term(2);
	nt->send({msg});

	checkLeaderTransferState(lead->raft_, StateFollower, 2);
}

TEST(raft, TestLeaderTransferRemoveNode) {
    auto nt = newNetwork({nullptr, nullptr, nullptr});
    nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	nt->ignore(raftpb::MsgTimeoutNow);

    auto lead = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));

	// The leadTransferee is removed when leadship transferring.
    nt->send({MessageHelper{.from = 3,.to = 1,.type = raftpb::MsgTransferLeader}.Done()});
    EXPECT_EQ(lead->raft_->leadTransferee_, 3);

    raftpb::ConfChange cc;
    cc.set_node_id(3);
    cc.set_type(raftpb::ConfChangeRemoveNode);
    raftpb::ConfChangeWrap ccw(cc);
	lead->raft_->applyConfChange(ccw.AsV2());

	checkLeaderTransferState(lead->raft_, StateLeader, 1);
}

TEST(raft, TestLeaderTransferDemoteNode) {
    auto nt = newNetwork({nullptr, nullptr, nullptr});
    nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	nt->ignore(raftpb::MsgTimeoutNow);

    auto lead = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));

	// The leadTransferee is demoted when leadship transferring.
    nt->send({MessageHelper{.from = 3,.to = 1,.type = raftpb::MsgTransferLeader}.Done()});
    EXPECT_EQ(lead->raft_->leadTransferee_, 3);

    std::vector<raftpb::ConfChangeSingle> changes;
    changes.emplace_back();
    changes.back().set_type(raftpb::ConfChangeRemoveNode);
    changes.back().set_node_id(3);
    changes.emplace_back();
    changes.back().set_type(raftpb::ConfChangeAddLearnerNode);
    changes.back().set_node_id(3);
    raftpb::ConfChangeV2 ccv2;
    *ccv2.mutable_changes() = {changes.begin(), changes.end()};
    lead->raft_->applyConfChange(ccv2);

	// Make the Raft group commit the LeaveJoint entry.
	lead->raft_->applyConfChange(raftpb::ConfChangeV2());
	checkLeaderTransferState(lead->raft_, StateLeader, 1);
}

// TestLeaderTransferBack verifies leadership can transfer back to self when last transfer is pending
TEST(raft, TestLeaderTransferBack) {
    auto nt = newNetwork({nullptr, nullptr, nullptr});
    nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	nt->isolate(3);

    auto lead = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));

    nt->send({MessageHelper{.from = 3,.to = 1,.type = raftpb::MsgTransferLeader}.Done()});
    EXPECT_EQ(lead->raft_->leadTransferee_, 3);

	// Transfer leadership back to self.
    nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgTransferLeader}.Done()});

	checkLeaderTransferState(lead->raft_, StateLeader, 1);
}

// TestLeaderTransferSecondTransferToAnotherNode verifies leader can transfer to another node
// when last transfer is pending.
TEST(raft, TestLeaderTransferSecondTransferToAnotherNode) {
	auto nt = newNetwork({nullptr, nullptr, nullptr});
    nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	nt->isolate(3);

    auto lead = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));

    nt->send({MessageHelper{.from = 3,.to = 1,.type = raftpb::MsgTransferLeader}.Done()});
    EXPECT_EQ(lead->raft_->leadTransferee_, 3);

	// Transfer leadership to another node.
    nt->send({MessageHelper{.from = 2,.to = 1,.type = raftpb::MsgTransferLeader}.Done()});

	checkLeaderTransferState(lead->raft_, StateFollower, 2);
}

// TestLeaderTransferSecondTransferToSameNode verifies second transfer leader request
// to the same node should not extend the timeout while the first one is pending.
TEST(raft, TestLeaderTransferSecondTransferToSameNode) {
    auto nt = newNetwork({nullptr, nullptr, nullptr});
    nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	nt->isolate(3);

    auto lead = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));

    nt->send({MessageHelper{.from = 3,.to = 1,.type = raftpb::MsgTransferLeader}.Done()});
    EXPECT_EQ(lead->raft_->leadTransferee_, 3);

	for (auto i = 0; i < lead->raft_->heartbeatTimeout_; i++) {
		lead->raft_->tick_();
	}
	// Second transfer leadership request to the same node.
    nt->send({MessageHelper{.from = 3,.to = 1,.type = raftpb::MsgTransferLeader}.Done()});

	for (auto i = 0; i < lead->raft_->electionTimeout_-lead->raft_->heartbeatTimeout_; i++) {
		lead->raft_->tick_();
	}

	checkLeaderTransferState(lead->raft_, StateLeader, 1);
}

// TestTransferNonMember verifies that when a MsgTimeoutNow arrives at
// a node that has been removed from the group, nothing happens.
// (previously, if the node also got votes, it would panic as it
// transitioned to StateLeader)
TEST(raft, TestTransferNonMember) {
	auto r = newTestRaft(1, 5, 1, newTestMemoryStorage({withPeers({2, 3, 4})}));
	r->Step({MessageHelper{.from = 2,.to = 1,.type = raftpb::MsgTimeoutNow}.Done()});

	r->Step({MessageHelper{.from = 2,.to = 1,.type = raftpb::MsgVoteResp}.Done()});
	r->Step({MessageHelper{.from = 3,.to = 1,.type = raftpb::MsgVoteResp}.Done()});
    EXPECT_EQ(r->state_, StateFollower);
}

// TestNodeWithSmallerTermCanCompleteElection tests the scenario where a node
// that has been partitioned away (and fallen behind) rejoins the cluster at
// about the same time the leader node gets partitioned away.
// Previously the cluster would come to a standstill when run with PreVote
// enabled.
TEST(raft, TestNodeWithSmallerTermCanCompleteElection) {
	auto n1 = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto n2 = newTestRaft(2, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto n3 = newTestRaft(3, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));

	n1->becomeFollower(1, None);
	n2->becomeFollower(1, None);
	n3->becomeFollower(1, None);

	n1->preVote_ = true;
	n2->preVote_ = true;
	n3->preVote_ = true;

	// cause a network partition to isolate node 3
	auto nt = newNetwork({std::make_shared<raftWarp>(n1), std::make_shared<raftWarp>(n2), std::make_shared<raftWarp>(n3)});
	nt->cut(1, 3);
	nt->cut(2, 3);

	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});
    auto sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));
    EXPECT_EQ(sm->raft_->state_, StateLeader);

    sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(2));
    EXPECT_EQ(sm->raft_->state_, StateFollower);

	nt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});
    sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(3));
    EXPECT_EQ(sm->raft_->state_, StatePreCandidate);

	nt->send({MessageHelper{.from = 2,.to = 2,.type = raftpb::MsgHup}.Done()});

	// check whether the term values are expected
	// a.Term == 3
	// b.Term == 3
	// c.Term == 1
    sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));
    EXPECT_EQ(sm->raft_->term_, 3);

    sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(2));
    EXPECT_EQ(sm->raft_->term_, 3);

    sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(3));
    EXPECT_EQ(sm->raft_->term_, 1);

	// check state
	// a == follower
	// b == leader
	// c == pre-candidate
    sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));
    EXPECT_EQ(sm->raft_->state_, StateFollower);
    sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(2));
    EXPECT_EQ(sm->raft_->state_, StateLeader);
    sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(3));
    EXPECT_EQ(sm->raft_->state_, StatePreCandidate);

	// recover the network then immediately isolate b which is currently
	// the leader, this is to emulate the crash of b.
	nt->recover();
	nt->cut(2, 1);
	nt->cut(2, 3);

	// call for election
	nt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	// do we have a leader?
    auto sma = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));
    auto smb = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(3));
    EXPECT_FALSE(sma->raft_->state_ != StateLeader && smb->raft_->state_ != StateLeader);
}

// TestPreVoteWithSplitVote verifies that after split vote, cluster can complete
// election in next round.
TEST(raft, TestPreVoteWithSplitVote) {
	auto n1 = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto n2 = newTestRaft(2, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto n3 = newTestRaft(3, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));

	n1->becomeFollower(1, None);
	n2->becomeFollower(1, None);
	n3->becomeFollower(1, None);

	n1->preVote_ = true;
	n2->preVote_ = true;
	n3->preVote_ = true;

	auto nt = newNetwork({std::make_shared<raftWarp>(n1), std::make_shared<raftWarp>(n2), std::make_shared<raftWarp>(n3)});
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	// simulate leader down. followers start split vote.
	nt->isolate(1);

	nt->send({MessageHelper{.from = 2,.to = 2,.type = raftpb::MsgHup}.Done(), MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});

	// check whether the term values are expected
	// n2.Term == 3
	// n3.Term == 3
    auto sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(2));
    EXPECT_EQ(sm->raft_->term_, 3);
    sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(3));
    EXPECT_EQ(sm->raft_->term_, 3);

	// check state
	// n2 == candidate
	// n3 == candidate
    sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(2));
    EXPECT_EQ(sm->raft_->state_, StateCandidate);
    sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(3));
    EXPECT_EQ(sm->raft_->state_, StateCandidate);

	// node 2 election timeout first
	nt->send({MessageHelper{.from = 2,.to = 2,.type = raftpb::MsgHup}.Done()});

	// check whether the term values are expected
	// n2.Term == 4
	// n3.Term == 4
    sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(2));
    EXPECT_EQ(sm->raft_->term_, 4);
    sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(3));
    EXPECT_EQ(sm->raft_->term_, 4);

	// check state
	// n2 == leader
	// n3 == follower
    sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(2));
    EXPECT_EQ(sm->raft_->state_, StateLeader);
    sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(3));
    EXPECT_EQ(sm->raft_->state_, StateFollower);
}

// TestPreVoteWithCheckQuorum ensures that after a node become pre-candidate,
// it will checkQuorum correctly.
TEST(raft, TestPreVoteWithCheckQuorum) {
	auto n1 = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto n2 = newTestRaft(2, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto n3 = newTestRaft(3, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));

	n1->becomeFollower(1, None);
	n2->becomeFollower(1, None);
	n3->becomeFollower(1, None);

	n1->preVote_ = true;
	n2->preVote_ = true;
	n3->preVote_ = true;

	n1->checkQuorum_ = true;
	n2->checkQuorum_ = true;
	n3->checkQuorum_ = true;

	auto nt = newNetwork({std::make_shared<raftWarp>(n1), std::make_shared<raftWarp>(n2), std::make_shared<raftWarp>(n3)});
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	// isolate node 1. node 2 and node 3 have leader info
	nt->isolate(1);

	// check state
    auto sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));
    EXPECT_EQ(sm->raft_->state_, StateLeader);
    sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(2));
    EXPECT_EQ(sm->raft_->state_, StateFollower);
    sm = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(3));
    EXPECT_EQ(sm->raft_->state_, StateFollower);

	// node 2 will ignore node 3's PreVote
	nt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});
	nt->send({MessageHelper{.from = 2,.to = 2,.type = raftpb::MsgHup}.Done()});

	// Do we have a leader?
    EXPECT_FALSE(n2->state_ != StateLeader && n3->state_ != StateFollower);
}

// TestLearnerCampaign verifies that a learner won't campaign even if it receives
// a MsgHup or MsgTimeoutNow.
TEST(raft, TestLearnerCampaign) {
	auto n1 = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1})}));
    raftpb::ConfChange cc;
    cc.set_node_id(2);
    cc.set_type(raftpb::ConfChangeAddLearnerNode);
    raftpb::ConfChangeWrap ccw(cc);
	n1->applyConfChange(ccw.AsV2());
	auto n2 = newTestRaft(2, 10, 1, newTestMemoryStorage({withPeers({1})}));
	n2->applyConfChange(ccw.AsV2());
	auto nt = newNetwork({std::make_shared<raftWarp>(n1), std::make_shared<raftWarp>(n2)});
	nt->send({MessageHelper{.from = 2,.to = 2,.type = raftpb::MsgHup}.Done()});
    EXPECT_TRUE(n2->isLearner_);
    EXPECT_EQ(n2->state_, StateFollower);


	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});
    EXPECT_FALSE(n1->state_ != StateLeader || n1->lead_ != 1);

	// NB: TransferLeader already checks that the recipient is not a learner, but
	// the check could have happened by the time the recipient becomes a learner,
	// in which case it will receive MsgTimeoutNow as in this test case and we
	// verify that it's ignored.

	nt->send({MessageHelper{.from = 1,.to = 2,.type = raftpb::MsgTimeoutNow}.Done()});
    EXPECT_EQ(n2->state_, StateFollower);
}

// simulate rolling update a cluster for Pre-Vote. cluster has 3 nodes [n1, n2, n3].
// n1 is leader with term 2
// n2 is follower with term 2
// n3 is partitioned, with term 4 and less log, state is candidate
static std::shared_ptr<network> newPreVoteMigrationCluster() {
	auto n1 = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto n2 = newTestRaft(2, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	auto n3 = newTestRaft(3, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));

	n1->becomeFollower(1, None);
	n2->becomeFollower(1, None);
	n3->becomeFollower(1, None);

	n1->preVote_ = true;
	n2->preVote_ = true;
	// We intentionally do not enable PreVote for n3, this is done so in order
	// to simulate a rolling restart process where it's possible to have a mixed
	// version cluster with replicas with PreVote enabled, and replicas without.

	auto nt = newNetwork({std::make_shared<raftWarp>(n1), std::make_shared<raftWarp>(n2), std::make_shared<raftWarp>(n3)});
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});

	// Cause a network partition to isolate n3.
	nt->isolate(3);
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "some data"}.Done()}}.Done()});
	nt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});
	nt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});

	// check state
	// n1.state == StateLeader
	// n2.state == StateFollower
	// n3.state == StateCandidate
    EXPECT_EQ(n1->state_, StateLeader);
    EXPECT_EQ(n2->state_, StateFollower);
    EXPECT_EQ(n3->state_, StateCandidate);

	// check term
	// n1.Term == 2
	// n2.Term == 2
	// n3.Term == 4
    EXPECT_EQ(n1->term_, 2);
    EXPECT_EQ(n2->term_, 2);
    EXPECT_EQ(n3->term_, 4);

	// Enable prevote on n3, then recover the network
	n3->preVote_ = true;
	nt->recover();

	return nt;
}

TEST(raft, TestPreVoteMigrationCanCompleteElection) {
	auto nt = newPreVoteMigrationCluster();

	// n1 is leader with term 2
	// n2 is follower with term 2
	// n3 is pre-candidate with term 4, and less log
    auto n2 = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(2));
    auto n3 = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(3));

	// simulate leader down
	nt->isolate(1);

	// Call for elections from both n2 and n3.
	nt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});
	nt->send({MessageHelper{.from = 2,.to = 2,.type = raftpb::MsgHup}.Done()});

	// check state
	// n2.state == Follower
	// n3.state == PreCandidate
    EXPECT_EQ(n2->raft_->state_, StateFollower);
    EXPECT_EQ(n3->raft_->state_, StatePreCandidate);

    nt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});
    nt->send({MessageHelper{.from = 2,.to = 2,.type = raftpb::MsgHup}.Done()});

	// Do we have a leader?
    EXPECT_FALSE(n2->raft_->state_ != StateLeader && n3->raft_->state_ != StateFollower);
}

TEST(raft, TestPreVoteMigrationWithFreeStuckPreCandidate) {
	auto nt = newPreVoteMigrationCluster();

	// n1 is leader with term 2
	// n2 is follower with term 2
	// n3 is pre-candidate with term 4, and less log
    auto n1 = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));
    auto n2 = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(2));
    auto n3 = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(3));

	nt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});
    EXPECT_EQ(n1->raft_->state_, StateLeader);
    EXPECT_EQ(n2->raft_->state_, StateFollower);
    EXPECT_EQ(n3->raft_->state_, StatePreCandidate);

	// Pre-Vote again for safety
	nt->send({MessageHelper{.from = 3,.to = 3,.type = raftpb::MsgHup}.Done()});
    EXPECT_EQ(n1->raft_->state_, StateLeader);
    EXPECT_EQ(n2->raft_->state_, StateFollower);
    EXPECT_EQ(n3->raft_->state_, StatePreCandidate);
	nt->send({MessageHelper{.from = 1,.to = 3,.term = n1->raft_->term_, .type = raftpb::MsgHeartbeat}.Done()});

	// Disrupt the leader so that the stuck peer is freed
    EXPECT_EQ(n1->raft_->state_, StateFollower);
    EXPECT_EQ(n3->raft_->term_, n1->raft_->term_);
}

void testConfChangeCheckBeforeCampaign(bool v2) {
	auto nt = newNetwork({nullptr, nullptr, nullptr});
    auto n1 = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(1));
    auto n2 = std::dynamic_pointer_cast<raftWarp>(nt->peers.at(2));
	nt->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});
    EXPECT_EQ(n1->raft_->state_, StateLeader);

	// Begin to remove the third node.
    raftpb::ConfChange cc;
    cc.set_node_id(2);
    cc.set_type(raftpb::ConfChangeRemoveNode);
    raftpb::ConfChangeWrap ccw(cc);
    std::string ccData;
    Error err;
    raftpb::EntryType ty;
	if (v2) {
		auto ccv2 = ccw.AsV2();
        ccData = ccv2.SerializeAsString();
		ty = raftpb::EntryConfChangeV2;
	} else {
		ccData = cc.SerializeAsString();
		ty = raftpb::EntryConfChange;
	}
    auto msg = MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp}.Done();
    auto entry = msg.mutable_entries()->Add();
    entry->set_type(ty);
    entry->set_data(ccData);
	nt->send({msg});

	// Trigger campaign in node 2
	for (auto i = 0; i < n2->raft_->randomizedElectionTimeout_; i++) {
		n2->raft_->tick_();
	}
	// It's still follower because committed conf change is not applied.
    EXPECT_EQ(n2->raft_->state_, StateFollower);

	// Transfer leadership to peer 2.

	nt->send({MessageHelper{.from = 2,.to = 1,.type = raftpb::MsgTransferLeader}.Done()});
    EXPECT_EQ(n1->raft_->state_, StateLeader);
	// It's still follower because committed conf change is not applied.
    EXPECT_EQ(n2->raft_->state_, StateFollower);
	// Abort transfer leader
	for (auto i = 0; i < n1->raft_->electionTimeout_; i++) {
		n1->raft_->tick_();
	}

	// Advance apply
	nextEnts(n2->raft_, nt->storage.at(2));

	// Transfer leadership to peer 2 again.
	nt->send({MessageHelper{.from = 2,.to = 1,.type = raftpb::MsgTransferLeader}.Done()});
    EXPECT_EQ(n1->raft_->state_, StateFollower);
    EXPECT_EQ(n2->raft_->state_, StateLeader);

	nextEnts(n1->raft_, nt->storage.at(1));
	// Trigger campaign in node 2
	for (auto i = 0; i < n1->raft_->randomizedElectionTimeout_; i++) {
		n1->raft_->tick_();
	}
    EXPECT_EQ(n1->raft_->state_, StateCandidate);
}

// TestConfChangeCheckBeforeCampaign tests if unapplied ConfChange is checked before campaign.
TEST(raft, TestConfChangeCheckBeforeCampaign) {
	testConfChangeCheckBeforeCampaign(false);
}

// TestConfChangeV2CheckBeforeCampaign tests if unapplied ConfChangeV2 is checked before campaign.
TEST(raft, TestConfChangeV2CheckBeforeCampaign) {
	testConfChangeCheckBeforeCampaign(true);
}

TEST(raft, TestFastLogRejection) {
    struct Test {
        std::vector<raftpb::Entry> leaderLog; // Logs on the leader
        std::vector<raftpb::Entry> followerLog; // Logs on the follower
        uint64_t rejectHintTerm = 0;  // Expected term included in rejected MsgAppResp.
        uint64_t rejectHintIndex = 0;  // Expected index included in rejected MsgAppResp.
        uint64_t nextAppendTerm = 0; // Expected term when leader appends after rejected.
        uint64_t nextAppendIndex = 0; // Expected index when leader appends after rejected.
        uint64_t followerCompact = 0; // Index at which the follower log is compacted.
    };
    std::vector<Test> tests = {
		// This case tests that leader can find the conflict index quickly.
		// Firstly leader appends (type=MsgApp,index=7,logTerm=4, entries=...);
		// After rejected leader appends (type=MsgApp,index=3,logTerm=2).
		{
			{
                    EntryHelper{.index = 1, .term = 1}.Done(),
                    EntryHelper{.index = 2, .term = 2}.Done(),
                    EntryHelper{.index = 3, .term = 2}.Done(),
                    EntryHelper{.index = 4, .term = 4}.Done(),
                    EntryHelper{.index = 5, .term = 4}.Done(),
                    EntryHelper{.index = 6, .term = 4}.Done(),
                    EntryHelper{.index = 7, .term = 4}.Done()
			},
			{
                    EntryHelper{.index = 1, .term = 1}.Done(),
                    EntryHelper{.index = 2, .term = 2}.Done(),
                    EntryHelper{.index = 3, .term = 2}.Done(),
                    EntryHelper{.index = 4, .term = 3}.Done(),
                    EntryHelper{.index = 5, .term = 3}.Done(),
                    EntryHelper{.index = 6, .term = 3}.Done(),
                    EntryHelper{.index = 7, .term = 3}.Done(),
                    EntryHelper{.index = 8, .term = 3}.Done(),
                    EntryHelper{.index = 9, .term = 3}.Done(),
                    EntryHelper{.index = 10, .term = 3}.Done(),
                    EntryHelper{.index = 11, .term = 3}.Done()
			},
			3,
			7,
			2,
			3
		},
		// This case tests that leader can find the conflict index quickly.
		// Firstly leader appends (type=MsgApp,index=8,logTerm=5, entries=...);
		// After rejected leader appends (type=MsgApp,index=4,logTerm=3).
		{
			{
                    EntryHelper{.index = 1,.term = 1}.Done(),
                    EntryHelper{.index = 2,.term = 2}.Done(),
                    EntryHelper{.index = 3,.term = 2}.Done(),
                    EntryHelper{.index = 4,.term = 3}.Done(),
                    EntryHelper{.index = 5,.term = 4}.Done(),
                    EntryHelper{.index = 6,.term = 4}.Done(),
                    EntryHelper{.index = 7,.term = 4}.Done(),
                    EntryHelper{.index = 8,.term = 5}.Done()
			},
			{
                    EntryHelper{.index = 1, .term = 1}.Done(),
                    EntryHelper{.index = 2, .term = 2}.Done(),
                    EntryHelper{.index = 3, .term = 2}.Done(),
                    EntryHelper{.index = 4, .term = 3}.Done(),
                    EntryHelper{.index = 5, .term = 3}.Done(),
                    EntryHelper{.index = 6, .term = 3}.Done(),
                    EntryHelper{.index = 7, .term = 3}.Done(),
                    EntryHelper{.index = 8, .term = 3}.Done(),
                    EntryHelper{.index = 9, .term = 3}.Done(),
                    EntryHelper{.index = 10, .term = 3}.Done(),
                    EntryHelper{.index = 11, .term = 3}.Done()
			},
			3,
			8,
			3,
			4
		},
		// This case tests that follower can find the conflict index quickly.
		// Firstly leader appends (type=MsgApp,index=4,logTerm=1, entries=...);
		// After rejected leader appends (type=MsgApp,index=1,logTerm=1).
		{
			{
                    EntryHelper{.index = 1, .term = 1}.Done(),
                    EntryHelper{.index = 2, .term = 1}.Done(),
                    EntryHelper{.index = 3, .term = 1}.Done(),
                    EntryHelper{.index = 4, .term = 1}.Done()
			},
			{
                    EntryHelper{.index = 1, .term = 1}.Done(),
                    EntryHelper{.index = 2, .term = 2}.Done(),
                    EntryHelper{.index = 3, .term = 2}.Done(),
                    EntryHelper{.index = 4, .term = 4}.Done()
			},
			1,
			1,
			1,
			1
		},
		// This case is similar to the previous case. However, this time, the
		// leader has a longer uncommitted log tail than the follower.
		// Firstly leader appends (type=MsgApp,index=6,logTerm=1, entries=...);
		// After rejected leader appends (type=MsgApp,index=1,logTerm=1).
		{
			{
                    EntryHelper{.index = 1, .term = 1}.Done(),
                    EntryHelper{.index = 2, .term = 1}.Done(),
                    EntryHelper{.index = 3, .term = 1}.Done(),
                    EntryHelper{.index = 4, .term = 1}.Done(),
                    EntryHelper{.index = 5, .term = 1}.Done(),
                    EntryHelper{.index = 6, .term = 1}.Done()
			},
			{
                    EntryHelper{.index = 1, .term = 1}.Done(),
                    EntryHelper{.index = 2, .term = 2}.Done(),
                    EntryHelper{.index = 3, .term = 2}.Done(),
                    EntryHelper{.index = 4, .term = 4}.Done()
			},
			1,
			1,
			1,
			1
		},
		// This case is similar to the previous case. However, this time, the
		// follower has a longer uncommitted log tail than the leader.
		// Firstly leader appends (type=MsgApp,index=4,logTerm=1, entries=...);
		// After rejected leader appends (type=MsgApp,index=1,logTerm=1).
		{
			{
                    EntryHelper{.index = 1, .term = 1}.Done(),
                    EntryHelper{.index = 2, .term = 1}.Done(),
                    EntryHelper{.index = 3, .term = 1}.Done(),
                    EntryHelper{.index = 4, .term = 1}.Done()
			},
			{
                    EntryHelper{.index = 1, .term = 1}.Done(),
                    EntryHelper{.index = 2, .term = 2}.Done(),
                    EntryHelper{.index = 3, .term = 2}.Done(),
                    EntryHelper{.index = 4, .term = 4}.Done(),
                    EntryHelper{.index = 5, .term = 4}.Done(),
                    EntryHelper{.index = 6, .term = 4}.Done()
			},
			1,
			1,
            1,
			1
		},
		// An normal case that there are no log conflicts.
		// Firstly leader appends (type=MsgApp,index=5,logTerm=5, entries=...);
		// After rejected leader appends (type=MsgApp,index=4,logTerm=4).
		{
			{
                    EntryHelper{.index = 1, .term = 1}.Done(),
                    EntryHelper{.index = 2, .term = 1}.Done(),
                    EntryHelper{.index = 3, .term = 1}.Done(),
                    EntryHelper{.index = 4, .term = 4}.Done(),
                    EntryHelper{.index = 5, .term = 5}.Done()
			},
			{
                    EntryHelper{.index = 1, .term = 1}.Done(),
                    EntryHelper{.index = 2, .term = 1}.Done(),
                    EntryHelper{.index = 3, .term = 1}.Done(),
                    EntryHelper{.index = 4, .term = 4}.Done()
			},
			4,
			4,
			4,
			4
		},
		// Test case from example comment in stepLeader (on leader).
		{
			{
                    EntryHelper{.index = 1, .term = 2}.Done(),
                    EntryHelper{.index = 2, .term = 5}.Done(),
                    EntryHelper{.index = 3, .term = 5}.Done(),
                    EntryHelper{.index = 4, .term = 5}.Done(),
                    EntryHelper{.index = 5, .term = 5}.Done(),
                    EntryHelper{.index = 6, .term = 5}.Done(),
                    EntryHelper{.index = 7, .term = 5}.Done(),
                    EntryHelper{.index = 8, .term = 5}.Done(),
                    EntryHelper{.index = 9, .term = 5}.Done()
			},
			{
                    EntryHelper{.index = 1, .term = 2}.Done(),
                    EntryHelper{.index = 2, .term = 4}.Done(),
                    EntryHelper{.index = 3, .term = 4}.Done(),
                    EntryHelper{.index = 4, .term = 4}.Done(),
                    EntryHelper{.index = 5, .term = 4}.Done(),
                    EntryHelper{.index = 6, .term = 4}.Done()
			},
			4,
			6,
			2,
			1
		},
		// Test case from example comment in handleAppendEntries (on follower).
		{
			{
                    EntryHelper{.index = 1, .term = 2}.Done(),
                    EntryHelper{.index = 2, .term = 2}.Done(),
                    EntryHelper{.index = 3, .term = 2}.Done(),
                    EntryHelper{.index = 4, .term = 2}.Done(),
                    EntryHelper{.index = 5, .term = 2}.Done()
			},
			{
                    EntryHelper{.index = 1, .term = 2}.Done(),
                    EntryHelper{.index = 2, .term = 4}.Done(),
                    EntryHelper{.index = 3, .term = 4}.Done(),
                    EntryHelper{.index = 4, .term = 4}.Done(),
                    EntryHelper{.index = 5, .term = 4}.Done(),
                    EntryHelper{.index = 6, .term = 4}.Done(),
                    EntryHelper{.index = 7, .term = 4}.Done(),
                    EntryHelper{.index = 8, .term = 4}.Done()
			},
			2,
			1,
			2,
			1
		},
        // A case when a stale MsgApp from leader arrives after the corresponding
		// log index got compacted.
		// A stale (type=MsgApp,index=3,logTerm=3,entries=[(term=3,index=4)]) is
		// delivered to a follower who has already compacted beyond log index 3. The
		// MsgAppResp rejection will return same index=3, with logTerm=0. The leader
		// will rollback by one entry, and send MsgApp with index=2,logTerm=1.
		{
            {
                    EntryHelper{.index = 1,.term = 1}.Done(),
                    EntryHelper{.index = 2,.term = 1}.Done(),
                    EntryHelper{.index = 3,.term = 3}.Done()
             },
            {
                    EntryHelper{.index = 1,.term = 1}.Done(),
                    EntryHelper{.index = 2,.term = 1}.Done(),
                    EntryHelper{.index = 3,.term = 3}.Done(),
                    EntryHelper{.index = 4,.term = 3}.Done(),
                    EntryHelper{.index = 5,.term = 3}.Done() // <- this entry and below are compacted
            },
			0,
			3,
			1,
			2,
            5
		},
	};

    for (auto& test : tests) {
        auto s1 = NewMemoryStorage();
        std::vector<uint64_t> voters = {1,2,3};
        *s1->snapshot_.mutable_metadata()->mutable_conf_state()->mutable_voters() = {voters.begin(), voters.end()};
        s1->Append(test.leaderLog);
        auto last = test.leaderLog[test.leaderLog.size()-1];
        raftpb::HardState hs;
        hs.set_term(last.term() - 1);
        hs.set_commit(last.index());
        s1->SetHardState(hs);
        auto n1 = newTestRaft(1, 10, 1, s1);
        n1->becomeCandidate(); // bumps Term to last.Term
        n1->becomeLeader();

        auto s2 = NewMemoryStorage();
        *s2->snapshot_.mutable_metadata()->mutable_conf_state()->mutable_voters() = {voters.begin(), voters.end()};
        s2->Append(test.followerLog);
        hs.Clear();
        hs.set_term(last.term());
        hs.set_vote(1);
        hs.set_commit(0);
        s2->SetHardState(hs);
        auto n2 = newTestRaft(2, 10, 1, s2);
        if (test.followerCompact != 0) {
            s2->Compact(test.followerCompact);
            // NB: the state of n2 after this compaction isn't realistic because the
            // commit index is still at 0. We do this to exercise a "doesn't happen"
            // edge case behaviour, in case it still does happen in some other way.
        }
        EXPECT_EQ(n2->Step(MessageHelper{.from = 1, .to = 2, .type = raftpb::MsgHeartbeat}.Done()), nullptr);
        auto msgs = readMessages(n2);
        EXPECT_EQ(msgs.size(), 1);
        EXPECT_EQ(raftpb::MsgHeartbeatResp, msgs[0].type());
        EXPECT_EQ(n1->Step(msgs[0]), nullptr);
        msgs = readMessages(n1);
        EXPECT_EQ(msgs.size(), 1);
        EXPECT_EQ(raftpb::MsgApp, msgs[0].type());
        EXPECT_EQ(n2->Step(msgs[0]), nullptr);
        msgs = readMessages(n2);
        EXPECT_EQ(msgs.size(), 1);
        EXPECT_EQ(raftpb::MsgAppResp, msgs[0].type());
        EXPECT_TRUE(msgs[0].reject());
        EXPECT_EQ(test.rejectHintTerm, msgs[0].logterm());
        EXPECT_EQ(test.rejectHintIndex, msgs[0].rejecthint());
        EXPECT_EQ(n1->Step(msgs[0]), nullptr);
        msgs = readMessages(n1);
        EXPECT_EQ(test.nextAppendTerm, msgs[0].logterm());
        EXPECT_EQ(test.nextAppendIndex, msgs[0].index());
	}
}

// TestLeaderSyncFollowerLog tests that the leader could bring a follower's log
// into consistency with its own.
// Reference: section 5.3, figure 7
TEST(raft, TestLeaderSyncFollowerLog) {
    std::vector<raftpb::Entry> ents = {
		{},
        EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1}.Done(), EntryHelper{.index = 3,.term = 1}.Done(),
        EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 4}.Done(),
        EntryHelper{.index = 6,.term = 5}.Done(), EntryHelper{.index = 7,.term = 5}.Done(),
        EntryHelper{.index = 8,.term = 6}.Done(), EntryHelper{.index = 9,.term = 6}.Done(), EntryHelper{.index = 10,.term = 6}.Done(),
	};
	uint64_t term = 8;
    std::vector<std::vector<raftpb::Entry>> tests = {
		{
			{},
                EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1}.Done(), EntryHelper{.index = 3,.term = 1}.Done(),
                EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 4}.Done(),
                EntryHelper{.index = 6,.term = 5}.Done(), EntryHelper{.index = 7,.term = 5}.Done(),
                EntryHelper{.index = 8,.term = 6}.Done(), EntryHelper{.index = 9,.term = 6}.Done(),
		},
		{
			{},
                EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1}.Done(), EntryHelper{.index = 3,.term = 1}.Done(),
                EntryHelper{.index = 4,.term = 4}.Done(),
		},
		{
			{},
                EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1}.Done(), EntryHelper{.index = 3,.term = 1}.Done(),
                EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 4}.Done(),
                EntryHelper{.index = 6,.term = 5}.Done(), EntryHelper{.index = 7,.term = 5}.Done(),
                EntryHelper{.index = 8,.term = 6}.Done(), EntryHelper{.index = 9,.term = 6}.Done(), EntryHelper{.index = 10, .term = 6}.Done(), EntryHelper{.index = 11,.term = 6}.Done(),
		},
		{
			{},
                EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1}.Done(), EntryHelper{.index = 3,.term = 1}.Done(),
                EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 4}.Done(),
                EntryHelper{.index = 6,.term = 5}.Done(), EntryHelper{.index = 7,.term = 5}.Done(),
                EntryHelper{.index = 8,.term = 6}.Done(), EntryHelper{.index = 9,.term = 6}.Done(), EntryHelper{.index = 10,.term = 6}.Done(),
                EntryHelper{.index = 11, .term = 7}.Done(), EntryHelper{.index = 12,.term = 7}.Done(),
		},
		{
			{},
                EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1}.Done(), EntryHelper{.index = 3,.term = 1}.Done(),
                EntryHelper{.index = 4,.term = 4}.Done(), EntryHelper{.index = 5,.term = 4}.Done(), EntryHelper{.index = 6,.term = 4}.Done(), EntryHelper{.index = 7,.term = 4}.Done(),
		},
		{
			{},
                EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1}.Done(), EntryHelper{.index = 3,.term = 1}.Done(),
                EntryHelper{.index = 4,.term = 2}.Done(), EntryHelper{.index = 5,.term = 2}.Done(), EntryHelper{.index = 6,.term = 2}.Done(),
                EntryHelper{.index = 7,.term = 3}.Done(), EntryHelper{.index = 8,.term = 3}.Done(), EntryHelper{.index = 9,.term = 3}.Done(), EntryHelper{.index = 10,.term = 3}.Done(), EntryHelper{.index = 11,.term = 3}.Done(),
		},
	};
    for (auto& tt : tests) {
		auto leadStorage = newTestMemoryStorage({withPeers({1, 2, 3})});
		leadStorage->Append(ents);
		auto lead = newTestRaft(1, 10, 1, leadStorage);
        raftpb::HardState hs;
        hs.set_commit(lead->raftLog_->lastIndex());
        hs.set_term(term);
		lead->loadState(hs);
		auto followerStorage = newTestMemoryStorage({withPeers({1, 2, 3})});
		followerStorage->Append(tt);
		auto follower = newTestRaft(2, 10, 1, followerStorage);
        raftpb::HardState hs1;
        hs1.set_term(term-1);
		follower->loadState(hs1);
		// It is necessary to have a three-node cluster.
		// The second may have more up-to-date log than the first one, so the
		// first node needs the vote from the third node to become the leader.
		auto n = newNetwork({std::make_shared<raftWarp>(lead), std::make_shared<raftWarp>(follower), nopStepper});
		n->send({MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done()});
		// The election occurs in the term after the one we loaded with
		// lead.loadState above.
		n->send({MessageHelper{.from = 3,.to = 1,.term = term + 1, .type = raftpb::MsgVoteResp}.Done()});

        auto msg = MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp}.Done();
        msg.mutable_entries()->Add();
		n->send({msg});

        EXPECT_EQ(ltoa(lead->raftLog_),ltoa(follower->raftLog_));
	}
}

}