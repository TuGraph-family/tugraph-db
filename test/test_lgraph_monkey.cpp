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

// LGraph monkey test
// Uses lgraph_api as a blackbox and tries different operations in some random manner

#include <algorithm>
#include <map>
#include <random>
#include <string>

#include "fma-common/configuration.h"
#include "fma-common/file_system.h"
#include "fma-common/utils.h"
#include "gtest/gtest.h"

#include "core/field_data_helper.h"
#include "lgraph/lgraph.h"
#include "./random_port.h"
#include "./test_tools.h"
using namespace lgraph_api;

class TestLGraphMonkey : public TuGraphTestWithParam<struct ParamMonkey> {};

struct ParamMonkey {
    ParamMonkey(size_t _nv, size_t _huge) {
        nv = _nv;
        percent_huge = _huge;
    }
    ParamMonkey(size_t _nv, size_t _epv, size_t _nvlabel, size_t _nelabel, size_t _huge,
                size_t _update, size_t _monkey) {
        nv = _nv;
        epv = _epv;
        nvlabel = _nvlabel;
        nelabel = _nelabel;
        percent_huge = _huge;
        percent_update = _update;
        n_monkey = _monkey;
    }
    size_t nv = 100;
    size_t epv = 30;
    size_t nvlabel = 3;
    size_t nelabel = 3;
    size_t percent_huge = 5;
    size_t percent_update = 10;
    size_t percent_delete = 10;
    size_t n_monkey = 100;
};

static std::string RandomString(size_t n) {
    std::string str(n, 0);
    for (size_t i = 0; i < n; i++) str[i] = (myrand() % 26) + 'a';
    return str;
}

uint64_t seed[2];

/* xorshift128plus 1.12ns/64bit */
inline uint64_t MyRand(uint64_t* seed) {
    uint64_t x = seed[0];
    uint64_t const y = seed[1];
    seed[0] = y;
    x ^= x << 23;
    x ^= x >> 17;
    x ^= y ^ (y >> 26);
    seed[1] = x;
    return x + y;
}

inline void MySrand(uint64_t sd) {
    seed[0] = sd * 7 + 24;
    seed[1] = sd * 31 + 921;
}

static const uint64_t MAX_UINT64 = std::numeric_limits<uint64_t>::max();

inline void ChoosePartitionRmat(uint64_t* seed, uint64_t* u, uint64_t* v, int step, double a,
                                double b, double c, double d) {
    uint64_t p;
    p = MyRand(seed);
    if (p < (b + d) * MAX_UINT64) *v = *v + step;
    if (p > b * MAX_UINT64 && p < (b + c + d) * MAX_UINT64) *u = *u + step;
}

inline void VaryPara(uint64_t* seed, double* a, double* b, double* c, double* d) {
    double s;
    *a = *a * (0.95 + 0.1 * MyRand(seed) / MAX_UINT64);
    *b = *b * (0.95 + 0.1 * MyRand(seed) / MAX_UINT64);
    *c = *c * (0.95 + 0.1 * MyRand(seed) / MAX_UINT64);
    *d = *d * (0.95 + 0.1 * MyRand(seed) / MAX_UINT64);
    s = *a + *b + *c + *d;
    *a = *a / s;
    *b = *b / s;
    *c = *c / s;
    *d = *d / s;
}

inline uint64_t Permutation(uint64_t x, uint64_t n) { return ((x + 495211) * 179428261 % n); }

static std::pair<std::string, std::vector<FieldSpec>> RandomSchema(size_t n_fields) {
    std::unordered_set<std::string> fields;
    while (fields.size() < n_fields) {
        fields.emplace(RandomString(10));
    }
    std::vector<FieldSpec> fs;
    // int i = 0;
    for (auto& fn : fields) {
        FieldSpec f;
        f.name = fn;
        f.optional = true;
        f.type = FieldType::INT64;
        // f.id = i++;
        fs.push_back(f);
    }
    if (n_fields >= 1) {
        fs[0].optional = false;
        fs[0].type = FieldType::STRING;
    }
    if (n_fields >= 2) fs[1].type = FieldType::STRING;
    return std::make_pair(RandomString(10), std::move(fs));
}

