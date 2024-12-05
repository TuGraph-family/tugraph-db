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

#include "value.h"
#include <cassert>
#include "bolt/temporal.h"

std::string Value::Serialize() const {
    std::string buffer;
    buffer.append(1, static_cast<char>(type));
    switch (type) {
        case ValueType::BOOL: {
            buffer.append(1, static_cast<char>(std::any_cast<bool>(data)));
            break;
        }
        case ValueType::INTEGER: {
            auto v = std::any_cast<int64_t>(data);
            buffer.append((const char*)&v, sizeof(v));
            break;
        }
        case ValueType::DOUBLE: {
            auto v = std::any_cast<double>(data);
            buffer.append((const char*)&v, sizeof(v));
            break;
        }
        case ValueType::FLOAT: {
            auto v = std::any_cast<float>(data);
            buffer.append((const char*)&v, sizeof(v));
            break;
        }
        case ValueType::STRING: {
            buffer.append(std::any_cast<const std::string&>(data));
            break;
        }
        case ValueType::ARRAY: {
            const auto& array = std::any_cast<const std::vector<Value>&>(data);
            if (array.empty()) {
                break;
            }
            auto t = array[0].type;
            buffer.append(1, static_cast<char>(t));
            void* p = nullptr;
            for (const auto& item : array) {
                if (item.type != t) {
                    THROW_CODE(ValueException, "Array elements must have the same type for serializing, error type: " + ::ToString(item.type));
                }
                switch (item.type) {
                    case ValueType::BOOL: {
                        if (!p) {
                            buffer.resize(buffer.size() + array.size());
                            p = buffer.data() + 2;
                        }
                        auto* tmp = (char*)p;
                        *tmp = static_cast<char>(item.AsBool());
                        tmp++;
                        p = tmp;
                        break;
                    }
                    case ValueType::INTEGER: {
                        if (!p) {
                            buffer.resize(buffer.size() +
                                          array.size() * sizeof(int64_t));
                            p = buffer.data() + 2;
                        }
                        auto* tmp = (int64_t *)p;
                        *tmp = item.AsInteger();
                        tmp++;
                        p = tmp;
                        break;
                    }
                    case ValueType::DOUBLE: {
                        if (!p) {
                            buffer.resize(buffer.size() +
                                          array.size() * sizeof(double));
                            p = buffer.data() + 2;
                        }
                        auto* tmp = (double *)p;
                        *tmp = item.AsDouble();
                        tmp++;
                        p = tmp;
                        break;
                    }
                    case ValueType::FLOAT: {
                        if (!p) {
                            buffer.resize(buffer.size() +
                                          array.size() * sizeof(float));
                            p = buffer.data() + 2;
                        }
                        auto* tmp = (float *)p;
                        *tmp = item.AsFloat();
                        tmp++;
                        p = tmp;
                        break;
                    }
                    case ValueType::STRING: {
                        const auto& s = item.AsString();
                        size_t len = s.size();
                        buffer.append((const char*)&len, sizeof(len));
                        buffer.append(s);
                        break;
                    }
                    default: {
                        THROW_CODE(ValueException, "Unsupported data type for serializing array, type: " + ::ToString(item.type));
                    }
                }
            }
            break;
        }
        case ValueType::Null: {
            break;
        }
        case ValueType::DATE: {
            auto& v = std::any_cast<const common::Date&>(data);
            int64_t days = v.GetStorage();
            buffer.append((const char*)&days, sizeof(days));
            break;
        }
        case ValueType::LOCALDATETIME: {
            auto& v = std::any_cast<const common::LocalDateTime&>(data);
            int64_t microseconds = v.GetStorage();
            buffer.append((const char*)&microseconds, sizeof(microseconds));
            break;
        }
        case ValueType::LOCALTIME: {
            auto& v = std::any_cast<const common::LocalTime&>(data);
            int64_t microseconds = v.GetStorage();
            buffer.append((const char*)&microseconds, sizeof(microseconds));
            break;
        }
        case ValueType::TIME: {
            auto& v = std::any_cast<const common::Time&>(data);
            int64_t nanosecond = std::get<0>(v.GetStorage());
            int64_t offset = std::get<1>(v.GetStorage());
            buffer.append((const char*)&nanosecond, sizeof(nanosecond));
            buffer.append((const char*)&offset, sizeof(offset));
            break;
        }
        case ValueType::DATETIME: {
            auto& v = std::any_cast<const common::DateTime&>(data);
            int64_t nanosecond = std::get<0>(v.GetStorage());
            int64_t offset = std::get<1>(v.GetStorage());
            buffer.append((const char*)&nanosecond, sizeof(nanosecond));
            buffer.append((const char*)&offset, sizeof(offset));
            break;
        }
        case ValueType::DURATION: {
            auto& v = std::any_cast<const common::Duration&>(data);
            int64_t months = v.months;
            int64_t days = v.days;
            int64_t seconds = v.seconds;
            int64_t nanosecond = v.nanos;
            buffer.append((const char*)&months, sizeof(months));
            buffer.append((const char*)&days, sizeof(days));
            buffer.append((const char*)&seconds, sizeof(seconds));
            buffer.append((const char*)&nanosecond, sizeof(nanosecond));
            break;
        }
        default: {
            THROW_CODE(ValueException, "Unsupported data type for value serializing, type: " + ::ToString(type));
        }
    }
    return buffer;
}

