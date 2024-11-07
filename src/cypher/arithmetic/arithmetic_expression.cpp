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

//
// Created by wt on 6/15/18.
//
#include <cmath>
#include <cstdint>
#include <random>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>
#include <regex>
#include "cypher/cypher_exception.h"
#include "cypher/cypher_types.h"
#include "cypher/parser/symbol_table.h"
#include "cypher/arithmetic/arithmetic_expression.h"
#include "cypher/filter/filter.h"
#include "common/logger.h"

#define CHECK_NODE(e)                                                                      \
    do {                                                                                   \
        if (!e.IsNode())                                                                   \
            THROW_CODE(CypherException, "Invalid argument of " + std::string(__func__)); \
    } while (0)

#define CHECK_RELP(e)                                                                      \
    do {                                                                                   \
        if (!e.IsRelationship())                                                           \
            THROW_CODE(CypherException, "Invalid argument of " + std::string(__func__));   \
    } while (0)

#define CHECK_CONSTANT(e)                                                                  \
    do {                                                                                   \
        if (e.type != Entry::CONSTANT)                                                     \
            THROW_CODE(CypherException, "Invalid argument of " + std::string(__func__)); \
    } while (0)

#define VALIDATE_IT(e) (e.node->vertex_)
#define VALIDATE_EIT(e) (r.relationship->edge_)
namespace cypher {

Value BuiltinFunction::Id(RTContext *ctx, const Record &record,
                                      const std::vector<ArithExprNode> &args) {
    /* note the 1st argument is DISTINCT_OR_NOT */
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    const auto& operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    // TODO(anyone) handle snapshot of node/relp
    if (r.IsNode()) {
        if (!VALIDATE_IT(r)) return {};
        return Value::Integer(r.node->vertex_->GetNativeId());
    } else if (r.IsRelationship()) {
        if (!VALIDATE_EIT(r)) return {};
        return Value::Integer(r.relationship->edge_->GetNativeId());
    } else {
        THROW_CODE(CypherException, "Invalid argument of " + std::string(__func__));
    }
}

Value BuiltinFunction::Type(RTContext *ctx, const Record &record,
                            const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    const auto& operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    CHECK_RELP(r);
    if (!VALIDATE_EIT(r)) return {};
    return Value::String(r.relationship->edge_->GetType());
}

Value BuiltinFunction::Labels(RTContext *ctx, const Record &record,
                            const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    const auto& operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    CHECK_NODE(r);
    auto labels = r.node->vertex_->GetLabels();
    std::vector<Value> ret;
    ret.reserve(labels.size());
    for (auto& label : labels) {
        ret.emplace_back(label);
    }
    return Value(ret);
}

Value BuiltinFunction::Properties(RTContext *ctx, const Record &record,
                                              const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    const auto& operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    if (r.type == Entry::NODE && r.node && VALIDATE_IT(r)) {
        return Value(r.node->vertex_->GetAllProperty());
    } else if (r.type == Entry::RELATIONSHIP && r.relationship && VALIDATE_EIT(r)) {
        return Value(r.relationship->edge_->GetAllProperty());
    } else if (r.type == Entry::PATH) {
        std::vector<Value> ret;
        for (auto &p : r.path) {
            if (p.is_node) {
                ret.emplace_back(std::any_cast<graphdb::Vertex&>(p.element).GetAllProperty());
            } else {
                ret.emplace_back(std::any_cast<graphdb::Edge&>(p.element).GetAllProperty());
            }
        }
        return Value(ret);
    } else {
        THROW_CODE(CypherException, "Invalid argument of " + std::string(__func__));
    }
}

Value BuiltinFunction::StartsWith(RTContext *ctx, const Record &record,
                                              const std::vector<ArithExprNode> &args) {
    if (args.size() != 3) CYPHER_ARGUMENT_ERROR();
    auto prefix = args[1].Evaluate(ctx, record);
    std::string prefix_str = prefix.constant.AsString();
    auto variable = args[2].Evaluate(ctx, record);
    std::string variable_str = variable.constant.IsNull()? "" : variable.constant.AsString();
    bool ret = variable_str.compare(0, prefix_str.size(), prefix_str) == 0;
    return Value(ret);
}

Value BuiltinFunction::EndsWith(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() != 3) CYPHER_ARGUMENT_ERROR();
    auto postfix = args[1].Evaluate(ctx, record);
    std::string postfix_str = postfix.constant.AsString();
    auto variable = args[2].Evaluate(ctx, record);
    std::string variable_str = variable.constant.IsNull() ? "" : variable.constant.AsString();
    bool ret = variable_str.size() >= postfix_str.size() &&
               variable_str.compare(variable_str.size() - postfix_str.size(),
                                    postfix_str.size(), postfix_str) == 0;
    return Value(ret);
}

