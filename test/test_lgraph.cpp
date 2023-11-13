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

#include <random>
#include "fma-common/logging.h"
#include "fma-common/configuration.h"
#include "gtest/gtest.h"
#include "core/lightning_graph.h"
#include "./ut_utils.h"

class TestLGraph : public TuGraphTest {};

lgraph::VertexId AddVertex(lgraph::Transaction& txn, const std::string& name,
                           const std::string& type) {
    std::vector<std::string> fnames = {"name", "type"};
    std::vector<std::string> values = {name, type};
    return txn.AddVertex((size_t)0, fnames, values);
}

bool UpdateVertex(lgraph::Transaction& txn, ::lgraph::VertexId vid, const std::string& name) {
    std::vector<std::string> fids = {"name"};
    std::vector<std::string> values = {name};
    return txn.SetVertexProperty(vid, fids, values);
}

bool UpdateEdge(lgraph::Transaction& txn, ::lgraph::VertexId src, ::lgraph::LabelId lid,
                ::lgraph::TemporalId tid, ::lgraph::VertexId dst, ::lgraph::EdgeId eid) {
    std::vector<std::string> fids = {"weight"};
    lgraph::FieldData w((float)((src + dst) * 10));
    std::vector<lgraph::FieldData> values = {w};
    return txn.SetEdgeProperty(lgraph_api::EdgeUid(src, dst, lid, tid, eid), fids, values);
}

lgraph::EdgeId AddEdge(lgraph::Transaction& txn, lgraph::VertexId src, lgraph::VertexId dst,
                       int8_t source, float weight) {
    std::vector<std::string> fids = {"source", "weight"};
    lgraph::FieldData s(source);
    lgraph::FieldData w(weight);
    std::vector<lgraph::FieldData> values = {s, w};
    return txn.AddEdge(src, dst, (size_t)0, fids, values).eid;
}

lgraph::EdgeId AddEdge2(lgraph::Transaction& txn, lgraph::VertexId src, lgraph::VertexId dst,
                        int64_t ts, float weight) {
    std::vector<std::string> fids = {"ts", "weight"};
    lgraph::FieldData t(ts);
    lgraph::FieldData w(weight);
    std::vector<lgraph::FieldData> values = {t, w};
    return txn.AddEdge(src, dst, (size_t)1, fids, values).eid;
}

bool UpsertEdge(lgraph::Transaction& txn, lgraph::VertexId src, lgraph::LabelId lid,
                lgraph::VertexId dst, int8_t source, float weight) {
    std::vector<std::string> fids = {"source", "weight"};
    lgraph::FieldData s(source);
    lgraph::FieldData w((float)((src + dst) * 10));
    std::vector<lgraph::FieldData> values = {s, w};
    lgraph::EdgeId eid = txn.UpsertEdge(src, dst, (size_t)lid, fids, values);
    return eid == 0;
}

bool UpsertEdge2(lgraph::Transaction& txn, lgraph::VertexId src, lgraph::LabelId lid,
                 lgraph::VertexId dst, int64_t ts, float weight) {
    std::vector<std::string> fids = {"ts", "weight"};
    lgraph::FieldData t(ts);
    lgraph::FieldData w(weight);
    std::vector<lgraph::FieldData> values = {t, w};
    lgraph::EdgeId eid = txn.UpsertEdge(src, dst, (size_t)lid, fids, values);
    return eid == 0;
}

std::string GetVName(int size, lgraph::VertexId vid) {
    std::string vname(std::max<int>(size, 0), 'v');
    vname.append(std::to_string(vid));
    return vname;
}

void DumpGraph(lgraph::Transaction& txn,
    bool dump_properties = true,
    bool dump_vertex_index = true,
    bool dump_edge_index = true) {
    // dump vertex and edges in the database
    for (auto vit = txn.GetVertexIterator(); vit.IsValid(); vit.Next()) {
        // write vertex id and vertex properties
        std::string vstr = fma_common::ToString(vit.GetId());
        if (dump_properties) {
            auto prop = txn.GetVertexFields(vit);
            vstr.append(": {");
            vstr.append(fma_common::ToString(prop));
            vstr.append("}");
        }
        UT_LOG() << vstr << ":";
        for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
            // write edge id and edge properties
            std::string estr = fma_common::ToString(eit.GetUid());
            if (dump_properties) {
                auto prop = txn.GetEdgeFields(eit);
                estr.append(": {");
                estr.append(fma_common::ToString(prop));
                estr.append("}");
            }
            UT_LOG() << "\t" << estr;
        }
    }
    // dump indexes
    if (dump_vertex_index) {
        for (auto index : txn.ListVertexIndexes()) {
            UT_LOG() << "vertex index: (" << index.label << ":" << index.field << ")";
            lgraph::FieldData last_key;
            std::string istr;
            for (auto it = txn.GetVertexIndexIterator(index.label, index.field);
                it.IsValid(); it.Next()) {
                if (it.GetKeyData() != last_key) {
                    istr.append("\n\t").append(it.GetKeyData().ToString()).append(":");
                    last_key = it.GetKeyData();
                }
                istr.append(" ").append(fma_common::ToString(it.GetVid()));
            }
            UT_LOG() << istr;
        }
    }
    if (dump_edge_index) {
        for (auto index : txn.ListEdgeIndexes()) {
            UT_LOG() << "edge index: (" << index.label << ":" << index.field << ")";
            lgraph::FieldData last_key;
            std::string istr;
            for (auto it = txn.GetEdgeIndexIterator(index.label, index.field); it.IsValid();
                 it.Next()) {
                if (it.GetKeyData() != last_key) {
                    istr.append("\n\t").append(it.GetKeyData().ToString()).append(":");
                    last_key = it.GetKeyData();
                }
                istr.append(" ").append(fma_common::ToString(it.GetUid()));
            }
            UT_LOG() << istr;
        }
    }
}

