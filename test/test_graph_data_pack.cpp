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

#include <set>
#include "fma-common/logging.h"
#include "fma-common/configuration.h"
#include "fma-common/string_formatter.h"

#include "core/graph_data_pack.h"
#include "./random_port.h"
#include "gtest/gtest.h"
#include "./ut_utils.h"

using namespace fma_common;
using namespace lgraph;
using namespace lgraph::graph;

class TestGraphDataPack : public TuGraphTest {};

int CompareKeys(const Value& a, const Value& b) {
    const char* pa = a.Data();
    size_t sa = a.Size();
    const char* pb = b.Data();
    size_t sb = b.Size();
    int r = memcmp(pa, pb, std::min(sa, sb));
    return r < 0 ? -1 : r > 0 ? 1 : sa > sb ? 1 : sa < sb ? -1 : 0;
}

#define TestHeaderParse(lid, tid, vid, eid)                                       \
    do {                                                                          \
        std::string buf(EdgeValue::GetHeaderSizeRequired(lid, tid, vid, eid), 0); \
        const char* p = EdgeValue::SetHeader(&buf[0], lid, tid, vid, eid);        \
        UT_EXPECT_EQ(p - &buf[0], buf.size());                                    \
        LabelId l;                                                                \
        TemporalId t;                                                             \
        VertexId v;                                                               \
        EdgeId e;                                                                 \
        p = EdgeValue::ParseHeader(buf.data(), l, t, v, e);                       \
        UT_EXPECT_EQ(p - buf.data(), buf.size());                                 \
        UT_EXPECT_EQ(l, lid);                                                     \
        UT_EXPECT_EQ(t, tid);                                                     \
        UT_EXPECT_EQ(v, vid);                                                     \
        UT_EXPECT_EQ(e, eid);                                                     \
    } while (0)

