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
#pragma once

#include <core/data_type.h>
#include <cmath>
#include <string>
#include "cypher/filter/iterator.h"
#include "cypher/parser/data_typedef.h"
#include "cypher/execution_plan/runtime_context.h"
#include "cypher/arithmetic/ast_expr_evaluator.h"
#include "cypher/arithmetic/agg_funcs.h"
#include "cypher/utils/geax_expr_util.h"
#include "graph/common.h"

namespace lgraph {
class Filter;
}

namespace cypher {

struct ArithOperandNode;
struct ArithExprNode;
struct SymbolTable;


static inline bool IsNumeric(const cypher::FieldData &x) { return x.IsInteger() || x.IsReal(); }

static inline void AddList(cypher::FieldData &ret, const cypher::FieldData &x) {
    if (x.type == cypher::FieldData::ARRAY)
        ret.array->insert(ret.array->end(), x.array->begin(), x.array->end());
    else
        ret.array->emplace_back(x.scalar);
}

static inline void AddString(cypher::FieldData &ret, const cypher::FieldData &x) {
    if (x.IsBool()) throw lgraph::CypherException("Type mismatch: cannot add BOOL with STRING");
    CYPHER_THROW_ASSERT(x.type != cypher::FieldData::ARRAY);
    ret.scalar.data.buf->append(x.scalar.ToString());
}

[[maybe_unused]]
static cypher::FieldData Add(const cypher::FieldData &x, const cypher::FieldData &y) {
    if (x.IsNull() || y.IsNull()) return cypher::FieldData();
    cypher::FieldData ret;
    if (x.type == cypher::FieldData::ARRAY || y.type == cypher::FieldData::ARRAY) {
        ret.array = new std::vector<cypher::FieldData>();
        ret.type = cypher::FieldData::ARRAY;
        AddList(ret, x);
        AddList(ret, y);
    } else if (x.IsString() || y.IsString()) {
        ret.scalar = ::lgraph::FieldData("");
        AddString(ret, x);
        AddString(ret, y);
    } else if (IsNumeric(x) && IsNumeric(y)) {
        if (x.IsInteger() && y.IsInteger()) {
            ret.scalar = ::lgraph::FieldData(x.scalar.integer() + y.scalar.integer());
        } else {
            double x_n = x.IsInteger() ? x.scalar.integer() : x.scalar.real();
            double y_n = y.IsInteger() ? y.scalar.integer() : y.scalar.real();
            ret.scalar = ::lgraph::FieldData(x_n + y_n);
        }
    }
    return ret;
}

[[maybe_unused]]
static cypher::FieldData Sub(const cypher::FieldData &x, const cypher::FieldData &y) {
    if (!((IsNumeric(x) || x.IsNull()) && (IsNumeric(y) || y.IsNull())))
        throw lgraph::CypherException("Type mismatch: expect Integer or Float in sub expr");
    if (x.IsNull() || y.IsNull()) return cypher::FieldData();
    cypher::FieldData ret;
    if (x.IsInteger() && y.IsInteger()) {
        ret.scalar = ::lgraph::FieldData(x.scalar.integer() - y.scalar.integer());
    } else {
        double x_n = x.IsInteger() ? x.scalar.integer() : x.scalar.real();
        double y_n = y.IsInteger() ? y.scalar.integer() : y.scalar.real();
        ret.scalar = ::lgraph::FieldData(x_n - y_n);
    }
    return ret;
}

[[maybe_unused]]
static cypher::FieldData Mul(const cypher::FieldData &x, const cypher::FieldData &y) {
    if (!((IsNumeric(x) || x.IsNull()) && (IsNumeric(y) || y.IsNull())))
        throw lgraph::CypherException("Type mismatch: expect Integer or Float in mul expr");
    if (x.IsNull() || y.IsNull()) return cypher::FieldData();
    cypher::FieldData ret;
    if (x.IsInteger() && y.IsInteger()) {
        ret.scalar = ::lgraph::FieldData(x.scalar.integer() * y.scalar.integer());
    } else {
        double x_n = x.IsInteger() ? x.scalar.integer() : x.scalar.real();
        double y_n = y.IsInteger() ? y.scalar.integer() : y.scalar.real();
        ret.scalar = ::lgraph::FieldData(x_n * y_n);
    }
    return ret;
}

[[maybe_unused]]
static cypher::FieldData Div(const cypher::FieldData &x, const cypher::FieldData &y) {
    if (!((IsNumeric(x) || x.IsNull()) && (IsNumeric(y) || y.IsNull())))
        throw lgraph::CypherException("Type mismatch: expect Integer or Float in mul expr");
    if (x.IsNull() || y.IsNull()) return cypher::FieldData();
    cypher::FieldData ret;
    if (x.IsInteger() && y.IsInteger()) {
        int64_t x_n = x.scalar.integer();
        int64_t y_n = y.scalar.integer();
        if (y_n == 0) throw lgraph::CypherException("Divided by zero");
        ret.scalar = ::lgraph::FieldData(x_n / y_n);
    } else {
        double x_n = x.IsInteger() ? x.scalar.integer() : x.scalar.real();
        double y_n = y.IsInteger() ? y.scalar.integer() : y.scalar.real();
        if (y_n == 0)
            // In neo4j:
            // 0.0 / 0.0 = null
            // 1.0 / 0.0 = ifinity.0
            // -1.0 / 0.0 = -ifinity.0
            CYPHER_TODO();
        else
            ret.scalar = ::lgraph::FieldData(x_n / y_n);
    }
    return ret;
}

[[maybe_unused]]
static cypher::FieldData Mod(const cypher::FieldData &x, const cypher::FieldData &y) {
    if (!((IsNumeric(x) || x.IsNull()) && (IsNumeric(y) || y.IsNull())))
        throw lgraph::CypherException("Type mismatch: expect Integer or Float in mul expr");
    if (x.IsNull() || y.IsNull()) return cypher::FieldData();
    cypher::FieldData ret;
    if (x.IsInteger() && y.IsInteger()) {
        int64_t x_n = x.scalar.integer();
        int64_t y_n = y.scalar.integer();
        if (y_n == 0) throw lgraph::CypherException("Divided by zero");
        ret.scalar = ::lgraph::FieldData(x_n % y_n);
    } else {
        // Float mod
        CYPHER_TODO();
    }
    return ret;
}

[[maybe_unused]]
static cypher::FieldData Pow(const cypher::FieldData &x, const cypher::FieldData &y) {
    if (!((IsNumeric(x) || x.IsNull()) && (IsNumeric(y) || y.IsNull())))
        throw lgraph::CypherException("Type mismatch: expect Integer or Float in mul expr");
    if (x.IsNull() || y.IsNull()) return cypher::FieldData();
    cypher::FieldData ret;
    double x_n = x.IsInteger() ? x.scalar.integer() : x.scalar.real();
    double y_n = y.IsInteger() ? y.scalar.integer() : y.scalar.real();
    ret.scalar = ::lgraph::FieldData(pow(x_n, y_n));
    return ret;
}

struct BuiltinFunction {
    typedef std::function<cypher::FieldData(RTContext *, const Record &,
                                            const std::vector<ArithExprNode> &)>
        FUNC;

