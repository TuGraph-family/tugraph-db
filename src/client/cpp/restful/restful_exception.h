/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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
