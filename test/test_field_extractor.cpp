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

#include "fma-common/logging.h"
#include "fma-common/string_formatter.h"
#include "gtest/gtest.h"

#include "core/schema.h"
#include "core/lmdb_transaction.h"
#include "lgraph/lgraph.h"
#include "./test_tools.h"
#include "./ut_utils.h"

using namespace lgraph;
using namespace lgraph_api;

class TestFieldExtractor : public TuGraphTest {};

template <typename T>
inline T ParseStringToT(const std::string& str) {
    T parsed;
    fma_common::TextParserUtils::ParseT(str, parsed);
    return parsed;
}

template <>
inline lgraph::Date ParseStringToT<lgraph::Date>(const std::string& str) {
    return lgraph::Date(str);
}
template <>
inline lgraph::DateTime ParseStringToT<lgraph::DateTime>(const std::string& str) {
    return lgraph::DateTime(str);
}

template <typename T, typename T2 = int64_t>
static void CheckParseDataType(FieldType ft, Value& v, const std::string& str_ok,
                               const FieldData& fd_ok, const std::string& str_fail,
                               const FieldData& fd_fail, bool test_out_of_range = false,
                               const T2& out_of_range = T2()) {
    _detail::FieldExtractor fe_nul(FieldSpec("f", ft, true));
    fe_nul.ParseAndSet(v, FieldData());
    UT_EXPECT_TRUE(fe_nul.GetIsNull(v));
    fe_nul.ParseAndSet(v, "");
    UT_EXPECT_TRUE(fe_nul.GetIsNull(v));

    // cannot be null
    FieldSpec fs("fs", ft, false);
    _detail::FieldExtractor fe(fs);
    UT_EXPECT_THROW(fe.ParseAndSet(v, FieldData()), lgraph::FieldCannotBeSetNullException);

    // check parse from string
    fe.ParseAndSet(v, str_ok);
    T parsed = ParseStringToT<T>(str_ok);
    // check GetConstRef
    UT_EXPECT_TRUE(fe.GetConstRef(v).AsType<T>() == parsed);
    // check FieldToString
    UT_EXPECT_EQ(UT_FMT("{}", parsed), fe.FieldToString(v));

    // check CopyDataRaw
    _detail::FieldExtractor fe2(fe);
    Value v2(v.Size());
    fe2.CopyDataRaw(v2, v, &fe);
    UT_EXPECT_TRUE(fe2.GetConstRef(v2).AsType<T>() == parsed);

    // check parse from FieldData
    fe.ParseAndSet(v, fd_ok);
    UT_EXPECT_TRUE(FieldData(fe.GetConstRef(v).AsType<T>()) == fd_ok);

    // check parse errors
    UT_EXPECT_THROW(fe.ParseAndSet(v, str_fail), lgraph::ParseStringException);
    UT_EXPECT_THROW(fe.ParseAndSet(v, fd_fail), lgraph::ParseFieldDataException);
    if (test_out_of_range) {
        UT_EXPECT_THROW(fe.ParseAndSet(v, FieldData(out_of_range)),
                        lgraph::ParseFieldDataException);
        UT_EXPECT_THROW(fe.ParseAndSet(v, fma_common::ToString(out_of_range)),
                        lgraph::ParseStringException);
    }
}

