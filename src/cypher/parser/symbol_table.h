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
// Created by wt on 19-2-25.
//
#pragma once
#include "cypher/parser/data_typedef.h"

namespace cypher {

struct SymbolNode {
    size_t id;  // index in record
    enum Type {
        CONSTANT = 0,
        NODE = 1,
        RELATIONSHIP = 2,
        PARAMETER = 3,
        NAMED_PATH = 4,
    } type;
    enum Scope {
        LOCAL,             // MATCH (n) RETURN n,1 AS num
        ARGUMENT,          // WITH a
        DERIVED_ARGUMENT,  // derived from argument, WITH a UNWIND a AS x
    } scope;

    SymbolNode(size_t i, Type t, Scope s) : id(i), type(t), scope(s) {}

    inline static std::string to_string(SymbolNode::Type& v) {
        switch (v) {
        case CONSTANT:
            return "CONSTANT";
        case NODE:
            return "NODE";
        case RELATIONSHIP:
            return "RELATIONSHIP";
        case PARAMETER:
            return "PARAMETER";
        case NAMED_PATH:
            return "NAMED_PATH";
        default:
            throw std::runtime_error("Unknown Field Type");
        }
    }
};

struct AnnotationCollection {
    /* Note that symbol table will be copyed into execution plan. */
    std::unordered_map<std::string, std::shared_ptr<parser::TUP_PATTERN_ELEMENT> > named_paths;
};

struct SymbolTable {
    std::unordered_map<std::string, SymbolNode> symbols;
    AnnotationCollection anot_collection;

    void DumpTable() const;
};

}  // namespace cypher
