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

#include "compiler/gpc/ops/plugin.h"
#include "compiler/gpc/cost/gpu_cost.h"
#include "compiler/gpc/ops/sddmm.h"
#include "compiler/gpc/ops/spmm.h"
#include "compiler/gpc/ops/utils.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <cuda_runtime.h>

namespace plugin {
using namespace gpc;

std::map<std::string, gpc::Sddmm *> &get_sddmm_cache(const std::string &key) {
    static std::map<std::string, gpc::Sddmm *> sddmm_caches;
    return sddmm_caches;
}

std::map<std::string, gpc::Spmm *> &get_spmm_cache(const std::string &key) {
    static std::map<std::string, gpc::Spmm *> spmm_caches;
    return spmm_caches;
}

inline std::string DebugString(std::vector<int> option) {
    std::stringstream ss;
    ss << "{";
    int option_size = option.size();
    for (int i = 0; i < option_size - 1; ++i)
        ss << std::to_string(option[i]) << ",";
    if (option_size != 0)
        ss << std::to_string(option[option_size - 1]);
    ss << "}";
    return ss.str();
}

#define OP_COMPUTE(CLASS)                                                      \
    void CLASS::Compute(const std::vector<void *> &inputs,                     \
                        const std::vector<void *> &outputs) {                  \
        reinterpret_cast<gpc::CLASS *>(op)->Compute(inputs, outputs);          \
    }

Spmm::Spmm(const std::vector<int> &params, const std::vector<int> &config,
           const std::string &sop, const std::string &sreduce_op)
    : BaseOp(params, config), sop(sop), sreduce_op(sreduce_op) {
    // If we already have config, then just build op with this config
    // It's for tuning.
    if (config.size() > 0) {
        op = new gpc::Spmm(true, false, params, config, sop, sreduce_op);
    }
}

Spmm::Spmm(const std::vector<int> &params, const std::string &sop,
           const std::string &sreduce_op)
    : BaseOp(params, get_config_cache(sop, sreduce_op, params)), sop(sop),
      sreduce_op(sreduce_op) {
    auto key = get_key(sop, sreduce_op, params);
    auto &op_cache = get_spmm_cache(key);
    if (op_cache.count(key) > 0) {
        op = op_cache[key];
        return;
    }
    // No op cache, but have config cache.
    if (config.size() > 0) {
        op = new gpc::Spmm(true, false, params, config, sop, sreduce_op);
        op_cache[key] = (gpc::Spmm *)op;
    }
}

// OP_COMPUTE(Spmm)

Sddmm::Sddmm(std::vector<int> params, std::vector<int> config, std::string sop)
    : BaseOp(params, config) {
    op = new gpc::Sddmm(params, config, sop);
}

Sddmm::Sddmm(const std::vector<int> &params, const std::string &sop)
    : BaseOp(params, get_config(sop, params)) {
    auto key = get_key(sop, params);
    auto &op_cache = get_sddmm_cache(key);
    if (op_cache.count(key) > 0) {
        op = op_cache[key];
    } else {
        op = new gpc::Sddmm(params, config, sop);
        op_cache[key] = (gpc::Sddmm *)op;
    }
}

OP_COMPUTE(Sddmm)

GpuCost SpmmCost(BaseOp *op, const std::vector<int> &option);
GpuCost SddmmCost(BaseOp *op, const std::vector<int> &option);

GpuCost BaseOpCost(BaseOp *op, const std::vector<int> &option) {
    if (dynamic_cast<Spmm *>(op)) {
        return SpmmCost(op, option);
    } else if (dynamic_cast<Sddmm *>(op)) {
        return SddmmCost(op, option);
    }
    return GpuCost();
}

GpuCost SpmmCost(BaseOp *op, const std::vector<int> &option) {
    double feat_len = op->params[1];
    double nodes = op->params[0];
    double nnz = op->params[3];
    int thread_x = option[1], thread_y = option[3];
    int rz_tile = option[4];
    int tile_x = option[0];
    int threads = thread_x * thread_y;

    // TODO 补充其他metrix，参考matrix multiply of dense.
    double nt = option[0];
    double nthread = option[1];
    double mt = option[2];
    double mthread = option[3];
    double kt = option[4];
    double do_row_balance = option[5] + 1;
    double do_preload = option[6] + 1;
    // int do_keep_order = config[7];

    // eff
    double col_eff = std::min(thread_x / 8.0, 1.0);
    double ratio = 1.0 * nnz / ((2 * nodes * feat_len) + nodes + 1 + nnz);
    double eff = col_eff * ratio;

    // parallel degree.
    double parallel = 1.0 / tile_x;

    // instruction parallel
    double ins_parallel = tile_x;

    // data reuse
    double data_reuse = 1.0 * tile_x / ins_parallel;

    // block size penalty
    double block_size_penalty = threads >= 128 ? 1 : 128.0 / threads;

    // share memory size portion.
    double share = 1.0 * rz_tile * thread_x;

    double feat_factor = thread_x >= 8 ? 1.0 : 2.0;

    return GpuCost(std::vector<double>{nt, nthread, do_row_balance, mthread, kt,
                                       do_preload, feat_len, nodes, nnz});
}

std::string CurrentTime() {
    auto time = std::time(nullptr);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%T");
    auto s = ss.str();
    std::replace(s.begin(), s.end(), ':', '-');
    return s;
}

void DumpCsv(const std::string &file, const std::stringstream &ss) {
    std::ofstream of(file);
    if (of.fail()) {
        std::cerr << "open failure as expected: " << std::strerror(errno)
                  << '\n';
    }
    of << ss.str();
    of.close();
}

void BaseOp::PrintCost(const std::vector<std::vector<int>> &option_space,
                       const std::vector<double> &run_time_list) {
    std::vector<GpuCost> gpu_costs;
    for (const auto &option : option_space) {
        gpu_costs.push_back(BaseOpCost(this, option));
    }
    bool print_cost = true;
    bool dump = true;

    std::vector<double> run_times(option_space.size());
    if (run_time_list.size() == 0) {
        std::iota(run_times.begin(), run_times.end(), 1); // Ranking
    } else {
        run_times = run_time_list;
    }

    // sort
    std::vector<size_t> indices(option_space.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&](size_t A, size_t B) -> bool {
        if (isnan(run_times[A]))
            return false;
        if (isnan(run_times[B]))
            return true;
        return run_times[A] < run_times[B];
    });