static void CheckParseStringAndBlob(FieldType ft, Value& v, const std::string& str_ok,
                                    const FieldData& fd_ok, const std::string& str_fail,
                                    const FieldData& fd_fail) {
    _detail::FieldExtractor fe_nul(FieldSpec("f", ft, true));
    if (ft == FieldType::STRING) {
        fe_nul.ParseAndSet(v, FieldData());
        UT_EXPECT_TRUE(fe_nul.GetIsNull(v));
        fe_nul.ParseAndSet(v, "");
        UT_EXPECT_TRUE(!fe_nul.GetIsNull(v));
        UT_EXPECT_TRUE(fe_nul.GetConstRef(v).Empty());
        FieldSpec fs("fs", ft, false);
        _detail::FieldExtractor fe(fs);
        UT_EXPECT_THROW(fe.ParseAndSet(v, FieldData()), lgraph::FieldCannotBeSetNullException);
        fe.ParseAndSet(v, str_ok);
        UT_EXPECT_EQ(fe.GetConstRef(v).AsType<std::string>(), str_ok);
        fe.ParseAndSet(v, fd_ok);
        UT_EXPECT_TRUE(fe.GetConstRef(v).AsType<std::string>() == fd_ok.AsString());
        UT_EXPECT_THROW(fe.ParseAndSet(v, str_fail), lgraph::DataSizeTooLargeException);
        UT_EXPECT_THROW(fe.ParseAndSet(v, fd_fail), lgraph::ParseIncompatibleTypeException);
    } else {
        std::map<BlobManager::BlobKey, std::string> blob_map;
        BlobManager::BlobKey curr_key = 0;

        auto blob_add = [&](const Value& v) {
            blob_map[curr_key] = v.AsString();
            return curr_key++;
        };
        auto blob_get = [&](const BlobManager::BlobKey& key) { return Value(blob_map[key]); };
        fe_nul.ParseAndSetBlob(v, FieldData(), blob_add);
        UT_EXPECT_TRUE(fe_nul.GetIsNull(v));
        fe_nul.ParseAndSetBlob(v, "", blob_add);
        UT_EXPECT_TRUE(!fe_nul.GetIsNull(v));
        UT_EXPECT_TRUE(fe_nul.GetConstRef(v).Empty());
        FieldSpec fs("fs", ft, false);
        _detail::FieldExtractor fe(fs);
        UT_EXPECT_THROW(fe.ParseAndSetBlob(v, FieldData(), blob_add),
                        lgraph::FieldCannotBeSetNullException);
        fe.ParseAndSetBlob(v, str_ok, blob_add);
        std::string read_str = fe.GetBlobConstRef(v, blob_get).AsType<std::string>();
        std::string decoded = lgraph_api::base64::Decode(str_ok);
        UT_EXPECT_EQ(read_str, decoded);
        fe.ParseAndSetBlob(v, fd_ok, blob_add);
        UT_EXPECT_TRUE(fe.GetBlobConstRef(v, blob_get).AsType<std::string>() == fd_ok.AsBlob());
        UT_EXPECT_THROW(fe.ParseAndSetBlob(v, str_fail, blob_add), lgraph::ParseStringException);
        UT_EXPECT_THROW(fe.ParseAndSetBlob(v, fd_fail, blob_add),
                        lgraph::ParseIncompatibleTypeException);
    }
}

TEST_F(TestFieldExtractor, FieldExtractor) {
    UT_LOG() << "Testing FieldExtractor";
    {
        Value value_tmp("teststringconstructor");
        value_tmp = Value(1024, 0);  // make sure this buffer is large enough for following tests

        FieldSpec fd_nul("FieldSpec", lgraph::FieldType::INT8, true);
        _detail::FieldExtractor fe_nul_1(fd_nul);
        fe_nul_1.ParseAndSet(value_tmp, FieldData());
        UT_EXPECT_TRUE(fe_nul_1.GetConstRef(value_tmp).Empty());

        CheckParseDataType<int8_t>(FieldType::INT8, value_tmp, "12", FieldData(-13), "asdf",
                                   FieldData("1234"), true, 1024);
        CheckParseDataType<int16_t>(FieldType::INT16, value_tmp, "345", FieldData(-(1 << 12)),
                                    "12.34", FieldData(1234 << 16), true, 1 << 16);
        CheckParseDataType<int32_t>(FieldType::INT32, value_tmp, "6789", FieldData(-(1377 << 20)),
                                    "1234-123", FieldData((int64_t)1 << 32), true,
                                    (int64_t)1 << 32);
        CheckParseDataType<int64_t>(FieldType::INT64, value_tmp, "890274", FieldData(-1323121),
                                    "kkk", FieldData("1234.67"));
        CheckParseDataType<float>(FieldType::FLOAT, value_tmp, "12.125", FieldData(-13.5), "float",
                                  FieldData("a1234"));
        CheckParseDataType<double>(FieldType::DOUBLE, value_tmp, "1332", FieldData(-13), "double",
                                   FieldData("d1234"));
        CheckParseDataType<lgraph::Date>(FieldType::DATE, value_tmp, "2019-09-08",
                                         FieldData::Date("2020-05-04"), "2020-11-12 01:02:03",
                                         FieldData("d1234"), true, "10240-09-01");
        CheckParseDataType<lgraph::DateTime>(FieldType::DATETIME, value_tmp, "2019-09-08 00:15:14",
                                             FieldData::DateTime("2020-05-04 20:24:34"),
                                             "not valid date", FieldData(123), true, "10240-09-01");
        CheckParseStringAndBlob(FieldType::STRING, value_tmp, "this is a string",
                                FieldData("another string"),
                                std::string(_detail::MAX_STRING_SIZE + 1, 'a'),
                                FieldData(12));
        CheckParseStringAndBlob(FieldType::BLOB, value_tmp,
                                lgraph_api::base64::Encode(std::string(1024, 'a')),
                                FieldData::Blob(std::string(_detail::MAX_STRING_SIZE + 1, 'b')),
                                "123",  // not base64-encoded
                                FieldData(12));
    }
}
