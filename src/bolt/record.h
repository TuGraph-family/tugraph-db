/**
 * Copyright 2022 AntGroup CO., Ltd.
 *
 * Copyright (c) "Neo4j"
 * Neo4j Sweden AB [https://neo4j.com]
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

/*
 * written by botu.wzy, inspired by Neo4j Go Driver
 */
#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <any>

namespace bolt {
struct Record {
    // values contains all the values in the record.
    std::vector<std::any> values;
    // keys contains names of the values in the record.
    // Should not be modified. Same instance is used for all records within the same result.
    std::vector<std::string> keys;

    // Get returns the value corresponding to the given key along with a boolean that is true if
    // a value was found and false if there were no key with the given name.
    //
    // If there are a lot of keys in combination with a lot of records to iterate,
    // consider to retrieve values from values slice directly or make a key -> index map
    // before iterating.
    // This implementation does not make or use a key -> index map since the overhead of making
    // the map might not be beneficial for small and few records.
    std::pair<std::any, bool> Get(const std::string& key) {
        for (size_t i = 0; i < keys.size(); i++) {
            if (keys[i] == key) {
                return {values[i], true};
            }
        }
        return {{}, false};
    }

    // AsMap returns a dictionary copy made of the record keys and the corresponding values
    std::unordered_map<std::string, std::any> AsMap() {
        std::unordered_map<std::string, std::any> result;
        for (size_t i = 0; i < keys.size(); i++) {
            result[keys[i]] = values[i];
        }
        return result;
    }
};
}  // namespace bolt