void Value::Deserialize(const char* p, size_t size) {
    type = static_cast<ValueType>(*p);
    p++;
    size--;
    switch (type) {
        case ValueType::BOOL: {
            assert(size == sizeof(bool));
            data = (bool)*p;
            break;
        }
        case ValueType::INTEGER: {
            assert(size == sizeof(int64_t));
            data = *(int64_t*)p;
            break;
        }
        case ValueType::DOUBLE: {
            assert(size == sizeof(double));
            data = *(double*)p;
            break;
        }
        case ValueType::FLOAT: {
            assert(size == sizeof(float));
            data = *(float*)p;
            break;
        }
        case ValueType::STRING: {
            data = std::string(p, size);
            break;
        }
        case ValueType::ARRAY: {
            data = std::vector<Value>{};
            if (size == 0) {
                break;
            }
            auto t = static_cast<ValueType>(*p);
            p++;
            size--;
            while (size > 0) {
                switch (t) {
                    case ValueType::BOOL: {
                        std::any_cast<std::vector<Value>&>(data).emplace_back((bool)*p);
                        p += 1;
                        size -= 1;
                        break;
                    }
                    case ValueType::INTEGER: {
                        if (!data.has_value()) {
                            std::vector<int64_t> ret;
                            data = std::move(ret);
                        }
                        std::any_cast<std::vector<Value>&>(data).emplace_back(*(int64_t*)p);
                        p += sizeof(int64_t);
                        size -= sizeof(int64_t);
                        break;
                    }
                    case ValueType::DOUBLE: {
                        if (!data.has_value()) {
                            std::vector<double> ret;
                            data = std::move(ret);
                        }
                        std::any_cast<std::vector<Value>&>(data).emplace_back(*(double *)p);
                        p += sizeof(double);
                        size -= sizeof(double);
                        break;
                    }
                    case ValueType::FLOAT: {
                        if (!data.has_value()) {
                            std::vector<float> ret;
                            data = std::move(ret);
                        }
                        std::any_cast<std::vector<Value>&>(data).emplace_back(*(float *)p);
                        p += sizeof(float);
                        size -= sizeof(float);
                        break;
                    }
                    case ValueType::STRING: {
                        if (!data.has_value()) {
                            std::vector<std::string> ret;
                            data = std::move(ret);
                        }
                        auto len = *(size_t*)p;
                        p += sizeof(size_t);
                        size -= sizeof(size_t);
                        std::any_cast<std::vector<Value>&>(data).emplace_back(std::string(p, len));
                        p += len;
                        size -= len;
                        break;
                    }
                    default: {
                        THROW_CODE(ValueException, "unexpected data type for Deserialize : " + ::ToString(t));
                    }
                }
            }
            break;
        }
        case ValueType::Null: {
            break;
        }
        case ValueType::DATE: {
            assert(size == sizeof(int64_t));
            data = common::Date(*(int64_t*)p);
            break;
        }
        case ValueType::LOCALDATETIME: {
            assert(size == sizeof(int64_t));
            data = common::LocalDateTime(*(int64_t*)p);
            break;
        }
        case ValueType::LOCALTIME: {
            assert(size == sizeof(int64_t));
            data = common::LocalTime(*(int64_t*)p);
            break;
        }
        case ValueType::TIME: {
            assert(size == sizeof(int64_t) * 2);
            data = common::Time(*(int64_t*)p, *(int64_t*)(p+8));
            break;
        }
        case ValueType::DATETIME: {
            assert(size == sizeof(int64_t) * 2);
            data = common::DateTime(*(int64_t*)p, *(int64_t*)(p+8));
            break;
        }
        case ValueType::DURATION: {
            assert(size == sizeof(int64_t) * 4);
            data = common::Duration(*(int64_t*)p, *(int64_t*)(p+8), *(int64_t*)(p+16), *(int64_t*)(p+24));
            break;
        }
        default: {
            THROW_CODE(ValueException, "unexpected data type for Deserialize : " + ::ToString(type));
        }
    }
}

