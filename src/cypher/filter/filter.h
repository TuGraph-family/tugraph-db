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
// Created by wt on 6/29/18.
//
#pragma once

#include <memory>
#include "core/lightning_graph.h"
#include "resultset/record.h"
#include "arithmetic/arithmetic_expression.h"
#include "execution_plan/ops/op.h"
#include "cypher/cypher_types.h"
#include "cypher/filter/iterator.h"
#include "lgraph/lgraph_date_time.h"
#include "utils/geax_util.h"

namespace cypher {
class LocateNodeByVid;
class LocateNodeByIndexedProp;
}  // namespace cypher

namespace lgraph {

struct FieldDataHash {
    size_t operator()(const lgraph::FieldData &fd) const {
        switch (fd.type) {
        case FieldType::NUL:
            return 0;
        case FieldType::BOOL:
            return std::hash<bool>()(fd.AsBool());
        case FieldType::INT8:
            return std::hash<int8_t>()(fd.AsInt8());
        case FieldType::INT16:
            return std::hash<int16_t>()(fd.AsInt16());
        case FieldType::INT32:
            return std::hash<int32_t>()(fd.AsInt32());
        case FieldType::INT64:
            return std::hash<int64_t>()(fd.AsInt64());
        case FieldType::FLOAT:
            return std::hash<float>()(fd.AsFloat());
        case FieldType::DOUBLE:
            return std::hash<double>()(fd.AsDouble());
        case FieldType::DATE:
            return std::hash<int32_t>()(fd.AsDate().DaysSinceEpoch());
        case FieldType::DATETIME:
            return std::hash<int64_t>()(fd.AsDateTime().MicroSecondsSinceEpoch());
        case FieldType::STRING:
            return std::hash<std::string>()(fd.AsString());
        case FieldType::BLOB:
            return std::hash<std::string>()(fd.AsBlob());
        case FieldType::POINT:
            {
                switch (fd.GetSRID()) {
                case ::lgraph_api::SRID::WGS84:
                    return std::hash<std::string>()(fd.AsWgsPoint().AsEWKB());
                case ::lgraph_api::SRID::CARTESIAN:
                    return std::hash<std::string>()(fd.AsCartesianPoint().AsEWKB());
                default:
                    THROW_CODE(InputError, "unsupported spatial srid");
                }
            }
        case FieldType::LINESTRING:
            {
                switch (fd.GetSRID()) {
                case ::lgraph_api::SRID::WGS84:
                    return std::hash<std::string>()(fd.AsWgsLineString().AsEWKB());
                case ::lgraph_api::SRID::CARTESIAN:
                    return std::hash<std::string>()(fd.AsCartesianLineString().AsEWKB());
                default:
                    THROW_CODE(InputError, "unsupported spatial srid");
                }
            }
        case FieldType::POLYGON:
            {
                switch (fd.GetSRID()) {
                case ::lgraph_api::SRID::WGS84:
                    return std::hash<std::string>()(fd.AsWgsPolygon().AsEWKB());
                case ::lgraph_api::SRID::CARTESIAN:
                    return std::hash<std::string>()(fd.AsCartesianPolygon().AsEWKB());
                default:
                    THROW_CODE(InputError, "unsupported spatial srid");
                }
            }
        case FieldType::SPATIAL:
            {
                switch (fd.GetSRID()) {
                case ::lgraph_api::SRID::WGS84:
                    return std::hash<std::string>()(fd.AsWgsSpatial().AsEWKB());
                case ::lgraph_api::SRID::CARTESIAN:
                    return std::hash<std::string>()(fd.AsCartesianSpatial().AsEWKB());
                default:
                    THROW_CODE(InputError, "unsupported spatial srid");
                }
            }
        default:
            throw std::runtime_error("Unhandled data type, probably corrupted data.");
        }
    }
};

class Filter {
 public:
    enum Type {
        EMPTY,
        UNARY,
        BINARY,
        RANGE_FILTER,
        TEST_NULL_FILTER,
        TEST_IN_FILTER,
        TEST_EXISTS_FILTER,
        LABEL_FILTER,
        STRING_FILTER,
        // todo (kehuang): AstExpr is currently temporarily used as a type of Filter, and will be
        // replaced with AstExpr in the future.
        GEAX_EXPR_FILTER,
    };

