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

#include <functional>
#include <map>
#include <typeinfo>

#include "fma-common/configuration.h"
#include "fma-common/logger.h"
#include "fma-common/utils.h"

#include "gtest/gtest.h"
#include "./ut_utils.h"
#include "core/field_data_helper.h"
#include "./test_tools.h"
using namespace lgraph;
using namespace lgraph::field_data_helper;

class TestFieldDataHelper : public TuGraphTest {};

void CheckFieldTypeSizeNameIsFixed(::lgraph::FieldType ft, const std::string& name, size_t s,
                                   bool is_fixed) {
    using namespace lgraph;
    using namespace lgraph::field_data_helper;
    FieldType parsed;
    UT_EXPECT_TRUE(TryGetFieldType(name, parsed));
    UT_EXPECT_EQ(FieldTypeSize(ft), s);
    UT_EXPECT_EQ(name, FieldTypeName(ft));
    UT_EXPECT_EQ(IsFixedLengthFieldType(ft), is_fixed);
    UT_EXPECT_EQ(TryGetFieldTypeName(ft), name);
}

TEST_F(TestFieldDataHelper, FieldTypeSize) {
    UT_LOG() << "Testing FieldTypeSize, FieldTypeName and IsFixedLengthFieldType";
    for (int ft = (int)FieldType::NUL; ft <= (int)FieldType::BLOB; ft++) {
        FieldType parsed;
        UT_EXPECT_TRUE(TryGetFieldType(FieldTypeName((FieldType)ft), parsed));
        UT_EXPECT_EQ((int)parsed, ft);
        UT_EXPECT_TRUE(TryGetFieldType(fma_common::ToLower(FieldTypeName((FieldType)ft)), parsed));
    }
    FieldType ft;
    UT_EXPECT_TRUE(!TryGetFieldType("non-type", ft));

    CheckFieldTypeSizeNameIsFixed(FieldType::NUL, "NUL", 0, false);
    CheckFieldTypeSizeNameIsFixed(FieldType::BOOL, "BOOL", 1, true);
    CheckFieldTypeSizeNameIsFixed(FieldType::INT8, "INT8", 1, true);
    CheckFieldTypeSizeNameIsFixed(FieldType::INT16, "INT16", 2, true);
    CheckFieldTypeSizeNameIsFixed(FieldType::INT32, "INT32", 4, true);
    CheckFieldTypeSizeNameIsFixed(FieldType::INT64, "INT64", 8, true);
    CheckFieldTypeSizeNameIsFixed(FieldType::FLOAT, "FLOAT", 4, true);
    CheckFieldTypeSizeNameIsFixed(FieldType::DOUBLE, "DOUBLE", 8, true);
    CheckFieldTypeSizeNameIsFixed(FieldType::DATE, "DATE", 4, true);
    CheckFieldTypeSizeNameIsFixed(FieldType::DATETIME, "DATETIME", 8, true);
    CheckFieldTypeSizeNameIsFixed(FieldType::STRING, "STRING", 0, false);
    CheckFieldTypeSizeNameIsFixed(FieldType::BLOB, "BLOB", 0, false);
}

TEST_F(TestFieldDataHelper, FieldTypeStorageType) {
    UT_LOG() << "Testing 2StorageType and 2CType";
#define _CHECK_STORE_AND_C_TYPE(FT, ST, CT)                                                       \
    do {                                                                                          \
        UT_EXPECT_TRUE(                                                                           \
            (std::is_same<typename FieldType2StorageType<FieldType::FT>::type, ST>::value));      \
        UT_EXPECT_TRUE((std::is_same<typename FieldType2CType<FieldType::FT>::type, CT>::value)); \
    } while (0)

    _CHECK_STORE_AND_C_TYPE(NUL, void, void);
    _CHECK_STORE_AND_C_TYPE(BOOL, int8_t, bool);
    _CHECK_STORE_AND_C_TYPE(INT8, int8_t, int8_t);
    _CHECK_STORE_AND_C_TYPE(INT16, int16_t, int16_t);
    _CHECK_STORE_AND_C_TYPE(INT32, int32_t, int32_t);
    _CHECK_STORE_AND_C_TYPE(INT64, int64_t, int64_t);
    _CHECK_STORE_AND_C_TYPE(FLOAT, float, float);
    _CHECK_STORE_AND_C_TYPE(DOUBLE, double, double);
    _CHECK_STORE_AND_C_TYPE(DATE, int32_t, Date);
    _CHECK_STORE_AND_C_TYPE(DATETIME, int64_t, DateTime);
    _CHECK_STORE_AND_C_TYPE(STRING, std::string, std::string);
    _CHECK_STORE_AND_C_TYPE(BLOB, std::string, std::string);
}

