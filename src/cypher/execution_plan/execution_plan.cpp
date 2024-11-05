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

#include "cypher/execution_plan/execution_plan_maker.h"
#include "cypher/execution_plan/pattern_graph_maker.h"
#include "cypher/execution_plan/execution_plan.h"
#include "common/logger.h"

namespace cypher {

ExecutionPlan::~ExecutionPlan() { OpBase::FreeStream(root_); }

geax::frontend::GEAXErrorCode ExecutionPlan::Build(geax::frontend::AstNode* astNode,
                                                   RTContext* ctx) {
    geax::frontend::GEAXErrorCode ret = geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    // build pattern graph
    PatternGraphMaker pattern_graph_maker(pattern_graphs_);
    ret = pattern_graph_maker.Build(astNode, ctx);
    if (ret != geax::frontend::GEAXErrorCode::GEAX_SUCCEED) {
        error_msg_ = pattern_graph_maker.ErrorMsg();
        return ret;
    }
    LOG_DEBUG("\n{}", DumpGraph());
    // build execution plan

    ExecutionPlanMaker execution_plan_maker(pattern_graphs_, obj_alloc_);
    ret = execution_plan_maker.Build(astNode, root_, ctx);
    if (ret != geax::frontend::GEAXErrorCode::GEAX_SUCCEED) {
        error_msg_ = execution_plan_maker.ErrorMsg();
        return ret;
    }
    result_info_ = execution_plan_maker.GetResultInfo();
    LOG_DEBUG("\n{}", DumpPlan(0, false));

    ctx->result_info_ = std::make_unique<ResultInfo>(result_info_);

    return ret;
}

int ExecutionPlan::Execute(RTContext* ctx) {
    OpBase::OpResult res;
    do {
        res = root_->Consume(ctx);
        LOG_INFO("root op result: {}", cypher::ToString(res));
    } while (res == OpBase::OP_OK);
    return 0;
}

std::string ExecutionPlan::DumpPlan(int indent, bool statistics) const {
    std::string s = statistics ? "Profile statistics:\n" : "Execution Plan:\n";
    OpBase::DumpStream(root_, indent, statistics, s);
    return s;
}

std::string ExecutionPlan::DumpGraph() const {
    std::string s;
    for (auto& g : pattern_graphs_) s.append(g.DumpGraph());
    return s;
}

std::string ExecutionPlan::ErrorMsg() { return error_msg_; }

OpBase* ExecutionPlan::Root() { return root_; }

bool ExecutionPlan::ReadOnly() const { return read_only_; }

void ExecutionPlan::Reset() {}

}  // namespace cypher