std::string ToString(ValueType t) {
    switch (t) {
        case ValueType::Null: return "Null";
        case ValueType::BOOL: return "BOOL";
        case ValueType::INTEGER: return "INTEGER";
        case ValueType::DOUBLE: return "DOUBLE";
        case ValueType::FLOAT: return "FLOAT";
        case ValueType::STRING: return "STRING";
        case ValueType::ARRAY : return "ARRAY";
        case ValueType::MAP : return "MAP";
        case ValueType::DATE : return "DATE";
        case ValueType::DATETIME:
            return "DATETIME";
        case ValueType::LOCALDATETIME : return "LOCALDATETIME";
        case ValueType::LOCALTIME : return "LOCALTIME";
        case ValueType::TIME : return "TIME";
        case ValueType::DURATION : return "DURATION";
        default: THROW_CODE(ValueException, "unexpected ValueType for Type ToString");
    }
}

std::string Value::ToString(bool str_quotation_mark) const {
    switch (type) {
        case ValueType::Null:
            return "null";
        case ValueType::BOOL:
            return std::any_cast<bool>(data) ? "true" : "false";
        case ValueType::INTEGER:
            return std::to_string(std::any_cast<int64_t>(data));
        case ValueType::DOUBLE:
            return std::to_string(std::any_cast<double>(data));
        case ValueType::FLOAT:
            return std::to_string(std::any_cast<float>(data));
        case ValueType::STRING: {
            const auto& d = std::any_cast<const std::string &>(data);
            if (str_quotation_mark) {
                return fmt::format("\"{}\"", d);
            } else {
                return d;
            }
        }
        case ValueType::ARRAY: {
            const auto& d = std::any_cast<const std::vector<Value>&>(data);
            return fmt::format("{}", d);
        }
        case ValueType::MAP: {
            const auto& d = std::any_cast<const std::unordered_map<std::string, Value>&>(data);
            return fmt::format("{}", d);
        }
        case ValueType::DATE: {
            return std::any_cast<const common::Date&>(data).ToString();
        }
        case ValueType::DATETIME: {
            return std::any_cast<const common::DateTime&>(data).ToString();
        }
        case ValueType::LOCALDATETIME: {
            return std::any_cast<const common::LocalDateTime&>(data).ToString();
        }
        case ValueType::LOCALTIME: {
            return std::any_cast<const common::LocalTime&>(data).ToString();
        }
        case ValueType::TIME: {
            return std::any_cast<const common::Time&>(data).ToString();
        }
        case ValueType::DURATION: {
            return std::any_cast<const common::Duration&>(data).ToString();
        }
        default: THROW_CODE(ValueException, "unexpected ValueType for Value ToString");
    }
}