    static inline std::string ToString(const Type &type) {
        switch (type) {
        case Type::EMPTY:
            return "EMPTY";
        case Type::UNARY:
            return "UNARY";
        case Type::BINARY:
            return "BINARY";
        case Type::RANGE_FILTER:
            return "RANGE_FILTER";
        case Type::TEST_NULL_FILTER:
            return "TEST_NULL_FILTER";
        case Type::TEST_IN_FILTER:
            return "TEST_IN_FILTER";
        case Type::TEST_EXISTS_FILTER:
            return "TEST_EXISTS_FILTER";
        case Type::LABEL_FILTER:
            return "LABEL_FILTER";
        case Type::STRING_FILTER:
            return "STRING_FILTER";
        case Type::GEAX_EXPR_FILTER:
            return "GEAX_EXPR_FILTER";
        default:
            throw lgraph::CypherException("unknown RecordEntryType");
        }
    }

    // disable copy constructor & assignment
    Filter(const Filter &rhs) = delete;

    Filter &operator=(const Filter &rhs) = delete;

    Filter &operator=(Filter &&rhs) = delete;

    Filter() : _type(EMPTY), _logical_op(lgraph::LogicalOp::LBR_EMPTY) {}

    Filter(lgraph::LogicalOp logical_op, const std::shared_ptr<Filter> &unary)
        : _type(UNARY), _logical_op(logical_op), _left(unary) {}

    Filter(lgraph::LogicalOp logical_op, const std::shared_ptr<Filter> &left,
           const std::shared_ptr<Filter> &right)
        : _type(BINARY), _logical_op(logical_op), _left(left), _right(right) {}

    Filter(Filter &&rhs) noexcept
        : _type(rhs._type),
          _logical_op(rhs._logical_op),
          _left(std::move(rhs._left)),
          _right(std::move(rhs._right)) {
        rhs._type = EMPTY;
    }

    virtual ~Filter() = default;

    virtual std::shared_ptr<Filter> Clone() const {
        auto clone = std::make_shared<Filter>();
        clone->_type = _type;
        clone->_logical_op = _logical_op;
        clone->_left = _left == nullptr ? nullptr : _left->Clone();
        clone->_right = _right == nullptr ? nullptr : _right->Clone();
        return clone;
    }

    const std::shared_ptr<Filter> &Left() const { return _left; }
    std::shared_ptr<Filter> &Left() { return _left; }

    const std::shared_ptr<Filter> &Right() const { return _right; }
    std::shared_ptr<Filter> &Right() { return _right; }

    Type Type() const { return _type; }

    lgraph::LogicalOp LogicalOp() const { return _logical_op; }

    virtual std::set<std::string> Alias() const {
        std::set<std::string> ret;
        switch (_type) {
        case EMPTY:
            return ret;
        case UNARY:
            return _left->Alias();
        case BINARY:
            {
                ret = _left->Alias();
                auto ret_r = _right->Alias();
                ret.insert(ret_r.begin(), ret_r.end());
                return ret;
            }
        default:
            return ret;
        }
    }

    virtual std::set<std::pair<std::string, std::string>> VisitedFields() const {
        throw lgraph::CypherException(fma_common::StringFormatter::Format(
            "Filter Type:{} VisitedFields not implemented yet", Type()));
    }

    /* For filter tree: If there are sub-filter(s) completely contained in ALIASES.
     * For leaf filter: If the filter is completely contained in ALIASES.
     * */
    virtual bool ContainAlias(const std::vector<std::string> &aliases) const {
        switch (_logical_op) {
        case lgraph::LBR_EMPTY:
        case lgraph::LBR_NOT:
            return _left && _left->ContainAlias(aliases);
        case lgraph::LBR_AND:
        case lgraph::LBR_OR:
        case lgraph::LBR_XOR:
            return _left && _right &&
                   (_left->ContainAlias(aliases) || _right->ContainAlias(aliases));
        default:
            return false;
        }
    }

    virtual void RealignAliasId(const cypher::SymbolTable &sym_tab) {
        if (_left) _left->RealignAliasId(sym_tab);
        if (_right) _right->RealignAliasId(sym_tab);
    }

    bool Empty() const { return _type == EMPTY; }

    bool IsLeaf() const { return _type != EMPTY && _type != UNARY && _type != BINARY; }

