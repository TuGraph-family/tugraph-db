/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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