class MonkeyTest {
    typedef int64_t VertexId;
    typedef int64_t EdgeId;
    typedef uint16_t LabelId;

    typedef std::map<std::string, FieldData> Properties;
    struct Edge {
        Edge() {}
        Edge(const std::string& l, const Properties& p) : label(l), props(p) {}

        std::string label;
        Properties props;
    };
    typedef std::map<EdgeUid, Edge, EdgeUid::InEdgeSortOrder> InEdges;
    typedef std::map<EdgeUid, Edge, EdgeUid::OutEdgeSortOrder> OutEdges;
    struct Vertex {
        Vertex() {}
        Vertex(const std::string& l, const Properties& p) : label(l), props(p) {}

        std::string label;
        Properties props;
        InEdges ins;
        OutEdges outs;
    };
    typedef std::map<VertexId, Vertex> Graph;
    typedef std::map<std::string, std::map<std::string, FieldSpec>> Schema;

    Graph vgraph_;  // in-memory graph for verification
    Schema vschema_;
    Schema eschema_;
    lgraph_api::Galaxy galaxy_;
    GraphDB db_;
    std::unique_ptr<Transaction> txn_;
    std::unordered_map<std::string, std::string> label_primary_;
    uint64_t unique_id_ = 0;

 private:
    std::pair<std::vector<std::string>, std::vector<FieldData>> ToKeysAndValues(
        const Properties& props) {
        std::vector<std::string> keys;
        keys.reserve(props.size());
        std::vector<FieldData> values;
        values.reserve(props.size());
        for (auto& kv : props) {
            keys.emplace_back(kv.first);
            values.emplace_back(kv.second);
        }
        return std::make_pair(std::move(keys), std::move(values));
    }

    bool AddLabel(bool is_vertex, Schema& schema, const std::string& label,
                  const std::vector<FieldSpec>& fds, const std::string& primary,
                  const lgraph::EdgeConstraints& constraints) {
        txn_.reset();
        bool r = is_vertex ? db_.AddVertexLabel(label, fds, VertexOptions(primary))
                           : db_.AddEdgeLabel(label, fds, EdgeOptions(constraints));
        auto vgit = schema.find(label);
        UT_EXPECT_EQ(r, vgit == schema.end());
        if (!r) return r;
        std::map<std::string, FieldSpec> tmp;
        for (auto& f : fds) tmp.emplace(f.name, f);
        schema.emplace_hint(vgit, label, std::move(tmp));
        if (is_vertex) {
            label_primary_[label] = primary;
        }
        return r;
    }

    void VerifySchema(bool is_vertex, const Schema& schema) {
        BeginReadTxn();
        auto labels = is_vertex ? txn_->ListVertexLabels() : txn_->ListEdgeLabels();
        UT_EXPECT_EQ(labels.size(), schema.size());
        for (auto& label : labels) {
            auto sit = schema.find(label);
            UT_EXPECT_TRUE(sit != schema.end());
            auto fds = is_vertex ? txn_->GetVertexSchema(label) : txn_->GetEdgeSchema(label);
            std::map<std::string, FieldSpec> schema;
            for (auto& fd : fds) schema.emplace(fd.name, fd);
            UT_EXPECT_TRUE(schema == sit->second);
        }
    }

    Properties GenProperties(const std::string& label, const Schema& schema, size_t size) {
        auto it = schema.find(label);
        UT_EXPECT_TRUE(it != schema.end());
        Properties props;
        std::string prefix;
        for (auto& fkv : it->second) {
            size_t psize = size;
            if (label_primary_.find(label) != label_primary_.end()) {
                if (fkv.first == label_primary_[label]) {
                    if (psize > lgraph::_detail::MAX_KEY_SIZE - 20)
                        psize = lgraph::_detail::MAX_KEY_SIZE - 20;
                    prefix = std::to_string(unique_id_++);
                }
            }
            const FieldSpec& fs = fkv.second;
            if (fs.type == FieldType::STRING) {
                size_t padding = psize >= prefix.size() ? psize - prefix.size() : psize;
                std::string prop = prefix + RandomString(padding);
                props.emplace(fs.name, FieldData(prop));
            } else {
                props.emplace(fs.name, FieldData::Int64(myrand()));
            }
        }
        return props;
    }

