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

#pragma once

#include <stdlib.h>
#include <random>
#include <string>

#include "core/data_type.h"
#include "lgraph/lgraph.h"
#include "./random_port.h"
#include "./test_tools.h"

static inline std::string RandomString(size_t n) {
    std::string str(n, 0);
    for (size_t i = 0; i < n; i++) str[i] = (myrand() % 26) + 'a';
    return str;
}

static inline std::map<std::string, lgraph_api::FieldData> GenProperties(
    const std::string& label,
    const std::map<std::string, std::map<std::string, lgraph_api::FieldSpec>>& schema,
    size_t size) {
    auto it = schema.find(label);
    assert(it != schema.end());
    std::map<std::string, lgraph_api::FieldData> props;
    for (auto& fkv : it->second) {
        const lgraph_api::FieldSpec& fs = fkv.second;
        if (fs.type == lgraph_api::FieldType::STRING) {
            props.emplace(fs.name, lgraph_api::FieldData(RandomString(size)));
        } else {
            props.emplace(fs.name, lgraph_api::FieldData(myrand()));
        }
    }
    return props;
}

class InMemoryGraph {
    typedef int64_t VertexId;
    typedef int64_t EdgeId;
    typedef uint16_t LabelId;
    typedef lgraph_api::FieldData FieldData;
    typedef lgraph_api::EdgeUid EdgeUid;
    typedef lgraph_api::FieldSpec FieldSpec;
    typedef lgraph_api::FieldType FieldType;
    typedef lgraph_api::Transaction Transaction;

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

    struct Schema {
        std::map<std::string, std::map<std::string, FieldSpec>> schema_defs;
        std::map<std::string, int64_t> label_to_id;
        std::vector<std::string> id_to_label;

        bool AddLabel(const std::string& str, const std::map<std::string, FieldSpec>& defs) {
            auto it = label_to_id.find(str);
            if (it != label_to_id.end()) return false;
            int64_t next_lid = label_to_id.empty() ? 0 : label_to_id.rbegin()->second;
            label_to_id.emplace(str, next_lid + 1);
            id_to_label.push_back(str);
            schema_defs[str] = defs;
            return true;
        }

        int64_t GetLabelId(const std::string& label) {
            auto it = label_to_id.find(label);
            return it == label_to_id.end() ? -1 : it->second;
        }

        std::string GetLabel(int64_t label_id) {
            return label_id < (int64_t)id_to_label.size() ? id_to_label[label_id] : "";
        }
    };

    typedef std::pair<std::vector<std::string>, std::vector<FieldData>> KeysAndValues;
    typedef std::map<VertexId, Vertex> Graph;

    Graph vgraph_;  // in-memory graph for verification
    Schema vschema_;
    Schema eschema_;

 private:
    bool AddLabel(bool is_vertex, Schema& schema, const std::string& label,
                  const std::vector<FieldSpec>& fds) {
        std::map<std::string, FieldSpec> tmp;
        for (auto& f : fds) tmp.emplace(f.name, f);
        return is_vertex ? vschema_.AddLabel(label, tmp) : eschema_.AddLabel(label, tmp);
    }

    void VerifySchema(lgraph_api::Transaction& txn, bool is_vertex, const Schema& schema) {
        auto labels = is_vertex ? txn.ListVertexLabels() : txn.ListEdgeLabels();
        UT_EXPECT_EQ(labels.size(), schema.schema_defs.size());
        for (auto& label : labels) {
            auto sit = schema.schema_defs.find(label);
            UT_ASSERT(sit != schema.schema_defs.end());
            auto fds = is_vertex ? txn.GetVertexSchema(label) : txn.GetEdgeSchema(label);
            std::map<std::string, FieldSpec> schema;
            for (auto& fd : fds) schema.emplace(fd.name, fd);
            UT_ASSERT(schema == sit->second);
        }
    }

 public:
    InMemoryGraph() {}

    bool AddVertexLabel(const std::string& label, const std::vector<FieldSpec>& fds) {
        return AddLabel(true, vschema_, label, fds);
    }

    bool AddEdgeLabel(const std::string& label, const std::vector<FieldSpec>& fds) {
        return AddLabel(false, eschema_, label, fds);
    }

    VertexId AddVertex(const std::string& label, const Properties& props) {
        VertexId vid = vgraph_.empty() ? 0 : vgraph_.rbegin()->first + 1;
        vgraph_.emplace_hint(vgraph_.end(), vid, Vertex(label, props));
        return vid;
    }