std::any Value::ToBolt() const {
    switch (type) {
        case ValueType::Null:
        case ValueType::BOOL:
        case ValueType::INTEGER:
        case ValueType::DOUBLE:
        case ValueType::FLOAT:
        case ValueType::STRING: {
            return data;
        }
        case ValueType::ARRAY: {
            std::vector<std::any> ret;
            for (const auto& v : std::any_cast<const std::vector<Value>&>(data)) {
                ret.push_back(v.ToBolt());
            }
            return ret;
        }
        case ValueType::MAP: {
            std::unordered_map<std::string, std::any> ret;
            for (auto& [k, v] : std::any_cast<const std::unordered_map<std::string, Value>&>(data)) {
                ret.emplace(k, v.ToBolt());
            }
            return ret;
        }
        case ValueType::DATE: {
            return bolt::Date{std::any_cast<const common::Date&>(data).GetStorage()};
        }
        case ValueType::DATETIME: {
            auto s = std::any_cast<const common::DateTime&>(data);
            if (std::get<0>(s.GetStorage()) % 1000000000 >= 0) {
                return bolt::DateTime{std::get<0>(s.GetStorage()) / 1000000000,
                                      std::get<0>(s.GetStorage()) % 1000000000,
                                      std::get<1>(s.GetStorage())};
            } else {
                return bolt::DateTime{std::get<0>(s.GetStorage()) / 1000000000 - 1,
                                      std::get<0>(s.GetStorage()) % 1000000000 + 1000000000,
                                      std::get<1>(s.GetStorage())};
            }
        }
        case ValueType::LOCALDATETIME: {
            auto s = std::any_cast<const common::LocalDateTime&>(data).GetStorage();
            if (s % 1000000000 >= 0) {
                return bolt::LocalDateTime{s / 1000000000, s % 1000000000};
            } else {
                return bolt::LocalDateTime{s / 1000000000 - 1, s % 1000000000 + 1000000000};
            }
        }
        case ValueType::LOCALTIME: {
            return bolt::LocalTime{std::any_cast<const common::LocalTime&>(data).GetStorage()};
        }
        case ValueType::TIME: {
            auto storage = std::any_cast<const common::Time&>(data).GetStorage();
            return bolt::Time{std::get<0>(storage), std::get<1>(storage)};
        }
        case ValueType::DURATION: {
            return bolt::Duration{std::any_cast<const common::Duration&>(data).months,
                                  std::any_cast<const common::Duration&>(data).days,
                                  std::any_cast<const common::Duration&>(data).seconds,
                                  std::any_cast<const common::Duration&>(data).nanos};
        }
        default: THROW_CODE(ValueException, "unexpected ValueType for Value ToString");
    }
}