 public:
    explicit MonkeyTest(const std::string& dir)
        : galaxy_(dir,
                  lgraph::_detail::DEFAULT_ADMIN_NAME,
                  lgraph::_detail::DEFAULT_ADMIN_PASS, false, true),
          db_(galaxy_.OpenGraph("default")) {
        // load from db to vgraph
    }

    void BeginReadTxn() {
        txn_.reset();
        txn_.reset(new Transaction(db_.CreateReadTxn()));
    }

    void BeginWriteTxn() {
        txn_.reset();
        txn_.reset(new Transaction(db_.CreateWriteTxn()));
    }

    void AbortTxn() {
        if (txn_) txn_->Abort();
        txn_.reset();
    }

    void CommitTxn() {
        if (txn_) txn_->Commit();
        txn_.reset();
    }

    bool AddVertexLabel(const std::string& label, const std::vector<FieldSpec>& fds,
                        const std::string& primary) {
        UT_DBG() << "AddVertexLabel(" << label << ")";
        return AddLabel(true, vschema_, label, fds, primary, {});
    }

    bool AddEdgeLabel(const std::string& label, const std::vector<FieldSpec>& fds,
                      const lgraph::EdgeConstraints& ec) {
        UT_DBG() << "AddEdgeLabel(" << label << ")";
        return AddLabel(false, eschema_, label, fds, {}, ec);
    }

    VertexId AddVertex(const std::string& label, size_t psize) {
        Properties props = GenProperties(label, vschema_, psize);
        auto kvs = ToKeysAndValues(props);
        VertexId vid = txn_->AddVertex(label, kvs.first, kvs.second);
        UT_DBG() << "AddVertex(label=" << label << ", psize=" << psize << "), vid=" << vid;
        auto it = vgraph_.find(vid);
        UT_EXPECT_TRUE(it == vgraph_.end());
        vgraph_.emplace_hint(it, vid, Vertex(label, props));
        return vid;
    }

    bool UpdateVertex(VertexId vid, size_t psize) {
        UT_DBG() << "UpdateVertex(vid=" << vid << ", psize=" << psize << ")";
        auto dbit = txn_->GetVertexIterator(vid);
        auto vgit = vgraph_.find(vid);
        UT_EXPECT_EQ(dbit.IsValid(), vgit != vgraph_.end());
        if (!dbit.IsValid()) return false;
        Properties props = GenProperties(vgit->second.label, vschema_, psize);
        auto kvs = ToKeysAndValues(props);
        dbit.SetFields(kvs.first, kvs.second);
        // todo: check schema
        vgit->second.props = props;
        return true;
    }

    std::vector<VertexId> GetVids() const {
        std::vector<VertexId> vids;
        vids.reserve(vgraph_.size());
        for (auto& kv : vgraph_) vids.emplace_back(kv.first);
        UT_DBG() << "GetVids() returns: " << UT_FMT("{}", vids);
        return vids;
    }

    std::vector<EdgeUid> GetEdges() const {
        std::vector<EdgeUid> euids;
        for (auto& v : vgraph_) {
            for (auto& e : v.second.outs) {
                euids.push_back(e.first);
            }
        }
        UT_DBG() << "GetEdges() returns: " << UT_FMT("{}", euids);
        return euids;
    }

    VertexId RandomV() const {
        if (vgraph_.empty()) return -1;
        int64_t max_vid = vgraph_.rbegin()->first;
        return vgraph_.lower_bound(myrand() % (max_vid + 1))->first;
    }

