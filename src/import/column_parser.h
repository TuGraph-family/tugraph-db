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

#include "fma-common/text_parser.h"

#include "core/data_type.h"
#include "core/field_data_helper.h"
#include "import/block_parser.h"
#include "import/import_config_parser.h"
#include "restful/server/json_convert.h"
#include "tools/json.hpp"

namespace lgraph {
namespace import_v2 {

/** Parse each line of a csv into a vector of FieldData, excluding SKIP columns.
 *  vector<ColumnSpec> specifies what each column contains.
 */
class ColumnParser : public BlockParser {
 protected:
    typedef std::function<std::tuple<size_t, bool>(const char*, const char*,
                                                   std::vector<FieldData>&)>
        ParseFunc;

    std::unique_ptr<fma_common::TextParser<std::vector<FieldData>, ParseFunc>> parser_;
    std::vector<FieldSpec> field_specs_;
    fma_common::InputFileStream* stream_{};
    bool own_stream_ = false;
    bool forgiving_ = false;
    std::string delimiter_;
    int64_t errors_ = 0;
    int64_t max_errors_ = 100;

 public:
    /**
     * Constructor
     *
     * @exception std::runtime_error    Raised when a runtime error condition
     * occurs.
     *
     * @param path              Full pathname of the file.
     * @param column_specs      The column specs.
     * @param block_size        Size of text block size. Each text block is
     * processed separately.
     * @param n_threads         Number of parsing threads.
     * @param n_header_lines    Number of header lines.
     */
    ColumnParser(const std::string& path, const std::vector<FieldSpec>& field_specs,
                 size_t block_size, size_t n_threads, size_t n_header_lines, bool forgiving,
                 const std::string& delimiter, int64_t max_err_msgs = 100) {
        std::unique_ptr<fma_common::InputFmaStream> stream(new fma_common::InputFmaStream(path));
        if (!stream->Good()) {
            FMA_LOG() << "Failed to open input file " << path;
            throw std::runtime_error("failed to open input file [" + path + "]");
        }
        own_stream_ = true;
        forgiving_ = forgiving;
        delimiter_ = delimiter;
        max_errors_ = max_err_msgs;
        Open(stream.release(), field_specs, block_size, n_threads, n_header_lines);
    }

    ColumnParser(fma_common::InputFileStream* file, const std::vector<FieldSpec>& field_specs,
                 size_t block_size, size_t n_threads, size_t n_header_lines, bool forgiving,
                 const std::string& delimiter, int64_t max_err_msgs = 100) {
        own_stream_ = false;
        forgiving_ = forgiving;
        delimiter_ = delimiter;
        max_errors_ = max_err_msgs;
        Open(file, field_specs, block_size, n_threads, n_header_lines);
    }

    ~ColumnParser() { Stop(); }

    /**
     * Reads a block of vector<FieldData>
     *
     * @exception std::runtime_error    Raised when ParseOneLine() fails to parse
     * a line
     *
     * @param [in,out] buf  The buffer.
     *
     * @return  True if it succeeds, false if it fails.
     */
    bool ReadBlock(std::vector<std::vector<FieldData>>& buf) { return parser_->ReadBlock(buf); }

    /** Stops the parser, even if the parsing is not complete */
    void Stop() {
        if (parser_) {
            parser_->Stop();
            parser_.reset();
        }
        if (own_stream_) {
            delete stream_;
            stream_ = nullptr;
            own_stream_ = false;
        }
    }

    /** Wait until the parsing completes */
    void WaitEmpty() {
        if (parser_) parser_->WaitEmpty();
    }

 protected:
    // just for testing ParseOneLine
    explicit ColumnParser(int64_t max_err_msgs) { max_errors_ = max_err_msgs; }

    void Open(fma_common::InputFileStream* file, const std::vector<FieldSpec>& field_specs,
              size_t block_size, size_t n_threads, size_t n_header_lines) {
        CheckDelimAndResetErrors();
        field_specs_ = field_specs;
        stream_ = file;
        parser_.reset(new fma_common::TextParser<std::vector<FieldData>, ParseFunc>(
            *stream_, GetParseFunc(), block_size, n_threads, n_header_lines));
    }

    void CheckDelimAndResetErrors() {
        if (delimiter_.empty()) {
            throw lgraph::InputError("Delimiter cannot be empty.");
        }
        if (delimiter_.find_first_of("\r\n") != delimiter_.npos) {
            throw lgraph::InputError("Delimiter cannot contain \\r or \\n.");
        }
        errors_ = 0;
    }