    /* Scalar functions */
    static cypher::FieldData Id(RTContext *ctx, const Record &record,
                                const std::vector<ArithExprNode> &args);

    static cypher::FieldData EUid(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);

    static cypher::FieldData Label(RTContext *ctx, const Record &record,
                                   const std::vector<ArithExprNode> &args);

    static cypher::FieldData Type(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);

    static cypher::FieldData EndNode(RTContext *ctx, const Record &record,
                                     const std::vector<ArithExprNode> &args);

    static cypher::FieldData StartNode(RTContext *ctx, const Record &record,
                                       const std::vector<ArithExprNode> &args);

    static cypher::FieldData Properties(RTContext *ctx, const Record &record,
                                        const std::vector<ArithExprNode> &args);

    static cypher::FieldData Head(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);

    static cypher::FieldData Last(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);

    static cypher::FieldData MaxInList(RTContext *ctx, const Record &record,
                                       const std::vector<ArithExprNode> &args);

    static cypher::FieldData MinInList(RTContext *ctx, const Record &record,
                                       const std::vector<ArithExprNode> &args);

    static cypher::FieldData Size(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);

    static cypher::FieldData Length(RTContext *ctx, const Record &record,
                                    const std::vector<ArithExprNode> &args);

    static cypher::FieldData Nodes(RTContext *ctx, const Record &record,
                                   const std::vector<ArithExprNode> &args);

