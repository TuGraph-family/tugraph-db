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

#include "core/field_extractor_v2.h"
#include "lgraph/lgraph_types.h"
#include "./ut_utils.h"

using namespace lgraph;
using namespace lgraph_api;

class TestFieldExtractorV2 : public TuGraphTest {};

// In this scenario, for every type we only check two cases:
// The specified field is the first one.
// The specified field is the second one, and the first data is an int64.

static DataOffset GetFixStart(const bool label_in_record, const FieldId count) {
    return (label_in_record ? sizeof(LabelId) : 0) + sizeof(FieldId) +
           (count + 7) / 8 + count * sizeof(DataOffset);
}

#define SET_AND_COMPARE_FIELD_VALUE(extr, ft)                                              \
    do {                                                                                   \
        extr.SetFixedSizeValue(v, field_data_helper::GetStoredValue<FieldType::ft>(data)); \
        typename field_data_helper::FieldType2StorageType<FieldType::ft>::type sd;         \
        extr.GetCopy(v, sd);                                                               \
        UT_EXPECT_EQ(sd, field_data_helper::GetStoredValue<FieldType::ft>(data));          \
    } while (0)

static void CheckSetAndGet(FieldType ft, Value& v, bool label_in_record, const FieldData& data,
                           const std::string& str_data) {
    _detail::FieldExtractorV2 fe(FieldSpec("FieldSpec", ft, true, 0));
    fe.SetLabelInRecord(label_in_record);
    fe.SetIsNull(v, true);
    UT_EXPECT_TRUE(fe.GetIsNull(v));
    // 1. id 0;
    fe.SetRecordCount(v, 1);

    // field 0 will start at the offset_area's end
    size_t position = fe.GetFieldOffset(v, 0);
    UT_EXPECT_EQ(position, GetFixStart(label_in_record, 1));

    // set the last offset,
    size_t final_offset = fe.GetOffsetPosition(v, 1);

    if (fe.IsFixedType()) {
        size_t size = fe.TypeSize();
        ::lgraph::_detail::UnalignedSet<DataOffset>(v.Data() + final_offset,
                                                    GetFixStart(label_in_record, 1) + size);
        switch (ft) {
        case FieldType::BOOL:
            SET_AND_COMPARE_FIELD_VALUE(fe, INT8);
            break;
        case FieldType::INT8:
            SET_AND_COMPARE_FIELD_VALUE(fe, INT8);
            break;
        case FieldType::INT16:
            SET_AND_COMPARE_FIELD_VALUE(fe, INT16);
            break;
        case FieldType::INT32:
            SET_AND_COMPARE_FIELD_VALUE(fe, INT32);
            break;
        case FieldType::INT64:
            SET_AND_COMPARE_FIELD_VALUE(fe, INT64);
            break;
        case FieldType::DOUBLE:
            SET_AND_COMPARE_FIELD_VALUE(fe, DOUBLE);
            break;
        case FieldType::FLOAT:
            SET_AND_COMPARE_FIELD_VALUE(fe, FLOAT);
            break;
        default:
            std::cout << "no";
        }
    } else {
        ::lgraph::_detail::UnalignedSet<DataOffset>(v.Data() + final_offset,
                                                    GetFixStart(label_in_record, 1));
        fe.SetVariableOffset(v, fe.GetFieldId(),
                             GetFixStart(label_in_record, 1) + sizeof(DataOffset));
        fe._SetVariableValueRaw(v, Value::ConstRef(str_data));
        Value v_copy;
        fe.GetCopy(v, v_copy);
        UT_EXPECT_EQ(str_data, v_copy.AsString());
    }

    v = Value(1024, 0);

    // 2. id 1;
    _detail::FieldExtractorV2 fe2(FieldSpec("FieldSpace", ft, true, 1));
    fe2.SetLabelInRecord(label_in_record);
    fe2.SetRecordCount(v, 2);
    DataOffset offset = fe2.GetOffsetPosition(v, 1);
    ::lgraph::_detail::UnalignedSet<DataOffset>(v.Data() + offset,
                                                GetFixStart(label_in_record, 2) + sizeof(int64_t));
    position = fe2.GetFieldOffset(v, 1);
    UT_EXPECT_EQ(position, GetFixStart(label_in_record, 2) + sizeof(int64_t));
    final_offset = fe2.GetOffsetPosition(v, 2);
    if (fe2.IsFixedType()) {
        size_t size = fe2.TypeSize();
        ::lgraph::_detail::UnalignedSet<DataOffset>(
            v.Data() + final_offset, GetFixStart(label_in_record, 2) + sizeof(int64_t) + size);
        switch (ft) {
        case FieldType::BOOL:
            SET_AND_COMPARE_FIELD_VALUE(fe2, BOOL);
            break;
        case FieldType::INT8:
            SET_AND_COMPARE_FIELD_VALUE(fe2, INT8);
            break;
        case FieldType::INT16:
            SET_AND_COMPARE_FIELD_VALUE(fe2, INT16);
            break;
        case FieldType::INT32:
            SET_AND_COMPARE_FIELD_VALUE(fe2, INT32);
            break;
        case FieldType::INT64:
            SET_AND_COMPARE_FIELD_VALUE(fe2, INT64);
            break;
        case FieldType::DOUBLE:
            SET_AND_COMPARE_FIELD_VALUE(fe2, DOUBLE);
            break;
        case FieldType::FLOAT:
            SET_AND_COMPARE_FIELD_VALUE(fe2, FLOAT);
            break;
        default:
            std::cout << "no" << std::endl;
        }
    } else {
        ::lgraph::_detail::UnalignedSet<DataOffset>(
            v.Data() + final_offset, GetFixStart(label_in_record, 2) + sizeof(DataOffset));
        fe2.SetVariableOffset(
            v, fe2.GetFieldId(),
            GetFixStart(label_in_record, 2) + sizeof(int64_t) + sizeof(DataOffset));
        fe2._SetVariableValueRaw(v, Value::ConstRef(str_data));
        Value v_copy;
        fe2.GetCopy(v, v_copy);
        UT_EXPECT_EQ(str_data, v_copy.AsString());
    }
}

