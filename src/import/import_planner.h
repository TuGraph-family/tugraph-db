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

#include <map>
#include <vector>

#include "fma-common/string_formatter.h"
#include "fma-common/assert.h"

namespace lgraph {
namespace import {
class PlanExecutor;

class LabelGraph {
 private:
    std::vector<std::vector<size_t>> efile_size_;
    std::vector<size_t> vfile_size_;
    size_t nv_;

 public:
    explicit LabelGraph(size_t nv) : nv_(nv) {
        efile_size_.resize(nv_);
        for (auto& es : efile_size_) es.resize(nv_);
        vfile_size_.resize(nv_);
    }

    void AddVFileSize(size_t vid, size_t size) { vfile_size_[vid] += size; }

    void AddEFileSize(size_t src, size_t dst, size_t size) {
        efile_size_[src][dst] += size;
        efile_size_[dst][src] += size;
    }

    const std::vector<std::vector<size_t>>& GetEFileSize() const { return efile_size_; }

    const std::vector<size_t>& GetVFileSize() const { return vfile_size_; }

    std::vector<size_t> CalculateBestPlan(size_t& max_memory) {
        // construct adjacency list
        std::vector<std::vector<size_t>> edges(nv_);
        for (size_t i = 0; i < nv_; i++) {
            for (size_t j = 0; j < nv_; j++) {
                if (i != j && efile_size_[i][j]) edges[i].push_back(j);
            }
        }
        // get plan
        std::vector<bool> vread(nv_, false);
        std::vector<bool> vdump(nv_, false);
        std::vector<size_t> plan;
        max_memory = 0;
        size_t memory = 0;
        while (plan.size() != nv_) {
            // get a connected component
            std::vector<size_t> cc;
            {
                size_t max_vsize = 0;
                size_t v = 0;
                for (size_t i = 0; i < nv_; i++) {
                    if (!vdump[i] && vfile_size_[i] > max_vsize) {
                        max_vsize = vfile_size_[i];
                        v = i;
                    }
                }
                cc = GetCC(edges, v);
            }
            // get the plan for this CC
            std::deque<size_t> to_dump;
            size_t max_vsize = 0;
            size_t firstv = 0;
            for (auto v : cc) {
                if (vfile_size_[v] > max_vsize) {
                    max_vsize = vfile_size_[v];
                    firstv = v;
                }
            }
            to_dump.push_back(firstv);
            memory += vfile_size_[firstv];
            vread[firstv] = true;
            while (!to_dump.empty()) {
                // read v
                size_t v = to_dump.front();
                // sort neighbours
                std::vector<std::pair<size_t, size_t>> vsize;  // [(vsize, vid)]
                for (auto n : edges[v]) {
                    if (!vread[n]) {
                        vsize.emplace_back(vfile_size_[n], n);
                    }
                }
                std::sort(vsize.begin(), vsize.end(), std::greater<std::pair<size_t, size_t>>());
                // load neighbours one by one
                for (auto p : vsize) {
                    memory += p.first;
                    to_dump.push_back(p.second);
                    vread[p.second] = true;
                }
                // calculate max memory
                max_memory = std::max<size_t>(max_memory, memory);
                // dump v
                to_dump.pop_front();
                memory -= vfile_size_[v];
                vdump[v] = true;
                plan.push_back(v);
            }
        }
        return plan;
    }

    std::string ToString() const {
        std::string ret;
        ret.append("  |\t");
        for (size_t i = 0; i < nv_; i++) {
            ret.append(fma_common::ToString(vfile_size_[i]));
            ret.append("|\t");
        }
        ret.append("\n");
        for (size_t i = 0; i < nv_; i++) {
            ret.append(fma_common::ToString(vfile_size_[i]));
            ret.append("|\t");
            for (size_t j = 0; j < nv_; j++) {
                ret.append(fma_common::ToString(efile_size_[i][j]));
                ret.append("|\t");
            }
            ret.append("\n");
        }
        return ret;
    }

 private:
    std::vector<size_t> GetCC(const std::vector<std::vector<size_t>>& edges,
                              size_t start_vid) const {
        std::unordered_set<size_t> cc;
        std::deque<size_t> to_visit;
        to_visit.push_back(start_vid);
        cc.insert(start_vid);
        while (!to_visit.empty()) {
            size_t v = to_visit.front();
            to_visit.pop_front();
            for (auto& n : edges[v]) {
                if (cc.find(n) == cc.end()) {
                    to_visit.push_back(n);
                    cc.insert(n);
                }
            }
        }
        return std::vector<size_t>(cc.begin(), cc.end());
    }
};

class PlanExecutor {
 public:
    struct Action {
        enum Type { LOAD_VERTEX, LOAD_EDGE, DUMP_VERTEX, DONE };

        Type type;
        size_t vid;
        std::pair<size_t, size_t> edge;