bool Value::operator>(const Value& rhs) const {
    if (type == ValueType::Null) return false;
    if (rhs.type == ValueType::Null) return true;
    if (type == ValueType::ARRAY || type == ValueType::MAP) {
        THROW_CODE(ValueException, "Unhandled data type");
    }
    try {
        switch (type) {
            case ValueType::BOOL:
                return std::any_cast<bool>(data) > std::any_cast<bool>(rhs.data);
            case ValueType::INTEGER:
                if (rhs.type == ValueType::DOUBLE) {
                    return std::any_cast<int64_t>(data) > std::any_cast<double>(rhs.data);
                } else if (rhs.type == ValueType::FLOAT) {
                    return std::any_cast<int64_t>(data) > std::any_cast<float>(rhs.data);
                } else {
                    return std::any_cast<int64_t>(data) > std::any_cast<int64_t>(rhs.data);
                }
            case ValueType::DOUBLE:
                if (rhs.type == ValueType::INTEGER) {
                    return std::any_cast<double>(data) > std::any_cast<int64_t>(rhs.data);
                } else if (rhs.type == ValueType::FLOAT){
                    return std::any_cast<double>(data) > std::any_cast<float>(rhs.data);
                } else {
                    return std::any_cast<double>(data) > std::any_cast<double>(rhs.data);
                }
            case ValueType::FLOAT:
                if (rhs.type == ValueType::INTEGER) {
                    return std::any_cast<float>(data) > std::any_cast<int64_t>(rhs.data);
                } else if (rhs.type == ValueType::DOUBLE){
                    return std::any_cast<float>(data) > std::any_cast<double>(rhs.data);
                } else {
                    return std::any_cast<float>(data) > std::any_cast<float>(rhs.data);
                }
            case ValueType::STRING:
                return std::any_cast<const std::string&>(data) > std::any_cast<const std::string&>(rhs.data);
            case ValueType::DATE:
                return std::any_cast<const common::Date>(data) > std::any_cast<const common::Date&>(rhs.data);
            case ValueType::DATETIME:
                return std::any_cast<const common::DateTime>(data) >
                       std::any_cast<const common::DateTime&>(rhs.data);
            case ValueType::LOCALDATETIME:
                return std::any_cast<const common::LocalDateTime>(data) > std::any_cast<const common::LocalDateTime&>(rhs.data);
            case ValueType::LOCALTIME:
                return std::any_cast<const common::LocalTime>(data) > std::any_cast<const common::LocalTime&>(rhs.data);
            case ValueType::TIME:
                return std::any_cast<const common::Time>(data) > std::any_cast<const common::Time&>(rhs.data);
            case ValueType::DURATION:
                return std::any_cast<const common::Duration>(data) > std::any_cast<const common::Duration&>(rhs.data);
            default:
                THROW_CODE(ValueException, "Unhandled data type");
        }
    } catch (std::exception &e) {
        THROW_CODE(ValueException, "Unable to compare two Value with different types. " + ::ToString(type) + " vs " + ::ToString(rhs.type));
    }
}

bool Value::operator>=(const Value& rhs) const {
    if (rhs.type == ValueType::Null) return true;
    if (type == ValueType::Null) return false;
    if (type == ValueType::ARRAY || type == ValueType::MAP) {
        THROW_CODE(ValueException, "Unhandled data type");
    }
    try {
        switch (type) {
            case ValueType::BOOL:
                return std::any_cast<bool>(data) >= std::any_cast<bool>(rhs.data);
            case ValueType::INTEGER:
                if (rhs.type == ValueType::DOUBLE) {
                    return std::any_cast<int64_t>(data) >= std::any_cast<double>(rhs.data);
                } else if (rhs.type == ValueType::FLOAT) {
                    return std::any_cast<int64_t>(data) >= std::any_cast<float>(rhs.data);
                } else {
                    return std::any_cast<int64_t>(data) >= std::any_cast<int64_t>(rhs.data);
                }
            case ValueType::DOUBLE:
                if (rhs.type == ValueType::INTEGER) {
                    return std::any_cast<double>(data) >= std::any_cast<int64_t>(rhs.data);
                } else if (rhs.type == ValueType::FLOAT){
                    return std::any_cast<double>(data) >= std::any_cast<float>(rhs.data);
                } else {
                    return std::any_cast<double>(data) >= std::any_cast<double>(rhs.data);
                }
            case ValueType::FLOAT:
                if (rhs.type == ValueType::INTEGER) {
                    return std::any_cast<float>(data) >= std::any_cast<int64_t>(rhs.data);
                } else if (rhs.type == ValueType::DOUBLE){
                    return std::any_cast<float>(data) >= std::any_cast<double>(rhs.data);
                } else {
                    return std::any_cast<float>(data) >= std::any_cast<float>(rhs.data);
                }
            case ValueType::STRING:
                return std::any_cast<const std::string&>(data) >= std::any_cast<const std::string&>(rhs.data);
            case ValueType::DATE:
                return std::any_cast<const common::Date>(data) >= std::any_cast<const common::Date&>(rhs.data);
            case ValueType::DATETIME:
                return std::any_cast<const common::DateTime>(data) >=
                       std::any_cast<const common::DateTime&>(rhs.data);
            case ValueType::LOCALDATETIME:
                return std::any_cast<const common::LocalDateTime>(data) >= std::any_cast<const common::LocalDateTime&>(rhs.data);
            case ValueType::LOCALTIME:
                return std::any_cast<const common::LocalTime>(data) >= std::any_cast<const common::LocalTime&>(rhs.data);
            case ValueType::TIME:
                return std::any_cast<const common::Time>(data) >= std::any_cast<const common::Time&>(rhs.data);
            case ValueType::DURATION:
                return std::any_cast<const common::Duration>(data) >= std::any_cast<const common::Duration&>(rhs.data);
            default:
                THROW_CODE(ValueException, "Unhandled data type");
        }
    } catch (std::exception &e) {
        THROW_CODE(ValueException, "Unable to compare two Value with different types. " + ::ToString(type) + " vs " + ::ToString(rhs.type));
    }
}