    static cypher::FieldData Relationships(RTContext *ctx, const Record &record,
                                           const std::vector<ArithExprNode> &args);

    /* List functions */
    static cypher::FieldData Labels(RTContext *ctx, const Record &record,
                                    const std::vector<ArithExprNode> &args);

    static cypher::FieldData Keys(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);

    static cypher::FieldData Range(RTContext *ctx, const Record &record,
                                   const std::vector<ArithExprNode> &args);

    static cypher::FieldData Subscript(RTContext *ctx, const Record &record,
                                       const std::vector<ArithExprNode> &args);

    static cypher::FieldData IsAsc(RTContext *ctx, const Record &record,
                                   const std::vector<ArithExprNode> &args);

    static cypher::FieldData IsDesc(RTContext *ctx, const Record &record,
                                    const std::vector<ArithExprNode> &args);

    static cypher::FieldData HasDuplicates(RTContext *ctx, const Record &record,
                                           const std::vector<ArithExprNode> &args);

    static cypher::FieldData GetMemberProp(RTContext *ctx, const Record &record,
                                           const std::vector<ArithExprNode> &args);

    /* Mathematical functions - numeric */
    static cypher::FieldData Abs(RTContext *ctx, const Record &record,
                                 const std::vector<ArithExprNode> &args);

    static cypher::FieldData Ceil(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);

    static cypher::FieldData Floor(RTContext *ctx, const Record &record,
                                   const std::vector<ArithExprNode> &args);

    static cypher::FieldData Rand(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);

    static cypher::FieldData Round(RTContext *ctx, const Record &record,
                                   const std::vector<ArithExprNode> &args);

    static cypher::FieldData Sign(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);

    static cypher::FieldData ToBoolean(RTContext *ctx, const Record &record,
                                       const std::vector<ArithExprNode> &args);

    static cypher::FieldData ToFloat(RTContext *ctx, const Record &record,
                                     const std::vector<ArithExprNode> &args);

    static cypher::FieldData ToInteger(RTContext *ctx, const Record &record,
                                       const std::vector<ArithExprNode> &args);

    static cypher::FieldData ToString(RTContext *ctx, const Record &record,
                                      const std::vector<ArithExprNode> &args);

    /* temporal functions - instant types */
    static cypher::FieldData Date(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);

    static cypher::FieldData DateComponent(RTContext *ctx, const Record &record,
                                           const std::vector<ArithExprNode> &args);

    static cypher::FieldData DateTime(RTContext *ctx, const Record &record,
                                      const std::vector<ArithExprNode> &args);

    static cypher::FieldData DateTimeComponent(RTContext *ctx, const Record &record,
                                               const std::vector<ArithExprNode> &args);

    /* String functions - string types */
    static cypher::FieldData SubString(RTContext *ctx, const Record &record,
                                                 const std::vector<ArithExprNode> &args);

    static cypher::FieldData Concat(RTContext *ctx, const Record &record,
                                    const std::vector<ArithExprNode> &args);

    static cypher::FieldData Mask(RTContext *ctx, const Record &record,
                                    const std::vector<ArithExprNode> &args);

    /* spatial functions */
    /**
     * create point type data by point(double a, double b, srid(4326 in default))
     * or point(EWKB);
    */
    static cypher::FieldData Point(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);
    /**
     * create point type data by pointwkb(wkb, srid(4326 in default)); 
    */
    static cypher::FieldData PointWKB(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);
    /**
     * create point type data by pointwkt(wkt, srid(4326 in default));
    */
    static cypher::FieldData PointWKT(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);
    /**
     * create linestring by LinieString(EKWB);
    */
    static cypher::FieldData LineString(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);
    /**
     *  create linestring by linestringwkb(wkb, srid(4326 in default)); 
    */
    static cypher::FieldData LineStringWKB(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);
    /**
     *  create linestring by linestringwkt(wkt, srid(4326 in default));
    */
    static cypher::FieldData LineStringWKT(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);
    /**
     *  create polygon by polygon(EWKB); 
    */
    static cypher::FieldData Polygon(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);
    /**
     *  create polygon by polygonwkb(wkb, srid(4326 in default)); 
    */
    static cypher::FieldData PolygonWKB(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);
    /**
     *  create polygon by polygonwkt(wkt, srid(4326 in default)); 
    */
    static cypher::FieldData PolygonWKT(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);


