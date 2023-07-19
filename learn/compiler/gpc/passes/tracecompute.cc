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

#include "compiler/gpc/passes/tracecompute.h"
#include "third_party/Halide/include/Halide.h"

#include <stdio.h>

using namespace Halide;
using namespace Halide::Internal;

class TraceCompute : public IRMutator {
  public:
    std::string name;
    std::map<int, int> indice_map; // {old, new}
    std::string load_name;
    using IRMutator::visit;

    TraceCompute(std::string name) : name(name) {}

    class CollectIndice : public IRVisitor {
      public:
        std::string name;
        std::map<int, int> indice_map; // {old, new}
        int index = 0;

        using IRVisitor::visit;
        CollectIndice(std::string name) : name(name) {}

        class GetIndex : public IRVisitor {
          public:
            int index = -1;
            using IRVisitor::visit;
            void visit(const Load *op) {
                index = op->index.as<IntImm>()->value;
            }
        };

        void visit(const Store *op) {
            if (op->name == name) {
                GetIndex gi;
                op->value.accept(&gi);
                indice_map[gi.index] = index++;
            }
        }
    };

    class GetLoadName : public IRVisitor {
      public:
        std::string name;
        std::string load_name;
        bool in_scope = false;
        GetLoadName(std::string name) : name(name) {}
        void visit(const Store *op) {
            if (load_name.empty() && op->name == name) {
                ScopedValue<bool> old_in_scope(in_scope, true);
                op->value.accept(this);
            }
        }
        void visit(const Load *op) {
            if (in_scope) {
                load_name = op->name;
            }
        }
    };

    Stmt visit(const ProducerConsumer *op) {
        if (op->is_producer) {
            return IRMutator::visit(op);
        }

        CollectIndice ci(name);
        op->body.accept(&ci);
        indice_map = ci.indice_map;

        GetLoadName gln(name);
        op->body.accept(&gln);
        load_name = gln.load_name;

        return IRMutator::visit(op);
    }
};

class ReplaceIndex : public IRMutator {
  public:
    using IRMutator::visit;
    TraceCompute *tc;
    bool in_store = false;
    ReplaceIndex(TraceCompute *tc) : tc(tc) {}

    Stmt visit(const Store *op) {
        if (op->name != tc->load_name) {
            return IRMutator::visit(op);
        }

        int idx = op->index.as<IntImm>()->value;
        if (tc->indice_map.count(idx) == 0) {
            return Evaluate::make(0);
        }

        auto new_idx = tc->indice_map[idx];
        ScopedValue<bool> old_in_store(in_store, true);
        auto new_value = mutate(op->value);

        return Store::make(op->name, new_value, new_idx, op->param,
                           op->predicate, op->alignment);
    }

    Expr visit(const Load *op) {
        if (op->name != tc->load_name) {
            return IRMutator::visit(op);
        }

        int idx = op->index.as<IntImm>()->value;

        auto new_idx = tc->indice_map[idx];

        return Load::make(op->type, op->name, new_idx, op->image, op->param,
                          op->predicate, op->alignment);
    }

    // Re-allocate register with smaller size
    Stmt visit(const Allocate *op) {
        if (op->name == tc->load_name) {
            int register_size = int(tc->indice_map.size());
            return Allocate::make(op->name, op->type, op->memory_type,
                                  {register_size}, op->condition,
                                  mutate(op->body));
        }
        return IRMutator::visit(op);
    }
};

void AddRemoveRedundantComputePass(Halide::Func &func) {
    // Note: before this pass, unroll schedule stage should be performed.
    auto tc = new TraceCompute(func.name());
    func.add_custom_lowering_pass(tc);
    func.add_custom_lowering_pass(new ReplaceIndex(tc));
}
