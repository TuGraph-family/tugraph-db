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
#include "cypher/parser/clause.h"

namespace cypher {
using namespace lgraph_log;
static std::string ToString(const SymbolTable &sym_tab) {
    std::string str = "Alias_ID_Map of Symbol Table:\n";
    for (auto &a : sym_tab.symbols) {
        auto s =
            fma_common::StringFormatter::Format("SYMBOL[{}]: ID {}, TYPE {}, SCOPE {}\n", a.first,
                                                a.second.id, a.second.type, a.second.scope);
        str.append(s);
    }
    str.append("Annotations of Symbol Table:\n");
    for (auto &a : sym_tab.anot_collection.named_paths) {
        str.append("ANOT-PATH[")
            .append(a.first)
            .append("]: ")
            .append(Serialize(*a.second))
            .append("\n");
    }
    return str;
}

void SymbolTable::DumpTable() const {
    if (LoggerManager::GetInstance().GetLevel() <= severity_level::DEBUG) {
        LOG_DEBUG() << ToString(*this);
    }
}

}  // namespace cypher
