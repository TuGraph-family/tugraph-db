/**
 * Copyright 2022 AntGroup CO., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#include <memory>
#include <optional>
#include <string>
#include <tuple>

#include "./graph_factory.h"
/* Make sure include graph_factory.h BEFORE antlr4-runtime.h. Otherwise causing the following error:
 * ‘EOF’ was not declared in this scope.
 * For the former (include/butil) uses macro EOF, which is undefined in antlr4. */
#include "./antlr4-runtime.h"
#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/AstDumper.h"
#include "geax-front-end/isogql/GQLResolveCtx.h"
#include "geax-front-end/isogql/GQLAstVisitor.h"
#include "geax-front-end/isogql/parser/AntlrGqlParser.h"

#include "cypher/parser/generated/LcypherLexer.h"
#include "cypher/parser/generated/LcypherParser.h"
#include "cypher/parser/cypher_base_visitor.h"
#include "cypher/parser/cypher_error_listener.h"
#include "cypher/rewriter/GenAnonymousAliasRewriter.h"
#include "fma-common/file_system.h"
#include "db/galaxy.h"
#include "cypher/execution_plan/runtime_context.h"
#include "cypher/execution_plan/execution_plan_v2.h"
#include "lgraph/lgraph_utils.h"
#include "./ut_utils.h"
#include "./ut_config.h"
#include "./ut_types.h"
#include "cypher/resultset/column_vector.h"
#include "cypher/resultset/bit_mask.h"
#include "cypher/resultset/cypher_string_t.h"

using namespace geax::frontend;
using geax::frontend::GEAXErrorCode;

using namespace cypher;

TEST(BitMaskTest, Constructor) {
    uint64_t capacity = 128;
    BitMask bm(capacity);
    // Ensure that initially no bits are set as null
    for (uint64_t i = 0; i < capacity; ++i) {
        EXPECT_FALSE(bm.IsBitSet(i));
    }
    EXPECT_TRUE(bm.HasNoNullsGuarantee());
}

TEST(BitMaskTest, SetAndCheckNullBits) {
    uint64_t capacity = 128;
    BitMask bm(capacity);
    bm.SetBit(10, true);
    bm.SetBit(63, true);
    bm.SetBit(127, true);
    EXPECT_TRUE(bm.IsBitSet(10));
    EXPECT_TRUE(bm.IsBitSet(63));
    EXPECT_TRUE(bm.IsBitSet(127));
    EXPECT_FALSE(bm.IsBitSet(0));
    EXPECT_FALSE(bm.IsBitSet(64));
    EXPECT_FALSE(bm.HasNoNullsGuarantee());
}

TEST(BitMaskTest, SetAllNull) {
    uint64_t capacity = 64;
    BitMask bm(capacity);
    bm.SetAllNull();

    for (uint64_t i = 0; i < capacity; ++i) {
        EXPECT_TRUE(bm.IsBitSet(i));
    }
    EXPECT_FALSE(bm.HasNoNullsGuarantee());
}

TEST(BitMaskTest, SetAllNonNull) {
    uint64_t capacity = 64;
    BitMask bm(capacity);
    bm.SetBit(20, true);
    bm.SetBit(40, true);
    EXPECT_TRUE(bm.IsBitSet(20));
    EXPECT_TRUE(bm.IsBitSet(40));
    bm.SetAllNonNull();

    for (uint64_t i = 0; i < capacity; ++i) {
        EXPECT_FALSE(bm.IsBitSet(i));
    }

    EXPECT_TRUE(bm.HasNoNullsGuarantee());
}

TEST(BitMaskTest, CopyConstructor) {
    uint64_t capacity = 64;
    BitMask bm1(capacity);

    bm1.SetBit(15, true);
    bm1.SetBit(30, true);

    // Use copy constructor
    BitMask bm2 = bm1;
    EXPECT_TRUE(bm2.IsBitSet(15));
    EXPECT_TRUE(bm2.IsBitSet(30));
    EXPECT_FALSE(bm2.IsBitSet(0));
    EXPECT_FALSE(bm2.IsBitSet(63));
}

TEST(BitMaskTest, CopyAssignment) {
    uint64_t capacity1 = 64;
    uint64_t capacity2 = 128;
    BitMask bm1(capacity1);
    BitMask bm2(capacity2);
    bm1.SetBit(10, true);
    bm2.SetBit(100, true);

    // Copy assignment
    bm2 = bm1;
    EXPECT_TRUE(bm2.IsBitSet(10));
}

TEST(BitMaskTest, Resize) {
    uint64_t initial_capacity = 64;
    BitMask bm(initial_capacity);
    bm.SetBit(10, true);
    bm.SetBit(63, true);
    EXPECT_TRUE(bm.IsBitSet(10));
    EXPECT_TRUE(bm.IsBitSet(63));

    // Resize the bitmask
    bm.resize(128);
    EXPECT_TRUE(bm.IsBitSet(10));
    EXPECT_TRUE(bm.IsBitSet(63));
    EXPECT_FALSE(bm.IsBitSet(64));
    EXPECT_FALSE(bm.IsBitSet(127));
}

TEST(BitMaskTest, SetNullFromRange) {
    uint64_t capacity = 128;
    BitMask bm(capacity);

    // Set a range of bits to null
    bm.SetNullFromRange(10, 20, true);

    for (uint64_t i = 10; i < 30; ++i) {
        EXPECT_TRUE(bm.IsBitSet(i));
    }
    EXPECT_FALSE(bm.IsBitSet(9));
    EXPECT_FALSE(bm.IsBitSet(30));
}