        static Action LoadVertex(size_t vid) {
            Action ret;
            ret.type = LOAD_VERTEX;
            ret.vid = vid;
            return ret;
        }

        static Action LoadEdge(size_t src, size_t dst) {
            Action ret;
            ret.type = LOAD_EDGE;
            ret.edge.first = src;
            ret.edge.second = dst;
            return ret;
        }

        static Action DumpVertex(size_t vid) {
            Action ret;
            ret.type = DUMP_VERTEX;
            ret.vid = vid;
            return ret;
        }

        static Action Done() {
            Action ret;
            ret.type = DONE;
            return ret;
        }

        std::string ToString() const {
            switch (type) {
            case LOAD_VERTEX:
                return fma_common::StringFormatter::Format("LoadVertex({})", vid);
            case LOAD_EDGE:
                return fma_common::StringFormatter::Format("LoadEdge({}, {})", edge.first,
                                                           edge.second);
            case DUMP_VERTEX:
                return fma_common::StringFormatter::Format("DumpVertex({})", vid);
            case DONE:
                return "Done";
            }
            return "INVALID";
        }
    };

 private:
    size_t nv_;
    std::vector<bool> vertex_loaded_;
    std::vector<std::vector<bool>> edge_loaded_;
    size_t vertex_order_num_ = 0;
    std::vector<size_t> load_order_;
    std::vector<bool> vertex_dumpped_;
    std::vector<size_t> plan_;

    std::vector<Action> actions_;
    size_t curr_action_ = 0;

 public:
    PlanExecutor(const std::vector<size_t>& plan, const std::vector<std::vector<size_t>>& edges) {
        plan_ = plan;
        nv_ = plan.size();
        vertex_loaded_.resize(nv_, false);
        vertex_dumpped_.resize(nv_, false);
        edge_loaded_.resize(nv_);
        load_order_.resize(nv_, std::numeric_limits<size_t>::max());
        for (auto& e : edge_loaded_) e.resize(nv_, true);
        for (size_t src = 0; src < nv_; src++) {
            for (size_t dst = 0; dst < nv_; dst++) {
                if (edges[src][dst] != 0) edge_loaded_[src][dst] = false;
            }
        }
        // construct the actions
        for (size_t i = 0; i < plan.size(); i++) {
            DoVertex(plan[i]);
        }
    }

    Action GetNextAction() {
        if (curr_action_ < actions_.size()) return actions_[curr_action_++];
        return Action::Done();
    }

    const std::vector<Action>& GetActions() const { return actions_; }

    std::string ToString() const {
        std::string ret;
        for (auto& act : actions_) {
            switch (act.type) {
            case Action::LOAD_VERTEX:
                fma_common::StringFormatter::Append(ret, "L {} -> ", act.vid);
                break;
            case Action::LOAD_EDGE:
                fma_common::StringFormatter::Append(ret, "L ({},{}) -> ", act.edge.first,
                                                    act.edge.second);
                break;
            case Action::DUMP_VERTEX:
                fma_common::StringFormatter::Append(ret, "D {} -> ", act.vid);
                break;
            case Action::DONE:
                break;
            }
        }
        ret.append("DONE");
        return ret;
    }

 private:
    void LoadVertex(size_t vid) {
        if (vertex_loaded_[vid]) return;
        vertex_loaded_[vid] = true;
        load_order_[vid] = vertex_order_num_++;
        actions_.push_back(Action::LoadVertex(vid));
    }

    void LoadEdge(size_t src, size_t dst) {
        if (edge_loaded_[src][dst]) return;
        edge_loaded_[src][dst] = true;
        actions_.push_back(Action::LoadEdge(src, dst));
    }

    /**
     * Read necessary data for vertex vid and dump it.
     *
     * \param vid   The vid.
     */
    void DoVertex(size_t vid) {
        if (vertex_dumpped_[vid]) {
            return;
        }
        LoadVertex(vid);
        // load depending vertexes
        for (size_t i = 0; i < nv_; i++) {
            size_t vv = plan_[i];
            if (!edge_loaded_[vv][vid]) {
                LoadVertex(vv);
                LoadEdge(vv, vid);
            }
            if (!edge_loaded_[vid][vv]) {
                LoadVertex(vv);
                LoadEdge(vid, vv);
            }
        }
        // validate that all previously loaded vertex has been dumped
        for (size_t i = 0; i < nv_; i++) {
            if (i != vid) {
                FMA_DBG_ASSERT(vertex_dumpped_[i] || !vertex_loaded_[i] ||
                               load_order_[i] > load_order_[vid]);
            }
        }
        // dump vertex
        actions_.push_back(Action::DumpVertex(vid));
        vertex_dumpped_[vid] = true;
    }
};
}  // namespace import
}  // namespace lgraph
