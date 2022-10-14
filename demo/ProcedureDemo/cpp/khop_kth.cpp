/*
 * 根据给定顶点，返回第k层的顶点个数
 */
#include "lgraph/lgraph.h"
#include "lgraph/olap_on_db.h"
#include "lgraph/lgraph_traversal.h"

#include "json.hpp"

#include <iostream>
#include <vector>
#include <unordered_set>
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
            std::shared_ptr<olap::ParallelBitset> ptr_(new olap::ParallelBitset(parallel_bitset_size_));
            parallel_bitset_visited_ = ptr_;
            for(auto iter = unordered_set_visited_.begin(); iter != unordered_set_visited_.end(); ++iter) {
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

extern "C" bool Process(GraphDB & db, const std::string & request, std::string & response) {
    int64_t root;
    size_t depth;
    std::string label;
    std::string field;
    size_t threshold_size = 10000;
    bool has_duplicate_edge = false;
    bool multi_threads = true;
    try {
        json input = json::parse(request);
        root = input["root"].get<int64_t>();
        depth = input["depth"].get<size_t>();
        label = input["label"].get<std::string>();
        field = input["field"].get<std::string>();
        if (input["threshold_size"].is_number()) {
            threshold_size = input["threshold_size"].get<size_t>();
        }
        if (input["has_duplicate_edge"].is_boolean()) {
            has_duplicate_edge = input["has_duplicate_edge"].get<bool>();
        }
        if (input["multi_threads"].is_boolean()) {
            multi_threads = input["multi_threads"].get<bool>();
        }
    } catch (std::exception & e) {
        std::cout << e.what() << std::endl;
        std::cout << "request: " << request << std::endl;
        throw std::runtime_error(e.what());
    }

    size_t size = 0;
    auto txn = db.CreateReadTxn();
    size_t vertex_label_id = txn.GetVertexLabelId(label);
    size_t vertex_field_id = txn.GetVertexFieldId(vertex_label_id, field);

    auto root_iter =
        txn.GetVertexIndexIterator(vertex_label_id, vertex_field_id, FieldData(root), FieldData(root));
    int64_t vertex_id;
    if (root_iter.IsValid()) {
        vertex_id = root_iter.GetVid();
    } else {
        json output;
        output["size"] = 0;
        response = output.dump();
        return true;
    }

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
                    if ((i == 0 && !has_duplicate_edge) || (!visited.Has(dst) && visited.Add(dst))) {
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
    json output;
    output["size"] = size;
    response = output.dump();
    return true;
}
