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

#ifndef GPC_COST_GPU_COST_H_
#define GPC_COST_GPU_COST_H_

#include "math.h"

#include <iostream>
#include <string>
#include <vector>

namespace gpc {

/*
cost function = RECIPROCAL(
    parallelism_degree *
    memory_access_eff * register_reuse
    * (shared_mem_reuse) ^ 1/3 * (1/shared_mem_bank_conflict) ^ 1/5
    * branch_eff
)
memory_access_eff = gld_eff * input.size() + gst_eff * output.size()
*/

class GpuCost {
  public:
    GpuCost(int init = -1) { init_ = init; }

    GpuCost(const std::vector<double> &metric_list)
        : metric_list_(metric_list) {}

    // TODO(jiny): Refactor this constructor to GpuCost([features],
    // [feature_weights])
    GpuCost(double gld_eff, double parallelism_degree,
            double latency_parallel_degree, double data_reuse = 1,
            double spill = 1, double block_size = 1)
        : gld_eff_(gld_eff), parallelism_degree_(parallelism_degree),
          latency_parallel_degree_(latency_parallel_degree),
          data_reuse_(data_reuse), spill_(spill), block_size_(block_size) {}
    ~GpuCost() = default;

    double value() const {
        // TODO(jinyue): parallelism_degree do not work well now.
        // Turn it on later if we have good strategy.
        // NOTE: use 1000 to make the value nice without being too little.
        return 1000 / (gld_eff_ * latency_parallel_degree_ * data_reuse_);
    }

    std::vector<double> metric_values() const;
    std::vector<std::string> metrics() const;
    std::string DebugString() const;
    friend std::ostream &operator<<(std::ostream &os, const GpuCost &cost);

  private:
    std::vector<double> metric_list_;
    int init_ = -1;
    double gld_eff_ = -1;
    double parallelism_degree_ = -1;
    double latency_parallel_degree_ = -1;
    double data_reuse_ = -1;
    double spill_ = -1;
    double block_size_ = -1;
};

} // namespace gpc

#endif // GPC_COST_GPU_COST_H_
