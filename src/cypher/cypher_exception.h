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
#include "fma-common/string_formatter.h"

namespace lgraph {

#ifndef NDEBUG
#define CYPHER_TODO_FILE_NAME std::string(__FILE__)
#else
#define CYPHER_TODO_FILE_NAME ""
#endif

#define CYPHER_TODO()                                                                        \
    do {                                                                                     \
        throw lgraph::CypherException(                                                       \
            fma_common::StringFormatter::Format("Function not implemented yet: {} at {}:{}", \
                                                __func__, CYPHER_TODO_FILE_NAME, __LINE__)); \
    } while (0)

#define CYPHER_INTL_ERR()                                                               \
    do {                                                                                \
        throw lgraph::CypherException(fma_common::StringFormatter::Format(              \
            "Internal error: {} at {}:{}", __func__, CYPHER_TODO_FILE_NAME, __LINE__)); \
    } while (0)

#define CYPHER_THROW_ASSERT(pred)       \
    do {                                \
        if (!(pred)) CYPHER_INTL_ERR(); \
    } while (0)

#define CYPHER_PARSER_CHECK(pred, msg)                               \
    if (!(pred)) {                                                   \
        throw lgraph::ParserException("error around '" + msg + "'"); \
    }
#define CYPHER_ARGUMENT_ERROR()                                                                   \
    do {                                                                                          \
        throw lgraph::CypherException(                                                            \
            fma_common::StringFormatter::Format("There are errors in the number of arguments or " \
                                                "the type of arguments of function {}.",          \
                                                __func__));                                       \
    } while (0)

using lgraph_api::LgraphException;
using lgraph_api::ErrorCode;
class CypherException : public LgraphException {
 public:
    CypherException() = delete;

    explicit CypherException(const std::string& err)
        : LgraphException(ErrorCode::CypherException, std::string("CypherException: ") + err) {}
};
class GqlException : public LgraphException {
 public:
    GqlException() = delete;

    explicit GqlException(const std::string& err)
        : LgraphException(ErrorCode::GqlException, std::string("GqlException: ") + err) {}
};

class LexerException : public LgraphException {
 public:
    explicit LexerException(const std::string& msg)
        : LgraphException(ErrorCode::LexerException, "LexerException: " + msg) {}
};

class ParserException : public LgraphException {
 public:
    explicit ParserException(const std::string& msg)
        : LgraphException(ErrorCode::ParserException, "ParserException: " + msg) {}
};

class EvaluationException : public LgraphException {
 public:
    explicit EvaluationException(const std::string& msg)
        : LgraphException(ErrorCode::EvaluationException, "EvaluationException: " + msg) {}
};

class TxnCommitException : public LgraphException {
 public:
    explicit TxnCommitException(const std::string& msg)
        : LgraphException(ErrorCode::TxnCommitException, "TxnCommitException: " + msg) {}
};

class ReminderException : public LgraphException {
 public:
    explicit ReminderException(const std::string& msg)
        : LgraphException(ErrorCode::ReminderException, msg) {}
};
}  // namespace lgraph
