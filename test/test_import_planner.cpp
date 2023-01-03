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

#include "fma-common/logging.h"
#include "gtest/gtest.h"
#include "import/import_planner.h"
#include "./random_port.h"
#include "./ut_utils.h"
using namespace fma_common;

class TestImportPlanner : public TuGraphTest {};

TEST_F(TestImportPlanner, ImportPlanner) {
    {
        // simple test 1
        lgraph::import::LabelGraph graph(3);
        graph.AddVFileSize(0, 3);
        graph.AddVFileSize(1, 20);
        graph.AddVFileSize(2, 10);
        graph.AddEFileSize(0, 2, 10);
        size_t max_memory;
        std::vector<size_t> plan = graph.CalculateBestPlan(max_memory);
        UT_EXPECT_EQ(max_memory, 20);
        UT_EXPECT_EQ(plan[0], 1);
        UT_EXPECT_EQ(plan[1], 2);
        UT_EXPECT_EQ(plan[2], 0);

        lgraph::import::PlanExecutor executor(plan, graph.GetEFileSize());
        while (true) {
            lgraph::import::PlanExecutor::Action action = executor.GetNextAction();
            if (action.type == lgraph::import::PlanExecutor::Action::DONE) break;
            switch (action.type) {
            case lgraph::import::PlanExecutor::Action::LOAD_VERTEX:
                {
                    UT_LOG() << "Load vertex " << action.vid;
                    break;
                }
            case lgraph::import::PlanExecutor::Action::LOAD_EDGE:
                {
                    UT_LOG() << "Load edge (" << action.edge.first << "," << action.edge.second
                             << ")";
                    break;
                }
            case lgraph::import::PlanExecutor::Action::DUMP_VERTEX:
                {
                    UT_LOG() << "Dump vertex " << action.vid;
                    break;
                }
            case lgraph::import::PlanExecutor::Action::DONE:
                {
                    break;
                }
            }
        }
    }
    {
        RandomSeed seed(1);
        size_t n = 7;
        lgraph::import::LabelGraph graph(n);
        for (size_t i = 0; i < n; i++) {
            if (i == 5) {
                graph.AddVFileSize(i, 1000);
                continue;
            } else {
                graph.AddVFileSize(i, rand_r(&seed) % 10 + i);
            }
            size_t ne = rand_r(&seed) % n;
            for (size_t j = 0; j < ne; j++) {
                size_t dst = rand_r(&seed) % n;
                if (dst == 5) continue;
                graph.AddEFileSize(i, dst, rand_r(&seed) % 10 + i + j);
            }
        }
        UT_LOG() << "Graph:\n" << ToString(graph);

        size_t max_memory;
        std::vector<size_t> plan = graph.CalculateBestPlan(max_memory);
        UT_LOG() << "plan:" << ToString(plan);
        UT_LOG() << "memory: " << max_memory;
        lgraph::import::PlanExecutor executor(plan, graph.GetEFileSize());
        UT_LOG() << "actions: " << ToString(executor);
    }
}
