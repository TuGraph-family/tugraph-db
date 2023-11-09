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

#include "gtest/gtest.h"

#include "core/schema_manager.h"
#include "./test_tools.h"
#include "./ut_utils.h"

using namespace fma_common;
using namespace lgraph;

class TestSchemaCommon : public TuGraphTest {};

TEST_F(TestSchemaCommon, SchemaCommon) {
    std::string dir = "./testdb";
    AutoCleanDir dir_cleaner("./testdb");
    {
        auto store = std::make_unique<LMDBKvStore>(dir, 1 << 30, false);
        auto txn = store->CreateWriteTxn();
        auto tbl = SchemaManager::OpenTable(*txn, *store, "v_schema");
        SchemaManager manager(*txn, std::move(tbl), true);
        std::vector<FieldSpec> student_fds{{"name", FieldType::STRING, false},
                                           {"id", FieldType::STRING, false},
                                           {"gender", FieldType::INT8, true},
                                           {"age", FieldType::INT8, true}};
        UT_EXPECT_TRUE(manager.AddLabel(*txn, true, "student", student_fds.size(),
                                        student_fds.data(), VertexOptions("id")));
        auto lid = manager.GetLabelId("student");
        UT_EXPECT_EQ(manager.DeleteLabel(*txn, "student"), true);
        UT_EXPECT_EQ(manager.DeleteLabel(*txn, "student"), false);
        UT_EXPECT_EQ(manager.AddLabel(*txn, true, "student", student_fds.size(), student_fds.data(),
                                      VertexOptions("id")),
                     true);
        UT_EXPECT_EQ(manager.AddLabel(*txn, true, "student", student_fds.size(), student_fds.data(),
                                      VertexOptions("id")),
                     false);
        UT_EXPECT_EQ(manager.GetSchema("student")->GetFieldSpecs().size(), 4);
        UT_EXPECT_EQ(manager.GetLabelId("student"), lid);
        UT_EXPECT_THROW(
            {
                try {
                    manager.GetLabelId("student2");
                } catch (std::exception& e) {
                    // and this tests that it has the correct message
                    std::string result = e.what();
                    UT_EXPECT_NE(result.find("does not exist"), std::string::npos);
                    throw;
                }
            },
            std::exception);
        std::vector<FieldSpec> teacher_fds{{"name", FieldType::STRING, false},
                                           {"id", FieldType::STRING, false},
                                           {"class", FieldType::INT8, true},
                                           {"score", FieldType::FLOAT, true}};
        UT_EXPECT_TRUE(manager.AddLabel(*txn, true, "teacher", teacher_fds.size(),
                                        teacher_fds.data(), VertexOptions("id")));
        txn->Commit();
    }
    {
        // make sure the changes are properly persisted
        auto store = std::make_unique<LMDBKvStore>(dir, 1 << 30, false);
        auto txn = store->CreateWriteTxn();
        auto tbl = SchemaManager::OpenTable(*txn, *store, "v_schema");
        SchemaManager manager(*txn, std::move(tbl), true);
        Schema* schema = manager.GetSchema("student");
        Schema* old_schema = manager.GetSchema(0);
        UT_EXPECT_EQ(schema->GetFieldSpecs().size(), 4);
        UT_EXPECT_EQ(old_schema->GetFieldSpecs().size(), 4);
    }
}
