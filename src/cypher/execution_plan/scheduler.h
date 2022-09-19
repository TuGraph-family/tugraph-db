/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

//
// Created by wt on 18-8-14.
//
#pragma once

#include <list>
#include <unordered_map>
#include <mutex>

#include "core/lightning_graph.h"
#include "db/db.h"
#include "plugin/cpp_plugin.h"
#include "plugin/python_plugin.h"

#include "execution_plan/execution_plan.h"
#include "lru_cache.h"

namespace lgraph {
class StateMachine;
}

namespace cypher {

struct ElapsedTime {
    double t_total = 0;
    double t_compile = 0;
    double t_exec = 0;
};

class ExecutionPlan;
class Scheduler {
 public:
    void Eval(RTContext *ctx, const std::string &script, ElapsedTime &elapsed);

    std::shared_ptr<ExecutionPlan> ParseQuery(RTContext *ctx, const std::string &script,
                                              double &elapsed) {
        return nullptr;
    }

    static bool DetermineReadOnly(const std::string &script, std::string& name, std::string& type);
};
}  // namespace cypher
