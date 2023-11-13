/* Copyright (c) 2022 AntGroup. All Rights Reserved. */
#include "fma-common/logging.h"
#include "./ut_utils.h"
#include "core/global_config.h"

class TestGlobalHAConfig : public testing::Test {
 protected:
    void SetUp() override {
        conf.enable_audit_log = true;
        conf.enable_ha = true;
        conf.enable_rpc = true;
        conf.enable_ssl = true;
    }
    lgraph::GlobalConfig conf;
    void TearDown() override {}
};

TEST_F(TestGlobalHAConfig, GlobalHAConfig) {
    UT_LOG() << conf.FormatAsString();
    auto map = conf.ToFieldDataMap();
    UT_LOG() << "check GlobalHAConfig data";
    UT_EXPECT_EQ(conf.enable_audit_log, map["enable_audit_log"].AsBool());
    UT_EXPECT_EQ(conf.enable_ha, map["enable_ha"].AsBool());
    UT_EXPECT_EQ(conf.enable_rpc, map["enable_rpc"].AsBool());
    UT_EXPECT_EQ(conf.enable_ssl, map["enable_ssl"].AsBool());
    UT_EXPECT_EQ(conf.audit_log_expire, map["audit_log_expire"].AsInt64());
    UT_EXPECT_EQ(conf.bind_host, map["bind_host"].AsString());
    UT_EXPECT_EQ(conf.durable, map["durable"].AsBool());
    UT_EXPECT_EQ(conf.http_disable_auth, map["disable_auth"].AsBool());
    UT_EXPECT_EQ(conf.enable_backup_log, map["enable_backup_log"].AsBool());
    UT_EXPECT_EQ(conf.ha_heartbeat_interval_ms, map["ha_heartbeat_interval_ms"].AsInt32());
    UT_EXPECT_EQ(conf.ha_node_remove_ms, map["ha_node_remove_ms"].AsInt32());
    UT_EXPECT_EQ(conf.ha_node_offline_ms, map["ha_node_offline_ms"].AsInt32());
    UT_EXPECT_EQ(conf.txn_optimistic, map["optimistic_txn"].AsBool());

    UT_EXPECT_EQ(conf.subprocess_max_idle_seconds, map["subprocess_max_idle_seconds"].AsInt32());
    UT_EXPECT_EQ(conf.thread_limit, map["thread_limit"].AsInt32());
    UT_EXPECT_EQ(conf.verbose, map["verbose"].AsInt32());
    UT_EXPECT_EQ(conf.http_port, map["port"].AsInt32());
    UT_EXPECT_EQ(conf.rpc_port, map["rpc_port"].AsInt32());
}
