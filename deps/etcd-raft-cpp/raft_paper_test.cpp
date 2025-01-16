#include "raft.h"
#include <gtest/gtest.h>
namespace eraft {
using namespace detail;

using testMemoryStorageOptions = std::function<void(std::shared_ptr<MemoryStorage>&)>;
extern std::shared_ptr<MemoryStorage> newTestMemoryStorage(const std::vector<testMemoryStorageOptions>& opts);
extern testMemoryStorageOptions withPeers(const std::vector<uint64_t>& peers);
extern std::shared_ptr<raft> newTestRaft(uint64_t id, int election, int heartbeat, std::shared_ptr<Storage> storage);
extern std::vector<raftpb::Message> readMessages(const std::shared_ptr<raft>& r);
extern void mustAppendEntry(std::shared_ptr<raft> r, const std::vector<raftpb::Entry>& ents);
extern void advanceMessagesAfterAppend(const std::shared_ptr<raft>& r);
extern std::vector<uint64_t> idsBySize(int size);

struct blackHole;
extern std::shared_ptr<blackHole> nopStepper;

struct network;
struct stateMachine;
extern std::shared_ptr<network> newNetwork(const std::vector<std::shared_ptr<stateMachine>>& peers);
// testUpdateTermFromMessage tests that if one server’s current term is
// smaller than the other’s, then it updates its current term to the larger
// value. If a candidate or leader discovers that its term is out of date,
// it immediately reverts to follower state.
// Reference: section 5.1
void testUpdateTermFromMessage(StateType state) {
	auto r = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	switch (state) {
        case StateFollower:
		    r->becomeFollower(1, 2);
            break;
        case StateCandidate:
		    r->becomeCandidate();
            break;
        case StateLeader:
		    r->becomeCandidate();
		    r->becomeLeader();
            break;
        default:
            break;
	}

    raftpb::Message msg;
    msg.set_type(raftpb::MsgApp);
    msg.set_term(2);
	r->Step(msg);
    EXPECT_EQ(r->term_, 2);
    EXPECT_EQ(r->state_, StateFollower);
};

TEST(raft, TestFollowerUpdateTermFromMessage) {
    testUpdateTermFromMessage(StateFollower);
}

TEST(raft, TestCandidateUpdateTermFromMessage) {
    testUpdateTermFromMessage(StateCandidate);
}

TEST(raft, TestLeaderUpdateTermFromMessage) {
    testUpdateTermFromMessage(StateLeader);
}


// TestRejectStaleTermMessage tests that if a server receives a request with
// a stale term number, it rejects the request.
// Our implementation ignores the request instead.
// Reference: section 5.1
TEST(raft, TestRejectStaleTermMessage) {
	bool called = false;
	auto fakeStep = [&called](raft&, raftpb::Message) -> Error {
		called = true;
		return {};
	};
	auto r = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	r->step_ = fakeStep;
    raftpb::HardState hs;
    hs.set_term(2);
	r->loadState(hs);
    raftpb::Message msg;
    msg.set_type(raftpb::MsgApp);
    msg.set_term(r->term_ - 1);
	r->Step(msg);
    EXPECT_FALSE(called);
}

// TestStartAsFollower tests that when servers start up, they begin as followers.
// Reference: section 5.2
TEST(raft, TestStartAsFollower) {
	auto r = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
    EXPECT_EQ(r->state_, StateFollower);
}

struct messageSlice {
    std::vector<raftpb::Message> data;
};

// TestLeaderBcastBeat tests that if the leader receives a heartbeat tick,
// it will send a MsgHeartbeat with m.Index = 0, m.LogTerm=0 and empty entries
// as heartbeat to all followers.
// Reference: section 5.2
TEST(raft, TestLeaderBcastBeat) {
	// heartbeat interval
	size_t hi = 1;
	auto r = newTestRaft(1, 10, hi, newTestMemoryStorage({withPeers({1, 2, 3})}));
	r->becomeCandidate();
	r->becomeLeader();
    for (uint64_t i = 0; i < 10; i++) {
        raftpb::Entry entry;
        entry.set_index(i+1);
		mustAppendEntry(r, {entry});
	}
    for (size_t i = 0; i < hi; i++) {
		r->tick_();
	}

	auto msgs = readMessages(r);
    std::sort(msgs.begin(), msgs.end(), [](const raftpb::Message& l, const raftpb::Message& r) {
        return l.ShortDebugString() < r.ShortDebugString();
    });

    std::vector<raftpb::Message> wmsgs = {MessageHelper{.from = 1,.to = 2,.term = 1, .type = raftpb::MsgHeartbeat}.Done(),
                                          MessageHelper{.from = 1,.to = 3,.term = 1, .type = raftpb::MsgHeartbeat}.Done()};
    EXPECT_TRUE(VectorEquals(msgs, wmsgs));
}

// testNonleaderStartElection tests that if a follower receives no communication
// over election timeout, it begins an election to choose a new leader. It
// increments its current term and transitions to candidate state. It then
// votes for itself and issues RequestVote RPCs in parallel to each of the
// other servers in the cluster.
// Reference: section 5.2
// Also if a candidate fails to obtain a majority, it will time out and
// start a new election by incrementing its term and initiating another
// round of RequestVote RPCs.
// Reference: section 5.2
void testNonleaderStartElection(StateType state) {
	// election timeout
	auto et = 10;
	auto r = newTestRaft(1, et, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
	switch (state) {
        case StateFollower:
            r->becomeFollower(1, 2);
            break;
        case StateCandidate:
            r->becomeCandidate();
            break;
        default:
            break;
	};
    for (auto i = 0; i < 2*et; i++) {
		r->tick_();
	}
	advanceMessagesAfterAppend(r);

    EXPECT_EQ(r->term_, 2);
    EXPECT_EQ(r->state_, StateCandidate);
    EXPECT_TRUE(r->trk_.votes_.at(r->id_));
	auto msgs = readMessages(r);
    std::sort(msgs.begin(), msgs.end(), [](const raftpb::Message& l, const raftpb::Message& r) {
        return l.ShortDebugString() < r.ShortDebugString();
    });
    std::vector<raftpb::Message> wmsgs = {MessageHelper{.from = 1,.to = 2,.term = 2,.type = raftpb::MsgVote}.Done(),
                                          MessageHelper{.from = 1,.to = 3,.term = 2,.type = raftpb::MsgVote}.Done()};
    EXPECT_TRUE(VectorEquals(msgs, wmsgs));
}

TEST(raft, TestFollowerStartElection) {
	testNonleaderStartElection(StateFollower);
}

TEST(raft, TestCandidateStartNewElection) {
	testNonleaderStartElection(StateCandidate);
}

// TestLeaderElectionInOneRoundRPC tests all cases that may happen in
// leader election during one round of RequestVote RPC:
// a) it wins the election
// b) it loses the election
// c) it is unclear about the result
// Reference: section 5.2
TEST(raft, TestLeaderElectionInOneRoundRPC) {
    struct Test {
        int size;
        std::map<uint64_t, bool> votes;
        StateType state;
    };
    std::vector<Test> tests = {
		// win the election when receiving votes from a majority of the servers
		{1, {}, StateLeader},
		{3, {{2, true}, {3, true}}, StateLeader},
		{3, {{2, true}}, StateLeader},
		{5, {{2, true}, {3, true}, {4, true}, {5, true}}, StateLeader},
		{5, {{2, true}, {3, true}, {4, true}}, StateLeader},
		{5, {{2, true}, {3, true}}, StateLeader},

		// return to follower state if it receives vote denial from a majority
		{3, {{2, false}, {3, false}}, StateFollower},
		{5, {{2, false}, {3, false}, {4, false}, {5, false}}, StateFollower},
		{5, {{2, true}, {3, false}, {4, false}, {5, false}}, StateFollower},

		// stay in candidate if it does not obtain the majority
		{3, {}, StateCandidate},
		{5, {{2, true}}, StateCandidate},
		{5, {{2, false}, {3, false}}, StateCandidate},
		{5, {}, StateCandidate},
	};
    for (auto& tt : tests) {
		auto r = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers(idsBySize(tt.size))}));
		r->Step(MessageHelper{.from = 1, .to = 1, .type = raftpb::MsgHup}.Done());
		advanceMessagesAfterAppend(r);
        for (auto& pair : tt.votes) {
            auto m = MessageHelper{.from = pair.first, .to = 1, .term = r->term_, .type = raftpb::MsgVoteResp}.Done();
            m.set_type(raftpb::MsgVoteResp);
            m.set_reject(!pair.second);
            r->Step(m);
        }
        EXPECT_EQ(r->state_, tt.state);
        EXPECT_EQ(r->term_, 1);
	}
}