Value BuiltinFunction::Contains(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() != 3) CYPHER_ARGUMENT_ERROR();
    auto substr = args[1].Evaluate(ctx, record);
    std::string substr_str = substr.constant.AsString();
    auto variable = args[2].Evaluate(ctx, record);
    std::string variable_str = variable.constant.IsNull()? "" : variable.constant.AsString();
    bool ret = variable_str.find(substr_str) != std::string::npos;
    return Value(ret);
}

Value BuiltinFunction::Regexp(RTContext *ctx, const Record &record,
                                          const std::vector<ArithExprNode> &args) {
    if (args.size() != 3) CYPHER_ARGUMENT_ERROR();
    auto regexp = args[1].Evaluate(ctx, record);
    std::string regexp_str = regexp.constant.AsString();
    auto variable = args[2].Evaluate(ctx, record);
    std::string variable_str = variable.constant.IsNull() ? "" : variable.constant.AsString();
    if (variable_str.empty()) return Value(false);
    bool ret = std::regex_match(variable_str, std::regex(regexp_str));
    return Value(ret);
}

Value BuiltinFunction::SubString(RTContext *ctx, const Record &record,
                                             const std::vector<ArithExprNode> &args) {
    /* Arguments:
     * start    An expression that returns an integer value.
     * length   An expression that returns an integer value.
     */
    if (args.size() != 4) CYPHER_ARGUMENT_ERROR();

    auto arg1 = args[1].Evaluate(ctx, record);
    switch (arg1.type) {
        case Entry::CONSTANT:
            if (arg1.constant.IsString()) {
                auto arg2 = args[2].Evaluate(ctx, record);
                auto arg3 = args[3].Evaluate(ctx, record);
                if (!arg2.IsInteger())
                    THROW_CODE(CypherException, "Argument 2 of `SUBSTRING()` expects int type: "
                                                  + arg2.ToString());
                if (!arg3.IsInteger())
                    THROW_CODE(CypherException, "Argument 3 of `SUBSTRING()` expects int type: "
                                                  + arg3.ToString());

                auto origin = arg1.constant.IsNull() ? "": arg1.constant.AsString();
                auto size = static_cast<int64_t>(origin.length());
                auto start = arg2.constant.AsInteger();
                auto length = arg3.constant.AsInteger();
                if (start < 1 || start > size)
                    THROW_CODE(CypherException, "Invalid argument 2 of `SUBSTRING()`: "
                                                  + arg2.ToString());
                if (length < 1 || start - 1 + length > size)
                    THROW_CODE(CypherException, "Invalid argument 3 of `SUBSTRING()`: "
                                                  + arg3.ToString());

                auto result = origin.substr(start - 1, length);
                return Value(result);
            }
            break;
        default:
            break;
    }

    THROW_CODE(CypherException, "Function `SUBSTRING()` is not supported for: "
                                  + arg1.ToString());
}

