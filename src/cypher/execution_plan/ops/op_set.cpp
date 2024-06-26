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

//
// Created by dcy on 19-8-22.
//

#include "cypher/execution_plan/ops/op_set.h"
#include "cypher/arithmetic/arithmetic_expression.h"

namespace cypher {

void OpSet::ExtractProperties(RTContext *ctx, const parser::Expression &p,
                       std::vector<lgraph::FieldData> &values) {
        ArithExprNode ae(p, *record->symbol_table);
        auto val = ae.Evaluate(ctx, *record);
        if (val.IsScalar()) {
            values.emplace_back(val.constant.scalar);
        } else if (val.IsArray()) {
            std::vector<float> vec;
            int dim = val.constant.array->size();
            if (dim >= 4096) {
                throw lgraph::ReminderException("The dimensions of a vector cannot exceed 4096.");
            }
            for (int i = 0; i < dim; ++i) {
                lgraph::FieldData FD = val.constant.array->at(i);
                float num;
                switch (FD.type) {
                case lgraph::FieldType::NUL:
                    throw std::bad_cast();
                case lgraph::FieldType::INT8:
                    num = static_cast<float>(FD.AsInt8());
                    break;
                case lgraph::FieldType::INT16:
                    num = static_cast<float>(FD.AsInt16());
                    break;
                case lgraph::FieldType::INT32:
                    num = static_cast<float>(FD.AsInt32());
                    break;
                case lgraph::FieldType::INT64:
                    num = static_cast<float>(FD.AsInt64());
                    break;
                case lgraph::FieldType::FLOAT:
                    num = static_cast<float>(FD.AsFloat());
                    break;
                case lgraph::FieldType::DOUBLE:
                    num = static_cast<float>(FD.AsDouble());
                    break;
                case lgraph::FieldType::BOOL:
                case lgraph::FieldType::DATE:
                case lgraph::FieldType::DATETIME:
                case lgraph::FieldType::STRING:
                case lgraph::FieldType::BLOB:
                case lgraph::FieldType::POINT:
                case lgraph::FieldType::LINESTRING:
                case lgraph::FieldType::POLYGON:
                case lgraph::FieldType::SPATIAL:
                case lgraph::FieldType::FLOAT_VECTOR:
                    throw std::bad_cast();
                }
                vec.emplace_back(num);
            }
            values.emplace_back(lgraph::FieldData(vec));
        } else {
            CYPHER_TODO();
        }
    }
}  // namespace cypher