    // Test whether only contains logical AND for all binary logical op.
    bool BinaryOnlyContainsAND() const {
        if (_type == BINARY && LogicalOp() != lgraph::LogicalOp::LBR_AND) {
            return false;
        }
        return (_left == nullptr || _left->BinaryOnlyContainsAND()) &&
               (_right == nullptr || _right->BinaryOnlyContainsAND());
    }

    /* Remove sub-filter nodes that are completely contained by the ALIAS.
     * e.g.
     * input filter: {a.uid > d.uid}&&{b.uid < c.uid}
     * alias: {a, b, c}
     * output filter: {a.uid > d.uid}
     * */
    static void RemoveAlias(std::shared_ptr<Filter> &f, const std::vector<std::string> &alias) {
        RemoveFilterWhen(f, [&alias](const auto &b, const auto &e) {
            std::vector<std::string> vs;
            std::set_intersection(b, e, alias.cbegin(), alias.cend(), std::back_inserter(vs));
            return vs.size() == (size_t)std::distance(b, e);
        });
    }

    /// Remove sub-filter nodes that doesn't meet condition `apply`
    template <typename F>
    static void RemoveFilterWhen(std::shared_ptr<Filter> &f, F &&apply) {
        if (f->_type == EMPTY) return;
        if (f->IsLeaf()) {
            auto fa = f->Alias();
            if (apply(fa.cbegin(), fa.cend())) f = nullptr;
            return;
        }
        if (f->_type == UNARY) {
            RemoveFilterWhen(f->_left, apply);
            /* squash */
            if (f->_left == nullptr) f = nullptr;
            return;
        }
        if (f->_type == BINARY) {
            // if (f->LogicalOp() != lgraph::LogicalOp::LBR_AND)
            //     CYPHER_TODO();  // only handle AND-tree
            RemoveFilterWhen(f->_left, apply);
            RemoveFilterWhen(f->_right, apply);
            /* squash */
            if (!f->_left && !f->_right) {
                f = nullptr;
            } else if (f->_left && !f->_right) {
                f = f->_left;
            } else if (!f->_left && f->_right) {
                f = f->_right;
            }
            return;
        }
    }

    virtual bool DoFilter(cypher::RTContext *ctx, const cypher::Record &record) {
        switch (_logical_op) {
        case lgraph::LBR_EMPTY:
            return _left && _left->DoFilter(ctx, record);
        case lgraph::LBR_NOT:
            return _left && !_left->DoFilter(ctx, record);
        case lgraph::LBR_AND:
            return _left && _right &&
                   (_left->DoFilter(ctx, record) && _right->DoFilter(ctx, record));
        case lgraph::LBR_OR:
            return _left && _right &&
                   (_left->DoFilter(ctx, record) || _right->DoFilter(ctx, record));
        case lgraph::LBR_XOR:
            return _left && _right &&
                   (_left->DoFilter(ctx, record) != _right->DoFilter(ctx, record));
        default:
            return false;
        }
    }

    virtual const std::string ToString() const {
        static const std::string null_str = "null";
        std::string left_str, right_str;
        switch (_logical_op) {
        case lgraph::LBR_EMPTY:
            return _left == nullptr ? null_str : _left->ToString();
        case lgraph::LBR_NOT:
            left_str = _left == nullptr ? null_str : _left->ToString();
            return "!" + left_str;
        case lgraph::LBR_AND:
            left_str = _left == nullptr ? null_str : _left->ToString();
            right_str = _right == nullptr ? null_str : _right->ToString();
            return ("(" + left_str + "&&" + right_str + ")");
        case lgraph::LBR_OR:
            left_str = _left == nullptr ? null_str : _left->ToString();
            right_str = _right == nullptr ? null_str : _right->ToString();
            return ("(" + left_str + "||" + right_str + ")");
        case lgraph::LBR_XOR:
            left_str = _left == nullptr ? null_str : _left->ToString();
            right_str = _right == nullptr ? null_str : _right->ToString();
            return ("(" + left_str + "^" + right_str + ")");
        default:
            return "";
        }
    }

 protected:
    enum Type _type = EMPTY;