    static cypher::FieldData StartsWith(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);

    static cypher::FieldData EndsWith(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);

    static cypher::FieldData Contains(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);

    static cypher::FieldData Regexp(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);

    static cypher::FieldData Exists(RTContext *ctx, const Record &record,
                                  const std::vector<ArithExprNode> &args);

    /* binary function (open cypher extension) */
    static cypher::FieldData Bin(RTContext *ctx, const Record &record,
                                 const std::vector<ArithExprNode> &args);

    static cypher::FieldData Coalesce(RTContext *ctx, const Record &record,
                                      const std::vector<ArithExprNode> &args);

    /* native API-like functions */
    static cypher::FieldData NativeGetEdgeField(RTContext *ctx, const Record &record,
                                                const std::vector<ArithExprNode> &args);

    /* Internal functions */
    static const std::string INTL_TO_PATH;
    static const std::string INTL_TO_LIST;
    static const std::string INTL_LIST_COMPREHENSION;

    static cypher::FieldData _ToPath(RTContext *ctx, const Record &record,
                                     const std::vector<ArithExprNode> &args);

    static cypher::FieldData _ToList(RTContext *ctx, const Record &record,
                                     const std::vector<ArithExprNode> &args);

    static cypher::FieldData _ListComprehension(RTContext *ctx, const Record &record,
                                                const std::vector<ArithExprNode> &args);
};

/* OperandNode represents either a constant numeric value,
 * a graph entity property, or a parameter. */
struct ArithOperandNode {
    // union {
    cypher::FieldData constant;
    struct Variadic {
        Node *node = nullptr;
        Relationship *relationship = nullptr;
        std::string alias;
        int alias_idx;
        std::string entity_prop;
    } variadic;
    //};
    struct Variable {
        bool hasMapFieldName;
        std::string _value_alias;
        std::string _map_field_name;
    } variable;
    /* AR_OperandNodeType type of leaf node,
     * either a constant: 3.14, a variable: node.property, or a parameter: $name. */
    enum ArithOperandType {
        AR_OPERAND_CONSTANT,
        AR_OPERAND_VARIADIC,
        AR_OPERAND_PARAMETER,
        AR_OPERAND_VARIABLE,
    } type;

    ArithOperandNode() = default;

    ArithOperandNode(const parser::Expression &expr, const SymbolTable &sym_tab);
    void SetConstant(const cypher::FieldData &data) {
        type = AR_OPERAND_CONSTANT;
        constant = data;
    }

    // TODO(anyone) remove
    void SetVariadic(const std::string &alias) {
        type = AR_OPERAND_VARIADIC;
        variadic.alias = alias;
    }
    void SetVariadic(const std::string &alias, const std::string &property) {
        SetVariadic(alias);
        variadic.entity_prop = property;
    }
    void SetVariable(const bool &hasMapFieldName = false, const std::string &value_alias = "",
                    const std::string &map_field_name = "") {
        type = AR_OPERAND_VARIABLE;
        variable.hasMapFieldName = hasMapFieldName;
        variable._value_alias = value_alias;
        variable._map_field_name = map_field_name;
    }

    void SetEntity(const std::string &alias, const SymbolTable &sym_tab);

    void SetEntity(const std::string &alias, const std::string &property,
                   const SymbolTable &sym_tab);
    void SetParameter(const std::string &param) {
        type = AR_OPERAND_PARAMETER;
        variadic.alias = param;
    }

    void SetParameter(const std::string &param, const SymbolTable &sym_tab);

    void Set(const parser::Expression &expr, const SymbolTable &sym_tab);