Value BuiltinFunction::Concat(RTContext *ctx, const Record &record,
                                          const std::vector<ArithExprNode> &args) {
    /* Arguments:
     * others[1...n]    At least one expression that returns a string value.
     */
    if (args.size() < 3) CYPHER_ARGUMENT_ERROR();

    auto arg1 = args[1].Evaluate(ctx, record);
    switch (arg1.type) {
        case Entry::CONSTANT:
            if (arg1.constant.IsString()) {
                auto result = arg1.constant.IsNull() ? "" : arg1.constant.AsString();

                for (int i = 2; i < (int)args.size(); ++i) {
                    auto arg = args[i].Evaluate(ctx, record);
                    if (!arg.IsString())
                        THROW_CODE(CypherException, "Argument " + std::to_string(i)
                                                      + " of `CONCAT()` expects string type: "
                                                      + arg.ToString());

                    result.append(arg.constant.AsString());
                }

                return Value(result);
            }
            break;
        default:
            break;
    }

    THROW_CODE(CypherException, "Function `CONCAT()` is not supported for: " + arg1.ToString());
}

Value BuiltinFunction::Mask(RTContext *ctx, const Record &record,
                                        const std::vector<ArithExprNode> &args) {
    /* Arguments:
     * start    An expression that returns an integer value.
     * end   An expression that returns an integer value.
     * mask_char An expression that returns a char value.
     */
    if (args.size() != 4 && args.size() != 5) CYPHER_ARGUMENT_ERROR();
    auto extractChineseCharacters = [](const std::string& text) {
        auto isUtf8StartByte = [](unsigned char c) {
            return (c & 0xC0) != 0x80;
        };
        std::vector<std::string> characters;
        std::string currentChar;
        for (unsigned char c : text) {
            if (isUtf8StartByte(c)) {
                if (!currentChar.empty()) {
                    characters.push_back(currentChar);
                    currentChar.clear();
                }
            }
            currentChar += c;
        }
        if (!currentChar.empty()) {
            characters.push_back(currentChar);
        }
        return characters;
    };
    auto arg1 = args[1].Evaluate(ctx, record);
    switch (arg1.type) {
        case Entry::CONSTANT:
            if (arg1.constant.IsString()) {
                auto arg2 = args[2].Evaluate(ctx, record);
                auto arg3 = args[3].Evaluate(ctx, record);
                if (!arg2.IsInteger())
                    THROW_CODE(CypherException, "Argument 2 of `MASK()` expects int type: "
                                                  + arg2.ToString());
                if (!arg3.IsInteger())
                    THROW_CODE(CypherException, "Argument 3 of `MASK()` expects int type: "
                                                  + arg3.ToString());

                std::string ss = "*";
                if (args.size() == 5) {
                    auto arg4 = args[4].Evaluate(ctx, record);
                    ss = arg4.constant.AsString();
                }
                auto origin = arg1.constant.IsNull() ? "" : arg1.constant.AsString();
                auto origin_strings = extractChineseCharacters(origin);
                auto size = static_cast<int64_t>(origin_strings.size());
                auto start = arg2.constant.AsInteger();
                auto end = arg3.constant.AsInteger();
                if (start < 1 || start > size)
                    THROW_CODE(CypherException, "Invalid argument 2 of `MASK()`: "
                                                  + arg2.ToString());
                if (end < start || end > size)
                    THROW_CODE(CypherException, "Invalid argument 3 of `MASK()`: "
                                                  + arg3.ToString());

                std::string result;
                for (int i = 0; i < start - 1; ++i)
                    result.append(origin_strings[i]);
                for (int i = start; i <= end; i++)
                    result.append(ss);
                for (int i = end; i < size; ++i)
                    result.append(origin_strings[i]);
                return Value(result);
            }
            break;
        default:
            break;
    }

    THROW_CODE(CypherException, "Function `MASK()` is not supported for: "
                                  + arg1.ToString());
}

Value BuiltinFunction::Exists(RTContext *ctx, const Record &record,
                              const std::vector<ArithExprNode> &args) {
    auto res = args[1].Evaluate(ctx, record);
    return Value(!res.IsNull());
}

