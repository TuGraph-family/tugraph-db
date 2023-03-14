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
// Created by wt on 6/12/18.
//
#pragma once

#include "lgraph/lgraph.h"
#include "record.h"
#include "resultset_statistics.h"
#include "parser/symbol_table.h"

namespace cypher {

/* A column within the result-info
 * a column can be referred to either by its name or alias */
struct Column {
    std::string name;
    std::string alias; /* AS ${alias} */
    bool aggregated;   /* 1 if column is aggregated, 0 otherwise. */
    lgraph::ElementType type;

    Column(const std::string &n, const std::string &a, bool ag, lgraph::ElementType t)
        : name(n), alias(a), aggregated(ag), type(t) {}

    explicit Column(const std::string &n)
        : name(n), alias(""), aggregated(false), type(lgraph::ElementType::ANY) {}

    std::string ToString() const {
        std::string str;
        if (!alias.empty()) {
            str = alias;
        } else {
            str = name;
        }
        return str;
    }
};

struct ResultSetHeader {
    std::vector<Column> colums; /* Vector of Columns, desired elements specified in return clause */
    std::vector<int> orderBys;  /* Array of indices into elements */

    std::string ToString() const {
        std::string str;
        if (colums.empty()) return str;
        auto it = colums.begin();
        str.append(it->ToString());
        it++;
        for (; it != colums.end(); it++) {
            str.append(",").append(it->ToString());
        }
        return str;
    }
};

struct ResultInfo {
    // std::vector<Record> records;
    ResultSetHeader header;  /* Describes how records should look like. */
    bool aggregated = false; /* Rather or not this is an aggregated result set. */
    bool ordered = false;    /* Rather or not this result set is ordered. */
    std::vector<std::pair<int, bool>> sort_items;
    int64_t skip = -1;
    int64_t limit = -1;    /* Max number of records in result-set. */
    bool distinct = false; /* Rather or not each record is unique. */
    ResultSetStatistics statistics;
};
}  // namespace cypher