TEST_F(TestFieldDataHelper, MakeFieldData) {
    UT_LOG() << "Testing MakeFieldData and GetStoredValue";
#define _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(FT, CD)                          \
    do {                                                                             \
        auto fd = MakeFieldData<FieldType::FT>(CD);                                  \
        UT_EXPECT_EQ(FieldType::FT, fd.type);                                        \
        UT_EXPECT_TRUE(GetStoredValue<FieldType::FT>(fd) ==                          \
                       static_cast<FieldType2StorageType<FieldType::FT>::type>(CD)); \
        UT_EXPECT_TRUE(GetFieldDataCValue<FieldType::FT>(fd) == (CD));               \
    } while (0)

    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(BOOL, true);
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(BOOL, false);
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(INT8, -128);
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(INT8, 10);
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(INT8, 127);
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(INT16, -32768);
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(INT16, 11);
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(INT16, 32767);
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(INT32, -((int64_t)1 << 31));
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(INT32, -100);
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(INT32, (((int64_t)1 << 31) - 1));
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(INT64, -((int64_t)1 << 31));
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(INT64, -100);
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(INT64, (((int64_t)1 << 31) - 1));
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(FLOAT, std::numeric_limits<float>::lowest());
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(FLOAT, 1028);
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(FLOAT, std::numeric_limits<float>::max());
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(DOUBLE, std::numeric_limits<double>::lowest());
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(DOUBLE, 1028);
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(DOUBLE, std::numeric_limits<double>::max());
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(DATE, Date("0000-01-01"));
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(DATE, Date("2090-10-10"));
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(DATE, Date("9999-12-31"));
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(DATETIME, DateTime("0000-01-01 00:00:00"));
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(DATETIME, DateTime("2090-10-10 11:23:58"));
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(DATETIME, DateTime("9999-12-31 23:59:59"));
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(STRING, "123kkkk");
    _CHECK_MAKE_FIELD_DATA_AND_GET_STORED_VALUE(BLOB, std::string(102, -128));
}

TEST_F(TestFieldDataHelper, MinAndMaxValues) {
    UT_LOG() << "Checking Min and Max values";

    UT_EXPECT_EQ(MinStoreValue<FieldType::BOOL>(), (int8_t) false);
    UT_EXPECT_EQ(MaxStoreValue<FieldType::BOOL>(), (int8_t) true);
    UT_EXPECT_EQ(MinStoreValue<FieldType::INT8>(), std::numeric_limits<int8_t>::min());
    UT_EXPECT_EQ(MaxStoreValue<FieldType::INT8>(), std::numeric_limits<int8_t>::max());
    UT_EXPECT_EQ(MinStoreValue<FieldType::INT16>(), std::numeric_limits<int16_t>::min());
    UT_EXPECT_EQ(MaxStoreValue<FieldType::INT16>(), std::numeric_limits<int16_t>::max());
    UT_EXPECT_EQ(MinStoreValue<FieldType::INT32>(), std::numeric_limits<int32_t>::min());
    UT_EXPECT_EQ(MaxStoreValue<FieldType::INT32>(), std::numeric_limits<int32_t>::max());
    UT_EXPECT_EQ(MinStoreValue<FieldType::INT64>(), std::numeric_limits<int64_t>::min());
    UT_EXPECT_EQ(MaxStoreValue<FieldType::INT64>(), std::numeric_limits<int64_t>::max());
    UT_EXPECT_EQ(MinStoreValue<FieldType::FLOAT>(), std::numeric_limits<float>::lowest());
    UT_EXPECT_EQ(MaxStoreValue<FieldType::FLOAT>(), std::numeric_limits<float>::max());
    UT_EXPECT_EQ(MinStoreValue<FieldType::DOUBLE>(), std::numeric_limits<double>::lowest());
    UT_EXPECT_EQ(MaxStoreValue<FieldType::DOUBLE>(), std::numeric_limits<double>::max());
    UT_EXPECT_EQ(MinStoreValue<FieldType::DATE>(), ::lgraph_api::MinDaysSinceEpochForDate());
    UT_EXPECT_EQ(MaxStoreValue<FieldType::INT8>(), std::numeric_limits<int8_t>::max());
}

