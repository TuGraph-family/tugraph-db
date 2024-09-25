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

#include <list>
#include <shared_mutex>

#include "parser/clause.h"
#include "parser/data_typedef.h"

namespace cypher {
class ASTCacheObj {
 public:
    std::vector<parser::SglQuery> stmts;
    parser::CmdType cmd;

    ASTCacheObj() {}

    ASTCacheObj(const std::vector<parser::SglQuery> &stmts, parser::CmdType cmd)
    : stmts(stmts), cmd(cmd) {
    }

    std::vector<parser::SglQuery> Stmt() {
        return stmts;
    }

    parser::CmdType CmdType() {
        return cmd;
    }
};

template<typename T>
class PlanCacheEntry {
 public:
    std::string key;
    T value;

    PlanCacheEntry(const std::string &key, const T &value) : key(key), value(value) {}
};

template<typename Value>
class LRUPlanCache {
    typedef PlanCacheEntry<Value> Entry;
    std::list<Entry> _item_list;
    std::unordered_map<std::string, decltype(_item_list.begin())> _item_map;
    size_t _max_size;
    mutable std::shared_mutex _mutex;
    inline void _KickOut() {
        while (_item_map.size() > _max_size) {
            auto last_it = _item_list.end();
            last_it--;
            _item_map.erase(last_it->key);
            _item_list.pop_back();
        }
    }

 public:
    explicit LRUPlanCache(size_t max_size) : _max_size(max_size) {}

    LRUPlanCache() : _max_size(512) {}

    void add_plan(std::string param_query, const Value &val) {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        auto it = _item_map.find(param_query);
        if (it == _item_map.end()) {
            _item_list.emplace_front(std::move(param_query), val);
            _item_map.emplace(_item_list.begin()->key, _item_list.begin());
            _KickOut();
        } else {
            // Overwrite the cached value if the query is already present in the cache.
            // And move the entry to the front of the list.
            it->second->value = val;
            _item_list.splice(_item_list.begin(), _item_list, it->second);
        }
    }

    // Get the cached value for the given parameterized query. Before calling this function,
    // you MUST parameterize the query using the fastQueryParam().
    bool get_plan(const std::string &param_query, Value &val) {
        // parameterized raw query
        std::shared_lock<std::shared_mutex> lock(_mutex);
        auto it = _item_map.find(param_query);
        if (it == _item_map.end()) {
            return false;
        }
        _item_list.splice(_item_list.begin(), _item_list, it->second);
        val = it->second->value;
        return true;
    }

    size_t max_size() const { return _max_size; }

    size_t current_size() const { return _item_map.size(); }
};

typedef LRUPlanCache<ASTCacheObj> ASTCache;
}  // namespace cypher