TEST_F(TestGraphDataPack, GraphDataPack) {
    size_t n_labels = 10;
    size_t e_per_label = 10;
    size_t e_per_vertex = 10;
    int argc = _ut_argc;
    char** argv = _ut_argv;

    Configuration config;
    config.Add(n_labels, "labels", true).Comment("Number of labels");
    config.Add(e_per_label, "edge_per_label", true).Comment("Number of edges per label");
    config.Add(e_per_vertex, "edge_per_vertex", true).Comment("Number of edges per {src, vid}");
    config.ParseAndFinalize(argc, argv);
    {
        UT_LOG() << "Testing KeyPacker";
        Value v1 = KeyPacker::CreatePackedDataKey(1);
        Value v2 = KeyPacker::CreatePackedDataKey(2);
        UT_EXPECT_EQ(CompareKeys(v1, v2), -1);
        UT_EXPECT_EQ(CompareKeys(v2, v1), 1);
        UT_EXPECT_EQ(CompareKeys(v1, KeyPacker::CreatePackedDataKey(1)), 0);
        UT_EXPECT_EQ(KeyPacker::GetFirstVid(v1), 1);
        UT_EXPECT_TRUE(KeyPacker::GetNodeType(v1) == PackType::PACKED_DATA);
        UT_EXPECT_EQ(CompareKeys(KeyPacker::CreatePackedDataKey(0xff),
                                 KeyPacker::CreatePackedDataKey(0x100)),
                     -1);
        UT_EXPECT_EQ(CompareKeys(KeyPacker::CreatePackedDataKey(0xff00),
                                 KeyPacker::CreatePackedDataKey(0x20000)),
                     -1);
        UT_EXPECT_EQ(CompareKeys(KeyPacker::CreatePackedDataKey(0x3ff00),
                                 KeyPacker::CreatePackedDataKey(0x2ffff)),
                     1);

        v2 = KeyPacker::CreateVertexOnlyKey(1);
        UT_EXPECT_EQ(CompareKeys(v1, v2), -1);
        UT_EXPECT_EQ(CompareKeys(v2, v1), 1);
        UT_EXPECT_EQ(CompareKeys(v2, KeyPacker::CreateVertexOnlyKey(1)), 0);
        UT_EXPECT_EQ(CompareKeys(v2, KeyPacker::CreateVertexOnlyKey(8192)), -1);
        UT_EXPECT_EQ(KeyPacker::GetFirstVid(v2), 1);
        UT_EXPECT_TRUE(KeyPacker::GetNodeType(v2) == PackType::VERTEX_ONLY);

        Value v3 = KeyPacker::CreateOutEdgeKey(EdgeUid(1, 3, 2, 0, 4));
        UT_EXPECT_EQ(CompareKeys(v1, v3), -1);
        UT_EXPECT_EQ(CompareKeys(v2, v3), -1);
        UT_EXPECT_EQ(CompareKeys(v3, v2), 1);
        UT_EXPECT_EQ(CompareKeys(v3, KeyPacker::CreateOutEdgeKey(EdgeUid(1, 3, 2, 0, 4))), 0);
        UT_EXPECT_EQ(CompareKeys(v3, KeyPacker::CreateOutEdgeKey(EdgeUid(2, 0, 0, 0, 0))), -1);
        UT_EXPECT_EQ(CompareKeys(v3, KeyPacker::CreateOutEdgeKey(EdgeUid(1, 3, 2, 0, 4))), 0);
        UT_EXPECT_EQ(CompareKeys(KeyPacker::CreateOutEdgeKey(EdgeUid(2, 0, 0, 0, 0)),
                                 KeyPacker::CreateOutEdgeKey(EdgeUid(2, 0, 1, 0, 0))),
                     -1);
        UT_EXPECT_EQ(CompareKeys(KeyPacker::CreateOutEdgeKey(EdgeUid(2, 0, 1, 0, 0)),
                                 KeyPacker::CreateOutEdgeKey(EdgeUid(2, 0, 2, 0, 0))),
                     -1);
        UT_EXPECT_EQ(CompareKeys(KeyPacker::CreateOutEdgeKey(EdgeUid(2, 0, 2, 0, 0)),
                                 KeyPacker::CreateOutEdgeKey(EdgeUid(2, 1, 2, 0, 0))),
                     -1);
        UT_EXPECT_EQ(CompareKeys(KeyPacker::CreateOutEdgeKey(EdgeUid(2, 1, 2, 0, 0)),
                                 KeyPacker::CreateOutEdgeKey(EdgeUid(2, 2, 2, 0, 0))),
                     -1);
        UT_EXPECT_EQ(CompareKeys(KeyPacker::CreateOutEdgeKey(EdgeUid(2, 2, 2, 0, 0)),
                                 KeyPacker::CreateOutEdgeKey(EdgeUid(2, 2, 2, 0, 2))),
                     -1);
        UT_EXPECT_EQ(KeyPacker::GetFirstVid(v3), 1);
        UT_EXPECT_TRUE(KeyPacker::GetNodeType(v3) == PackType::OUT_EDGE);
        UT_EXPECT_EQ(KeyPacker::GetLabel(v3), 2);
        UT_EXPECT_EQ(KeyPacker::GetSecondVid(v3), 3);
        UT_EXPECT_EQ(KeyPacker::GetEid(v3), 4);
        UT_EXPECT_EQ(CompareKeys(KeyPacker::CreateOutEdgeKey(EdgeUid(2, 2, 256, 0, 0)),
                                 KeyPacker::CreateOutEdgeKey(EdgeUid(2, 2, 128, 0, 0))),
                     1);
        UT_EXPECT_EQ(CompareKeys(KeyPacker::CreateOutEdgeKey(EdgeUid(2, 2, 256, 0, 0x1fffffff)),
                                 KeyPacker::CreateOutEdgeKey(EdgeUid(2, 2, 256, 0, 0xfffff000))),
                     -1);

        Value v4 = KeyPacker::CreateInEdgeKey(EdgeUid(1, 3, 2, 0, 4));
        UT_EXPECT_EQ(CompareKeys(v1, v4), -1);
        UT_EXPECT_EQ(CompareKeys(v2, v4), -1);
        UT_EXPECT_EQ(CompareKeys(v3, v4), -1);
        UT_EXPECT_EQ(CompareKeys(v4, v2), 1);
        UT_EXPECT_EQ(CompareKeys(v4, KeyPacker::CreateInEdgeKey(EdgeUid(1, 3, 2, 0, 4))), 0);
        UT_EXPECT_EQ(CompareKeys(v4, KeyPacker::CreateInEdgeKey(EdgeUid(2, 0, 0, 0, 0))), -1);
        UT_EXPECT_EQ(CompareKeys(v4, KeyPacker::CreateInEdgeKey(EdgeUid(1, 3, 2, 0, 4))), 0);
        UT_EXPECT_EQ(CompareKeys(KeyPacker::CreateInEdgeKey(EdgeUid(2, 0, 0, 0, 0)),
                                 KeyPacker::CreateInEdgeKey(EdgeUid(2, 0, 1, 0, 0))),
                     -1);
        UT_EXPECT_EQ(CompareKeys(KeyPacker::CreateInEdgeKey(EdgeUid(2, 0, 1, 0, 0)),
                                 KeyPacker::CreateInEdgeKey(EdgeUid(2, 0, 2, 0, 0))),
                     -1);
        UT_EXPECT_EQ(CompareKeys(KeyPacker::CreateInEdgeKey(EdgeUid(2, 0, 2, 0, 0)),
                                 KeyPacker::CreateInEdgeKey(EdgeUid(2, 1, 2, 0, 0))),
                     -1);
        UT_EXPECT_EQ(CompareKeys(KeyPacker::CreateInEdgeKey(EdgeUid(2, 1, 2, 0, 0)),
                                 KeyPacker::CreateInEdgeKey(EdgeUid(2, 2, 2, 0, 0))),
                     -1);
        UT_EXPECT_EQ(CompareKeys(KeyPacker::CreateInEdgeKey(EdgeUid(2, 2, 2, 0, 0)),
                                 KeyPacker::CreateInEdgeKey(EdgeUid(2, 2, 2, 0, 2))),
                     -1);
        UT_EXPECT_EQ(KeyPacker::GetFirstVid(v4), 1);
        UT_EXPECT_TRUE(KeyPacker::GetNodeType(v4) == PackType::IN_EDGE);
        UT_EXPECT_EQ(KeyPacker::GetLabel(v4), 2);
        UT_EXPECT_EQ(KeyPacker::GetSecondVid(v4), 3);
        UT_EXPECT_EQ(KeyPacker::GetEid(v4), 4);
    }

    {
        UT_LOG() << "Testing variable length edge id packer";
        UT_EXPECT_EQ(EdgeValue::GetLidSizeRequired(0), 0);
        UT_EXPECT_EQ(EdgeValue::GetLidSizeRequired(1), 1);
        UT_EXPECT_EQ(EdgeValue::GetLidSizeRequired(127), 1);
        UT_EXPECT_EQ(EdgeValue::GetLidSizeRequired(255), 1);
        UT_EXPECT_EQ(EdgeValue::GetLidSizeRequired(256), 2);
        UT_EXPECT_EQ(EdgeValue::GetLidSizeRequired(65533), 2);

        UT_EXPECT_EQ(EdgeValue::GetVidSizeRequired(0), 0);
        UT_EXPECT_EQ(EdgeValue::GetVidSizeRequired((int64_t)1), 1);
        UT_EXPECT_EQ(EdgeValue::GetVidSizeRequired(((int64_t)1 << 8) - 1), 1);
        UT_EXPECT_EQ(EdgeValue::GetVidSizeRequired((int64_t)1 << 8), 2);
        UT_EXPECT_EQ(EdgeValue::GetVidSizeRequired(((int64_t)1 << 16) - 1), 2);
        UT_EXPECT_EQ(EdgeValue::GetVidSizeRequired((int64_t)1 << 16), 3);
        UT_EXPECT_EQ(EdgeValue::GetVidSizeRequired(((int64_t)1 << 24) - 1), 3);
        UT_EXPECT_EQ(EdgeValue::GetVidSizeRequired((int64_t)1 << 24), 4);
        UT_EXPECT_EQ(EdgeValue::GetVidSizeRequired(((int64_t)1 << 32) - 1), 4);
        UT_EXPECT_EQ(EdgeValue::GetVidSizeRequired((int64_t)1 << 32), 5);
        UT_EXPECT_EQ(EdgeValue::GetVidSizeRequired(((int64_t)1 << 40) - 1), 5);

        UT_EXPECT_EQ(EdgeValue::GetPidSizeRequired(0), 0);
        UT_EXPECT_EQ(EdgeValue::GetPidSizeRequired(1), 8);
        UT_EXPECT_EQ(EdgeValue::GetPidSizeRequired(((int64_t)1 << 40) - 1), 8);

        UT_EXPECT_EQ(EdgeValue::GetHeaderSizeRequired(0, 0, 0, 0), 1);
        UT_EXPECT_EQ(EdgeValue::GetHeaderSizeRequired(1, 0, 0, 0), 2);
        UT_EXPECT_EQ(EdgeValue::GetHeaderSizeRequired(1, 0, 1, 0), 3);
        UT_EXPECT_EQ(EdgeValue::GetHeaderSizeRequired(1, 0, 1, 1), 4);
        UT_EXPECT_EQ(EdgeValue::GetHeaderSizeRequired(1, 1, 1, 1), 12);
        UT_EXPECT_EQ(EdgeValue::GetHeaderSizeRequired(255, 0, 0, 0), 2);
        UT_EXPECT_EQ(EdgeValue::GetHeaderSizeRequired(65535, 0, 0, 0), 3);
        UT_EXPECT_EQ(EdgeValue::GetHeaderSizeRequired(65535, 1, ((int64_t)1 << 40) - 1,
                                                      ((int64_t)1 << 24) - 1),
                     19);

        TestHeaderParse(0, 0, 0, 0);
        TestHeaderParse(255, 0, 255, 255);
        TestHeaderParse(255, 1, 255, 255);
        TestHeaderParse(255, ((int64_t)1 << 60), 255, 255);
        TestHeaderParse(65535, 0, ((int64_t)1 << 40) - 1, ((int64_t)1 << 24) - 1);
    }

    {
        UT_LOG() << "Testing EdgeValue constructors";
        {
            // default constructor
            EdgeValue ev;
            UT_EXPECT_EQ(ev.GetEdgeCount(), 0);
            UT_EXPECT_EQ(ev.GetBuf().Size(), 1);
        }
        {
            // constructing from a list of edges
            std::vector<std::tuple<LabelId, TemporalId, VertexId, std::string>> edges;
            std::vector<EdgeValue> evs;
            for (size_t label = 0; label < n_labels; label++) {
                for (size_t vid = 0; vid < e_per_label; vid++) {
                    for (size_t eid = 0; eid < e_per_vertex; eid++) {
                        edges.emplace_back(
                            (LabelId)label, 0, (VertexId)vid,
                            fma_common::StringFormatter::Format("{}_{}_{}_{}", label, 0, vid, eid));
                    }
                }
            }
            LabelId last_lid = 0;
            VertexId last_vid = -1;
            TemporalId last_tid = -1;
            EdgeId last_eid = -1;
            for (auto it = edges.begin(); it != edges.end();) {
                decltype(it) next_it;
                EdgeValue ev(it, edges.end(), last_lid, last_tid, last_vid, last_eid, next_it);
                evs.emplace_back(std::move(ev));
                it = next_it;
            }
            // validate evs
            size_t nedges = 0;
            for (auto& ev : evs) nedges += ev.GetEdgeCount();
            UT_EXPECT_EQ(nedges, n_labels * e_per_label * e_per_vertex);
            size_t next_id = 0;
            for (auto& ev_ : evs) {
                EdgeValue ev(std::move(ev_));
                UT_EXPECT_EQ(ev_.GetEdgeCount(), 0);
                LabelId lid;
                VertexId vid;
                TemporalId tid;
                EdgeId eid;
                const char* prop;
                size_t psize;
                for (int i = 0; i < (int)ev.GetEdgeCount(); i++) {
                    ev.ParseNthEdge(i, lid, tid, vid, eid, prop, psize);
                    UT_EXPECT_EQ((lid * e_per_label + vid) * e_per_vertex + eid, next_id);
                    next_id++;
                    UT_EXPECT_EQ(
                        fma_common::StringFormatter::Format("{}_{}_{}_{}", lid, tid, vid, eid),
                        std::string(prop, prop + psize));
                }
            }
        }
    }

    {
        UT_LOG() << "Testing EdgeValue modify";
        {
            // testing upsert
            EdgeValue ev;
            Value prop;
            bool exist = false;
            size_t pos = 0;
            UT_EXPECT_EQ(ev.UpsertEdge(0, 0, 0, 0, prop, exist, pos), 1);
            UT_EXPECT_EQ(ev.UpsertEdge(0, 0, 1, 0, prop, exist, pos), 6);
            UT_EXPECT_EQ(ev.UpsertEdge(250, 0, 1, 0, prop, exist, pos), 7);
            UT_EXPECT_EQ(ev.UpsertEdge(250, 0, 253, 0, prop, exist, pos), 7);
            UT_EXPECT_EQ(ev.UpsertEdge(250, 0, 0x10000, 0, prop, exist, pos), 9);
            UT_EXPECT_EQ(ev.UpsertEdge(250, 0, 0x1000000, 0, prop, exist, pos), 10);
            UT_EXPECT_EQ(ev.UpsertEdge(250, 0, 0x1000000, 255, prop, exist, pos), 11);
            UT_EXPECT_EQ(ev.UpsertEdge(65535, 0, 0x1000000, 255, prop, exist, pos), 12);
            UT_EXPECT_EQ(ev.GetEdgeCount(), 8);
            UT_EXPECT_EQ(ev.GetBuf().Size(), 64);
        }

        EdgeValue ev;
        UT_EXPECT_EQ(ev.GetEdgeCount(), 0);
        size_t pos;
        // insert
        for (size_t i = 5; i < 10; i++) {
            bool exist;
            char eid = static_cast<char>(i);
            Value prop(1);
            memset(prop.Data(), eid, (int)prop.Size());
            int64_t r =
                ev.UpsertEdge(static_cast<lgraph::LabelId>(i), 0, 20, eid, prop, exist, pos);
            UT_EXPECT_GT(r, 0);
        }
        for (size_t i = 0; i < 5; i++) {
            bool exist;
            char eid = static_cast<char>(i);
            Value prop(1);
            memset(prop.Data(), eid, (int)prop.Size());
            int64_t r =
                ev.UpsertEdge(static_cast<lgraph::LabelId>(i), 0, 20, eid, prop, exist, pos);
            UT_EXPECT_GT(r, 0);
        }
        UT_EXPECT_EQ(ev.GetEdgeCount(), 10);
        for (size_t i = 0; i < ev.GetEdgeCount(); i++) {
            LabelId lid;
            VertexId vid;
            TemporalId tid;
            EdgeId eid;
            const char* prop;
            size_t psize;
            ev.ParseNthEdge(i, lid, tid, vid, eid, prop, psize);
            UT_EXPECT_EQ(lid, i);
            UT_EXPECT_EQ(tid, 0);
            UT_EXPECT_EQ(vid, 20);
            UT_EXPECT_EQ(eid, i);
            UT_EXPECT_EQ(psize, 1);
            UT_EXPECT_EQ((int)(*prop), i);
        }
        // update property
        for (size_t i = 0; i < 10; i++) {
            bool exist;
            char eid = static_cast<char>(i);
            Value prop(5 + eid);
            memset(prop.Data(), eid, (int)prop.Size());
            ev.UpsertEdge(static_cast<lgraph::LabelId>(i), 0, 20, eid, prop, exist, pos);
            UT_EXPECT_TRUE(exist);
        }
        for (size_t i = 0; i < ev.GetEdgeCount(); i++) {
            LabelId lid;
            VertexId vid;
            TemporalId tid;
            EdgeId eid;
            const char* prop;
            size_t psize;
            ev.ParseNthEdge(i, lid, tid, vid, eid, prop, psize);
            UT_EXPECT_EQ(lid, i);
            UT_EXPECT_EQ(tid, 0);
            UT_EXPECT_EQ(vid, 20);
            UT_EXPECT_EQ(eid, i);
            UT_EXPECT_EQ(psize, i + 5);
            for (size_t j = 0; j < psize; j++) {
                UT_EXPECT_EQ((int)prop[j], i);
            }
        }
        UT_EXPECT_EQ((int)ev.GetEdgeCount(), 10);
        std::set<int> changed;
        unsigned int seed = 0;
        srand(seed);
        for (size_t i = 0; i < 10; i++) {
            bool exist;
            char eid = myrand() % 10;
            Value prop(eid + 2);
            memset(prop.Data(), eid, (int)prop.Size());
            int64_t r = ev.UpsertEdge(eid, 0, 20, eid, prop, exist, pos);
            UT_EXPECT_TRUE(exist || r);
            if (changed.find(eid) != changed.end()) {
                UT_EXPECT_EQ(r, 0);
            } else {
                changed.insert(eid);
                UT_EXPECT_EQ(r, -3);
            }
        }
        UT_EXPECT_EQ((int)ev.GetEdgeCount(), 10);
        for (size_t i = 0; i < 30; i++) {
            bool exist;
            int vid = myrand() % 30;
            char eid = myrand() % 100;
            Value prop(eid + 2);
            memset(prop.Data(), eid, (int)prop.Size());
            int64_t r =
                ev.UpsertEdge(static_cast<lgraph::LabelId>(i), 0, vid, eid, prop, exist, pos);
            UT_EXPECT_TRUE(exist || r);
            if (exist) {
                UT_LOG() << "updating edge " << vid << "," << (int)eid;
            }
        }
        LabelId last_lid = 0;
        TemporalId last_tid = -1;
        int64_t last_vid = -1;
        int64_t last_eid = -1;
        LOG() << "Now we have " << (int)ev.GetEdgeCount() << " edges";
        std::set<std::tuple<LabelId, TemporalId, int64_t, int64_t>> edges;
        for (size_t i = 0; i < ev.GetEdgeCount(); i++) {
            LabelId lid;
            TemporalId tid;
            VertexId vid;
            EdgeId eid;
            const char* prop;
            size_t psize;
            ev.ParseNthEdge(i, lid, tid, vid, eid, prop, psize);
            edges.emplace(lid, tid, vid, eid);
            for (size_t j = 0; j < psize; j++) {
                UT_EXPECT_EQ((int)prop[j], eid);
            }
            UT_EXPECT_GE(lid, last_lid);
            if (lid == last_lid) {
                UT_EXPECT_GE(tid, last_tid);
                if (last_tid != tid) {
                    last_tid = tid;
                    last_vid = vid;
                    last_eid = eid;
                } else {
                    UT_EXPECT_GT(vid, last_vid);
                    if (vid != last_vid) {
                        last_vid = vid;
                        last_eid = eid;
                    } else {
                        UT_EXPECT_GT(eid, last_eid);
                    }
                }
            } else {
                last_lid = lid;
                last_tid = tid;
                last_vid = vid;
                last_eid = eid;
            }
        }

        const auto& first_edge = *edges.begin();
        UT_EXPECT_TRUE(ev.DeleteEdgeIfExist(std::get<0>(first_edge), std::get<1>(first_edge),
                                            std::get<2>(first_edge), std::get<3>(first_edge)));
        edges.erase(edges.begin());
        const auto& last_edge = *edges.rbegin();
        UT_EXPECT_TRUE(ev.DeleteEdgeIfExist(std::get<0>(last_edge), std::get<1>(last_edge),
                                            std::get<2>(last_edge), std::get<3>(last_edge)));
        edges.erase(--edges.end());
        int rand_num = myrand() % edges.size();
        auto it = edges.begin();
        for (size_t i = 0; i < rand_num; i++) it++;
        const auto& rand_edge = *it;
        UT_EXPECT_TRUE(ev.DeleteEdgeIfExist(std::get<0>(rand_edge), std::get<1>(rand_edge),
                                            std::get<2>(rand_edge), std::get<3>(rand_edge)));
        edges.erase(it);

        // test delete
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                int lid = myrand() % 30;
                int tid = 0;
                int vid = 20;
                int eid = lid;
                bool r = ev.DeleteEdgeIfExist(lid, tid, vid, eid);
                std::tuple<LabelId, TemporalId, int64_t, int64_t> p((LabelId)lid, tid, vid, eid);
                if (edges.find(p) != edges.end()) {
                    UT_LOG() << "Deleted (" << lid << ", " << tid << ", " << vid << ", " << eid
                             << ")";
                    UT_EXPECT_TRUE(r);
                    edges.erase(p);
                } else {
                    UT_EXPECT_TRUE(!r);
                }
            }
        }
        last_lid = 0;
        last_vid = -1;
        last_eid = -1;
        UT_LOG() << "After delete, we have " << (int)ev.GetEdgeCount() << " edges";
        std::set<std::tuple<LabelId, TemporalId, int64_t, int64_t>> edges_left;
        for (int i = 0; i < (int)ev.GetEdgeCount(); i++) {
            LabelId lid;
            VertexId vid;
            TemporalId tid;
            EdgeId eid;
            const char* prop;
            size_t psize;
            ev.ParseNthEdge(i, lid, tid, vid, eid, prop, psize);
            edges_left.emplace(lid, tid, vid, eid);
            for (size_t j = 0; j < psize; j++) {
                UT_EXPECT_EQ((int)prop[j], eid);
            }
            UT_EXPECT_GE(lid, last_lid);
            if (lid == last_lid) {
                UT_EXPECT_GE(vid, last_vid);
                if (last_vid != vid) {
                    last_vid = vid;
                    last_eid = eid;
                } else {
                    UT_EXPECT_GT(eid, last_eid);
                }
            } else {
                last_lid = lid;
                last_vid = vid;
                last_eid = eid;
            }
        }
        UT_EXPECT_EQ(edges.size(), edges_left.size());
        std::vector<std::tuple<LabelId, TemporalId, int64_t, int64_t>> diff(edges.size());
        auto dit = std::set_difference(edges.begin(), edges.end(), edges_left.begin(),
                                       edges_left.end(), diff.begin());
        UT_EXPECT_TRUE(dit == diff.begin());

        // test split
        EdgeValue lhs = ev.SplitEven();
        UT_LOG() << "OutEdgeValue is split into two parts of " << ev.GetEdgeCount() << "/"
                 << lhs.GetEdgeCount() << " edges and " << ev.GetBuf().Size() << "/"
                 << lhs.GetBuf().Size() << " bytes";
        last_lid = 0;
        last_vid = -1;
        last_eid = -1;
        for (int i = 0; i < lhs.GetEdgeCount(); i++) {
            LabelId lid;
            TemporalId tid;
            VertexId vid;
            EdgeId eid;
            const char* prop;
            size_t psize;
            lhs.ParseNthEdge(i, lid, tid, vid, eid, prop, psize);
            for (size_t j = 0; j < psize; j++) {
                UT_EXPECT_EQ((int)prop[j], eid);
            }
            UT_EXPECT_GE(lid, last_lid);
            if (lid == last_lid) {
                UT_EXPECT_GE(vid, last_vid);
                if (last_vid != vid) {
                    last_vid = vid;
                    last_eid = eid;
                } else {
                    UT_EXPECT_GT(eid, last_eid);
                }
            } else {
                last_lid = lid;
                last_vid = vid;
                last_eid = eid;
            }
        }
        for (int i = 0; i < ev.GetEdgeCount(); i++) {
            LabelId lid;
            VertexId vid;
            TemporalId tid;
            EdgeId eid;
            const char* prop;
            size_t psize;
            ev.ParseNthEdge(i, lid, tid, vid, eid, prop, psize);
            for (size_t j = 0; j < psize; j++) {
                UT_EXPECT_EQ((int)prop[j], eid);
            }
            UT_EXPECT_GE(lid, last_lid);
            if (lid == last_lid) {
                UT_EXPECT_GE(vid, last_vid);
                if (last_vid != vid) {
                    last_vid = vid;
                    last_eid = eid;
                } else {
                    UT_EXPECT_GT(eid, last_eid);
                }
            } else {
                last_lid = lid;
                last_vid = vid;
                last_eid = eid;
            }
        }
    }
}
