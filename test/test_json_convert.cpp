/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "fma-common/logger.h"
#include "gtest/gtest.h"
#include "restful/server/json_convert.h"
#include "./ut_utils.h"
class TestJsonConvert : public TuGraphTest {};

TEST_F(TestJsonConvert, JsonConvert) {
    using namespace lgraph;
    auto js = ValueToJson(std::map<std::string, int>({{"a", 1}}));
    UT_LOG() << _TS(js.serialize());
    auto level_store = {AccessLevel::NONE, AccessLevel::WRITE, AccessLevel::READ,
                        AccessLevel::WRITE};
    for (auto i : level_store) {
        AccessLevel src;
        auto js = ValueToJson(i);
        JsonToType<AccessLevel>(js, src);
    }
    lgraph::Schema* mysch = new lgraph::Schema;
    auto ret_schema = ValueToJson(mysch);
    UT_EXPECT_TRUE(ret_schema.as_bool());
    fma_common::HardwareInfo::CPURate cpurate;
    cpurate.selfCPURate = 12;
    auto ret_cpurate = ValueToJson(cpurate).as_object();
    UT_EXPECT_EQ(ret_cpurate.at("self"), 12);

    fma_common::HardwareInfo::DiskRate drate;
    drate.readRate = 222;
    auto ret_drate = ValueToJson(drate).as_object();
    UT_EXPECT_EQ(ret_drate.at("read"), 222);

    fma_common::HardwareInfo::MemoryInfo minfo;
    minfo.total = 14;
    auto ret_minfo = ValueToJson(minfo).as_object();
    UT_EXPECT_EQ(ret_minfo.at("total"), 14);
    fma_common::DiskInfo dinfo;
    size_t graph_used = 1;
    auto ret_dinfo = ValueToJson(dinfo, graph_used).as_object();
    UT_EXPECT_EQ(ret_dinfo.at("self"), 1);
    lgraph::AuditLog al;
    auto ret_al = ValueToJson(al).as_object();
    UT_EXPECT_FALSE(ret_al.at("success").as_bool());
    std::vector<std::pair<std::string, std::string>> vec;
    std::string s1 = "first";
    std::string s2 = "second";
    vec.push_back(std::make_pair(s1, s2));
    auto ret_vec = ValueToJson(vec).as_array();
    UT_EXPECT_EQ(ret_vec[0][0].as_string(), "first");
    TaskTracker::TaskDesc taskdec;
    auto ret_taskdec = ValueToJson(taskdec).as_object();
    UT_EXPECT_EQ(ret_taskdec.at("task_id").as_string(), "-1_-1");
    AclManager::FieldAccess s;
    ValueToJson(s);
}
