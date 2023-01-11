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

#include <deque>
#include <unordered_map>
#include <vector>

#include "fma-common/configuration.h"
#include "fma-common/logger.h"
#include "gtest/gtest.h"

#include "core/field_data_helper.h"
#include "import/vid_table.h"
#include "./random_port.h"
#include "./ut_utils.h"
using namespace lgraph;
using namespace lgraph::field_data_helper;
using namespace import_v2;

RandomSeed vid_seed(0);

class TestVidTable : public TuGraphTest {};

template <FieldType FT>
typename FieldType2StorageType<FT>::type MakeRandomKey(int i) {
    return static_cast<typename FieldType2StorageType<FT>::type>(rand_r(&vid_seed));
}

template <>
std::string MakeRandomKey<FieldType::STRING>(int i) {
    std::string ret;
    ret.reserve(i);
    for (int j = 0; j < i; j++) ret.push_back(rand_r(&vid_seed) % 128);
    return ret;
}

template <>
std::string MakeRandomKey<FieldType::BLOB>(int i) {
    std::string ret;
    ret.reserve(i);
    for (int j = 0; j < i; j++) ret.push_back(rand_r(&vid_seed) % 256);
    return ret;
}

template <FieldType FT>
FieldData MakeKey(int i) {
    return MakeFieldData<FT>(static_cast<typename FieldType2CType<FT>::type>(i));
}

template <>
FieldData MakeKey<FieldType::STRING>(int i) {
    return FieldData::String(std::string(i, 'a'));
}

template <>
FieldData MakeKey<FieldType::BLOB>(int i) {
    std::string b(i, 0);
    memset(&b[0], 129, i);
    return FieldData::Blob(std::move(b));
}

template <FieldType FT>
void TestSingleVidTable(int n_keys) {
    UT_LOG() << "Testing SingleVidTable with type " << FieldTypeName(FT);
    typename lgraph::import_v2::SingleVidTable<FT> vt;
    for (int i = 0; i < n_keys; i++) {
        UT_EXPECT_TRUE(vt.AddKey(MakeKey<FT>(i), i));
    }
    for (int i = 0; i < n_keys; i++) {
        UT_EXPECT_TRUE(!vt.AddKey(MakeKey<FT>(i), i));
    }
    for (int i = 0; i < n_keys; i++) {
        VidType vid;
        UT_EXPECT_TRUE(vt.GetVid(MakeKey<FT>(i), vid));
        UT_EXPECT_EQ(vid, i);
    }
}

template <FieldType FT, int N_BUCKETS = 13>
void TestGetBucketId(int n_keys) {
    UT_LOG() << "Tesing GetBucketId with type " << FieldTypeName(FT);
    std::vector<size_t> bucket_hits(N_BUCKETS, 0);
    for (int i = 0; i < n_keys; i++) {
        bucket_hits[GetBucketId(MakeRandomKey<FT>(i), N_BUCKETS)]++;
    }
    UT_LOG() << "key distributions: " << fma_common::ToString(bucket_hits);
    size_t min_hit = n_keys;
    size_t max_hit = 0;
    for (auto& h : bucket_hits) {
        min_hit = std::min<size_t>(min_hit, h);
        max_hit = std::max<size_t>(max_hit, h);
    }
    UT_EXPECT_LT(max_hit, min_hit * 2);
}

void TestAllVidTables() { AllVidTables tbl; }