TEST_F(TestFieldDataHelper, GetConstRefOfFieldDataContent) {
    UT_LOG() << "Testing GetConstRefOfFieldDataContent";

#define _CHECK_CONST_REF(FT, D)                                                                  \
    do {                                                                                         \
        typedef typename FieldType2CType<FieldType::FT>::type CT;                                \
        CT cd = static_cast<CT>(D);                                                              \
        UT_EXPECT_TRUE(                                                                          \
            GetConstRefOfFieldDataContent(MakeFieldData<FieldType::FT>(cd)).AsType<CT>() == cd); \
    } while (0)

    _CHECK_CONST_REF(BOOL, true);
    _CHECK_CONST_REF(BOOL, false);
    _CHECK_CONST_REF(INT8, 33);
    _CHECK_CONST_REF(INT16, 334);
    _CHECK_CONST_REF(INT32, 31113);
    _CHECK_CONST_REF(INT64, 322223);
    _CHECK_CONST_REF(FLOAT, 322223.222);
    _CHECK_CONST_REF(DOUBLE, -3223);
    _CHECK_CONST_REF(DATE, "2019-01-02");
    _CHECK_CONST_REF(DATETIME, "2019-01-02 02:22:34");
    _CHECK_CONST_REF(STRING, std::string(111, 22));
    _CHECK_CONST_REF(BLOB, std::string(111, -22));
}

TEST_F(TestFieldDataHelper, ParseStringIntoStorageType) {
    UT_LOG() << "Testing ParseStringIntoStorageType";

#define _CHECK_PARSE_STRING_SUCC(FT, s, v)                                \
    do {                                                                  \
        typedef typename FieldType2StorageType<FieldType::FT>::type ST;   \
        ST pd;                                                            \
        UT_EXPECT_TRUE(ParseStringIntoStorageType<FieldType::FT>(s, pd)); \
        UT_EXPECT_EQ(static_cast<ST>(v), pd);                             \
    } while (0)

    _CHECK_PARSE_STRING_SUCC(BOOL, "true", true);
    _CHECK_PARSE_STRING_SUCC(BOOL, "false", false);
    _CHECK_PARSE_STRING_SUCC(BOOL, "1", true);
    _CHECK_PARSE_STRING_SUCC(BOOL, "0", false);
    _CHECK_PARSE_STRING_SUCC(BOOL, "-192", true);
    _CHECK_PARSE_STRING_SUCC(INT8, "-122", -122);
    _CHECK_PARSE_STRING_SUCC(INT8, "127", 127);
    _CHECK_PARSE_STRING_SUCC(INT16, "-32764", -32764);
    _CHECK_PARSE_STRING_SUCC(INT16, "32764", 32764);
    _CHECK_PARSE_STRING_SUCC(INT32, "-4658756", -4658756);
    _CHECK_PARSE_STRING_SUCC(INT32, "4658756", 4658756);
    _CHECK_PARSE_STRING_SUCC(INT64, "-1000000000000", -1000000000000L);
    _CHECK_PARSE_STRING_SUCC(INT64, "1000000000000", 1000000000000L);
    _CHECK_PARSE_STRING_SUCC(DATE, "2019-09-01", Date("2019-09-01").DaysSinceEpoch());
    _CHECK_PARSE_STRING_SUCC(DATETIME, "2019-09-01 09:58:45",
                             DateTime("2019-09-01 09:58:45").MicroSecondsSinceEpoch());
    _CHECK_PARSE_STRING_SUCC(STRING, "str", "str");
    _CHECK_PARSE_STRING_SUCC(BLOB, ::lgraph_api::base64::Encode("orig_str"), "orig_str");

#define _CHECK_PARSE_STRING_FAIL(FT, s)                                    \
    do {                                                                   \
        typedef typename FieldType2StorageType<FieldType::FT>::type ST;    \
        ST pd;                                                             \
        UT_EXPECT_TRUE(!ParseStringIntoStorageType<FieldType::FT>(s, pd)); \
    } while (0)
    _CHECK_PARSE_STRING_FAIL(BOOL, "kk");
    _CHECK_PARSE_STRING_FAIL(INT8, "kkk");
    _CHECK_PARSE_STRING_FAIL(INT8, "999");
    _CHECK_PARSE_STRING_FAIL(INT8, "-999");
    _CHECK_PARSE_STRING_FAIL(INT16, "-33999");
    _CHECK_PARSE_STRING_FAIL(INT16, "33999");
    _CHECK_PARSE_STRING_FAIL(INT32, "10000000000");
    _CHECK_PARSE_STRING_FAIL(INT32, "0.9898");
    _CHECK_PARSE_STRING_FAIL(INT64, "-0.9898");
    _CHECK_PARSE_STRING_FAIL(BLOB, "kkk");
}

