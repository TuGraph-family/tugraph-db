/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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

class FieldNotFoundException : public InputError {
 public:
    explicit FieldNotFoundException(const std::string& fname)
        : InputError(FMA_FMT("Field [{}] does not exist.", fname)) {}

    explicit FieldNotFoundException(size_t fid)
        : InputError(FMA_FMT("Field [#{}] does not exist.", fid)) {}
};

class FieldAlreadyExistsException : public InputError {
 public:
    explicit FieldAlreadyExistsException(const std::string& fname)
        : InputError(FMA_FMT("Field [{}] defined more than once.", fname)) {}

    explicit FieldAlreadyExistsException(size_t fid)
        : InputError(FMA_FMT("Field [#{}] defined more than once.", fid)) {}
};

class FieldCannotBeNullTypeException : public InputError {
 public:
    explicit FieldCannotBeNullTypeException(const std::string& fname)
        : InputError(FMA_FMT("Field [{}] cannot be NUL type.", fname)) {}

    explicit FieldCannotBeNullTypeException(size_t fid)
        : InputError(FMA_FMT("Field [#{}] cannot be NUL type.", fid)) {}
};

class FieldCannotBeDeletedException : public InputError {
 public:
    explicit FieldCannotBeDeletedException(const std::string& fname)
        : InputError(FMA_FMT("Field [{}] cannot be deleted.", fname)) {}

    explicit FieldCannotBeDeletedException(size_t fid)
        : InputError(FMA_FMT("Field [#{}] cannot be deleted.", fid)) {}
};

class FieldCannotBeSetNullException : public InputError {
 public:
    explicit FieldCannotBeSetNullException(const std::string& fname)
        : InputError(FMA_FMT("Field [{}] is not optional.", fname)) {}

    explicit FieldCannotBeSetNullException(size_t fid)
        : InputError(FMA_FMT("Field [#{}] is not optional.", fid)) {}
};

class TooManyFieldsException : public InputError {
 public:
    TooManyFieldsException()
        : InputError(FMA_FMT("Too many fields. Maximum={}", _detail::MAX_NUM_FIELDS)) {}
};

class ParseStringException : public InputError {
 public:
    ParseStringException(const std::string& field, const std::string& str, FieldType dst_type)
        : InputError(FMA_FMT(
              "Failed to set field [{}]: Failed to parse string into type [{}], string is:{}",
              field, field_data_helper::TryGetFieldTypeName(dst_type),
              _detail::LimitLengthString(str))) {}
};

class ParseIncompatibleTypeException : public InputError {
 public:
    ParseIncompatibleTypeException(const std::string& field, FieldType src, FieldType dst)
        : InputError(FMA_FMT("Failed to set field [{}]: Cannot convert data of type [{}] into [{}]",
                             field, field_data_helper::TryGetFieldTypeName(src),
                             field_data_helper::TryGetFieldTypeName(dst))) {}
};

class ParseFieldDataException : public InputError {
 public:
    ParseFieldDataException(const std::string& field, const FieldData& in, FieldType dst_type)
        : InputError(FMA_FMT("Failed to set field [{}]: Cannot convert input [{}] into type [{}]",
                             field, in, field_data_helper::TryGetFieldTypeName(dst_type))) {}
};

class DataSizeTooLargeException : public InputError {
 public:
    DataSizeTooLargeException(const std::string& field, size_t dsize, size_t max_size)
        : InputError(FMA_FMT("Failed to set field [{}]: Data size too big, max is {}, given {}",
                             field, max_size, dsize)) {}
};

class RecordSizeLimitExceededException : public InputError {
 public:
    RecordSizeLimitExceededException(const std::string& field, size_t dsize, size_t max_size)
        : InputError(
              FMA_FMT("Failed to set field [{}]: Record size limit exceeded, max is {}, given {}",
                      field, max_size, dsize)) {}
};

class LabelNotExistException : public InputError {
 public:
    explicit LabelNotExistException(const std::string& lname)
        : InputError(FMA_FMT("Label [{}] does not exist.", lname)) {}

    explicit LabelNotExistException(size_t lid)
        : InputError(FMA_FMT("Label [#{}] does not exist.", lid)) {}
};

class LabelExistException : public InputError {
 public:
    LabelExistException(const std::string& label, bool is_vertex)
        : InputError(
              FMA_FMT("{} label [{}] already exists.", is_vertex ? "Vertex" : "Edge", label)) {}
};

class PrimaryIndexCannotBeDeletedException : public InputError {
 public:
    explicit PrimaryIndexCannotBeDeletedException(const std::string& fname)
        : InputError(FMA_FMT("Primary index [{}] can not be deleted.", fname)) {}

    explicit PrimaryIndexCannotBeDeletedException(size_t fid)
        : InputError(FMA_FMT("Primary index [#{}] can not be deleted.", fid)) {}
};

class IndexNotExistException : public InputError {
 public:
    IndexNotExistException(const std::string& label, const std::string& field)
        : InputError(FMA_FMT("VertexIndex [{}:{}] does not exist.", label, field)) {}
};

class IndexExistException : public InputError {
 public:
    IndexExistException(const std::string& label, const std::string& field)
        : InputError(FMA_FMT("VertexIndex [{}:{}] already exist.", label, field)) {}
};

class FullTextIndexNotExistException : public InputError {
 public:
    FullTextIndexNotExistException(const std::string& label, const std::string& field)
        : InputError(FMA_FMT("FullText Index [{}:{}] does not exist.", label, field)) {}
};

class FullTextIndexExistException : public InputError {
 public:
    FullTextIndexExistException(const std::string& label, const std::string& field)
        : InputError(FMA_FMT("FullText Index [{}:{}] already exist.", label, field)) {}
};

class UserNotExistException : public InputError {
 public:
    explicit UserNotExistException(const std::string& user)
        : InputError(FMA_FMT("User [{}] does not exist.", user)) {}
};

class UserExistException : public InputError {
 public:
    explicit UserExistException(const std::string& user)
        : InputError(FMA_FMT("User [{}] already exist.", user)) {}
};

class GraphNotExistException : public InputError {
 public:
    explicit GraphNotExistException(const std::string& graph)
        : InputError(FMA_FMT("graph [{}] does not exist.", graph)) {}
};

class GraphExistException : public InputError {
 public:
    explicit GraphExistException(const std::string& graph)
        : InputError(FMA_FMT("graph [{}] already exist.", graph)) {}
};

class RoleNotExistException : public InputError {
 public:
    explicit RoleNotExistException(const std::string& role)
        : InputError(FMA_FMT("role [{}] does not exist.", role)) {}
};

class RoleExistException : public InputError {
 public:
    explicit RoleExistException(const std::string& role)
        : InputError(FMA_FMT("role [{}] already exist.", role)) {}
};

class PluginNotExistException : public InputError {
 public:
    explicit PluginNotExistException(const std::string& plugin)
        : InputError(FMA_FMT("plugin [{}] does not exist.", plugin)) {}
};

class PluginExistException : public InputError {
 public:
    explicit PluginExistException(const std::string& plugin)
        : InputError(FMA_FMT("plugin [{}] already exist.", plugin)) {}
};

class TaskNotExistException : public InputError {
 public:
    explicit TaskNotExistException(const std::string& task_id)
        : InputError(
              FMA_FMT("Task [{}] not exist.", task_id)) {}
};

class TaskKilledFailedException : public InputError {
 public:
    explicit TaskKilledFailedException(const std::string& task_id)
        : InputError(
              FMA_FMT("Task [{}]  did not respond to kill signal.", task_id)) {}
};

}  // namespace lgraph
