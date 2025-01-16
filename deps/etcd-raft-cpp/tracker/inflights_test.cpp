#include <gtest/gtest.h>
#include "inflights.h"

namespace tracker {
std::vector<detail::inflight> inflightsBuffer(std::vector<uint64_t> indices, std::vector<uint64_t> sizes) {
    EXPECT_EQ(indices.size(), sizes.size());
    std::vector<detail::inflight> buffer;
    for (size_t i = 0; i < indices.size(); i++) {
        buffer.emplace_back(indices[i], sizes[i]);
    }
	return buffer;
}

TEST(tracker, TestInflightsAdd) {
	// no rotating case
    Inflights in {.size_ = 10, .buffer_ = std::vector<detail::inflight>(10)};
	for (size_t i = 0; i < 5; i++) {
		in.Add(i, 100+i);
	}

    Inflights wantIn {
        .start_ = 0,
        .count_ = 5,
        .bytes_ = 510,
        .size_ = 10,
        .buffer_ = inflightsBuffer(
        {0, 1, 2, 3, 4, 0, 0, 0, 0, 0},
        {100, 101, 102, 103, 104, 0, 0, 0, 0, 0})
    };
    EXPECT_EQ(wantIn, in);

	for (auto i = 5; i < 10; i++) {
		in.Add(i, 100+i);
	}

    Inflights wantIn2 {
        .start_ = 0,
        .count_ = 10,
        .bytes_ = 1045,
        .size_ = 10,
        .buffer_ = inflightsBuffer(
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
    {100, 101, 102, 103, 104, 105, 106, 107, 108, 109})
    };
    EXPECT_EQ(wantIn2, in);

	// rotating case
    Inflights in2 {.start_ = 5, .size_ = 10, .buffer_ = std::vector<detail::inflight>(10)};

	for (auto i = 0; i < 5; i++) {
		in2.Add(i, 100+i);
	}
    Inflights wantIn21 {
        .start_ = 5,
        .count_ = 5,
        .bytes_ = 510,
        .size_ = 10,
        .buffer_ = inflightsBuffer(
        {0, 0, 0, 0, 0, 0, 1, 2, 3, 4},
        {0, 0, 0, 0, 0, 100, 101, 102, 103, 104}),
    };
    EXPECT_EQ(wantIn21, in2);

	for (auto i = 5; i < 10; i++) {
		in2.Add(i, 100+i);
	}
    Inflights wantIn22 {
        .start_ = 5,
        .count_ = 10,
        .bytes_ = 1045,
        .size_ = 10,
        .buffer_ = inflightsBuffer(
                {5, 6, 7, 8, 9, 0, 1, 2, 3, 4},
                {105, 106, 107, 108, 109, 100, 101, 102, 103, 104})
    };
    EXPECT_EQ(wantIn22, in2);
}

TEST(tracker, TestInflightFreeTo) {
	// no rotating case
	auto in = NewInflights(10, 0);
	for (size_t i = 0; i < 10; i++) {
		in->Add(i, 100+i);
	}

	in->FreeLE(0);

	 Inflights wantIn0 {
        .start_ = 1,
        .count_ = 9,
        .bytes_ = 945,
        .size_ = 10,
        .buffer_ = inflightsBuffer(
                {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
                {100, 101, 102, 103, 104, 105, 106, 107, 108, 109}),
	};
    EXPECT_EQ(wantIn0, *in);

	in->FreeLE(4);

    Inflights wantIn {
        .start_ = 5,
        .count_ = 5,
        .bytes_ = 535,
        .size_ = 10,
        .buffer_ = inflightsBuffer(
                {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
                {100, 101, 102, 103, 104, 105, 106, 107, 108, 109}),
    };
    EXPECT_EQ(wantIn, *in);

	in->FreeLE(8);

    Inflights wantIn2 {
        .start_ = 9,
        .count_ = 1,
        .bytes_ = 109,
        .size_ = 10,
        .buffer_ = inflightsBuffer(
                {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
                {100, 101, 102, 103, 104, 105, 106, 107, 108, 109})
    };
    EXPECT_EQ(wantIn2, *in);

	// rotating case
	for (size_t i = 10; i < 15; i++) {
		in->Add(i, 100+i);
	}

	in->FreeLE(12);

    Inflights wantIn3 {
        .start_ = 3,
        .count_ = 2,
        .bytes_ = 227,
        .size_ = 10,
        .buffer_ = inflightsBuffer(
                {10, 11, 12, 13, 14, 5, 6, 7, 8, 9},
                {110, 111, 112, 113, 114, 105, 106, 107, 108, 109})
    };
    EXPECT_EQ(wantIn3, *in);

	in->FreeLE(14);

    Inflights wantIn4 {
        .start_ = 0,
        .count_ = 0,
        .size_ = 10,
        .buffer_ = inflightsBuffer({10, 11, 12, 13, 14, 5, 6, 7, 8, 9},{110, 111, 112, 113, 114, 105, 106, 107, 108, 109})
    };
    EXPECT_EQ(wantIn4, *in);
}

TEST(tracker, TestInflightsFull) {
    struct Test {
        std::string name;
        int size = 0;
        uint64_t maxBytes = 0;
        int fullAt = 0;
        uint64_t freeLE = 0;
        int againAt = 0;
    };
    std::vector<Test> tests = {
		{.name = "always-full", .size = 0, .fullAt = 0},
		{.name = "single-entry", .size = 1, .fullAt = 1, .freeLE = 1, .againAt = 2},
		{.name = "single-entry-overflow", .size = 1, .maxBytes = 10, .fullAt = 1, .freeLE = 1, .againAt = 2},
		{.name = "multi-entry", .size = 15, .fullAt = 15, .freeLE = 6, .againAt = 22},
		{.name = "slight-overflow", .size = 8, .maxBytes = 400, .fullAt = 4, .freeLE = 2, .againAt = 7},
		{.name = "exact-max-bytes", .size = 8, .maxBytes = 406, .fullAt = 4, .freeLE = 3, .againAt = 8},
		{.name = "larger-overflow", .size = 15, .maxBytes = 408, .fullAt = 5, .freeLE = 1, .againAt = 6}
	};
    for (auto& tc : tests) {
        auto in = NewInflights(tc.size, tc.maxBytes);
        auto addUntilFull = [&](int begin, int end) {
            for (auto i = begin; i < end; i++) {
                if (in->Full()) {
                    ERAFT_FATAL("full at %d, want %d", i, end);
                }
                in->Add(i, 100+i);
            }
            if (!in->Full()) {
                ERAFT_FATAL("not full at %d", end);
            }
        };

        addUntilFull(0, tc.fullAt);
        in->FreeLE(tc.freeLE);
        addUntilFull(tc.fullAt, tc.againAt);
        bool panic = false;
        try {
            in->Add(100, 1024);
        } catch (const std::exception& e) {
            EXPECT_EQ(typeid(e), typeid(PanicException));
            panic = true;
        }
        EXPECT_TRUE(panic);
    }
}

TEST(tracker, TestInflightsReset) {
	auto in = NewInflights(10, 1000);
	// Imitate a semi-realistic flow during which the inflight tracker is
	// periodically reset to empty. Byte usage must not "leak" across resets.
	auto index = 0;
	for (int epoch = 0; epoch < 100; epoch++) {
		in->reset();
		// Add 5 messages. They should not max out the limit yet.
		for (int i = 0; i < 5; i++) {
            EXPECT_FALSE(in->Full());
			index++;
			in->Add(index, 16);
		}
		// Ack all but last 2 indices.
		in->FreeLE(index - 2);
        EXPECT_FALSE(in->Full());
        EXPECT_EQ(2, in->Count());
	}
	in->FreeLE(index);
    EXPECT_EQ(0, in->Count());
}

}