 private:
    lgraph::LogicalOp _logical_op;
    std::shared_ptr<Filter> _left = nullptr;  // also used for unary operation
    std::shared_ptr<Filter> _right = nullptr;
    const cypher::SymbolTable *sym_tab_ = nullptr;
};

static std::set<std::string> ExtractAlias(const cypher::ArithExprNode &ae) {
    std::set<std::string> ret;
    if (ae.type == cypher::ArithExprNode::AR_EXP_OPERAND &&
        ae.operand.type == cypher::ArithOperandNode::AR_OPERAND_VARIADIC) {
        ret.emplace(ae.operand.variadic.alias);
    } else if (ae.type == cypher::ArithExprNode::AR_EXP_OP &&
               (ae.op.type == cypher::ArithOpNode::AR_OP_FUNC ||
                ae.op.type == cypher::ArithOpNode::AR_OP_MATH)) {
        for (auto &c : ae.op.children) {
            if (c.type == cypher::ArithExprNode::AR_EXP_OP) {
                /* The argument of the op is AR_EXP_OP, e.g. abs(r.weight - 20.21)  */
                auto sa = ExtractAlias(c);
                ret.insert(sa.begin(), sa.end());
            } else if (c.operand.type != cypher::ArithOperandNode::AR_OPERAND_CONSTANT) {
                /* Exclude constants such as the 1st argument DISTINCT_OR_NOT */
                ret.emplace(c.operand.variadic.alias);
            }
        }
    } else if (ae.type == cypher::ArithExprNode::AR_EXP_OP &&
               ae.op.type == cypher::ArithOpNode::AR_OP_AGGREGATE) {
        CYPHER_TODO();
    } else {
        // return empty
    }
    return ret;
}

static std::set<std::pair<std::string, std::string>> ExtractExpr(const cypher::ArithExprNode &ae) {
    std::set<std::pair<std::string, std::string>> ret;
    if (ae.type == cypher::ArithExprNode::AR_EXP_OPERAND &&
        ae.operand.type == cypher::ArithOperandNode::AR_OPERAND_VARIADIC) {
        if (!ae.operand.variadic.entity_prop.empty())
            ret.emplace(std::make_pair(ae.operand.variadic.alias, ae.operand.variadic.entity_prop));
    }
    return ret;
}

class RangeFilter : public Filter {
    lgraph::CompareOp _compare_op;    // < <= > >= = !=
    cypher::ArithExprNode _ae_left;   // can be: n, n.name, id(n), etc.
    cypher::ArithExprNode _ae_right;  // value to compare against
    const cypher::SymbolTable *sym_tab_;

 public:
    RangeFilter() { _type = RANGE_FILTER; }

    RangeFilter(lgraph::CompareOp compare_op, const cypher::ArithExprNode &ae_left,
                const cypher::ArithExprNode &ae_right)
        : _compare_op(compare_op), _ae_left(ae_left), _ae_right(ae_right) {
        _type = RANGE_FILTER;
        sym_tab_ = nullptr;
    }

    RangeFilter(lgraph::CompareOp compare_op, const cypher::ArithExprNode &ae_left,
                const cypher::ArithExprNode &ae_right, const cypher::SymbolTable *sym_tab)
        : _compare_op(compare_op), _ae_left(ae_left), _ae_right(ae_right), sym_tab_(sym_tab) {
        _type = RANGE_FILTER;
    }
    RangeFilter(RangeFilter &&rhs) noexcept
        : _compare_op(rhs._compare_op),
          _ae_left(std::move(rhs._ae_left)),
          _ae_right(std::move(rhs._ae_right)) {
        _type = rhs._type;
    }

    std::shared_ptr<Filter> Clone() const override {
        auto clone = std::make_shared<RangeFilter>(_compare_op, _ae_left, _ae_right);
        return clone;
    }

    std::set<std::string> Alias() const override {
        std::set<std::string> ret;
        auto a = ExtractAlias(_ae_left);
        ret.insert(a.begin(), a.end());
        auto b = ExtractAlias(_ae_right);
        ret.insert(b.begin(), b.end());
        return ret;
    }

    std::set<std::pair<std::string, std::string>> VisitedFields() const override {
        std::set<std::pair<std::string, std::string>> ret;
        auto a = ExtractExpr(_ae_left);
        ret.insert(a.begin(), a.end());
        auto b = ExtractExpr(_ae_right);
        ret.insert(b.begin(), b.end());
        return ret;
    }

