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

#include "compiler/gpc/cost/gpu_cost.h"

#include <iomanip>
#include <sstream>

namespace gpc {

// {"mem_eff", "parall", "instr_parall", "reuse", "spill", "blck_size"};
std::vector<std::string> GpuCost::metrics() const {
    std::vector<std::string> result;
    for (int i = 0; i < metric_list_.size(); ++i) {
        result.push_back(std::to_string(i));
    }
    return result;
}

// {gld_eff_, parallelism_degree_, latency_parallel_degree_, data_reuse_,
//         spill_,   block_size_};
std::vector<double> GpuCost::metric_values() const { return metric_list_; }

std::string GpuCost::DebugString() const {
    std::stringstream ss;

    ss << std::fixed;
    ss << std::setprecision(2);

    ss << "{";

    ss << "memory efficiency " << gld_eff_ * 100 << "%, ";
    ss << "parallelism degree " << parallelism_degree_ << ",";
    ss << "lantency parallel degree " << latency_parallel_degree_ << ",";
    ss << "data reuse degree " << data_reuse_ << ",";
    ss << "spill " << spill_ << ",";
    ss << "block_size " << block_size_ << ",";

    ss << "}";
    return ss.str();
}

std::ostream &operator<<(std::ostream &os, const GpuCost &cost) {
    os << cost.DebugString();
    return os;
}

} // namespace gpc
