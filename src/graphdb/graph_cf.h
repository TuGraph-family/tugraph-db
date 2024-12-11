/**
* Copyright 2024 AntGroup CO., Ltd.
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
// Created by botu.wzy
//

#pragma once
#include<unordered_map>
#include <rocksdb/utilities/transaction_db.h>
namespace graphdb {
struct GraphCF {
    rocksdb::ColumnFamilyHandle* graph_topology = nullptr;
    rocksdb::ColumnFamilyHandle* vertex_property = nullptr;
    rocksdb::ColumnFamilyHandle* edge_property = nullptr;
    rocksdb::ColumnFamilyHandle* vertex_label_vid = nullptr;
    rocksdb::ColumnFamilyHandle* edge_type_eid = nullptr;
    rocksdb::ColumnFamilyHandle* name_id = nullptr;
    rocksdb::ColumnFamilyHandle* meta_info = nullptr;
    rocksdb::ColumnFamilyHandle* index = nullptr;
    rocksdb::ColumnFamilyHandle* wal = nullptr;
    rocksdb::ColumnFamilyHandle* vector_index_manifest = nullptr;
};
}