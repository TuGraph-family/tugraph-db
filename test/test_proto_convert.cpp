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

//
// Created by hct on 18-12-29.
//

#include "fma-common/logger.h"
#include "fma-common/utils.h"
#include "server/proto_convert.h"
#include "gtest/gtest.h"
#include "./ut_utils.h"

class TestProtoConvert : public TuGraphTest {};

TEST_F(TestProtoConvert, ProtoConvert) {
    using namespace lgraph;
    using namespace fma_common;
    // test field data convert
    {
        ProtoFieldData pfd;
        {
            FieldData fd((int64_t)123);
            FieldDataConvert::FromLGraphT(fd, &pfd);
            UT_EXPECT_EQ(fd.AsInt64(), pfd.int64_());
            auto fd2 = FieldDataConvert::ToLGraphT(pfd);
            UT_EXPECT_EQ(fd.AsInt64(), fd2.AsInt64());
        }
        {
            FieldData fd((double)3.14);
            FieldDataConvert::FromLGraphT(fd, &pfd);
            UT_EXPECT_EQ(fd.AsDouble(), pfd.dp());
            auto fd2 = FieldDataConvert::ToLGraphT(pfd);
            UT_EXPECT_EQ(fd.AsDouble(), fd2.AsDouble());
        }
        {
            FieldData fd("hello");
            FieldDataConvert::FromLGraphT(fd, &pfd);
            UT_EXPECT_EQ(fd.AsString(), pfd.str());
            UT_EXPECT_TRUE(pfd.has_str());
            auto fd2 = FieldDataConvert::ToLGraphT(pfd);
            UT_EXPECT_EQ(fd.AsString(), fd2.AsString());
        }
        {
            FieldData fd;
            FieldDataConvert::FromLGraphT(fd, &pfd);
            UT_EXPECT_EQ(pfd.Data_case(), pfd.DATA_NOT_SET);
            auto fd2 = FieldDataConvert::ToLGraphT(pfd);
            UT_EXPECT_TRUE(fd2.is_null());
        }
        {
            std::vector<FieldData> fds;
            fds.emplace_back((int64_t)333);
            fds.emplace_back((double)277.1);
            fds.emplace_back("test convert string");
            fds.emplace_back();
            ::google::protobuf::RepeatedPtrField<ProtoFieldData> pfds, pfds2;
            FieldDataConvert::FromLGraphT(fds, &pfds);
            auto fds2 = FieldDataConvert::ToLGraphT(pfds);
            UT_EXPECT_TRUE(fds.size() == fds2.size());
            for (size_t i = 0; i < fds.size(); i++) UT_EXPECT_TRUE(fds[i] == fds2[i]);
            FieldDataConvert::FromLGraphT(std::move(fds2), &pfds2);
            UT_EXPECT_TRUE(fds.size() == pfds2.size());
            for (size_t i = 0; i < fds.size(); i++)
                UT_EXPECT_TRUE(fds[i] ==
                               FieldDataConvert::ToLGraphT(pfds2.Get(static_cast<int>(i))));
        }
    }

    // test field spec convert
    {
        std::vector<FieldSpec> fss;
        fss.emplace_back("int8", FieldType::INT8, true);
        fss.emplace_back("int8", FieldType::INT16, false);
        fss.emplace_back("int8", FieldType::INT32, true);
        fss.emplace_back("int8", FieldType::INT64, true);
        fss.emplace_back("int8", FieldType::FLOAT, false);
        fss.emplace_back("int8", FieldType::DOUBLE, false);
        fss.emplace_back("int8", FieldType::STRING, true);
        ::google::protobuf::RepeatedPtrField<ProtoFieldSpec> pfss, pfss2;
        FieldSpecConvert::FromLGraphT(fss, &pfss);
        UT_EXPECT_EQ(pfss.size(), fss.size());
        for (size_t i = 0; i < fss.size(); i++)
            UT_EXPECT_TRUE(fss[i] == FieldSpecConvert::ToLGraphT(pfss.Get(static_cast<int>(i))));
        auto fss2 = FieldSpecConvert::ToLGraphT(pfss);
        UT_EXPECT_EQ(fss.size(), fss2.size());
        for (size_t i = 0; i < fss.size(); i++) UT_EXPECT_TRUE(fss[i] == fss2[i]);
        FieldSpecConvert::FromLGraphT(std::move(fss2), &pfss2);
        UT_EXPECT_EQ(pfss2.size(), fss.size());
        for (size_t i = 0; i < fss.size(); i++)
            UT_EXPECT_TRUE(fss[i] == FieldSpecConvert::ToLGraphT(pfss2.Get(static_cast<int>(i))));
    }
}
