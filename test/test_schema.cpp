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

class TestSchema : public TuGraphTest {};

static Schema ConstructSimpleSchema() {
    Schema s;
    s.SetSchema(true,
                std::vector<FieldSpec>({FieldSpec("int16", FieldType::INT16, false, 0),
                                        FieldSpec("string", FieldType::STRING, true, 1),
                                        FieldSpec("blob", FieldType::BLOB, true, 2),
                                        FieldSpec("date", FieldType::DATE, false, 3)}),
                "int16", "", {}, {});
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
    UT_EXPECT_THROW_CODE(
        s.SetSchema(true,
                    std::vector<FieldSpec>({FieldSpec("int16", FieldType::INT16, true, 0),
                                            FieldSpec("int16", FieldType::INT16, true, 1)}),
                    "int16", "", {}, {}),
        FieldAlreadyExists);
    UT_EXPECT_THROW_CODE(
        s.SetSchema(true, std::vector<FieldSpec>({FieldSpec("int16", FieldType::NUL, true)}),
                    "int16", "", {}, {}),
        FieldCannotBeNullType);
    UT_EXPECT_THROW_CODE(s.SetSchema(true,
                                     std::vector<FieldSpec>({
                                         FieldSpec("int16", FieldType::INT16, true, 0),
                                         FieldSpec("int16", FieldType::INT16, true, 1),
                                         FieldSpec("int16", FieldType::INT16, true, 1),
                                     }),
                                     "int16", "", {}, {}),
                         FieldIdConflict);
    std::vector<FieldSpec> fs;
    for (size_t i = 0; i < _detail::MAX_NUM_FIELDS + 1; i++)
        fs.emplace_back(UT_FMT("f_{}", i), FieldType::INT16, true);
    UT_EXPECT_THROW_MSG(s.SetSchema(true, fs, "f_0", "", {}, {}), "Invalid Field");
}

