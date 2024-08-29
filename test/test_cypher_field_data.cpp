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

#include "core/data_type.h"
#include "gtest/gtest.h"
#include "./ut_utils.h"
#include "cypher/cypher_types.h"

std::vector<::lgraph::FieldData> g_list = {
    lgraph::FieldData(1), lgraph::FieldData(2), lgraph::FieldData(3),
    lgraph::FieldData(4), lgraph::FieldData(5),
};

std::unordered_map<std::string, lgraph::FieldData> g_map = {{"a", lgraph::FieldData(1)},
                                                            {"b", lgraph::FieldData(2)},
                                                            {"c", lgraph::FieldData(3)},
                                                            {"d", lgraph::FieldData(4)},
                                                            {"e", lgraph::FieldData(5)}};

std::vector<std::unordered_map<std::string, lgraph::FieldData>> g_nest_list = {
    {std::make_pair("1", lgraph::FieldData(1))},
    {std::make_pair("2", lgraph::FieldData(2))},
    {std::make_pair("3", lgraph::FieldData(3))},
    {std::make_pair("4", lgraph::FieldData(4))},
    {std::make_pair("5", lgraph::FieldData(5))},
};

std::unordered_map<std::string, std::vector<lgraph::FieldData>> g_nest_map = {
    std::make_pair("a", std::vector<lgraph::FieldData>{lgraph::FieldData(1)}),
    std::make_pair("b", std::vector<lgraph::FieldData>{lgraph::FieldData(2)}),
    std::make_pair("c", std::vector<lgraph::FieldData>{lgraph::FieldData(3)}),
    std::make_pair("d", std::vector<lgraph::FieldData>{lgraph::FieldData(4)}),
    std::make_pair("e", std::vector<lgraph::FieldData>{lgraph::FieldData(5)})
};

class TestCypherFieldData : public TuGraphTest {};

TEST_F(TestCypherFieldData, DataType) {
    cypher::FieldData default_data;
    UT_EXPECT_TRUE(default_data.type == cypher::FieldData::FieldType::SCALAR);

    cypher::FieldData sdata(lgraph::FieldData("test"));
    UT_EXPECT_TRUE(sdata.type == cypher::FieldData::FieldType::SCALAR);

    cypher::FieldData ldata(g_list);
    UT_EXPECT_TRUE(ldata.type == cypher::FieldData::FieldType::ARRAY);

    cypher::FieldData ldata_move(std::vector<::lgraph::FieldData>{
        lgraph::FieldData("A"),
        lgraph::FieldData("B"),
        lgraph::FieldData("C"),
        lgraph::FieldData("D"),
        lgraph::FieldData("E"),
    });
    UT_EXPECT_TRUE(ldata_move.type == cypher::FieldData::FieldType::ARRAY);

    cypher::FieldData mdata(g_map);
    UT_EXPECT_TRUE(mdata.type == cypher::FieldData::FieldType::MAP);

    cypher::FieldData mdata_move(
        std::unordered_map<std::string, lgraph::FieldData>{{"Z", lgraph::FieldData(9)},
                                                           {"Y", lgraph::FieldData(8)},
                                                           {"X", lgraph::FieldData(7)},
                                                           {"W", lgraph::FieldData(6)},
                                                           {"V", lgraph::FieldData(5)}});
    UT_EXPECT_TRUE(mdata_move.type == cypher::FieldData::FieldType::MAP);

    {
        cypher::FieldData sdata_copy(sdata);
        UT_EXPECT_TRUE(sdata_copy.type == cypher::FieldData::FieldType::SCALAR);
        UT_EXPECT_TRUE(sdata_copy == sdata);
        cypher::FieldData sdata_move(std::move(sdata_copy));
        UT_EXPECT_TRUE(sdata_move.type == cypher::FieldData::FieldType::SCALAR);
        UT_EXPECT_TRUE(sdata_move == sdata);
    }

    {
        cypher::FieldData ldata_copy(ldata);
        UT_EXPECT_TRUE(ldata_copy.type == cypher::FieldData::FieldType::ARRAY);
        UT_EXPECT_ANY_THROW(ldata_copy == ldata);
        cypher::FieldData ldata_move(std::move(ldata_copy));
        UT_EXPECT_TRUE(ldata_move.type == cypher::FieldData::FieldType::ARRAY);
        UT_EXPECT_TRUE(ldata_copy.type == cypher::FieldData::FieldType::SCALAR);
        UT_EXPECT_ANY_THROW(ldata_move == ldata);
    }

    {
        cypher::FieldData mdata_copy(mdata);
        UT_EXPECT_TRUE(mdata_copy.type == cypher::FieldData::FieldType::MAP);
        UT_EXPECT_ANY_THROW(mdata_copy == mdata);
        cypher::FieldData mdata_move(std::move(mdata_copy));
        UT_EXPECT_TRUE(mdata_move.type == cypher::FieldData::FieldType::MAP);
        UT_EXPECT_TRUE(mdata_copy.type == cypher::FieldData::FieldType::SCALAR);
        UT_EXPECT_ANY_THROW(mdata_copy == mdata);
    }

    {
        std::vector<cypher::FieldData> nest_list;
        for (auto &item : g_nest_list) {
            nest_list.emplace_back(item);
        }
        cypher::FieldData nest_field_list(nest_list);
        UT_EXPECT_TRUE(nest_field_list.type == cypher::FieldData::FieldType::ARRAY);
        cypher::FieldData nest_field_list_move(std::move(nest_list));
        UT_EXPECT_TRUE(nest_field_list_move.type == cypher::FieldData::FieldType::ARRAY);
        UT_EXPECT_TRUE((*(*nest_field_list_move.array)[0].map)["1"] ==
                       cypher::FieldData(1));
        std::unordered_map<std::string, cypher::FieldData> nest_map;
        for (auto &item : g_nest_map) {
            nest_map.emplace(item);
        }
        cypher::FieldData nest_field_map(nest_map);
        UT_EXPECT_TRUE(nest_field_map.type == cypher::FieldData::FieldType::MAP);
        cypher::FieldData nest_field_map_move(std::move(nest_map));
        UT_EXPECT_TRUE(nest_field_map_move.type == cypher::FieldData::FieldType::MAP);
        UT_EXPECT_TRUE((*(nest_field_map_move.map->at("a").array))[0] ==
                       cypher::FieldData(1));
    }
}
