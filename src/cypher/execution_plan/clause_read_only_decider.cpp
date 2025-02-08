/**
 * Copyright 2023 AntGroup CO., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License") {}
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#include "cypher/execution_plan/clause_read_only_decider.h"
#include "db/galaxy.h"

namespace cypher {

std::any ClauseReadOnlyDecider::visit(geax::frontend::InQueryProcedureCall* node) {
    std::string func_name = std::get<std::string>(node->name());
    if (func_name == "db.plugin.callPlugin") {
        auto args1 = dynamic_cast<geax::frontend::VString*>(node->args()[0]);
        auto args2 = dynamic_cast<geax::frontend::VString*>(node->args()[1]);
        if (args1 && args2) {
            auto type = args1->val();
            auto name = args2->val();
            if (ctx_->graph_.empty()) {
                THROW_CODE(InvalidParameter, "empty graph in clause ReadOnly decider");
            }
            if (ctx_->user_.empty()) {
                THROW_CODE(InvalidParameter, "empty user in clause ReadOnly decider");
            }
            auto db = ctx_->galaxy_->OpenGraph(ctx_->user_, ctx_->graph_);
            read_only_ =
                db.IsReadOnlyPlugin(type == "CPP" ? lgraph::PluginManager::PluginType::CPP
                                                  : lgraph::PluginManager::PluginType::PYTHON,
                                    ctx_->user_, name);
        }
    } else {
        auto pp = cypher::global_ptable.GetProcedure(func_name);
        if (pp && !pp->read_only) read_only_ = false;
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ClauseReadOnlyDecider::visit(geax::frontend::NamedProcedureCall* node) {
    std::string func_name = std::get<std::string>(node->name());
    if (func_name == "db.plugin.callPlugin") {
        auto args1 = dynamic_cast<geax::frontend::VString*>(node->args()[0]);
        auto args2 = dynamic_cast<geax::frontend::VString*>(node->args()[1]);
        if (args1 && args2) {
            auto type = args1->val();
            auto name = args2->val();
            if (ctx_->graph_.empty()) {
                THROW_CODE(InvalidParameter, "empty graph in clause ReadOnly decider");
            }
            if (ctx_->user_.empty()) {
                THROW_CODE(InvalidParameter, "empty user in clause ReadOnly decider");
            }
            auto db = ctx_->galaxy_->OpenGraph(ctx_->user_, ctx_->graph_);
            read_only_ =
                db.IsReadOnlyPlugin(type == "CPP" ? lgraph::PluginManager::PluginType::CPP
                                                  : lgraph::PluginManager::PluginType::PYTHON,
                                    ctx_->user_, name);
        }
    } else {
        auto pp = cypher::global_ptable.GetProcedure(func_name);
        if (pp && !pp->read_only) read_only_ = false;
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

}  // namespace cypher