// TestFollowerVote tests that each follower will vote for at most one
// candidate in a given term, on a first-come-first-served basis.
// Reference: section 5.2
TEST(raft, TestFollowerVote) {
    struct Test {
        uint64_t vote;
        uint64_t nvote;
        bool wreject;
    };
    std::vector<Test> tests = {
		{None, 2, false},
		{None, 3, false},
		{2, 2, false},
		{3, 3, false},
		{2, 3, true},
		{3, 2, true},
	};
    for (auto& tt : tests) {
		auto r = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
        raftpb::HardState hs;
        hs.set_term(1);
        hs.set_vote(tt.vote);
		r->loadState(hs);
		r->Step(MessageHelper{.from = tt.nvote, .to = 1, .term = 1, .type = raftpb::MsgVote}.Done());

		auto msgs = r->msgsAfterAppend_;
        auto m = MessageHelper{.from = 1, .to = tt.nvote, .type = raftpb::MsgVoteResp}.Done();
        m.set_term(1);
        m.set_reject(tt.wreject);
        std::vector<raftpb::Message> wmsgs = {m};
        EXPECT_TRUE(VectorEquals(msgs, wmsgs));
	}
}

// TestCandidateFallback tests that while waiting for votes,
// if a candidate receives an AppendEntries RPC from another server claiming
// to be leader whose term is at least as large as the candidate's current term,
// it recognizes the leader as legitimate and returns to follower state.
// Reference: section 5.2
TEST(raft, TestCandidateFallback) {
    std::vector<raftpb::Message> tests = {MessageHelper{.from = 2,.to = 1,.term = 1,.type = raftpb::MsgApp}.Done(),
                                          MessageHelper{.from = 2,.to = 1,.term = 2,.type = raftpb::MsgApp}.Done()};
    for (auto& tt : tests) {
		auto r = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
		r->Step(MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgHup}.Done());
        EXPECT_EQ(r->state_, StateCandidate);
		r->Step(tt);

        EXPECT_EQ(r->state_, StateFollower);
        EXPECT_EQ(r->term_, tt.term());
	}
}

