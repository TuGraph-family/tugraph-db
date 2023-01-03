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
#include "fma-common/configuration.h"
#include "gtest/gtest.h"
#include "import/import_data_file.h"
#include "./ut_utils.h"

using namespace fma_common;
using namespace lgraph;
using namespace import_v2;

class TestImportDataFile : public TuGraphTest {
 protected:
    void SetUp() {
        TuGraphTest::SetUp();
        n1 = 2222;
        n2 = 3333;
        nbucket = 27;
        bufsize = 1030;
    }

    int n1;
    int n2;
    int nbucket;
    int bufsize;
};

void WriteVertex(ImportDataFile& idf, const std::string& label, VidType start_id, VidType end_id,
                 size_t bucket_size, VidType nv, size_t prop_size) {
    std::vector<ImportDataFile::VertexDataWithVid> vs(nv);
    for (VidType i = 0; i < nv; i++) {
        vs[i].vid = i + start_id;
        vs[i].data = DenseString(std::string(prop_size, i % 128));
    }
    idf.StartWritingVertex(label, start_id);
    VidType psize = std::max<VidType>(1, nv / 3);
    for (VidType b = 0; b < nv; b += psize) {
        VidType e = std::min<VidType>(nv, b + psize);
        idf.WriteVertexes(
            std::vector<ImportDataFile::VertexDataWithVid>(vs.begin() + b, vs.begin() + e));
    }
    idf.EndWritingVertexAndSetBucketSize(label, end_id, bucket_size);
}

void WriteEdge(ImportDataFile& idf, size_t prop_size, const std::string& src_label,
               VidType src_start_id, VidType src_end_id, VidType src_nv,
               const std::string& dst_label, VidType dst_start_id, VidType dst_end_id,
               VidType dst_nv) {
    std::vector<ImportDataFile::EdgeData> outs;
    std::vector<ImportDataFile::EdgeData> ins;
    for (VidType i = 0; i < src_nv; i++) {
        VidType src = i + src_start_id;
        VidType dst = dst_start_id + (i + 33) % dst_nv;
        ImportDataFile::EdgeData ed;
        ed.vid1 = src;
        ed.lid = 0;
        ed.vid2 = dst;
        ed.prop = DenseString(std::string(prop_size, i % 128));
        outs.emplace_back(std::move(ed));
        ed.vid1 = dst;
        ed.vid2 = src;
        ins.emplace_back(std::move(ed));
    }
    idf.StartWritingEdge(src_label, dst_label);
    VidType psize = std::max<VidType>(1, (VidType)outs.size() / 3);
    for (VidType b = 0; b < outs.size(); b += psize) {
        VidType e = std::min<VidType>((VidType)outs.size(), b + psize);
        idf.WriteEdges(std::vector<ImportDataFile::EdgeData>(outs.begin() + b, outs.begin() + e),
                       std::vector<ImportDataFile::EdgeData>());
    }
    psize = std::max<VidType>(1, (VidType)ins.size() / 3);
    for (VidType b = 0; b < ins.size(); b += psize) {
        VidType e = std::min<VidType>((VidType)ins.size(), b + psize);
        idf.WriteEdges(std::vector<ImportDataFile::EdgeData>(),
                       std::vector<ImportDataFile::EdgeData>(ins.begin() + b, ins.begin() + e));
    }
    idf.EndWritingEdge();
}

void ReadVertex(ImportDataFile& idf, const std::string& label, VidType start_id, VidType end_id,
                std::vector<ImportDataFile::VertexDataWithVid>& all_v,
                std::vector<ImportDataFile::EdgeData>& all_outs,
                std::vector<ImportDataFile::EdgeData>& all_ins) {
    all_v.clear();
    all_outs.clear();
    all_ins.clear();

    idf.StartReading(label, start_id, end_id, 3, 3);
    ImportDataFile::BucketData bucket;
    while (idf.ReadBucket(bucket)) {
        all_v.insert(all_v.end(), bucket.vdata.begin(), bucket.vdata.end());
        all_outs.insert(all_outs.end(), bucket.oes.begin(), bucket.oes.end());
        all_ins.insert(all_ins.end(), bucket.ins.begin(), bucket.ins.end());
    }
    idf.EndReadingVertex();
}

