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

#ifndef GPC_OPS_SDDMM_H
#define GPC_OPS_SDDMM_H

#include "compiler/gpc/ops/utils.h"

#include "third_party/Halide/include/Halide.h"
#include "third_party/Halide/include/HalideRuntimeCuda.h"
#include "third_party/Halide/tools/halide_benchmark.h"

using namespace Halide;
using namespace Halide::Internal;

namespace gpc {

class Sddmm {
  public:
    int op_enum;

    int nodes;
    int edges;
    int lhs_len;
    int rhs_len;
    int reduce_size;
    int lhs;
    int rhs;
    int has_idx;

    Buffer<int> irow;
    Buffer<int> icol;
    Buffer<int> iedge;
    std::vector<Buffer<float>> iuev; // u, e, v
    Buffer<float> bout;

    Func ifrow;
    Func ifcol;
    Func ifedge;
    Func ifu;
    Func ife;
    Func ifv;
    Func fout;
    Func fintm;

    Target target;
    const halide_device_interface_t *interface;

    std::string s_max_registers;

    enum SddmmOp { DOT, ADD, SUB, MUL, DIV, UNKNOWN_OP };

    static SddmmOp op2int(std::string op) {
        if (op == "dot")
            return DOT;
        if (op == "add")
            return ADD;
        if (op == "sub")
            return SUB;
        if (op == "mul")
            return MUL;
        if (op == "div")
            return DIV;
        return UNKNOWN_OP;
    }

    inline void attach(const std::vector<void *> &inputs,
                       const std::vector<void *> &outputs) {
#define ATTACH(B, PTRI)                                                        \
    if (inputs[PTRI])                                                          \
        B.device_wrap_native(interface, uintptr_t(inputs[PTRI]));
#define ATTACHO(B, PTRI)                                                       \
    B.device_wrap_native(interface, uintptr_t(outputs[PTRI]));
        ATTACH(irow, 0);
        ATTACH(icol, 1);
        ATTACH(iedge, 2);
        ATTACH(iuev[lhs], 3);
        ATTACH(iuev[rhs], 4);
        ATTACHO(bout, 0);
    }

    inline void detach() {
#define DETACH(B)                                                              \
    if (B.raw_buffer()->device)                                                \
        B.device_detach_native();
        DETACH(irow);
        DETACH(icol);
        DETACH(iedge);
        DETACH(iuev[lhs]);
        DETACH(iuev[rhs]);
        DETACH(bout);
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

        if (irow.raw_buffer()->device) {
            detach();
        }
        attach(inputs, outputs);
        fout.realize(bout, target);
    }