    std::stringstream ss;
    ss << std::fixed;
    ss << std::setprecision(2);
    std::string deli = ",";

    // Print a csv format infomation.
    // Print header.
    ss << std::left << std::setw(28) << "Option" + deli << std::left
       << std::setw(14) << "Time(us)" + deli << std::left << std::setw(14)
       << "Cost Time" + deli;
    const auto &cost_info_metric = gpu_costs[indices[0]];
    for (auto &i : cost_info_metric.metrics()) {
        ss << std::left << std::setw(14) << std::string(i) + deli;
    }

    for (auto i = 0; i < option_space.size(); ++i) {
        if (run_times[indices[i]] < 0) {
            // exclude invalid options.
            continue;
        }
        const auto &option = option_space[indices[i]];
        auto option_s = DebugString(option);
        std::replace(option_s.begin(), option_s.end(), ',', '_');
        ss << "\n";
        std::stringstream option_ss;
        option_ss << "s" << i << ":" << option_s << deli;
        ss << std::left << std::setw(28) << option_ss.str();
        // ==========================================================
        // use performance(1/time) instead of real time. If we use time, the
        // worst time option will dominate the loss function. Then the first 100
        // option will not predict well. It's very important.
        ss << std::left << std::setw(14)
           << std::to_string(1.0f / run_times[indices[i]]) + deli;
        // ==========================================================
        if (print_cost) {
            const auto &cost_info = gpu_costs[indices[i]];
            // Print cost value.
            ss << std::left << std::setw(14)
               << std::to_string(cost_info.value()) + deli;
            // Print cost info.
            for (auto &v : cost_info.metric_values()) {
                ss << std::left << std::setw(14) << std::to_string(v) + deli;
            }
        }
    }
    ss << "\n";
    std::string file_path = "./";
    if (dump) {
        DumpCsv(file_path + "/cost.csv", ss);
        DumpCsv(file_path + "/cost" + CurrentTime() + ".csv", ss);
    } else {
        std::cout << "\n" << ss.str();
    }
}

