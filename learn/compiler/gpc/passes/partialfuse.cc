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

#include "compiler/gpc/passes/partialfuse.h"
#include "third_party/Halide/include/Halide.h"

#include <stdio.h>

using namespace Halide;
using namespace Halide::Internal;

// partialfuse fuses several loops which may have different loop extent.
// for example:
// for i = 0, p:
//   ...s1
// for j = 0, q:
//   ...s2
// If we know q is bigger than p, then we can partialfuse to
// for i = 0, p:
//   j = i
//.  ...s1
//.  ...s2
// for jj = 0, q-p:
//   ...s2_left

bool is_candidate(const For *op) {
    if (op->name.find("$") == std::string::npos) {
        return false;
    }
    auto var = op->extent.as<Variable>();
    if (!var)
        return false;
    if (var->name.find(GetMagicRange()) == std::string::npos) {
        return false;
    }
    return true;
}

// TODO(jin): Do we need to implement this?
// Unbound loading variable which is known as index.
// For example:
// let a = colptr[m]
// let clamped_a = max(min(a, 100), 0)
// let k = clamped_a
// If we know a is an index, which must be a integer ranging from 0 to 100
// , we can remove the clamp expression:
// let k = colptr[m]
// ---
// DELETE: promise_clamped will do the thing.
// Actually we need to unbound RDom k; k.where(k<rowptr(m))
// ---
class UnboundIndexVar : public IRMutator {
    using IRMutator::visit;

    Stmt visit(const For *op) { return IRMutator::visit(op); }
};

// Step 1: replace p with p1, p2,... pi...pn, where n = tile size
// Step 3: partial fuse with only 1 level fuse.
// Step 2: replace pi with magic_range
class PartialfuseIndex : public IRMutator {
  public:
    using IRMutator::visit;
    class PartialfuseIndexHelper : public IRMutator {
        int rindex = 0;
        bool inside_for_loops = false;

        class MutateStart : public IRMutator {
          public:
            using IRMutator::visit;
            int rindex = 0;
            MutateStart(int rindex) : rindex(rindex) {}
            Expr visit(const Variable *op) {
                if (op->name == GetMagicStart() && RewriteMagicStart()) {
                    return Variable::make(op->type,
                                          op->name + std::to_string(rindex));
                }
                return IRMutator::visit(op);
            }
        };

        Stmt visit(const For *op) {
            ScopedValue<bool> old_in_loop(inside_for_loops, true);
            auto extent = op->extent;
            auto var = op->extent.as<Variable>();
            auto body = MutateStart(rindex).mutate(op->body);
            if (var && var->name == GetMagicRange()) {
                auto new_extent = Variable::make(
                    var->type, var->name + std::to_string(rindex));
                rindex++;
                return For::make(op->name, std::move(op->min),
                                 std::move(new_extent), op->for_type,
                                 op->device_api, std::move(body));
            }

            return IRMutator::visit(op);
        }
    };

    bool inside_for_loops = false;
    Stmt visit(const For *op) {
        ScopedValue<bool> old_in_loop(inside_for_loops, true);
        return IRMutator::visit(op);
    }

    Stmt visit(const ProducerConsumer *op) {
        if (inside_for_loops && op->is_producer) {
            auto new_body = PartialfuseIndexHelper().mutate(op->body);
            return ProducerConsumer::make(op->name, true, new_body);
        }
        return IRMutator::visit(op);
    }
};

class CollectMagicToken : public IRMutator {
  public:
    using IRMutator::visit;
    std::list<Expr> magic_list;
    std::map<std::string, Expr> range_map; // magic_name, range pair
    std::map<std::string, Expr> start_map; // magic_name, start
    int index = 0;

    class ContainMagicRow : public IRVisitor {
      public:
        bool flag = false;
        void visit(const Load *op) {
            if (op->name == GetMagicRow()) {
                flag = true;
            }
            return IRVisitor::visit(op);
        }
    };

    class FindRange : public IRVisitor {
      public:
        Expr range;
        Expr start;
        void visit(const Cast *op) {
            ContainMagicRow cmr;
            op->accept(&cmr);
            if (!cmr.flag)
                return;
            range = op->value;
            start = op->value.as<Sub>()->b;
        }
    };

    Stmt visit(const For *op) {
        if (is_candidate(op)) {
            magic_list.push_back(op->extent);
            // Find the first param range.
            FindRange fr;
            op->body.accept(&fr);
            range_map[op->extent.as<Variable>()->name] = fr.range;
            start_map[GetMagicStart() + std::to_string(index++)] = fr.start;
        }
        return IRMutator::visit(op);
    }
};

class RewriteMagicToken : public IRMutator {
    using IRMutator::visit;

