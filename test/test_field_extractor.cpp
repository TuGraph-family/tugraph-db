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
template <>
inline lgraph::PointWgs84 ParseStringToT<lgraph::PointWgs84>(const std::string& str) {
    return lgraph::PointWgs84(str);
}
template<>
inline lgraph::PointCartesian ParseStringToT<lgraph::PointCartesian>
(const std::string &str) {
    return lgraph::PointCartesian(str);
}
template <>
inline lgraph::LineStringWgs84 ParseStringToT<lgraph::LineStringWgs84>(const std::string& str) {
    return lgraph::LineStringWgs84(str);
}
template<>
inline lgraph::LineStringCartesian ParseStringToT<lgraph::LineStringCartesian>
(const std::string &str) {
    return lgraph::LineStringCartesian(str);
}
template <>
inline lgraph::PolygonWgs84 ParseStringToT<lgraph::PolygonWgs84>(const std::string& str) {
    return lgraph::PolygonWgs84(str);
}
template<>
inline lgraph::PolygonCartesian ParseStringToT<lgraph::PolygonCartesian>
(const std::string &str) {
    return lgraph::PolygonCartesian(str);
}
template <>
inline lgraph::SpatialWgs84 ParseStringToT<lgraph::SpatialWgs84>(const std::string& str) {
    return lgraph::SpatialWgs84(str);
}
template<>
inline lgraph::SpatialCartesian ParseStringToT<lgraph::SpatialCartesian>
(const std::string &str) {
    return lgraph::SpatialCartesian(str);
}
template <>
inline std::vector<float> ParseStringToT<std::vector<float>>(const std::string& str) {
    std::vector<float> vec;
    // check if there are only numbers and commas
    std::regex nonNumbersAndCommas("[^0-9,.]");
    if (std::regex_search(str, nonNumbersAndCommas)) {
        THROW_CODE(InputError, "This is not a float vector string");
    }
    // Check if the string conforms to the following format : 1.000000,2.000000,3.000000,...
    std::regex vector("^(?:[-+]?\\d*(?:\\.\\d+)?)(?:,[-+]?\\d*(?:\\.\\d+)?){1,}$");
    if (!std::regex_match(str, vector)) {
        THROW_CODE(InputError, "This is not a float vector string");
    }
    // check if there are 1.000,,2.000 & 1.000,2.000,
    if (str.front() == ',' || str.back() == ',' || str.find(",,") != std::string::npos) {
        THROW_CODE(InputError, "This is not a float vector string");
    }
    std::regex pattern("-?[0-9]+\\.?[0-9]*");
    std::sregex_iterator begin_it(str.begin(), str.end(), pattern), end_it;
    while (begin_it != end_it) {
        std::smatch match = *begin_it;
        vec.push_back(std::stof(match.str()));
        ++begin_it;
    }
    return vec;
}

