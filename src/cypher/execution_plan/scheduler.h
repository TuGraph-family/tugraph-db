/**
 * Copyright 2022 AntGroup CO., Ltd.
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
#include "execution_plan/runtime_context.h"
#include "cypher/execution_plan/lru_cache.h"

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

    static bool DetermineReadOnly(cypher::RTContext *ctx,
                                  const std::string &script,
                                  std::string& name,
                                  std::string& type);
};
}  // namespace cypher
