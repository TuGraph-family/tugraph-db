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
        : LgraphException(ErrorCode::FieldNotFound,
                          "Field [{}] does not exist.", fname) {}

    explicit FieldNotFoundException(size_t fid)
        : LgraphException(ErrorCode::FieldNotFound,
                          "Field [#{}] does not exist.", fid) {}
};

class FieldAlreadyExistsException : public LgraphException {
 public:
    explicit FieldAlreadyExistsException(const std::string& fname)
        : LgraphException(ErrorCode::FieldAlreadyExists,
                          "Field [{}] defined more than once.", fname) {}

    explicit FieldAlreadyExistsException(size_t fid)
        : LgraphException(ErrorCode::FieldAlreadyExists,
                          "Field [#{}] defined more than once.", fid) {}
};

class FieldCannotBeNullTypeException : public LgraphException {
 public:
    explicit FieldCannotBeNullTypeException(const std::string& fname)
        : LgraphException(ErrorCode::FieldCannotBeNullType,
                          "Field [{}] cannot be NUL type.", fname) {}

    explicit FieldCannotBeNullTypeException(size_t fid)
        : LgraphException(ErrorCode::FieldCannotBeNullType,
                          "Field [#{}] cannot be NUL type.", fid) {}
};

class FieldCannotBeDeletedException : public LgraphException {
 public:
    explicit FieldCannotBeDeletedException(const std::string& fname)
        : LgraphException(ErrorCode::FieldCannotBeDeleted,
                          "Field [{}] cannot be deleted.", fname) {}

    explicit FieldCannotBeDeletedException(size_t fid)
        : LgraphException(ErrorCode::FieldCannotBeDeleted,
                          "Field [#{}] cannot be deleted.", fid) {}
};

class FieldCannotBeSetNullException : public LgraphException {
 public:
    explicit FieldCannotBeSetNullException(const std::string& fname)
        : LgraphException(ErrorCode::FieldCannotBeSetNull,
                          "Field [{}] is not optional.", fname) {}

    explicit FieldCannotBeSetNullException(size_t fid)
        : LgraphException(ErrorCode::FieldCannotBeSetNull,
                          "Field [#{}] is not optional.", fid) {}
};

class ParseStringException : public LgraphException {
 public:
    ParseStringException(const std::string& field, const std::string& str, FieldType dst_type)
        : LgraphException(ErrorCode::ParseStringException,
              "Failed to set field [{}]: Failed to parse string into type [{}], string is:{}",
              field, field_data_helper::TryGetFieldTypeName(dst_type),
              _detail::LimitLengthString(str)) {}
};

class ParseIncompatibleTypeException : public LgraphException {
 public:
    ParseIncompatibleTypeException(const std::string& field, FieldType src, FieldType dst)
        : LgraphException(ErrorCode::ParseIncompatibleType,
                          "Failed to set field [{}]: Cannot convert data of type [{}] into [{}]",
                          field, field_data_helper::TryGetFieldTypeName(src),
                          field_data_helper::TryGetFieldTypeName(dst)) {}
};

class ParseFieldDataException : public LgraphException {
 public:
    ParseFieldDataException(const std::string& field, const FieldData& in, FieldType dst_type)
        : LgraphException(ErrorCode::ParseFieldDataException,
                          "Failed to set field [{}]: Cannot convert input [{}] into type [{}]",
                             field, in, field_data_helper::TryGetFieldTypeName(dst_type)) {}
};

class DataSizeTooLargeException : public LgraphException {
 public:
    DataSizeTooLargeException(const std::string& field, size_t dsize, size_t max_size)
        : LgraphException(ErrorCode::DataSizeTooLarge,
                          "Failed to set field [{}]: Data size too big, max is {}, given {}",
                          field, max_size, dsize) {}
};

class RecordSizeLimitExceededException : public LgraphException {
 public:
    RecordSizeLimitExceededException(const std::string& field, size_t dsize, size_t max_size)
        : LgraphException(
              ErrorCode::RecordSizeLimitExceeded,
              "Failed to set field [{}]: Record size limit exceeded, max is {}, given {}",
              field, max_size, dsize) {}
};

class LabelNotExistException : public LgraphException {
 public:
    explicit LabelNotExistException(const std::string& lname)
        : LgraphException(ErrorCode::LabelNotExist,
                          "Label [{}] does not exist.", lname) {}

    explicit LabelNotExistException(size_t lid)
        : LgraphException(ErrorCode::LabelNotExist,
                          "Label [#{}] does not exist.", lid) {}
};