template <typename T, typename T2 = int64_t>
static void CheckParseDataType(FieldType ft, Value& v, const std::string& str_ok,
                               const FieldData& fd_ok, const std::string& str_fail,
                               const FieldData& fd_fail, bool test_out_of_range = false,
                               const T2& out_of_range = T2()) {
    _detail::FieldExtractorV1 fe_nul(FieldSpec("f", ft, true));
    fe_nul.ParseAndSet(v, FieldData());
    UT_EXPECT_TRUE(fe_nul.GetIsNull(v));
    fe_nul.ParseAndSet(v, "");
    UT_EXPECT_TRUE(fe_nul.GetIsNull(v));

    // cannot be null
    FieldSpec fs("fs", ft, false);
    _detail::FieldExtractorV1 fe(fs);
    UT_EXPECT_THROW_CODE(fe.ParseAndSet(v, FieldData()), FieldCannotBeSetNull);

    // check parse from string
    fe.ParseAndSet(v, str_ok);
    T parsed = ParseStringToT<T>(str_ok);
    // check GetConstRef
    UT_EXPECT_TRUE(fe.GetConstRef(v).AsType<T>() == parsed);
    // check FieldToString
    UT_EXPECT_EQ(UT_FMT("{}", parsed), fe.FieldToString(v));

    // check CopyDataRaw
    _detail::FieldExtractorV1 fe2(fe);
    Value v2(v.Size());
    fe2.CopyDataRaw(v2, v, &fe);
    UT_EXPECT_TRUE(fe2.GetConstRef(v2).AsType<T>() == parsed);

    // check parse from FieldData
    fe.ParseAndSet(v, fd_ok);
    if (ft != FLOAT_VECTOR) {
        UT_EXPECT_TRUE(FieldData(fe.GetConstRef(v).AsType<T>()) == fd_ok);
    } else {
        UT_EXPECT_TRUE(FieldData(fe.GetConstRef(v).AsType<T>()).AsFloatVector() ==
                       fd_ok.AsFloatVector());
    }

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
    _detail::FieldExtractorV1 fe_nul(FieldSpec("f", ft, true));
    if (ft == FieldType::STRING) {
        fe_nul.ParseAndSet(v, FieldData());
        UT_EXPECT_TRUE(fe_nul.GetIsNull(v));
        fe_nul.ParseAndSet(v, "");
        UT_EXPECT_TRUE(!fe_nul.GetIsNull(v));
        UT_EXPECT_TRUE(fe_nul.GetConstRef(v).Empty());
        FieldSpec fs("fs", ft, false);
        _detail::FieldExtractorV1 fe(fs);
        UT_EXPECT_THROW_CODE(fe.ParseAndSet(v, FieldData()), FieldCannotBeSetNull);
        fe.ParseAndSet(v, str_ok);
        UT_EXPECT_EQ(fe.GetConstRef(v).AsType<std::string>(), str_ok);
        fe.ParseAndSet(v, fd_ok);
        UT_EXPECT_TRUE(fe.GetConstRef(v).AsType<std::string>() == fd_ok.AsString());
        UT_EXPECT_THROW_CODE(fe.ParseAndSet(v, str_fail), DataSizeTooLarge);
        UT_EXPECT_THROW_CODE(fe.ParseAndSet(v, fd_fail), ParseIncompatibleType);
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
        _detail::FieldExtractorV1 fe(fs);
        UT_EXPECT_THROW_CODE(fe.ParseAndSetBlob(v, FieldData(), blob_add),
                        FieldCannotBeSetNull);
        fe.ParseAndSetBlob(v, str_ok, blob_add);
        std::string read_str = fe.GetBlobConstRef(v, blob_get).AsType<std::string>();
        std::string decoded = lgraph_api::base64::Decode(str_ok);
        UT_EXPECT_EQ(read_str, decoded);
        fe.ParseAndSetBlob(v, fd_ok, blob_add);
        UT_EXPECT_TRUE(fe.GetBlobConstRef(v, blob_get).AsType<std::string>() == fd_ok.AsBlob());
        UT_EXPECT_THROW(fe.ParseAndSetBlob(v, str_fail, blob_add), lgraph::ParseStringException);
        UT_EXPECT_THROW_CODE(fe.ParseAndSetBlob(v, fd_fail, blob_add),
                        ParseIncompatibleType);
    }
}

