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

#include "compiler/gpc/passes/rowbalance.h"
#include "compiler/gpc/passes/partialfuse.h"
#include "third_party/Halide/include/Halide.h"

#include <stdio.h>

using namespace Halide;
using namespace Halide::Internal;

class RowBalance : public IRMutator {
  public:
    using IRMutator::visit;
    bool inside_for_loops = false;
    int tile_thread_y = 1;
    bool do_row_balance = false;
    std::vector<std::string> replaces;

    RowBalance(int tile_thread_y, bool do_row_balance,
               std::vector<std::string> names)
        : tile_thread_y(tile_thread_y), do_row_balance(do_row_balance),
          replaces(names) {}

    class RecordRowIdx : public IRVisitor {
      public:
        using IRVisitor::visit;
        std::vector<Expr> starts;
        void visit(const LetStmt *op) {
            if (op->name == GetMagicStart() + "0") {
                if (auto load = op->value.as<Load>()) {
                    starts.push_back(load->index);
                }
            }
            return IRVisitor::visit(op);
        }
    };

    class MutateRowIdx : public IRMutator {
      public:
        RecordRowIdx *rri;
        MutateRowIdx(RecordRowIdx *rri) : rri(rri) {}
        Expr mutate(const Expr &e) {
            // e == s: return magic[s]
            // e == s + 1: return magic[s] + 1
            for (auto &s : rri->starts) {
                auto load =
                    Load::make(Int(32), GetMagicRowIndices(), s, Buffer<>(),
                               Parameter(), const_true(), ModulusRemainder());
                if (Halide::Internal::equal(e, s)) {
                    return load;
                }
                if (Halide::Internal::equal(e, simplify(s + 1))) {
                    return load + 1;
                }
            }

            return IRMutator::mutate(e);
        }
    };

    class ReplaceRow : public IRMutator {
      public:
        std::vector<std::string> replaces;
        RecordRowIdx *rri;

        ReplaceRow(RecordRowIdx *rri, std::vector<std::string> replaces)
            : rri(rri), replaces(replaces) {}

        Expr mutateIdx(Expr index) {
            MutateRowIdx m(rri);
            return m.mutate(index);
        }

        bool should_replace(std::string name) {
            for (auto x : replaces) {
                if (x == name)
                    return true;
            }
            return false;
        }

        Expr visit(const Load *op) {
            if (!should_replace(op->name))
                return IRMutator::visit(op);
            auto new_idx = mutateIdx(op->index);
            return Load::make(Int(32), op->name, new_idx, op->image, op->param,
                              op->predicate, op->alignment);
        }

        Stmt visit(const Store *op) {
            if (!should_replace(op->name))
                return IRMutator::visit(op);
            auto new_idx = mutateIdx(op->index);
            return Store::make(op->name, op->value, new_idx, op->param,
                               op->predicate, op->alignment);
        }
    };

    class RemoveMagic : public IRMutator {
        bool in_scope = false;
        Expr visit(const Cast *op) {
            ScopedValue<bool> old_in_scope(in_scope, true);
            return IRMutator::visit(op);
        }

        Expr visit(const Load *op) {
            // eliminate magic
            if (in_scope && op->name == GetMagicRowIndices()) {
                return Expr(0);
            }
            return IRMutator::visit(op);
        }
    };

    Stmt visit(const For *op) {
        ScopedValue<bool> old_in_loop(inside_for_loops, true);
        auto new_body = op->body;
        if (do_row_balance) {
            RecordRowIdx rri;
            new_body.accept(&rri);
            ReplaceRow rr(&rri, replaces);
            new_body = rr.mutate(new_body);
        }
        RemoveMagic rm;
        new_body = rm.mutate(new_body);
        auto new_op = For::make(op->name, op->min, op->extent, op->for_type,
                                op->device_api, new_body);

        return new_op;
    }
};

// Rewrite magic_start load and output store.
// look, when m > 1, it's complicated.
void AddRowBalancePass(Halide::Func &func, int tile_thread_y,
                       bool do_row_balance) {
    // Must set min to 0 in this pass.
    auto bufs = func.output_buffers();
    bufs[0].dim(1).set_min(0).dim(0).set_min(0);

    std::vector<std::string> names;
    names.push_back(GetMagicRow());
    for (auto &b : bufs) {
        names.push_back(b.name());
    }

    func.add_custom_lowering_pass(
        new RowBalance(tile_thread_y, do_row_balance, names));
}