// testNonleaderElectionTimeoutRandomized tests that election timeout for
// follower or candidate is randomized.
// Reference: section 5.2
void testNonleaderElectionTimeoutRandomized(StateType state) {
	int et = 10;
	auto r = newTestRaft(1, et, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
    std::unordered_map<int, bool> timeouts;
    for (auto round = 0; round < 50*et; round++) {
		switch (state) {
            case StateFollower:
                r->becomeFollower(r->term_+1, 2);
                break;
            case StateCandidate:
                r->becomeCandidate();
                    break;
            default:
                break;
		}

		auto time = 0;
		while (readMessages(r).empty()) {
			r->tick_();
			time++;
		}
		timeouts[time] = true;
	}

	for (auto d = et; d < 2*et; d++) {
        EXPECT_TRUE(timeouts.at(d));
	}
}

TEST(raft, TestFollowerElectionTimeoutRandomized) {
	testNonleaderElectionTimeoutRandomized(StateFollower);
}
TEST(raft, TestCandidateElectionTimeoutRandomized) {
	testNonleaderElectionTimeoutRandomized(StateCandidate);
}

// testNonleadersElectionTimeoutNonconflict tests that in most cases only a
// single server(follower or candidate) will time out, which reduces the
// likelihood of split vote in the new election.
// Reference: section 5.2
void testNonleadersElectionTimeoutNonconflict(StateType state) {
	auto et = 10;
	auto size = 5;
    std::vector<std::shared_ptr<raft>> rs(size);
	auto ids = idsBySize(size);
    for (size_t k = 0; k < rs.size(); k++) {
		rs[k] = newTestRaft(ids[k], et, 1, newTestMemoryStorage({withPeers(ids)}));
	}
	auto conflicts = 0;
	for (auto round = 0; round < 1000; round++) {
        for (auto& r : rs) {
			switch (state) {
			case StateFollower:
				r->becomeFollower(r->term_+1, None);
                break;
			case StateCandidate:
				r->becomeCandidate();
                break;
            default:
                break;
			}
		}

		auto timeoutNum = 0;
		while(timeoutNum == 0) {
            for (auto& r : rs) {
				r->tick_();
				if (!readMessages(r).empty()) {
					timeoutNum++;
				}
			}
		}
		// several rafts time out at the same tick
		if (timeoutNum > 1) {
			conflicts++;
		}
	}

    EXPECT_TRUE(float(conflicts)/1000 < 0.3);
}

TEST(raft, TestFollowersElectionTimeoutNonconflict) {
	testNonleadersElectionTimeoutNonconflict(StateFollower);
}

TEST(raft, TestCandidatesElectionTimeoutNonconflict) {
	testNonleadersElectionTimeoutNonconflict(StateCandidate);
}

raftpb::Message acceptAndReply(const raftpb::Message& m) {
    EXPECT_EQ(m.type(), raftpb::MsgApp);
    raftpb::Message ret;
    ret.set_from(m.to());
    ret.set_to(m.from());
    ret.set_term(m.term());
    ret.set_type(raftpb::MsgAppResp);
    ret.set_index(m.index() + m.entries().size());
    return ret;
};

void commitNoopEntry(std::shared_ptr<raft> r, std::shared_ptr<MemoryStorage> s) {
    EXPECT_EQ(r->state_, StateLeader);
	r->bcastAppend();
	// simulate the response of MsgApp
	auto msgs = readMessages(r);
    for (auto& m : msgs) {
        EXPECT_FALSE(m.type() != raftpb::MsgApp || m.entries().size() != 1 || !m.entries(0).data().empty());
		r->Step(acceptAndReply(m));
	}
	// ignore further messages to refresh followers' commit index
	readMessages(r);
	s->Append(r->raftLog_->nextUnstableEnts());
	r->raftLog_->appliedTo(r->raftLog_->committed_, 0 /* size */);
	r->raftLog_->stableTo(r->raftLog_->lastIndex(), r->raftLog_->lastTerm());
}

// TestLeaderStartReplication tests that when receiving client proposals,
// the leader appends the proposal to its log as a new entry, then issues
// AppendEntries RPCs in parallel to each of the other servers to replicate
// the entry. Also, when sending an AppendEntries RPC, the leader includes
// the index and term of the entry in its log that immediately precedes
// the new entries.
// Also, it writes the new entry into stable storage.
// Reference: section 5.3
TEST(raft, TestLeaderStartReplication) {
	auto s = newTestMemoryStorage({withPeers({1, 2, 3})});
	auto r = newTestRaft(1, 10, 1, s);
	r->becomeCandidate();
	r->becomeLeader();
	commitNoopEntry(r, s);
	auto li = r->raftLog_->lastIndex();

    std::vector<raftpb::Entry> ents = {EntryHelper{.data = "some data"}.Done()};
	r->Step(MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = ents}.Done());

    EXPECT_EQ(r->raftLog_->lastIndex(), li+1);
    EXPECT_EQ(r->raftLog_->committed_, li);
	auto msgs = readMessages(r);
    std::sort(msgs.begin(), msgs.end(), [](const raftpb::Message& l, const raftpb::Message& r) {
        return l.ShortDebugString() < r.ShortDebugString();
    });
    std::vector<raftpb::Entry> wents = {EntryHelper{.index = li+1, .term = 1, .data = "some data"}.Done()};
    std::vector<raftpb::Message> wmsgs;
    {
        raftpb::Message tmp;
        tmp.set_from(1);
        tmp.set_to(2);
        tmp.set_term(1);
        tmp.set_type(raftpb::MsgApp);
        tmp.set_index(li);
        tmp.set_logterm(1);
        *tmp.mutable_entries() = {wents.begin(), wents.end()};
        tmp.set_commit(li);
        wmsgs.push_back(tmp);
    }
    {
        raftpb::Message tmp;
        tmp.set_from(1);
        tmp.set_to(3);
        tmp.set_term(1);
        tmp.set_type(raftpb::MsgApp);
        tmp.set_index(li);
        tmp.set_logterm(1);
        *tmp.mutable_entries() = {wents.begin(), wents.end()};
        tmp.set_commit(li);
        wmsgs.push_back(tmp);
    }
    EXPECT_TRUE(VectorEquals(msgs, wmsgs));
    auto g = r->raftLog_->nextUnstableEnts();
    EXPECT_TRUE(VectorEquals(g, wents));
}

