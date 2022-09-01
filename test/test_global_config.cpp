/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "fma-common/logging.h"
#include "core/global_config.h"
#include "gtest/gtest.h"
#include "./ut_utils.h"

class TestGlobalConfig : public TuGraphTest {
 protected:
    void SetUp() {
        conf.enable_audit_log = true;
        conf.enable_rpc = true;
        conf.enable_ssl = true;
    }
    lgraph::GlobalConfig conf;
};

TEST_F(TestGlobalConfig, GlobalConfig) {
    UT_LOG() << conf.FormatAsString();
    auto map = conf.ToFieldDataMap();
    UT_LOG() << "check GlobalConfig data";
    UT_EXPECT_EQ(conf.enable_audit_log, map["enable_audit_log"].AsBool());
    UT_EXPECT_EQ(conf.enable_rpc, map["enable_rpc"].AsBool());
    UT_EXPECT_EQ(conf.enable_ssl, map["enable_ssl"].AsBool());
    UT_EXPECT_EQ(conf.audit_log_expire, map["audit_log_expire"].AsInt64());
    UT_EXPECT_EQ(conf.bind_host, map["bind_host"].AsString());
    UT_EXPECT_EQ(conf.durable, map["durable"].AsBool());
    UT_EXPECT_EQ(conf.http_disable_auth, map["disable_auth"].AsBool());
    UT_EXPECT_EQ(conf.enable_backup_log, map["enable_backup_log"].AsBool());
    UT_EXPECT_EQ(conf.txn_optimistic, map["optimistic_txn"].AsBool());

    UT_EXPECT_EQ(conf.subprocess_max_idle_seconds, map["subprocess_max_idle_seconds"].AsInt32());
    UT_EXPECT_EQ(conf.thread_limit, map["thread_limit"].AsInt32());
    UT_EXPECT_EQ(conf.verbose, map["verbose"].AsInt32());
    UT_EXPECT_EQ(conf.http_port, map["port"].AsInt32());
    UT_EXPECT_EQ(conf.rpc_port, map["rpc_port"].AsInt32());
}