    void RealignAliasId(const SymbolTable &sym_tab);
    Entry Evaluate(RTContext *ctx, const Record &record) const {
        if (type == AR_OPERAND_CONSTANT) {
            return Entry(constant);
        } else if (type == AR_OPERAND_VARIADIC) {
            CYPHER_THROW_ASSERT(record.values.size() > (size_t)variadic.alias_idx);
            const auto &entry = record.values[variadic.alias_idx];
            switch (entry.type) {
            case Entry::NODE:
            case Entry::RELATIONSHIP:
            case Entry::NODE_SNAPSHOT:
            case Entry::RELP_SNAPSHOT:
                if (!variadic.entity_prop.empty()) {
                    return Entry(
                        cypher::FieldData(entry.GetEntityField(ctx, variadic.entity_prop)));
                }
                return entry;
            case Entry::VAR_LEN_RELP:
            case Entry::CONSTANT:
                return entry;
            default:
                CYPHER_TODO();
            }
        } else if (type == AR_OPERAND_PARAMETER) {
            if (record.values[variadic.alias_idx].type == Entry::UNKNOWN) {
                throw lgraph::CypherException("Undefined parameter: " + ToString());
            }
            return record.values[variadic.alias_idx];
        } else if (type == AR_OPERAND_VARIABLE) {
            return Entry(constant);
        } else {
            THROW_CODE(InternalError, "Invalid type.");
        }
        CYPHER_THROW_ASSERT(false);
        return Entry();
    }
    std::string ToString() const {
        std::string str;
        if (type == AR_OPERAND_VARIADIC) {
            str.append(variadic.alias);
            if (!variadic.entity_prop.empty()) str.append(".").append(variadic.entity_prop);
        } else if (type == AR_OPERAND_CONSTANT) {
            str = constant.ToString();
        } else if (type == AR_OPERAND_PARAMETER) {
            str = variadic.alias;
        } else if (type == AR_OPERAND_VARIABLE) {
            if (!variable.hasMapFieldName) {
                str.append(variable._value_alias);
            } else {
                str.append(variable._value_alias)
                .append(".")
                .append(variable._map_field_name);
            }
        }
        return str;
    }
};

/* Op represents an operation applied to child args. */
struct ArithOpNode {
    BuiltinFunction::FUNC func = nullptr;
    std::shared_ptr<AggCtx> agg_func;
    std::string func_name;
    std::vector<ArithExprNode> children; /* Child nodes. (args) */
    std::shared_ptr<lgraph::Filter> fp;
    /* AR_OPType type of operation
     * either an aggregation function which requires a context
     * or a stateless function. */
    enum ArithOpType {
        AR_OP_FUNC,
        AR_OP_AGGREGATE,
        // TODO(anyone) construct internal functions like _to_path
        AR_OP_CASE,
        AR_OP_FILTER,
        AR_OP_MATH,
    } type;

