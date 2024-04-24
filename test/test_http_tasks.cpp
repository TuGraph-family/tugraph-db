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

#include "gtest/gtest.h"
#include "http/algo_task.h"
#include "http/import_task.h"
#include "http/http_utils.h"
#include "db/galaxy.h"
#include "./ut_utils.h"
#include "./test_tools.h"

class TestHttpTasks : public TuGraphTest {};

TEST_F(TestHttpTasks, HttpAlgoTask) {
    GTEST_SKIP() << "Temporarily disable TestHttpTasks.HttpAlgoTask";
    using namespace lgraph;

    std::string db_dir = "./testdb";
    AutoCleanDir cleaner(db_dir);
    StateMachine::Config sm_config;
    sm_config.db_dir = db_dir;
    StateMachine state_machine(sm_config, nullptr);
    http::HttpService http_service(&state_machine);

    const std::string taskId = http::GetRandomUuid();
    const std::string token = "";  // TODO(qsp): get token
    const std::string algoName = "pagerank";
    const std::string algoType = "cpp";
    const std::string version = "v1";
    const std::string jobParam = "{\"max_iter\": 10}";
    const std::string nodeType = "node";
    const std::string edgeType = "edge";
    const std::string outputType = "file";
    const double timeout = 3600.0;
    const bool inProcess = false;

    http::AlgorithmTask task(&http_service, taskId, "test_task", "admin", token, "default",
                             algoName, algoType, version, jobParam, nodeType, edgeType, outputType,
                             timeout, inProcess);
    // task();  // TODO(qishipeng): run the task. signal 11 here.
}

TEST_F(TestHttpTasks, HttpImportTask) {
    // TODO(anyone): add test for import task
}
