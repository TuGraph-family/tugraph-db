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

TEST(ColumnVectorTest, Constructor) {
    ColumnVector cv(sizeof(int32_t), 10);
    EXPECT_EQ(cv.GetElementSize(), sizeof(int32_t));
    EXPECT_EQ(cv.GetCapacity(), 10);
}

TEST(ColumnVectorTest, SetAndGetValue) {
    ColumnVector cv(sizeof(int32_t), 10);
    int32_t value = 42;
    cv.SetValue<int32_t>(0, value);
    EXPECT_EQ(cv.GetValue<int32_t>(0), value);
}

TEST(ColumnVectorTest, SetNullAndCheck) {
    ColumnVector cv(sizeof(int32_t), 10);
    cv.SetNull(0, true);
    EXPECT_TRUE(cv.IsNull(0));
    cv.SetNull(0, false);
    EXPECT_FALSE(cv.IsNull(0));
}

TEST(StringColumnTest, AddShortString) {
    ColumnVector cv(sizeof(cypher_string_t), 10);
    std::string shortString = "abcd";  // 4 bytes short string
    StringColumn::AddString(&cv, 0, shortString);

    auto& storedString = cv.GetValue<cypher_string_t>(0);
    EXPECT_EQ(storedString.GetAsString(), shortString);
}

TEST(StringColumnTest, AddLongString) {
    ColumnVector cv(sizeof(cypher_string_t), 10);
    std::string longString = "This is a very long string to test overflow buffer.";  // > 12 bytes
    StringColumn::AddString(&cv, 0, longString);

    auto& storedString = cv.GetValue<cypher_string_t>(0);
    EXPECT_EQ(storedString.GetAsString(), longString);
}

TEST(ColumnVectorTest, CopyConstructor) {
    ColumnVector cv1(sizeof(int32_t), 10);
    int32_t value = 99;
    cv1.SetValue<int32_t>(0, value);

    ColumnVector cv2 = cv1;  // Use copy constructor
    EXPECT_EQ(cv2.GetValue<int32_t>(0), value);
}

TEST(ColumnVectorTest, CopyAssignment) {
    ColumnVector cv1(sizeof(int32_t), 10);
    int32_t value = 100;
    cv1.SetValue<int32_t>(0, value);

    ColumnVector cv2(sizeof(int32_t), 5);
    cv2 = cv1;  // Use copy assignment operator
    EXPECT_EQ(cv2.GetValue<int32_t>(0), value);
}

TEST(ColumnVectorTest, ResizeOverflowBuffer) {
    ColumnVector cv(sizeof(cypher_string_t), 1);
    std::string longString = "This string will cause the overflow buffer to resize.";

    // Add long string to trigger overflow buffer allocation
    StringColumn::AddString(&cv, 0, longString);
    EXPECT_EQ(cv.GetValue<cypher_string_t>(0).GetAsString(), longString);

    // Access the overflow buffer to check for resize
    void* initialPtr = cv.AllocateOverflow(1);
    cv.AllocateOverflow(2000);  // Force resize
    void* newPtr = cv.AllocateOverflow(1);
    EXPECT_NE(initialPtr, newPtr);  // Pointer should change after resize
}

TEST(ColumnVectorTest, AccessEmptyVector) {
    ColumnVector cv(sizeof(int32_t), 0);  // 容量为0
    EXPECT_THROW(cv.SetValue<int32_t>(0, 42), std::out_of_range);
    EXPECT_THROW(cv.GetValue<int32_t>(0), std::out_of_range);
}

TEST(ColumnVectorTest, MaximumCapacity) {
    ColumnVector cv(sizeof(int32_t), DEFAULT_VECTOR_CAPACITY);
    EXPECT_NO_THROW(cv.SetValue<int32_t>(DEFAULT_VECTOR_CAPACITY - 1, 42));
    EXPECT_EQ(cv.GetValue<int32_t>(DEFAULT_VECTOR_CAPACITY - 1), 42);
}

TEST(ColumnVectorTest, NegativeIndexAccess) {
    ColumnVector cv(sizeof(int32_t), 10);
    EXPECT_THROW(cv.SetValue<int32_t>(-1, 42), std::out_of_range);
}

TEST(ColumnVectorTest, OverflowBufferExpansion) {
    ColumnVector cv(sizeof(cypher_string_t), 1);

    // Add long string to trigger overflow buffer allocation
    std::string longString(2000, 'x');
    StringColumn::AddString(&cv, 0, longString);
    EXPECT_EQ(cv.GetValue<cypher_string_t>(0).GetAsString(), longString);
}

TEST(ColumnVectorTest, LargeScaleData) {
    size_t largeSize = 1000000;
    ColumnVector cv(sizeof(int32_t), largeSize);

    for (size_t i = 0; i < largeSize; ++i) {
        cv.SetValue<int32_t>(i, static_cast<int32_t>(i));
    }

    for (size_t i = 0; i < largeSize; ++i) {
        EXPECT_EQ(cv.GetValue<int32_t>(i), static_cast<int32_t>(i));
    }
}