    /*
     * For filter tree: If there are sub-filter(s) completely contained in ALIASES.
     * For leaf filter: If the filter is completely contained in ALIASES.
     * */
    bool ContainAlias(const std::vector<std::string> &alias) const override {
        auto a1 = Alias();
        if (a1.empty()) {
            /* a. constant filter: where 1 = 2
             * b. invalid args: where type(4) = 'ACTED_IN'  */
            return false;
        }
        std::set<std::string> a2(alias.begin(), alias.end());
        std::vector<std::string> vs;
        std::set_intersection(a1.begin(), a1.end(), a2.begin(), a2.end(), std::back_inserter(vs));
        return vs.size() == a1.size();
    }

    void RealignAliasId(const cypher::SymbolTable &sym_tab) override {
        _ae_left.RealignAliasId(sym_tab);
        _ae_right.RealignAliasId(sym_tab);
    }

    bool DoFilter(cypher::RTContext *ctx, const cypher::Record &record) override {
        auto left = _ae_left.Evaluate(ctx, record);
        cypher::Entry right;
        if (_ae_right.type == cypher::ArithExprNode::AR_EXP_OPERAND) {
            switch (_ae_right.operand.type) {
            case cypher::ArithOperandNode::AR_OPERAND_VARIABLE:
                {
                    if (sym_tab_ == nullptr) {
                        throw lgraph::CypherException("Filter sym_tab is nullptr");
                    }
                    auto value_alias = _ae_right.operand.variable._value_alias;
                    auto map_field_name = _ae_right.operand.variable._map_field_name;
                    auto hasMapFieldName = _ae_right.operand.variable.hasMapFieldName;
                    auto it = sym_tab_->symbols.find(value_alias);
                    if (it == sym_tab_->symbols.end()) {
                        throw lgraph::CypherException("Undefined variable: " + value_alias);
                    }
                    int value_rec_idx_ = it->second.id;
                    cypher::FieldData constant = record.values[value_rec_idx_].constant;

                    if (hasMapFieldName) {
                        auto map = constant.map;
                        if (map->find(map_field_name) == map->end()) {
                            throw lgraph::CypherException("Undefined map_field_name: "
                                                        + map_field_name);
                        }
                        right = cypher::Entry(map->at(map_field_name).scalar);
                    } else {
                        right = cypher::Entry(constant);
                    }
                    break;
                }
            default:
                {
                    right = _ae_right.Evaluate(ctx, record);
                    break;
                }
            }
        } else {
            right = _ae_right.Evaluate(ctx, record);
        }
        if (left.type != right.type) return false;
        switch (_compare_op) {
        case lgraph::LBR_EQ:
            return left == right;
        case lgraph::LBR_NEQ:
            return left != right;
        case lgraph::LBR_LT:
            return left < right;
        case lgraph::LBR_GT:
            return left > right;
        case lgraph::LBR_LE:
            return !(left > right);
        case lgraph::LBR_GE:
            return !(left < right);
        default:
            return false;
        }
    }

    lgraph::CompareOp GetCompareOp() { return _compare_op; }
    const cypher::ArithExprNode &GetAeLeft() { return _ae_left; }
    const cypher::ArithExprNode &GetAeRight() { return _ae_right; }

    static std::map<lgraph::CompareOp, std::string> _compare_name;

    const std::string ToString() const override {
        std::string str("{");
        str.append(_ae_left.ToString())
            .append(" ")
            .append(_compare_name[_compare_op])
            .append(" ")
            .append(_ae_right.ToString())
            .append("}");
        return str;
    }
};

class TestNullFilter : public Filter {
    bool compare_op_test_is_null_ = true;  // 1: is null, 0: is not null
    cypher::ArithExprNode ae_left_;        // can be: n, n.name, id(n), etc.

 public:
    TestNullFilter() { _type = TEST_NULL_FILTER; }

    TestNullFilter(bool test_is_null, const cypher::ArithExprNode &ae_left)
        : compare_op_test_is_null_(test_is_null), ae_left_(ae_left) {
        _type = TEST_NULL_FILTER;
    }

    TestNullFilter(TestNullFilter &&rhs) noexcept
        : compare_op_test_is_null_(rhs.compare_op_test_is_null_),
          ae_left_(std::move(rhs.ae_left_)) {
        _type = rhs._type;
    }

    std::shared_ptr<Filter> Clone() const override {
        auto clone = std::make_shared<TestNullFilter>(compare_op_test_is_null_, ae_left_);
        return clone;
    }