TEST_F(TestFieldDataHelper, FieldDataTypeConvert) {
    UT_LOG() << "Testing FieldDataTypeConvert";

#define _TEST_FD_CONVERT(SRC_TYPE, SRC_VAL, DST_TYPE, DST_STORE_VAL, SUCC)        \
    do {                                                                          \
        typedef typename FieldType2StorageType<FieldType::DST_TYPE>::type ST;     \
        ST converted;                                                             \
        UT_EXPECT_EQ(FieldDataTypeConvert<FieldType::DST_TYPE>::Convert(          \
                         MakeFieldData<FieldType::SRC_TYPE>(SRC_VAL), converted), \
                     SUCC);                                                       \
        if (SUCC) {                                                               \
            UT_EXPECT_EQ(converted, static_cast<ST>(DST_STORE_VAL));              \
        }                                                                         \
    } while (0)

    _TEST_FD_CONVERT(INT8, 1, BOOL, 1, true);
    _TEST_FD_CONVERT(INT64, 0, BOOL, 0, true);
    // out of range
    _TEST_FD_CONVERT(INT64, 1000000000L, BOOL, 1, false);
    // bool can only be converted from integer types
    _TEST_FD_CONVERT(DOUBLE, 0.1, BOOL, 1, false);
    _TEST_FD_CONVERT(DATE, Date("2019-01-02"), BOOL, 1, false);
    _TEST_FD_CONVERT(STRING, "true", BOOL, 1, false);
    // int types can be converted from int types
    _TEST_FD_CONVERT(INT8, 127, INT64, 127, true);
    _TEST_FD_CONVERT(INT64, 127, INT8, 127, true);
    _TEST_FD_CONVERT(INT64, 32767, INT16, 32767, true);
    _TEST_FD_CONVERT(INT64, 10000000000L, INT32, 1, false);  // out of range
    _TEST_FD_CONVERT(STRING, "111", INT64, 1, false);
    _TEST_FD_CONVERT(BLOB, "111", INT64, 1, false);
    // floating points can be converted from fp or int types
    _TEST_FD_CONVERT(INT8, 127, FLOAT, 127, true);
    _TEST_FD_CONVERT(INT32, 1 << 30, DOUBLE, 1 << 30, true);
    _TEST_FD_CONVERT(FLOAT, 0.25, DOUBLE, 0.25, true);
    _TEST_FD_CONVERT(DOUBLE, 0.125, FLOAT, 0.125, true);
    _TEST_FD_CONVERT(DATE, Date("2019-01-02"), DOUBLE, 1, false);
    // Date and DateTime can be converted from string
    _TEST_FD_CONVERT(INT32, 127, DATE, 127, false);
    _TEST_FD_CONVERT(INT64, 127, DATETIME, 127, false);
    _TEST_FD_CONVERT(STRING, "2019-01-02", DATE, (int32_t)Date("2019-01-02"), true);
    _TEST_FD_CONVERT(STRING, "2019-01-02 00:09:33", DATETIME,
                     (int64_t)DateTime("2019-01-02 00:09:33"), true);
    // string can only be converted from string
    _TEST_FD_CONVERT(INT8, 127, STRING, "", false);
    _TEST_FD_CONVERT(INT64, 127, STRING, "", false);
    _TEST_FD_CONVERT(DOUBLE, 127.1, STRING, "", false);
    _TEST_FD_CONVERT(DATE, Date("2019-01-02"), STRING, "", false);
    // binary can only be converted from bin or string
    _TEST_FD_CONVERT(STRING, ::lgraph_api::base64::Encode("kkkkkkkk"), BLOB, "kkkkkkkk", true);
    _TEST_FD_CONVERT(STRING, ::lgraph_api::base64::Encode(std::string(11, -99)), BLOB,
                     std::string(11, -99), true);
    _TEST_FD_CONVERT(INT64, 1234, BLOB, "", false);
    _TEST_FD_CONVERT(DOUBLE, 1234, BLOB, "", false);
    _TEST_FD_CONVERT(DATE, Date("2019-01-02"), BLOB, "", false);
}

