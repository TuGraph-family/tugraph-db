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

#include "fma-common/configuration.h"
#include "fma-common/logger.h"
#include "fma-common/utils.h"
#include "gtest/gtest.h"
#include "./ut_utils.h"

#include "core/audit_logger.h"
#include "core/data_type.h"

#include "./test_tools.h"

class TestAuditLogger : public TuGraphTest {};

TEST_F(TestAuditLogger, AuditLogger) {
    using namespace lgraph;
    {
        AutoCleanDir cleaner("./audit");
        AuditLogger &logger = AuditLogger::GetInstance();
        logger.Init("./audit", 1);
        logger.SetEnable(true);
        bool flag = logger.IsEnabled();
        UT_LOG() << "logger enable flag:" << flag;
        UT_EXPECT_EQ(flag, true);
        UT_LOG() << "log is empty";
        int64_t idx = logger.GetLogIdx();
        UT_EXPECT_EQ(idx, 0);
        int64_t beg, end;
        beg = lgraph_api::DateTime::LocalNow().MicroSecondsSinceEpoch();
        end = beg + 100000000;
        auto logs = logger.GetLog(time_t(), end);
        std::string str_beg, str_end;
        UT_EXPECT_EQ(logs.size(), 0);
        str_beg = lgraph_api::DateTime(beg).ToString();
        str_end = lgraph_api::DateTime(end).ToString();

        logs = logger.GetLog(str_beg, str_end);
        UT_EXPECT_EQ(logs.size(), 0);

        // write log
        int64_t log_id =
            logger.WriteLog(false, "user1", "graph1", LogApiType::Cypher, true, true, "000", 0);
        UT_EXPECT_EQ(log_id, 1);
        log_id =
            logger.WriteLog(true, "user1", "graph1", LogApiType::Cypher, true, true, "", log_id);
        UT_EXPECT_EQ(log_id, 1);
        fma_common::SleepS(1);
        // logs = logger.GetLog(time_t(), end);
        logs = logger.GetLog(beg, end);
        UT_EXPECT_EQ(logs.size(), 1);
        auto &log = logs[0];
        UT_EXPECT_EQ(log.user, "user1");
        UT_EXPECT_EQ(log.graph, "graph1");

        logs = logger.GetLog(str_beg, "");
        UT_EXPECT_EQ(logs.size(), 1);

        logs = logger.GetLog(str_beg, str_end);
        UT_EXPECT_EQ(logs.size(), 1);

        idx = logger.GetLogIdx();
        UT_EXPECT_EQ(idx, 1);
        UT_LOG() << "logger Idx:" << idx;
        std::string log_info(2048, 'a');
        int size = 4500;
        for (int i = 0; i < size; i++) {
            log_id = logger.WriteLog(false, "user1", "graph1", LogApiType::Cypher, true, true,
                                     log_info, 0);
            log_id = logger.WriteLog(true, "user1", "graph1", LogApiType::Cypher, true, true, "",
                                     log_id);
        }
        fma_common::SleepS(1);
        UT_LOG() << beg << " " << str_beg;
        logs = logger.GetLog(str_beg, "", "", size, false);
        UT_EXPECT_EQ(logs.size(), size);

        logger.Init("./audit", 1);
        logs = logger.GetLog(beg, end, "", size, false);
        UT_EXPECT_EQ(logs.size(), size);
    }

    UT_LOG() << "Testing BEG_AUDIT_LOG and AUDIT_LOG_SUCC macros";
    {
        AuditLogger &logger = AuditLogger::GetInstance();
        logger.Close();
        AutoCleanDir cleaner("./audit");
        logger.Init("./audit", 1);
        logger.SetEnable(true);
        BEG_AUDIT_LOG("user", "graph", lgraph::LogApiType::SingleApi, true, "SingleApi Task1");
        AUDIT_LOG_SUCC();
        BEG_AUDIT_LOG("user", "graph", lgraph::LogApiType::Cypher, true, "Cypher Task1");
        AUDIT_LOG_FAIL("parse error");
        // ascending order
        auto logs = logger.GetLog(0, lgraph_api::DateTime::LocalNow().MicroSecondsSinceEpoch() + 1,
        "", 100, false);
        UT_EXPECT_EQ(logs.size(), 2);
        UT_EXPECT_EQ(logs[0].content, "SingleApi Task1    Successful");
        UT_EXPECT_EQ(logs[0].success, true);
        UT_EXPECT_EQ(logs[1].content, "Cypher Task1    Failed: parse error");
        UT_EXPECT_EQ(logs[1].success, false);
        // limit to 1
        logs = logger.GetLog(0, lgraph_api::DateTime::LocalNow().MicroSecondsSinceEpoch() + 1,
        "", 1, false);
        UT_EXPECT_EQ(logs.size(), 1);
        UT_EXPECT_EQ(logs[0].content, "SingleApi Task1    Successful");
        UT_EXPECT_EQ(logs[0].success, true);
        // descending order
        logs = logger.GetLog(0, lgraph_api::DateTime::LocalNow().MicroSecondsSinceEpoch() + 1,
        "", 100, true);
        UT_EXPECT_EQ(logs.size(), 2);
        UT_EXPECT_EQ(logs[1].content, "SingleApi Task1    Successful");
        UT_EXPECT_EQ(logs[1].success, true);
        UT_EXPECT_EQ(logs[0].content, "Cypher Task1    Failed: parse error");
        UT_EXPECT_EQ(logs[0].success, false);
        // limit to 1
        logs = logger.GetLog(0, lgraph_api::DateTime::LocalNow().MicroSecondsSinceEpoch() + 1,
               "", 1, true);
        UT_EXPECT_EQ(logs.size(), 1);
        UT_EXPECT_EQ(logs[0].content, "Cypher Task1    Failed: parse error");
        UT_EXPECT_EQ(logs[0].success, false);
    }
}
