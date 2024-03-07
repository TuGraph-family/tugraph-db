//  Copyright 2024 AntGroup CO., Ltd.
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
X(InvalidGalaxyError, "Invalid Galaxy.") \
X(InvalidGraphDBError, "Invalid GraphDB.") \
X(InvalidTxnError, "Invalid transaction.") \
X(InvalidIteratorError, "Invalid iterator.") \
X(InvalidForkError, "Write transactions cannot be forked.") \
X(TaskKilledException, "Task killed.") \
X(TxnConflictError, "Transaction conflicts with an earlier one.") \
X(WriteNotAllowedError, "Access denied.") \
X(DBNotExistError, "The specified TuGraph DB does not exist.") \
X(IOError, "IO Error.") \
X(UnauthorizedError, "Authentication failed.") \
X(InternalErrorException, "InternalErrorException.") \
X(OutOfRangeError, "OutOfRangeError.") \
X(BadRequestException, "BadRequestException.") \
X(InvalidParameterError, "InvalidParameterError.") \
X(InternalError, "InternalError.") \
X(TimeoutException, "TimeoutException.") \
X(KvException, "KvException.") \
X(InputError, "InputError.") \
X(FieldNotFoundException, "FieldNotFoundException.") \
X(FieldAlreadyExistsException, "FieldAlreadyExistsException.") \
X(FieldCannotBeNullTypeException, "FieldCannotBeNullTypeException.") \
X(FieldCannotBeDeletedException, "FieldCannotBeDeletedException.") \
X(FieldCannotBeSetNullException, "FieldCannotBeSetNullException.") \
X(TooManyFieldsException, "TooManyFieldsException.") \
X(ParseStringException, "ParseStringException.") \
X(ParseIncompatibleTypeException, "ParseIncompatibleTypeException.") \
X(ParseFieldDataException, "ParseFieldDataException.") \
X(DataSizeTooLargeException, "DataSizeTooLargeException.") \
X(RecordSizeLimitExceededException, "RecordSizeLimitExceededException.") \
X(LabelNotExistException, "LabelNotExistException.") \
X(LabelExistException, "LabelExistException.") \
X(PrimaryIndexCannotBeDeletedException, "PrimaryIndexCannotBeDeletedException.") \
X(IndexNotExistException, "IndexNotExistException.") \
X(IndexExistException, "IndexExistException.") \
X(FullTextIndexNotExistException, "FullTextIndexNotExistException.") \
X(FullTextIndexExistException, "FullTextIndexExistException.") \
X(UserNotExistException, "UserNotExistException.") \
X(UserExistException, "UserExistException.") \
X(GraphNotExistException, "GraphNotExistException.") \
X(GraphExistException, "GraphExistException.") \
X(RoleNotExistException, "RoleNotExistException.") \
X(RoleExistException, "RoleExistException.") \
X(PluginNotExistException, "PluginNotExistException.") \
X(PluginExistException, "PluginExistException.") \
X(TaskNotExistException, "TaskNotExistException.") \
X(TaskKilledFailedException, "TaskKilledFailedException.") \
X(InvalidPluginNameException, "InvalidPluginNameException.") \
X(InvalidPluginVersionException, "InvalidPluginVersionException.") \
X(CypherException, "CypherException.") \
X(GqlException, "GqlException.") \
X(LexerException, "LexerException.") \
X(ParserException, "ParserException.") \
X(EvaluationException, "EvaluationException.") \
X(TxnCommitException, "TxnCommitException.") \
X(ReminderException, "ReminderException.")



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
    ErrorCode code() const {return code_;};
    const std::string& msg() {return msg_;};
    const char* what() const noexcept override {
        return what_.c_str();
    };
 private:
    ErrorCode code_;
    std::string msg_;
    std::string what_;
};

#define THROW_CODE(code,...) throw lgraph_api::LgraphException(lgraph_api::ErrorCode::code, ##__VA_ARGS__)

}  // namespace lgraph_api
