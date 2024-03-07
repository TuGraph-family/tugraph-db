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

#pragma once

#include "fma-common/binary_read_write_helper.h"

#include "core/data_type.h"

namespace fma_common {
template <typename StreamT>
struct BinaryReader<StreamT, lgraph::FieldSpec> {
    static size_t Read(StreamT& stream, lgraph::FieldSpec& fs) {
        return BinaryRead(stream, fs.name) + BinaryRead(stream, fs.type) +
               BinaryRead(stream, fs.optional);
    }
};

template <typename StreamT>
struct BinaryWriter<StreamT, lgraph::FieldSpec> {
    static size_t Write(StreamT& stream, const lgraph::FieldSpec& fs) {
        return BinaryWrite(stream, fs.name) + BinaryWrite(stream, fs.type) +
               BinaryWrite(stream, fs.optional);
    }
};
}  // namespace fma_common

namespace lgraph {

namespace _detail {
inline std::string LimitLengthString(const std::string& str) {
    if (str.size() < 128) return str;
    return str.substr(0, 128) + "...";
}
}  // namespace _detail

using lgraph_api::LgraphException;
using lgraph_api::ErrorCode;
class FieldNotFoundException : public LgraphException {
 public:
    explicit FieldNotFoundException(const std::string& fname)
        : LgraphException(ErrorCode::FieldNotFoundException, FMA_FMT("Field [{}] does not exist.", fname)) {}

    explicit FieldNotFoundException(size_t fid)
        : LgraphException(ErrorCode::FieldNotFoundException, FMA_FMT("Field [#{}] does not exist.", fid)) {}
};

class FieldAlreadyExistsException : public LgraphException {
 public:
    explicit FieldAlreadyExistsException(const std::string& fname)
        : LgraphException(ErrorCode::FieldAlreadyExistsException, FMA_FMT("Field [{}] defined more than once.", fname)) {}

    explicit FieldAlreadyExistsException(size_t fid)
        : LgraphException(ErrorCode::FieldAlreadyExistsException, FMA_FMT("Field [#{}] defined more than once.", fid)) {}
};

class FieldCannotBeNullTypeException : public LgraphException {
 public:
    explicit FieldCannotBeNullTypeException(const std::string& fname)
        : LgraphException(ErrorCode::FieldCannotBeNullTypeException, FMA_FMT("Field [{}] cannot be NUL type.", fname)) {}

    explicit FieldCannotBeNullTypeException(size_t fid)
        : LgraphException(ErrorCode::FieldCannotBeNullTypeException, FMA_FMT("Field [#{}] cannot be NUL type.", fid)) {}
};

class FieldCannotBeDeletedException : public LgraphException {
 public:
    explicit FieldCannotBeDeletedException(const std::string& fname)
        : LgraphException(ErrorCode::FieldCannotBeDeletedException, FMA_FMT("Field [{}] cannot be deleted.", fname)) {}

    explicit FieldCannotBeDeletedException(size_t fid)
        : LgraphException(ErrorCode::FieldCannotBeDeletedException, FMA_FMT("Field [#{}] cannot be deleted.", fid)) {}
};

class FieldCannotBeSetNullException : public LgraphException {
 public:
    explicit FieldCannotBeSetNullException(const std::string& fname)
        : LgraphException(ErrorCode::FieldCannotBeSetNullException, FMA_FMT("Field [{}] is not optional.", fname)) {}

    explicit FieldCannotBeSetNullException(size_t fid)
        : LgraphException(ErrorCode::FieldCannotBeSetNullException, FMA_FMT("Field [#{}] is not optional.", fid)) {}
};

class TooManyFieldsException : public LgraphException {
 public:
    TooManyFieldsException()
        : LgraphException(ErrorCode::TooManyFieldsException, FMA_FMT("Too many fields. Maximum={}", _detail::MAX_NUM_FIELDS)) {}
};

class ParseStringException : public LgraphException {
 public:
    ParseStringException(const std::string& field, const std::string& str, FieldType dst_type)
        : LgraphException(ErrorCode::ParseStringException, FMA_FMT(
              "Failed to set field [{}]: Failed to parse string into type [{}], string is:{}",
              field, field_data_helper::TryGetFieldTypeName(dst_type),
              _detail::LimitLengthString(str))) {}
};

class ParseIncompatibleTypeException : public LgraphException {
 public:
    ParseIncompatibleTypeException(const std::string& field, FieldType src, FieldType dst)
        : LgraphException(ErrorCode::ParseIncompatibleTypeException, FMA_FMT("Failed to set field [{}]: Cannot convert data of type [{}] into [{}]",
                             field, field_data_helper::TryGetFieldTypeName(src),
                             field_data_helper::TryGetFieldTypeName(dst))) {}
};

class ParseFieldDataException : public LgraphException {
 public:
    ParseFieldDataException(const std::string& field, const FieldData& in, FieldType dst_type)
        : LgraphException(ErrorCode::ParseFieldDataException, FMA_FMT("Failed to set field [{}]: Cannot convert input [{}] into type [{}]",
                             field, in, field_data_helper::TryGetFieldTypeName(dst_type))) {}
};

class DataSizeTooLargeException : public LgraphException {
 public:
    DataSizeTooLargeException(const std::string& field, size_t dsize, size_t max_size)
        : LgraphException(ErrorCode::DataSizeTooLargeException, FMA_FMT("Failed to set field [{}]: Data size too big, max is {}, given {}",
                             field, max_size, dsize)) {}
};

class RecordSizeLimitExceededException : public LgraphException {
 public:
    RecordSizeLimitExceededException(const std::string& field, size_t dsize, size_t max_size)
        : LgraphException(ErrorCode::RecordSizeLimitExceededException,
              FMA_FMT("Failed to set field [{}]: Record size limit exceeded, max is {}, given {}",
                      field, max_size, dsize)) {}
};

class LabelNotExistException : public LgraphException {
 public:
    explicit LabelNotExistException(const std::string& lname)
        : LgraphException(ErrorCode::LabelNotExistException, FMA_FMT("Label [{}] does not exist.", lname)) {}