    bool inside_for_loops = false;
    Stmt visit(const For *op) {
        ScopedValue<bool> old_in_loop(inside_for_loops, true);
        return IRMutator::visit(op);
    }

    Stmt visit(const ProducerConsumer *op) {
        if (inside_for_loops && op->is_producer) {
            auto s = op->body;
            auto rl = new CollectMagicToken;
            s = rl->mutate(op->body);
            for (auto it : rl->range_map) {
                s = LetStmt::make(it.first, it.second, s);
            }
            if (RewriteMagicStart()) {
                for (auto it : rl->start_map) {
                    s = LetStmt::make(it.first, it.second, s);
                }
            }
            delete rl;
            return ProducerConsumer::make(op->name, true, s);
        }
        return IRMutator::visit(op);
    }
};

class FindLoop : public IRVisitor {
  public:
    bool findloop = false;
    using IRVisitor::visit;
    void visit(const For *op) { findloop = true; }
};

// Collect paritial fuse neccessary infos.
class CollectFuseBaseAndLoopBodies : public IRMutator {
  public:
    using IRMutator::visit;
    Expr fuse_base;
    std::vector<Stmt> loop_bodies;
    Stmt visit(const For *op) {
        if (!is_candidate(op)) {
            return IRMutator::visit(op);
        }
        if (!fuse_base.defined()) {
            fuse_base = op->extent;
        }
        // Collect fusing loop bodies.
        loop_bodies.push_back(op->body);

        return IRMutator::visit(op);
    }
};

class CollectFuseLets : public IRMutator {
  public:
    using IRMutator::visit;
    CollectFuseBaseAndLoopBodies *cf;
    bool inside_magic_loop = false;
    bool magic_loop_begin = false;
    std::vector<const LetStmt *> lets;
    size_t count = 1;
    CollectFuseLets(CollectFuseBaseAndLoopBodies *cf) : cf(cf) {}

    Stmt visit(const For *op) {
        if (is_candidate(op)) {
            magic_loop_begin = true;
            if (count >= cf->loop_bodies.size()) {
                magic_loop_begin = false; // in the end of magic loop.
            }
            count++;
            ScopedValue<bool> sv(inside_magic_loop, true);
            return IRMutator::visit(op);
        }
        return IRMutator::visit(op);
    }

    Stmt visit(const LetStmt *op) {
        if (magic_loop_begin && !inside_magic_loop) {
            lets.push_back(op);
        }
        return IRMutator::visit(op);
    }

    Expr visit(const Let *op) { return IRMutator::visit(op); }
};

class PartialfuseReal : public IRMutator {
    class PartialfuseRealHelper : public IRMutator {
      public:
        using IRMutator::visit;
        size_t loop_index = 0;
        CollectFuseBaseAndLoopBodies *cf;
        CollectFuseLets *cl;

        PartialfuseRealHelper(CollectFuseBaseAndLoopBodies *cf,
                              CollectFuseLets *cl)
            : cf(cf), cl(cl) {}

        Stmt visit(const For *op) {
            if (!is_candidate(op)) {
                return IRMutator::visit(op);
            }
            auto new_extent = op->extent;
            if (loop_index > 0) {
                new_extent = new_extent - cf->fuse_base;

                // Uncomment this to remove redandunt for loop for the left
                // computations. return Evaluate::make(0);
            }

            // fuse [ ..:end]
            std::vector<Stmt> stmts;
            for (size_t i = loop_index + 1; i < cf->loop_bodies.size(); i++) {
                stmts.push_back(cf->loop_bodies[i]);
            }
            auto block = Block::make(stmts);
            auto new_body = op->body;
            if (block.defined()) {
                new_body = Block::make(new_body, block);
            }
            auto new_stmt = For::make(op->name, op->min, new_extent,
                                      op->for_type, op->device_api, new_body);
            // Insert the fuse lets before the first for loop.
            if (loop_index == 0) {
                for (size_t i = 0; i < cl->lets.size(); ++i) {
                    auto let = cl->lets[cl->lets.size() - 1 - i];
                    new_stmt = LetStmt::make(let->name, let->value, new_stmt);
                }
            }

            loop_index++;
            return new_stmt;
        }
    };

    bool inside_for_loops = false;
    Stmt visit(const For *op) {
        ScopedValue<bool> old_in_loop(inside_for_loops, true);
        return IRMutator::visit(op);
    }