    bool UpdateVertex(VertexId vid, const Properties& props) {
        auto vgit = vgraph_.find(vid);
        if (vgit == vgraph_.end()) return false;
        vgit->second.props = props;
        return true;
    }

    std::vector<VertexId> GetVids() const {
        std::vector<VertexId> vids;
        vids.reserve(vgraph_.size());
        for (auto& kv : vgraph_) vids.emplace_back(kv.first);
        return vids;
    }

    std::vector<EdgeUid> GetEdges() const {
        std::vector<EdgeUid> euids;
        for (auto& v : vgraph_) {
            for (auto& e : v.second.outs) {
                euids.push_back(e.first);
            }
        }
        return euids;
    }

    VertexId RandomV() const {
        if (vgraph_.empty()) return -1;
        int64_t max_vid = vgraph_.rbegin()->first;
        return vgraph_.lower_bound(myrand() % max_vid)->first;
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

    EdgeUid AddEdge(VertexId src, VertexId dst, const std::string& label, const Properties& props) {
        auto vgit = vgraph_.find(src);
        if (vgit == vgraph_.end() || vgraph_.find(dst) == vgraph_.end())
            THROW_CODE(InputError, "no such source");
        auto lid = eschema_.GetLabelId(label);
        auto& out_edges = vgit->second.outs;
        EdgeId eid;
        if (out_edges.empty()) {
            eid = 0;
        } else {
            auto eit = out_edges.lower_bound(EdgeUid(src, dst, (uint16_t)lid + 1, 0, 0));
            if (eit == out_edges.end()) {
                auto rb = out_edges.rbegin();
                if (rb->first.dst == dst && rb->first.lid == lid) {
                    eid = rb->first.eid + 1;
                } else {
                    eid = 0;
                }
            } else {
                eit--;
                if (eit->first.dst == dst && eit->first.lid == lid) {
                    eid = eit->first.eid + 1;
                } else {
                    eid = 0;
                }
            }
        }
        EdgeUid euid(src, dst, (uint16_t)lid, 0, eid);
        out_edges.emplace(euid, Edge(label, props));
        vgit = vgraph_.find(dst);
        assert(vgit != vgraph_.end());
        assert(vgit->second.ins.find(euid) == vgit->second.ins.end());
        vgit->second.ins.emplace(euid, Edge(label, props));
        return euid;
    }

    bool UpdateEdge(const EdgeUid& euid, const Properties& props) {
        auto vgit_src = vgraph_.find(euid.src);
        auto vgit_dst = vgraph_.find(euid.dst);
        if (vgit_src == vgraph_.end() || vgit_dst == vgraph_.end()) return false;
        auto vgoeit = vgit_src->second.outs.find(euid);
        if (vgoeit == vgit_src->second.outs.end()) return false;
        auto vgieit = vgit_dst->second.ins.find(euid);
        vgoeit->second.props = props;
        vgieit->second.props = props;
        return true;
    }

    bool DeleteVertex(VertexId vid) {
        auto vgit = vgraph_.find(vid);
        if (vgit == vgraph_.end()) return false;
        vgit->second.ins.size();
        vgit->second.outs.size();
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
        auto vgit_src = vgraph_.find(euid.src);
        auto vgit_dst = vgraph_.find(euid.dst);
        if (vgit_src == vgraph_.end() || vgit_dst == vgraph_.end()) return false;
        auto vgoeit = vgit_src->second.outs.find(euid);
        if (vgoeit == vgit_src->second.outs.end()) return false;
        auto vgieit = vgit_dst->second.ins.find(euid);
        vgit_src->second.outs.erase(vgoeit);
        vgit_dst->second.ins.erase(vgieit);
        return true;
    }

    // verify that db has exactly the same content with vgraph
    void Verify(lgraph_api::GraphDB* db) {
        auto txn = db->CreateReadTxn();
        // check schemas
        VerifySchema(txn, true, vschema_);
        VerifySchema(txn, false, eschema_);
        // check vertex and edges
#if 0
        auto vgit = vgraph_.begin();
        for (auto dbit = txn.GetVertexIterator(); dbit.IsValid();
            dbit.Next(), vgit++) {
            UT_ASSERT(vgit != vgraph_.end());
            auto vid = vgit->first;
            UT_CHECK_EQ(dbit.GetId(), vgit->first);
            UT_CHECK_EQ(dbit.GetLabel(), vgit->second.label);
            auto fvs = dbit.GetAllFields();
            UT_ASSERT(fvs == vgit->second.props);
            // check in-edges
            UT_CHECK_EQ(dbit.GetNumInEdges(), vgit->second.ins.size());
            auto vgieit = vgit->second.ins.begin();
            for (auto ieit = dbit.GetInEdgeIterator(); ieit.IsValid();
                ieit.Next(), vgieit++) {
                UT_ASSERT(vgieit != vgit->second.ins.end());
                UT_ASSERT(vgieit->first == ieit.GetUid());           // check EdgeId
                UT_CHECK_EQ(vgieit->second.label, ieit.GetLabel());  // label
                auto t = ieit.GetAllFields();
                UT_ASSERT(vgieit->second.props == t);  // props
            }
            // check out-edges
            auto vgoeit = vgit->second.outs.begin();
            for (auto oeit = dbit.GetOutEdgeIterator(); oeit.IsValid();
                oeit.Next(), vgoeit++) {
                UT_ASSERT(vgoeit != vgit->second.outs.end());
                UT_ASSERT(vgoeit->first == oeit.GetUid());           // check EdgeId
                UT_CHECK_EQ(vgoeit->second.label, oeit.GetLabel());  // label
                UT_ASSERT(vgoeit->second.props == oeit.GetAllFields());  // props
            }
        }
#else
        size_t nv = vgraph_.size();
        size_t ne = 0;
        for (auto vgit = vgraph_.begin(); vgit != vgraph_.end(); vgit++) {
            ne += vgit->second.outs.size();
        }
        size_t nv_validate = 0;
        size_t ne_validate = 0;
        for (auto vit = txn.GetVertexIterator(); vit.IsValid(); vit.Next()) {
            nv_validate++;
            for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) ne_validate++;
        }
        UT_EXPECT_EQ(nv, nv_validate);
        UT_EXPECT_EQ(ne, ne_validate);
#endif
    }
};

