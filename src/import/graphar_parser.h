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

// The map to get primary data of a vertex
typedef std::unordered_map<GraphArchive::IdType, FieldData> PrimaryMap;

// Parse gar file into a block of FieldData
class GraphArParser : public BlockParser {
 protected:
    CsvDesc cd_;              // Schema definition and config
    bool label_read = false;  // Means the block has been read

    static std::unordered_map<std::string, PrimaryMap> primary_maps;

    GraphArchive::Property GetPrimaryKey(const GraphArchive::VertexInfo&);

    template <typename T>
    FieldData ParseData(T& data, const std::string& prop, const GraphArchive::DataType& data_type);

    void MapIdPrimary(PrimaryMap& primary_map, const GraphArchive::GraphInfo& graph_info,
                      const std::string& ver_label);

 public:
    explicit GraphArParser(const CsvDesc& cd);

    bool ReadBlock(std::vector<std::vector<FieldData>>& buf);

    ~GraphArParser();
};

}  // namespace import_v2
}  // namespace lgraph
