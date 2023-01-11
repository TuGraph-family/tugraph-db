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

#include <exception>
#include <string>

#include "core/data_type.h"

namespace lgraph {
class RpcException : public std::exception {
    std::string err_;

 public:
    explicit RpcException(const std::string& err) : err_("rpc exception: " + err) {}

    const char* what() const noexcept override { return err_.c_str(); }
};

class RpcStatusException : public RpcException {
 public:
    explicit RpcStatusException(const std::string& code)
        : RpcException("Server returned error: " + code) {}
};

class RpcConnectionException : public RpcException {
 public:
    explicit RpcConnectionException(const std::string& msg)
        : RpcException("Connection failed: " + msg) {}
};
}  // namespace lgraph