    Stmt visit(const ProducerConsumer *op) {
        if (inside_for_loops && op->is_producer) {
            auto cf = new CollectFuseBaseAndLoopBodies();
            cf->mutate(op->body);
            auto cl = new CollectFuseLets(cf);
            cl->mutate(op->body);
            auto new_body = PartialfuseRealHelper(cf, cl).mutate(op->body);
            delete cf;
            delete cl;
            return ProducerConsumer::make(op->name, true, new_body);
        }
        return IRMutator::visit(op);
    }
};
// Hoist t36 out of loop:
// for (f.s1.k$x, 0, magic_p) {
//   ...stmt...
//   let t36 = r[t54]
//   ...
// }
// It's safe to hoist `t36` if there is no store to buffer `r` in stmt.
// Halide is conservative because buffer indexing has side-effect(Maybe we can
// define a read-only buffer).
class HoistLet : public IRMutator {
  public:
    using IRMutator::visit;
    std::list<const LetStmt *> lets;
    bool inside_for_loops = false;
    bool in_loop = false;
    std::string loop_var;

    Stmt visit(const For *op) {
        // Limit to the innermost `For` loop
        FindLoop fl;
        op->body.accept(&fl);
        if (fl.findloop) {
            return IRMutator::visit(op);
        }

        ScopedValue<bool> inside_for_loops_scope(inside_for_loops, true);
        ScopedValue<std::string> loop_var_scope(loop_var, op->name);
        auto body = mutate(op->body);
        if (!lets.empty()) {
            auto new_stmt = For::make(op->name, op->min, op->extent,
                                      op->for_type, op->device_api, body);
            for (auto let : lets) {
                new_stmt = LetStmt::make(let->name, let->value, new_stmt);
            }
            lets.clear();
            return new_stmt;
        }
        return IRMutator::visit(op);
    }

    class NameVisitor : public IRVisitor {
      public:
        using IRVisitor::visit;
        std::string name;
        bool find = false;
        NameVisitor(std::string name) : name(name) {}
        void visit(const Variable *op) {
            if (op->name == name) {
                find = true;
            }
        }
    };

    Stmt visit(const LetStmt *op) {
        if (!inside_for_loops) {
            return IRMutator::visit(op);
        }

        NameVisitor nv(loop_var);
        op->value.accept(&nv);
        if (!nv.find) {
            lets.push_front(op);
            // Hoist the op and remove it in old body
            return op->body;
        }

        return IRMutator::visit(op);
    }
};

class RemoveDupLet : public IRMutator {
  public:
    using IRMutator::visit;
    std::set<std::string> visited_set;
    Stmt visit(const LetStmt *op) {
        if (visited_set.count(op->name) == 1) {
            return mutate(op->body);
        } else {
            visited_set.insert(op->name);
            return IRMutator::visit(op);
        }
    }
};

class RemoveMagicCast : public IRMutator {
    using IRMutator::visit;
    bool match(Expr e) {
        if (e.as<Cast>()) {
            return true;
            // return equal(cast->value, Expr(0));
        }
        return false;
    }
    Expr visit(const Add *op) {
        if (match(op->a))
            return op->b;
        if (match(op->b))
            return op->a;
        return IRMutator::visit(op);
    }
};

class RevertMagicCSE : public IRMutator {
  public:
    using IRMutator::visit;
    std::map<std::string, Expr> lets;
    bool in_magic_expr = false;

    ~RevertMagicCSE() {}

    Stmt visit(const LetStmt *op) {
        lets[op->name] = op->value;
        return IRMutator::visit(op);
    }

    Expr visit(const Cast *op) {
        ScopedValue<bool> old_in_magic_expr(in_magic_expr, true);
        return IRMutator::visit(op);
    }

    Expr visit(const Variable *op) {
        if (!in_magic_expr) {
            return IRMutator::visit(op);
        }
        if (lets.find(op->name) != lets.end()) {
            return lets[op->name];
        }
        return IRMutator::visit(op);
    }
};

void AddPartialfusePass(Func &func) {
    // Revert magic CSE temperally. Otherwise, we will miss let stmt.
    func.add_custom_lowering_pass(new RevertMagicCSE);

    func.add_custom_lowering_pass(new PartialfuseIndex);

    func.add_custom_lowering_pass(new PartialfuseReal);

    func.add_custom_lowering_pass(new RewriteMagicToken);
    func.add_custom_lowering_pass(new HoistLet);
    func.add_custom_lowering_pass(
        new RemoveMagicCast); // RemoveMagicCast should happen after HoistLet,
                              // otherwise some symbols will not be found.

    // TODO some passes hase internal state, which will re-entry(the register
    // pass is not released until program exits) , so we need to clear these
    // internal state. Otherwise, we can not compile the function more than
    // twice. (Or we can release/destruct them in Lower.cpp) e.g.,
    // PartialfuseIndex::index

    // Above pass will introduce redundant lets. Remove them.
    // func.add_custom_lowering_pass(new RemoveDupLet);
}