    EdgeUid RandomE() const {
        if (vgraph_.empty()) return EdgeUid(-1, 0, 0, 0, 0);
        VertexId vid = RandomV();
        auto it = vgraph_.find(vid);
        size_t i = 0;
        while (i < vgraph_.size()) {
            if (it->second.outs.empty()) {
                i++;
                it++;
                if (it == vgraph_.end()) it = vgraph_.begin();
                continue;
            }
            auto eit = it->second.outs.begin();
            for (size_t j = 0; j < myrand() % it->second.outs.size(); j++) eit++;
            return eit->first;
        }
        return EdgeUid(-1, 0, 0, 0, 0);
    }

    EdgeUid AddEdge(VertexId src, VertexId dst, const std::string& label, size_t psize) {
        Properties props = GenProperties(label, eschema_, psize);
        auto kvs = ToKeysAndValues(props);
        EdgeUid euid = txn_->AddEdge(src, dst, label, kvs.first, kvs.second);
        UT_DBG() << UT_FMT("AddEdge(src={}, dst={}, label={}, psize={}), euid={}", src, dst, label,
                           psize, euid);
        auto vgit = vgraph_.find(src);
        UT_EXPECT_TRUE(vgit != vgraph_.end());
        UT_EXPECT_TRUE(vgit->second.outs.find(euid) == vgit->second.outs.end());
        vgit->second.outs.emplace(euid, Edge(label, props));
        vgit = vgraph_.find(dst);
        UT_EXPECT_TRUE(vgit != vgraph_.end());
        UT_EXPECT_TRUE(vgit->second.ins.find(euid) == vgit->second.ins.end());
        vgit->second.ins.emplace(euid, Edge(label, props));
        return euid;
    }

    bool UpdateEdge(const EdgeUid& euid, size_t psize) {
        UT_DBG() << UT_FMT("UpdateEdge(euid={}, psize={})", euid, psize);
        auto dbit = txn_->GetOutEdgeIterator(euid);
        auto vgit_src = vgraph_.find(euid.src);
        auto vgit_dst = vgraph_.find(euid.dst);
        if (vgit_src == vgraph_.end() || vgit_dst == vgraph_.end()) {
            UT_EXPECT_TRUE(!dbit.IsValid());
            return false;
        }
        auto vgoeit = vgit_src->second.outs.find(euid);
        UT_EXPECT_EQ(dbit.IsValid(), vgoeit != vgit_src->second.outs.end());
        auto vgieit = vgit_dst->second.ins.find(euid);
        UT_EXPECT_EQ(dbit.IsValid(), vgieit != vgit_dst->second.ins.end());
        if (!dbit.IsValid()) return false;
        Properties props = GenProperties(vgoeit->second.label, eschema_, psize);
        auto kvs = ToKeysAndValues(props);
        dbit.SetFields(kvs.first, kvs.second);
        vgoeit->second.props = props;
        vgieit->second.props = props;
        return true;
    }

    bool DeleteVertex(VertexId vid) {
        UT_DBG() << UT_FMT("DeleteVertex(vid={})", vid);
        auto dbit = txn_->GetVertexIterator(vid);
        auto vgit = vgraph_.find(vid);
        UT_EXPECT_EQ(dbit.IsValid(), vgit != vgraph_.end());
        if (!dbit.IsValid()) return false;
        size_t ni, no;
        dbit.Delete(&ni, &no);
        UT_EXPECT_EQ(ni, vgit->second.ins.size());
        UT_EXPECT_EQ(no, vgit->second.outs.size());
        for (auto& out : vgit->second.outs) {
            EdgeUid euid = out.first;
            vgraph_[euid.dst].ins.erase(euid);
        }
        for (auto& in : vgit->second.ins) {
            EdgeUid euid = in.first;
            vgraph_[euid.src].outs.erase(euid);
        }
        vgraph_.erase(vgit);
        return true;
    }

