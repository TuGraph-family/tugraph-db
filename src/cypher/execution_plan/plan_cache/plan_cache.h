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

// #include <boost/any.hpp>
namespace cypher {
class ASTCacheObj {
    public:
    struct AST {
        std::vector<parser::SglQuery> stmts;
        parser::CmdType cmd;
    } ast_;
    boost::any value_;

    ASTCacheObj(const std::vector<parser::SglQuery> &stmts, parser::CmdType cmd) {
        value_ = AST{stmts, cmd};
    }

    ASTCacheObj(boost::any val) : value_(val) {
        ast_ = boost::any_cast<AST>(value_);
    }

    boost::any to_any() {
        return value_;
    }

    std::vector<parser::SglQuery> Stmt() {
        return ast_.stmts;
    }

    parser::CmdType CmdType() {
        return ast_.cmd;
    }
};

class PlanCacheEntry {
    public:
    std::string key;
    boost::any value;

    PlanCacheEntry(const std::string &key, const boost::any &value) : key(key), value(value) {}
};

class LRUPlanCache {
    std::list<PlanCacheEntry> _item_list;
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

    void add_plan(RTContext *ctx, const boost::any &val) {
        std::string query = ctx->param_query_;

        std::unique_lock<std::shared_mutex> _guard(_mutex);
        auto it = _item_map.find(query);
        if (it == _item_map.end()) {
            _item_list.push_front(PlanCacheEntry(query, val));
            _item_map.emplace(query, _item_list.begin());
            _KickOut();
        } else {
            it->second->value = val;
            _item_list.splice(_item_list.begin(), _item_list, it->second);
        }
    }

    bool get_plan(RTContext *ctx, const std::string &raw_query, boost::any &val) {
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
}  // namespace cypher
