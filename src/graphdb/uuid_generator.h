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

#pragma once

#include <boost/uuid/uuid_generators.hpp>
#include <mutex>
#include <sstream>
#include <string>

namespace graphdb {

class UUIDGenerator {
   public:
    std::string Next();
    std::string NextNoLock();

   private:
    boost::uuids::random_generator generator_;
    std::ostringstream oss_;
    std::mutex mutex_;
};

}  // namespace graphdb
