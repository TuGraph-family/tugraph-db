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

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>

#include "core/data_type.h"
#include "core/field_data_helper.h"
#include "import/block_parser.h"
#include "import/import_config_parser.h"
#include "restful/server/json_convert.h"
#include "tools/json.hpp"

namespace lgraph {
namespace import_v2 {


class JsonLinesParser : public BlockParser {
 public:
    typedef std::function<std::tuple<size_t, bool>(const char*, const char*,
                                                   std::vector<FieldData>&)>
        ParseFunc;
    JsonLinesParser(std::unique_ptr<fma_common::InputFileStream> stream,
                    const std::vector<FieldSpec>& field_specs, size_t block_size, size_t n_threads,
                    size_t n_header_lines, bool forgiving, int64_t max_err_msgs = 100)
        : stream_(std::move(stream)),
          field_specs_(field_specs),
          forgiving_(forgiving),
          max_errors_(max_err_msgs) {
        init(block_size, n_threads, n_header_lines);
    }
    JsonLinesParser(const std::string& path, const std::vector<FieldSpec>& field_specs,
                    size_t block_size, size_t n_threads, size_t n_header_lines, bool forgiving,
                    int64_t max_err_msgs = 100)
        : stream_(new fma_common::InputFmaStream(path)),
          field_specs_(field_specs),
          forgiving_(forgiving),
          max_errors_(max_err_msgs) {
        if (!stream_->Good()) {
            LOG_INFO() << "Failed to open input file " << path;
            throw std::runtime_error("failed to open input file [" + path + "]");
        }
        init(block_size, n_threads, n_header_lines);
    }

    ~JsonLinesParser() { parser_->Stop(); }

    bool ReadBlock(std::vector<std::vector<FieldData>>& buf) { return parser_->ReadBlock(buf); }

 private:
    void init(size_t block_size, size_t n_threads, size_t n_header_lines) {
        parser_.reset(new fma_common::TextParser<std::vector<FieldData>, ParseFunc>(
            *stream_,
            [this](const char* start, const char* end, std::vector<FieldData>& fds) {
                return parse_jsonline(start, end, fds);
            },
            block_size, n_threads, n_header_lines));
    }