Value BuiltinFunction::Abs(RTContext *ctx, const Record &record,
                                       const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    const auto& operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    CHECK_CONSTANT(r);
    if (r.constant.IsInteger()) {
        return r.constant.AsInteger() < 0
                   ? Value(-r.constant.AsInteger())
                   : r.constant;
    } else if (r.constant.IsDouble()) {
        return r.constant.AsDouble() < 0
                   ? Value(-r.constant.AsDouble())
                   : r.constant;
    }
    CYPHER_ARGUMENT_ERROR();
}

Value BuiltinFunction::Ceil(RTContext *ctx, const Record &record,
                                        const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    const auto& operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    CHECK_CONSTANT(r);
    if (r.constant.IsInteger()) {
        return r.constant;
    } else if (r.constant.IsDouble()) {
        return Value(std::ceil(r.constant.AsDouble()));
    }
    CYPHER_ARGUMENT_ERROR();
}

Value BuiltinFunction::Floor(RTContext *ctx, const Record &record,
                                         const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    const auto& operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    CHECK_CONSTANT(r);
    if (r.constant.IsInteger()) {
        return r.constant;
    } else if (r.constant.IsDouble()) {
        return Value(std::floor(r.constant.AsDouble()));
    }
    CYPHER_ARGUMENT_ERROR();
}

Value BuiltinFunction::Rand(RTContext *ctx, const Record &record,
                                        const std::vector<ArithExprNode> &args) {
    if (args.size() != 1) CYPHER_ARGUMENT_ERROR();
    std::random_device rd;                        // device
    std::mt19937 rng(rd());                       // engine
    std::uniform_real_distribution<> dist(0, 1);  // distribution
    return Value(dist(rng));
}

Value BuiltinFunction::Round(RTContext *ctx, const Record &record,
                                         const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    const auto& operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    CHECK_CONSTANT(r);
    if (r.constant.IsInteger()) {
        return r.constant;
    } else if (r.constant.IsDouble()) {
        return Value(std::round(r.constant.AsDouble()));
    }
    CYPHER_ARGUMENT_ERROR();
}

Value BuiltinFunction::Sign(RTContext *ctx, const Record &record,
                                        const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    const auto& operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    CHECK_CONSTANT(r);
    if (r.constant.IsInteger()) {
        int8_t sign = std::signbit(r.constant.AsInteger()) ? -1 : 1;
        return Value::Integer(sign);
    } else if (r.constant.IsDouble()) {
        int8_t sign = std::signbit(r.constant.AsDouble()) ? -1 : 1;
        return Value::Integer(sign);
    }
    CYPHER_ARGUMENT_ERROR();
}

Value BuiltinFunction::ToBoolean(RTContext *ctx, const Record &record,
                                             const std::vector<cypher::ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    const auto& operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    switch (r.type) {
        case Entry::CONSTANT:
            if (r.constant.IsBool()) {
                return r.constant;
            } else if (r.constant.IsString()) {
                const std::string& d = r.constant.AsString();
                if (d == "True" || d == "true") return Value(true);
                if (d == "False" || d == "false") return Value(false);
            }
        default:
            break;
    }
    CYPHER_ARGUMENT_ERROR();
}

Value BuiltinFunction::ToFloat(RTContext *ctx, const Record &record,
                                           const std::vector<cypher::ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    const auto& operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    switch (r.type) {
        case Entry::CONSTANT:
            if (r.constant.IsInteger()) {
                return Value(static_cast<double>(r.constant.AsInteger()));
            }
            if (r.constant.IsDouble()) {
                return r.constant;
            }
            if (r.constant.IsString()) {
                try {
                    return Value(std::stod(r.constant.AsString()));
                } catch (std::exception &e) {
                    LOG_WARN(e.what());
                    break;
                }
            }
        default:
            break;
    }
    CYPHER_ARGUMENT_ERROR();
}