TEST_F(TestSchema, HasBlob) {
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

TEST_F(TestSchema, GetFieldExtractor) {
    Schema s = ConstructSimpleSchema();
    for (auto& fs : s.GetFieldSpecs()) UT_EXPECT_TRUE(s.GetFieldExtractor(fs.name));
    for (size_t i = 0; i < s.GetNumFields(); i++) UT_EXPECT_TRUE(s.GetFieldExtractor(i));
    UT_EXPECT_THROW_CODE(s.GetFieldExtractor(s.GetNumFields()), FieldNotFound);
    UT_EXPECT_THROW_CODE(s.GetFieldExtractor("non-existing"), FieldNotFound);
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
    FieldSpec fd_0("name", FieldType::STRING, false, 0);
    FieldSpec fd_1("uid", FieldType::INT32, false, 1);
    FieldSpec fd_2("weight", FieldType::FLOAT, false, 2);
    FieldSpec fd_3("age", FieldType::INT8, true, 3);
    FieldSpec fd_4("addr", FieldType::STRING, true, 4);
    FieldSpec fd_5("float", FieldType::DOUBLE, true, 5);
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
    UT_EXPECT_EQ(schema.GetFieldExtractor("name")->FieldToString(record), "peter");
    UT_EXPECT_EQ(schema.GetFieldExtractor("uid")->FieldToString(record), "101");
    UT_EXPECT_EQ(schema.GetFieldExtractor("weight")->FieldToString(record), "6.525e1");
    UT_EXPECT_EQ(schema.GetFieldExtractor("age")->FieldToString(record), "49");
    UT_EXPECT_EQ(schema.GetFieldExtractor("addr")->FieldToString(record), "fifth avenue");
    UT_EXPECT_THROW_CODE(schema.GetFieldExtractor("hash"), FieldNotFound);
    UT_EXPECT_THROW_CODE(schema.GetFieldExtractor(1024), FieldNotFound);
    const _detail::FieldExtractor fe_temp = *(schema.GetFieldExtractor("name"));
    _detail::FieldExtractor fe_5(*schema.GetFieldExtractor(0));
}

TEST_F(TestSchema, ParseAndSetStringBlob) {
    Schema schema(true);
    Value value;
    schema.SetSchema(true, {FieldSpec("name", FieldType::STRING, true, 0)}, "name", "", {}, {});
    value = schema.CreateEmptyRecord();
    const _detail::FieldExtractor* extra = schema.GetFieldExtractor("name");
    schema.ParseAndSet(value, FieldData(), extra);
    UT_EXPECT_TRUE(extra->GetIsNull(value));
    schema.ParseAndSet(value, "", extra);
    UT_EXPECT_TRUE(!extra->GetIsNull(value));
    UT_EXPECT_TRUE(extra->GetConstRef(value).Empty());

    schema.SetSchema(true, {FieldSpec("name", FieldType::STRING, false, 0)}, "name", "", {}, {});
    extra = schema.GetFieldExtractor("name");
    UT_EXPECT_THROW_CODE(schema.ParseAndSet(value, FieldData(), extra), FieldCannotBeSetNull);
    schema.ParseAndSet(value, "DATA", extra);
    UT_EXPECT_EQ(extra->GetConstRef(value).AsType<std::string>(), std::string("DATA"));
    schema.ParseAndSet(value, FieldData("DATA"), extra);
    UT_EXPECT_TRUE(extra->GetConstRef(value).AsType<std::string>() == "DATA");
    UT_EXPECT_THROW_CODE(
        schema.ParseAndSet(value, std::string(_detail::MAX_STRING_SIZE + 1, 'a'), extra),
        DataSizeTooLarge);
    UT_EXPECT_THROW_CODE(schema.ParseAndSet(value, FieldData(12), extra), ParseIncompatibleType);
    std::map<BlobManager::BlobKey, std::string> blob_map;
    BlobManager::BlobKey curr_key = 0;
    auto blob_add = [&](const Value& v) {
        blob_map[curr_key] = v.AsString();
        return curr_key++;
    };
    auto blob_get = [&](const BlobManager::BlobKey& key) { return Value(blob_map[key]); };
    schema.SetSchema(true, {FieldSpec("blob", FieldType::BLOB, true, 0)}, "blob", "", {}, {});
    extra = schema.GetFieldExtractor("blob");
    schema.ParseAndSetBlob(value, FieldData(), blob_add, extra);
    UT_EXPECT_TRUE(extra->GetIsNull(value));
    schema.SetSchema(true, {FieldSpec("blob", FieldType::BLOB, false, 0)}, "blob", "", {}, {});
    UT_EXPECT_THROW_CODE(schema.ParseAndSetBlob(value, FieldData(), blob_add, extra),
                         FieldCannotBeSetNull);

    schema.ParseAndSetBlob(value, lgraph_api::base64::Encode(std::string(1024, 'a')), blob_add,
                           extra);
    std::string read_str = extra->GetBlobConstRef(value, blob_get).AsType<std::string>();
    std::string decoded =
        lgraph_api::base64::Decode(lgraph_api::base64::Encode(std::string(1024, 'a')));
    UT_EXPECT_EQ(read_str, decoded);
    schema.ParseAndSetBlob(value, FieldData::Blob(std::string(_detail::MAX_STRING_SIZE + 1, 'b')),
                           blob_add, extra);
    UT_EXPECT_TRUE(extra->GetBlobConstRef(value, blob_get).AsType<std::string>() ==
                   FieldData::Blob(std::string(_detail::MAX_STRING_SIZE + 1, 'b')).AsBlob());
    UT_EXPECT_THROW(schema.ParseAndSetBlob(value, "123", blob_add, extra),
                    lgraph::ParseStringException);
    UT_EXPECT_THROW_CODE(schema.ParseAndSetBlob(value, FieldData(10), blob_add, extra),
                         ParseIncompatibleType);
}
template <typename T>
void SetAndCheckData(T data, int ind, Schema& schema, Value& value) {
    schema.SetField(value, ind, data);
    [&]() { };
    UT_EXPECT_EQ(schema.GetField(value, ind, []()->{}), data);

}


TEST_F(TestSchema, TestParseAndSetBase) {
    std::vector<FieldSpec> field_vec;
    for (int i = 1; i <= 16; i++) {
        field_vec.push_back(FieldSpec(lgraph_api::to_string((FieldType)i), (FieldType)i, false, i));
    }
    Schema schema(true);
    schema.SetSchema(true, field_vec, "INT64", "", {}, {});
    Value value = schema.CreateEmptyRecord();

}