TEST_F(TestVidTable, VidTable) {
    int n_bucket_key = 2000;
    int n_vid_key = 1000;
    int argc = _ut_argc;
    char** argv = _ut_argv;
    fma_common::Configuration config;
    config.Add(n_bucket_key, "bucket_key", true)
        .Comment("Number of keys to generate when testing GetBucketId");
    config.Add(n_vid_key, "vid_key", true)
        .Comment("Number of keys to generate when testing SingleVidTable");
    config.ParseAndFinalize(argc, argv);
    // GetBucketId
    UT_LOG() << "\nTesting GetBucketId";
    {
        srand(0);
        TestGetBucketId<FieldType::BOOL, 13>(n_bucket_key);
        TestGetBucketId<FieldType::INT8, 13>(n_bucket_key);
        TestGetBucketId<FieldType::INT16, 13>(n_bucket_key);
        TestGetBucketId<FieldType::INT32, 13>(n_bucket_key);
        TestGetBucketId<FieldType::INT64, 13>(n_bucket_key);
        TestGetBucketId<FieldType::FLOAT, 13>(n_bucket_key);
        TestGetBucketId<FieldType::DOUBLE, 13>(n_bucket_key);
        TestGetBucketId<FieldType::DATE, 13>(n_bucket_key);
        TestGetBucketId<FieldType::DATETIME, 13>(n_bucket_key);
        TestGetBucketId<FieldType::STRING, 13>(n_bucket_key);
        TestGetBucketId<FieldType::BLOB, 13>(n_bucket_key);
    }

    // AllVidTables
    UT_LOG() << "\nTesting AllVidTables";
    {
        AllVidTables tbl;
        std::string label1 = "string_v";
        std::string label2 = "int_v";
        std::string label3 = "int64";
        std::string label4 = "float";
        std::string label5 = "double";
        std::string label7 = "bool";
        std::string label8 = "int16";
        std::string label9 = "date";
        std::string label10 = "datetime";
        std::string label11 = "bin";

        tbl.StartVidTable(label1, FieldType::STRING, 1);
        tbl.SealVidTable(label1, 1000);
        tbl.StartVidTable(label2, FieldType::INT32, 1000);
        tbl.SealVidTable(label2, 2000);

        tbl.StartVidTable(label3, FieldType::INT64, 1000);
        tbl.StartVidTable(label4, FieldType::FLOAT, 1000);
        tbl.StartVidTable(label5, FieldType::DOUBLE, 1000);
        tbl.StartVidTable(label7, FieldType::BOOL, 1000);
        tbl.StartVidTable(label8, FieldType::INT16, 1000);
        tbl.StartVidTable(label9, FieldType::DATE, 1000);
        tbl.StartVidTable(label10, FieldType::DATETIME, 1000);
        tbl.StartVidTable(label11, FieldType::BLOB, 1000);

        auto vit_table = tbl.GetVidTable(label1);
        UT_EXPECT_TRUE(vit_table);
        UT_EXPECT_TRUE(vit_table->AddKey(FieldData("string_key"), 120));
        VidType vid = 0;
        UT_EXPECT_TRUE(vit_table->GetVid(FieldData("string_key"), vid));
        UT_EXPECT_EQ(vid, 120);
        UT_EXPECT_EQ(tbl.GetStartVid(label1), 1);
        UT_EXPECT_EQ(tbl.GetEndVid(label1), 1000);
        tbl.DeleteVidTable(label1);

        vit_table = tbl.GetVidTable(label2);
        auto pt = tbl.GetVidTable("not");
        UT_EXPECT_EQ(pt, nullptr);
        UT_EXPECT_TRUE(vit_table);
        UT_EXPECT_TRUE(vit_table->AddKey(FieldData::Int32(122), 122));
        UT_EXPECT_TRUE(vit_table->GetVid(FieldData::Int32(122), vid));
        UT_EXPECT_EQ(vid, 122);
        UT_EXPECT_EQ(tbl.GetStartVid(label2), 1000);
        UT_EXPECT_EQ(tbl.GetEndVid(label2), 2000);
    }

    // SingleVidTable
    UT_LOG() << "\nTesting SingleVidTable";
    {
        // correct behavior
        TestSingleVidTable<FieldType::BOOL>(2);
        TestSingleVidTable<FieldType::INT8>(127);
        TestSingleVidTable<FieldType::INT16>(std::min(n_vid_key, 32767));
        TestSingleVidTable<FieldType::INT32>(n_vid_key);
        TestSingleVidTable<FieldType::INT64>(n_vid_key);
        TestSingleVidTable<FieldType::FLOAT>(n_vid_key);
        TestSingleVidTable<FieldType::DOUBLE>(n_vid_key);
        TestSingleVidTable<FieldType::DATE>(n_vid_key);
        TestSingleVidTable<FieldType::DATETIME>(n_vid_key);
        TestSingleVidTable<FieldType::STRING>(n_vid_key);
        TestSingleVidTable<FieldType::BLOB>(n_vid_key);
    }
}
