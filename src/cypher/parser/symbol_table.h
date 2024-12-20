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
#include "geax-front-end/ast/expr/Ref.h"
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

    inline static std::string to_string(const SymbolNode::Type& t) {
        static std::unordered_map<SymbolNode::Type, std::string> type_map = {
            {SymbolNode::Type::CONSTANT, "CONSTANT"},
            {SymbolNode::Type::NODE, "NODE"},
            {SymbolNode::Type::RELATIONSHIP, "RELATIONSHIP"},
            {SymbolNode::Type::PARAMETER, "PARAMETER"},
            {SymbolNode::Type::NAMED_PATH, "NAMED_PATH"},
        };
        auto it = type_map.find(t);
        if (it == type_map.end()) {
            throw std::runtime_error("Unknown SymbolNode::Type");
        }
        return it->second;
    }

    inline static std::string to_string(const SymbolNode::Scope& s) {
        static std::unordered_map<SymbolNode::Scope, std::string> scope_map = {
            {SymbolNode::Scope::LOCAL, "LOCAL"},
            {SymbolNode::Scope::ARGUMENT, "ARGUMENT"},
            {SymbolNode::Scope::DERIVED_ARGUMENT, "DERIVED_ARGUMENT"},
        };
        auto it = scope_map.find(s);
        if (it == scope_map.end()) {
            throw std::runtime_error("Unknown SymbolNode::Scope");
        }
        return it->second;
    }
};

struct AnnotationCollection {
    /* Note that symbol table will be copyed into execution plan. */
    std::unordered_map<std::string, std::shared_ptr<parser::TUP_PATTERN_ELEMENT> > named_paths;
    std::unordered_map<std::string, std::vector<
        std::shared_ptr<geax::frontend::Ref>>> path_elements;
};

struct SymbolTable {
    std::unordered_map<std::string, SymbolNode> symbols;
    AnnotationCollection anot_collection;

    void DumpTable() const;
};

}  // namespace cypher
