/**
 * Copyright 2024 AntGroup CO., Ltd.
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

#include "uuid_generator.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace graphdb {

std::string UUIDGenerator::Next() {
    std::unique_lock lock(mutex_);
    oss_.clear();
    oss_ << generator_();
    return oss_.str();
}

std::string UUIDGenerator::NextNoLock() {
    oss_.clear();
    oss_ << generator_();
    return oss_.str();
}

}  // namespace graphdb
