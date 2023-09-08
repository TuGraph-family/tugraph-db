/**
 * Copyright 2023 AntGroup CO., Ltd.
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

#include <gtest/gtest.h>

#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/AstDumper.h"
#include "geax-front-end/isogql/GQLResolveCtx.h"
#include "geax-front-end/isogql/GQLAstVisitor.h"
#include "geax-front-end/isogql/parser/AntlrGqlParser.h"

#include "geax-front-end/utils/Logging.h"

namespace geax {
namespace frontend {

class GQLVisitorDumperTest : public ::testing::Test {
protected:
    geax::common::ObjectArenaAllocator objAlloc_;
};

TEST_F(GQLVisitorDumperTest, base) {
    GEAXErrorCode ret = GEAXErrorCode::GEAX_SUCCEED;
    std::string path = "../../../../test/case/isogql/all.isogql";
    std::ifstream testFile(path);
    std::string isogql;
    LOG(INFO) << "Begin run TEST: " << path;
    while (std::getline(testFile, isogql)) {
        if (isogql[0] == '-') {
            continue;
        }
        AntlrGqlParser parser(isogql);
        auto rule = parser.gqlRequest();
        if (GEAX_IS_NULL(rule)) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(ERROR) << "Failed to parser: " << KV("gql", isogql) << DK(ret);
            break;
        }

        GQLResolveCtx ctx{objAlloc_};
        GQLAstVisitor visitor{ctx};
        rule->accept(&visitor);
        ret = visitor.error();
        if (GEAX_FAIL(ret)) {
            LOG(ERROR) << "Failed to visit: " << KV("gql", isogql) << DK(ret);
            break;
        }

        auto result = visitor.result();
        if (GEAX_IS_NULL(result)) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(ERROR) << "Result of Visitor is not set: " << KV("gql", isogql) << DK(ret);
            break;
        }

        AstDumper dumper;
        ret = dumper.handle(result);
        if (GEAX_FAIL(ret)) {
            LOG(ERROR) << "Failed to dumper: " << KV("gql", isogql)
                       << DKV("error msg", dumper.error_msg()) << DKV("dump", dumper.dump())
                       << DK(ret);
            break;
        }
    }
    testFile.close();
    ASSERT_TRUE(GEAX_OK(ret));
    LOG(INFO) << "Succeed run TEST: " << path;
}

}  // end of namespace frontend
}  // end of namespace geax