Value BuiltinFunction::ToString(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    const auto& operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    switch (r.type) {
        case Entry::CONSTANT:
            if (r.constant.IsString()) {
                return r.constant;
            }
            if (r.constant.IsInteger()) {
                try {
                    return Value(std::to_string(r.constant.AsInteger()));
                } catch (std::exception &e) {
                    LOG_WARN(e.what());
                    break;
                }
            }
            if (r.constant.IsDouble()) {
                try {
                    return Value(std::to_string(r.constant.AsDouble()));
                } catch (std::exception &e) {
                    LOG_WARN(e.what());
                    break;
                }
            }
            if (r.constant.IsBool()) {
                bool d = r.constant.AsBool();
                if (d)
                    return Value("true");
                else
                    return Value("false");
            }
        default:
            break;
    }
    CYPHER_ARGUMENT_ERROR();
}

Value BuiltinFunction::ToInteger(RTContext *ctx, const Record &record,
                                             const std::vector<cypher::ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    const auto& operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    switch (r.type) {
        case Entry::CONSTANT:
            if (r.constant.IsInteger()) {
                return r.constant;
            }
            if (r.constant.IsDouble()) {
                return Value(static_cast<int64_t>(r.constant.AsDouble()));
            }
            if (r.constant.IsString()) {
                try {
                    return Value(
                        static_cast<int64_t>(std::stoll(r.constant.AsString())));
                } catch (std::exception &e) {
                    LOG_WARN(e.what());
                    break;
                }
            }
        default:
            break;
    }
    CYPHER_ARGUMENT_ERROR();
}

Value BuiltinFunction::Date(RTContext *ctx, const Record &record,
                                        const std::vector<ArithExprNode> &args) {
    if (args.size() > 2) CYPHER_ARGUMENT_ERROR();
    if (args.size() == 1) {
        // date() Returns the current date;
        return Value(common::Date());
    } else {
        CYPHER_THROW_ASSERT(args.size() == 2);
        // date(string) Returns a Date by parsing a string.
        auto r = args[1].Evaluate(ctx, record);
        if (r.IsMap()) {
            auto dt = common::Date(r.constant);
            return Value(dt);
        } else if (r.IsString()) {
            auto dt = common::Date(r.constant.AsString());
            return Value(dt);
        } else {
            CYPHER_ARGUMENT_ERROR();
        }
    }
}

Value BuiltinFunction::LocalDateTime(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() > 2) CYPHER_ARGUMENT_ERROR();
    if (args.size() == 1) {
        // localdatetime() Returns the current LocalDateTime.
        return Value(common::LocalDateTime());
    } else {
        CYPHER_THROW_ASSERT(args.size() == 2);
        // datetime(string) Returns a DateTime by parsing a string.
        auto r = args[1].Evaluate(ctx, record);
        if (!r.IsString()) CYPHER_ARGUMENT_ERROR();
        auto dt = common::LocalDateTime(r.constant.AsString());
        return Value(dt);
    }
}

Value BuiltinFunction::Range(RTContext *ctx, const Record &record,
                             const std::vector<ArithExprNode> &args) {
    /* Arguments:
     * start    An expression that returns an integer value.
     * end      An expression that returns an integer value.
     * step     A numeric expression defining the difference between any two consecutive values,
     * with a default of 1.
     */
    if (args.size() < 3 || args.size() > 4) CYPHER_ARGUMENT_ERROR();
    auto start = args[1].Evaluate(ctx, record);
    auto end = args[2].Evaluate(ctx, record);
    int64_t step_i = 1;
    if (args.size() == 4) {
        auto step = args[3].Evaluate(ctx, record);
        if (!step.IsInteger()) CYPHER_ARGUMENT_ERROR();
        step_i = step.constant.AsInteger();
    }
    if (!start.IsInteger() || !end.IsInteger()) CYPHER_ARGUMENT_ERROR();
    auto start_i = start.constant.AsInteger();
    auto end_i = end.constant.AsInteger();
    Value ret = Value::Array({});
    if (step_i == 0) {
        THROW_CODE(InvalidParameter, "range() arg 3 must not be zero");
    }
    if ((step_i > 0 && start_i >= end_i) || (step_i < 0 && start_i <= end_i)) {
        return ret;
    }
    for (auto i = start_i; i <= end_i; i += step_i) {
        ret.AsArray().emplace_back(i);
    }
    return ret;
}

