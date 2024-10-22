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

// #include "gtest/gtest.h"

// #include "core/field_extractor_v2.h"
// #include "lgraph/lgraph_types.h"
// #include "lgraph/data_type.h"
// #include "./ut_utils.h"

// using namespace lgraph;
// using namespace lgraph_api;

// class TestFieldExtractorV2 : public TuGraphTest {};

// // In this scenario, for every types we only check two cases:
// // The specified field is the first one.
// // The specified field is the second one, and the first data is an int64.

// static DataOffset GetFixStart(const bool label_in_record, const FieldId count) {
//     return sizeof(VersionId) + (label_in_record ? sizeof(LabelId) : 0) + sizeof(FieldId) +
//            (count + 7) / 8 + count * sizeof(DataOffset);
// }

// #define COMPARE_FIELD_VALUE(ft)                                                            \
//     do {                                                                                   \
//         typedef typename field_data_helper::FieldType2StorageType<FieldType::ft>::type ST; \
//         typedef typename field_data_helper::FieldType2CType<FieldType::ft>::type CT;       \
//         ST d = 0;                                                                          \
//         GetCopy(v, d);                                                                     \
//         UT_EXPECT_EQ(data, d);                                                             \
//     } while (0);

// template <typename T>
// static void CheckSetAndGet(FieldType ft, Value& v, bool label_in_record, T data,
//                            const std::string& str_data) {
//     _detail::FieldExtractorV2 fe(FieldSpecV2("FieldSpec", ft, true, 0));
//     fe.SetLabelInRecord(label_in_record);
//     fe.SetIsNull(v, true);
//     UT_EXPECT_TRUE(fe.GetIsNull(v));
//     // 1. id 0;
//     fe.SetRecordCount(v, 1);

//     // field 0 will start at the offset_area's end
//     size_t position = fe.GetFieldOffset(v, 0);
//     UT_EXPECT_EQ(position, GetFixStart(label_in_record, 1));

//     // set the last offset,
//     size_t final_offset = fe.GetOffsetPosition(v, 1);

//     if (fe.IsFixedType()) {
//         size_t size = fe.TypeSize();
//         ::lgraph::_detail::UnalignedSet<DataOffset>(v.Data() + final_offset,
//                                                     GetFixStart(label_in_record, 1) + size);
//         fe._SetFixedSizeValueRaw(v, data);
//         COMPARE_FIELD_VALUE(::lgraph_api::to_string(ft))
//     } else {
//         ::lgraph::_detail::UnalignedSet<DataOffset>(v.Data() + final_offset,
//                                                     GetFixStart(label_in_record, 1));
//         fe.SetVariableOffset(v, fe.GetFieldId(),
//                              GetFixStart(label_in_record, 1) + sizeof(DataOffset));
//         fe._SetVariableValueRaw(v, Value::ConstRef(str_data));
//         Value v_copy;
//         fe.GetCopy(v, v_copy);
//         UT_EXPECT_EQ(str_data, v_copy.ToString())
//     }

//     // 2. id 1;
//     fe(FieldSpecV2("FieldSpace", ft, true, 1));
//     fe.SetRecordCount(v, 2);
//     position = fe.GetFieldOffset(v, 1);
//     UT_EXPECT_EQ(position, GetFixStart(label_in_record, 2) + sizeof(int64_t));
//     final_offset = fe.GetOffsetPosition(v, 2);
//     if (fe.IsFixedType()) {
//         size_t size = fe.TypeSize();
//         ::lgraph::_detail::UnalignedSet<DataOffset>(
//             v.Data() + final_offset, GetFixStart(label_in_record, 2) + size(int64_t) + size);
//         fe._SetFixedSizeValueRaw(v, data);
//         COMPARE_FIELD_VALUE(::lgraph_api::to_string(ft))
//     } else {
//         ::lgraph::_detail::UnalignedSet<DataOffset>(
//             v.Data() + final_offset, GetFixStart(label_in_record, 2) + sizeof(DataOffset));
//         fe.SetVariableOffset(
//             v, fe.GetFieldId(),
//             GetFixStart(label_in_record, 2) + sizeof(int64_t) + sizeof(DataOffset));
//         fe._SetVariableValueRaw(v, Value::ConstRef(str_data));
//         Value v_copy;
//         fe.GetCopy(v, v_copy);
//         UT_EXPECT_EQ(str_data, v_copy.ToString());
//     }
// }

// TEST_F(TestFieldExtractorV2, FieldExtractorV2) {
//     UT_LOG() << "Testing FieldExtractorV2";
//     Value value(1024, 0);
//     CheckSetAndGet(FieldType::INT8, value, false, (int_8)1, "");
//     CheckSetAndGet(FieldType::INT8, value, true, (int_8)10, "");
//     CheckSetAndGet(FieldType::INT16, value, false, (int_16)1, "");
//     CheckSetAndGet(FieldType::INT16, value, true, (int_16)10, "");
//     CheckSetAndGet(FieldType::INT32, value, false, (int_32)10, "");
//     CheckSetAndGet(FieldType::INT32, value, true, (int_32)10, "");
//     CheckSetAndGet(FieldType::INT64, value, false, (int_64)10, "");
//     CheckSetAndGet(FieldType::INT64, value, true, (int_64)10, "");
//     CheckSetAndGet(FieldType::FLOAT, value, false, (float)10.0, "");
//     CheckSetAndGet(FieldType::FLOAT, value, true, (float)10.0, "");
//     CheckSetAndGet(FieldType::DOUBLE, value, false, (double)10.0, "");
//     CheckSetAndGet(FieldType::DOUBLE, value, true, (double)10.0, "");
//     // test string data as variable length data
//     // all types read and write will test at schema level
//     CheckSetAndGet(FieldType::STRING, value, false, "", "test");
//     CheckSetAndGet(FieldType::STRING, value, true, "", "test");
// }
