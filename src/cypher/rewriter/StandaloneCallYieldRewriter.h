/**
* Copyright 2022 AntGroup CO., Ltd.
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

#pragma once

#include "geax-front-end/ast/clause/ElementFiller.h"
#include "geax-front-end/common/ObjectAllocator.h"
#include "cypher/utils/ast_node_visitor_impl.h"
#include "cypher/parser/data_typedef.h"
#include "cypher/procedure/procedure.h"

namespace cypher {

class StandaloneCallYieldRewriter : public AstNodeVisitorImpl {
   public:
    StandaloneCallYieldRewriter(geax::common::ObjectArenaAllocator& allocator) : objAlloc_(allocator) {}
    std::any visit(geax::frontend::NamedProcedureCall* node) override {
        if (node->yield()) {
            return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
        }
        std::string procedure_name = std::get<std::string>(node->name());
        auto p = global_ptable.GetProcedure(procedure_name);
        if (!p) {
            THROW_CODE(CypherException, "No such procedure: {}", procedure_name);
        }
        auto yield_fields = objAlloc_.allocate<geax::frontend::YieldField>();
        for (auto& r : p->signature.result_list) {
            std::string name = r.name;
            std::string alias = r.name;
            yield_fields->appendItem(std::move(name), std::move(alias));
        }
        node->setYield(yield_fields);
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

   private:
    geax::common::ObjectArenaAllocator& objAlloc_;
    size_t idx_ = 0;
};

}  // namespace cypher
