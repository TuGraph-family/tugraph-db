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
#include "execution_plan/runtime_context.h"
#include "plan_cache_param.h"

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

template<typename T>
class LRUPlanCache {
    typedef PlanCacheEntry<T> Entry;
    std::list<Entry> _item_list;
    std::unordered_map<std::string, decltype(_item_list.begin())> _item_map;
    size_t _cache_size;
    mutable std::shared_mutex _mutex;
    void _KickOut() {
        std::unique_lock<std::shared_mutex> _guard(_mutex);
        while (_item_map.size() > _cache_size) {
            auto last_it = _item_list.end();
            last_it--;
            _item_map.erase(last_it->key);
            _item_list.pop_back();
        }
    }

 public:
    explicit LRUPlanCache(size_t cache_size) : _cache_size(cache_size) {}

    LRUPlanCache() : _cache_size(512) {}

    void add_plan(RTContext *ctx, const T &val) {
        std::string query = ctx->param_query_;

        std::unique_lock<std::shared_mutex> _guard(_mutex);
        auto it = _item_map.find(query);
        if (it == _item_map.end()) {
            _item_list.push_front(Entry(query, val));
            _item_map.emplace(query, _item_list.begin());
            _KickOut();
        } else {
            it->second->value = val;
            _item_list.splice(_item_list.begin(), _item_list, it->second);
        }
    }

    bool get_plan(RTContext *ctx, const std::string &raw_query, T &val) {
        // parameterized raw query
        std::string query = fastQueryParam(ctx, raw_query);

        std::shared_lock<std::shared_mutex> _guard(_mutex); 
        auto it = _item_map.find(query);
        if (it == _item_map.end()) {
            ctx->param_query_ = std::move(query);
            return false;
        }
        _item_list.splice(_item_list.begin(), _item_list, it->second);
        val = it->second->value;
        return true;
    }
};

typedef LRUPlanCache<ASTCacheObj> ASTCache;
}  // namespace cypher