// TestLeaderCommitEntry tests that when the entry has been safely replicated,
// the leader gives out the applied entries, which can be applied to its state
// machine.
// Also, the leader keeps track of the highest index it knows to be committed,
// and it includes that index in future AppendEntries RPCs so that the other
// servers eventually find out.
// Reference: section 5.3
TEST(raft, TestLeaderCommitEntry) {
	auto s = newTestMemoryStorage({withPeers({1, 2, 3})});
	auto r = newTestRaft(1, 10, 1, s);
	r->becomeCandidate();
	r->becomeLeader();
	commitNoopEntry(r, s);
	auto li = r->raftLog_->lastIndex();
	r->Step(MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp,.entries =  {EntryHelper{.data = "some data"}.Done()}}.Done());

    for (auto& m : readMessages(r)) {
		r->Step(acceptAndReply(m));
	}
    EXPECT_EQ(r->raftLog_->committed_, li+1);
    std::vector<raftpb::Entry> wents = {EntryHelper{.index = li+1, .term = 1, .data = "some data"}.Done()};
    auto g = r->raftLog_->nextCommittedEnts(true);
    EXPECT_TRUE(VectorEquals(g, wents));
	auto msgs = readMessages(r);
    std::sort(msgs.begin(), msgs.end(), [](const raftpb::Message& l, const raftpb::Message& r) {
        return l.ShortDebugString() < r.ShortDebugString();
    });
    for (size_t i = 0; i < msgs.size(); i++) {
        EXPECT_EQ(i+2, msgs[i].to());
        EXPECT_EQ(msgs[i].type(), raftpb::MsgApp);
        EXPECT_EQ(msgs[i].commit(), li+1);
	}
}

