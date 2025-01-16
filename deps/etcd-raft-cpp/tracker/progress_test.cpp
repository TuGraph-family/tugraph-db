#include <gtest/gtest.h>
#include "progress.h"

namespace tracker {

TEST(tracker, TestProgressString) {
	auto ins = NewInflights(1, 0);
	ins->Add(123, 1);
    Progress pr;
    pr.match_ = 1;
    pr.next_ = 2;
    pr.state_ = StateType::StateSnapshot;
    pr.pendingSnapshot_ = 123;
    pr.recentActive_ = false;
    pr.msgAppFlowPaused_ = true;
    pr.isLearner_ = true;
    pr.inflights_ = ins;

	const std::string exp = "StateSnapshot match=1 next=2 learner paused pendingSnap=123 inactive inflight=1[full]";
    EXPECT_EQ(exp, pr.String());
}

TEST(tracker, TestProgressIsPaused) {
    struct Test {
        StateType state;
        bool paused;
        bool w;
    };
    std::vector<Test> tests = {
		{StateType::StateProbe, false, false},
		{StateType::StateProbe, true, true},
		{StateType::StateReplicate, false, false},
		{StateType::StateReplicate, true, true},
		{StateType::StateSnapshot, false, true},
		{StateType::StateSnapshot, true, true},
	};
    for (auto& tt : tests) {
        Progress p;
        p.state_ = tt.state;
        p.msgAppFlowPaused_ = tt.paused;
        p.inflights_ = NewInflights(256, 0);
        EXPECT_EQ(tt.w, p.IsPaused());
	}
}

// TestProgressResume ensures that MaybeUpdate and MaybeDecrTo will reset
// MsgAppFlowPaused.
TEST(tracker, TestProgressResume) {
    Progress p;
    p.next_ = 2;
    p.msgAppFlowPaused_ = true;
	p.MaybeDecrTo(1, 1);
    EXPECT_FALSE(p.msgAppFlowPaused_);
	p.msgAppFlowPaused_ = true;
	p.MaybeUpdate(2);
    EXPECT_FALSE(p.msgAppFlowPaused_);
}

TEST(tracker, TestProgressBecomeProbe) {
	auto match = 1;
    struct Test {
        Progress p;
        uint64_t wnext;
    };
    std::vector<Test> tests;
    tests.emplace_back();
    tests.back().p.state_ = StateType::StateReplicate;
    tests.back().p.match_ = match;
    tests.back().p.next_ = 5;
    tests.back().p.inflights_ = NewInflights(256, 0);
    tests.back().wnext = 2;

    // snapshot finish
    tests.emplace_back();
    tests.back().p.state_ = StateType::StateSnapshot;
    tests.back().p.match_ = match;
    tests.back().p.next_ = 5;
    tests.back().p.pendingSnapshot_ = 10;
    tests.back().p.inflights_ = NewInflights(256, 0);
    tests.back().wnext = 11;

    // snapshot failure
    tests.emplace_back();
    tests.back().p.state_ = StateType::StateSnapshot;
    tests.back().p.match_ = match;
    tests.back().p.next_ = 5;
    tests.back().p.pendingSnapshot_ = 0;
    tests.back().p.inflights_ = NewInflights(256, 0);
    tests.back().wnext = 2;

    for (auto& tt : tests) {
        tt.p.BecomeProbe();
        EXPECT_EQ(StateType::StateProbe, tt.p.state_);
        EXPECT_EQ(match, tt.p.match_);
        EXPECT_EQ(tt.wnext, tt.p.next_);
    }
}

TEST(tracker, TestProgressBecomeReplicate) {
    Progress p;
    p.state_ = StateType::StateProbe;
    p.match_ = 1;
    p.next_ = 5;
    p.inflights_ = NewInflights(256, 0);
	p.BecomeReplicate();
    EXPECT_EQ(StateType::StateReplicate, p.state_);
    EXPECT_EQ(1, p.match_);
    EXPECT_EQ(p.match_+1, p.next_);
}

TEST(tracker, TestProgressBecomeSnapshot) {
    Progress p;
    p.state_ = StateType::StateProbe;
    p.match_ = 1;
    p.next_ = 5;
    p.inflights_ = NewInflights(256, 0);
	p.BecomeSnapshot(10);
    EXPECT_EQ(StateType::StateSnapshot, p.state_);
    EXPECT_EQ(1, p.match_);
    EXPECT_EQ(10, p.pendingSnapshot_);
}

TEST(tracker, TestProgressUpdate) {
    uint64_t prevM = 3;
    uint64_t prevN = 5;
    struct Test {
        uint64_t update;
        uint64_t wm;
        uint64_t wn;
        bool wok;
    };
    std::vector<Test> tests = {
		{prevM - 1, prevM, prevN, false},        // do not decrease match, next
		{prevM, prevM, prevN, false},            // do not decrease next
		{prevM + 1, prevM + 1, prevN, true},     // increase match, do not decrease next
		{prevM + 2, prevM + 2, prevN + 1, true}, // increase match, next
	};
    for (auto& tt : tests) {
        Progress p;
        p.match_ = prevM;
        p.next_ = prevN;
        EXPECT_EQ(tt.wok, p.MaybeUpdate(tt.update));
        EXPECT_EQ(tt.wm, p.match_);
        EXPECT_EQ(tt.wn, p.next_);
	}
}

TEST(tracker, TestProgressMaybeDecr) {
    struct Test {
        StateType state;
        uint64_t m;
        uint64_t n;
        uint64_t rejected;
        uint64_t last;
        bool w;
        uint64_t wn;
    };
    std::vector<Test> tests = {
		{
			// state replicate and rejected is not greater than match
            StateType::StateReplicate, 5, 10, 5, 5, false, 10,
		},
		{
			// state replicate and rejected is not greater than match
            StateType::StateReplicate, 5, 10, 4, 4, false, 10,
		},
		{
			// state replicate and rejected is greater than match
			// directly decrease to match+1
            StateType::StateReplicate, 5, 10, 9, 9, true, 6,
		},
		{
			// next-1 != rejected is always false
            StateType::StateProbe, 0, 0, 0, 0, false, 0,
		},
		{
			// next-1 != rejected is always false
            StateType::StateProbe, 0, 10, 5, 5, false, 10,
		},
		{
			// next>1 = decremented by 1
            StateType::StateProbe, 0, 10, 9, 9, true, 9,
		},
		{
			// next>1 = decremented by 1
            StateType::StateProbe, 0, 2, 1, 1, true, 1,
		},
		{
			// next<=1 = reset to 1
            StateType::StateProbe, 0, 1, 0, 0, true, 1,
		},
		{
			// decrease to min(rejected, last+1)
            StateType::StateProbe, 0, 10, 9, 2, true, 3,
		},
		{
			// rejected < 1, reset to 1
            StateType::StateProbe, 0, 10, 9, 0, true, 1,
		}
	};
    for (auto& tt : tests) {
        Progress p;
        p.state_ = tt.state;
        p.match_ = tt.m;
        p.next_ = tt.n;
        EXPECT_EQ(tt.w, p.MaybeDecrTo(tt.rejected, tt.last));
        EXPECT_EQ(tt.m, p.match_);
        EXPECT_EQ(tt.wn, p.next_);
	}
}

}
