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

#include "fma-common/utils.h"
#include "gtest/gtest.h"

#include "core/schema.h"
#include "core/lmdb_transaction.h"
#include "lgraph/lgraph.h"
#include "./test_tools.h"
using namespace lgraph;
using namespace lgraph_api;

class TestSchemaFastAlter : public TuGraphTest {};

static Schema ConstructSimpleSchema() {
    Schema s;
    s.SetFastAlterSchema(true);
    s.SetSchema(true,
                std::vector<FieldSpec>({FieldSpec("int16", FieldType::INT16, false),
                                        FieldSpec("string", FieldType::STRING, true),
                                        FieldSpec("blob", FieldType::BLOB, true),
                                        FieldSpec("date", FieldType::DATE, false)}),
                "int16", "", {}, {});
    return s;
}

TEST_F(TestSchemaFastAlter, LoadStoreSchema) {
    Schema s = ConstructSimpleSchema();
    Value v = s.StoreSchema();
    Schema s2;
    s2.LoadSchema(v);
    UT_EXPECT_TRUE(s.GetFieldSpecsAsMap() == s2.GetFieldSpecsAsMap());
}

TEST_F(TestSchemaFastAlter, ConstructorsAndOperators) {
    Schema s = ConstructSimpleSchema();
    UT_EXPECT_EQ(s.GetNumFields(), 4);
    Schema s2 = s;
    UT_EXPECT_EQ(s2.GetNumFields(), 4);
    UT_EXPECT_TRUE(s.GetFieldSpecsAsMap() == s2.GetFieldSpecsAsMap());
    Schema s3 = std::move(s2);
    UT_EXPECT_EQ(s3.GetNumFields(), 4);
    UT_EXPECT_TRUE(s.GetFieldSpecsAsMap() == s3.GetFieldSpecsAsMap());
    s2 = s3;
    UT_EXPECT_EQ(s2.GetNumFields(), 4);
    UT_EXPECT_TRUE(s.GetFieldSpecsAsMap() == s2.GetFieldSpecsAsMap());
    s3.ClearFields();
    UT_EXPECT_EQ(s3.GetNumFields(), 0);
}

TEST_F(TestSchemaFastAlter, SetSchema) {
    Schema s;
    UT_EXPECT_THROW_CODE(
        s.SetSchema(true,
                    std::vector<FieldSpec>({FieldSpec("int16", FieldType::INT16, true),
                                            FieldSpec("int16", FieldType::INT16, true)}),
                    "int16", "", {}, {}),
        FieldAlreadyExists);
    UT_EXPECT_THROW_CODE(
        s.SetSchema(true, std::vector<FieldSpec>({FieldSpec("int16", FieldType::NUL, true)}),
                    "int16", "", {}, {}),
        FieldCannotBeNullType);
    std::vector<FieldSpec> fs;
    for (size_t i = 0; i < _detail::MAX_NUM_FIELDS + 1; i++)
        fs.emplace_back(UT_FMT("f_{}", i), FieldType::INT16, true);
    UT_EXPECT_THROW_MSG(s.SetSchema(true, fs, "f_0", "", {}, {}), "Invalid Field");
}

TEST_F(TestSchemaFastAlter, HasBlob) {
    Schema s = ConstructSimpleSchema();
    UT_EXPECT_TRUE(s.HasBlob());
    Schema s2 = s;
    UT_EXPECT_TRUE(s2.HasBlob());
    s.SetSchema(true, std::vector<FieldSpec>({FieldSpec("f", FieldType::INT16, true)}), "f", "", {},
                {});
    UT_EXPECT_TRUE(!s.HasBlob());
    s = s2;
    UT_EXPECT_TRUE(s.HasBlob());
    s.ClearFields();
    UT_EXPECT_TRUE(!s.HasBlob());
}

TEST_F(TestSchemaFastAlter, GetFieldId) {
    Schema s = ConstructSimpleSchema();
    std::vector<std::string> fnames;
    for (auto& fs : s.GetFieldSpecs()) {
        fnames.push_back(fs.name);
    }
    UT_EXPECT_EQ(fnames.size(), 4);
    std::reverse(fnames.begin(), fnames.end());
    std::vector<size_t> fids = s.GetFieldIds(fnames);
    for (size_t i = 0; i < fnames.size(); i++) {
        UT_EXPECT_EQ(s.GetFieldId(fnames[i]), fids[i]);
        size_t fid;
        UT_EXPECT_TRUE(s.TryGetFieldId(fnames[i], fid));
        UT_EXPECT_EQ(fid, fids[i]);
    }
    size_t fid;
    UT_EXPECT_TRUE(!s.TryGetFieldId("non-existing", fid));
}