TEST_F(TestFieldExtractor, FieldExtractor) {
    UT_LOG() << "Testing FieldExtractor";
    {
        Value value_tmp("teststringconstructor");
        value_tmp = Value(1024, 0);  // make sure this buffer is large enough for following tests

        FieldSpec fd_nul("FieldSpec", lgraph::FieldType::INT8, true);
        _detail::FieldExtractorV1 fe_nul_1(fd_nul);
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
        // testing spatial data;
        CheckParseDataType<lgraph::PointWgs84>(FieldType::POINT, value_tmp,
                           "0101000020E6100000000000000000F03F0000000000000040",
                           FieldData::Point("0101000020E6100000000000000000F03F0000000000000040"),
                           "aasd332423d", FieldData(3215), false);
        CheckParseDataType<lgraph::PointCartesian>(FieldType::POINT, value_tmp,
                           "0101000020231C0000000000000000F03F0000000000000040",
                           FieldData::Point("0101000020231C0000000000000000F03F0000000000000040"),
                           "aasd332423d", FieldData(3215), false);
        CheckParseDataType<lgraph::LineStringCartesian>(FieldType::LINESTRING, value_tmp,
                           "0102000020231C00000300000000000000000000000000000000000000000"
                           "000000000004000000000000000400000000000000840000000000000F03F",
                           FieldData::LineString("0102000020231C000003000000000000"
                           "0000000000000000000000000000000000000000400000000"
                           "0000000400000000000000840000000000000F03F"),
                           "aasd332423d", FieldData(3215), false);
        CheckParseDataType<lgraph::LineStringWgs84>(FieldType::LINESTRING, value_tmp,
                           "0102000020E61000000300000000000000000000000000000000000000000"
                           "000000000004000000000000000400000000000000840000000000000F03F",
                           FieldData::LineString("0102000020E610000003000000000000"
                           "0000000000000000000000000000000000000000400000000"
                           "0000000400000000000000840000000000000F03F"),
                           "aasd332423d", FieldData(3215), false);
        CheckParseDataType<lgraph::PolygonWgs84>(FieldType::POLYGON, value_tmp,
                           "0103000020E6100000010000000500000000000000000000000000000000"
                           "00000000000000000000000000000000001C400000000000001040000000000000"
                           "00400000000000000040000000000000000000000000000000000000000000000000",
                           FieldData::Polygon("0103000020E61000000100000005000000"
                           "000000000000000000000000000000000000000"
                           "0000000000000000000001C400000000000001040000000000000"
                           "00400000000000000040000000000000000000000000000000000000000000000000"),
                           "aasd332423d", FieldData(3215), false);
        CheckParseDataType<lgraph::PolygonCartesian>(FieldType::POLYGON, value_tmp,
                           "0103000020231C0000010000000500000000000000000000000000000000"
                           "00000000000000000000000000000000001C400000000000001040000000000000"
                           "00400000000000000040000000000000000000000000000000000000000000000000",
                           FieldData::Polygon("0103000020231C00000100000005000000"
                           "000000000000000000000000000000000000000"
                           "0000000000000000000001C400000000000001040000000000000"
                           "00400000000000000040000000000000000000000000000000000000000000000000"),
                           "aasd332423d", FieldData(3215), false);
        CheckParseDataType<lgraph::SpatialWgs84>(FieldType::SPATIAL, value_tmp,
                           "0103000020E6100000010000000500000000000000000000000000000000"
                           "00000000000000000000000000000000001C400000000000001040000000000000"
                           "00400000000000000040000000000000000000000000000000000000000000000000",
                           FieldData::Spatial
                           ("0103000020E6100000010000000500000000000000000000000000000000"
                           "00000000000000000000000000000000001C400000000000001040000000000000"
                           "00400000000000000040000000000000000000000000000000000000000000000000"),
                           "aasd332423d", FieldData(3215), false);
        CheckParseDataType<lgraph::SpatialCartesian>(FieldType::SPATIAL, value_tmp,
                           "0102000020231C00000300000000000000000000000000000000000000000"
                           "000000000004000000000000000400000000000000840000000000000F03F",
                           FieldData::Spatial("0102000020231C000003000000000000"
                           "0000000000000000000000000000000000000000400000000"
                           "0000000400000000000000840000000000000F03F"),
                           "aasd332423d", FieldData(3215), false);
        // testing float vector
        std::vector<float> vec1 = {1.111, 2.111, 3.111, 4.111, 5.111};
        CheckParseDataType<std::vector<float>>(
            FieldType::FLOAT_VECTOR, value_tmp, "1.111000,2.111000,3.111000,4.111000,5.111000",
            FieldData::FloatVector(vec1), "abcdefg", FieldData("sad"), false);

        std::vector<float> vec2 = {1111111, 2111111, 3111111, 4111111, 5111111};
        CheckParseDataType<std::vector<float>>(
            FieldType::FLOAT_VECTOR, value_tmp, "1111111,2111111,3111111,4111111,5111111",
            FieldData::FloatVector(vec2), "abcdefg", FieldData("sad"), false);

        std::vector<float> vec3 = {1111, 2111, 3111, 4111, 5111};
        CheckParseDataType<std::vector<float>>(
            FieldType::FLOAT_VECTOR, value_tmp, "1111.000,2111.000,3111.000,4111.000,5111.000",
            FieldData::FloatVector(vec3), "abcdefg", FieldData("sad"), false);

        std::vector<float> vec4 = {111.1111, 222.2222, 333.3333};
        CheckParseDataType<std::vector<float>>(
            FieldType::FLOAT_VECTOR, value_tmp, "111.1111,222.2222,333.3333",
            FieldData::FloatVector(vec4), "abcdefg", FieldData("sad"), false);

        std::vector<float> vec5 = {0.111111, 0.222222, 0.3333333};
        CheckParseDataType<std::vector<float>>(
            FieldType::FLOAT_VECTOR, value_tmp, "0.111111,0.222222,0.3333333",
            FieldData::FloatVector(vec5), "abcdefg", FieldData("sad"), false);

        std::vector<float> vec6 = {111111.0, 222222.0, 333333.0};
        CheckParseDataType<std::vector<float>>(
            FieldType::FLOAT_VECTOR, value_tmp, "111111.0,222222.0,333333.0",
            FieldData::FloatVector(vec6), "abcdefg", FieldData("sad"), false);

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
