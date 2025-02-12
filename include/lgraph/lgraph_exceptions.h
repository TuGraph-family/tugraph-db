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
#include "fma-common/string_formatter.h"

namespace lgraph_api {

#define ERROR_CODES \
X(UnknownError, "Unknown error.") \
X(InvalidGalaxy, "Invalid Galaxy.") \
X(InvalidGraphDB, "Invalid GraphDB.") \
X(InvalidTxn, "Invalid transaction.") \
X(InvalidIterator, "Invalid iterator.") \
X(InvalidFork, "Write transactions cannot be forked.") \
X(TaskKilled, "Task is killed.") \
X(TxnConflict, "Transaction conflicts with an earlier one.") \
X(WriteNotAllowed, "Access denied.") \
X(DBNotExist, "The specified TuGraph DB does not exist.") \
X(IOError, "IO Error.") \
X(Unauthorized, "Authentication failed.") \
X(InternalError, "Internal error.") \
X(OutOfRange, "Out of range.") \
X(BadRequest, "Bad request.") \
X(InvalidParameter, "Invalid parameter.") \
X(Timeout, "Timeout.") \
X(KvException, "Kv exception.") \
X(InputError, "Input error.") \
X(FieldNotFound, "Field not found.") \
X(FieldAlreadyExists, "Field already exists.") \
X(FieldCannotBeNullType, "Field cannot be null type.") \
X(FieldCannotBeDeleted, "Field cannot be deleted.") \
X(FieldCannotBeSetNull, "Field cannot be set null.") \
X(ParseStringException, "Parse string exception.") \
X(ParseIncompatibleType, "Parse incompatible type.") \
X(ParseFieldDataException, "Parse field data exception.") \
X(DataSizeTooLarge, "Data size too large.") \
X(VectorSizeTooLarge, "Vector size too large.") \
X(RecordSizeLimitExceeded, "Record size limit exceeded.") \
X(LabelNotExist, "Label not exist.") \
X(LabelExist, "Label exist.") \
X(PrimaryIndexCannotBeDeleted, "Primary index cannot be deleted.") \
X(IndexNotExist, "Index not exist.") \
X(IndexExist, "Index exist.") \
X(FullTextIndexNotExist, "FullText index not exist.") \
X(FullTextIndexExist, "FullText index exist.") \
X(UserNotExist, "User not exist.") \
X(UserExist, "User exist.") \
X(GraphNotExist, "Graph not exist.") \
X(GraphExist, "Graph exist.") \
X(RoleNotExist, "Role not exist.") \
X(RoleExist, "Role exist.") \
X(PluginNotExist, "Plugin not exist.") \
X(PluginExist, "Plugin exist.") \
X(TaskNotExist, "Task not exist.") \
X(TaskKilledFailed, "Task killed failed.") \
X(InvalidPluginName, "Invalid plugin name.") \
X(InvalidPluginVersion, "Invalid plugin version.") \
X(CypherException, "Cypher exception.") \
X(GqlException, "Gql exception.") \
X(LexerException, "Lexer exception.") \
X(ParserException, "Parser exception.") \
X(EvaluationException, "Evaluation exception.") \
X(TxnCommitException, "Txn commit exception.") \
X(ReminderException, "Reminder exception.") \
X(GraphCreateException, "Graph create exception.") \
X(CypherParameterTypeError, "Cypher parameter type error.") \
X(ReachMaximumEid, "Edge eid exceeds the limit.")      \
X(ReachMaximumCompositeIndexField, "The size of composite index fields exceeds the limit.") \
X(PluginDisabled, "Plugin disabled!") \
X(BoltDataException, "Bolt data exception") \
X(VectorIndexException, "Vector index exception") \
X(BoltRaftError, "Bolt Raft error")

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
    explicit LgraphException(ErrorCode code, const std::string& msg);
    explicit LgraphException(ErrorCode code, const char* msg);
    template <typename... Ts>
    explicit LgraphException(ErrorCode code, const char* format, const Ts&... ds)
        : code_(code), msg_(FMA_FMT(format, ds...)) {
        what_ = FMA_FMT("[{}] {}", ErrorCodeToString(code_), msg_);
    }
    ErrorCode code() const {return code_;}
    const std::string& msg() const {return msg_;}
    const char* what() const noexcept override {
        return what_.c_str();
    };
 private:
    ErrorCode code_;
    std::string msg_;
    std::string what_;
};

#define THROW_CODE(code, ...) throw lgraph_api::LgraphException( \
    lgraph_api::ErrorCode::code, ##__VA_ARGS__)

}  // namespace lgraph_api
