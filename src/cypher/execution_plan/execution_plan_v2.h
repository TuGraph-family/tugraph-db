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

#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/common/ObjectAllocator.h"
#include "cypher/execution_plan/ops/op.h"

namespace cypher {
class ExecutionPlanV2 {
 public:
    ExecutionPlanV2() = default;
    ~ExecutionPlanV2();
    geax::frontend::GEAXErrorCode Build(geax::frontend::AstNode* astNode, RTContext* ctx);
    int Execute(RTContext* ctx);
    std::string DumpPlan(int indent, bool statistics) const;
    std::string DumpGraph() const;
    std::string ErrorMsg();
    OpBase* Root();
    void SetReadOnly(bool read_only) { read_only_ = read_only; }

 private:
    bool read_only_ = true;
    ResultInfo result_info_;
    std::vector<PatternGraph> pattern_graphs_;
    OpBase* root_ = nullptr;
    std::string error_msg_;
    geax::common::ObjectArenaAllocator obj_alloc_;

 private:
    DISABLE_COPY(ExecutionPlanV2);
    DISABLE_MOVE(ExecutionPlanV2);
    bool ReadOnly() const;
    void Reset();
};

}  // namespace cypher