    /* Arithmetic function repository. */
    static std::unordered_map<std::string, BuiltinFunction::FUNC> RegisterFuncs() {
        std::unordered_map<std::string, BuiltinFunction::FUNC> ae_registered_funcs;
        /* functions & extension in OpenCypher 9 */
        ae_registered_funcs.emplace("id", BuiltinFunction::Id);
        ae_registered_funcs.emplace("euid", BuiltinFunction::EUid);
        ae_registered_funcs.emplace("label", BuiltinFunction::Label);
        ae_registered_funcs.emplace("type", BuiltinFunction::Type);
        ae_registered_funcs.emplace("endnode", BuiltinFunction::EndNode);
        ae_registered_funcs.emplace("startnode", BuiltinFunction::StartNode);
        ae_registered_funcs.emplace("properties", BuiltinFunction::Properties);
        ae_registered_funcs.emplace("head", BuiltinFunction::Head);
        ae_registered_funcs.emplace("last", BuiltinFunction::Last);
        ae_registered_funcs.emplace("maxinlist", BuiltinFunction::MaxInList);
        ae_registered_funcs.emplace("mininlist", BuiltinFunction::MinInList);
        ae_registered_funcs.emplace("size", BuiltinFunction::Size);
        ae_registered_funcs.emplace("length", BuiltinFunction::Length);
        ae_registered_funcs.emplace("nodes", BuiltinFunction::Nodes);
        ae_registered_funcs.emplace("relationships", BuiltinFunction::Relationships);
        ae_registered_funcs.emplace("labels", BuiltinFunction::Labels);
        ae_registered_funcs.emplace("keys", BuiltinFunction::Keys);
        ae_registered_funcs.emplace("range", BuiltinFunction::Range);
        ae_registered_funcs.emplace("subscript", BuiltinFunction::Subscript);
        ae_registered_funcs.emplace("isasc", BuiltinFunction::IsAsc);
        ae_registered_funcs.emplace("isdesc", BuiltinFunction::IsDesc);
        ae_registered_funcs.emplace("hasduplicates", BuiltinFunction::HasDuplicates);
        ae_registered_funcs.emplace("getmemberprop", BuiltinFunction::GetMemberProp);
        ae_registered_funcs.emplace("abs", BuiltinFunction::Abs);
        ae_registered_funcs.emplace("ceil", BuiltinFunction::Ceil);
        ae_registered_funcs.emplace("floor", BuiltinFunction::Floor);
        ae_registered_funcs.emplace("rand", BuiltinFunction::Rand);
        ae_registered_funcs.emplace("round", BuiltinFunction::Round);
        ae_registered_funcs.emplace("sign", BuiltinFunction::Sign);
        ae_registered_funcs.emplace("toboolean", BuiltinFunction::ToBoolean);
        ae_registered_funcs.emplace("tofloat", BuiltinFunction::ToFloat);
        ae_registered_funcs.emplace("tostring", BuiltinFunction::ToString);
        ae_registered_funcs.emplace("tointeger", BuiltinFunction::ToInteger);
        ae_registered_funcs.emplace("date", BuiltinFunction::Date);
        ae_registered_funcs.emplace("datecomponent", BuiltinFunction::DateComponent);
        ae_registered_funcs.emplace("datetime", BuiltinFunction::DateTime);
        ae_registered_funcs.emplace("datetimecomponent", BuiltinFunction::DateTimeComponent);
        ae_registered_funcs.emplace("substring", BuiltinFunction::SubString);
        ae_registered_funcs.emplace("concat", BuiltinFunction::Concat);
        ae_registered_funcs.emplace("mask", BuiltinFunction::Mask);
        ae_registered_funcs.emplace("bin", BuiltinFunction::Bin);
        ae_registered_funcs.emplace("coalesce", BuiltinFunction::Coalesce);
        /* spatial functions */
        ae_registered_funcs.emplace("point", BuiltinFunction::Point);
        ae_registered_funcs.emplace("pointwkb", BuiltinFunction::PointWKB);
        ae_registered_funcs.emplace("pointwkt", BuiltinFunction::PointWKT);
        ae_registered_funcs.emplace("linestring", BuiltinFunction::LineString);
        ae_registered_funcs.emplace("linestringwkb", BuiltinFunction::LineStringWKB);
        ae_registered_funcs.emplace("linestringwkt", BuiltinFunction::LineStringWKT);
        ae_registered_funcs.emplace("polygon", BuiltinFunction::Polygon);
        ae_registered_funcs.emplace("polygonwkb", BuiltinFunction::PolygonWKB);
        ae_registered_funcs.emplace("polygonwkt", BuiltinFunction::PolygonWKT);
        ae_registered_funcs.emplace("startswith", BuiltinFunction::StartsWith);
        ae_registered_funcs.emplace("endswith", BuiltinFunction::EndsWith);
        ae_registered_funcs.emplace("contains", BuiltinFunction::Contains);
        ae_registered_funcs.emplace("regexp", BuiltinFunction::Regexp);
        ae_registered_funcs.emplace("exists", BuiltinFunction::Exists);

        /* native API-like functions */
        ae_registered_funcs.emplace("native.getedgefield", BuiltinFunction::NativeGetEdgeField);
        /* internal functions */
        ae_registered_funcs.emplace(BuiltinFunction::INTL_TO_PATH, BuiltinFunction::_ToPath);
        ae_registered_funcs.emplace(BuiltinFunction::INTL_TO_LIST, BuiltinFunction::_ToList);
        ae_registered_funcs.emplace(BuiltinFunction::INTL_LIST_COMPREHENSION,
                                    BuiltinFunction::_ListComprehension);
        return ae_registered_funcs;
    }

    template <typename T>
    static std::shared_ptr<AggCtx> InitAggCtx() {
        return std::make_shared<T>();
    }