const std::string cost_model_bin = "./python/cost/learn.py";

std::vector<std::vector<int>>
BaseOp::FilterWithCost(const std::vector<std::vector<int>> &option_space) {
    std::vector<double> pref_predicts;

    // Predict the cost time with cost.csv using torch model.
    // Get back the cost predicts and save to pref_predicts.
    // Sort index with pref_predicts.

    // Dump option_space's cost features value to cost.csv
    PrintCost(option_space, {});

    // By default using cost.csv for inferring
    const std::string file_path = "./cost.csv";
    const std::string model_path = "./cost.model";
    const std::string command = "python " + cost_model_bin +
                                " --file=" + file_path +
                                " --model_file=" + model_path;
    std::cout << "Executing command: " << command << "\n";
    FILE *file = popen(command.c_str(), "r");
    if (file == nullptr) {
        std::cout << "Can not execute command: " << command << "\n";
    }
    std::string output;
    char str[256];
    while (fgets(str, 256, file) != nullptr) {
        output.append(str);
    }
    // std::cout << "cost predict output is " << output << "\n";
    std::istringstream f(output);
    std::string s;
    size_t i = 0;
    while (getline(f, s, ',') && i < option_space.size()) {
        ++i;
        pref_predicts.push_back(std::stod(s));
    }
    int status = pclose(file);
    if (status == -1) {
        std::cout << "popen failed: " << command << "\n";
    }

    // Sort the options with cost values.
    std::vector<size_t> indices(option_space.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&](size_t A, size_t B) -> bool {
        if (isnan(pref_predicts[A]))
            return false;
        if (isnan(pref_predicts[B]))
            return true;
        return pref_predicts[A] > pref_predicts[B];
    });

    // Trunc the option space, here we assume the first top filtered options are
    // good.
    const float kFilterTopRate = 0.1;
    size_t filter_size = option_space.size() * kFilterTopRate;
    // If zero size, we still use the primary option space.
    if (filter_size == 0)
        filter_size = option_space.size();
    std::vector<std::vector<int>> sorted_option_space(filter_size);
    std::vector<double> sorted_cost_list(filter_size);
    for (size_t i = 0; i < filter_size; ++i) {
        sorted_option_space[i] = option_space[indices[i]];
        sorted_cost_list[i] = pref_predicts[indices[i]];
        // std::cout << "option: " << DebugString(sorted_option_space[i]) << ",
        // "
        //        << sorted_cost_list[i] << "\n";
    }

    return sorted_option_space;
}

GpuCost SddmmCost(BaseOp *op, const std::vector<int> &option) {
    int feat_len = op->params[2];

    int thread_x = option[0];
    int tile_x = option[1];

    double out_eff = std::min(thread_x / 8.0, 1.0);
    double in_eff = std::min(tile_x / 8.0, 1.0);
    int warp_size = tile_x;
    double threads = warp_size * thread_x;
    double penalty = 1.0 * warp_size / feat_len;

    return GpuCost(out_eff, in_eff, 1, penalty, warp_size, threads);
}

void Spmm::Compute(const std::vector<void *> &inputs,
                   const std::vector<void *> &outputs) {
    if (op) {
        reinterpret_cast<gpc::Spmm *>(op)->Compute(inputs, outputs);
        return;
    }
    if (config.size() == 0) {
        config = tune(inputs, outputs); // first time compute, tune first.
        op = new gpc::Spmm(true, false, params, config, sop, sreduce_op);

        auto key = get_key(sop, sreduce_op, params);
        auto &op_cache = get_spmm_cache(key);
        op_cache[key] = (gpc::Spmm *)op;
        reinterpret_cast<gpc::Spmm *>(op)->Compute(inputs, outputs);
        return;
    }
    assert(false && "Not possible here.");
}

