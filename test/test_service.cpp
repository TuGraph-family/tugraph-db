/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include <unistd.h>
#include <cstdio>
#include <fstream>

#include "fma-common/configuration.h"
#include "fma-common/logging.h"
#include "gtest/gtest.h"

#include "server/service.h"
#include "./ut_utils.h"
using namespace std;
using namespace fma_common;
using namespace lgraph;

class TestService : public TuGraphTestWithParam<std::string> {};
//*/
class MyTestService : public Service {
    double interval_;
    std::string pid_file_;

 public:
    MyTestService(double interval, const std::string& service_name, const std::string& pid_path)
        : Service(service_name, pid_path), interval_(interval), pid_file_(pid_path) {}

    int Start() {
        remove(pid_file_.c_str());
        Service::Start();
        Service::Start();
        remove(pid_file_.c_str());
        return 0;
    }

    int Run() override {
        fma_common::Logger::Get().SetDevice(std::shared_ptr<fma_common::LogDevice>(
            new fma_common::FileLogDevice("./testservice.log")));
        int count_run = 0;
        pid_t pid = getpid();
        fma_common::SleepS(interval_);
        UT_LOG() << "Finish";
        return 0;
    }
};

TEST_P(TestService, Service) {
    std::string cmd = "run";
    double interval = 1;
    cmd = GetParam();
    int argc = _ut_argc;
    char** argv = _ut_argv;
    fma_common::Configuration config;
    config.Add(cmd, "c,cmd", true)
        .Comment("Command to execute")
        .SetPossibleValues({"start", "stop", "restart", "run"});
    config.Add(interval, "i,interval", true).Comment("Sleep interval of the service");
    config.ParseAndFinalize(argc, argv);

    MyTestService service(interval, "test_service", "./test_service.pid");
    MyTestService servicetes(interval, "test_service2", "test_service2.pid");

    UT_LOG() << "excute command: " << cmd;
    if (cmd == "run") {
        service.Run();
        servicetes.Run();
    } else if (cmd == "start") {
        servicetes.Start();
        service.Start();
    } else if (cmd == "restart") {
        servicetes.Restart();
        service.Restart();
    } else if (cmd == "stop") {
        servicetes.Stop();
        service.Stop();
    }
}

INSTANTIATE_TEST_CASE_P(TestService, TestService, testing::Values("start", "run", "restart"));
