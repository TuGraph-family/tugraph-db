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

#include "compiler/gpc/passes/partialpreload.h"
#include "compiler/gpc/passes/partialfuse.h"
#include "third_party/Halide/include/Halide.h"

#include <stdio.h>

using namespace Halide;
using namespace Halide::Internal;

// The reduce for loop.
bool is_split_candidate(const For *op) {
    if (op->name.find("$") != std::string::npos) {
        return true;
    }
    return false;
}

Expr new_expr(Expr e, int stride) {
    auto new_var = Variable::make(Int(32), ".__thread_id_y");
    new_var = Mul::make(new_var, stride);
    auto new_e = Add::make(e, new_var);
    return new_e;
}

/*
Pseudocode:
Because we only do preload in the first loop, so it's the same with function:
for (n, 0, extent) {
    for (r.x, 0, range) {
        o += buf[r.x]...
    }
}

Preload buf in n.
Or split r.x with ro, ri and then preload buf in ro.
*/

/*PartialSplit:
    for (r.x, 0, range) {
        o += buf[r.x]...
    }

=>

    split:
    for (ro, 0, new_extent_o) {
        for (ri, 0, new_extent_i) {
            o += buf[new_index]...
        }
    }
If the original range is p(p>=0), and tile size is t:
inner extent: min(t, p - ro * t)
outer extent: (p + 1) / 2
*/

Stmt make_barrier(int mask) {
    return Evaluate::make(Call::make(Int(32), Call::gpu_thread_barrier,
                                     {IntImm::make(Int(32), mask)},
                                     Call::Intrinsic));
}

class PartialSplit : public IRMutator {
  public:
    using IRMutator::visit;
    bool candidate = false;
    bool visited = false;
    std::string idx;
    Expr inner_extent;
    Expr outer_extent;
    Expr inner_var;
    Expr outer_var;
    int k_tile_size = 1;
    PartialSplit(int k_tile_size) : k_tile_size(k_tile_size) {}

    Stmt visit(const For *op) {
        // Only consider the first candidate loop.
        if (candidate || (!candidate && !is_split_candidate(op))) {
            return IRMutator::visit(op);
        }
        candidate = true;
        idx = op->name;
        auto iname = op->name + ".ri";
        auto oname = op->name + ".ro";
        inner_var = Variable::make(Int(32), iname);
        outer_var = Variable::make(Int(32), oname);

        inner_extent =
            Min::make(k_tile_size,
                      Sub::make(op->extent, Mul::make(outer_var, k_tile_size)));

        outer_extent =
            Div::make(Add::make(op->extent, k_tile_size - 1), k_tile_size);
        // The inner loop
        // auto inner_body = IfThenElse::make(LT::make(Variable::make(Int(32),
        // iname), inner_extent), mutate(op->body));
        auto inner_body = mutate(op->body);
        auto inner_loop = For::make(iname, Expr(0), inner_extent, op->for_type,
                                    op->device_api, inner_body);
        visited = true;
        // The outer loop
        // auto  new_body = Block::make(inner_loop, make_barrier(0));
        auto new_body = inner_loop;
        auto outer_loop = For::make(oname, Expr(0), outer_extent, op->for_type,
                                    op->device_api, new_body);
        return outer_loop;
    }

    // r.x => new_index
    Expr visit(const Variable *op) {
        if (visited || op->name != idx) {
            return IRMutator::visit(op);
        }
        return Add::make(inner_var, Mul::make(outer_var, k_tile_size));
    }
};

class GpuParallelLoad : public IRMutator {
  public:
    using IRMutator::visit;
    int n_thread_size = 1;
    int k_tile_size = 1;

    GpuParallelLoad(int n_thread_size, int k_tile_size)
        : n_thread_size(n_thread_size), k_tile_size(k_tile_size) {}

    Stmt visit(const For *op) {
        std::string outer_name = op->name + "o";
        int inner_extent = n_thread_size;
        int outer_extent = std::max(k_tile_size / n_thread_size, 1);

        auto new_var = Variable::make(Int(32), ".__thread_id_x");
        new_var =
            Add::make(new_var, Mul::make(inner_extent,
                                         Variable::make(Int(32), outer_name)));
        auto new_body = substitute(op->name, new_var, op->body);
        new_body = IfThenElse::make(LT::make(new_var, op->extent), new_body);
        new_body = For::make(outer_name, Expr(0), outer_extent, op->for_type,
                             op->device_api, new_body);
        // Add barrier
        new_body = Block::make(new_body, make_barrier(0));
        return new_body;
    }
};