TEST_F(TestImportDataFile, ImportDataFile) {
#ifdef _WIN32
    std::string dir = ".\\_test_tmp_";
#else
    std::string dir = "./_test_tmp_";
#endif

    VidType nv1 = 10;
    VidType nv2 = 20;
    size_t prop_size = 10;
    size_t n_bucket = 13;
    size_t buf_size = 1 << 10;

    nv1 = n1;
    nv2 = n2;
    n_bucket = nbucket;
    buf_size = bufsize;
    int argc = _ut_argc;
    char** argv = _ut_argv;
    Configuration config;
    config.Add(dir, "d,dir", true).Comment("Working directory");
    config.Add(nv1, "nv1", true).Comment("Number of vertex 1");
    config.Add(nv2, "nv2", true).Comment("Number of vertex 2");
    config.Add(prop_size, "p,prop_size", true).Comment("Property size");
    config.Add(n_bucket, "n_bucket", true).Comment("Bucket memory size in MB");
    config.Add(buf_size, "buf_size", true).Comment("Buffer size");
    config.ParseAndFinalize(argc, argv);
    {
        fma_common::file_system::MkDir(dir);
        ImportDataFile idf(dir, buf_size);

        UT_LOG() << "writing vertex 1";
        VidType start_id1 = 1024;
        VidType end_id1 = start_id1 + nv1 + 2048;
        WriteVertex(idf, "v1", start_id1, end_id1, n_bucket, nv1, prop_size);

        UT_LOG() << "writing vertex 2";
        VidType start_id2 = end_id1;
        VidType end_id2 = start_id2 + nv2 + 2048;
        WriteVertex(idf, "v2", start_id2, end_id2, n_bucket * 3, nv2, prop_size);

        UT_LOG() << "writing edges from v1 to v1";
        WriteEdge(idf, prop_size, "v1", start_id1, end_id1, nv1, "v1", start_id1, end_id1, nv1);

        UT_LOG() << "writing edges from v1 to v2";
        WriteEdge(idf, prop_size, "v1", start_id1, end_id1, nv1, "v2", start_id2, end_id2, nv2);

        UT_LOG() << "writing edges from v2 to v1";
        WriteEdge(idf, prop_size, "v2", start_id2, end_id2, nv2, "v1", start_id1, end_id1, nv1);

        UT_LOG() << "writing edges from v2 to v2";
        WriteEdge(idf, prop_size, "v2", start_id2, end_id2, nv2, "v2", start_id2, end_id2, nv2);

        UT_LOG() << "reading vertex 1";
        std::vector<ImportDataFile::VertexDataWithVid> all_v;
        std::vector<ImportDataFile::EdgeData> all_outs;
        std::vector<ImportDataFile::EdgeData> all_ins;
        ReadVertex(idf, "v1", start_id1, end_id1, all_v, all_outs, all_ins);
        UT_EXPECT_EQ(all_v.size(), nv1);
        for (VidType i = 0; i < all_v.size(); i++) {
            UT_EXPECT_EQ(all_v[i].vid, i + start_id1);
            auto& data = all_v[i].data;
            UT_EXPECT_EQ(data.size(), prop_size);
            UT_EXPECT_EQ(data.data()[data.size() - 1], i % 128);
        }
        UT_EXPECT_EQ(all_outs.size(), nv1 * 2);
        UT_EXPECT_EQ(all_ins.size(), nv1 + nv2);

        UT_LOG() << "reading vertex 2";
        ReadVertex(idf, "v2", start_id2, end_id2, all_v, all_outs, all_ins);
        UT_EXPECT_EQ(all_v.size(), nv2);
        for (VidType i = 0; i < all_v.size(); i++) {
            UT_EXPECT_EQ(all_v[i].vid, i + start_id2);
            auto& data = all_v[i].data;
            UT_EXPECT_EQ(data.size(), prop_size);
            UT_EXPECT_EQ(data.data()[data.size() - 1], i % 128);
        }
        UT_EXPECT_EQ(all_outs.size(), nv2 * 2);
        UT_EXPECT_EQ(all_ins.size(), nv1 + nv2);
    }
}
