﻿/**
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

#include "fma-common/logging.h"
#include "fma-common/utils.h"
#include "gtest/gtest.h"

#include "core/schema.h"
#include "core/kv_store_transaction.h"
#include "lgraph/lgraph.h"
#include "./test_tools.h"
using namespace lgraph;
using namespace lgraph_api;

class TestSchema : public TuGraphTest {};

static Schema ConstructSimpleSchema() {
    Schema s;
    s.SetSchema(true,
                std::vector<FieldSpec>({FieldSpec("int16", FieldType::INT16, false),
                                        FieldSpec("string", FieldType::STRING, true),
                                        FieldSpec("blob", FieldType::BLOB, true),
                                        FieldSpec("date", FieldType::DATE, false)}),
                "int16", {});
    return s;
}

class InMemoryBlobManager {
    std::map<BlobManager::BlobKey, Value> map_;
    BlobManager::BlobKey next_key_ = 0;

 public:
    BlobManager::BlobKey Add(const Value& v) {
        map_[next_key_] = v.MakeCopy();
        return next_key_++;
    }

    Value Get(const BlobManager::BlobKey& bk) { return map_[bk]; }
};

TEST_F(TestSchema, LoadStoreSchema) {
    Schema s = ConstructSimpleSchema();
    Value v = s.StoreSchema();
    Schema s2;
    s2.LoadSchema(v);
    UT_EXPECT_TRUE(s.GetFieldSpecsAsMap() == s2.GetFieldSpecsAsMap());
}

TEST_F(TestSchema, ConstructorsAndOperators) {
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

TEST_F(TestSchema, SetSchema) {
    Schema s;
    UT_EXPECT_THROW(
        s.SetSchema(true,
                    std::vector<FieldSpec>({FieldSpec("int16", FieldType::INT16, true),
                                            FieldSpec("int16", FieldType::INT16, true)}),
                    "int16", {}),
        lgraph::FieldAlreadyExistsException);
    UT_EXPECT_THROW(
        s.SetSchema(true, std::vector<FieldSpec>({FieldSpec("int16", FieldType::NUL, true)}),
                    "int16", {}),
        lgraph::FieldCannotBeNullTypeException);
    std::vector<FieldSpec> fs;
    for (size_t i = 0; i < _detail::MAX_NUM_FIELDS + 1; i++)
        fs.emplace_back(UT_FMT("f_{}", i), FieldType::INT16, true);
    UT_EXPECT_THROW(s.SetSchema(true, fs, "f_0", {}), lgraph::TooManyFieldsException);
}

TEST_F(TestSchema, HasBlob) {
    Schema s = ConstructSimpleSchema();
    UT_EXPECT_TRUE(s.HasBlob());
    Schema s2 = s;
    UT_EXPECT_TRUE(s2.HasBlob());
    s.SetSchema(true, std::vector<FieldSpec>({FieldSpec("f", FieldType::INT16, true)}), "f", {});
    UT_EXPECT_TRUE(!s.HasBlob());
    s = s2;
    UT_EXPECT_TRUE(s.HasBlob());
    s.ClearFields();
    UT_EXPECT_TRUE(!s.HasBlob());
}

TEST_F(TestSchema, GetFieldExtractor) {
    Schema s = ConstructSimpleSchema();
    for (auto& fs : s.GetFieldSpecs()) UT_EXPECT_TRUE(s.GetFieldExtractor(fs.name));
    for (size_t i = 0; i < s.GetNumFields(); i++) UT_EXPECT_TRUE(s.GetFieldExtractor(i));
    UT_EXPECT_THROW(s.GetFieldExtractor(s.GetNumFields()), lgraph::FieldNotFoundException);
    UT_EXPECT_THROW(s.GetFieldExtractor("non-existing"), lgraph::FieldNotFoundException);
}

TEST_F(TestSchema, GetFieldId) {
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

TEST_F(TestSchema, DumpRecord) {
    Value v_old("name");
    Value v_new("name1");
    Schema schema(false);
    Schema schema_1(true);
    Schema schema_lg = schema;
    KvTransaction kv_store;
    FieldSpec fd_0("name", FieldType::STRING, false);
    FieldSpec fd_1("uid", FieldType::INT32, false);
    FieldSpec fd_2("weight", FieldType::FLOAT, false);
    FieldSpec fd_3("age", FieldType::INT8, true);
    FieldSpec fd_4("addr", FieldType::STRING, true);
    FieldSpec fd_5("float", FieldType::DOUBLE, true);
    std::vector<FieldSpec> fds{fd_0, fd_1, fd_2, fd_3, fd_4, fd_5};
    schema.SetSchema(true, fds, "uid", {});
    schema_1.SetSchema(true, fds, "uid", {});
    UT_EXPECT_EQ(schema.GetNumFields(), 6);
    UT_LOG() << "size of schema:" << schema.GetNumFields();
    schema.SetSchema(true, fds, "uid", {});
    Value va_tmp = schema.CreateEmptyRecord();
    UT_EXPECT_THROW(schema_1.SetField(va_tmp, (std::string) "name", FieldData()),
                    lgraph::FieldCannotBeSetNullException);
    UT_EXPECT_THROW(schema_1.SetField(va_tmp, (std::string) "age", FieldData(256)),
                    lgraph::ParseFieldDataException);
    UT_EXPECT_THROW(schema_1.SetField(va_tmp, (std::string) "name", FieldData(256)),
                    lgraph::ParseIncompatibleTypeException);
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
        UT_EXPECT_THROW(schema.CreateRecord(fid.size(), fid.data(), value.data()),
                        lgraph::FieldCannotBeSetNullException);
    }

    std::vector<size_t> fid = schema.GetFieldIds({"name", "uid", "weight", "age", "addr"});
    std::vector<std::string> value{"peter", "101", "65.25", "49", "fifth avenue"};
    Value record = schema.CreateRecord(fid.size(), fid.data(), value.data());
    // UT_LOG() << "record: " << schema.DumpRecord(record);
    auto field_id = schema.GetFieldId("float");
    const _detail::FieldExtractor* fe_0 = schema.GetFieldExtractor("name");
    const _detail::FieldExtractor* fe_1 = schema.GetFieldExtractor("uid");
    const _detail::FieldExtractor* fe_2 = schema.GetFieldExtractor("weight");
    const _detail::FieldExtractor* fe_3 = schema.GetFieldExtractor("age");
    const _detail::FieldExtractor* fe_4 = schema.GetFieldExtractor("addr");
    UT_EXPECT_THROW(schema.GetFieldExtractor("hash"), FieldNotFoundException);
    UT_EXPECT_THROW(schema.GetFieldExtractor(1024), FieldNotFoundException);
    const _detail::FieldExtractor fe_temp = *(schema.GetFieldExtractor("name"));
    _detail::FieldExtractor fe_5(*schema.GetFieldExtractor(0));
}
