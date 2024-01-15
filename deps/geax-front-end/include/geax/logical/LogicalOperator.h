/**
 * Copyright 2023 AntGroup CO., Ltd.
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

#include <iostream>
#include <cstdint>
#include <memory>
#include <vector>
#include <string>

#include "geax/logical/LogicalOperatorVisitor.h"
#include "geax-front-end/GEAXErrorCode.h"

namespace geax::logical {

enum class LogicalOperatorType : uint32_t {
    // core
    AllNodesScan,
    NodeByLabelsScan,
    NodeSeek,
    ExpandAll,
    VarLengthExpand,
    Create,
    Delete,
    SetProperty,
    Apply,
    SemiApply,
    AntiSemiApply,
    Argument,
    ProcedureCall,
    CartesianProduct,
    ProduceResults,
    Optional,
    Filter,
    Projection,
    Sort,
    Limit,
    Skip,
    Distinct,
    Aggregation,
    // extensions
    Join,
    SemiJoin,
    AntiJoin,
    Union,
    Intersect,
    Minus,
    GetEdges,
    ExtractEdge,
};

class LogicalOperator : public std::enable_shared_from_this<LogicalOperator> {
 public:
    explicit LogicalOperator(LogicalOperatorType type) : type_(type) {}
    virtual ~LogicalOperator() = default;
    virtual std::any accept(LogicalOperatorVisitor* visitor) = 0;
    virtual std::string toString() const = 0;
    LogicalOperatorType type() const { return type_; }
    void addChild(std::shared_ptr<LogicalOperator> child) {
        children_.push_back(child);
        child->parent_ = shared_from_this();
    }
    const std::vector<std::shared_ptr<LogicalOperator>>& children() const {
        return children_;
    }
    std::weak_ptr<LogicalOperator> parent() const { return parent_; }

 protected:
    LogicalOperatorType type_;
    std::vector<std::shared_ptr<LogicalOperator>> children_;
    std::weak_ptr<LogicalOperator> parent_;
};

};  // namespace geax::logical