Value BuiltinFunction::Size(RTContext *ctx, const Record &record,
                                        const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto r = args[1].Evaluate(ctx, record);
    if (r.IsArray()) {
        return Value::Integer(r.constant.AsArray().size());
    } else if (r.IsString()) {
        return Value::Integer(r.constant.AsString().size());
    }
    THROW_CODE(CypherException, "Type mismatch: expected String or List<T>: " + r.ToString());
}

Value BuiltinFunction::Head(RTContext *ctx, const Record &record,
                            const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto r = args[1].Evaluate(ctx, record);
    if (!r.IsArray()) {
        THROW_CODE(CypherException, "List expected in head(): " + r.ToString());
    }
    return r.constant.AsArray().empty() ? Value()
                                     : Value(r.constant.AsArray().front());
}

Value BuiltinFunction::Last(RTContext *ctx, const Record &record,
                                        const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto r = args[1].Evaluate(ctx, record);
    if (!r.IsArray()) {
        THROW_CODE(CypherException, "List expected in last(): " + r.ToString());
    }
    return r.constant.AsArray().empty() ? Value()
                                        : Value(r.constant.AsArray().back());
}

Value BuiltinFunction::ToFloat32List(RTContext *ctx, const Record &record,
                            const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    const auto& operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    std::vector<Value> ret;
    if (r.IsConstant() && r.constant.IsArray()) {
        ret.reserve(r.constant.AsArray().size());
        for (auto& item : r.constant.AsArray()) {
            if (item.IsDouble()) {
                ret.emplace_back((float)item.AsDouble());
            } else if (item.IsInteger()) {
                ret.emplace_back((float)item.AsInteger());
            } else {
                THROW_CODE(InvalidParameter, "List element type error");
            }
        }
        return Value(std::move(ret));
    }
    THROW_CODE(InvalidParameter, "The parameter type of `toFloat32List` should be an double list.");
}

Value BuiltinFunction::StartNode(RTContext *ctx, const Record &record, const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    const auto& operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    CHECK_RELP(r);
    return Value(r.relationship->edge_->GetNativeStartId());
}

Value BuiltinFunction::EndNode(RTContext *ctx, const Record &record, const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    const auto& operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    CHECK_RELP(r);
    return Value(r.relationship->edge_->GetNativeEndId());
}

Value BuiltinFunction::Keys(RTContext *ctx, const Record &record, const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    const auto& operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    CHECK_NODE(r);
    auto props = r.node->vertex_->GetAllProperty();
    std::vector<std::string> ret;
    for (auto &prop : props) {
        ret.push_back(prop.first);
    }
    return Value(ret);
}

Value BuiltinFunction::Coalesce(RTContext *ctx, const Record &record, const std::vector<ArithExprNode> &args) {
    if (args.size() == 1) CYPHER_ARGUMENT_ERROR();
    for (int i = 1; i < (int)args.size(); i++) {
        auto r = args[i].Evaluate(ctx, record);
        CHECK_CONSTANT(r);
        if (!r.constant.IsNull()) {
            return r.constant;
        }
    }
    return {};
}

void ArithOperandNode::SetEntity(const std::string &alias, const SymbolTable &sym_tab) {
    type = AR_OPERAND_VARIADIC;
    variadic.alias = alias;
    auto it = sym_tab.symbols.find(alias);
    if (it == sym_tab.symbols.end()) {
        THROW_CODE(InputError,
            "Variable `{}` not defined", alias);
    }
    // CYPHER_THROW_ASSERT(it != sym_tab.symbols.end());
    variadic.alias_idx = it->second.id;
}

