/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "gtest/gtest.h"

#include "core/schema_manager.h"
#include "./test_tools.h"
#include "./ut_utils.h"

using namespace fma_common;
using namespace lgraph;

class TestSchemaManager : public TuGraphTest {};

TEST_F(TestSchemaManager, SchemaManager) {
    std::string dir = "./testdb";
    AutoCleanDir dir_cleaner("./testdb");
    {
        KvStore store(dir, 1 << 30, false);
        KvTransaction txn = store.CreateWriteTxn();
        KvTable tbl = SchemaManager::OpenTable(txn, store, "v_schema");
        SchemaManager manager(txn, tbl, true);
        std::vector<FieldSpec> student_fds{{"name", FieldType::STRING, false},
                                           {"id", FieldType::STRING, false},
                                           {"gender", FieldType::INT8, true},
                                           {"age", FieldType::INT8, true}};
        UT_EXPECT_TRUE(manager.AddLabel(txn, true, "student", student_fds.size(),
                                        student_fds.data(), "id", {}));
        auto lid = manager.GetLabelId("student");
        UT_EXPECT_EQ(manager.DeleteLabel(txn, "student"), true);
        UT_EXPECT_EQ(manager.DeleteLabel(txn, "student"), false);
        UT_EXPECT_EQ(manager.AddLabel(txn, true, "student", student_fds.size(), student_fds.data(),
                                      "id", {}),
                     true);
        UT_EXPECT_EQ(manager.AddLabel(txn, true, "student", student_fds.size(), student_fds.data(),
                                      "id", {}),
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
        UT_EXPECT_TRUE(manager.AddLabel(txn, true, "teacher", teacher_fds.size(),
                                        teacher_fds.data(), "id", {}));
        txn.Commit();
    }
    {
        // make sure the changes are properly persisted
        KvStore store(dir, 1 << 30, false);
        KvTransaction txn = store.CreateWriteTxn();
        KvTable tbl = SchemaManager::OpenTable(txn, store, "v_schema");
        SchemaManager manager(txn, tbl, true);
        Schema* schema = manager.GetSchema("student");
        Schema* old_schema = manager.GetSchema(0);
        UT_EXPECT_EQ(schema->GetFieldSpecs().size(), 4);
        UT_EXPECT_EQ(old_schema->GetFieldSpecs().size(), 4);
    }
}
