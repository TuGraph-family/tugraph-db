//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once

#include <exception>
#include <stdexcept>
#include <string>
#include <sstream>
#if LGRAPH_ENABLE_BOOST_STACKTRACE
#include <boost/stacktrace.hpp>
#endif

namespace lgraph_api {
/** @brief   User input error. Base class for a variety of input errors. */
class InputError : public std::exception {
 protected:
    std::string err_;

 public:
    explicit InputError(const std::string& err) : err_(err) {
#if LGRAPH_ENABLE_BOOST_STACKTRACE
        std::ostringstream oss;
        oss << boost::stacktrace::stacktrace();
        err_.append("\nBEGIN_STACK =============\n" + oss.str() +
                    "\nEND_STACK =============\n");
#endif
    }

    const char* what() const noexcept override { return err_.c_str(); }
};

/** @brief   Data out of range. */
class OutOfRangeError : public std::range_error {
 public:
    explicit OutOfRangeError(const std::string& err) : std::range_error(err) {}
};

/** @brief   An invalid parameter is passed. */
class InvalidParameterError : public InputError {
 public:
    explicit InvalidParameterError(const std::string& msg = "Invalid parameter.")
        : InputError(msg) {}
};

/** @brief   Function called on an invalid Galaxy. */
class InvalidGalaxyError : public InvalidParameterError {
 public:
    InvalidGalaxyError() : InvalidParameterError("Invalid Galaxy.") {}
};

/** @brief   Function called on an invalid GraphDB. */
class InvalidGraphDBError : public InvalidParameterError {
 public:
    InvalidGraphDBError() : InvalidParameterError("Invalid GraphDB.") {}
};

/** @brief   Function called on an invalid transaction. */
class InvalidTxnError : public InvalidParameterError {
 public:
    InvalidTxnError() : InvalidParameterError("Invalid transaction.") {}
};

/** @brief   Function called on an invalid iterator. */
class InvalidIteratorError : public InvalidParameterError {
 public:
    InvalidIteratorError() : InvalidParameterError("Invalid iterator.") {}
};

/** @brief   ForkTxn called on a write transaction. */
class InvalidForkError : public InvalidParameterError {
 public:
    InvalidForkError() : InvalidParameterError("Write transactions cannot be forked.") {}
};

/** @brief   Task is being killed per user request or timeout. */
class TaskKilledException : public std::runtime_error {
 public:
    TaskKilledException() : std::runtime_error("Task killed.") {}
};

/** @brief   A conflict is detected when committing this transaction. */
class TxnConflictError : public std::runtime_error {
 public:
    TxnConflictError()
        : std::runtime_error("Transaction conflicts with an earlier one.") {}
};

/** @brief   Write operation is tried on a read-only GraphDB or read-only transaction. */
class WriteNotAllowedError : public std::runtime_error {
 public:
    explicit WriteNotAllowedError(const std::string& msg = "Access denied.")
        : std::runtime_error(msg) {}
};

/** @brief   Specified database does not exist, wrong directory? */
class DBNotExistError : public InputError {
 public:
    explicit DBNotExistError(const std::string& msg = "The specified TuGraph DB does not exist.")
        : InputError(msg) {}
};

/** @brief   An i/o error. */
class IOError : public std::runtime_error {
 public:
    explicit IOError(const std::string& msg = "IO Error.") : std::runtime_error(msg) {}
};

/** @brief   User not authorized to perform this action. */
class UnauthorizedError : public InputError {
 public:
    explicit UnauthorizedError(const std::string& msg = "Authentication failed.")
        : InputError(msg) {}
};

class InternalErrorException : public std::exception {
    std::string err_;

 public:
    explicit InternalErrorException(const std::string& err) : err_("InternalError " + err) {}

    const char* what() const noexcept override { return err_.c_str(); }
};

class BadRequestException : public std::exception {
    std::string err_;

 public:
    explicit BadRequestException(const std::string& err) : err_("BadRequest " + err) {}

    const char* what() const noexcept override { return err_.c_str(); }
};

}  // namespace lgraph_api