void ArithOperandNode::SetEntity(const std::string &alias, const std::string &property,
                                 const SymbolTable &sym_tab) {
    SetEntity(alias, sym_tab);
    variadic.entity_prop = property;
}

void ArithOperandNode::SetParameter(const std::string &param, const SymbolTable &sym_tab) {
    type = AR_OPERAND_PARAMETER;
    variadic.alias = param;
    auto it = sym_tab.symbols.find(param);
    if (it == sym_tab.symbols.end()) {
        THROW_CODE(CypherException, "Parameter not defined: " + param);
    }
    variadic.alias_idx = it->second.id;
}

void ArithOperandNode::RealignAliasId(const SymbolTable &sym_tab) {
    if (type == AR_OPERAND_CONSTANT) return;
    auto it = sym_tab.symbols.find(variadic.alias);
    CYPHER_THROW_ASSERT(it != sym_tab.symbols.end());
    variadic.alias_idx = it->second.id;
}


void ArithExprNode::SetOperand(ArithOperandNode::ArithOperandType operand_type,
                               const std::string &opd, const SymbolTable &sym_tab) {
    if (operand_type == ArithOperandNode::AR_OPERAND_VARIADIC) {
        operand.SetEntity(opd, sym_tab);
    } else if (operand_type == ArithOperandNode::AR_OPERAND_PARAMETER) {
        operand.SetParameter(opd, sym_tab);
    } else {
        THROW_CODE(CypherException, std::string(__func__) + ": operand type error");
    }
    type = AR_EXP_OPERAND;
}

void ArithExprNode::SetOperand(ArithOperandNode::ArithOperandType operand_type,
                               const std::string &alias, const std::string &property,
                               const SymbolTable &sym_tab) {
    SetOperand(operand_type, alias, property);
    CYPHER_THROW_ASSERT(sym_tab.symbols.find(alias) != sym_tab.symbols.end());
    operand.variadic.alias_idx = sym_tab.symbols.find(alias)->second.id;
}

void ArithExprNode::RealignAliasId(const SymbolTable &sym_tab) {
    if (type == AR_EXP_OPERAND) {
        return operand.RealignAliasId(sym_tab);
    } else {
        return op.RealignAliasId(sym_tab);
    }
}