// TestLeaderAcknowledgeCommit tests that a log entry is committed once the
// leader that created the entry has replicated it on a majority of the servers.
// Reference: section 5.3
TEST(raft, TestLeaderAcknowledgeCommit) {
    struct Test {
        int size;
        std::unordered_map<uint64_t, bool> nonLeaderAcceptors;
        bool wack;
    };
    std::vector<Test> tests = {
		{1, {}, true},
		{3, {}, false},
		{3, {{2, true}}, true},
		{3, {{2,true}, {3,true}}, true},
		{5, {}, false},
		{5, {{2, true}}, false},
		{5, {{2,true}, {3,true}}, true},
		{5, {{2,true}, {3,true}, {4,true}}, true},
		{5, {{2,true}, {3,true}, {4,true}, {5,true}}, true},
	};
    for (auto& tt : tests) {
		auto s = newTestMemoryStorage({withPeers(idsBySize(tt.size))});
		auto r = newTestRaft(1, 10, 1, s);
		r->becomeCandidate();
		r->becomeLeader();
		commitNoopEntry(r, s);
		auto li = r->raftLog_->lastIndex();
		r->Step(MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "some data"}.Done()}}.Done());
		advanceMessagesAfterAppend(r);
        auto msgs = r->msgs_;
        for (auto& m : msgs) {
			if (tt.nonLeaderAcceptors.count(m.to()) && tt.nonLeaderAcceptors.at(m.to())) {
				r->Step(acceptAndReply(m));
			}
		}
        EXPECT_EQ(r->raftLog_->committed_ > li, tt.wack);
	}
}

