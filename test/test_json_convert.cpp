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
    lgraph::Schema mysch;
    auto ret_schema = ValueToJson(&mysch);
    UT_EXPECT_TRUE(ret_schema.as_bool());
    fma_common::HardwareInfo::CPURate cpurate;
    cpurate.selfCPURate = 12;
    auto ret_cpurate = ValueToJson(cpurate).as_object();
    UT_EXPECT_EQ(ret_cpurate.at(_TU("self")), 12);

    fma_common::HardwareInfo::DiskRate drate;
    drate.readRate = 222;
    auto ret_drate = ValueToJson(drate).as_object();
    UT_EXPECT_EQ(ret_drate.at(_TU("read")), 222);

    fma_common::HardwareInfo::MemoryInfo minfo;
    minfo.total = 14;
    auto ret_minfo = ValueToJson(minfo).as_object();
    UT_EXPECT_EQ(ret_minfo.at(_TU("total")), 14);
    fma_common::DiskInfo dinfo;
    size_t graph_used = 1;
    auto ret_dinfo = ValueToJson(dinfo, graph_used).as_object();
    UT_EXPECT_EQ(ret_dinfo.at(_TU("self")), 1);
    lgraph::AuditLog al;
    auto ret_al = ValueToJson(al).as_object();
    UT_EXPECT_FALSE(ret_al.at(_TU("success")).as_bool());
    std::vector<std::pair<std::string, std::string>> vec;
    std::string s1 = "first";
    std::string s2 = "second";
    vec.push_back(std::make_pair(s1, s2));
    auto ret_vec = ValueToJson(vec).as_array();
    UT_EXPECT_EQ(_TS(ret_vec[0][0].as_string()), "first");
    TaskTracker::TaskDesc taskdec;
    auto ret_taskdec = ValueToJson(taskdec).as_object();
    UT_EXPECT_EQ(_TS(ret_taskdec.at(_TU("task_id")).as_string()), "-1_-1");
    AclManager::FieldAccess s;
    ValueToJson(s);
}