TEST_F(TestFieldExtractorV2, FieldExtractorV2) {
    UT_LOG() << "Testing FieldExtractorV2";
    Value value(1024, 0);
    CheckSetAndGet(FieldType::BOOL, value, false, FieldData(true), "");
    CheckSetAndGet(FieldType::BOOL, value, true, FieldData(true), "");
    CheckSetAndGet(FieldType::INT8, value, false, FieldData((int8_t)1), "");
    CheckSetAndGet(FieldType::INT8, value, true, FieldData((int8_t)10), "");
    CheckSetAndGet(FieldType::INT16, value, false, FieldData((int16_t)1), "");
    CheckSetAndGet(FieldType::INT16, value, true, FieldData((int16_t)10), "");
    CheckSetAndGet(FieldType::INT32, value, false, FieldData((int32_t)10), "");
    CheckSetAndGet(FieldType::INT32, value, true, FieldData((int32_t)10), "");
    CheckSetAndGet(FieldType::INT64, value, false, FieldData((int64_t)10), "");
    CheckSetAndGet(FieldType::INT64, value, true, FieldData((int64_t)10), "");
    CheckSetAndGet(FieldType::FLOAT, value, false, FieldData((float)10.0), "");
    CheckSetAndGet(FieldType::FLOAT, value, true, FieldData((float)10.0), "");
    CheckSetAndGet(FieldType::DOUBLE, value, false, FieldData((double)10.0), "");
    CheckSetAndGet(FieldType::DOUBLE, value, true, FieldData((double)10.0), "");
    // test string data as variable length data
    // all types read and write will test at schema level
    CheckSetAndGet(FieldType::STRING, value, false, FieldData(), "test");
    CheckSetAndGet(FieldType::STRING, value, true, FieldData(), "test");
}