    explicit LabelNotExistException(size_t lid)
        : LgraphException(ErrorCode::LabelNotExistException, FMA_FMT("Label [#{}] does not exist.", lid)) {}
};

class LabelExistException : public LgraphException {
 public:
    LabelExistException(const std::string& label, bool is_vertex)
        : LgraphException(ErrorCode::LabelExistException,
              FMA_FMT("{} label [{}] already exists.", is_vertex ? "Vertex" : "Edge", label)) {}
};

class PrimaryIndexCannotBeDeletedException : public LgraphException {
 public:
    explicit PrimaryIndexCannotBeDeletedException(const std::string& fname)
        : LgraphException(ErrorCode::PrimaryIndexCannotBeDeletedException, FMA_FMT("Primary index [{}] can not be deleted.", fname)) {}

    explicit PrimaryIndexCannotBeDeletedException(size_t fid)
        : LgraphException(ErrorCode::PrimaryIndexCannotBeDeletedException, FMA_FMT("Primary index [#{}] can not be deleted.", fid)) {}
};

class IndexNotExistException : public LgraphException {
 public:
    IndexNotExistException(const std::string& label, const std::string& field)
        : LgraphException(ErrorCode::IndexNotExistException, FMA_FMT("VertexIndex [{}:{}] does not exist.", label, field)) {}
};

class IndexExistException : public LgraphException {
 public:
    IndexExistException(const std::string& label, const std::string& field)
        : LgraphException(ErrorCode::IndexExistException, FMA_FMT("VertexIndex [{}:{}] already exist.", label, field)) {}
};

class FullTextIndexNotExistException : public LgraphException {
 public:
    FullTextIndexNotExistException(const std::string& label, const std::string& field)
        : LgraphException(ErrorCode::FullTextIndexNotExistException, FMA_FMT("FullText Index [{}:{}] does not exist.", label, field)) {}
};

class FullTextIndexExistException : public LgraphException {
 public:
    FullTextIndexExistException(const std::string& label, const std::string& field)
        : LgraphException(ErrorCode::FullTextIndexExistException, FMA_FMT("FullText Index [{}:{}] already exist.", label, field)) {}
};

class UserNotExistException : public LgraphException {
 public:
    explicit UserNotExistException(const std::string& user)
        : LgraphException(ErrorCode::UserNotExistException, FMA_FMT("User [{}] does not exist.", user)) {}
};

class UserExistException : public LgraphException {
 public:
    explicit UserExistException(const std::string& user)
        : LgraphException(ErrorCode::UserExistException, FMA_FMT("User [{}] already exist.", user)) {}
};

class GraphNotExistException : public LgraphException {
 public:
    explicit GraphNotExistException(const std::string& graph)
        : LgraphException(ErrorCode::GraphNotExistException, FMA_FMT("graph [{}] does not exist.", graph)) {}
};

class GraphExistException : public LgraphException {
 public:
    explicit GraphExistException(const std::string& graph)
        : LgraphException(ErrorCode::GraphExistException, FMA_FMT("graph [{}] already exist.", graph)) {}
};

class GraphCreateException : public LgraphException {
 public:
    explicit GraphCreateException(const std::string& graph)
        : LgraphException(FMA_FMT("Failed to create graph [{}] with lmdb file.", graph)) {}
};

class RoleNotExistException : public LgraphException {
 public:
    explicit RoleNotExistException(const std::string& role)
        : LgraphException(ErrorCode::RoleNotExistException, FMA_FMT("role [{}] does not exist.", role)) {}
};

class RoleExistException : public LgraphException {
 public:
    explicit RoleExistException(const std::string& role)
        : LgraphException(ErrorCode::RoleExistException, FMA_FMT("role [{}] already exist.", role)) {}
};

class PluginNotExistException : public LgraphException {
 public:
    explicit PluginNotExistException(const std::string& plugin)
        : LgraphException(ErrorCode::PluginNotExistException, FMA_FMT("plugin [{}] does not exist.", plugin)) {}
};

class PluginExistException : public LgraphException {
 public:
    explicit PluginExistException(const std::string& plugin)
        : LgraphException(ErrorCode::PluginExistException, FMA_FMT("plugin [{}] already exist.", plugin)) {}
};

class TaskNotExistException : public LgraphException {
 public:
    explicit TaskNotExistException(const std::string& task_id)
        : LgraphException(ErrorCode::TaskNotExistException,
              FMA_FMT("Task [{}] not exist.", task_id)) {}
};

class TaskKilledFailedException : public LgraphException {
 public:
    explicit TaskKilledFailedException(const std::string& task_id)
        : LgraphException(ErrorCode::TaskKilledFailedException,
              FMA_FMT("Task [{}]  did not respond to kill signal.", task_id)) {}
};

}  // namespace lgraph