int TuneLevel() {
    char *tune_level = getenv("GPC_TUNELEVEL");
    if (!tune_level)
        return 1;
    if (std::string(tune_level) == "2") {
        return 2;
    }
    return 1;
}

std::vector<int> Spmm::tune(const std::vector<void *> &inputs,
                            const std::vector<void *> &outputs) {
    auto m = params[0];
    auto n = params[1];
    auto option = get_config_cache(sop, sreduce_op, params);
    if (option.size() > 0) {
        return option;
    }

    std::cout << "Tuning the key of SpMM: " << vector2string(params) << "\n";

    std::vector<int> ot = {1}; // ntile
    std::vector<int> oa;       // nthread
    std::vector<int> ob;       // mthread
    for (int i = std::min(8, n); i <= 128 && i <= n; i = i * 2) {
        oa.push_back(i);
    }
    if (n == 41) {
        oa.push_back(41);
    }
    for (int i = 4; i <= 64 && i <= m; i = i * 2) {
        ob.push_back(i);
    }
    std::vector<int> oc{1, 2, 4, 8, 16}; // k
    int nthread = std::min(n, 8);
    double time = std::numeric_limits<double>::infinity();
    std::vector<std::vector<int>> space;
    std::vector<int> shared_range = {0, 1};
    std::vector<int> rb_range = {0, 1};
    std::vector<int> mt_range = {1}; // mt can't tile.

    for (auto t : ot) {
        for (auto a : oa) {
            if (t * a > n)
                continue;
            for (auto b : ob) {
                if (a * b >= 1024 || a * b < 32)
                    continue;
                for (auto c : oc) {
                    for (auto s : shared_range) {
                        int factor = 1;
                        // if (sop == "mul") factor = 1; // TODO use option.
                        if (sop == "mul")
                            factor = 2;
                        if (s && a * b * c * 4 * factor > 48 * 1024)
                            continue;
                        for (auto row_balance : rb_range) {
                            for (auto mt : mt_range) {
                                auto config = std::vector<int>{
                                    t,     a,           mt, b,
                                    a * c, row_balance, s}; // nt, nth, mt, mth,
                                                            // kt, row_balance,
                                                            // shared mem
                                space.push_back(config);
                            }
                        }
                    }
                }
            }
        }
    }

    // Tune
    std::vector<int> result{16, nthread, 1, n / nthread, nthread, true, true};
    std::random_shuffle(space.begin(), space.end());
    std::cout << "search space: " << space.size() << ".\n";
    auto search_size = space.size();
    auto tune_level = TuneLevel();
    if (tune_level == 2) {
        std::cout << "heavy tune\n";
        search_size = search_size;
    } else {
        search_size = std::max(search_size / 10, (size_t)10);
    }
    for (int i = 0; i < search_size; ++i) {
        auto &config = space[i];
        auto spmm = new plugin::Spmm(params, config, sop, sreduce_op);
        {
            std::cout << "Timing config: " << vector2string(config) << "\n";
            spmm->Compute(inputs, outputs); // compile
            cudaDeviceSynchronize();
            gpc::LogTimeScope lts(vector2string(config).c_str(), false);
            spmm->Compute(inputs, outputs);
            cudaDeviceSynchronize();
            if (lts.GetTimeMS() < time) {
                result = config;
                time = lts.GetTimeMS();
                std::cout << "Get better time: " << time << "ms\n";
            }
        }
    }

    std::cout << "space size " << space.size() << "\n";
    std::cout << "Spmm: The best config is " << vector2string(result)
              << ". The time is " << time << " ms\n";

    return result;
};

} // namespace plugin
