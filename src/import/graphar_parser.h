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

#include "core/data_type.h"
#include "core/field_data_helper.h"
#include "import/block_parser.h"
#include "import/import_config_parser.h"

namespace lgraph {
namespace import_v2 {

// Parse gar file into a block of FieldData
class GraphArParser : public BlockParser {
 protected:
    CsvDesc cd_;              // Schema definition and config
    bool label_read = false;  // Means the block has been read

    /**
     * @brief  Traverse the vertex properties to get the primary key.
     */
    GraphArchive::Property GetPrimaryKey(const GraphArchive::VertexInfo& ver_info) {
        auto ver_groups = ver_info.GetPropertyGroups();
        for (auto ver_props : ver_groups)
            for (auto prop : ver_props.GetProperties())
                if (prop.is_primary) return prop;
        return GraphArchive::Property();
    }

    /**
     * @brief  Parse the gar data of a vertex property to FieldData.
     * @param   vertex  The GraphAr vertex.
     * @param   prop  The property name.
     * @param   data_type  The GraphAr DataType of the vetex property.
     * @return  The converted FieldData.
     */
    FieldData ParseVertexData(GraphArchive::Vertex& vertex, const std::string& prop,
                              const GraphArchive::DataType& data_type) {
        switch (data_type.id()) {
        case GraphArchive::Type::STRING:
            return FieldData::String(ToStdString(vertex.property<std::string>(prop).value()));
        case GraphArchive::Type::BOOL:
            return FieldData(vertex.property<bool>(prop).value());
        case GraphArchive::Type::INT32:
            return FieldData(vertex.property<int32_t>(prop).value());
        case GraphArchive::Type::INT64:
            return FieldData(vertex.property<int64_t>(prop).value());
        case GraphArchive::Type::FLOAT:
            return FieldData(vertex.property<float>(prop).value());
        case GraphArchive::Type::DOUBLE:
            return FieldData(vertex.property<double>(prop).value());
        case GraphArchive::Type::USER_DEFINED:
            // todo
            break;
        default:
            break;
        }
        return FieldData();
    }

    /**
     * @brief  Parse the gar data of an edge property to FieldData.
     * @param   vertex  The GraphAr edge.
     * @param   prop  The property name.
     * @param   data_type  The GraphAr DataType of the edge property.
     * @return  The converted FieldData.
     */
    FieldData ParseEdgeData(GraphArchive::Edge& edge, const std::string& prop,
                            const GraphArchive::DataType& data_type) {
        switch (data_type.id()) {
        case GraphArchive::Type::STRING:
            return FieldData::String(ToStdString(edge.property<std::string>(prop).value()));
        case GraphArchive::Type::BOOL:
            return FieldData(edge.property<bool>(prop).value());
        case GraphArchive::Type::INT32:
            return FieldData(edge.property<int32_t>(prop).value());
        case GraphArchive::Type::INT64:
            return FieldData(edge.property<int64_t>(prop).value());
        case GraphArchive::Type::FLOAT:
            return FieldData(edge.property<float>(prop).value());
        case GraphArchive::Type::DOUBLE:
            return FieldData(edge.property<double>(prop).value());
        case GraphArchive::Type::USER_DEFINED:
            // todo
            break;
        default:
            break;
        }
        return FieldData();
    }

    /**
     * Map the GraphAr vertex ID and the vertex primary property FieldData, to get the
     * primary data faster.
     * @param   primary_map  To save the map.
     * @param   graph_info  The GraphAr graph information, to get the vertex information.
     * @param   ver_label  The vertex label to get the vertex information.
     */
    void MapIdPrimary(std::unordered_map<GraphArchive::IdType, FieldData>& primary_map,
                      const GraphArchive::GraphInfo& graph_info, const std::string& ver_label) {
        auto ver_info = graph_info.GetVertexInfo(ver_label).value();
        GraphArchive::Property ver_prop = GetPrimaryKey(ver_info);
        auto vertices = GraphArchive::ConstructVerticesCollection(graph_info, ver_label).value();
        for (auto vertex : vertices) {
            primary_map[vertex.id()] = ParseVertexData(vertex, ver_prop.name, ver_prop.type);
        }
    }

 public:
    explicit GraphArParser(const CsvDesc& cd) : cd_(cd) {}

    /**
     * Read the gar data via the CsvDesc, and parse the gar data to the block of FieldData.
     * @param   buf  The block import_v3 needed.
     * @return  Whether the read process has been finished.
     */
    bool ReadBlock(std::vector<std::vector<FieldData>>& buf) {
        if (label_read) return false;
        label_read = true;
        auto graph_info = GraphArchive::GraphInfo::Load(cd_.path).value();
        if (cd_.is_vertex_file) {
            auto vertices =
                GraphArchive::ConstructVerticesCollection(graph_info, cd_.label).value();
            auto ver_info = graph_info.GetVertexInfo(cd_.label).value();
            std::vector<FieldData> temp_vf;
            for (auto it = vertices.begin(); it != vertices.end(); ++it) {
                auto vertex = *it;
                temp_vf.clear();
                for (auto prop : cd_.columns) {
                    auto data_type = ver_info.GetPropertyType(prop).value();
                    FieldData fd = ParseVertexData(vertex, prop, data_type);
                    temp_vf.emplace_back(fd);
                }
                buf.emplace_back(temp_vf);
            }
            return true;
        } else {
            auto edges_collection = GraphArchive::ConstructEdgesCollection(
                              graph_info, cd_.edge_src.label, cd_.label, cd_.edge_dst.label,
                              GraphArchive::AdjListType::ordered_by_source)
                              .value();
            auto edges = std::get<
                GraphArchive::EdgesCollection<GraphArchive::AdjListType::ordered_by_source>>(
                edges_collection);
            auto edge_info =
                graph_info.GetEdgeInfo(cd_.edge_src.label, cd_.label, cd_.edge_dst.label).value();

            std::unordered_map<GraphArchive::IdType, FieldData> src_id_primary;
            std::unordered_map<GraphArchive::IdType, FieldData> dst_id_primary;
            MapIdPrimary(src_id_primary, graph_info, cd_.edge_src.label);
            MapIdPrimary(dst_id_primary, graph_info, cd_.edge_dst.label);

            size_t edge_num = edges.size();
            buf.reserve(edge_num);
            std::vector<FieldData> temp_vf;
            for (auto it = edges.begin(); it != edges.end(); ++it) {
                auto edge = *it;
                temp_vf.clear();
                for (auto prop : cd_.columns) {
                    if (prop == "SRC_ID") {
                        auto src_data = src_id_primary[edge.source()];
                        temp_vf.emplace_back(src_data);
                    } else if (prop == "DST_ID") {
                        auto dst_data = dst_id_primary[edge.destination()];
                        temp_vf.emplace_back(dst_data);
                    } else {
                        auto data_type = edge_info.GetPropertyType(prop).value();
                        FieldData edge_data = ParseEdgeData(edge, prop, data_type);
                        temp_vf.emplace_back(edge_data);
                    }
                }
                buf.emplace_back(temp_vf);
            }
            return true;
        }
        return false;
    }

    ~GraphArParser() {}
};

}  // namespace import_v2
}  // namespace lgraph
