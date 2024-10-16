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

#include <graphar/high-level/graph_reader.h>

#include "import/graphar_parser.h"
#include "import/import_exception.h"

namespace lgraph {
namespace import_v2 {

// The map contains PrimaryMaps of all vertices as endpoints of edges
std::unordered_map<std::string, PrimaryMap> GraphArParser::primary_maps = {};

GraphArParser::GraphArParser(const CsvDesc& cd) : cd_(cd) {}

GraphArParser::~GraphArParser() {}

/**
 * @brief  Traverse the vertex properties to get the primary key.
 * @param ver_info  The gar vertex information.
 * @return The primary key of the vertex.
 */
graphar::Property GraphArParser::GetPrimaryKey(const graphar::VertexInfo& ver_info) {
    const auto& ver_groups = ver_info.GetPropertyGroups();
    for (const auto& ver_props : ver_groups)
        for (const auto& prop : ver_props->GetProperties())
            if (prop.is_primary) return prop;
    THROW_CODE(InputError, "The primary key of [" + ver_info.GetType() + "] is not found!");
}

/**
 * @brief   Parse the gar data of a vertex or edge property to FieldData.
 * @param   T  The GraphAr vertex data or edge data.
 * @param   prop  The property name.
 * @param   data_type  The GraphAr DataType of the vetex property.
 * @return  The converted FieldData.
 */
template <typename T>
FieldData GraphArParser::ParseData(T& data, const std::string& prop,
                                   const std::shared_ptr<graphar::DataType>& type) {
    switch (type->id()) {
    case graphar::Type::BOOL:
        return FieldData(data.template property<bool>(prop).value());
    case graphar::Type::INT32:
        return FieldData(data.template property<int32_t>(prop).value());
    case graphar::Type::INT64:
        return FieldData(data.template property<int64_t>(prop).value());
    case graphar::Type::FLOAT:
        return FieldData(data.template property<float>(prop).value());
    case graphar::Type::DOUBLE:
        return FieldData(data.template property<double>(prop).value());
    case graphar::Type::STRING:
        return FieldData::String(data.template property<std::string>(prop).value());
    case graphar::Type::DATE:
        return FieldData::Date(
            Date(data.template property<graphar::Date>(prop).value().value())
        );
    default:
        THROW_CODE(InputError, "Unsupported data type error!");
        break;
    }
    return FieldData();
}

/**
 * Map the GraphAr vertex ID and the vertex primary property FieldData, to get the
 * primary data.
 * @param[out]  primary_map  To save the map.
 * @param[in]   graph_info  The GraphAr graph information, to get the vertex information.
 * @param[in]   ver_label  The vertex label to get the vertex information.
 */
void GraphArParser::MapIdPrimary(PrimaryMap& primary_map,
                                 const std::shared_ptr<graphar::GraphInfo>& graph_info,
                                 const std::string& ver_label) {
    if (primary_maps.count(ver_label)) {
        primary_map = primary_maps[ver_label];
        return;
    }
    auto ver_info = graph_info->GetVertexInfo(ver_label);
    graphar::Property ver_prop = GetPrimaryKey(*ver_info);
    if (!ver_prop.is_primary) {
        THROW_CODE(InputError, "the primary key of [" + ver_label + "] is not found!");
    }
    auto vertices = graphar::VerticesCollection::Make(graph_info, ver_label).value();
    for (auto it = vertices->begin(); it != vertices->end(); ++it) {
        auto vertex = *it;
        primary_map[vertex.id()] =
            ParseData<graphar::Vertex>(vertex, ver_prop.name, ver_prop.type);
    }
    primary_maps[ver_label] = primary_map;
}

/**
 * @brief  Get the adjacency list type of the edge
 *
 * @param  edge_info An object to describe the edge information
 * @return  One of graphar::AdjListType
 */
graphar::AdjListType GraphArParser::GetOneAdjListType(
    const graphar::EdgeInfo& edge_info) {
    if (edge_info.HasAdjacentListType(graphar::AdjListType::unordered_by_source))
        return graphar::AdjListType::unordered_by_source;
    if (edge_info.HasAdjacentListType(graphar::AdjListType::unordered_by_dest))
        return graphar::AdjListType::unordered_by_dest;
    if (edge_info.HasAdjacentListType(graphar::AdjListType::ordered_by_source))
        return graphar::AdjListType::ordered_by_source;
    if (edge_info.HasAdjacentListType(graphar::AdjListType::ordered_by_dest))
        return graphar::AdjListType::ordered_by_dest;
    THROW_CODE(InputError, "The GraphAr edge" + edge_info.GetEdgeType() + " has not AdjListType!");
}

/**
 * Read the gar data via the CsvDesc, and parse the gar data to the block of FieldData.
 * @param[out]   buf  The block import_v3 needed.
 * @return  Whether the read process has been finished.
 */
bool GraphArParser::ReadBlock(std::vector<std::vector<FieldData>>& buf) {
    if (label_read) return false;
    label_read = true;
    auto graph_info = graphar::GraphInfo::Load(cd_.path).value();
    if (cd_.is_vertex_file) {
        auto vertices = graphar::VerticesCollection::Make(graph_info, cd_.label).value();
        auto ver_info = graph_info->GetVertexInfo(cd_.label);
        for (auto it = vertices->begin(); it != vertices->end(); ++it) {
            auto vertex = *it;
            std::vector<FieldData> temp_vf;
            for (std::string& prop : cd_.columns) {
                auto data_type = ver_info->GetPropertyType(prop).value();
                if(!vertex.IsValid(prop)) {
                    temp_vf.emplace_back();
                    continue;
                }
                FieldData fd = ParseData<graphar::Vertex>(vertex, prop, data_type);
                temp_vf.emplace_back(std::move(fd));
            }
            buf.emplace_back(std::move(temp_vf));
        }
        return true;
    } else {
        auto edge_info = graph_info->GetEdgeInfo(cd_.edge_src.label, cd_.label, cd_.edge_dst.label);
        auto edges =
            graphar::EdgesCollection::Make(graph_info, cd_.edge_src.label, cd_.label,
                                                cd_.edge_dst.label, GetOneAdjListType(*edge_info))
                .value();

        PrimaryMap src_id_primary;
        PrimaryMap dst_id_primary;
        MapIdPrimary(src_id_primary, graph_info, cd_.edge_src.label);
        MapIdPrimary(dst_id_primary, graph_info, cd_.edge_dst.label);

        size_t edge_num = edges->size();
        buf.reserve(edge_num);
        for (auto it = edges->begin(); it != edges->end(); ++it) {
            auto edge = *it;
            std::vector<FieldData> temp_vf;
            for (auto& prop : cd_.columns) {
                if (prop == "SRC_ID") {
                    FieldData src_data = src_id_primary[edge.source()];
                    temp_vf.emplace_back(std::move(src_data));
                } else if (prop == "DST_ID") {
                    FieldData dst_data = dst_id_primary[edge.destination()];
                    temp_vf.emplace_back(std::move(dst_data));
                } else {
                    auto data_type = edge_info->GetPropertyType(prop).value();
                    if(!edge.IsValid(prop)) {
                        temp_vf.emplace_back();
                        continue;
                    }
                    FieldData edge_data = ParseData<graphar::Edge>(edge, prop, data_type);
                    temp_vf.emplace_back(std::move(edge_data));
                }
            }
            buf.emplace_back(std::move(temp_vf));
        }
        return true;
    }
}

}  // namespace import_v2
}  // namespace lgraph