    Sddmm(std::vector<int> params, std::vector<int> config, std::string op)
        : op_enum(op2int(op)) {

        // Set target.
        target = get_host_target();
        target.set_feature(Target::CUDA);
        target.set_feature(Target::CUDACapability50);
        interface = get_device_interface_for_device_api(Halide::DeviceAPI::CUDA,
                                                        target);

        // inputs' dimentions
        nodes = params[0];
        edges = params[1];
        lhs_len = params[2];
        rhs_len = params[3];
        reduce_size = params[4];
        lhs = params[5];
        rhs = params[6];
        has_idx = params[7];

        if (op != "dot" && reduce_size != 1) {
            // TODO: To be implemented. We can easily support reduce_size > 1 in
            // the future.
            std::cerr << "add and other ele ops's reduce_size should be 1.\n";
        }

        if (!(lhs_len == rhs_len && lhs_len == reduce_size)) {
            std::cerr << "lhs, rhs, reduce: " << lhs_len << " " << rhs_len
                      << " " << reduce_size << std::endl;
        }

        // Create buffer
        irow = Buffer<int>(edges, "row");
        icol = Buffer<int>(edges, "col");
        iedge = Buffer<int>(edges, "edge");
        iuev.push_back(Buffer<float>({lhs_len, nodes}, "u"));
        // Too big w_feat_len_ will cause overflow int32 of halide buffer.
        if (lhs == 1 || rhs == 1) {
            iuev.push_back(Buffer<float>({rhs_len, edges}, "e"));
        } else {
            iuev.push_back(
                Buffer<float>({1, 1}, "e")); // We do not use it. Just a fake.
        }
        iuev.push_back(Buffer<float>({rhs_len, nodes}, "v"));
        bout = Buffer<float>(edges);

        ifrow = Func(irow);
        ifcol = Func(icol);
        ifedge = Func(iedge);
        ifu = Func(iuev[0]);
        ife = Func(iuev[1]);
        ifv = Func(iuev[2]);

        Var vedge("vedge");
        auto row = promise_clamped(ifrow(vedge), 0, nodes - 1);
        auto edge = promise_clamped(ifedge(vedge), 0, nodes - 1);
        auto col = promise_clamped(ifcol(vedge), 0, nodes - 1);

        std::vector<Expr> idx(3);
        idx[0] = row;
        idx[1] = has_idx ? edge : vedge;
        idx[2] = col;
        std::vector<Func> ifs(3);
        ifs[0] = ifu;
        ifs[1] = ife;
        ifs[2] = ifv;
        auto leftidx = idx[lhs];
        auto rightidx = idx[rhs];
        auto leftex = ifs[lhs](0, idx[lhs]);
        auto rightex = ifs[rhs](0, idx[rhs]);
        Func sddmm("sddmm");
        Func out("o" + vector2string(config));
        switch (op_enum) {
        case DOT: {
            RDom rk(0, reduce_size);
            sddmm(vedge) = 0.0f;
            sddmm(vedge) += ifs[lhs](rk, leftidx) * ifs[rhs](rk, rightidx);
        }

        break;
        case ADD:
            sddmm(vedge) = leftex + rightex;
            break;
        case SUB:
            sddmm(vedge) = leftex - rightex;
            break;
        case MUL:
            sddmm(vedge) = leftex * rightex;
            break;
        case DIV:
            sddmm(vedge) = leftex / rightex;
            break;
        default:
            std::cerr << "UNKNOWN OP\n";
            abort();
        }
        out(vedge) = sddmm(vedge);
        fout = out;
        fintm = sddmm;

        // ===============
        // Schedule
        // ===============
        // kernel config
        int tx = config[0];
        int vecx = config[1];

        auto y = fout.args()[0];
        const int warp_threshold = 2;
        if (op_enum == DOT && reduce_size >= warp_threshold) {
            auto rx = fintm.rvars()[0];
            Var yi, ylane, u("u");
            fout.split(y, y, yi, tx)
                .gpu_blocks(y)
                .split(yi, yi, ylane, 1)
                .gpu_threads(yi, ylane);
            RVar ro("ro"), ri("ri");
            Func rintm = fintm.update()
                             .split(rx, ri, ro, vecx)
                             .reorder(ri, ro)
                             .rfactor(ro, u); // vecx is warp size
            rintm.compute_at(fout, yi).update().gpu_lanes(u);
            rintm.gpu_lanes(u);

            // unroll seems to has little effect on acceleration.
            fintm.update().unroll(ro);
            rintm.update().unroll(ri);
        } else {
            Var vo, vi, vv;
            fout.split(y, vo, vi, tx, TailStrategy::GuardWithIf)
                .split(vo, vo, vv, vecx)
                .gpu_threads(vi)
                .gpu_blocks(vo);  // vectorize make no better here.
            if (op_enum == DOT) { // debug
                auto rx = fintm.rvars()[0];
                fintm.update().unroll(rx);
            }
            if (vecx > 1)
                fout.vectorize(vv);
        }

        // Set max registers env
        int threads = tx;
        if (op_enum == DOT && reduce_size >= warp_threshold) {
            threads *= vecx;
        }
        int max_registers = 65536 / threads;
        max_registers =
            (max_registers >= 255 ? 255 : (max_registers >= 128 ? 128 : 64));
        s_max_registers = std::to_string(max_registers);

        // Set dim bounds.
        out.bound(vedge, 0, edges);
        out.output_buffers()[0].dim(0).set_min(0);
    }
};

} // namespace gpc
#endif // GPC_OPS_SDDMM_H