TEST_F(TestLGraph, LGraph) {
    using namespace lgraph;
    using namespace fma_common;

    AutoCleanDir _("./testdb");
    UT_LOG() << "Simple tests";
    {
        DBConfig config;
        config.dir = "./testdb";
        LightningGraph db(config);
        db.DropAllData();
        std::vector<FieldSpec> v_fds = {{"name", FieldType::STRING, false},
                                        {"type", FieldType::INT8, false}};
        std::vector<FieldSpec> e1_fds = {{"source", FieldType::INT8, false},
                                         {"weight", FieldType::FLOAT, false}};
        UT_EXPECT_TRUE(db.AddLabel("v", v_fds, true, VertexOptions("name")));
        UT_EXPECT_TRUE(db.AddLabel("e1", e1_fds, false, EdgeOptions()));
        // ASSERT(db.BlockingAddIndex("v", "name", false));

        Transaction txn = db.CreateWriteTxn();
        AddVertex(txn, "v1", "1");
        VertexId vid2_ = AddVertex(txn, "v2", "2");
        {
            VertexIndexIterator iit = txn.GetVertexIndexIterator("v", "name", "v2", "v2");
            UT_EXPECT_TRUE(iit.IsValid());
            UT_EXPECT_EQ(iit.GetVid(), 1);
            UT_EXPECT_TRUE(txn.DeleteVertex(vid2_));
            VertexIndexIterator iit2 = txn.GetVertexIndexIterator("v", "name", "v2", "v2");
            UT_EXPECT_TRUE(!iit2.IsValid());
        }
        txn.Commit();
        db.DropAllData();
        // db
        size_t msize, next_vid;
        db.GetDBStat(msize, next_vid);
        UT_EXPECT_EQ(next_vid, 0);
        size_t num = db.GetNumVertices();
        UT_EXPECT_EQ(num, 0);
    }

    UT_LOG() << "Label and field name checks";
    {
        DBConfig config;
        config.dir = "./testdb";
        LightningGraph db(config);
        db.DropAllData();
        // not ok
        UT_EXPECT_ANY_THROW(db.AddLabel(
            "_102834_basd", std::vector<FieldSpec>(), true, VertexOptions()));
        // ok
        std::vector<FieldSpec> fields;
        fields.emplace_back("a0", FieldType::STRING, false);
        for (size_t i = 1; i < 1024; i++)
            fields.emplace_back("a" + std::to_string(i), FieldType::STRING, true);
        UT_EXPECT_TRUE(db.AddLabel("ok", fields, true, VertexOptions("a0")));
        // label already exists
        UT_EXPECT_TRUE(!db.AddLabel("ok", fields, true, VertexOptions("a0")));
        // too many fields
        fields.emplace_back("aa" + std::to_string(1024), FieldType::STRING, true);
        UT_EXPECT_ANY_THROW(db.AddLabel("too_many_fields", fields, true, VertexOptions("a0")));
        // cannot start with digits
        UT_EXPECT_ANY_THROW(db.AddLabel("102834_basd",
                                        std::vector<FieldSpec>({{"id", FieldType::STRING, false}}),
                                        true, VertexOptions("id")));
        // empty label name
        UT_EXPECT_ANY_THROW(db.AddLabel(
            "", std::vector<FieldSpec>({{"id", FieldType::STRING, false}}),
            true, VertexOptions("id")));
        // label name
        db.AddLabel(std::string(255, 'e'),
                                        std::vector<FieldSpec>({{"id", FieldType::STRING, false}}),
                                        true, VertexOptions("id"));
        // long label name
        UT_EXPECT_ANY_THROW(db.AddLabel(std::string(256, 'a'),
                                        std::vector<FieldSpec>({{"id", FieldType::STRING, false}}),
                                        true, VertexOptions("id")));
        // strange label names
        for (auto& str : std::vector<std::string>{"#", "!", "+", "-", "*", "/"})
            UT_EXPECT_ANY_THROW(db.AddLabel(
                str, std::vector<FieldSpec>({{"id", FieldType::STRING, false}}),
                true, VertexOptions("id")));
        // duplicate field name
        UT_EXPECT_ANY_THROW(
            db.AddLabel("duplicate_name",
                        std::vector<FieldSpec>(2, FieldSpec("name", FieldType::STRING, false)),
                        true, VertexOptions("name")));
        // too many fields
        UT_EXPECT_ANY_THROW(db.AddLabel("too_many_fields",
                                        std::vector<FieldSpec>(1025, FieldSpec()),
                                            true, VertexOptions()));
        // empty field name
        UT_EXPECT_ANY_THROW(db.AddLabel(
            "empty_field_name", std::vector<FieldSpec>{FieldSpec("", FieldType::STRING, false)},
            true, VertexOptions()));
        // field name beginning with number
        UT_EXPECT_ANY_THROW(
            db.AddLabel("field_name_start_with_digit",
                        std::vector<FieldSpec>{FieldSpec("10234_kkk", FieldType::STRING, false)},
                        true, VertexOptions("10234_kkk")));
        // field name ok
        db.AddLabel(
            "field_name_not_long",
            std::vector<FieldSpec>{FieldSpec(std::string(255, 'a'), FieldType::STRING, false)},
            true, VertexOptions(std::string(255, 'a')));
        // field name too long
        UT_EXPECT_ANY_THROW(db.AddLabel(
            "field_name_too_long",
            std::vector<FieldSpec>{FieldSpec(std::string(256, 'a'), FieldType::STRING, false)},
            true, VertexOptions(std::string(256, 'a'))));
        // invalid character in field name
        for (auto& str : std::vector<std::string>{"#", "!", "+", "-", "*", "/"})
            UT_EXPECT_ANY_THROW(db.AddLabel(
                "invalid_character_in_field_name",
                std::vector<FieldSpec>{FieldSpec(str, FieldType::STRING, false)},
                true, VertexOptions(str)));
#ifndef _MSC_VER
        // vs encodes Chinese character with non-unicode, which causes trouble
        // ok with Chinese label name
        UT_EXPECT_TRUE(db.AddLabel(
            "field_name_with_chinese_character",
            std::vector<FieldSpec>{FieldSpec("中文", FieldType::STRING, false)},
            true, VertexOptions("中文")));
#endif
        // ok
        UT_EXPECT_TRUE(
            db.AddLabel("field_name_with_underscore",
                        std::vector<FieldSpec>{FieldSpec("_good_", FieldType::STRING, false)}, true,
                        VertexOptions("_good_")));
        // ok with value <= MAX_PROP_SIZE
#if 0
        {
            auto txn = db.CreateWriteTxn();
            UT_EXPECT_ANY_THROW(txn.AddVertex(
                std::string("field_name_with_underscore"), std::vector<std::string>{"_good_"},
                std::vector<std::string>{std::string((size_t)1 << 31, 'a')}));
            int64_t vid = txn.AddVertex(std::string("field_name_with_underscore"),
                                        std::vector<std::string>{"_good_"},
                                        std::vector<std::string>{std::string(1 << 30, 'a')});
            txn.Commit();
            txn = db.CreateReadTxn();
            auto it = txn.GetVertexIterator(vid);
            UT_EXPECT_TRUE(it.IsValid());
            UT_EXPECT_EQ(txn.GetVertexField(it, std::string("_good_")).AsString(),
                         std::string(1 << 30, 'a'));
        }
#endif
        {
            // create a label with more than one fields
            UT_EXPECT_TRUE(
                db.AddLabel("multiple_fields",
                            std::vector<FieldSpec>{FieldSpec("int16", FieldType::INT16, false),
                                                   FieldSpec("int32", FieldType::INT32, true),
                                                   FieldSpec("string", FieldType::STRING, true),
                                                   FieldSpec("string2", FieldType::STRING, false)},
                            true, VertexOptions("int16")));
            auto txn = db.CreateWriteTxn();
            // value too large
#if 0
            UT_EXPECT_ANY_THROW(
                txn.AddVertex(std::string("multiple_fields"),
                              std::vector<std::string>{"string", "string2", "int16", "int32"},
                              std::vector<std::string>{std::string(1 << 30, 'a'),
                                                       std::string(1 << 30, 'b'), "16", ""}));
            size_t mem = fma_common::HardwareInfo::GetAvailableMemory();
            if (mem >= (size_t)8 << 30) {
                UT_LOG() << "Free memory " << mem / 1024 / 1024 << "MB, running big value test";
                // this might fail due to OOM on CircleCI
                int64_t vid =
                    txn.AddVertex(std::string("multiple_fields"),
                                  std::vector<std::string>{"string2", "string", "int16", "int32"},
                                  std::vector<std::string>{
                                      std::string(((size_t)1 << 31) - 2048, 'a'), "", "16", ""});
                auto it = txn.GetVertexIterator(vid);
                UT_EXPECT_TRUE(it.IsValid());
                UT_EXPECT_EQ(txn.GetVertexField(it, std::string("string2")).AsString(),
                             std::string(((size_t)1 << 31) - 2048, 'a'));
            } else {
                UT_LOG() << "Free memory " << mem / 1024 / 1024 << "MB, skipping big value test";
            }
#endif
        }
    }

    UT_LOG() << "Performance tests";
    srand(0);
    size_t nv = 10;
    size_t epv = 1;
    float epv_sigma = 3;
    float vnsize = 20;
    float vnsize_sigma = 10;
    int argc = _ut_argc;
    char** argv = _ut_argv;
    Configuration config;
    config.Add(nv, "nv,v", true);
    config.Add(epv, "epv,e", true);
    config.Add(epv_sigma, "esigma", true);
    config.Add(vnsize, "vsize", true);
    config.Add(vnsize_sigma, "vsigma", true);
    config.ParseAndFinalize(argc, argv);
    DBConfig conf;
    conf.dir = "./testdb";
    LightningGraph db(conf);
    db.DropAllData();
    std::vector<FieldSpec> v_fds = {{"name", FieldType::STRING, false},
                                    {"type", FieldType::INT8, false}};
    std::vector<FieldSpec> e_fds = {{"source", FieldType::INT8, false},
                                    {"weight", FieldType::FLOAT, false}};
    UT_EXPECT_TRUE(db.AddLabel("v", v_fds, true, VertexOptions("name")));
    UT_EXPECT_TRUE(db.AddLabel("e", e_fds, false, EdgeOptions()));
    UT_EXPECT_TRUE(db._AddEmptyIndex("v", "type", lgraph::IndexType::NonuniqueIndex, true));
    UT_EXPECT_TRUE(db._AddEmptyIndex("e", "weight", lgraph::IndexType::NonuniqueIndex, false));
    db.DropAllIndex();
    UT_EXPECT_TRUE(db._AddEmptyIndex("v", "name", lgraph::IndexType::GlobalUniqueIndex, true));
    UT_EXPECT_TRUE(db._AddEmptyIndex("v", "type", lgraph::IndexType::NonuniqueIndex, true));
    UT_EXPECT_TRUE(db._AddEmptyIndex("e", "weight", lgraph::IndexType::NonuniqueIndex, false));
    // UT_EXPECT_TRUE(db.BlockingAddIndex("v", "name", true));
    db.DropAllVertex();
    while (!db.IsIndexed("v", "name", true)) fma_common::SleepUs(100);

    {  // test exception
        UT_EXPECT_ANY_THROW(db.AddLabel("_@lgraph@_", v_fds, true, VertexOptions("name")));
        std::vector<FieldSpec> v_fds_error = {{"_@lgraph@_", FieldType::STRING, false},
                                              {"type", FieldType::INT8, false}};
        UT_EXPECT_ANY_THROW(db.AddLabel("test", v_fds_error, true, VertexOptions("_@lgraph@_")));
    }

    std::map<VertexId, std::map<VertexId, std::set<EdgeId>>> all_edges;
    std::map<VertexId, int> vn_size;
    std::mt19937 rgen(0);
    std::uniform_int_distribution<VertexId> vid_dist;
    std::normal_distribution<float> epv_dist((float)epv, epv_sigma);
    std::normal_distribution<float> vname_size_dist(vnsize, vnsize_sigma);
    // add vertexes
    try {
        UT_LOG() << "Adding vertexes";
        auto txn = db.CreateWriteTxn();
        for (size_t i = 0; i < nv; i++) {
            // limit vname size to avoid index error: index key must be less than 508 bytes
            int vname_size = std::min<int>(460, (int)(vname_size_dist(rgen)));
            vname_size = std::max<int>(vname_size, 1);
            vn_size[i] = vname_size;
            UT_EXPECT_EQ(AddVertex(txn, GetVName(vname_size, i), std::to_string(i % 128)), i);
        }
        txn.Commit();
        UT_LOG() << "Vertex added: " << nv;
    } catch (std::exception& e) {
        ERR() << "Error occurred: " << e.what();
        UT_EXPECT_TRUE(false);
    }

    // check vertex data integrity
    try {
        UT_LOG() << "Checking vertex data";
        auto txn = db.CreateReadTxn();
        std::vector<std::string> fields = {"name", "type"};
        for (auto& kv : vn_size) {
            VertexId vid = kv.first;
            int vnsize = kv.second;
            auto vit = txn.GetVertexIterator(kv.first);
            UT_EXPECT_TRUE(vit.IsValid());
            UT_EXPECT_EQ(vit.GetId(), vid);
            auto fds = txn.GetVertexFields(vit, fields);
            UT_EXPECT_TRUE(!fds[0].is_null());
            UT_EXPECT_EQ(fds[0].type, FieldType::STRING);
            UT_EXPECT_EQ(fds[0].AsString(), GetVName(vnsize, vid));
            UT_EXPECT_TRUE(!fds[1].is_null());
            UT_EXPECT_EQ(fds[1].AsInt8(), vid % 128);
        }
        txn.Abort();
    } catch (std::exception& e) {
        ERR() << "Error occurred: " << e.what();
        UT_EXPECT_TRUE(false);
    }

    // add edges
    try {
        UT_LOG() << "Adding edges";
        size_t nedges = 0;
        for (size_t i = 0; i < nv; i++) {
            auto txn = db.CreateWriteTxn();
            int ne = (int)epv_dist(rgen);
            for (int j = 0; j < ne; j++) {
                VertexId src, dst;
                src = vid_dist(rgen);
                dst = vid_dist(rgen);
                if (src % 100 >= 20) {
                    // normal add
                    src %= nv;
                    dst %= nv;
                } else if (src % 100 >= 10) {
                    // src/dst does not exist
                    src %= 6734855;
                    dst %= 678956;
                } else {
                    // vertex id too large
                }
                bool success = false;
                try {
                    if (src == 0) {
                        UT_LOG() << "adding " << src << "->" << dst;
                    }
                    EdgeId eid = AddEdge(txn, src, dst, (src + dst) % 128, (float)(src + dst));
                    all_edges[src][dst].insert(eid);
                    success = true;
                    nedges++;
                    auto eit2 = txn.GetOutEdgeIterator(EdgeUid(src, dst, 0, 0, eid), false);
                    auto eit = std::move(eit2);
                    UT_EXPECT_TRUE(eit.IsValid());
                    auto data = txn.GetEdgeFields(eit, std::vector<size_t>{0, 1});
                    UT_EXPECT_TRUE(!data[0].is_null());
                    UT_EXPECT_EQ(data[0].AsInt8(), (src + dst) % 128);
                    UT_EXPECT_TRUE(!data[1].is_null());
                    UT_EXPECT_EQ(data[1].AsFloat(), (double)(src + dst));
                } catch (std::exception&) {
                    // UT_LOG() << "expected: failed to add " << src << "->" << dst
                    //    << ": " << e.what();
                }
                if (vn_size.find(src) != vn_size.end() && vn_size.find(dst) != vn_size.end()) {
                    UT_EXPECT_TRUE(success);
                } else {
                    UT_EXPECT_TRUE(!success);
                }
            }
            txn.Commit();
        }
        UT_LOG() << "Edges added: " << nedges;
    } catch (std::exception& e) {
        ERR() << "Error occurred: " << e.what();
        UT_EXPECT_TRUE(false);
    }

    // check edges
    try {
        UT_LOG() << "Checking edge data";
        auto txn = db.CreateReadTxn();
        std::vector<std::string> fields = {"source", "weight"};
        for (auto& kv : all_edges) {
            VertexId src = kv.first;
            for (auto& dst_eid : kv.second) {
                VertexId dst = dst_eid.first;
                for (EdgeId eid : dst_eid.second) {
                    auto eit = txn.GetOutEdgeIterator(EdgeUid(src, dst, 0, 0, eid), false);
                    UT_EXPECT_TRUE(eit.IsValid());
                    auto data = txn.GetEdgeFields(eit, fields);
                    UT_EXPECT_TRUE(!data[0].is_null());
                    UT_EXPECT_EQ(data[0].AsInt8(), (src + dst) % 128);
                    UT_EXPECT_TRUE(!data[1].is_null());
                    UT_EXPECT_EQ(data[1].AsFloat(), (double)(src + dst));
                }
            }
        }
        DumpGraph(txn);
    } catch (std::exception& e) {
        ERR() << "Error occurred: " << e.what();
        UT_EXPECT_TRUE(false);
    }

    // Test AddEdge2
    try {
        LOG() << "AddEdge with primary";
        std::vector<FieldSpec> e2_fds = {{"ts", FieldType::INT64, false},
                                         {"weight", FieldType::FLOAT, false}};
        EdgeOptions options;
        options.temporal_field = "ts";
        options.temporal_field_order = TemporalFieldOrder::ASC;
        ASSERT(db.AddLabel("e2", e2_fds, false, options));
        auto txn = db.CreateWriteTxn();
        VertexId src = AddVertex(txn, "v10", "10");
        VertexId dst1 = AddVertex(txn, "v11", "11");
        VertexId dst2 = AddVertex(txn, "v12", "12");
        VertexId dst3 = AddVertex(txn, "v13", "13");
        // FMA_LOG() << txn.GetAllLabels(false).size();
        AddEdge2(txn, src, dst2, 3, 0.3);
        AddEdge2(txn, src, dst3, 1, 0.1);
        AddEdge2(txn, src, dst3, 1, 0.1);
        AddEdge2(txn, src, dst1, 2, 0.2);

        {
            auto eit = txn.GetOutEdgeIterator(EdgeUid(src, 0, 1, 0, 0), true);
            UT_ASSERT(eit.IsValid() && eit.GetTemporalId() == 1);
            eit.Next();
            UT_ASSERT(eit.IsValid() && eit.GetTemporalId() == 1);
            eit.Next();
            UT_ASSERT(eit.IsValid() && eit.GetTemporalId() == 2);
            eit.Next();
            UT_ASSERT(eit.IsValid() && eit.GetTemporalId() == 3);
        }

        {
            UT_ASSERT(UpsertEdge2(txn, src, 1, dst3, 2, 2.2) == false);
            UT_ASSERT(UpsertEdge2(txn, src, 1, dst3, 1, 1.1) == true);
            {
#if 0
                auto eit = txn.GetOutEdgeIterator(EdgeUid(src, 0, 1, 0, 0), true);
                while (eit.IsValid()) {
                    FMA_LOG() << "<<<< " << eit.GetUid().ToString()
                              << txn.GetEdgeField(eit, size_t(1)).AsFloat();
                    eit.Next();
#endif
            }
            auto eit2 = txn.GetOutEdgeIterator(EdgeUid(src, 0, 1, 0, 0), true);
            UT_ASSERT(eit2.IsValid() && eit2.GetTemporalId() == 1);
            UT_ASSERT(fabs(txn.GetEdgeField(eit2, size_t(1)).AsFloat() - 1.1) < 1e-6);
            eit2.Next();
            UT_ASSERT(eit2.IsValid() && eit2.GetTemporalId() == 1);
            UT_ASSERT(fabs(txn.GetEdgeField(eit2, size_t(1)).AsFloat() - 0.1) < 1e-6);
            eit2.Next();
            eit2.Next();
            UT_ASSERT(eit2.IsValid() && eit2.GetTemporalId() == 2);
            UT_ASSERT(fabs(txn.GetEdgeField(eit2, size_t(1)).AsFloat() - 2.2) < 1e-6);
        }
    } catch (std::exception& e) {
        ERR() << "Error occurred: " << e.what();
        UT_EXPECT_TRUE(false);
    }

    try {
        UT_LOG() << "Checking index";
        auto txn = db.CreateReadTxn();
        for (auto& kv : vn_size) {
            VertexId vid = kv.first;
            int size = kv.second;
            FieldData data(GetVName(size, vid));
            auto iit = txn.GetVertexIndexIterator("v", "name", data, data);
            UT_EXPECT_TRUE(iit.IsValid());
            UT_EXPECT_EQ(iit.GetVid(), vid);
            UT_EXPECT_TRUE(!iit.Next());
        }
        DumpGraph(txn);
    } catch (std::exception& e) {
        ERR() << "Error occurred: " << e.what();
        UT_EXPECT_TRUE(false);
    }

    std::set<std::tuple<VertexId, LabelId, VertexId, EdgeId>> updated_edges;
    try {
        UT_LOG() << "Updating vertex";
        // update vertex attributes
        for (size_t i = 0; i < nv; i++) {
            auto txn = db.CreateWriteTxn();
            VertexId vid = vid_dist(rgen) % nv;
            int vname_size = std::min<int>(460, (int)(vname_size_dist(rgen)));
            vname_size = std::max<int>(vname_size, 1);
            vn_size[vid] = vname_size;
            UT_EXPECT_TRUE(UpdateVertex(txn, vid, GetVName(vname_size, vid)));
            txn.Commit();
        }
        UT_LOG() << "Updating edges";
        // update edge attributes
        for (size_t i = 0; i < nv; i++) {
            auto txn = db.CreateWriteTxn();
            VertexId vid = vid_dist(rgen) % nv;
            auto edges = txn.ListOutEdges(vid);
            if (edges.empty()) continue;
            int ne = (int)epv_dist(rgen);
            for (int j = 0; j < ne; j++) {
                int64_t v = vid_dist(rgen) % edges.size();
                auto edge = edges[v];
                LabelId lid = edge.lid;
                TemporalId tid = edge.tid;
                VertexId vid2 = edge.dst;
                EdgeId eid = edge.eid;
                if (eid == 0) {
                    UT_EXPECT_TRUE(
                        UpsertEdge(txn, vid, lid, vid2, (vid + vid2) % 128, (float)(vid + vid2)));
                } else {
                    UT_EXPECT_TRUE(UpdateEdge(txn, vid, lid, tid, vid2, eid));
                }
                updated_edges.emplace(vid, lid, vid2, eid);
            }
            txn.Commit();
        }
        auto txn = db.CreateReadTxn();
        DumpGraph(txn);
    } catch (std::exception& e) {
        ERR() << "Error occurred: " << e.what();
        UT_EXPECT_TRUE(false);
    }

    try {
        UT_LOG() << "Deleting edges";
        auto txn = db.CreateWriteTxn();
        for (size_t i = 0; i < nv * 2; i++) {
            VertexId src = vid_dist(rgen) % nv;
            VertexId dst = vid_dist(rgen) % nv;
            auto eit = txn.GetOutEdgeIterator(EdgeUid(src, dst, 0, 0, 0), true);
            auto& edges = all_edges[src];
            auto eeit = edges.find(dst);
            if (eeit != edges.end()) {
                UT_EXPECT_TRUE(eit.IsValid());
                UT_EXPECT_EQ(eit.GetDst(), dst);
                EdgeId eid = eit.GetEdgeId();
                txn.DeleteEdge(eit);
                UT_EXPECT_EQ(eeit->second.erase(eid), 1);
                if (eeit->second.empty()) edges.erase(eeit);
                std::tuple<VertexId, LabelId, VertexId, EdgeId> e(src, 0, dst, eid);
                updated_edges.erase(e);
            } else {
                UT_EXPECT_TRUE(!eit.IsValid() || eit.GetDst() != dst || eit.GetEdgeId() != 0);
            }
        }

        UT_LOG() << "Deleting vertex";
        int n = (int)vid_dist(rgen) % nv;
        for (int i = 0; i < n; i++) {
            VertexId vid = vid_dist(rgen) % (nv * 3);
            auto vit = txn.GetVertexIterator(vid);
            if (vn_size.find(vid) != vn_size.end()) {
                auto srcs = vit.ListSrcVids();
                auto inedges = txn.ListInEdges(vid);
                auto outedges = txn.ListOutEdges(vid);
                vn_size.erase(vid);
                size_t no = 0;
                auto& edges = all_edges[vid];
                for (auto& dst_eids : edges) {
                    no += dst_eids.second.size();
                }
                size_t ni = 0;
                for (auto src : srcs) {
                    auto& edges = all_edges[src][vid];
                    UT_EXPECT_TRUE(!edges.empty());
                    ni += edges.size();
                    all_edges[src].erase(vid);
                }
                edges.clear();
                UT_EXPECT_TRUE(vit.IsValid());
                size_t nii, noo;
                txn.DeleteVertex(vit, &nii, &noo);
                UT_EXPECT_EQ(noo, no);
                UT_EXPECT_EQ(nii, ni);
                for (auto ie : inedges) {
                    updated_edges.erase(std::tuple<VertexId, LabelId, VertexId, EdgeId>(
                        ie.src, ie.lid, ie.dst, ie.eid));
                }
                for (auto oe : outedges) {
                    updated_edges.erase(std::tuple<VertexId, LabelId, VertexId, EdgeId>(
                        oe.src, oe.lid, oe.dst, oe.eid));
                }
            } else {
                UT_EXPECT_TRUE(!vit.IsValid());
            }
        }
        txn.Commit();
    } catch (std::exception& e) {
        ERR() << "Error occurred: " << e.what();
        UT_EXPECT_TRUE(false);
    }

    try {
        UT_LOG() << "Checking data integrity";
        size_t nv1 = 0;
        size_t ne = 0;
        size_t nem = 0;
        auto txn = db.CreateReadTxn();
        auto vit = txn.GetVertexIterator();
        while (vit.IsValid()) {
            nv1++;
            VertexId vid = vit.GetId();
            auto& edges = all_edges[vid];
            auto eit = vit.GetOutEdgeIterator();
            while (eit.IsValid()) {
                ne++;
                VertexId dst = eit.GetDst();
                EdgeId eid = eit.GetEdgeId();
                auto w = txn.GetEdgeField(eit, (size_t)1);
                UT_EXPECT_TRUE(!w.is_null());
                UT_EXPECT_EQ(w.type, FieldType::FLOAT);
                auto mit = edges.find(dst);
                UT_EXPECT_TRUE(mit != edges.end());
                auto eeit = mit->second.find(eid);
                UT_EXPECT_TRUE(eeit != mit->second.end());
                std::tuple<VertexId, LabelId, VertexId, EdgeId> e(vid, 0, dst, eid);
                auto ueit = updated_edges.find(e);
                if (ueit != updated_edges.end()) {
                    nem++;
                    UT_EXPECT_EQ(w.AsFloat(), (float)(vid + dst) * 10);
                } else {
                    UT_EXPECT_EQ(w.AsFloat(), (float)(vid + dst));
                }
                eit.Next();
            }
            vit.Next();
        }
        UT_EXPECT_EQ(vn_size.size(), nv1);
        UT_EXPECT_EQ(nem, updated_edges.size());
        for (auto& kv : all_edges) {
            for (auto& dst_edges : kv.second) {
                ne -= dst_edges.second.size();
            }
        }
        UT_EXPECT_EQ(ne, 0);
    } catch (std::exception& e) {
        ERR() << "Error occurred: " << e.what();
        UT_EXPECT_TRUE(false);
    }
    UT_LOG() << "test delete all index";
    {
        db.DropAllIndex();
        auto txn = db.CreateReadTxn();
        db.Snapshot(txn, "db");
        txn.Abort();
        DBConfig config1;
        config1.dir = "./lgdb";
        LightningGraph db1(config1);
        db1.LoadSnapshot("db");
        db1.DropAllIndex();
        std::vector<FieldSpec> e_q = {
            {"date", FieldType::DATE, false},         {"BOOL", FieldType::BOOL, false},
            {"DATETIME", FieldType::DATETIME, false}, {"FLOAT", FieldType::FLOAT, false},
            {"DOUBLE", FieldType::DOUBLE, false},     {"STRING1", FieldType::STRING, false}};
        UT_EXPECT_TRUE(db1.AddLabel("vl", e_q, true, VertexOptions("date")));
        UT_EXPECT_TRUE(db1.BlockingAddIndex("vl", "DATETIME",
                                            lgraph::IndexType::GlobalUniqueIndex, true));
        UT_EXPECT_TRUE(db1.BlockingAddIndex("vl", "BOOL",
                                            lgraph::IndexType::GlobalUniqueIndex, true));
        UT_EXPECT_TRUE(db1.BlockingAddIndex("vl", "FLOAT",
                                            lgraph::IndexType::GlobalUniqueIndex, true));
        UT_EXPECT_TRUE(db1.BlockingAddIndex("vl", "DOUBLE",
                                            lgraph::IndexType::GlobalUniqueIndex, true));
        UT_EXPECT_TRUE(db1.BlockingAddIndex("vl", "STRING1",
                                            lgraph::IndexType::NonuniqueIndex, true));
        size_t n_mod = 20000;
        db1.DelLabel("v1", true, &n_mod);
    }

    UT_LOG() << "Test adding label with illegal field name";
    {
        DBConfig config;
        config.dir = "./testdb";
        LightningGraph db(config);
        db.DropAllData();
        std::vector<FieldSpec> v_fds1 = {{"SKIP", FieldType::STRING, false},
                                         {"id", FieldType::INT8, false}};
        std::vector<FieldSpec> v_fds2 = {{"SRC_ID", FieldType::STRING, false},
                                         {"id", FieldType::INT8, false}};
        std::vector<FieldSpec> v_fds3 = {{"DST_ID", FieldType::STRING, false},
                                         {"id", FieldType::INT8, false}};
        std::vector<FieldSpec> e_fds = {{"DST_ID", FieldType::STRING, false},
                                        {"SKIP", FieldType::STRING, false},
                                        {"DST_ID", FieldType::STRING, false},
                                        {"weight", FieldType::FLOAT, false}};
        UT_EXPECT_ANY_THROW(db.AddLabel("v1", v_fds1, true, VertexOptions("id")));
        UT_EXPECT_ANY_THROW(db.AddLabel("v1", v_fds2, true, VertexOptions("id")));
        UT_EXPECT_ANY_THROW(db.AddLabel("v1", v_fds3, true, VertexOptions("id")));
        UT_EXPECT_ANY_THROW(db.AddLabel("e1", e_fds, false, EdgeOptions()));
    }

    UT_LOG() << "Test batch adding index";
    {
        DBConfig config;
        config.dir = "./indexdb";
        AutoCleanDir _("./indexdb");
        LightningGraph db(config);
        db.DropAllData();
        std::vector<FieldSpec> v_fds = {{"uid", FieldType::STRING, false},
                       {"name", FieldType::STRING, false}, {"phone", FieldType::STRING, false}};
        std::vector<FieldSpec> e_fds = {{"since", FieldType::INT8, false},
                       {"weight", FieldType::FLOAT, false}, {"comments", FieldType::INT8, false}};
        UT_EXPECT_TRUE(db.AddLabel("person", v_fds, true, VertexOptions("uid")));
        UT_EXPECT_TRUE(db.AddLabel("follow", e_fds, false, EdgeOptions()));
        std::vector<std::string> v_properties = {"uid", "name", "phone"};
        std::vector<std::string> e_properties = {"since", "weight", "comments"};
        Transaction txn = db.CreateWriteTxn();
        VertexId v_id1 = txn.AddVertex(std::string("person"), v_properties,
                                       std::vector<std::string>{"uid1", "name1", "phone"});
        VertexId v_id2 = txn.AddVertex(std::string("person"), v_properties,
                                       std::vector<std::string>{"uid2", "name2", "phone"});
        VertexId v_id3 = txn.AddVertex(std::string("person"), v_properties,
                                       std::vector<std::string>{"uid3", "name3", "phone"});
        txn.AddEdge(v_id1, v_id2, std::string("follow"), e_properties,
                    std::vector<std::string>{"19", "0.1", "19"});
        txn.AddEdge(v_id2, v_id3, std::string("follow"), e_properties,
                    std::vector<std::string>{"20", "0.1", "19"});
        txn.AddEdge(v_id1, v_id3, std::string("follow"), e_properties,
                    std::vector<std::string>{"21", "0.1", "19"});
        txn.AddEdge(v_id2, v_id3, std::string("follow"), e_properties,
                    std::vector<std::string>{"22", "0.2", "19"});
        txn.Commit();
        UT_EXPECT_TRUE(db.BlockingAddIndex("person", "name",
                                            lgraph::IndexType::GlobalUniqueIndex, true));
        UT_EXPECT_ANY_THROW(db.BlockingAddIndex("person", "phone",
                                            lgraph::IndexType::GlobalUniqueIndex, true));
        UT_EXPECT_TRUE(db.BlockingAddIndex("follow", "since",
                                           lgraph::IndexType::GlobalUniqueIndex, false));
        UT_EXPECT_TRUE(db.BlockingAddIndex("follow", "weight",
                                                lgraph::IndexType::PairUniqueIndex, false));
        UT_EXPECT_ANY_THROW(db.BlockingAddIndex("follow", "comments",
                                           lgraph::IndexType::PairUniqueIndex, false));
        db.DropAllIndex();
        std::vector<lgraph::IndexSpec> vertex_idxs{
            {"person", "name", lgraph::IndexType::GlobalUniqueIndex},
            {"person", "phone", lgraph::IndexType::GlobalUniqueIndex}
        };
        UT_EXPECT_ANY_THROW(db.OfflineCreateBatchIndex(vertex_idxs, 1, true));
        std::vector<lgraph::IndexSpec> edge_idxs{
            {"follow", "since", lgraph::IndexType::GlobalUniqueIndex},
            {"follow", "weight", lgraph::IndexType::GlobalUniqueIndex},
            {"follow", "comments", lgraph::IndexType::GlobalUniqueIndex}
        };
        UT_EXPECT_ANY_THROW(db.OfflineCreateBatchIndex(edge_idxs, 1, false));
    }
}
