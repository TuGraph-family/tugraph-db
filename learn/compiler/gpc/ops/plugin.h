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

#ifndef GPC_OPS_PLUGIN_H
#define GPC_OPS_PLUGIN_H

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "compiler/gpc/ops/utils.h"

namespace plugin {

bool use_cache() {
    char *usecache = getenv("GPC_USECACHE");
    if (!usecache)
        return false;
    if (std::string(usecache) == "1") {
        return true;
    }
    return false;
}

#define COMPUTE()                                                              \
    void Compute(const std::vector<void *> &inputs,                            \
                 const std::vector<void *> &outputs);

class BaseOp {
  public:
    void *op = nullptr;
    std::vector<int> params;
    std::vector<int> config;
    virtual ~BaseOp() = default;
    BaseOp(const std::vector<int> &params, const std::vector<int> &config)
        : params(params), config(config) {}
    void PrintCost(const std::vector<std::vector<int>> &option_space,
                   const std::vector<double> &run_time_list);
    std::vector<std::vector<int>>
    FilterWithCost(const std::vector<std::vector<int>> &option_space);
};

class Spmm : public BaseOp {
  public:
    Spmm(const std::vector<int> &params, const std::vector<int> &config,
         const std::string &sop, const std::string &sreduce_op);
    Spmm(const std::vector<int> &params, const std::string &sop,
         const std::string &sreduce_op);

    ~Spmm() = default;
    COMPUTE();

  private:
    std::vector<int> get_config(const std::string &sop,
                                const std::string &sreduce_op,
                                const std::vector<int> &params) {
        // auto config = tune(sop, sreduce_op, params);
        return std::vector<int>{}; // default empty
    }

    std::vector<int> tune(const std::vector<void *> &inputs,
                          const std::vector<void *> &outputs);

    std::string get_key(const std::string &sop, const std::string &sreduce_op,
                        const std::vector<int> &params) {
        std::stringstream ss;
        ss << sop;
        ss << sreduce_op;
        for (auto &p : params) {
            ss << p;
        }
        return ss.str();
    }

    std::vector<int> get_config_cache(const std::string &sop,
                                      const std::string &sreduce_op,
                                      const std::vector<int> &params) {
        if (!use_cache()) {
            return std::vector<int>{};
        }

        auto m = params[0];
        auto n = params[1];
        switch (m) {
        case 232965:
            return reddit(sop, n);
        default:
            return std::vector<int>{1, std::min(n, 32), 1, 16, 16, 1, 1};
        }
    }

    std::vector<int> reddit(const std::string &sop, int n) {
        // Reddit Dataset, spmm copy_lhs sum
        // nt, nth, mt, mth, kt, row_balance, shared mem, keep order
        switch (n) {
        case 1:
            if (sop == "mul") {
                return std::vector<int>{1, 1, 1, 64, 16, 0, 1};
            } else {
                return std::vector<int>{1, 1, 1, 32, 8, 0, 0};
            }
        case 2:
            return std::vector<int>{1, 2, 1, 16, 16, 0, 0};
        case 4:
            return std::vector<int>{1, 4, 1, 8, 16, 1, 1};
        case 8:
            return std::vector<int>{1, 8, 1, 64, 32, 1, 1};
        case 16:
            return std::vector<int>{1, 16, 1, 32, 64, 1, 1};
        case 32:
            return std::vector<int>{2, 16, 1, 32, 128, 1, 1};

        case 41:
            if (sop == "mul") {
                return std::vector<int>{1, 41, 1, 4, 656, 0, 1};
            } else {
                return std::vector<int>{1, 41, 1, 16, 328, 0, 1};
            }

        case 64:
        case 128:
        case 256:
            if (sop == "mul") {
                return std::vector<int>{1, 32, 1, 4, 512, 0, 1};
            } else {
                return std::vector<int>{1, 32, 1, 8, 512, 0, 1};
            }

        case 512:
        case 1024:
            return std::vector<int>{2, 32, 1, 16, 128, 1, 1};

        case 602:
            return std::vector<int>{4, 16, 1, 32, 64, 1, 1};
        default:
            return std::vector<int>{};
        }
    }

  private:
    std::string sop;
    std::string sreduce_op;
};

class Sddmm : public BaseOp {
  public:
    Sddmm(std::vector<int> params, std::vector<int> config,
          std::string op_string);
    Sddmm(const std::vector<int> &params, const std::string &op_string);

    ~Sddmm() = default;
    COMPUTE();

  private:
    std::string get_key(const std::string &op, const std::vector<int> &params) {
        std::stringstream ss;
        ss << op;
        for (auto &p : params) {
            ss << p;
        }
        return ss.str();
    }

    std::vector<int> get_config(const std::string &op,
                                const std::vector<int> &params) {
        if (op == "dot") {
            switch (params[0]) {
            case 232965:
                return reddit(params[4]);
            case 2449029:
                return products(params[4]);
            case 132534:
                return proteins(params[4]);
            default:
                std::cout << "default is empty options.\n";
                return std::vector<int>{16, 4};
            }
        } else {
            return std::vector<int>{32, 4};
        }
    }

    std::vector<int> proteins(int n) {
        switch (n) {
        case 1:
            return std::vector<int>{512, 4};
        case 2:
        case 4:
            return std::vector<int>{128, 2};
        case 8:
            return std::vector<int>{64, 4};
        case 16:
            return std::vector<int>{128, 4};
        case 32:
            return std::vector<int>{16, 8};
        case 41:
            return std::vector<int>{};
        case 64:
            return std::vector<int>{32, 8};
        case 128:
        case 256:
            return std::vector<int>{16, 16};
        case 512:
        case 1024:
            return std::vector<int>{8, 32};
        default:
            return std::vector<int>{};
        }

        return std::vector<int>{};
    }

    std::vector<int> products(int n) {
        switch (n) {
        case 1:
            return std::vector<int>{256, 1};
        case 2:
            return std::vector<int>{256, 2};
        case 4:
            return std::vector<int>{256, 2};
        case 8:
            return std::vector<int>{32, 4};
        case 16:
            return std::vector<int>{16, 8};
        case 32:
            return std::vector<int>{8, 16};
        case 64:
            return std::vector<int>{8, 16};
        case 128:
            return std::vector<int>{8, 16};
        case 256:
            return std::vector<int>{8, 32};
        case 512:
            return std::vector<int>{8, 32};
        case 1024:
            return std::vector<int>{8, 32};
        default:
            return std::vector<int>{};
        }

        return std::vector<int>{};
    }

    std::vector<int> reddit(int n) {
        switch (n) {
        case 1:
        case 2:
        case 4:
        case 8:
            return std::vector<int>{128, 2};
        case 16:
            return std::vector<int>{64, 4};
        case 32:
            return std::vector<int>{32, 4};
        case 41:
            return std::vector<int>{16, 4};
        case 64:
        case 128:
        case 256:
            return std::vector<int>{8, 16};
        case 512:
            return std::vector<int>{4, 32};
        case 1024:
            return std::vector<int>{2, 32};
        default:
            return std::vector<int>{};
        }
        return std::vector<int>{};
    }
};

} // namespace plugin

#endif // GPC_OPS_PLUGIN_H