    std::set<std::string> Alias() const override { return ExtractAlias(ae_left_); }

    bool ContainAlias(const std::vector<std::string> &alias) const override {
        auto a1 = Alias();
        if (a1.empty()) {
            /* constant filter: where 2 is null  */
            return false;
        }
        std::set<std::string> a2(alias.begin(), alias.end());
        std::vector<std::string> vs;
        std::set_intersection(a1.begin(), a1.end(), a2.begin(), a2.end(), std::back_inserter(vs));
        return vs.size() == a1.size();
    }

    void RealignAliasId(const cypher::SymbolTable &sym_tab) override {
        ae_left_.RealignAliasId(sym_tab);
    }

    bool DoFilter(cypher::RTContext *ctx, const cypher::Record &record) override {
        auto left = ae_left_.Evaluate(ctx, record);
        return compare_op_test_is_null_ == left.constant.EqualNull();
    }

    const std::string ToString() const override {
        std::string str("{");
        str.append(ae_left_.ToString())
            .append(compare_op_test_is_null_ ? " IS NULL" : " IS NOT NULL")
            .append("}");
        return str;
    }
};

class TestInFilter : public Filter {
    cypher::ArithExprNode ae_left_;
    cypher::ArithExprNode ae_right_;
    std::unordered_set<lgraph::FieldData, FieldDataHash> right_set_;
    cypher::OpBase *producer_op_ = nullptr;
    size_t timestamp;

    friend class cypher::LocateNodeByVid;
    friend class cypher::LocateNodeByIndexedProp;

 public:
    TestInFilter() { _type = TEST_IN_FILTER; }

    TestInFilter(const cypher::ArithExprNode &ae_left, const cypher::ArithExprNode &ae_right)
        : ae_left_(ae_left), ae_right_(ae_right), timestamp(0) {
        _type = TEST_IN_FILTER;
    }

    TestInFilter(TestInFilter &&rhs) noexcept
        : ae_left_(std::move(rhs.ae_left_)), ae_right_(std::move(rhs.ae_right_)) {
        _type = rhs._type;
    }

    void SetProducerOp(cypher::OpBase *producer_op) { producer_op_ = producer_op; }

    std::shared_ptr<Filter> Clone() const override {
        auto clone = std::make_shared<TestInFilter>(ae_left_, ae_right_);
        return clone;
    }

    std::set<std::string> Alias() const override {
        std::set<std::string> ret;
        auto a = ExtractAlias(ae_left_);
        ret.insert(a.begin(), a.end());
        return ret;
    }

    std::set<std::string> RhsAlias() { return ExtractAlias(ae_right_); }

    bool ContainAlias(const std::vector<std::string> &alias) const override {
        auto a1 = Alias();
        if (a1.empty()) return false;
        std::set<std::string> a2(alias.begin(), alias.end());
        std::vector<std::string> vs;
        std::set_intersection(a1.begin(), a1.end(), a2.begin(), a2.end(), std::back_inserter(vs));
        return vs.size() == a1.size();
    }

    void RealignAliasId(const cypher::SymbolTable &sym_tab) override {
        ae_left_.RealignAliasId(sym_tab);
        ae_right_.RealignAliasId(sym_tab);
    }

    bool DoFilter(cypher::RTContext *ctx, const cypher::Record &record) override {
        auto left = ae_left_.Evaluate(ctx, record);
        auto right = ae_right_.Evaluate(ctx, record);

        if (!left.IsScalar() || !right.IsArray()) CYPHER_TODO();

        // optimize with hash_set
        if (producer_op_ != nullptr) {
            // producer_op_ != nullptr means it can be optimized with hash_set
            if (timestamp != producer_op_->stats.profileRecordCount) {
                right_set_.clear();
                for (auto &r : *right.constant.array) {
                    right_set_.emplace(r.scalar);
                }
                timestamp++;
#ifndef NDEBUG
                LOG_DEBUG() << "[" << __FILE__ << "] " << "reset set: ";
                for (auto it = right_set_.begin(); it != right_set_.end(); ++it) {
                    LOG_DEBUG() << (*it).ToString();
                }
#endif
            }
            if (right_set_.find(left.constant.scalar) != right_set_.end()) {
                return true;
            }
        } else {
            // only process argument in a loop
            LOG_WARN() << "[" << __FILE__ << "] " << "do not use unordered_set";
            for (auto &r : *right.constant.array) {
                if (left.constant.scalar == r.scalar) return true;
            }
        }

        return false;
    }