    std::function<std::tuple<size_t, bool>(const char*, const char*, std::vector<FieldData>&)>
    GetParseFunc() {
        if (delimiter_.size() == 1) {
            char delim = delimiter_[0];
            return [this, delim](const char* b, const char* e, std::vector<FieldData>& r) {
                return ParseOneLineWithCharDelim(b, e, r, delim);
            };
        } else if (delimiter_.size() == 2) {
            char delim1 = delimiter_[0];
            char delim2 = delimiter_[1];
            return [this, delim1, delim2](const char* b, const char* e, std::vector<FieldData>& r) {
                return ParseOneLineWithTwoCharDelim(b, e, r, delim1, delim2);
            };
        } else {
            return [this](const char* b, const char* e, std::vector<FieldData>& r) {
                return ParseOneLineWithStringDelim(b, e, r, delimiter_);
            };
        }
    }

    std::tuple<size_t, bool> ParseOneLineWithCharDelim(const char* beg, const char* end,
                                                       std::vector<FieldData>& values, char delim) {
        return ParseOneLine(
            beg, end, values,
            [delim](const char* p, const char* end) {
                while (p < end && *p != delim && fma_common::TextParserUtils::IsTrimable(*p)) p++;
                return p;
            },  // trim
            [delim](const char* p, const char* end) {
                if (p < end) {
                    return *p == delim ? p + 1 : p;
                } else {
                    return p;
                }
            },  // detect and skip delimiter
            [delim](const char* p, const char* end) {
                if (p != end && *p == '"') return SkipQuotedString(p, end);
                while (p < end && *p != delim && !fma_common::TextParserUtils::IsNewLine(*p)) p++;
                return p;
            },  // drop one field
            [this, delim](const char* p, const char* end, std::string& data) -> size_t {
                if (p != end && *p == '"')
                    return fma_common::TextParserUtils::ParseQuotedString(p, end, data);
                const char* orig = p;
                while (p < end && !fma_common::TextParserUtils::IsNewLine(*p) && *p != delim) p++;
                // trim right
                const char* rhs = p - 1;
                while (rhs > orig && fma_common::TextParserUtils::IsTrimable(*rhs)) rhs--;
                data.assign(orig, rhs + 1);
                return p - orig;
            });  // parse one string
    }

    std::tuple<size_t, bool> ParseOneLineWithTwoCharDelim(const char* beg, const char* end,
                                                          std::vector<FieldData>& values,
                                                          char delim1, char delim2) {
        return ParseOneLine(
            beg, end, values,
            [delim1, delim2](const char* p, const char* end) {
                while (p < end) {
                    if (*p == delim1 && p + 1 < end && p[1] == delim2) {
                        return p;
                    } else {
                        if (fma_common::TextParserUtils::IsTrimable(*p))
                            p++;
                        else
                            break;
                    }
                }
                return p;
            },  // trim
            [delim1, delim2](const char* p, const char* end) {
                if (p < end && *p == delim1 && p + 1 < end && p[1] == delim2) return p + 2;
                return p;
            },  // detect and skip delimiter
            [delim1, delim2](const char* p, const char* end) {
                if (p != end && *p == '"') return SkipQuotedString(p, end);
                while (p < end) {
                    if (fma_common::TextParserUtils::IsNewLine(*p) ||
                        (*p == delim1 && p + 1 < end && p[1] == delim2))
                        break;
                    p++;
                }
                return p;
            },  // drop one field
            [delim1, delim2](const char* p, const char* end, std::string& data) -> size_t {
                if (p != end && *p == '"')
                    return fma_common::TextParserUtils::ParseQuotedString(p, end, data);
                const char* orig = p;
                while (p < end) {
                    if (fma_common::TextParserUtils::IsNewLine(*p) ||
                        (*p == delim1 && p + 1 < end && p[1] == delim2))
                        break;
                    p++;
                }
                data.assign(orig, p);
                return p - orig;
            });  // parse one string
    }

    std::tuple<size_t, bool> ParseOneLineWithStringDelim(const char* beg, const char* end,
                                                         std::vector<FieldData>& values,
                                                         const std::string& delim) {
        char delim0 = delim[0];
        return ParseOneLine(
            beg, end, values,
            [delim, delim0](const char* p, const char* end) {
                while (p < end) {
                    if (*p == delim0) {
                        if (strncmp(p, delim.data(), std::min<size_t>(end - p, delim.size())) == 0)
                            break;
                        else
                            p++;
                    } else {
                        if (fma_common::TextParserUtils::IsTrimable(*p))
                            p++;
                        else
                            break;
                    }
                }
                return p;
            },  // trim
            [delim, delim0](const char* p, const char* end) {
                if (strncmp(p, delim.data(), std::min<size_t>(end - p, delim.size())) == 0)
                    return p + delim.size();
                else
                    return p;
            },  // detect and skip delimiter
            [delim, delim0](const char* p, const char* end) {
                if (p != end && *p == '"') return SkipQuotedString(p, end);
                while (p < end) {
                    if (fma_common::TextParserUtils::IsNewLine(*p) ||
                        (*p == delim0 &&
                         strncmp(p, delim.data(), std::min<size_t>(end - p, delim.size())) == 0))
                        break;
                    p++;
                }
                return p;
            },  // drop one field
            [this, delim, delim0](const char* p, const char* end, std::string& data) -> size_t {
                if (p != end && *p == '"')
                    return fma_common::TextParserUtils::ParseQuotedString(p, end, data);
                const char* orig = p;
                while (p < end) {
                    if (fma_common::TextParserUtils::IsNewLine(*p) ||
                        (*p == delim0 &&
                         strncmp(p, delim.data(), std::min<size_t>(end - p, delim.size())) == 0))
                        break;
                    p++;
                }
                data.assign(orig, p);
                return p - orig;
            });  // parse one string
    }

