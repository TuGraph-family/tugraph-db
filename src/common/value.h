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
#include <any>

#include "common/exceptions.h"
#include "common/temporal/temporal.h"
#include "common/type_traits.h"

// Don't change the order
enum class ValueType : char {
    Null = 0,
    BOOL,
    INTEGER,
    DOUBLE,
    STRING,
    ARRAY,
    MAP,
    DATE,
    LOCALDATETIME,
    FLOAT, // for vector index
    LOCALTIME,
};

struct Value {
    std::any data;
    ValueType type;

    Value() {
        type = ValueType::Null;
    }

    explicit Value(bool d) {
        type = ValueType::BOOL;
        data = d;
    }
    explicit Value(int64_t d) {
        type = ValueType::INTEGER;
        data = d;
    }
    explicit Value(double d) {
        type = ValueType::DOUBLE;
        data = d;
    }
    explicit Value(float d) {
        type = ValueType::FLOAT;
        data = d;
    }
    explicit Value(std::string d) {
        type = ValueType::STRING;
        data = std::move(d);
    }
    explicit Value(const char* d) {
        type = ValueType::STRING;
        data = std::string(d);
    }
    explicit Value(const std::vector<bool>& d) {
        std::vector<Value> tmp;
        tmp.reserve(d.size());
        for (auto item : d) {
            tmp.emplace_back(item);
        }
        type = ValueType::ARRAY;
        data = std::move(tmp);
    }
    explicit Value(const std::vector<int64_t>& d) {
        std::vector<Value> tmp;
        tmp.reserve(d.size());
        for (auto item : d) {
            tmp.emplace_back(item);
        }
        type = ValueType::ARRAY;
        data = std::move(tmp);
    }
    explicit Value(const std::vector<double>& d) {
        std::vector<Value> tmp;
        tmp.reserve(d.size());
        for (auto item : d) {
            tmp.emplace_back(item);
        }
        type = ValueType::ARRAY;
        data = std::move(tmp);
    }
    explicit Value(const std::vector<float>& d) {
        std::vector<Value> tmp;
        tmp.reserve(d.size());
        for (auto item : d) {
            tmp.emplace_back(item);
        }
        type = ValueType::ARRAY;
        data = std::move(tmp);
    }
    explicit Value(std::vector<std::string> d) {
        std::vector<Value> tmp;
        tmp.reserve(d.size());
        for (auto& item : d) {
            tmp.emplace_back(std::move(item));
        }
        type = ValueType::ARRAY;
        data = std::move(tmp);
    }
    explicit Value(std::vector<Value> d) {
        type = ValueType::ARRAY;
        data = std::move(d);
    }
    explicit Value(std::unordered_map<std::string, Value> d) {
        type = ValueType::MAP;
        data = std::move(d);
    }
    explicit Value(const common::Date& d) {
        type = ValueType::DATE;
        data = d;
    }
    explicit Value(const common::LocalDateTime& d) {
        type = ValueType::LOCALDATETIME;
        data = d;
    }
    explicit Value(const common::LocalTime& d) {
        type = ValueType::LOCALTIME;
        data = d;
    }

    [[nodiscard]]
    bool AsBool() const {
        if (LIKELY(type == ValueType::BOOL))
            return std::any_cast<bool>(data);
        THROW_CODE(UnknownError, "AsBool, but Value is not BOOL");
    }
    [[nodiscard]]
    int64_t AsInteger() const {
        if (LIKELY(type == ValueType::INTEGER))
            return std::any_cast<int64_t>(data);
        THROW_CODE(UnknownError, "AsInteger, but Value is not Integer");
    }
    [[nodiscard]]
    double AsDouble() const {
        if (LIKELY(type == ValueType::DOUBLE))
            return std::any_cast<double>(data);
        THROW_CODE(UnknownError, "AsDouble, but Value is not DOUBLE");
    }
    [[nodiscard]]
    float AsFloat() const {
        if (LIKELY(type == ValueType::FLOAT))
            return std::any_cast<float>(data);
        THROW_CODE(UnknownError, "AsFloat, but Value is not FLOAT");
    }
    [[nodiscard]]
    const std::string& AsString() const {
        if (LIKELY(type == ValueType::STRING))
            return std::any_cast<const std::string&>(data);
        THROW_CODE(UnknownError, "AsString, but Value is not STRING");
    }
    [[nodiscard]]
    std::string& AsString()  {
        if (LIKELY(type == ValueType::STRING))
            return std::any_cast<std::string&>(data);
        THROW_CODE(UnknownError, "AsString, but Value is not STRING");
    }

