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

#include "core/data_type.h"

namespace lgraph {
namespace import_v2 {

// The base class for ColumnParser, JsonLinesParser and GraphArParser
class BlockParser {
 public:
    virtual bool ReadBlock(std::vector<std::vector<FieldData>>& buf) = 0;
    virtual ~BlockParser() {}
};

}  // namespace import_v2
}  // namespace lgraph