// TestLeaderCommitPrecedingEntries tests that when leader commits a log entry,
// it also commits all preceding entries in the leader’s log, including
// entries created by previous leaders.
// Also, it applies the entry to its local state machine (in log order).
// Reference: section 5.3
TEST(raft, TestLeaderCommitPrecedingEntries) {
    std::vector<std::vector<raftpb::Entry>> tests = {
            {},{EntryHelper{.index = 1,.term = 2}.Done()}, {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done()}, {EntryHelper{.index = 1,.term = 1}.Done()}
    };
    for (auto& tt : tests) {
		auto storage = newTestMemoryStorage({withPeers({1, 2, 3})});
		storage->Append(tt);
		auto r = newTestRaft(1, 10, 1, storage);
        raftpb::HardState hs;
        hs.set_term(2);
		r->loadState(hs);
		r->becomeCandidate();
		r->becomeLeader();
		r->Step(MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp, .entries = {EntryHelper{.data = "some data"}.Done()}}.Done());

        for (auto& m : readMessages(r)) {
			r->Step(acceptAndReply(m));
		}

		auto li = tt.size();
        auto wents = tt;
        wents.push_back(EntryHelper{.index = li+1, .term = 3}.Done());
        wents.push_back(EntryHelper{.index = li+2, .term = 3, .data = "some data"}.Done());
        auto g = r->raftLog_->nextCommittedEnts(true);
        EXPECT_TRUE(VectorEquals(g, wents));
	}
}

// TestFollowerCommitEntry tests that once a follower learns that a log entry
// is committed, it applies the entry to its local state machine (in log order).
// Reference: section 5.3
TEST(raft, TestFollowerCommitEntry) {
    struct Test {
        std::vector<raftpb::Entry> ents;
        uint64_t commit;
    };
    std::vector<Test> tests = {
		{
            {EntryHelper{.index = 1,.term = 1,.data = "some data"}.Done()},
			1,
		},
		{
            {EntryHelper{.index = 1,.term = 1,.data = "some data"}.Done(), EntryHelper{.index = 2,.term = 1,.data = "some data"}.Done()},
			2,
		},
		{
            {EntryHelper{.index = 1,.term = 1,.data = "some data2"}.Done(), EntryHelper{.index = 2,.term = 1,.data = "some data"}.Done()},
			2,
		},
		{
            {EntryHelper{.index = 1,.term = 1,.data = "some data"}.Done(), EntryHelper{.index = 2,.term = 1,.data = "some data2"}.Done()},
			1,
		}
	};
    for (auto& tt : tests) {
		auto r = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
		r->becomeFollower(1, 2);

        raftpb::Message msg;
        msg.set_from(2);
        msg.set_to(1);
        msg.set_type(raftpb::MsgApp);
        msg.set_term(1);
        *msg.mutable_entries() = {tt.ents.begin(), tt.ents.end()};
        msg.set_commit(tt.commit);
		r->Step(msg);
        EXPECT_EQ(r->raftLog_->committed_, tt.commit);
        std::vector<raftpb::Entry> wents{tt.ents.begin(), tt.ents.begin() + tt.commit};
        auto g = r->raftLog_->nextCommittedEnts(true);
        EXPECT_TRUE(VectorEquals(g, wents));
	}
}

// TestFollowerCheckMsgApp tests that if the follower does not find an
// entry in its log with the same index and term as the one in AppendEntries RPC,
// then it refuses the new entries. Otherwise it replies that it accepts the
// append entries.
// Reference: section 5.3
TEST(raft, TestFollowerCheckMsgApp) {
    std::vector<raftpb::Entry> ents = {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done()};
    struct Test {
        uint64_t term;
        uint64_t index;
        uint64_t windex;
        bool  wreject;
        uint64_t wrejectHint;
        uint64_t wlogterm;
    };
    std::vector<Test> tests = {
		// match with committed entries
		{0, 0, 1, false, 0, 0},
		{ents[0].term(), ents[0].index(), 1, false, 0, 0},
		// match with uncommitted entries
		{ents[1].term(), ents[1].index(), 2, false, 0, 0},

		// unmatch with existing entry
		{ents[0].term(), ents[1].index(), ents[1].index(), true, 1, 1},
		// unexisting entry
		{ents[1].term() + 1, ents[1].index() + 1, ents[1].index() + 1, true, 2, 2},
	};
    for (auto& tt : tests) {
		auto storage = newTestMemoryStorage({withPeers({1, 2, 3})});
		storage->Append(ents);
		auto r = newTestRaft(1, 10, 1, storage);
        raftpb::HardState hs;
        hs.set_commit(1);
		r->loadState(hs);
		r->becomeFollower(2, 2);

        {
            raftpb::Message msg;
            msg.set_from(2);
            msg.set_to(1);
            msg.set_type(raftpb::MsgApp);
            msg.set_term(2);
            msg.set_logterm(tt.term);
            msg.set_index(tt.index);
            r->Step(msg);
        }
		auto msgs = readMessages(r);

        raftpb::Message msg;
        msg.set_from(1);
        msg.set_to(2);
        msg.set_type(raftpb::MsgAppResp);
        msg.set_term(2);
        msg.set_index(tt.windex);
        msg.set_reject(tt.wreject);
        msg.set_rejecthint(tt.wrejectHint);
        msg.set_logterm(tt.wlogterm);
        std::vector<raftpb::Message> wmsgs = {msg};

        EXPECT_TRUE(VectorEquals(msgs, wmsgs));
	}
}

