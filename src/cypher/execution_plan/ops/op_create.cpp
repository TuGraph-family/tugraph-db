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
// Created by wt on 18-7-27.
//

#include "cypher/execution_plan/ops/op_create.h"
#include "cypher/arithmetic/arithmetic_expression.h"

namespace cypher {
void OpCreate::ExtractProperties(RTContext *ctx, const parser::TUP_PROPERTIES &properties,
                       VEC_STR &fields, std::vector<lgraph::FieldData> &values) {
        using namespace parser;
        Expression map_literal = std::get<0>(properties);
        CYPHER_THROW_ASSERT(map_literal.type == Expression::NA ||
                            map_literal.type == Expression::MAP);
        if (map_literal.type != Expression::MAP) return;
        for (auto &prop : map_literal.Map()) {
            fields.emplace_back(prop.first);
            Expression p;
            if (prop.second.type == Expression::LIST) {
                p = prop.second;
            } else if (prop.second.type == Expression::MAP) {
                CYPHER_TODO();
            } else {
                p = prop.second;
            }
            ArithExprNode ae(p, *record->symbol_table);
            auto value = ae.Evaluate(ctx, *record);
            CYPHER_THROW_ASSERT(!value.IsNull());
            if (value.IsArray()) {
                std::vector<float> vec;
                int dim = value.constant.array->size();
                if (dim >= 4096) {
                    throw lgraph::ReminderException(
                        "The dimensions of a vector cannot exceed 4096.");
                }
                for (int i = 0; i < dim; ++i) {
                    lgraph::FieldData FD = value.constant.array->at(i);
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
                values.emplace_back(value.constant.scalar);
            }
        }
    }
}  // namespace cypher
