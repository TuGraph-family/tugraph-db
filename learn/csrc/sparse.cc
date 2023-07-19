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

#include <iostream>
#include <torch/extension.h>

#include "ops/plugin.h"
#include "ops/utils.h"

std::string get_key(const std::string &op, const std::string &reduce, int m,
                    int n, int nnz) {
    std::stringstream ss;
    ss << op;
    ss << reduce;
    ss << m;
    ss << ",";
    ss << n;
    ss << ",";
    ss << nnz;
    return ss.str();
}

static std::map<std::string, void *> &get_spmm_caches() {
    static std::map<std::string, void *> spmm_caches;
    return spmm_caches;
}

int CountSize(int start, torch::Tensor &tensor) {
    int count = 1;
    auto dim = tensor.dim();
    for (int i = start; i < dim; ++i) {
        count *= tensor.size(i);
    }
    return count;
}

// csr: rowPtr, colIndices, edgeMap.
void spmm_apply(const std::string &op, const std::string &reduce,
                torch::Tensor sorted_row, std::vector<torch::Tensor> csr,
                torch::Tensor ufeat, torch::Tensor efeat, torch::Tensor out,
                std::vector<torch::Tensor> out_aux) {
    int m = csr[0].size(0) - 1;
    int k = m;
    int nnz = csr[1].size(0);
    int n = CountSize(1, out);
    int rhs_len = 1;
    if (efeat.size(0) != 0) {
        rhs_len = CountSize(1, efeat);
    }

    std::vector<int> params{m, n, nnz / m, nnz, rhs_len};

    auto spmm_op = new plugin::Spmm(params, op, reduce);

    // compute.
    float *pefeat = efeat.data_ptr<float>();
    std::vector<void *> inputs{
        ufeat.data_ptr<float>(), sorted_row.data_ptr<int>(),
        csr[0].data_ptr<int>(),  csr[1].data_ptr<int>(),
        csr[2].data_ptr<int>(),  pefeat,
    };
    if (op == "copy_lhs") {
        std::vector<void *> outputs{out.data_ptr<float>(),
                                    out_aux[0].data_ptr<int>()};
        spmm_op->Compute(inputs, outputs);
    } else if (op == "copy_rhs") {
        std::vector<void *> outputs{out.data_ptr<float>(),
                                    out_aux[1].data_ptr<int>()};
        spmm_op->Compute(inputs, outputs);
    } else {
        // u_mul_e
        std::vector<void *> outputs{out.data_ptr<float>(), nullptr};
        spmm_op->Compute(inputs, outputs);
    }
}

void sddmm_apply(const std::string &op, std::vector<at::Tensor> coo,
                 at::Tensor lhs, at::Tensor rhs, at::Tensor out, int lhs_target,
                 int rhs_target) {
    int nodes = lhs.size(0);
    int edges = coo[0].size(0);
    int lhs_len = CountSize(1, lhs);
    int rhs_len = CountSize(1, rhs);
    int reduce_len = 1;
    assert(lhs_len == rhs_len && "Should be equal in gpc.");
    if (op == "dot") {
        reduce_len = lhs_len;
    }
    bool has_idx = false;
    std::vector<int> params{nodes,      edges,      lhs_len,    rhs_len,
                            reduce_len, lhs_target, rhs_target, has_idx};

    auto sddmm = new plugin::Sddmm(params, op);

    std::vector<void *> inputs{
        coo[0].data_ptr<int>(), coo[1].data_ptr<int>(), nullptr,
        lhs.data_ptr<float>(),  rhs.data_ptr<float>(),  out.data_ptr<float>()};
    std::vector<void *> outputs{out.data_ptr<float>()};
    sddmm->Compute(inputs, outputs);
}

PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) {
    m.def("spmm", &spmm_apply, "GPC spmm operator.");
    m.def("sddmm", &sddmm_apply, "GPC sddmm operator.");
}
