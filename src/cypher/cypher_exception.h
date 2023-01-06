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

inline fma_common::Logger& PlanLogger() {
    static fma_common::Logger& logger = fma_common::Logger::Get("cypher.execution_plan");
    return logger;
}

inline fma_common::Logger& ParserLogger() {
    static fma_common::Logger& logger = fma_common::Logger::Get("cypher.parser");
    return logger;
}

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

class CypherException : public InputError {
 public:
    CypherException() = default;

    explicit CypherException(const std::string& err)
        : InputError(std::string("CypherException: ") + err) {}
};

class LexerException : public CypherException {
 public:
    explicit LexerException(const std::string& msg) : CypherException("LexerException: " + msg) {}
};

class ParserException : public CypherException {
 public:
    explicit ParserException(const std::string& msg) : CypherException("ParserException: " + msg) {}
};

class EvaluationException : public CypherException {
 public:
    explicit EvaluationException(const std::string& msg)
        : CypherException("EvaluationException: " + msg) {}
};

class TxnCommitException : public CypherException {
 public:
    explicit TxnCommitException(const std::string& msg)
        : CypherException("TxnCommitException: " + msg) {}
};

class ReminderException : public InputError {
 public:
    explicit ReminderException(const std::string& msg) : InputError(msg) {}
};
}  // namespace lgraph