TEST_F(TestFieldDataHelper, TryFieldDataToValueOfFieldType) {
    UT_LOG() << "Testing TryFieldDataToValueOfFieldType";

#define _TEST_FD_TO_VALUE_OF_FT(SRC_TYPE, SRC_VAL, DST_TYPE, DST_STORE_VAL, SUCC)       \
    do {                                                                                \
        typedef typename FieldType2StorageType<FieldType::DST_TYPE>::type ST;           \
        FieldData fd = MakeFieldData<FieldType::SRC_TYPE>(SRC_VAL);                     \
        Value v;                                                                        \
        UT_EXPECT_EQ(TryFieldDataToValueOfFieldType(fd, FieldType::DST_TYPE, v), SUCC); \
        if (SUCC) {                                                                     \
            UT_EXPECT_EQ(v.AsType<ST>(), static_cast<ST>(DST_STORE_VAL));               \
        }                                                                               \
    } while (0)

    _TEST_FD_TO_VALUE_OF_FT(BOOL, true, BOOL, true, true);
    _TEST_FD_TO_VALUE_OF_FT(INT8, 1, BOOL, true, true);
    _TEST_FD_TO_VALUE_OF_FT(INT8, 0, BOOL, false, true);
    _TEST_FD_TO_VALUE_OF_FT(INT8, 12, BOOL, true, false);
    _TEST_FD_TO_VALUE_OF_FT(BOOL, 1, INT8, 1, true);
    _TEST_FD_TO_VALUE_OF_FT(INT8, 12, INT8, 12, true);
    _TEST_FD_TO_VALUE_OF_FT(INT16, 16, INT16, 16, true);
    _TEST_FD_TO_VALUE_OF_FT(INT64, 32766, INT16, 32766, true);
    _TEST_FD_TO_VALUE_OF_FT(INT32, 16, INT32, 16, true);
    _TEST_FD_TO_VALUE_OF_FT(INT8, 16, INT32, 16, true);
    _TEST_FD_TO_VALUE_OF_FT(INT64, 16, INT64, 16, true);
    _TEST_FD_TO_VALUE_OF_FT(INT32, 1 << 30, INT64, 1 << 30, true);
    _TEST_FD_TO_VALUE_OF_FT(FLOAT, 1 << 30, FLOAT, 1 << 30, true);
    _TEST_FD_TO_VALUE_OF_FT(INT32, 1 << 30, FLOAT, 1 << 30, true);
    _TEST_FD_TO_VALUE_OF_FT(DOUBLE, 1 << 30, DOUBLE, 1 << 30, true);
    _TEST_FD_TO_VALUE_OF_FT(INT32, 1 << 30, DOUBLE, 1 << 30, true);
    _TEST_FD_TO_VALUE_OF_FT(DATE, Date("2000-09-08"), DATE, Date("2000-09-08").DaysSinceEpoch(),
                            true);
    _TEST_FD_TO_VALUE_OF_FT(STRING, "2000-09-08", DATE, Date("2000-09-08").DaysSinceEpoch(), true);
    _TEST_FD_TO_VALUE_OF_FT(DATETIME, DateTime("2000-09-08 09:08:22"), DATETIME,
                            DateTime("2000-09-08 09:08:22").MicroSecondsSinceEpoch(), true);
    _TEST_FD_TO_VALUE_OF_FT(STRING, "2000-09-08 09:08:22", DATETIME,
                            DateTime("2000-09-08 09:08:22").MicroSecondsSinceEpoch(), true);
    _TEST_FD_TO_VALUE_OF_FT(STRING, "hello", STRING, "hello", true);
    _TEST_FD_TO_VALUE_OF_FT(INT32, 12, STRING, "hello", false);
    _TEST_FD_TO_VALUE_OF_FT(BLOB, "hello", BLOB, "hello", true);
    _TEST_FD_TO_VALUE_OF_FT(STRING, ::lgraph_api::base64::Encode("hello"), BLOB, "hello", true);
}

