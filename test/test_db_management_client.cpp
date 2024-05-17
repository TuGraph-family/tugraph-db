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

#include <iostream>
#include <filesystem>

#include "fma-common/utils.h"
#include "gtest/gtest.h"
#include "server/db_management_client.h"

#include "./ut_utils.h"
#include "./test_tools.h"

class TestDBManagementClient : public TuGraphTest {};

TEST_F(TestDBManagementClient, DBManagementClient) {
    GTEST_SKIP() << "Temporarily disable TestDBManagementClient.DBManagementClient";
    using namespace lgraph;

    // init dbmanagement client
    const std::string host = "127.0.0.1";
    const int16_t port = 9091;
    try {
        DBManagementClient::GetInstance().Init(host, port, "localhost:5091");
    } catch (std::exception& e) {
        UT_EXPECT_EQ(1, 0);
    }

    const std::string uuid1 = "uuid1";
    const std::string uuid2 = "uuid2";
    const std::string name1 = "task1";
    const std::string name2 = "task2";
    const std::string status = "SUCCESS";
    const std::string period = "IMMEDIATE";
    const std::string name = "Khop_test";
    const std::string type = "Python";
    const std::string user = "user";
    const std::int64_t create_time = 1694138458457;
    const std::int64_t runtime = 100;
    const std::string result = "this is only a test of result";

    // test exception handle
    {
        UT_EXPECT_ANY_THROW(DBManagementClient::GetInstance().CreateJob(uuid1, name1, create_time,
                                                                        period, name, type, user));
        UT_EXPECT_ANY_THROW(
            DBManagementClient::GetInstance().UpdateJobStatus(uuid1, status, runtime, result));
        UT_EXPECT_ANY_THROW(DBManagementClient::GetInstance().GetJobStatus());
        UT_EXPECT_ANY_THROW(DBManagementClient::GetInstance().GetJobResult(uuid1));
        UT_EXPECT_ANY_THROW(DBManagementClient::GetInstance().DeleteJob(uuid1));
    }

    // start db management
    std::string cmd =
        "rm tugraph_db_management_*.db & nohup java -jar tugraph-db-management-0.0.1-SNAPSHOT.jar "
        " --spring.profiles.active=ut > log.txt 2>&1 & echo $! > pidfile.txt";
    int rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
    // sleep and wait db management start
    fma_common::SleepS(30);

    // test crud
    try {
        auto& client = DBManagementClient::GetInstance();
        // create a new job
        client.CreateJob(uuid1, name1, create_time, period, name, type, user);
        client.CreateJob(uuid2, name2, create_time, period, name, type, user);

        // update job status
        client.UpdateJobStatus(uuid1, status, runtime, result);

        // read job status
        std::vector<DbMgr::Job> jobs = client.GetJobStatus();
        UT_EXPECT_EQ(2, jobs.size());
        DbMgr::Job job = client.GetJobStatusById(uuid1);
        UT_EXPECT_EQ(status, job.status());

        // read job result by id
        DbMgr::AlgoResult job_result = client.GetJobResult(uuid1);
        UT_EXPECT_EQ(result, job_result.result());

        // delete job
        client.DeleteJob(uuid2);
        jobs = client.GetJobStatus();
        UT_EXPECT_EQ(1, jobs.size());
    } catch (std::exception& e) {
        LOG_ERROR() << e.what();
        UT_EXPECT_EQ(1, 0);
    }

    // stop db management
    cmd = "kill -9 `cat pidfile.txt`";
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
}