TEST_F(TestSchemaFastAlter, CreateEmptyRecord) {
    Schema s = ConstructSimpleSchema();
    Value v = s.CreateEmptyRecord();
    // | VERSION 1 byte| LABEL 2 bytes | COUNT 2 bytes | NULLARRAY 1byte| OFFSET 16 bytes |
    // | FIX-DATA 2 + 4 + 4 + 4 = 14 bytes | V-DATA 8 bytes |
    UT_EXPECT_EQ(v.Size(), 44);
    for (size_t i = 0; i < s.GetNumFields(); i++) {
        UT_EXPECT_EQ(s.GetFieldExtractorV2(i)->GetIsNull(v),
                     s.GetFieldExtractorV2(i)->GetFieldSpec().optional);
    }
    UT_EXPECT_EQ(s.GetFieldExtractorV2(0)->GetRecordCount(v), 4);
}

TEST_F(TestSchemaFastAlter, DumpRecord) {
    Value v_old("name");

    Value v_new("name1");
    Schema schema(false);
    Schema schema_1(true);
    schema.SetFastAlterSchema(true);
    schema_1.SetFastAlterSchema(true);
    Schema schema_lg = schema;
    FieldSpec fd_0("name", FieldType::STRING, false);
    FieldSpec fd_1("uid", FieldType::INT32, false);
    FieldSpec fd_2("weight", FieldType::FLOAT, false);
    FieldSpec fd_3("age", FieldType::INT8, true);
    FieldSpec fd_4("addr", FieldType::STRING, true);
    FieldSpec fd_5("float", FieldType::DOUBLE, true);
    std::vector<FieldSpec> fds{fd_0, fd_1, fd_2, fd_3, fd_4, fd_5};
    schema.SetSchema(true, fds, "uid", "", {}, {});
    schema_1.SetSchema(true, fds, "uid", "", {}, {});
    UT_EXPECT_EQ(schema.GetNumFields(), 6);
    UT_LOG() << "size of schema:" << schema.GetNumFields();
    schema.SetSchema(true, fds, "uid", "", {}, {});
    Value va_tmp = schema.CreateEmptyRecord();
    UT_EXPECT_THROW_CODE(schema_1.SetField(va_tmp, (std::string) "name", FieldData()),
                         FieldCannotBeSetNull);
    UT_EXPECT_THROW(schema_1.SetField(va_tmp, (std::string) "age", FieldData(256)),
                    lgraph::ParseFieldDataException);
    UT_EXPECT_THROW_CODE(schema_1.SetField(va_tmp, (std::string) "name", FieldData(256)),
                         ParseIncompatibleType);
    UT_EXPECT_TRUE(schema_1.GetField(va_tmp, (std::string) "does_not_exist",
                                     [](const BlobManager::BlobKey&) { return Value(); }) ==
                   FieldData());

    // update index
    Value v_new_1("name");
    FieldData v_feild_str("weight");
    FieldData v_feild_int((int64_t)12);
    FieldData v_feild_real(12.01);
    FieldData v_feild_nul;
    UT_LOG() << "schema: " << fma_common::ToString(schema.GetFieldSpecs());
    {
        std::vector<size_t> fid = schema.GetFieldIds({"name", "uid", "weight", "age"});
        std::vector<std::string> value{"marko", "one", "80.2", "45"};
        // failed to parse uid
        UT_EXPECT_THROW(schema.CreateRecord(fid.size(), fid.data(), value.data()),
                        lgraph::ParseStringException);
    }
    {
        std::vector<size_t> fid = schema.GetFieldIds({"name", "uid"});
        std::vector<std::string> value{"marko", "300"};
        // missing weight field
        UT_EXPECT_THROW_CODE(schema.CreateRecord(fid.size(), fid.data(), value.data()),
                             FieldCannotBeSetNull);
    }

    std::vector<size_t> fid = schema.GetFieldIds({"name", "uid", "weight", "age", "addr"});
    std::vector<std::string> value{"peter", "101", "65.25", "49", "fifth avenue"};
    Value record = schema.CreateRecord(fid.size(), fid.data(), value.data());
    // UT_LOG() << "record: " << schema.DumpRecord(record);

    UT_EXPECT_EQ(schema.GetFieldId("float"), 5);
    UT_EXPECT_EQ(schema.GetFieldExtractorV2("name")->FieldToString(record), "peter");
    UT_EXPECT_EQ(schema.GetFieldExtractorV2("uid")->FieldToString(record), "101");
    UT_EXPECT_EQ(schema.GetFieldExtractorV2("weight")->FieldToString(record), "6.525e1");
    UT_EXPECT_EQ(schema.GetFieldExtractorV2("age")->FieldToString(record), "49");
    UT_EXPECT_EQ(schema.GetFieldExtractorV2("addr")->FieldToString(record), "fifth avenue");
    UT_EXPECT_THROW_CODE(schema.GetFieldExtractorV2("hash"), FieldNotFound);
    UT_EXPECT_THROW_CODE(schema.GetFieldExtractorV2(1024), FieldNotFound);
}