    const std::string ToString() const override {
        std::string str("{");
        str.append(ae_left_.ToString()).append(" IN ").append(ae_right_.ToString()).append("}");
        return str;
    }
};

class TestExists : public Filter {
    cypher::ArithOperandNode property_;                               // e.g. n.name
    std::shared_ptr<cypher::PatternGraph> nested_pattern_ = nullptr;  // e.g. (n)-[:MARRIED]->()
    cypher::OpBase *nested_plan_ = nullptr;

    void BuildNestedExecutionPlan();

 public:
    TestExists() { _type = TEST_EXISTS_FILTER; }

    explicit TestExists(const cypher::ArithOperandNode &property) : property_(property) {
        _type = TEST_EXISTS_FILTER;
    }

    TestExists(const cypher::SymbolTable &sym_tab,
               const std::shared_ptr<cypher::PatternGraph> &pattern);

    TestExists(TestExists &&rhs) noexcept
        : property_(std::move(rhs.property_)),
          nested_pattern_(rhs.nested_pattern_),
          nested_plan_(rhs.nested_plan_) {
        _type = rhs._type;
        rhs.nested_pattern_ = nullptr;
        rhs.nested_plan_ = nullptr;
    }

    ~TestExists() override;

    std::shared_ptr<Filter> Clone() const override {
        auto clone = std::make_shared<TestExists>();
        clone->property_ = property_;
        clone->nested_pattern_ = nested_pattern_;
        if (nested_plan_) {
            clone->BuildNestedExecutionPlan();
        }
        return clone;
    }

    std::set<std::string> Alias() const override {
        // extract the non-anonymous alias of pattern
        if (nested_plan_) {
            std::set<std::string> ret;
            std::string anonymous_prefix(parser::ANONYMOUS);
            for (auto &n : nested_pattern_->GetNodes()) {
                if (n.Alias().compare(0, anonymous_prefix.size(), anonymous_prefix) != 0) {
                    ret.emplace(n.Alias());
                }
            }
            for (auto &r : nested_pattern_->GetRelationships()) {
                if (r.Alias().compare(0, anonymous_prefix.size(), anonymous_prefix) != 0) {
                    ret.emplace(r.Alias());
                }
            }
            return ret;
        }
        return std::set<std::string>{property_.variadic.alias};
    }

    bool ContainAlias(const std::vector<std::string> &alias) const override {
        auto a1 = Alias();
        if (a1.empty()) {
            /* constant filter: where 2 is null  */
            return false;
        }
        std::set<std::string> a2(alias.begin(), alias.end());
        std::vector<std::string> vs;
        std::set_intersection(a1.begin(), a1.end(), a2.begin(), a2.end(), std::back_inserter(vs));
        return vs.size() == a1.size();
    }

    bool DoFilter(cypher::RTContext *ctx, const cypher::Record &record) override;

    const std::string ToString() const override {
        std::string str("{EXISTS(");
        if (!nested_plan_) {
            str.append(property_.ToString()).append(")}");
        } else {
            str.append("NestedPattern(");
            for (auto &node : nested_pattern_->GetNodes()) str.append(node.Alias()).append(",");
            str.pop_back();
            str.append("))}");
        }
        return str;
    }
};

class LabelFilter : public Filter {
    std::string _alias;             // variable
    std::set<std::string> _labels;  // labels

 public:
    LabelFilter() { _type = LABEL_FILTER; }

    LabelFilter(const std::string &alias, const std::set<std::string> &labels)
        : _alias(alias), _labels(labels) {
        _type = LABEL_FILTER;
    }

    std::shared_ptr<Filter> Clone() const override {
        auto clone = std::make_shared<LabelFilter>(_alias, _labels);
        return clone;
    }

    std::set<std::string> Alias() const override { return std::set<std::string>{_alias}; }

    bool ContainAlias(const std::vector<std::string> &alias) const override {
        for (auto &a : alias) {
            if (a == _alias) return true;
        }
        return false;
    }