class LabelExistException : public LgraphException {
 public:
    LabelExistException(const std::string& label, bool is_vertex)
        : LgraphException(ErrorCode::LabelExist,
                          "{} label [{}] already exists.", is_vertex ? "Vertex" : "Edge", label) {}
};

class PrimaryIndexCannotBeDeletedException : public LgraphException {
 public:
    explicit PrimaryIndexCannotBeDeletedException(const std::string& fname)
        : LgraphException(ErrorCode::PrimaryIndexCannotBeDeleted,
                          "Primary index [{}] can not be deleted.", fname) {}

    explicit PrimaryIndexCannotBeDeletedException(size_t fid)
        : LgraphException(ErrorCode::PrimaryIndexCannotBeDeleted,
                          "Primary index [#{}] can not be deleted.", fid) {}
};

class IndexNotExistException : public LgraphException {
 public:
    IndexNotExistException(const std::string& label, const std::string& field)
        : LgraphException(ErrorCode::IndexNotExist,
                          "VertexIndex [{}:{}] does not exist.", label, field) {}
};

class IndexExistException : public LgraphException {
 public:
    IndexExistException(const std::string& label, const std::string& field)
        : LgraphException(ErrorCode::IndexExist,
                          "VertexIndex [{}:{}] already exist.", label, field) {}
};

class FullTextIndexNotExistException : public LgraphException {
 public:
    FullTextIndexNotExistException(const std::string& label, const std::string& field)
        : LgraphException(ErrorCode::FullTextIndexNotExist,
                          "FullText Index [{}:{}] does not exist.", label, field) {}
};

class FullTextIndexExistException : public LgraphException {
 public:
    FullTextIndexExistException(const std::string& label, const std::string& field)
        : LgraphException(ErrorCode::FullTextIndexExist,
                          "FullText Index [{}:{}] already exist.", label, field) {}
};

class UserNotExistException : public LgraphException {
 public:
    explicit UserNotExistException(const std::string& user)
        : LgraphException(ErrorCode::UserNotExist,
                          "User [{}] does not exist.", user) {}
};

class UserExistException : public LgraphException {
 public:
    explicit UserExistException(const std::string& user)
        : LgraphException(ErrorCode::UserExist,
                          "User [{}] already exist.", user) {}
};

class GraphNotExistException : public LgraphException {
 public:
    explicit GraphNotExistException(const std::string& graph)
        : LgraphException(ErrorCode::GraphNotExist,
                          "graph [{}] does not exist.", graph) {}
};

class GraphExistException : public LgraphException {
 public:
    explicit GraphExistException(const std::string& graph)
        : LgraphException(ErrorCode::GraphExist,
                          "graph [{}] already exist.", graph) {}
};

class GraphCreateException : public LgraphException {
 public:
    explicit GraphCreateException(const std::string& graph)
        : LgraphException(ErrorCode::GraphCreateException,
                          "Failed to create graph [{}] with lmdb file.", graph) {}
};

class RoleNotExistException : public LgraphException {
 public:
    explicit RoleNotExistException(const std::string& role)
        : LgraphException(ErrorCode::RoleNotExist,
                          "role [{}] does not exist.", role) {}
};

class RoleExistException : public LgraphException {
 public:
    explicit RoleExistException(const std::string& role)
        : LgraphException(ErrorCode::RoleExist,
                          "role [{}] already exist.", role) {}
};

class PluginNotExistException : public LgraphException {
 public:
    explicit PluginNotExistException(const std::string& plugin)
        : LgraphException(ErrorCode::PluginNotExist,
                          "plugin [{}] does not exist.", plugin) {}
};

class PluginExistException : public LgraphException {
 public:
    explicit PluginExistException(const std::string& plugin)
        : LgraphException(ErrorCode::PluginExist,
                          "plugin [{}] already exist.", plugin) {}
};

class TaskNotExistException : public LgraphException {
 public:
    explicit TaskNotExistException(const std::string& task_id)
        : LgraphException(ErrorCode::TaskNotExist,
              FMA_FMT("Task [{}] not exist.", task_id)) {}
};

class TaskKilledFailedException : public LgraphException {
 public:
    explicit TaskKilledFailedException(const std::string& task_id)
        : LgraphException(ErrorCode::TaskKilledFailed,
                          "Task [{}]  did not respond to kill signal.", task_id) {}
};

}  // namespace lgraph