    std::tuple<size_t, bool> parse_jsonline(const char* start, const char* end,
                                            std::vector<FieldData>& fds) {
        using namespace web;
        using namespace boost;
        size_t trim_count = 0;
        const char* original_starting = start;
        while (start < end && fma_common::TextParserUtils::IsTrimable(*start)) {
            start++;
            trim_count++;
        }
        if (start == end) {
            return std::tuple<size_t, bool>(trim_count, false);
        }

#define SKIP_OR_THROW(except)                                                           \
    if (forgiving_) {                                                                   \
        if (errors_++ < max_errors_) LOG_INFO() << except.what();                        \
        while (start < end && !fma_common::TextParserUtils::IsNewLine(*start)) start++; \
        while (start < end && fma_common::TextParserUtils::IsNewLine(*start)) start++;  \
        return std::tuple<size_t, bool>(start - original_starting, false);              \
    } else {                                                                            \
        std::throw_with_nested(except);                                                 \
    }

        // use stream parse to avoid memory copy
        iostreams::stream<iostreams::array_source> istr(start, end - start);
        std::error_code err_code;
        json::value json_obj = json::value::parse(istr, err_code);
        switch (err_code.value()) {
        case 0:
            break;
        case 1:
            {
                istr.unget();  // hack
                break;
            }
        default:
            {
                SKIP_OR_THROW(ParseJsonException(start, end, err_code.message()));
            }
        }
        using namespace lgraph::field_data_helper;
        try {
            for (size_t column = 0; column < field_specs_.size(); column++) {
                FieldSpec& field_spec = field_specs_[column];
                if (field_spec.name.empty()) {
                    continue;
                }
                if (json_obj.at(column).is_null() && field_spec.optional) {
                    fds.emplace_back();
                    continue;
                }
                FieldData fd;
                switch (field_spec.type) {
                case FieldType::NUL:
                    FMA_ASSERT(false);
                case FieldType::BOOL:
                    {
                        const auto& val = json_obj.at(column);
                        if (val.is_string()) {
                            const auto& str = ToStdString(val.as_string());
                            ParseStringIntoFieldData<FieldType::BOOL>(str.data(),
                                                                      str.data() + str.size(), fd);
                        } else {
                            fd = FieldData::Bool(val.as_bool());
                        }
                        break;
                    }
                case FieldType::INT8:
                    {
                        const auto& val = json_obj.at(column);
                        if (val.is_string()) {
                            const auto& str = ToStdString(val.as_string());
                            ParseStringIntoFieldData<FieldType::INT8>(str.data(),
                                                                      str.data() + str.size(), fd);
                        } else {
                            fd = FieldData::Int8(val.as_number().to_int32());
                        }
                        break;
                    }
                case FieldType::INT16:
                    {
                        const auto& val = json_obj.at(column);
                        if (val.is_string()) {
                            const auto& str = ToStdString(val.as_string());
                            ParseStringIntoFieldData<FieldType::INT16>(str.data(),
                                                                       str.data() + str.size(), fd);
                        } else {
                            fd = FieldData::Int16(val.as_number().to_int32());
                        }
                        break;
                    }
                case FieldType::INT32:
                    {
                        const auto& val = json_obj.at(column);
                        if (val.is_string()) {
                            const auto& str = ToStdString(val.as_string());
                            ParseStringIntoFieldData<FieldType::INT32>(str.data(),
                                                                       str.data() + str.size(), fd);
                        } else {
                            fd = FieldData::Int32(val.as_number().to_int32());
                        }
                        break;
                    }
                case FieldType::INT64:
                    {
                        const auto& val = json_obj.at(column);
                        if (val.is_string()) {
                            const auto& str = ToStdString(val.as_string());
                            ParseStringIntoFieldData<FieldType::INT64>(str.data(),
                                                                       str.data() + str.size(), fd);
                        } else {
                            fd = FieldData::Int64(val.as_number().to_int64());
                        }
                        break;
                    }
                case FieldType::FLOAT:
                    {
                        const auto& val = json_obj.at(column);
                        if (val.is_string()) {
                            const auto& str = ToStdString(val.as_string());
                            ParseStringIntoFieldData<FieldType::FLOAT>(str.data(),
                                                                       str.data() + str.size(), fd);
                        } else {
                            fd = FieldData::Float(static_cast<float>(val.as_double()));
                        }
                        break;
                    }
                case FieldType::DOUBLE:
                    {
                        const auto& val = json_obj.at(column);
                        if (val.is_string()) {
                            const auto& str = ToStdString(val.as_string());
                            ParseStringIntoFieldData<FieldType::DOUBLE>(
                                str.data(), str.data() + str.size(), fd);
                        } else {
                            fd = FieldData::Double(val.as_double());
                        }
                        break;
                    }
                case FieldType::DATE:
                    fd = FieldData::Date(ToStdString(json_obj.at(column).as_string()));
                    break;
                case FieldType::DATETIME:
                    fd = FieldData::DateTime(ToStdString(json_obj.at(column).as_string()));
                    break;
                case FieldType::STRING:
                    fd = FieldData::String(ToStdString(json_obj.at(column).as_string()));
                    break;
                case FieldType::BLOB:
                    fd = FieldData::Blob(ToStdString(json_obj.at(column).as_string()));
                    break;
                case FieldType::POINT:
                    // TODO(shw): Support import for point type;
                case FieldType::LINESTRING:
                    // TODO(shw): support import for linestring type;
                case FieldType::POLYGON:
                    // TODO(shw): support import for polygon type;
                case FieldType::SPATIAL:
                    // TODO(shw): support import for spatial type;
                    throw std::runtime_error("do not support spatial type now!");
                case FieldType::FLOAT_VECTOR:
                    throw std::runtime_error("do not support FLOAT_VECTOR type now!");
                }
                if (fd.is_null()) {
                    throw std::bad_cast();
                }
                fds.emplace_back(std::move(fd));
            }
        } catch (std::exception& e) {
            SKIP_OR_THROW(JsonReadException(start, end, e.what()));
        } catch (...) {
            SKIP_OR_THROW(JsonReadException(start, end, "Unknown exception"));
        }
        return std::tuple<size_t, bool>(static_cast<size_t>(istr.tellg()) + trim_count, true);
    }

    std::unique_ptr<fma_common::InputFileStream> stream_;
    std::vector<FieldSpec> field_specs_;
    std::unique_ptr<fma_common::TextParser<std::vector<FieldData>, ParseFunc>> parser_;
    bool forgiving_ = false;
    int64_t errors_ = 0;
    int64_t max_errors_ = 100;
#undef SKIP_OR_THROW
};

}  // namespace import_v2
}  // namespace lgraph
