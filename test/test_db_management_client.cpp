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

#include "./ut_utils.h"
#include "./test_tools.h"
#include "fma-common/utils.h"
#include "gtest/gtest.h"
#include "server/db_management_client.h"

class TestDBManagementClient : public TuGraphTest {};

TEST_F(TestDBManagementClient, DBManagementClient) {
    using namespace lgraph;

    // set up cmd
    std::string cmd;
    int rt;

    // set up test veriables
    std::string exception_msg = "failed to connect to db management.";
    std::string host = "127.0.0.1";
    std::string port = "8888";
    std::int64_t start_time = 169413845845;
    std::string period = "IMMEDIATE";
    std::string name = "Khop_test";
    std::string type = "Python";
    std::string user = "user";
    std::int64_t create_time = 1694138458457;
    int job_id = 1;
    std::string status = "SUCCESS";
    std::int64_t runtime = 100;
    std::string result = "this is only a test of result";

    // init dbmanagement client
    try {
        DBManagementClient::GetInstance().InitChannel("localhost:5091");
    } catch(std::exception& e) {
        UT_EXPECT_EQ(1, 0);
    }

    // test exception handle
    try {
        DBManagementClient::GetInstance()
            .CreateJob(host, port, start_time, period, name, type, user, create_time);
        UT_EXPECT_EQ(1, 0);
    } catch(std::exception& e) {
        UT_EXPECT_EQ(e.what(), exception_msg);
    }

    try {
        DBManagementClient::GetInstance().UpdateJob(host, port, job_id, status, runtime, result);
        UT_EXPECT_EQ(1, 0);
    } catch(std::exception& e) {
        UT_EXPECT_EQ(e.what(), exception_msg);
    }

    try {
        DBManagementClient::GetInstance().ReadJob(host, port);
        UT_EXPECT_EQ(1, 0);
    } catch(std::exception& e) {
        UT_EXPECT_EQ(e.what(), exception_msg);
    }

    try {
        DBManagementClient::GetInstance().ReadJobResult(host, port, job_id);
        UT_EXPECT_EQ(1, 0);
    } catch(std::exception& e) {
        UT_EXPECT_EQ(e.what(), exception_msg);
    }

    try {
        DBManagementClient::GetInstance().DeleteJob(host, port, job_id);
        UT_EXPECT_EQ(1, 0);
    } catch(std::exception& e) {
        UT_EXPECT_EQ(e.what(), exception_msg);
    }

    // start db management
    cmd = "nohup java -jar tugraph-db-management-0.0.1-SNAPSHOT.jar --spring.profiles.active=ut "
          "> log.txt 2>&1 & echo $! > pidfile.txt";
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
    // sleep and wait db management start
    fma_common::SleepS(60);

    // test crud
    try {
        // create a new job
        job_id = DBManagementClient::GetInstance()
            .CreateJob(host, port, start_time, period, name, type, user, create_time);
        UT_EXPECT_EQ(1, job_id);
        // test jobid self increment
        job_id = DBManagementClient::GetInstance()
            .CreateJob(host, port, start_time, period, name, type, user, create_time);
        UT_EXPECT_EQ(2, job_id);

        // update this job
        DBManagementClient::GetInstance().UpdateJob(host, port, job_id, status, runtime, result);

        // read all jobs status
        std::vector<db_management::Job> jobs =
            DBManagementClient::GetInstance().ReadJob(host, port);
        UT_EXPECT_EQ(2, jobs.size());
        // read job by jobid
        db_management::Job job =
            DBManagementClient::GetInstance().ReadJob(host, port, job_id);
        UT_EXPECT_EQ(2, job.job_id());

        // read job result by id
        db_management::AlgoResult job_result =
            DBManagementClient::GetInstance().ReadJobResult(host, port, job_id);
        UT_EXPECT_EQ(result, job_result.result());

        // delete job
        DBManagementClient::GetInstance().DeleteJob(host, port, job_id);

        // test if deleted successfully
        jobs = DBManagementClient::GetInstance().ReadJob(host, port);
        UT_EXPECT_EQ(1, jobs.size());
    } catch(std::exception& e) {
        DEBUG_LOG(ERROR) << e.what();
        UT_EXPECT_EQ(1, 0);
    }

    // stop db management
    cmd = "kill -9 `cat pidfile.txt`";
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
}
