/**
* Copyright 2024 AntGroup CO., Ltd.
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

#include <gtest/gtest.h>
#include <filesystem>
#include <utility>
#include "common/logger.h"
#include "common/value.h"
#include "graphdb/graph_db.h"
#include "transaction/transaction.h"
namespace fs = std::filesystem;
static std::string testdb = "testdb";
using namespace graphdb;

static inline double GetTime() {
    using namespace std::chrono;
    high_resolution_clock::duration tp = high_resolution_clock::now().time_since_epoch();
    return (double)tp.count() * high_resolution_clock::period::num /
           high_resolution_clock::period::den;
}

class RandomNumberGenerator {
    uint64_t s[2]{};

   public:
    explicit RandomNumberGenerator(uint64_t seed = 0) {
        s[0] = seed;
        s[1] = seed + 1;
    }
    uint64_t next() {
        uint64_t x = s[0];
        uint64_t y = s[1];
        s[0] = y;
        x ^= (x << 23);
        s[1] = x ^ y ^ (x >> 17) ^ (y >> 26);
        return s[1] + y;
    }
};

class BenchmarkLightningGraph {
    const std::string chars = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789";

    GraphDB& db;
    size_t vertices;
    RandomNumberGenerator rng;

    std::string encode_no(size_t no) {
        std::string no_s = std::to_string(no);
        while (no_s.size() < 10) {
            no_s = "0" + no_s;
        }
        return no_s;
    }
    std::string random_no() { return encode_no(rng.next() % vertices); }
    std::string random_name() {
        std::string name;
        for (int i = 0; i < 10; i++) {
            name += chars[rng.next() % chars.size()];
        }
        return name;
    }
    void write_vertex(std::string no, std::string name) {
        auto txn = db.BeginTransaction();
        auto vid =
            txn->CreateVertex({"Person"},{{"no", Value::String(std::move(no))},{"name", Value::String(std::move(name))}});
        txn->Commit();
    }
    void write_edge(std::string no_from, std::string no_to) {
        auto txn = db.BeginTransaction();
        auto viter = txn->NewVertexIterator("Person",
                                            std::unordered_map<std::string, Value>{{"no", Value::String(std::move(no_from))}});
        EXPECT_TRUE(dynamic_cast<GetVertexByUniqueIndex*>(viter.get()));
        EXPECT_TRUE(viter->Valid());
        auto vertex_from = viter->GetVertex();
        viter = txn->NewVertexIterator("Person",
                                       std::unordered_map<std::string, Value>{{"no", Value::String(std::move(no_to))}});
        EXPECT_TRUE(dynamic_cast<GetVertexByUniqueIndex*>(viter.get()));
        EXPECT_TRUE(viter->Valid());
        auto vertex_to = viter->GetVertex();
        txn->CreateEdge(vertex_from, vertex_to, "Knows", {});
        txn->Commit();
    }
    size_t read_neighbour(std::string no, size_t depth = 1) {
        auto txn = db.BeginTransaction();
        auto viter = txn->NewVertexIterator("Person",
                                            std::unordered_map<std::string, Value>{{"no", Value::String(std::move(no))}});
        std::vector<int64_t> src_vertexs;
        std::vector<int64_t> dst_vertexs;
        src_vertexs.push_back(viter->GetVertex().GetId());
        std::unordered_set<int64_t> visited;
        auto types = std::unordered_set<std::string>{"Knows"};
        std::unordered_set<uint32_t> type_set;
        for (auto &type : types) {
            auto tid = txn->db()->id_generator().GetTid(type);
            if (tid) {
                type_set.insert(tid.value());
            }
        }
        for (size_t i = 0; i < depth; ++i) {
            for (auto vid : src_vertexs) {
                auto eit = std::make_unique<graphdb::ScanEdgeByVidDirectionTypes>(
                    txn.get(), vid, graphdb::EdgeDirection::BOTH,
                    type_set);
                while (eit->Valid()) {
                    int64_t dst = eit->GetEdge().GetOtherEnd(vid).GetId();
                    if (!visited.count(dst)) {
                        visited.insert(dst);
                        dst_vertexs.push_back(dst);
                    }
                    eit->Next();
                }
            }
            std::swap(src_vertexs, dst_vertexs);
            dst_vertexs.clear();
            visited.clear();
        }
        auto size = src_vertexs.size();
        txn->Commit();
        return size;
    }

   public:
    BenchmarkLightningGraph(GraphDB& db, size_t vertices) : db(db), vertices(vertices) {}
    double test_write_vertex(size_t count) {
        double time_start = GetTime();
        for (size_t i = 0; i < count; i++) {
            std::string no = encode_no(vertices++);
            std::string name = random_name();
            write_vertex(no, name);
        }
        return GetTime() - time_start;
    }
    double test_write_edge(size_t count) {
        double time_start = GetTime();
        for (size_t i = 0; i < count; i++) {
            std::string no_from = random_no();
            std::string no_to = random_no();
            write_edge(no_from, no_to);
        }
        return GetTime() - time_start;
    }
    double test_read_neighbour(size_t count, int depth) {
        double time_start = GetTime();
        size_t checksum = 0;
        for (size_t i = 0; i < count; i++) {
            std::string no = random_no();
            checksum += read_neighbour(no, depth);
        }
        std::cout << "checksum: " << checksum << std::endl;
        return GetTime() - time_start;
    }
};

TEST(Benchmark, khop) {
    size_t n = 10000;
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    graphDB->AddVertexPropertyIndex("person_id", true, "Person", "no");
    BenchmarkLightningGraph bm(*graphDB, 0);

    LOG_INFO("The time of {} vertexes insertion is {}s", n, bm.test_write_vertex(n));
    LOG_INFO("The time of {} edges insertion is {}s", n * 10, bm.test_write_edge(n * 10));
    LOG_INFO("The time of {} vertexes 1-hop is {}s", n, bm.test_read_neighbour(n, 1));
    LOG_INFO("The time of {} vertexes 2-hop is {}s", n, bm.test_read_neighbour(n, 2));
    LOG_INFO("The time of {} vertexes 3-hop is {}s", n, bm.test_read_neighbour(n, 3));
}