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
// Created by wt on 21-3-23.
//
#pragma once

#include <list>
#include <mutex>
#include <unordered_map>

namespace cypher {

template <typename KEY_T, typename VAL_T>
class LRUCacheThreadSafe {
    std::list<std::pair<KEY_T, VAL_T>> _item_list;
    std::unordered_map<KEY_T, decltype(_item_list.begin())> _item_map;
    size_t _cache_size;
    mutable std::mutex _mutex;  // allows const methods to modify mutable member

    /* purge extra items */
    void KickOut() {
        while (_item_map.size() > _cache_size) {
            auto last_it = _item_list.end();
            last_it--;
            _item_map.erase(last_it->first);
            _item_list.pop_back();
        }
    }

 public:
    explicit LRUCacheThreadSafe(size_t cache_size) : _cache_size(cache_size) {}

    LRUCacheThreadSafe() : _cache_size(256) {}

    void Put(const KEY_T &key, const VAL_T &val) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _item_map.find(key);
        if (it == _item_map.end()) {
            _item_list.push_front(std::make_pair(key, val));
            _item_map.emplace(key, _item_list.begin());
            KickOut();
        } else {
            it->second->second = val;
            _item_list.splice(_item_list.begin(), _item_list, it->second);
        }
    }

    bool Get(const KEY_T &key, VAL_T &val) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _item_map.find(key);
        if (it == _item_map.end()) return false;
        _item_list.splice(_item_list.begin(), _item_list, it->second);
        val = it->second->second;
        return true;
    }

    const std::list<std::pair<KEY_T, VAL_T>> &List() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _item_list;
    }
};

template <typename KEY_T, typename VAL_T>
class LRUCacheThreadUnsafe {
    std::list<std::pair<KEY_T, VAL_T>> _item_list;
    std::unordered_map<KEY_T, decltype(_item_list.begin())> _item_map;
    size_t _cache_size;

    /* purge extra items */
    void KickOut() {
        while (_item_map.size() > _cache_size) {
            auto last_it = _item_list.end();
            last_it--;
            _item_map.erase(last_it->first);
            _item_list.pop_back();
        }
    }

 public:
    explicit LRUCacheThreadUnsafe(size_t cache_size) : _cache_size(cache_size) {}

    LRUCacheThreadUnsafe() : _cache_size(256) {}

    void Put(const KEY_T &key, const VAL_T &val) {
        auto it = _item_map.find(key);
        if (it == _item_map.end()) {
            _item_list.push_front(std::make_pair(key, val));
            _item_map.emplace(key, _item_list.begin());
            KickOut();
        } else {
            it->second->second = val;
            _item_list.splice(_item_list.begin(), _item_list, it->second);
        }
    }

    bool Get(const KEY_T &key, VAL_T &val) {
        auto it = _item_map.find(key);
        if (it == _item_map.end()) return false;
        _item_list.splice(_item_list.begin(), _item_list, it->second);
        val = it->second->second;
        return true;
    }

    const std::list<std::pair<KEY_T, VAL_T>> &List() const { return _item_list; }
};
}  // namespace cypher
