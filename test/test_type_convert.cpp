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

#include "core/data_type.h"
#include "core/field_data_helper.h"
#include "core/type_convert.h"
#include "./ut_utils.h"
using namespace lgraph;
using namespace lgraph::field_data_helper;
using namespace lgraph::_detail;

class TestTypeConvert : public TuGraphTest {};

template <typename T>
inline T MaxMinus2() {
    return std::numeric_limits<T>::max() - 2;
}

TEST_F(TestTypeConvert, TypeConvert) {
    UT_EXPECT_EQ(ValueToFieldData(Value::ConstRef(MaxMinus2<int8_t>()), FieldType::INT8).AsInt8(),
                 MaxMinus2<int8_t>());
    UT_EXPECT_EQ(
        ValueToFieldData(Value::ConstRef(MaxMinus2<int16_t>()), FieldType::INT16).AsInt16(),
        MaxMinus2<int16_t>());
    UT_EXPECT_EQ(
        ValueToFieldData(Value::ConstRef(MaxMinus2<int32_t>()), FieldType::INT32).AsInt32(),
        MaxMinus2<int32_t>());
    UT_EXPECT_EQ(
        ValueToFieldData(Value::ConstRef(MaxMinus2<int64_t>()), FieldType::INT64).AsInt64(),
        MaxMinus2<int64_t>());
    UT_EXPECT_EQ(ValueToFieldData(Value::ConstRef(MaxMinus2<float>()), FieldType::FLOAT).AsFloat(),
                 MaxMinus2<float>());
    UT_EXPECT_EQ(
        ValueToFieldData(Value::ConstRef(MaxMinus2<double>()), FieldType::DOUBLE).AsDouble(),
        MaxMinus2<double>());
    std::string blobs = "for blob data";
    UT_EXPECT_EQ(ValueToFieldData(Value::ConstRef(blobs), FieldType::STRING).AsString(), blobs);
    std::string s = "to lbr data";
    UT_EXPECT_EQ(ValueToFieldData(Value::ConstRef(s), FieldType::STRING).AsString(), s);
}
