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

/*
 * written by botu.wzy
 */

#pragma once

#include <exception>
#include <stdexcept>
#include <string>
#include <sstream>
#include <spdlog/fmt/fmt.h>

#define ERROR_CODES \
X(UnknownError, "Unknown Error.") \
X(VertexIdNotFound, "Vertex id not found.") \
X(StorageEngineError, "Storage engine error.") \
X(EdgeTypeNotFound, "Edge type not found.") \
X(EdgeIdNotFound, "Edge id not found.")     \
X(VertexIndexAlreadyExist, "Vertex index already exist.") \
X(IndexValueAlreadyExist, "Index value already exist.") \
X(NoSuchGraph, "No such graph.")  \
X(GraphAlreadyExists, "The graph already exists.") \
X(IndexBusy, "Index is building.") \
X(CypherException, "Cypher exception.") \
X(ParserException, "Parser exception.") \
X(InputError, "Input error.") \
X(EvaluationException, "Evaluation exception.") \
X(ReminderException, "Reminder exception.") \
X(FullTextIndexNotFound, "FullText index not found.")     \
X(VectorIndexNotFound, "Vector index not found.")         \
X(VertexUniqueIndexNotFound, "Vertex unique index not found.")      \
X(BoltDataException, "Bolt data exception.") \
X(ValueException, "Value exception.")       \
X(OutOfRange, "Out of range.") \
X(InvalidParameter, "Invalid parameter.") \
X(VectorIndexException, "Vector index exception.")        \
X(VertexVectorIndexAlreadyExist, "Vertex vector index already exist.") \
X(VertexFullTextIndexAlreadyExist, "Vertex fulltext index already exist.") \
X(ConnectionDisconnected, "Connection has been disconnected.")      \
X(Unimplemented, "Unimplemented.")

enum class ErrorCode {
#define X(code, msg) code,
    ERROR_CODES
#undef X
};

const char* ErrorCodeToString(ErrorCode code);
const char* ErrorCodeDesc(ErrorCode code);

class LgraphException : public std::exception {
public:
    explicit LgraphException(ErrorCode code);
    explicit LgraphException(ErrorCode code, std::string msg);
    explicit LgraphException(ErrorCode code, const char* msg);
    template <typename... Ts>
    explicit LgraphException(ErrorCode code, const char* format, const Ts&... ds)
            : code_(code), msg_(fmt::format(format, ds...)) {
        what_ = fmt::format("[{}] {}", ErrorCodeToString(code_), msg_);
    }
    [[nodiscard]] ErrorCode code() const {return code_;}
    [[nodiscard]] const std::string& msg() const {return msg_;}
    [[nodiscard]] const char* what() const noexcept override {
        return what_.c_str();
    };
private:
    ErrorCode code_;
    std::string msg_;
    std::string what_;
};

#define THROW_CODE(code, ...) throw LgraphException(ErrorCode::code, ##__VA_ARGS__)