/*PartialPreload
    for (ro, 0, new_extent_o) {
        for (ri, 0, new_extent_i) {
            o += buf[new_index]...
        }
    }

=>

    for (ro, 0, new_extent_o) {
        // preload buf
        allocate buf_im[type * size]
        for (ri, 0, new_extent_i) { // ri should considered as pre var(maybe
overwrite it as pure var, maybe not). buf_im[ri] = buf[new_indx];
        }
        // the pre-body
        for (ri, 0, new_extent_i) {
            o += buf_im[ri]
        }
    }


*/

inline bool innames(const std::string &name,
                    const std::vector<std::string> &names) {
    for (auto &n : names) {
        if (name == n)
            return true;
    }
    return false;
}

class PartialPreload : public IRMutator {
  public:
    using IRMutator::visit;
    std::vector<std::string> buf_names;
    int k_tile_size = 1;
    int m_tile_size = 1;
    int m_thread_size = 1;
    int n_thread_size = 1;
    Halide::Target target;
    PartialPreload(std::vector<std::string> buf_names, int k_tile_size,
                   int m_tile_size, int m_thread_size, int n_thread_size,
                   Halide::Target target)
        : buf_names(buf_names), k_tile_size(k_tile_size),
          m_tile_size(m_tile_size), m_thread_size(m_thread_size),
          n_thread_size(n_thread_size), target(target) {}

    class PartialPreloadHelper : public IRMutator {
      public:
        PartialSplit *ps;
        bool candidate = false;
        bool in_loop = false;
        std::vector<std::string> buf_names;
        std::vector<std::string> buf_im_names;
        // std::string buf_name;     // = "c";
        // std::string buf_im_name;  // = "c$im";
        int m_tile_size = 1;
        int m_thread_size = 1;
        int n_thread_size = 1;
        Halide::Target target;
        PartialPreloadHelper(std::vector<std::string> buf_names,
                             PartialSplit *ps, int m_tile_size,
                             int m_thread_size, int n_thread_size,
                             Halide::Target target)
            : buf_names(buf_names), ps(ps), m_tile_size(m_tile_size),
              m_thread_size(m_thread_size), n_thread_size(n_thread_size),
              target(target) {
            for (auto buf : buf_names) {
                buf_im_names.push_back(buf + "$im");
            }
        }

        class FindMagicStart : public IRVisitor {
          public:
            using IRVisitor::visit;
            std::string magic_start;
            void visit(const Variable *op) {
                if (op->name.find(GetMagicStart()) != std::string::npos &&
                    magic_start.length() == 0) {
                    magic_start = op->name;
                }
                return IRVisitor::visit(op);
            }
        };

        class FindBufLoadInLoop : public IRVisitor {
          public:
            using IRVisitor::visit;
            std::vector<Expr> new_idxs;
            std::set<std::string> idxset;
            std::vector<std::string> buf_names;
            FindBufLoadInLoop(std::vector<std::string> buf_names)
                : buf_names(buf_names) {}
            void visit(const Load *op) {
                if (!innames(op->name, buf_names)) {
                    return IRVisitor::visit(op);
                }
                FindMagicStart fms;
                op->index->accept(&fms);
                if (idxset.count(fms.magic_start) == 0) {
                    idxset.insert(fms.magic_start);
                    new_idxs.push_back(op->index);
                }
                return IRVisitor::visit(op);
            }
        };

        class MutateLoad : public IRMutator {
            // Mutate buf load to buf_im load and with new idx.
          public:
            using IRMutator::visit;
            PartialPreloadHelper *pp;
            std::map<std::string, int> emap; // {magic_startX, new_idx};
            int mark_idx = 0;
            MutateLoad(PartialPreloadHelper *pp) : pp(pp) {}

            Expr visit(const Load *op) {
                if (!innames(op->name, pp->buf_names)) {
                    return IRMutator::visit(op);
                }
                int idx = mark_idx;

                FindMagicStart fms;
                op->index->accept(&fms);
                if (emap.count(fms.magic_start) != 0) {
                    // old index, we have found
                    idx = emap[fms.magic_start];
                } else {
                    // new index
                    emap[fms.magic_start] = idx;
                    mark_idx++;
                }
                auto load_idx = Add::make(pp->ps->inner_var,
                                          Mul::make(idx, pp->ps->k_tile_size));
                load_idx =
                    new_expr(load_idx, pp->ps->k_tile_size * pp->m_tile_size);

                auto new_name = op->name + "$im"; // TODO WORKAROUND
                // return Load::make(op->type, pp->buf_im_names[0],
                return Load::make(op->type, new_name, load_idx, op->image,
                                  op->param, op->predicate, op->alignment);
            }
        };