    bool DeleteEdge(const EdgeUid& euid) {
        UT_DBG() << UT_FMT("DeleteEdge(euid={})", euid);
        auto dbit = txn_->GetOutEdgeIterator(euid);
        auto vgit_src = vgraph_.find(euid.src);
        auto vgit_dst = vgraph_.find(euid.dst);
        if (vgit_src == vgraph_.end() || vgit_dst == vgraph_.end()) {
            UT_EXPECT_TRUE(!dbit.IsValid());
            return false;
        }
        auto vgoeit = vgit_src->second.outs.find(euid);
        UT_EXPECT_EQ(dbit.IsValid(), vgoeit != vgit_src->second.outs.end());
        auto vgieit = vgit_dst->second.ins.find(euid);
        UT_EXPECT_EQ(dbit.IsValid(), vgieit != vgit_dst->second.ins.end());
        if (!dbit.IsValid()) return false;
        dbit.Delete();
        vgit_src->second.outs.erase(vgoeit);
        vgit_dst->second.ins.erase(vgieit);
        return true;
    }

    // verify that db has exactly the same content with vgraph
    void Verify() {
        // check schemas
        VerifySchema(true, vschema_);
        VerifySchema(false, eschema_);
        // check vertex and edges
        auto vgit = vgraph_.begin();
        for (auto dbit = txn_->GetVertexIterator(); dbit.IsValid(); dbit.Next(), vgit++) {
            UT_EXPECT_TRUE(vgit != vgraph_.end());
            UT_EXPECT_EQ(dbit.GetId(), vgit->first);
            UT_EXPECT_EQ(dbit.GetLabel(), vgit->second.label);
            auto fvs = dbit.GetAllFields();
            UT_EXPECT_TRUE(fvs == vgit->second.props);
            // check in-edges
            UT_EXPECT_EQ(dbit.GetNumInEdges(), vgit->second.ins.size());
            auto vgieit = vgit->second.ins.begin();
            for (auto ieit = dbit.GetInEdgeIterator(); ieit.IsValid(); ieit.Next(), vgieit++) {
                UT_EXPECT_TRUE(vgieit != vgit->second.ins.end());
                UT_EXPECT_TRUE(vgieit->first == ieit.GetUid());       // check EdgeId
                UT_EXPECT_EQ(vgieit->second.label, ieit.GetLabel());  // label
                auto t = ieit.GetAllFields();
                UT_EXPECT_TRUE(vgieit->second.props == t);  // props
            }
            // check out-edges
            auto vgoeit = vgit->second.outs.begin();
            for (auto oeit = dbit.GetOutEdgeIterator(); oeit.IsValid(); oeit.Next(), vgoeit++) {
                UT_EXPECT_TRUE(vgoeit != vgit->second.outs.end());
                UT_EXPECT_TRUE(vgoeit->first == oeit.GetUid());               // check EdgeId
                UT_EXPECT_EQ(vgoeit->second.label, oeit.GetLabel());          // label
                UT_EXPECT_TRUE(vgoeit->second.props == oeit.GetAllFields());  // props
            }
        }
    }
};