    bool DoFilter(cypher::RTContext *ctx, const cypher::Record &record) override {
        CYPHER_THROW_ASSERT(record.symbol_table && record.symbol_table->symbols.find(_alias) !=
                                                       record.symbol_table->symbols.end());
        auto idx = record.symbol_table->symbols.find(_alias)->second.id;
        CYPHER_THROW_ASSERT(record.values[idx].type == cypher::Entry::NODE);
        auto node = record.values[idx].node;
        CYPHER_THROW_ASSERT(node && node->ItRef());
        if (!node->IsValidAfterMaterialize(ctx)) return false;
        auto l = node->ItRef()->GetLabel();
        return _labels.find(l) != _labels.end();
    }

    const std::string ToString() const override {
        std::string str("{");
        str.append(_alias).append(":").append(fma_common::ToString(_labels)).append("}");
        return str;
    }
};

class StringFilter : public Filter {
 public:
    enum Comparison {
        STARTS_WITH,
        ENDS_WITH,
        CONTAINS,
        REGEXP,
    } compare_op = STARTS_WITH;
    cypher::ArithExprNode lhs;  // e.g. n.name
    cypher::ArithExprNode rhs;

    StringFilter() { _type = STRING_FILTER; }

    StringFilter(Comparison comp_op, const cypher::ArithExprNode &l, const cypher::ArithExprNode &r)
        : compare_op(comp_op), lhs(l), rhs(r) {
        _type = STRING_FILTER;
    }

    std::shared_ptr<Filter> Clone() const override {
        auto clone = std::make_shared<StringFilter>(compare_op, lhs, rhs);
        return clone;
    }

    std::set<std::string> Alias() const override {
        std::set<std::string> ret;
        auto a = ExtractAlias(lhs);
        ret.insert(a.begin(), a.end());
        auto b = ExtractAlias(rhs);
        ret.insert(b.begin(), b.end());
        return ret;
    }

    bool ContainAlias(const std::vector<std::string> &alias) const override {
        auto a1 = Alias();
        if (a1.empty()) {
            return false;
        }
        std::set<std::string> a2(alias.begin(), alias.end());
        std::vector<std::string> vs;
        std::set_intersection(a1.begin(), a1.end(), a2.begin(), a2.end(), std::back_inserter(vs));
        return vs.size() == a1.size();
    }

    void RealignAliasId(const cypher::SymbolTable &sym_tab) override {
        lhs.RealignAliasId(sym_tab);
        rhs.RealignAliasId(sym_tab);
    }

    bool DoFilter(cypher::RTContext *ctx, const cypher::Record &record) override;

    const std::string ToString() const override {
        static const std::map<Comparison, std::string> compare_name = {
            {STARTS_WITH, "STARTS WITH"},
            {ENDS_WITH, "ENDS WITH"},
            {CONTAINS, "CONTAINS"},
            {REGEXP, "REGEXP"},
        };
        std::string str("{");
        str.append(lhs.ToString())
            .append(" ")
            .append(compare_name.at(compare_op))
            .append(" ")
            .append(rhs.ToString())
            .append("}");
        return str;
    }
};

class GeaxExprFilter : public Filter {
 private:
    cypher::ArithExprNode arith_expr_;

 public:
    GeaxExprFilter(geax::frontend::Expr *expr, const cypher::SymbolTable &sym_tab) {
        arith_expr_.SetAstExp(expr, sym_tab);
        _type = GEAX_EXPR_FILTER;
    }

    cypher::ArithExprNode &GetArithExpr() { return arith_expr_; }

    std::shared_ptr<Filter> Clone() const override { NOT_SUPPORT_AND_THROW(); }

    std::set<std::string> Alias() const override { NOT_SUPPORT_AND_THROW(); }

    std::set<std::pair<std::string, std::string>> VisitedFields() const override {
        NOT_SUPPORT_AND_THROW();
    }

    bool ContainAlias(const std::vector<std::string> &alias) const override {
        NOT_SUPPORT_AND_THROW();
    }

    void RealignAliasId(const cypher::SymbolTable &sym_tab) override { NOT_SUPPORT_AND_THROW(); }

    bool DoFilter(cypher::RTContext *ctx, const cypher::Record &record) override {
        auto res = arith_expr_.Evaluate(ctx, record);
        if (res.IsBool()) {
            return res.constant.scalar.AsBool();
        }
        NOT_SUPPORT_AND_THROW();
    }

    const std::string ToString() const override { return arith_expr_.ToString(); }
};

}  // namespace lgraph