        Stmt visit(const For *op) {
            // Only consider the first candidate loop.
            if (candidate || (!candidate && !is_split_candidate(op))) {
                return IRMutator::visit(op);
            }
            candidate = true;

            ScopedValue<bool> old_in_loop(in_loop, true);

            FindBufLoadInLoop fbl(buf_names);
            op->body.accept(&fbl);

            // insert a for loop to preload buf
            auto loop_idx = ps->inner_var;
            auto buf_type = Int(32);

            auto s = Evaluate::make(0);

            auto size = fbl.new_idxs.size();
            for (int bi = 0; bi < buf_names.size(); ++bi) {
                for (size_t i = 0; i < size; i++) {
                    int idx = size - 1 - i;
                    auto rhs = Load::make(
                        buf_type, buf_names[bi], fbl.new_idxs[idx], Buffer<>(),
                        Parameter(), const_true(), ModulusRemainder());
                    auto store_idx =
                        Add::make(loop_idx, Mul::make(idx, ps->k_tile_size));
                    store_idx =
                        new_expr(store_idx, ps->k_tile_size * m_tile_size);
                    auto store_op = Store::make(
                        buf_im_names[bi], rhs, store_idx, Parameter(),
                        const_true(), ModulusRemainder());

                    s = Block::make(store_op, s);
                }
            }

            auto for_loop =
                For::make(loop_idx.as<Variable>()->name, 0, ps->inner_extent,
                          op->for_type, op->device_api, s);
            if (target.has_gpu_feature()) {
                for_loop = GpuParallelLoad(n_thread_size, ps->k_tile_size)
                               .mutate(for_loop);
            }

            auto new_body =
                Block::make(for_loop, MutateLoad(this).mutate(op->body));

            auto mem_type = target.has_gpu_feature() ? MemoryType::GPUShared
                                                     : MemoryType::Register;
            auto alloc_size = Mul::make(ps->k_tile_size, m_tile_size);
            alloc_size = Mul::make(alloc_size, m_thread_size);

            // debug
            // new_body = IfThenElse::make(LT::make(Variable::make(Int(32),
            // op->name), op->extent), new_body);

            new_body = For::make(op->name, op->min, op->extent, op->for_type,
                                 op->device_api, new_body);
            // TODO: This shared mem should put between gpu grids and blocks,
            // not under blocks. It will use twice space, otherwise.
            for (auto im : buf_im_names) {
                new_body = Allocate::make(im, buf_type, mem_type, {alloc_size},
                                          const_true(), new_body);
            }

            // todo add sync barrier
            return new_body;
        }
    };

    bool inside_for_loops = false;
    PartialPreload(int k_tile_size) : k_tile_size(k_tile_size) {}
    Stmt visit(const For *op) {
        ScopedValue<bool> old_in_loop(inside_for_loops, true);
        return IRMutator::visit(op);
    }

    Stmt visit(const ProducerConsumer *op) {
        if (inside_for_loops && op->is_producer) {
            auto new_body = op->body;
            auto ps = new PartialSplit(k_tile_size);
            new_body = ps->mutate(new_body);
            new_body =
                PartialPreloadHelper(buf_names, ps, m_tile_size, m_thread_size,
                                     n_thread_size, target)
                    .mutate(new_body);
            delete ps;
            return ProducerConsumer::make(op->name, true, new_body);
        }
        return IRMutator::visit(op);
    }
};

class UnfoldLets : public IRMutator {
  public:
    using IRMutator::visit;
    bool visited = false;
    std::map<std::string, Expr> lets; // {name, value}
    std::string name;

    UnfoldLets(std::string name) : name(name) {}

    class RecordLets : public IRVisitor {
      public:
        using IRVisitor::visit;
        std::map<std::string, Expr> lets; // {name, value}
        void visit(const LetStmt *op) {
            lets[op->name] = op->value;
            return IRVisitor::visit(op);
        }
    };

    Stmt visit(const For *op) {
        if (visited)
            return IRMutator::visit(op);
        visited = true;
        auto new_body = op->body;

        // Record letstmt.
        RecordLets rl;
        new_body.accept(&rl);
        lets = rl.lets;

        // Unfold letstmt in `buf` load.
        new_body = mutate(new_body);
        return For::make(op->name, op->min, op->extent, op->for_type,
                         op->device_api, new_body);
    }

    Expr visit(const Load *op) {
        // if (op->name != name) return IRMutator::visit(op);
        if (auto var = op->index.as<Variable>()) {
            if (lets.count(var->name)) {
                return Load::make(Int(32), op->name, lets[var->name], op->image,
                                  op->param, op->predicate, op->alignment);
            }
        }
        return IRMutator::visit(op);
    }
};

