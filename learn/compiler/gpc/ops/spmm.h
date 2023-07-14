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

#ifndef GPC_OPS_SPMM_H
#define GPC_OPS_SPMM_H

#include <chrono>

#include "compiler/gpc/ops/utils.h"
#include "compiler/gpc/passes/partialfuse.h"
#include "compiler/gpc/passes/partialpreload.h"
#include "compiler/gpc/passes/rowbalance.h"
#include "compiler/gpc/passes/tracecompute.h"

#include "third_party/Halide/include/Halide.h"
#include "third_party/Halide/include/HalideRuntimeCuda.h"
#include "third_party/Halide/tools/halide_benchmark.h"

using namespace Halide;
using namespace Halide::Internal;

namespace gpc {

enum SpmmOp { MUL, COPY_LHS, COPY_RHS, UNKNOWN_OP };

enum SpmmReduceOp { SUM, MAX, MIN, UNKNOWN_REDUCE_OP };

int op2int(std::string op) {
    if (op == "mul" || op == "u_mul_e")
        return MUL;
    if (op == "copy_lhs")
        return COPY_LHS;
    if (op == "copy_rhs")
        return COPY_RHS;
    return UNKNOWN_OP;
}

int reduce2int(std::string op) {
    if (op == "sum")
        return SUM;
    if (op == "max")
        return MAX;
    if (op == "min")
        return MIN;
    return UNKNOWN_REDUCE_OP;
}

class Spmm {
  public:
    bool register_tile = false;
    double time = 0.0f;
    int op_enum;
    int reduce_op_enum;

    int row;
    int nn;
    int elems_perrow;
    int nnz;

    ImageParam rowindices;
    ImageParam rowptr;
    ImageParam colidx;
    ImageParam b;
    ImageParam iedge;
    ImageParam iedge_map;

    Func fout;

    Target target;
    const halide_device_interface_t *interface;

    Buffer<float> ib;
    Buffer<int> irowindices;
    Buffer<int> irow;
    Buffer<int> icol;
    Buffer<float> b_edge;
    Buffer<int> b_edge_map;
    Buffer<float> bout;
    Buffer<int> bindex;

    std::string s_max_registers;

    static std::vector<std::vector<int>> build_search_space() {}

    static std::vector<int>
    search_best_config(std::vector<std::vector<int>> search_space) {
        // TODO: to be implemented.
        // For s in search_space
        // build a Spmm instance and Compute, get the time.
        // sort the time, get the best config.
        // return the config.
    }

    inline void attach(const std::vector<void *> &inputs,
                       const std::vector<void *> &outputs) {

#define ATTACH(B, PTRI)                                                        \
    if (inputs[PTRI]) {                                                        \
        B.device_wrap_native(interface, uintptr_t(inputs[PTRI]));              \
    }
#define ATTACHO(B, PTRI)                                                       \
    if (outputs[PTRI]) {                                                       \
        B.device_wrap_native(interface, uintptr_t(outputs[PTRI]));             \
    }

        ATTACH(irowindices, 1);
        ATTACH(irow, 2);
        ATTACH(icol, 3);
        ATTACH(ib, 0);
        ATTACH(b_edge_map, 4);
        ATTACH(b_edge, 5);
        ATTACHO(bout, 0);
        ATTACHO(bindex, 1);
    }

    inline void detach() {
#define DETACH(B)                                                              \
    if (B.raw_buffer()->device) {                                              \
        B.device_detach_native();                                              \
    }

        DETACH(irowindices)
        DETACH(irow)
        DETACH(icol)
        DETACH(b_edge)
        DETACH(b_edge_map)
        DETACH(ib)
        DETACH(bout)
        DETACH(bindex)
    }

    inline void set() {
        rowindices.set(irowindices);
        rowptr.set(irow);
        colidx.set(icol);
        b.set(ib);
        iedge.set(b_edge);
        iedge_map.set(b_edge_map);
    }

    void Compute(const std::vector<void *> &inputs,
                 const std::vector<void *> &outputs) {
        // set max registers env
        int result =
            setenv("HL_CUDA_JIT_MAX_REGISTERS", s_max_registers.c_str(), true);
        if (result != 0) {
            std::cerr << "ERROR!!! HL_CUDA_JIT_MAX_REGISTERS unset!!!"
                      << std::endl;
        }

        if (irow.raw_buffer()->device != 0) {
            detach();
        }
        attach(inputs, outputs);
        set();

        switch (reduce_op_enum) {
        case SUM:
            fout.realize(bout, target);
            break;
        case MAX:
        case MIN:
        default:
            Realization r(bout, bindex);
            fout.realize(r, target);
        }
    }