// checks that two graphs are equal
static void CheckGraphEqual(const std::string& db1, const std::string& graph1,
                            const std::string& user1, const std::string& pass1,
                            const std::string& db2, const std::string& graph2,
                            const std::string& user2, const std::string& pass2) {
    InMemoryGraph img;
    {
        lgraph_api::Galaxy g(db1, true, false);
        g.SetCurrentUser(user1, pass1);
        auto graph = g.OpenGraph(graph1, true);
        auto txn = graph.CreateReadTxn();
        for (auto& label : txn.ListVertexLabels()) {
            img.AddVertexLabel(label, txn.GetVertexSchema(label));
        }
        for (auto& label : txn.ListEdgeLabels()) {
            img.AddEdgeLabel(label, txn.GetEdgeSchema(label));
        }
        // copy vertices
        for (auto vit = txn.GetVertexIterator(); vit.IsValid(); vit.Next()) {
            img.AddVertex(vit.GetLabel(), vit.GetAllFields());
        }
        // copy edges
        for (auto vit = txn.GetVertexIterator(); vit.IsValid(); vit.Next()) {
            for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                img.AddEdge(eit.GetSrc(), eit.GetDst(), eit.GetLabel(), eit.GetAllFields());
            }
        }
    }
    // now check with graph2
    lgraph_api::Galaxy g(db2, false, false);
    g.SetCurrentUser(user2, pass2);
    auto graph = g.OpenGraph(graph2, true);
    img.Verify(&graph);
}

class GraphGen {
    uint64_t seed[2];

 public:
    void GenSchema(
        size_t n_fields,
        const std::function<void(const std::string& label,
                                 const std::vector<lgraph_api::FieldSpec>& fds)>& add_label_func) {
        using lgraph_api::FieldSpec;
        using lgraph_api::FieldType;
        std::unordered_set<std::string> fields;
        while (fields.size() < n_fields) {
            fields.emplace(RandomString(10));
        }
        std::vector<lgraph_api::FieldSpec> fs;
        for (auto& fn : fields) {
            FieldSpec f;
            f.name = fn;
            f.optional = true;
            f.type = FieldType::INT64;
            fs.push_back(f);
        }
        if (n_fields >= 1) fs[0].type = FieldType::STRING;
        if (n_fields >= 2) fs[1].type = FieldType::STRING;
        add_label_func(RandomString(10), fs);
    }

    void GenVertexes() {}

    void GenEdges(
        size_t nv, size_t epv, int percent_huge, const std::vector<std::string>& elabels,
        const std::map<std::string, std::map<std::string, lgraph_api::FieldSpec>> schema,
        const std::function<void(int64_t src, int64_t dst, const std::string& label,
                                 const std::map<std::string, lgraph_api::FieldData>& props)>&
            add_edge_func) {
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
            const std::string& label = elabels[myrand() % elabels.size()];
            add_edge_func(src, dst, label, GenProperties(label, schema, psize));
        }
    }

 private:
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
};
