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

#include <exception>
#include <string>
#include "tools/lgraph_log.h"
#include "core/data_type.h"
#include "core/field_data_helper.h"

namespace lgraph {
namespace _detail {
inline std::string BinaryLine(const char* beg, const char* end) {
    std::string ret;
    for (const char* p = beg; p < end; p++) {
        ret.append(fma_common::ToString((int)*p));
        ret.push_back('(');
        if (fma_common::TextParserUtils::IsGraphical(*p)) {
            ret.push_back(*p);
        } else {
            ret.push_back(' ');
        }
        ret.push_back(')');
        ret.push_back(' ');
    }
    return ret;
}

inline std::string BinaryLine(const std::string& line) {
    return BinaryLine(line.data(), line.data() + line.size());
}

inline std::string DumpLine(const char* beg, const char* end) {
    const char* e = beg;
    while (e != end && *e != '\r' && *e != '\n') e++;
    return fma_common::StringFormatter::Format("\t> {}\nBinary form of the line is:\n\t> {}",
                                               std::string(beg, e), BinaryLine(beg, e));
}
}  // namespace _detail

class LineParserException : public std::exception {
 protected:
    std::string err_;
    std::string line_;

 public:
    LineParserException(const char* line_beg, const char* end) {
        const char* p = line_beg;
        while (p != end && !fma_common::TextParserUtils::IsNewLine(*p)) p++;
        line_ = std::string(line_beg, p);
    }

    explicit LineParserException(const std::string& line) : line_(line) {}

 protected:
    virtual void Print(int indent) const {
        LOG_INFO() << std::string(indent, ' ')
                   << "Due to: " << err_ << "\n>Error line:\n\t" << line_
                  << "\n>Binary format:\n\t" << _detail::BinaryLine(line_);
    }
};

class PrintLineException : public LineParserException {
    mutable std::string msg_;

 public:
    PrintLineException(const char* beg, const char* end) : LineParserException(beg, end) {}

    const char* what() const noexcept override {
        msg_ = err_ + "\n>Error line:\n\t" + line_ + "\n>Binary format:\n\t" +
               _detail::BinaryLine(line_);
        return msg_.c_str();
    }
};

class FailToSkipColumnException : public PrintLineException {
 public:
    FailToSkipColumnException(const char* line_beg, const char* end, size_t column,
                              const std::string& msg = "")
        : PrintLineException(line_beg, end) {
        err_ = fma_common::StringFormatter::Format("Failed to skip column {}: {}", column, msg);
    }
};

class FailToParseColumnException : public PrintLineException {
 public:
    FailToParseColumnException(const char* line_beg, const char* error_beg, const char* end,
                               size_t column, FieldType type, const std::string& msg = "")
        : PrintLineException(line_beg, end) {
        err_ = fma_common::StringFormatter::Format(
            "Failed to parse column {} into type {}: {}\nThe error item is ", column,
            field_data_helper::FieldTypeName(type), msg);
        int cnt = 0;
        while (error_beg < end && *error_beg != '\n' && cnt < 30) {
            err_ += *error_beg++;
            ++cnt;
        }
        if (error_beg < end && *error_beg != '\n') err_ += "......";
    }
};

class MissingDelimiterException : public PrintLineException {
 public:
    MissingDelimiterException(const char* line_beg, const char* end, size_t column)
        : PrintLineException(line_beg, end) {
        err_ = fma_common::StringFormatter::Format("Missing delimiter after column {}", column);
    }
};

class ParseJsonException : public PrintLineException {
 public:
    ParseJsonException(const char* line_beg, const char* end, const std::string& msg)
        : PrintLineException(line_beg, end) {
        err_ = "json parsing failed, error msg : " + msg;
    }
};

class JsonReadException : public PrintLineException {
 public:
    JsonReadException(const char* line_beg, const char* end, const std::string& msg)
        : PrintLineException(line_beg, end) {
        err_ = "json reading failed, error msg : " + msg;
    }
};

}  // namespace lgraph