    [[nodiscard]]
    const std::vector<Value>& AsArray() const {
        if (LIKELY(type == ValueType::ARRAY))
            return std::any_cast<const std::vector<Value>&>(data);
        THROW_CODE(UnknownError, "AsArray, but Value is not ARRAY");
    }
    std::vector<Value>& AsArray() {
        if (LIKELY(type == ValueType::ARRAY))
            return std::any_cast<std::vector<Value>&>(data);
        THROW_CODE(UnknownError, "AsArray, but Value is not ARRAY");
    }
    [[nodiscard]]
    const std::unordered_map<std::string, Value>& AsMap() const {
        if (LIKELY(type == ValueType::MAP))
            return std::any_cast<const std::unordered_map<std::string, Value>&>(data);
        THROW_CODE(UnknownError, "AsMap, but Value is not MAP");
    }
    [[nodiscard]] const common::Date& AsDate() const {
        if (LIKELY(type == ValueType::DATE))
            return std::any_cast<const common::Date&>(data);
        THROW_CODE(UnknownError, "AsDate, but Value is not DATE");
    }
    [[nodiscard]] const common::LocalDateTime& AsLocalDateTime() const {
        if (LIKELY(type == ValueType::LOCALDATETIME))
            return std::any_cast<const common::LocalDateTime&>(data);
        THROW_CODE(UnknownError, "AsLocalDateTime, but Value is not LocalDateTime");
    }
    [[nodiscard]] const common::LocalTime& AsLocalTime() const {
        if (LIKELY(type == ValueType::LOCALTIME))
            return std::any_cast<const common::LocalTime&>(data);
        THROW_CODE(UnknownError, "AsLocalDateTime, but Value is not LocalTime");
    }

    static Value Bool(bool b) { return Value(b); }
    static Value Integer(int64_t b) { return Value(b); }
    static Value Double(double b) { return Value(b); }
    static Value Float(float b) { return Value(b); }
    static Value String(std::string b) { return Value(std::move(b)); }
    static Value BoolArray(const std::vector<bool>& b) { return Value(b); }
    static Value IntegerArray(const std::vector<int64_t>& b) { return Value(b); }
    static Value DoubleArray(const std::vector<double>& b) { return Value(b); }
    static Value StringArray(std::vector<std::string> b) { return Value(std::move(b)); }
    static Value Array(std::vector<Value> b) { return Value(std::move(b)); }
    static Value Map(std::unordered_map<std::string, Value> b) { return Value(std::move(b)); }
    static Value Date(const class common::Date& b) { return Value(b); }
    static Value LocalDateTime(const class common::LocalDateTime& b) { return Value(b); }
    static Value LocalTime(const class common::LocalTime& b) { return Value(b); }

    [[nodiscard]] bool IsNull() const {return type == ValueType::Null;}
    [[nodiscard]] bool IsBool() const {return type == ValueType::BOOL;}
    [[nodiscard]] bool IsInteger() const {return type == ValueType::INTEGER;}
    [[nodiscard]] bool IsString() const {return type == ValueType::STRING;}
    [[nodiscard]] bool IsDouble() const {return type == ValueType::DOUBLE;}
    [[nodiscard]] bool IsFloat() const {return type == ValueType::FLOAT;}
    [[nodiscard]] bool IsScalar() const {
        return type == ValueType::Null ||
               type == ValueType::BOOL ||
               type == ValueType::INTEGER ||
               type == ValueType::STRING ||
               type == ValueType::DOUBLE ||
               type == ValueType::FLOAT;
    }
    [[nodiscard]] bool IsArray() const {return type == ValueType::ARRAY;}
    [[nodiscard]] bool IsMap() const {return type == ValueType::MAP;}
    [[nodiscard]] bool IsDate() const {return type == ValueType::DATE;}
    [[nodiscard]] bool IsLocalDateTime() const {return type == ValueType::LOCALDATETIME;}
    [[nodiscard]] bool IsLocalTime() const {return type == ValueType::LOCALTIME;}

    [[nodiscard]] std::string ToString(bool str_quotation_mark = true) const;
    [[nodiscard]] std::any ToBolt() const;

    [[nodiscard]] std::string Serialize() const;
    void Deserialize(const char* p , size_t size);
    bool operator>(const Value& rhs) const;
    bool operator>=(const Value& rhs) const;
    bool operator<(const Value& rhs) const { return !(*this >= rhs); }
    bool operator<=(const Value& rhs) const { return !(*this > rhs); }
    bool operator==(const Value& b) const;
    bool operator!=(const Value& b) const {return !(operator==(b));}
};

std::string ToString(ValueType);

struct ValueHash {
    size_t operator()(const Value &fd) const {
        switch (fd.type) {
            case ValueType::BOOL:
                return std::hash<bool>()(fd.AsBool());
            case ValueType::INTEGER:
                return std::hash<int64_t>()(fd.AsInteger());
            case ValueType::DOUBLE:
                return std::hash<double>()(fd.AsDouble());
            case ValueType::STRING:
                return std::hash<std::string>()(fd.AsString());
            default:
                throw std::runtime_error("Unhandled data type for hash.");
        }
    }
};

template <>
struct fmt::formatter<std::vector<Value>> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const std::vector<Value>& vec, FormatContext& ctx) {
        auto out = ctx.out();
        out = fmt::format_to(out, "[");
        for (auto it = vec.begin(); it != vec.end(); ++it) {
            if (it != vec.begin()) {
                out = fmt::format_to(out, ", ");
            }
            out = fmt::format_to(out, "{}", it->ToString());
        }
        out = fmt::format_to(out, "]");
        return out;
    }
};

template <>
struct fmt::formatter<std::unordered_map<std::string, Value>> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const std::unordered_map<std::string, Value>& map, FormatContext& ctx) {
        auto out = ctx.out();
        out = fmt::format_to(out, "{{");
        for (auto it = map.begin(); it != map.end(); ++it) {
            if (it != map.begin()) {
                out = fmt::format_to(out, ", ");
            }
            out = fmt::format_to(out, R"({}:{})", it->first, it->second.ToString());

        }
        out = fmt::format_to(out, "}}");
        return out;
    }
};