    Spmm(bool register_tile, bool refparam,
         std::vector<int> params = std::vector<int>(),
         std::vector<int> config = std::vector<int>(),
         std::string op = "copy_lhs", std::string reduce_op = "sum")
        : register_tile(register_tile), op_enum(op2int(op)),
          reduce_op_enum(reduce2int(reduce_op)) {
        // Set target.
        target = get_host_target();
        target.set_feature(Target::CUDA);
        target.set_feature(Target::CUDACapability50);
        interface = get_device_interface_for_device_api(Halide::DeviceAPI::CUDA,
                                                        target);

        // inputs' dimentions
        row = params[0];
        nn = params[1];
        elems_perrow = params[2];
        nnz = params[3];
        int w_len = params[4];
        int broadcast_dim = nn / w_len;

        // Create buffer
        ib = Buffer<float>(nn, row);
        irowindices = Buffer<int>(row);
        irow = Buffer<int>(row + 1);
        icol = Buffer<int>(nnz);
        bout = Buffer<float>(nn, row);
        bindex = Buffer<int>(nn, row);
        b_edge = Buffer<float>(w_len, nnz);
        b_edge_map = Buffer<int>(nnz);

        // kernel config
        int nt = config[0];
        int nthread = config[1];
        int mt = config[2];
        int mthread = config[3];
        int kt = config[4];
        int do_row_balance = config[5];
        int do_preload = config[6];
        int do_keep_order = config[7];

        // Set max registers env
        int threads = mthread * nthread;
        int max_registers = 65536 / threads;
        max_registers =
            (max_registers >= 255 ? 255 : (max_registers >= 128 ? 128 : 64));
        s_max_registers = std::to_string(max_registers);

        // Outputs
        Func spmm("f");
        Func out("o" + vector2string(config));
        // Inputs
        rowindices = ImageParam(Int(32), 1, GetMagicRowIndices());
        rowptr = ImageParam(Int(32), 1, GetMagicRow());
        colidx = ImageParam(Int(32), 1, "c");
        b = ImageParam(Float(32), 2, "b");
        iedge = ImageParam(Float(32), 2, "edge");
        iedge_map = ImageParam(Int(32), 1, "emap");

        // The min(start index) is always zero.
        // TODO(jin): Move these to compiler passes.
        rowptr.dim(0).set_min(0);
        colidx.dim(0).set_min(0);
        b.dim(0).set_min(0);
        b.dim(1).set_min(0);
        iedge.dim(0).set_min(0);
        iedge.dim(1).set_min(0);
        iedge_map.dim(0).set_min(0);

        // Vars
        Var m("m"), n("n"), mii, mi, mo, nii, ni, no;
        RVar ko, ki;

        auto magic_range_str = GetMagicRange();
        auto magic_start_str = GetMagicStart();
        Param<int> param(
            magic_range_str); // `param` marks(equals) `magic_range`.
        Param<int> p_start(magic_start_str); // `p_start` equals `rowptr(m)`.
        auto magic_range = (rowptr(m + 1)) - (rowptr(m));
        RDom k(0, param, "k");
        auto cl_rowptr = p_start + k;
        // auto cl_rowptr = promise_clamped(rowptr(m) + k, 0, /*nnz*/ 256 - 1);
        // // TODO: I think there is no need to clamp for index variable.
        auto cl_col = promise_clamped(colidx(cl_rowptr), 0, /*K*/ row - 1);

        auto magic_indices = rowindices(0);

        if (refparam) {
            // This is a reference for which `compute_at` works as expect.
            // Here the `colidx` is only allocated size of 2 as expected,
            // instead of wrongly 128. The rowptr array index is replaced by
            // `Param`, which has a single point bounds inference. It's
            // important to have both param_start and range as `Param`. NOTE:
            // use `RDom k(param_start, param_end)` is also OK.
            Param<int> param_start("col_start");
            param_start.set(0);
            cl_col = promise_clamped(colidx(param_start + k.x), 0, row - 1);
            spmm(n, m) = 0.0f;
            spmm(n, m) += b(n, cl_col);
            out(n, m) = spmm(n, m);

            spmm.update().split(k, ko, ki, 16, TailStrategy::GuardWithIf);
            /*
            out.split(m, m, mi, 2).reorder(mi, n, m);
            spmm.compute_at(out, n);
            spmm.update().reorder(m, ki, ko).unroll(m);
            */
            // colidx.in().compute_at(spmm, ko);
        } else if (register_tile) {
            // Core code.
            Expr e_get_input;
            Expr e_binary_op;
            Expr e_arg_idx;
            Expr e_col;
            Expr e_edge;
            e_col = promise_clamped(colidx(cl_rowptr), 0, /*K*/ row - 1);
            auto get_input_b = b(n, e_col);
            e_edge = promise_clamped(iedge_map(cl_rowptr), 0, /*nnz*/ nnz - 1);
            // TODO here we actually do not need iedge_map. Precompute it.
            // e_edge = promise_clamped(cl_rowptr, 0, nnz - 1);
            auto get_input_e = iedge(0, e_edge);
            // auto get_input_e = iedge(n / broadcast_dim, e_edge);
            switch (op2int(op)) {
            case MUL:
                e_get_input = get_input_b * get_input_e;
                e_arg_idx = e_col; // Not happen.
                break;
            case COPY_RHS:
                e_get_input = get_input_e;
                e_arg_idx = e_edge;
                break;
            case COPY_LHS:
            default:
                e_get_input = get_input_b;
                e_arg_idx = e_col;
            }
            e_binary_op = e_get_input + magic_indices + magic_range;
            switch (reduce2int(reduce_op)) {
            case MAX:
                spmm(n, m) = Tuple(-std::numeric_limits<float>::infinity(), 0);
                spmm(n, m) = Tuple(max(spmm(n, m)[0], e_binary_op),
                                   select(spmm(n, m)[0] > e_binary_op,
                                          spmm(n, m)[1], e_arg_idx));
                break;
            case MIN:
                spmm(n, m) = Tuple(std::numeric_limits<float>::infinity(), 0);
                spmm(n, m) = Tuple(max(spmm(n, m)[0], e_binary_op),
                                   select(spmm(n, m)[0] < e_binary_op,
                                          spmm(n, m)[1], e_arg_idx));
                break;
            case SUM:
            default:
                spmm(n, m) = 0.0f;
                spmm(n, m) += e_binary_op;
            }

            out(n, m) = spmm(n, m);

            out.split(n, no, ni, nt * nthread).split(ni, nii, ni, nthread);
            out.split(m, mo, mi, mt * mthread)
                .split(mi, mi, mii, mt); // tile inner
            // .split(mi, mii, mi, mthread);  // tile outer. some cases fail.
            // we can not know which is better now(tile inner or outer).
            switch (do_keep_order) {
            // case 1:
            // out.reorder(
            //     /* tiles */ nii, mii,
            //     /* threads */ ni, mi,
            //     /* blocks */ no, mo); // May cause products
            //     CUDA_ERROR_INVALID_VALUE error.
            // break;
            default:
                out.reorder(
                    /* tiles */ nii, mii,
                    /* threads */ ni, mi,
                    /* blocks */ mo,
                    no); // block tile, `mo` first, so the block scheduler will
                         // do better than `no` first.
            }
            spmm.compute_at(out, ni);
            spmm.update().reorder(n, k);

            // unroll
            // NOTE: This unroll is neccesay for TraceComputePass in 2d register
            // tile opt.
            spmm.unroll(n).unroll(m);
            spmm.update().unroll(n);
            spmm.update().unroll(m);
            out.unroll(nii).unroll(mii);

            // Schedule GPU.
            out
                // .gpu_blocks(mo)
                .gpu_blocks(mo, no)
                .gpu_threads(ni)
                .gpu_threads(mi);

            // Set dim bounds.
            out.bound(n, 0, nn).bound(m, 0, row);

            // Custom passes(partialfuse).
            AddPartialfusePass(out); // 6 passes
            // AddRemoveRedundantComputePass pass should happen before
            // AddPartialPreloadPass.
            AddRemoveRedundantComputePass(out); // 2 passes
            if (do_preload) {
                // auto preload_func = [&](ImageParam &param) {
                auto preload_func = [&](std::vector<std::string> buf_names) {
                    AddPartialPreloadPass(
                        out, buf_names, target, /*k tile size*/ kt,
                        /*row tile size*/ mt, /*row thread size*/ mthread,
                        /*column thread size*/ nthread);
                };
                std::vector<std::string> buf_names;
                switch (op2int(
                    op)) { // TODO(jiny): should have a config for preloading.
                case MUL:
                    buf_names.push_back(iedge_map.name());
                    buf_names.push_back(colidx.name());
                    break;
                case COPY_RHS:
                    buf_names.push_back(iedge_map.name());
                    break;
                case COPY_LHS:
                    buf_names.push_back(colidx.name());
                    break;
                default:
                    assert("UNKNOWN_OP");
                }
                preload_func(buf_names);
            }
            AddRowBalancePass(out, mthread, do_row_balance);
        } else {
            spmm(n, m) = 0.0f;
            spmm(n, m) += b(n, cl_col);
            out(n, m) = spmm(n, m);
            // out.parallel(m).parallel(n);
        }

        fout = out;
        // fout.compile_jit(); // custom passes(like UnfoldLets) have some state
        // which can not be re-entry, so don't compile twice.
    }
};

} // namespace gpc
#endif // GPC_OPS_SPMM_H