TEST_F(TestFieldDataHelper, FieldDataToValueOfFieldType) {
    UT_LOG() << "Testing FieldDataToValueOfFieldType";

#define _TEST_CALL_FD_TO_VALUE_OF_FT(SRC_TYPE, SRC_VAL, DST_TYPE, DST_STORE_VAL) \
    do {                                                                         \
        typedef typename FieldType2StorageType<FieldType::DST_TYPE>::type ST;    \
        FieldData fd = MakeFieldData<FieldType::SRC_TYPE>(SRC_VAL);              \
        Value v;                                                                 \
        FieldDataToValueOfFieldType(fd, FieldType::DST_TYPE);                    \
    } while (0);

    UT_EXPECT_ANY_THROW(_TEST_CALL_FD_TO_VALUE_OF_FT(BLOB, "hellostring", STRING, "hellostring"));
}

TEST_F(TestFieldDataHelper, ParseStringToValueOfFieldType) {
    UT_LOG() << "Testing ParseStringToValueOfFieldType";

#define _TEST_PARSE_TO_V_OF_FT(FT, STR, LOG_EXPECT_STORE_VAL)                \
    do {                                                                     \
        typedef typename FieldType2StorageType<FieldType::FT>::type ST;      \
        Value v = ParseStringToValueOfFieldType(STR, FieldType::FT);         \
        UT_EXPECT_EQ(v.AsType<ST>(), static_cast<ST>(LOG_EXPECT_STORE_VAL)); \
    } while (0);

    _TEST_PARSE_TO_V_OF_FT(BOOL, "true", 1);
    _TEST_PARSE_TO_V_OF_FT(BOOL, "false", 0);
    _TEST_PARSE_TO_V_OF_FT(INT8, "111", 111);
    _TEST_PARSE_TO_V_OF_FT(INT16, "-22111", -22111);
    _TEST_PARSE_TO_V_OF_FT(INT32, "65536", 65536);
    _TEST_PARSE_TO_V_OF_FT(INT64, "10000000000", 10000000000L);
    _TEST_PARSE_TO_V_OF_FT(FLOAT, "3.4123", 3.4123);
    _TEST_PARSE_TO_V_OF_FT(DOUBLE, "3.4123", 3.4123);
    _TEST_PARSE_TO_V_OF_FT(DATE, "1000-01-02", Date("1000-01-02").DaysSinceEpoch());
    _TEST_PARSE_TO_V_OF_FT(DATETIME, "1000-01-01 01:01:01",
                           DateTime("1000-01-01 01:01:01").MicroSecondsSinceEpoch());
    _TEST_PARSE_TO_V_OF_FT(BLOB, "aGVsbG9zdHJpbmc=", "hellostring");

    UT_EXPECT_ANY_THROW(_TEST_PARSE_TO_V_OF_FT(BLOB, "abc", "abc"));
    UT_EXPECT_ANY_THROW(_TEST_PARSE_TO_V_OF_FT(BOOL, "tr", 1));
    UT_EXPECT_ANY_THROW(_TEST_PARSE_TO_V_OF_FT(BOOL, "false1", 0));
    UT_EXPECT_ANY_THROW(_TEST_PARSE_TO_V_OF_FT(INT8, "128", 12));
    UT_EXPECT_ANY_THROW(_TEST_PARSE_TO_V_OF_FT(INT16, "-a22111", -22111));
    UT_EXPECT_ANY_THROW(_TEST_PARSE_TO_V_OF_FT(INT32, "10000000000", 65536));
    UT_EXPECT_ANY_THROW(_TEST_PARSE_TO_V_OF_FT(INT64, "2.341", 2.341));
    UT_EXPECT_ANY_THROW(_TEST_PARSE_TO_V_OF_FT(FLOAT, "3.4k", 3.4));
    UT_EXPECT_ANY_THROW(_TEST_PARSE_TO_V_OF_FT(DOUBLE, "2019-02-02", 3.4123));
    UT_EXPECT_ANY_THROW(
        _TEST_PARSE_TO_V_OF_FT(DATE, "1000-01-02 00:01:02", Date("1000-01-02").DaysSinceEpoch()));
    UT_EXPECT_ANY_THROW(_TEST_PARSE_TO_V_OF_FT(
        DATETIME, "1000-01-01", DateTime("1000-01-01 01:01:01").MicroSecondsSinceEpoch()));
}

