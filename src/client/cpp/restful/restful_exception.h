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

#include <stdexcept>
#include <string>

namespace lgraph {
class RestfulException : public std::runtime_error {
 public:
    explicit RestfulException(const std::string& err) : std::runtime_error(err) {}
};

class StatusException : public RestfulException {
 public:
    explicit StatusException(const std::string& code)
        : RestfulException("status code error [" + code + "]") {}
};

class ConnectionException : public RestfulException {
 public:
    explicit ConnectionException(const std::string& msg)
        : RestfulException("connection fails [" + msg + "]") {}
};
}  // namespace lgraph