TEST_P(TestLGraphMonkey, LGraphMonkey) {
    std::string dir = "testdb";
    size_t nv = 100;
    size_t nvlabel = 3;
    size_t nelabel = 3;
    size_t epv = 30;
    size_t percent_huge = 5;
    size_t percent_update = 10;
    size_t percent_delete = 10;
    size_t n_monkey = 100;
    bool verbose = false;

    nv = GetParam().nv;
    epv = GetParam().epv;
    nvlabel = GetParam().nvlabel;
    nelabel = GetParam().nelabel;
    percent_huge = GetParam().percent_huge;
    percent_delete = GetParam().percent_delete;
    percent_update = GetParam().percent_update;
    n_monkey = GetParam().n_monkey;
    int argc = _ut_argc;
    char** argv = _ut_argv;
    fma_common::Configuration config;
    config.Add(dir, "dir", true).Comment("DB dir");
    config.Add(nv, "nv", true).Comment("Number of vertex");
    config.Add(nvlabel, "vlabel", true).Comment("Number of vertex labels");
    config.Add(nelabel, "elabel", true).Comment("Number of edge labels");
    config.Add(epv, "epv", true).Comment("Number of edges per vertex");
    config.Add(percent_huge, "huge", true).Comment("Percentage of huge vertex/edge");
    config.Add(percent_update, "update", true).Comment("Percentage of vertex/edges to update");
    config.Add(percent_delete, "delete", true).Comment("Percentage of vertex/edges to delete");
    config.Add(n_monkey, "monkey", true).Comment("Number of monkey operations to perform");
    config.Add(verbose, "verbose", true).Comment("Print verbose info");
    config.ParseAndFinalize(argc, argv);

    { fma_common::FileSystem::GetFileSystem(dir).RemoveDir(dir); }
    std::vector<std::string> vlabels;
    std::vector<std::string> elabels;
    MonkeyTest test(dir);
    UT_LOG() << "Adding labels";
    {
        // add labels
        while (vlabels.size() < nvlabel) {
            auto lschema = RandomSchema(10);
            if (test.AddVertexLabel(lschema.first, lschema.second, lschema.second[0].name)) {
                vlabels.emplace_back(lschema.first);
            }
        }
        while (elabels.size() < nelabel) {
            auto lschema = RandomSchema(10);
            if (test.AddEdgeLabel(lschema.first, lschema.second, {})) {
                elabels.emplace_back(lschema.first);
            }
        }
    }
    UT_LOG() << "Adding vertex";
    int64_t max_vid = 0;
    {
        test.BeginWriteTxn();
        for (size_t i = 0; i < nv; i++) {
            size_t psize = myrand() % 100 <= percent_huge ? myrand() % (1 << 12) : myrand() % 10;
            auto vid = test.AddVertex(vlabels[myrand() % nvlabel], psize);
            UT_EXPECT_EQ(vid, i);
        }
        max_vid = nv - 1;
        test.CommitTxn();
        // try adding large vertex, which should fail
        UT_EXPECT_ANY_THROW(
            {
                test.BeginWriteTxn();
                for (int i = 0; i < 10; i++) {
                    size_t psize = lgraph::_detail::MAX_STRING_SIZE + 2;
                    auto vid = test.AddVertex(vlabels[myrand() % nvlabel], psize);
                    UT_DBG() << "success vid: " << vid;
                }
                test.CommitTxn();
            });
        test.AbortTxn();
        nv = max_vid + 1;
    }
    UT_LOG() << "Adding edges";
    {
        test.BeginWriteTxn();
        seed[0] = 132;
        seed[1] = 11;
        std::vector<size_t> nos(nv * 10, 0);
        std::vector<size_t> nis(nv * 10, 0);
        for (size_t i = 0; i < nv * epv; i++) {
            uint64_t step;

            double a = 0.57;
            double b = 0.19;
            double c = 0.19;
            double d = 0.05;
            uint64_t src = 0;
            uint64_t dst = 0;
            step = ((uint64_t)nv >> 1);
            while (step >= 1) {
                ChoosePartitionRmat(seed, &src, &dst, static_cast<int>(step), a, b, c, d);
                VaryPara(seed, &a, &b, &c, &d);
                step = step >> 1;
            }
            src = Permutation(src, nv);
            dst = Permutation(dst, nv);

            size_t psize = myrand() % 100 <= percent_huge ? myrand() % (1 << 12) : myrand() % 10;
            EdgeUid euid = test.AddEdge(src, dst, elabels[myrand() % nelabel], psize);
            UT_EXPECT_EQ(euid.src, src);
            UT_EXPECT_EQ(euid.dst, dst);
        }
        test.CommitTxn();
        // try adding some large edges
        UT_EXPECT_ANY_THROW(
            {
                test.BeginWriteTxn();
                for (int i = 0; i < 10; i++) {
                    size_t psize = lgraph::_detail::MAX_PROP_SIZE + 1;
                    test.AddEdge(0, 0, elabels[myrand() % nelabel], psize);
                }
                test.CommitTxn();
            });
        test.AbortTxn();
    }
    std::random_device rd;
    std::mt19937 rg(0);
    UT_LOG() << "Delete vertex";
    {
        test.BeginWriteTxn();
        test.GetVids();
        std::vector<int64_t> vids(nv, 0);
        for (size_t i = 0; i < nv; i++) vids[i] = i;
        std::shuffle(vids.begin(), vids.end(), rg);
        for (size_t i = 0; i < percent_delete * nv / 100; i++) {
            int64_t vid = vids[i];
            UT_EXPECT_TRUE(test.DeleteVertex(vid));
        }
        // try deleting already-deleted vertex
        for (size_t i = 0; i < percent_delete * nv / 100; i++) {
            int64_t vid = vids[i];
            UT_EXPECT_TRUE(!test.DeleteVertex(vid));
        }
        test.CommitTxn();
    }
    UT_LOG() << "Update vertex";
    {
        test.BeginWriteTxn();
        std::vector<int64_t> vids = test.GetVids();
        std::shuffle(vids.begin(), vids.end(), rg);
        for (size_t i = 0; i < percent_update * nv / 100; i++) {
            int64_t vid = vids[i];
            size_t psize = myrand() % 100 <= percent_huge ? myrand() % (1 << 12) : myrand() % 10;
            UT_EXPECT_TRUE(test.UpdateVertex(vid, psize));
        }
        test.CommitTxn();
    }
    UT_LOG() << "Delete edge";
    {
        test.BeginWriteTxn();
        std::vector<EdgeUid> euids = test.GetEdges();
        std::shuffle(euids.begin(), euids.end(), rg);
        for (size_t i = 0; i < percent_delete * euids.size() / 100; i++) {
            UT_EXPECT_TRUE(test.DeleteEdge(euids[i]));
        }
        for (size_t i = 0; i < percent_delete * euids.size() / 100; i++) {
            UT_EXPECT_TRUE(!test.DeleteEdge(euids[i]));
        }
        test.CommitTxn();
    }
    UT_LOG() << "Update edge";
    {
        test.BeginWriteTxn();
        std::vector<EdgeUid> euids = test.GetEdges();
        std::shuffle(euids.begin(), euids.end(), rg);
        for (size_t i = 0; i < percent_update * euids.size() / 100; i++) {
            size_t psize = myrand() % 100 <= percent_huge ? myrand() % (1 << 12) : myrand() % 10;
            UT_EXPECT_TRUE(test.UpdateEdge(euids[i], psize));
        }
        test.CommitTxn();
    }
    UT_LOG() << "Monkey test";
    {
        test.BeginWriteTxn();
        for (size_t m = 0; m < n_monkey; m++) {
            size_t psize = myrand() % 100 <= percent_huge ? myrand() % (1 << 12) : myrand() % 10;
            int64_t vid;
            int64_t dst;
            EdgeUid euid;
            switch (myrand() % 6) {
            case 0:
                // add vertex
                test.AddVertex(vlabels[myrand() % nvlabel], psize);
                break;
            case 1:
                // add edge
                vid = test.RandomV();
                dst = test.RandomV();
                if (vid == -1 || dst == -1) break;
                test.AddEdge(vid, dst, elabels[myrand() % nvlabel], psize);
                break;
            case 2:
                // del vertex
                vid = test.RandomV();
                UT_EXPECT_TRUE(vid == -1 || test.DeleteVertex(vid));
                break;
            case 3:
                // del edge
                euid = test.RandomE();
                UT_EXPECT_TRUE(euid.src == -1 || test.DeleteEdge(euid));
                break;
            case 4:
                // update vertex
                vid = test.RandomV();
                UT_EXPECT_TRUE(vid == -1 || test.UpdateVertex(vid, psize));
                break;
            case 5:
                // update edge
                euid = test.RandomE();
                UT_EXPECT_TRUE(euid.src == -1 || test.UpdateEdge(euid, psize));
                break;
            }
        }
        test.CommitTxn();
    }
    test.Verify();
}
INSTANTIATE_TEST_CASE_P(TestLGraphMonkey, TestLGraphMonkey,
                        testing::Values(ParamMonkey(1000, 20),
                                        ParamMonkey(10, 100, 2, 2, 30, 30, 5000)));
