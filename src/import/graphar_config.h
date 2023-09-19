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

#pragma once

#include <gar/graph.h>
#include <gar/graph_info.h>

#include "tools/json.hpp"

namespace lgraph {
namespace import_v3 {

typedef std::map<std::string, std::string> Dict;

/**
 * @brief  Parse the gar DataType to FieldType in config.
 * 
 * @param   data_type  The GraphAr DataType of the vetex or  property.
 * @param   type_name  The FieldType string which used to make json object.
 */
inline void ParseType(const GraphArchive::DataType& data_type, std::string& type_name) {
    switch (data_type.id()) {
    case GraphArchive::Type::BOOL:
        type_name = "BOOL";
        break;
    case GraphArchive::Type::INT32:
        type_name = "INT32";
        break;
    case GraphArchive::Type::INT64:
        type_name = "INT64";
        break;
    case GraphArchive::Type::FLOAT:
        type_name = "FLOAT";
        break;
    case GraphArchive::Type::DOUBLE:
        type_name = "DOUBLE";
        break;
    case GraphArchive::Type::STRING:
        type_name = "STRING";
        break;
    default:
        break;
    }
}

/**
 * @brief  Check which AdjListType the Gar Edge info contains.
 * 
 * @param   edge_info  The GraphAr edge info.
 * @param   adj_list_type  The gar adj_list_type which used to get edge collection.
 * The adj_list_type value maybe unordered_by_source, unordered_by_dest, ordered_by_source
 * or ordered_by_dest.
 * Reference: https://alibaba.github.io/GraphAr/reference/api-reference-cpp.html#adj-list-type
 */
inline void CheckAdjListType(const GraphArchive::EdgeInfo& edge_info,
                      GraphArchive::AdjListType& adj_list_type) {
    for (std::uint8_t i = 0;
         i <= static_cast<std::uint8_t>(GraphArchive::AdjListType::ordered_by_dest); ++i) {
        GraphArchive::AdjListType type = static_cast<GraphArchive::AdjListType>(i);
        if (edge_info.ContainAdjList(type)) adj_list_type = type;
    }
}

/**
 * Traverse all properties of the vertex, get the primary key, the properties and the property
 * names. Keep the original order in yml config.
 * @param   ver_info  The gar vertex information.
 * @param   primary  The primary key of the vertex.
 * @param   props  All the properties of the vertex. One of it maybe {"name":"id","type":"INT64"}.
 * @param   prop_names  All the property names of the vertex. One of it maybe "id".
 */
inline void WalkVertex(const GraphArchive::VertexInfo& ver_info, std::string& primary,
                std::vector<Dict>& props, std::vector<std::string>& prop_names) {
    auto ver_groups = ver_info.GetPropertyGroups();
    for (auto ver_props : ver_groups) {
        for (auto prop : ver_props.GetProperties()) {
            if (prop.is_primary) primary = prop.name;
            prop_names.emplace_back(prop.name);
            std::string type_name;
            ParseType(prop.type, type_name);
            props.emplace_back(
                std::map<std::string, std::string>{{"name", prop.name}, {"type", type_name}});
        }
    }
}

/**
 * Traverse all properties of the edge, get the properties and the property names.
 * Keep the original order in yml config. Similar to WalkVertex, but don't get primary.
 * @param   edge_info  The gar edge information.
 * @param   props  All the properties of the vertex. One of it maybe {"name":"id","type":"INT64"}.
 * @param   prop_names  All the property names of the vertex. One of it maybe "id".
 */
inline void WalkEdge(const GraphArchive::EdgeInfo& edge_info, std::vector<Dict>& props,
              std::vector<std::string>& prop_names) {
    GraphArchive::AdjListType adj_list_type = GraphArchive::AdjListType::ordered_by_dest;
    CheckAdjListType(edge_info, adj_list_type);
    auto edge_groups = edge_info.GetPropertyGroups(adj_list_type).value();
    for (auto edge_props : edge_groups) {
        for (auto prop : edge_props.GetProperties()) {
            prop_names.emplace_back(prop.name);
            std::string type_name;
            ParseType(prop.type, type_name);
            props.emplace_back(
                std::map<std::string, std::string>{{"name", prop.name}, {"type", type_name}});
        }
    }
}

/**
 * @brief   Read the gar yml file to construct the import config in json form.
 * 
 * @param   gar_conf  The json object of the import config used in import_v3. 
 * @param   path  The location of gar yml file.
 */
inline void ParserGraphArConf(nlohmann::json& gar_conf, const std::string& path) {
    auto graph_info = GraphArchive::GraphInfo::Load(path).value();
    gar_conf["schema"] = {};
    gar_conf["files"] = {};
    auto vertex_infos = graph_info.GetVertexInfos();
    for (const auto& [key, value] : vertex_infos) {
        nlohmann::json schema_node;
        schema_node["label"] = value.GetLabel();
        schema_node["type"] = "VERTEX";
        std::string primary;
        std::vector<Dict> properties;
        std::vector<std::string> prop_names;
        WalkVertex(value, primary, properties, prop_names);
        schema_node["primary"] = primary;
        schema_node["properties"] = properties;
        gar_conf["schema"].push_back(schema_node);

        nlohmann::json file_node;
        file_node["path"] = path;
        file_node["format"] = "GraphAr";
        file_node["label"] = value.GetLabel();
        file_node["columns"] = prop_names;
        gar_conf["files"].push_back(file_node);
    }

    auto edge_infos = graph_info.GetEdgeInfos();
    for (const auto& [key, value] : edge_infos) {
        nlohmann::json schema_node;
        schema_node["label"] = value.GetEdgeLabel();
        schema_node["type"] = "EDGE";
        std::vector<Dict> properties;
        std::vector<std::string> prop_names = {"SRC_ID", "DST_ID"};
        WalkEdge(value, properties, prop_names);
        schema_node["properties"] = properties;
        gar_conf["schema"].push_back(schema_node);

        nlohmann::json file_node;
        file_node["path"] = path;
        file_node["format"] = "GraphAr";
        file_node["label"] = value.GetEdgeLabel();
        file_node["SRC_ID"] = value.GetSrcLabel();
        file_node["DST_ID"] = value.GetDstLabel();
        file_node["columns"] = prop_names;
        gar_conf["files"].push_back(file_node);
    }
}

}  // namespace import_v3
}  // namespace lgraph
