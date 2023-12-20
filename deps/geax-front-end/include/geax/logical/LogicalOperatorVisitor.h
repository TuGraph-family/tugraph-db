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

#include <memory>
#include <any>

namespace geax::logical {

class LogicalAllNodesScan;
class LogicalNodeByLabelsScan;
class LogicalNodeSeek;
class LogicalExpandAll;
class LogicalVarLengthExpand;
class LogicalCreate;
class LogicalDelete;
class LogicalSetProperty;
class LogicalApply;
class LogicalSemiApply;
class LogicalAntiSemiApply;
class LogicalArgument;
class LogicalProcedureCall;
class LogicalCartesianProduct;
class LogicalProduceResults;
class LogicalOptional;
class LogicalFilter;
class LogicalProjection;
class LogicalSort;
class LogicalLimit;
class LogicalSkip;
class LogicalDistinct;
class LogicalAggregation;
class LogicalJoin;
class LogicalSemiJoin;
class LogicalAntiJoin;
class LogicalUnion;
class LogicalIntersect;
class LogicalMinus;
class LogicalGetEdges;
class LogicalExtractEdge;

class LogicalOperatorVisitor {
 public:
    LogicalOperatorVisitor() = default;
    virtual ~LogicalOperatorVisitor() = default;
    virtual std::any visit(std::shared_ptr<LogicalAllNodesScan> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalNodeByLabelsScan> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalNodeSeek> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalExpandAll> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalVarLengthExpand> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalCreate> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalDelete> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalSetProperty> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalApply> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalSemiApply> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalAntiSemiApply> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalArgument> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalProcedureCall> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalCartesianProduct> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalProduceResults> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalOptional> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalFilter> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalProjection> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalSort> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalLimit> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalSkip> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalDistinct> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalAggregation> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalJoin> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalSemiJoin> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalAntiJoin> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalUnion> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalIntersect> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalMinus> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalGetEdges> op) = 0;
    virtual std::any visit(std::shared_ptr<LogicalExtractEdge> op) = 0;
};

}  // namespace geax::logical