    static std::unordered_map<std::string, std::function<std::shared_ptr<AggCtx>()>>
    RegisterAggFuncs() {
        std::unordered_map<std::string, std::function<std::shared_ptr<AggCtx>()>>
            agg_registered_funcs;
        agg_registered_funcs.emplace("sum", [] { return std::make_shared<SumAggCtx>(); });
        agg_registered_funcs.emplace("avg", [] { return std::make_shared<AvgAggCtx>(); });
        agg_registered_funcs.emplace("max", [] { return std::make_shared<MaxAggCtx>(); });
        agg_registered_funcs.emplace("min", [] { return std::make_shared<MinAggCtx>(); });
        agg_registered_funcs.emplace("count", [] { return std::make_shared<CountAggCtx>(); });
        agg_registered_funcs.emplace("count(*)",
            [] { return std::make_shared<CountStarAggCtx>(); });
        agg_registered_funcs.emplace("collect", [] { return std::make_shared<CollectAggCtx>(); });
        agg_registered_funcs.emplace("percentilecont",
                                     [] { return std::make_shared<PercentileContAggCtx>(); });
        agg_registered_funcs.emplace("percentiledisc",
                                     [] { return std::make_shared<PercentileDiscAggCtx>(); });
        agg_registered_funcs.emplace("stdev", [] { return std::make_shared<StDevAggCtx>(); });
        agg_registered_funcs.emplace("stdevp", [] { return std::make_shared<StDevPAggCtx>(); });
        agg_registered_funcs.emplace("variance", [] { return std::make_shared<VarianceAggCtx>(); });
        agg_registered_funcs.emplace("variancep",
                                     [] { return std::make_shared<VariancePAggCtx>(); });
        return agg_registered_funcs;
    }

    void SetFunc(const std::string &fname) {
        /* Register arithmetic expression functions.
         * Note: only called once */
        static std::unordered_map<std::string, BuiltinFunction::FUNC> ae_registered_funcs =
            ArithOpNode::RegisterFuncs();
        static std::unordered_map<std::string, std::function<std::shared_ptr<AggCtx>()>>
            agg_registered_funcs = ArithOpNode::RegisterAggFuncs();
        func_name = fname;
        if (fname.find(CUSTOM_FUNCTION_PREFIX) == 0) {
            /* Setting func to nullptr to indicate custom function,
             * seek in plugin manager while runtime.  */
            type = AR_OP_FUNC;
            func = nullptr;
        } else {
            std::transform(func_name.begin(), func_name.end(), func_name.begin(), ::tolower);
            auto it = ae_registered_funcs.find(func_name);
            if (it != ae_registered_funcs.end()) {
                type = AR_OP_FUNC;
                func = it->second;
            } else {
                auto agg_it = agg_registered_funcs.find(func_name);
                if (agg_it == agg_registered_funcs.end()) {
                    throw lgraph::EvaluationException("unregistered function: " + fname);
                }
                type = AR_OP_AGGREGATE;
                agg_func = agg_it->second();
            }
        }
    }

    void RealignAliasId(const SymbolTable &sym_tab);

    Entry Evaluate(RTContext *ctx, const Record &record) const;

    std::string ToString() const;
};

/* AR_ExpNode a node within an arithmetic expression tree,
 * This node can take one of two forms:
 * 1. OperandNode
 * 2. OpNode */
struct ArithExprNode {
    // note: avoid put non-primitive data into union
    ArithOperandNode operand;
    ArithOpNode op;
    parser::Expression expression_;
    geax::frontend::Expr *expr_;
    std::shared_ptr<AstExprEvaluator> evaluator;

    /* AR_ExpNodeType lists the type of nodes within
     * an arithmetic expression tree. */
    enum ArithExprType {
        AR_EXP_OPERAND,
        AR_EXP_OP,
        // todo (kehuang): AstExpr is currently temporarily used as a type of ArithExpr, and will be
        // replaced with AstExpr in the future.
        AR_AST_EXP,
    } type;

    // The string representation of the node
    std::string resolved_name;

    ArithExprNode() = default;

    ArithExprNode(const parser::Expression &expr, const SymbolTable &sym_tab);

    ArithExprNode(geax::frontend::Expr *expr, const SymbolTable &sym_tab) {
        SetAstExp(expr, sym_tab);
    }

    void SetAstExp(geax::frontend::Expr *expr, const SymbolTable &sym_tab) {
        expr_ = expr;
        evaluator = std::make_shared<AstExprEvaluator>(expr, &sym_tab);
        type = AR_AST_EXP;
    }

    void SetOperand(ArithOperandNode::ArithOperandType operand_type, const std::string &opd,
                    const SymbolTable &sym_tab);