// TestFollowerAppendEntries tests that when AppendEntries RPC is valid,
// the follower will delete the existing conflict entry and all that follow it,
// and append any new entries not already in the log.
// Also, it writes the new entry into stable storage.
// Reference: section 5.3
TEST(raft, TestFollowerAppendEntries) {
    struct Test {
        uint64_t index;
        uint64_t term;
        std::vector<raftpb::Entry> ents;
        std::vector<raftpb::Entry> wents;
        std::vector<raftpb::Entry> wunstable;
    };
    std::vector<Test> tests = {
		{
			2, 2,
            {EntryHelper{.index = 3,.term = 3}.Done()},
            {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done(), EntryHelper{.index = 3,.term = 3}.Done()},
            {EntryHelper{.index = 3,.term = 3}.Done()}
		},
		{
			1, 1,
            {EntryHelper{.index = 2,.term = 3}.Done(), EntryHelper{.index = 3,.term = 4}.Done()},
            {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 3}.Done(), EntryHelper{.index = 3,.term = 4}.Done()},
            {EntryHelper{.index = 2,.term = 3}.Done(), EntryHelper{.index = 3,.term = 4}.Done()},
		},
		{
			0, 0,
            {EntryHelper{.index = 1,.term = 1}.Done()},
            {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done()},
            {},
		},
		{
			0, 0,
            {EntryHelper{.index = 1,.term = 3}.Done()},
            {EntryHelper{.index = 1,.term = 3}.Done()},
            {EntryHelper{.index = 1,.term = 3}.Done()},
		}
	};
    for (auto& tt : tests) {
		auto storage = newTestMemoryStorage({withPeers({1, 2, 3})});
		storage->Append({EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done()});
		auto r = newTestRaft(1, 10, 1, storage);
		r->becomeFollower(2, 2);

        raftpb::Message msg;
        msg.set_from(2);
        msg.set_to(1);
        msg.set_type(raftpb::MsgApp);
        msg.set_term(2);
        msg.set_logterm(tt.term);
        msg.set_index(tt.index);
        *msg.mutable_entries() = {tt.ents.begin(), tt.ents.end()};
		r->Step(msg);

        auto g = r->raftLog_->allEntries();
        EXPECT_TRUE(VectorEquals(g, tt.wents));
        g = r->raftLog_->nextUnstableEnts();
        EXPECT_TRUE(VectorEquals(g, tt.wunstable));
	}
}

// TestVoteRequest tests that the vote request includes information about the candidate’s log
// and are sent to all of the other nodes.
// Reference: section 5.4.1
TEST(raft, TestVoteRequest) {
    struct Test {
        std::vector<raftpb::Entry> ents;
        uint64_t wterm;
    };
    std::vector<Test> tests = {
		{{EntryHelper{.index = 1,.term = 1}.Done()}, 2},
		{{EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done()}, 3},
	};
    for (auto& tt : tests) {
		auto r = newTestRaft(1, 10, 1, newTestMemoryStorage({withPeers({1, 2, 3})}));
        raftpb::Message msg;
        msg.set_from(2);
        msg.set_to(1);
        msg.set_type(raftpb::MsgApp);
        msg.set_term(tt.wterm - 1);
        msg.set_logterm(0);
        msg.set_index(0);
        *msg.mutable_entries() = {tt.ents.begin(), tt.ents.end()};
        r->Step(msg);
		readMessages(r);

		for (auto i = 1; i < r->electionTimeout_*2; i++) {
			r->tickElection();
		}

		auto msgs = readMessages(r);
        std::sort(msgs.begin(), msgs.end(), [](const raftpb::Message& l, const raftpb::Message& r) {
            return l.ShortDebugString() < r.ShortDebugString();
        });
        EXPECT_EQ(msgs.size(), 2);
        for (size_t i = 0; i < msgs.size(); i++) {
            EXPECT_EQ(msgs[i].type(), raftpb::MsgVote);
            EXPECT_EQ(msgs[i].to(), i+2);
            EXPECT_EQ(msgs[i].term(), tt.wterm);
            auto windex = tt.ents[tt.ents.size()-1].index();
            auto wlogterm = tt.ents[tt.ents.size()-1].term();
            EXPECT_EQ(msgs[i].index(), windex);
            EXPECT_EQ(msgs[i].logterm(), wlogterm);
		}
	}
}