bool Value::operator==(const Value& b) const {
    if (this->type != b.type) {
        return false;
    }
    switch (this->type) {
        case ValueType::BOOL: {
            return std::any_cast<bool>(this->data) == std::any_cast<bool>(b.data);
        }
        case ValueType::INTEGER: {
            return std::any_cast<int64_t>(this->data) == std::any_cast<int64_t>(b.data);
        }
        case ValueType::DOUBLE: {
            return std::abs(std::any_cast<double>(this->data) - std::any_cast<double>(b.data))
                < std::numeric_limits<double>::epsilon();
        }
        case ValueType::FLOAT: {
            return std::abs(std::any_cast<float>(this->data) - std::any_cast<float>(b.data))
                   < std::numeric_limits<float>::epsilon();
        }
        case ValueType::STRING: {
            return std::any_cast<const std::string&>(this->data) == std::any_cast<const std::string&>(b.data);
        }
        case ValueType::ARRAY: {
            const auto& l = std::any_cast<const std::vector<Value>&>(this->data);
            const auto& r = std::any_cast<const std::vector<Value>&>(b.data);
            if (l.size() != r.size()) {
                return false;
            }
            for (size_t i = 0; i < l.size(); i++) {
                if (!(l[i] == r[i])) {
                    return false;
                }
            }
            return true;
        }
        case ValueType::MAP: {
            const auto& l = std::any_cast<const std::unordered_map<std::string, Value>&>(this->data);
            const auto& r = std::any_cast<const std::unordered_map<std::string, Value>&>(b.data);
            if (l.size() != r.size()) {
                return false;
            }
            for (const auto& pair : l) {
                auto iter = r.find(pair.first);
                if (iter == r.end() || !(iter->second == pair.second)) {
                    return false;
                }
            }
            return true;
        }
        case ValueType::DATE:
            return std::any_cast<const common::Date>(this->data) == std::any_cast<const common::Date&>(b.data);
        case ValueType::LOCALDATETIME:
            return std::any_cast<const common::LocalDateTime>(this->data) == std::any_cast<const common::LocalDateTime&>(b.data);
        case ValueType::DATETIME:
            return std::any_cast<const common::DateTime>(this->data) ==
                   std::any_cast<const common::DateTime&>(b.data);
        case ValueType::LOCALTIME:
            return std::any_cast<const common::LocalTime>(data) == std::any_cast<const common::LocalTime&>(b.data);
        case ValueType::TIME:
            return std::any_cast<const common::Time>(data) == std::any_cast<const common::Time&>(b.data);
        case ValueType::DURATION:
            return std::any_cast<const common::Duration>(data) == std::any_cast<const common::Duration&>(b.data);
        case ValueType::Null: {
            return true;
        }
    }
    return false;
}