    void SetOperand(ArithOperandNode::ArithOperandType operand_type, const std::string &alias,
                    const std::string &property) {
        CYPHER_THROW_ASSERT(operand_type == ArithOperandNode::AR_OPERAND_VARIADIC);
        type = AR_EXP_OPERAND;
        operand.SetVariadic(alias, property);
    }

    void SetOperand(ArithOperandNode::ArithOperandType operand_type,
                    const cypher::FieldData &data) {
        CYPHER_THROW_ASSERT(operand_type == ArithOperandNode::AR_OPERAND_CONSTANT);
        type = AR_EXP_OPERAND;
        operand.SetConstant(data);
    }

    void SetOperandVariable(ArithOperandNode::ArithOperandType operand_type,
                    const bool &hasMapFieldName = false, const std::string &value_alias = "",
                    const std::string &map_field_name = "") {
        CYPHER_THROW_ASSERT(operand_type == ArithOperandNode::AR_OPERAND_VARIABLE);
        type = AR_EXP_OPERAND;
        operand.SetVariable(hasMapFieldName, value_alias, map_field_name);
    }

    void SetOperand(ArithOperandNode::ArithOperandType operand_type, const std::string &alias,
                    const std::string &property, const SymbolTable &sym_tab);

    void Set(const parser::Expression &expr, const SymbolTable &sym_tab);

    void RealignAliasId(const SymbolTable &sym_tab);

    /* Evaluate arithmetic expression tree. */
    Entry Evaluate(RTContext *ctx, const Record &record) const {
        if (type == AR_EXP_OPERAND) {
            return operand.Evaluate(ctx, record);
        } else if (type == AR_EXP_OP) {
            return op.Evaluate(ctx, record);
        } else {
            return EvaluateAstExpr(ctx, record);
        }
    }

    Entry EvaluateAstExpr(RTContext *ctx, const Record &record) const {
        return evaluator->Evaluate(ctx, &record);
    }

    void Aggregate(RTContext *ctx, const Record &record) {
        if (type == AR_EXP_OP) {
            if (op.type == ArithOpNode::AR_OP_AGGREGATE) {
                /* Process child nodes before aggregating. */
                std::vector<Entry> args;
                for (auto &c : op.children) args.emplace_back(c.Evaluate(ctx, record));
                /* Aggregate */
                // see https://stackoverflow.com/questions/74425128
                // Trying to understand: clang's side-effect warnings for typeid on a polymorphic
                // object
                auto &agg_ctx = *op.agg_func;
                if (typeid(agg_ctx) == typeid(CountStarAggCtx) && args.empty()) {
                    /* count(*), only count in when record is not null */
                    Entry count_in = record.Null()
                                         ? Entry(cypher::FieldData())
                                         : Entry(cypher::FieldData(lgraph::FieldData(1.0)));
                    args.emplace_back(count_in);
                }
                op.agg_func->Step(args);
            } else {
                /* Keep searching for aggregation nodes. */
                for (auto &child : op.children) {
                    child.Aggregate(ctx, record);
                }
            }
        } else if (type == AR_AST_EXP) {
            evaluator->Aggregate(ctx, &record);
        }
    }

    void Reduce() {
        if (type == AR_EXP_OP) {
            if (op.type == ArithOpNode::AR_OP_AGGREGATE) {
                /* Reduce */
                op.agg_func->ReduceNext();
            } else {
                /* Keep searching for aggregation nodes. */
                for (auto &child : op.children) {
                    child.Reduce();
                }
            }
        } else if (type == AR_AST_EXP) {
            evaluator->Reduce();
        }
    }

    bool ContainsAggregation() const {
        if (type == AR_EXP_OP) {
            if (op.type == ArithOpNode::AR_OP_AGGREGATE) return true;
            for (auto &child : op.children) {
                if (child.ContainsAggregation()) return true;
            }
        } else if (type == AR_AST_EXP) {
            AstAggExprDetector agg_expr_detector(expr_);
            return agg_expr_detector.Validate() && agg_expr_detector.HasValidAggFunc();
        }
        return false;
    }

    std::string ToString() const {
        if (type == AR_EXP_OPERAND) {
            return operand.ToString();
        } else if (type == AR_EXP_OP) {
            return op.ToString();
        } else {
            return cypher::ToString(expr_);
        }
    }
};
}  // namespace cypher