// TestVoter tests the voter denies its vote if its own log is more up-to-date
// than that of the candidate.
// Reference: section 5.4.1
TEST(raft, TestVoter) {
    struct Test {
        std::vector<raftpb::Entry> ents;
        uint64_t logterm;
        uint64_t index;
        bool wreject;
    };
    std::vector<Test> tests = {
		// same logterm
		{{EntryHelper{.index = 1,.term = 1}.Done()}, 1, 1, false},
		{{EntryHelper{.index = 1,.term = 1}.Done()}, 1, 2, false},
		{{EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1}.Done()}, 1, 1, true},
		// candidate higher logterm
		{{EntryHelper{.index = 1,.term = 1}.Done()}, 2, 1, false},
		{{EntryHelper{.index = 1,.term = 1}.Done()}, 2, 2, false},
		{{EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 1}.Done()}, 2, 1, false},
		// voter higher logterm
		{{EntryHelper{.index = 1,.term = 2}.Done()}, 1, 1, true},
		{{EntryHelper{.index = 1,.term = 2}.Done()}, 1, 2, true},
		{{EntryHelper{.index = 1,.term = 2}.Done(), EntryHelper{.index = 2,.term = 1}.Done()}, 1, 1, true},
	};
    for (auto& tt : tests) {
		auto storage = newTestMemoryStorage({withPeers({1, 2})});
		storage->Append(tt.ents);
		auto r = newTestRaft(1, 10, 1, storage);
        raftpb::Message msg;
        msg.set_from(2);
        msg.set_to(1);
        msg.set_type(raftpb::MsgVote);
        msg.set_term(3);
        msg.set_logterm(tt.logterm);
        msg.set_index(tt.index);
		r->Step(msg);

		auto msgs = readMessages(r);
        EXPECT_EQ(msgs.size(), 1);
		auto m = msgs[0];
        EXPECT_EQ(m.type(), raftpb::MsgVoteResp);
        EXPECT_EQ(m.reject(), tt.wreject);
	}
}

// TestLeaderOnlyCommitsLogFromCurrentTerm tests that only log entries from the leader’s
// current term are committed by counting replicas.
// Reference: section 5.4.2
TEST(raft, TestLeaderOnlyCommitsLogFromCurrentTerm) {
    std::vector<raftpb::Entry> ents = {EntryHelper{.index = 1,.term = 1}.Done(), EntryHelper{.index = 2,.term = 2}.Done()};
    struct Test {
        uint64_t index;
        uint64_t wcommit;
    };
    std::vector<Test> tests = {
		// do not commit log entries in previous terms
		{1, 0},
		{2, 0},
		// commit log in current term
		{3, 3},
	};
    for (auto& tt : tests) {
		auto storage = newTestMemoryStorage({withPeers({1, 2})});
		storage->Append(ents);
		auto r = newTestRaft(1, 10, 1, storage);
        raftpb::HardState hs;
        hs.set_term(2);
		r->loadState(hs);
		// become leader at term 3
		r->becomeCandidate();
		r->becomeLeader();
		readMessages(r);
		// propose a entry to current term
		r->Step(MessageHelper{.from = 1,.to = 1,.type = raftpb::MsgProp,.entries = {{}}}.Done());

        auto msg = MessageHelper{.from = 2,.to = 1,.term = r->term_, .type = raftpb::MsgAppResp}.Done();
        msg.set_index(tt.index);
		r->Step(msg);
		advanceMessagesAfterAppend(r);
        EXPECT_EQ(r->raftLog_->committed_, tt.wcommit);
	}
}

}