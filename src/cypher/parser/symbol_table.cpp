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
// Created by wt on 2019/12/31.
//
#include "cypher/parser/symbol_table.h"
#include "common/logger.h"
#include <spdlog/fmt/fmt.h>

namespace cypher {
static std::string ToString(const SymbolTable &sym_tab) {
    std::string str = "Alias_ID_Map of Symbol Table:\n";
    for (auto &a : sym_tab.symbols) {
        auto s =
            fmt::format("SYMBOL[{}]: ID {}, TYPE {}, SCOPE {}\n", a.first,
                                                a.second.id, (int)a.second.type, (int)a.second.scope);
        str.append(s);
    }
    return str;
}

void SymbolTable::DumpTable() const {
        LOG_DEBUG(ToString(*this));
}

}  // namespace cypher
