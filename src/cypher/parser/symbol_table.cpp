﻿/**
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
#include "symbol_table.h"
#include "clause.h"

static std::string ToString(const cypher::SymbolTable &sym_tab) {
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

void cypher::SymbolTable::DumpTable() const {
    if (ParserLogger().GetLevel() >= fma_common::LogLevel::LL_DEBUG) {
        FMA_DBG_STREAM(ParserLogger()) << ToString(*this);
    }
}
