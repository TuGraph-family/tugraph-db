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

#include <thread>
#include "fma-common/configuration.h"
#include "fma-common/logging.h"
#include "gtest/gtest.h"

#include "core/lightning_graph.h"
#include "plugin/cpp_plugin.h"
#include "protobuf/ha.pb.h"
#include "server/state_machine.h"
#include "./ut_utils.h"

using namespace lgraph;
class TestMthread : public TuGraphTest {};

#ifdef USELESS_CODE
void task1(PluginManager *pman, const std::string &plugin_name) {
    for (int i = 0; i < 10; i++) {
        std::string output;
        auto ec = pman->Call(PluginManager::PluginType::CPP, plugin_name, "", 0, false, output);
        UT_LOG() << std::this_thread::get_id() << "|" << i << ":Plugin returned " << ec << ": "
                 << output;
    }
}
#endif

void task2(StateMachine *sm, const std::string &plugin_name) {
    LGraphRequest proto_req;
    LGraphResponse proto_resp;
    proto_req.set_is_write_op(true);
    PluginRequest *preq = proto_req.mutable_plugin_request();
    preq->set_type(PluginRequest::CPP);
    CallPluginRequest *req = preq->mutable_call_plugin_request();
    req->set_name(plugin_name);
    double timeout = 0;
    req->set_timeout(timeout);
    bool in_process = false;
    req->set_in_process(in_process);
    proto_req.clear_is_write_op();
    UT_LOG() << "Forwarding to state machine";
    for (int i = 0; i < 10; i++) {
        sm->HandleRequest(nullptr, &proto_req, &proto_resp, nullptr);
        if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
            const std::string reply = proto_resp.plugin_response().call_plugin_response().reply();
            UT_LOG() << "Plugin request successfully returned:" << reply;
        } else {
            UT_LOG() << "Plugin request failed with RSM error " << proto_resp.error_code() << ": "
                     << proto_resp.error();
        }
    }
}

TEST_F(TestMthread, Mthread) {
    std::string plugin_dir = "./cpp_plugin", plugin_name = "sleep";
    int num_th = 4;
    int argc = _ut_argc;
    char ** argv = _ut_argv;
    fma_common::Configuration config;
    config.Add(plugin_dir, "cpp_plugin", true).Comment("Directory for C++ plugins");
    config.Add(plugin_name, "plugin", true).Comment("cpp plugin name");
    config.Add(num_th, "th", true).Comment("number of threads");
    config.ParseAndFinalize(argc, argv);

    std::vector<std::thread> threads;
    StateMachine::Config config_;
    config_.db_dir = "./testdb";
    std::unique_ptr<lgraph::StateMachine> state_machine(new lgraph::StateMachine(config_, nullptr));
    state_machine->Start();
    for (int i = 0; i < num_th; i++) {
        threads.emplace_back(std::thread(task2, state_machine.get(), plugin_name));
    }
    for (auto &t : threads) {
        t.join();
    }
    UT_LOG() << "All threads joined.\n";
}