class MoveSharedMem : public IRMutator {
    using IRMutator::visit;
    bool visited = false;

    class CollectAlloc : public IRMutator {
      public:
        std::set<std::string> alloc_names;
        std::vector<const Allocate *> allocs;
        Stmt visit(const Allocate *op) {
            if (op->memory_type == MemoryType::GPUShared) {
                if (alloc_names.count(op->name) == 0) {
                    alloc_names.insert(op->name);
                    allocs.push_back(op);
                }
                return mutate(op->body);
            }
            return IRMutator::visit(op);
        }
    };

    Stmt visit(const For *op) {
        if (op->for_type == ForType::GPUThread && !visited) {
            visited = true;
            // Collect allocations.
            CollectAlloc ca;
            auto new_body = op->body;
            new_body = ca.mutate(new_body);

            // Make allocation + new_body.
            new_body = For::make(op->name, op->min, op->extent, op->for_type,
                                 op->device_api, new_body);
            for (auto a : ca.allocs) {
                new_body = Allocate::make(a->name, a->type, a->memory_type,
                                          a->extents, const_true(), new_body);
            }
            return new_body;
        }
        return IRMutator::visit(op);
    }
};

class RemoveDebug : public IRMutator {
    using IRMutator::visit;

    Expr visit(const Load *op) {
        // if (op->name == "edge") return Expr(1.0f);
        // if (op->name == "b") return Expr(1.0f);
        // if (op->name == "c$im") return Expr(0);
        return IRMutator::visit(op);
    }
};

// I think Halide only support `one` shared memory instance.
class AlignSharedMemAddr : public IRMutator {
    using IRMutator::visit;
    Expr size = Expr(0);
    Stmt visit(const Allocate *op) {
        if (op->memory_type == MemoryType::GPUShared) {
            if (op->name == "emap$im") { // TODO, workaround
                return mutate(op->body);
            }
            if (op->name == "c$im") {
                size = op->extents[0];
                return Allocate::make(op->name, op->type, op->memory_type,
                                      {op->extents[0] * 2}, const_true(),
                                      mutate(op->body));
            }
        }
        return IRMutator::visit(op);
    }

    Expr visit(const Load *op) {
        if (op->name == "emap$im") {
            return Load::make(op->type, "c$im", size + op->index, op->image,
                              op->param, op->predicate, op->alignment);
        }
        return IRMutator::visit(op);
    }

    Stmt visit(const Store *op) {
        if (op->name == "emap$im") {
            return Store::make("c$im", op->value, size + op->index, Parameter(),
                               const_true(), ModulusRemainder());
        }
        return IRMutator::visit(op);
    }
};

class UseEdge : public IRMutator {
    using IRMutator::visit;
    Expr visit(const Load *op) {
        if (op->name == "emap") {
            return Load::make(op->type, "edge", op->index, op->image, op->param,
                              op->predicate, op->alignment);
        }

        if (op->name == "edge") {
            auto load = op->index.as<Mul>()->a.as<Load>();
            if (!load) {
                std::cerr << "error, not a load... partialpreload\n";
            }
            return Load::make(Float(32), load->name, load->index, load->image,
                              load->param, load->predicate, load->alignment);
        }

        return IRMutator::visit(op);
    }
};

void AddPartialPreloadPass(Halide::Func &func,
                           std::vector<std::string> buf_names,
                           Halide::Target target, int k_tile_size,
                           int m_tile_size, int m_thread_size,
                           int n_thread_size) {
    assert(buf_names.size() >= 1 && "AddPartialPreloadPass bufs >= 1");
    // TODO(jiny): Use our own lowering process, so we don't need these `Unfold`
    // like passes. Unfold letstmt made for `buf` load.
    func.add_custom_lowering_pass(new UnfoldLets(buf_names[0]));

    // Partial preload, only for the first loop. This pass is for GPU load
    // efficiency, no good for CPU.
    auto pp = new PartialPreload(buf_names, k_tile_size, m_tile_size,
                                 m_thread_size, n_thread_size, target);
    func.add_custom_lowering_pass(pp);

    // Move the shared memory allocate to between gpu grids and blocks.
    func.add_custom_lowering_pass(new MoveSharedMem());

    if (buf_names.size() > 1) {
        func.add_custom_lowering_pass(new AlignSharedMemAddr());
        // just use iedge, pre select edge with emap. TODO more nice
        func.add_custom_lowering_pass(new UseEdge());
        // func.add_custom_lowering_pass(new UseEdge2());
    }

    // func.add_custom_lowering_pass(new RemoveDebug());
}
