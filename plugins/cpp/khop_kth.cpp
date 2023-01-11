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

/*
 * Returns the number of vertices in the k-th layer
 * according to the given vertex.
 */
#include <iostream>
#include <vector>
#include <unordered_set>

#include "lgraph/lgraph.h"
#include "lgraph/lgraph_traversal.h"
#include "tools/json.hpp"

using json = nlohmann::json;
using namespace lgraph_api;

class UnorderedParallelBitset {
 public:
    size_t size_;
    size_t parallel_bitset_size_;
    size_t threshold_size_;
    bool use_unordered_set_;
    std::shared_ptr<olap::ParallelBitset> parallel_bitset_visited_;
    std::unordered_set<int64_t> unordered_set_visited_;

    UnorderedParallelBitset(size_t parallel_bitset_size, size_t threshold_size) {
        size_ = 0;
        parallel_bitset_size_ = parallel_bitset_size;
        threshold_size_ = threshold_size;
        use_unordered_set_ = true;
    }

    ~UnorderedParallelBitset() {}

    bool Has(int64_t vid) {
        if (use_unordered_set_) {
            return unordered_set_visited_.find(vid) != unordered_set_visited_.end();
        } else {
            return parallel_bitset_visited_->Has(vid);
        }
    }

    bool Add(int64_t vid) {
        if (use_unordered_set_ && size_ >= threshold_size_) {
            use_unordered_set_ = false;
            std::shared_ptr<olap::ParallelBitset> ptr_(
                new olap::ParallelBitset(parallel_bitset_size_));
            parallel_bitset_visited_ = ptr_;
            for (auto iter = unordered_set_visited_.begin(); iter != unordered_set_visited_.end();
                 ++iter) {
                parallel_bitset_visited_->Add(*iter);
            }
        }
        if (use_unordered_set_) {
            unordered_set_visited_.emplace(vid);
        } else {
            parallel_bitset_visited_->Add(vid);
        }
        size_ += 1;
        return true;
    }

    void Clear() {
        if (use_unordered_set_) {
            unordered_set_visited_.clear();
        } else {
            parallel_bitset_visited_->Clear();
        }
        size_ = 0;
    }
};

extern "C" bool Process(GraphDB& db, const std::string& request, std::string& response) {
    double start_time;

    // prepare
    start_time = get_time();
    std::string root = "0";
    size_t depth = 3;
    std::string label = "node";
    std::string field = "id";
    size_t threshold_size = 10000;
    bool has_duplicate_edge = false;
    bool multi_threads = true;
    std::cout << "Input: " << request << std::endl;
    try {
        json input = json::parse(request);
        parse_from_json(root, "root", input);
        parse_from_json(depth, "depth", input);
        parse_from_json(label, "label", input);
        parse_from_json(field, "field", input);
        parse_from_json(threshold_size, "threshold_size", input);
        parse_from_json(has_duplicate_edge, "has_duplicate_edge", input);
        parse_from_json(multi_threads, "multi_threads", input);
    } catch (std::exception& e) {
        response = "json parse error: " + std::string(e.what());
        std::cout << response << std::endl;
        return false;
    }

    size_t size = 0;
    auto txn = db.CreateReadTxn();

    auto root_iter = txn.GetVertexIndexIterator(label, field, root, root);
    int64_t vertex_id;
    if (root_iter.IsValid()) {
        vertex_id = root_iter.GetVid();
    } else {
        json output;
        output["size"] = 0;
        response = output.dump();
        return true;
    }
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    if (multi_threads) {
        auto frontierTraversal = traversal::FrontierTraversal(db, txn, 1);
        frontierTraversal.SetFrontier(vertex_id);
        for (size_t i = 0; i < depth; ++i) {
            frontierTraversal.ResetVisited();
            frontierTraversal.ExpandOutEdges();
        }
        auto& pv = frontierTraversal.GetFrontier();
        size = pv.Size();
    } else {
        size_t num_vertices = txn.GetNumVertices();
        std::vector<int64_t> src_vertexs;
        src_vertexs.push_back(vertex_id);
        std::vector<int64_t> dst_vertexs;
        auto vit = txn.GetVertexIterator();
        UnorderedParallelBitset visited(num_vertices, threshold_size);
        for (size_t i = 0; i < depth; ++i) {
            for (auto vid : src_vertexs) {
                vit.Goto(vid);
                for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                    int64_t dst = eit.GetDst();
                    if ((i == 0 && !has_duplicate_edge) ||
                        (!visited.Has(dst) && visited.Add(dst))) {
                        dst_vertexs.push_back(dst);
                    }
                }
            }
            std::swap(src_vertexs, dst_vertexs);
            dst_vertexs.clear();
            visited.Clear();
        }
        size = src_vertexs.size();
    }
    auto core_cost = get_time() - start_time;

    json output;
    output["size"] = size;
    output["prepare_cost"] = prepare_cost;
    output["core_cost"] = core_cost;
    output["total_cost"] = prepare_cost + core_cost;
    response = output.dump();
    return true;
}