static bool IsMathOperator(const ArithExprNode &node) {
    if (node.type != ArithExprNode::AR_EXP_OPERAND ||
        node.operand.type != ArithOperandNode::AR_OPERAND_CONSTANT ||
        !node.operand.constant.IsString()) {
        return false;
    }
    auto &s = node.operand.constant.AsString();
    static const int8_t operator_table[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, /* %, *, +, -, / */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, /* ^ */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    return s.size() == 1 && operator_table[(size_t)(s[0])] == (int8_t)1;
}

void ArithOpNode::RealignAliasId(const cypher::SymbolTable &sym_tab) {
    for (auto &c : children) c.RealignAliasId(sym_tab);
}

Entry ArithOpNode::Evaluate(RTContext *ctx, const Record &record) const {
    switch (type) {
    case AR_OP_AGGREGATE:
        /* Aggregation function should be reduced by now.
         * TODO(anyone) verify above statement. */
        return agg_func->result;
    case AR_OP_FUNC:
        {
            CYPHER_TODO();
#if 0
            if (func) return Entry(func(ctx, record, children));
            /* if !func, custom function */
            std::string input, output;
            std::string name = func_name.substr(std::string(CUSTOM_FUNCTION_PREFIX).size());
            // evaluate every child and add to input for output
            for (int i = 1; i < (int)children.size(); i++) {
                auto v = children[i].Evaluate(ctx, record);
                input.append(v.ToString());
                if (i != (int)children.size() - 1) input.append(",");
            }
            auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
            bool exists =
                ac_db.CallPlugin(ctx->txn_.get(), lgraph::plugin::Type::CPP,
                                 "A_DUMMY_TOKEN_FOR_CPP_PLUGIN", name, input, 0, false, output);
            if (!exists) {
                THROW_CODE(InputError, "Plugin [{}] does not exist.", name);
            }
            return Entry(cypher::FieldData(lgraph_api::FieldData(output)));
#endif
        }
    case AR_OP_FILTER:
        return Entry(Value(fp->DoFilter(ctx, record)));
    case AR_OP_CASE:
        {
            CYPHER_THROW_ASSERT(!children.empty() &&
                                children[0].type == ArithExprNode::AR_EXP_OPERAND &&
                                children[0].operand.type == ArithOperandNode::AR_OPERAND_CONSTANT);
            auto case_type = children[0].operand.constant.AsInteger();
            switch (case_type) {
            case 0:
            case 1:
                for (int i = 1; i < (int)children.size() - 1; i += 2) {
                    CYPHER_THROW_ASSERT(children[i].op.type == ArithOpNode::AR_OP_FILTER);
                    auto pred = children[i].Evaluate(ctx, record);
                    CYPHER_THROW_ASSERT(pred.IsBool());
                    if (pred.constant.AsBool()) return children[i + 1].Evaluate(ctx, record);
                }
                return case_type == 0 ? Entry() : children.back().Evaluate(ctx, record);
            case 2:
                {
                    CYPHER_TODO();
#if 0
                    auto test = children.back().Evaluate(ctx, record);
                    for (int i = 1; i < (int)children.size() - 1; i += 2) {
                        auto value = children[i].Evaluate(ctx, record);
                        if (value == test) return children[i + 1].Evaluate(ctx, record);
                    }
                    return Entry();
#endif
                }
            case 3:
                {
                    CYPHER_TODO();
#if 0
                    auto test = children[children.size() - 2].Evaluate(ctx, record);
                    for (int i = 1; i < (int)children.size() - 2; i += 2) {
                        auto value = children[i].Evaluate(ctx, record);
                        if (value == test) return children[i + 1].Evaluate(ctx, record);
                    }
                    return children.back().Evaluate(ctx, record);
#endif
                }
            default:
                return Entry();
            }
        }
    case AR_OP_MATH:
        {
            /* TODO(anyone) move out to AR_OP_FUNC */
            std::stack<Value> s;
            for (auto &c : children) {
                if (!IsMathOperator(c)) {
                    auto v = c.Evaluate(ctx, record);
                    s.emplace(v.constant);
                } else {
                    CYPHER_THROW_ASSERT(!s.empty());
                    auto y = s.top();
                    s.pop();
                    CYPHER_THROW_ASSERT(!s.empty());
                    auto x = s.top();
                    s.pop();
                    char token = c.operand.constant.AsString().at(0);
                    Value ret;
                    switch (token) {
                    case '+':
                        ret = Add(x, y);
                        break;
                    case '-':
                        ret = Sub(x, y);
                        break;
                    case '*':
                        ret = Mul(x, y);
                        break;
                    case '/':
                        ret = Div(x, y);
                        break;
                    case '%':
                        ret = Mod(x, y);
                        break;
                    case '^':
                        ret = Pow(x, y);
                        break;
                    default:
                        THROW_CODE(CypherException, "Invalid mathematical operator: " +
                                                      c.operand.constant.AsString());
                    }
                    s.emplace(ret);
                }
            }
            CYPHER_THROW_ASSERT(!s.empty());
            return Entry(s.top());
        }
    default:
        CYPHER_TODO();
    }
}

std::string ArithOpNode::ToString() const {
    std::string str;
    str.append(func_name).append("(");
    for (auto &c : children) str.append(c.ToString()).append(",");
    if (str[str.size() - 1] == ',') str.pop_back();
    str.append(")");
    return str;
}
}  // namespace cypher