    static inline const char* SkipQuotedString(const char* b, const char* e) {
        assert(*b == '"');
        // quoted
        b++;
        while (b != e && !fma_common::TextParserUtils::IsNewLine(*b)) {
            if (*b == '"') {
                b++;
                if (b == e || *b != '"') {
                    break;
                } else {
                    // *b == '"'
                    b++;
                }
            } else {
                b++;
            }
        }
        return b;
    }

    /**
     * Parse one CSV line into a vector of FieldData
     *
     * @exception FailToSkipColumnException     Thrown when a Fail To Skip Column
     * error condition occurs.
     * @exception FailToParseColumnException    Thrown when a Fail To Parse Column
     * error condition occurs.
     * @exception MissingDelimiterException     Thrown when a Missing Delimiter
     * error condition occurs.
     *
     * @param          beg      The beg.
     * @param          end      The end.
     * @param [in,out] values   The values.
     *
     * @return  A size_t indicating the number of characters parsed.
     */
    template <typename TrimFuncT, typename DetectAndSkipDelimT, typename DropOneFieldT,
              typename ParseOneStringT>
    std::tuple<size_t, bool> ParseOneLine(const char* beg, const char* end,
                                          std::vector<FieldData>& values, const TrimFuncT& trim,
                                          const DetectAndSkipDelimT& detect_and_skip_delim,
                                          const DropOneFieldT& drop_one_field,
                                          const ParseOneStringT& parse_one_string) {
        values.clear();
        const char* p = beg;
        bool success = true;
#define WARN_OR_THROW(except)                                    \
    if (forgiving_) {                                            \
        if (errors_++ < max_errors_) FMA_LOG() << except.what(); \
        success = false;                                         \
        break;                                                   \
    } else {                                                     \
        throw except;                                            \
    }

        for (size_t column = 0; column < field_specs_.size() && success; column++) {
            auto& field_spec = field_specs_[column];
            if (field_spec.name.empty()) {
                p = drop_one_field(p, end);
            } else {
                p = trim(p, end);
                FieldData fd;
                size_t s = ::lgraph::field_data_helper::ParseStringIntoFieldData(
                    field_spec.type, p, end, fd, parse_one_string);
                if (s) {
                    p += s;
                    values.emplace_back(std::move(fd));
                } else {
                    if (field_spec.optional) {
                        values.emplace_back();
                    } else {
                        WARN_OR_THROW(
                            FailToParseColumnException(beg, p, end, column, field_spec.type, ""));
                    }
                }
            }
            p = trim(p, end);
            if (column != field_specs_.size() - 1) {
                // in the middle, check for delimiter
                const char* newp = detect_and_skip_delim(p, end);
                if (newp == p) {
                    WARN_OR_THROW(MissingDelimiterException(beg, end, column));
                }
                p = newp;
            }
        }
#define WARN_OR_THROW2(except)                                                \
    if (forgiving_) {                                                         \
        if (errors_++ < max_errors_) FMA_LOG() << except.what();              \
        if (errors_ == max_errors_) FMA_LOG() << "ignore more error message"; \
        success = false;                                                      \
    } else {                                                                  \
        throw except;                                                         \
    }

        if (p != end && !fma_common::TextParserUtils::IsNewLine(*p)) {
            // check for unclean parsing
            if (field_specs_.empty() || field_specs_.back().name.empty()) {
                WARN_OR_THROW2(FailToSkipColumnException(beg, end, field_specs_.size() - 1,
                                                         "there is more content than we expect"));
            } else {
                WARN_OR_THROW2(FailToParseColumnException(beg, p, end, field_specs_.size() - 1,
                                                          field_specs_.back().type,
                                                          "there is more content than we expect"));
            }
        }
        if (!success)
            while (p < end && !fma_common::TextParserUtils::IsNewLine(*p)) p++;
        while (p < end && fma_common::TextParserUtils::IsNewLine(*p)) p++;
        return std::tuple<size_t, bool>(p - beg, success);
    }
};

}  // namespace import_v2
}  // namespace lgraph