TEST_F(TestFieldDataHelper, ValueCompare) {
    UT_LOG() << "Testing ValueCompare";
#define _TEST_VALUE_COMPARE(FT, VAL_A, VAL_B, RES)                                       \
    do {                                                                                 \
        FieldData a = MakeFieldData<FieldType::FT>(VAL_A);                               \
        FieldData b = MakeFieldData<FieldType::FT>(VAL_B);                               \
        Value va = Value::ConstRef(GetStoredValue<FieldType::FT>(a));                    \
        Value vb = Value::ConstRef(GetStoredValue<FieldType::FT>(b));                    \
        int r = ValueCompare<FieldType::FT>(va.Data(), va.Size(), vb.Data(), vb.Size()); \
        if ((RES) == 0) {                                                                \
            UT_EXPECT_EQ(r, (RES));                                                      \
        } else if ((RES) == 1) {                                                         \
            UT_EXPECT_GT(r, 0);                                                          \
        } else if ((RES) == -1) {                                                        \
            UT_EXPECT_LT(r, 0);                                                          \
        }                                                                                \
    } while (0)

    _TEST_VALUE_COMPARE(BOOL, true, true, 0);
    _TEST_VALUE_COMPARE(BOOL, true, false, 1);
    _TEST_VALUE_COMPARE(BOOL, false, true, -1);
    _TEST_VALUE_COMPARE(INT8, 100, 100, 0);
    _TEST_VALUE_COMPARE(INT8, 100, -100, 1);
    _TEST_VALUE_COMPARE(INT8, -128, 127, -1);
    _TEST_VALUE_COMPARE(INT16, 1000, 1000, 0);
    _TEST_VALUE_COMPARE(INT16, 1000, -1000, 1);
    _TEST_VALUE_COMPARE(INT16, -32768, -1000, -1);
    _TEST_VALUE_COMPARE(INT32, -327680, -1000, -1);
    _TEST_VALUE_COMPARE(INT64, -3276800000LL, -1000, -1);
    _TEST_VALUE_COMPARE(FLOAT, (float)-3276800000.222, -1000, -1);
    _TEST_VALUE_COMPARE(DOUBLE, 3276800000.222, -1000, 1);
    _TEST_VALUE_COMPARE(DATE, Date("2222-02-01"), Date("2222-02-01"), 0);
    _TEST_VALUE_COMPARE(DATE, Date("2222-02-01"), Date("2222-02-02"), -1);
    _TEST_VALUE_COMPARE(DATE, Date("1222-02-01"), Date("0222-02-02"), 1);
    _TEST_VALUE_COMPARE(DATETIME, DateTime("1222-02-01 23:23:34"), DateTime("1222-02-01 23:23:34"),
                        0);
    _TEST_VALUE_COMPARE(DATETIME, DateTime("0222-02-01 23:23:34"), DateTime("1222-02-01 23:23:34"),
                        -1);
    _TEST_VALUE_COMPARE(DATETIME, DateTime("9222-02-01 23:23:34"), DateTime("1222-02-01 23:23:34"),
                        1);
    _TEST_VALUE_COMPARE(STRING, std::string(3, 'a'), std::string(3, 'a'), 0);
    _TEST_VALUE_COMPARE(STRING, std::string(3, 'a'), std::string(4, 'a'), -1);
    _TEST_VALUE_COMPARE(STRING, std::string(30, 'a'), std::string(4, 'a'), 1);
    _TEST_VALUE_COMPARE(STRING, std::string(1, 'b'), std::string(4, 'a'), 1);
    _TEST_VALUE_COMPARE(BLOB, std::string(3, 'a'), std::string(3, 'a'), 0);
    _TEST_VALUE_COMPARE(BLOB, std::string(3, 'a'), std::string(4, 'a'), -1);
    _TEST_VALUE_COMPARE(BLOB, std::string(30, 'a'), std::string(4, 'a'), 1);
    _TEST_VALUE_COMPARE(BLOB, std::string(30, 'a'), std::string(4, 'a'), 1);
}
