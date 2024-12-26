//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/**
 *  @file   olap_on_db.h
 *  @brief  TuGraph OLAP interface. To implement a plugin that perform graph analytics on TuGraph,
 * user can load a Snapshot from the database, and then use the Gather-Apply-Scatter style interface
 *          to do the computation.
 */

#pragma once

#include <exception>
#include "lgraph/lgraph.h"
#include "lgraph/lgraph_txn.h"
#include "lgraph/lgraph_utils.h"
#include "lgraph/olap_base.h"
#include "fma-common/fma_stream.h"

namespace lgraph_api {
namespace olap {

/**
 * The available options for (graph construction) flags.
 */
static constexpr size_t SNAPSHOT_PARALLEL = 1ul << 0;
// Enforce parallelism when generating a graph (which consumes more memory)
static constexpr size_t SNAPSHOT_UNDIRECTED = 1ul << 1;
// Make the generated graph undirected (identical out edges and in edges)

// Todo:
// Add flag of id_mapping to define whether to do id_mapping or skip
static constexpr size_t SNAPSHOT_IDMAPPING = 1ul << 2;
// Do id_mapping for generated graph from input database (continuous vertex id)

/**
 * The following options are not implemented yet.
 */
static constexpr size_t SNAPSHOT_OUT_DEGREE = 1ul << 3;
// Generate the out_degree array
static constexpr size_t SNAPSHOT_IN_DEGREE = 1ul << 4;
// Generate the in_degree array
static constexpr size_t SNAPSHOT_OUT_EDGES = 1ul << 5;
// Generate the out edges
static constexpr size_t SNAPSHOT_IN_EDGES = 1ul << 6;
// Generate the in edges

/**
 * @brief   Snapshot is a derived class of Graph.
 *          Snapshot instances represent static (sub)graphs exported from
 *          LightningGraph. The internal organization uses compressed sparse
 *          matrix formats which are optimized for read-only accesses.
 *
 *          EdgeData is used for representing edge weights (the default type
 *          is Empty which is used for unweighted graphs).
 *
 * @tparam  EdgeData    Type of the edge data.
 */
template <typename EdgeData>
class OlapOnDB : public OlapBase<EdgeData> {
    GraphDB *db_;
    Transaction &txn_;
    ParallelVector<size_t> original_vids_;
    cuckoohash_map<size_t, size_t> vid_map_;
    size_t flags_;
    std::function<bool(VertexIterator &)> vertex_filter_;
    std::function<bool(OutEdgeIterator &, EdgeData &)> out_edge_filter_;

    void Init(size_t num_vertices) {
        this->num_vertices_ = num_vertices;
        this->num_edges_ = 0;
        size_t edge_data_size = std::is_same<EdgeData, Empty>::value ? 0 : sizeof(EdgeData);
        this->adj_unit_size_ = edge_data_size + sizeof(size_t);
        this->edge_unit_size_ = this->adj_unit_size_ + sizeof(size_t);

        if (flags_ & SNAPSHOT_UNDIRECTED) {
            this->edge_direction_policy_ = INPUT_SYMMETRIC;
        } else {
            this->edge_direction_policy_ = DUAL_DIRECTION;
        }

        this->out_degree_.ReAlloc(this->num_vertices_);
        this->in_degree_.ReAlloc(this->num_vertices_);
        this->out_index_.ReAlloc(this->num_vertices_ + 1);
        this->in_index_.ReAlloc(this->num_vertices_ + 1);
        this->out_edges_.ReAlloc(MAX_NUM_EDGES);
        this->in_edges_.ReAlloc(MAX_NUM_EDGES);
        this->lock_array_.ReAlloc(this->num_vertices_);
    }

    /**
     * @brief   This decision formula is used to
     *          determine whether to stop the algorithm running in OlapOnDB.
     */

    bool CheckKillThisTask() {
        auto task_ctx = GetThreadContext();
        return ShouldKillThisTask(task_ctx);
    }

    void Construct() {
        original_vids_.ReAlloc(this->num_vertices_);
        vid_map_.reserve(this->num_vertices_);
        auto task_ctx = GetThreadContext();
        auto worker = Worker::SharedWorker();
        if ((flags_ & SNAPSHOT_PARALLEL) && txn_.IsReadOnly()) {
            // parallel generation
            worker->Delegate([&]() {
                int num_threads = 0;
#pragma omp parallel
                {
                    if (omp_get_thread_num() == 0) {
                        num_threads = omp_get_num_threads();
                    }
                };

                std::vector<size_t> partition_offset(num_threads + 1, 0);
                std::vector<size_t> out_edges_partition_offset(num_threads + 1, 0);
#pragma omp parallel
                {
                    ParallelVector<size_t> local_original_vids(this->num_vertices_);
                    ParallelVector<size_t> local_out_index(this->num_vertices_ + 1);
                    ParallelVector<AdjUnit<EdgeData>> local_out_edges(MAX_NUM_EDGES);

                    auto txn = db_->ForkTxn(txn_);
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    auto vit = txn.GetVertexIterator();
                    for (size_t start = 64 * thread_id; start < this->num_vertices_;
                         start += 64 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 64;
                        if (end > this->num_vertices_) end = this->num_vertices_;
                        for (vit.Goto(start, true); vit.IsValid(); vit.Next()) {
                            size_t original_vid = vit.GetId();
                            if (original_vid >= end) break;
                            if (!vertex_filter_ || vertex_filter_(vit)) {
                                local_original_vids.Append(original_vid, false);
                            }
                        }
                    }

                    partition_offset[thread_id + 1] = local_original_vids.Size();

#pragma omp barrier

                    if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE1_ABORT;

                    if (thread_id == 0) {
                        for (int thread_id = 0; thread_id < num_threads; thread_id++) {
                            partition_offset[thread_id + 1] += partition_offset[thread_id];
                        }
                        this->num_vertices_ = partition_offset[num_threads];
                        original_vids_.Resize(this->num_vertices_);
                    }
#pragma omp barrier

                    memcpy(original_vids_.Data() + partition_offset[thread_id],
                           local_original_vids.Data(), sizeof(size_t) * local_original_vids.Size());
                    local_out_index.Append(0, false);

#pragma omp barrier

                    for (size_t vi = partition_offset[thread_id];
                         vi < partition_offset[thread_id + 1]; vi++) {
                        if (vi % 1024 == 0 && ShouldKillThisTask(task_ctx)) break;
                        vid_map_.insert(original_vids_[vi], vi);
                    }

#pragma omp barrier

                    if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE1_ABORT;

                    for (size_t vi = partition_offset[thread_id];
                         vi < partition_offset[thread_id + 1]; vi++) {
                        if (vi % 64 == 0 && ShouldKillThisTask(task_ctx)) break;
                        size_t original_vid = original_vids_[vi];
                        vit.Goto(original_vid);
                        for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                            size_t dst = eit.GetDst();
                            EdgeData edata;
                            if (vid_map_.contains(dst) &&
                                (!out_edge_filter_ || out_edge_filter_(eit, edata))) {
                                AdjUnit<EdgeData> out_edge;
                                out_edge.neighbour = dst;
                                if (!std::is_same<EdgeData, Empty>::value) {
                                    out_edge.edge_data = edata;
                                }
                                local_out_edges.Append(out_edge, false);
                            }
                        }
                        local_out_index.Append(local_out_edges.Size(), false);
                    }

                    out_edges_partition_offset[thread_id + 1] = local_out_edges.Size();

#pragma omp barrier

                    if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE1_ABORT;

                    if (thread_id == 0) {
                        for (int thread_id = 0; thread_id < num_threads; thread_id++) {
                            out_edges_partition_offset[thread_id + 1] +=
                                out_edges_partition_offset[thread_id];
                        }
                        this->num_edges_ = out_edges_partition_offset[num_threads];
                        this->out_edges_.Resize(this->num_edges_);
                        this->out_index_.Resize(this->num_vertices_ + 1);
                    }

#pragma omp barrier

                    if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE1_ABORT;

                    for (size_t ii = 0; ii < local_out_index.Size(); ii++) {
                        if (ii % 1024 == 0 && ShouldKillThisTask(task_ctx)) break;
                        local_out_index[ii] += out_edges_partition_offset[thread_id];
                    }
                    if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE1_ABORT;
                    memcpy(this->out_index_.Data() + partition_offset[thread_id],
                           local_out_index.Data(), sizeof(size_t) * local_out_index.Size());

                    if (thread_id == 0) {
                        this->out_index_[this->num_vertices_] = this->num_edges_;
                    }

                    if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE1_ABORT;
                    memcpy(this->out_edges_.Data() + out_edges_partition_offset[thread_id],
                           local_out_edges.Data(),
                           sizeof(AdjUnit<EdgeData>) * local_out_edges.Size());

                SNAPSHOT_PHASE1_ABORT:
                    {};
                }
            });
        } else {
            // sequential generation
            for (auto vit = txn_.GetVertexIterator(); vit.IsValid(); vit.Next()) {
                if (!vertex_filter_ || vertex_filter_(vit)) {
                    size_t original_vid = vit.GetId();
                    original_vids_.Append(original_vid, false);
                }
            }
            this->num_vertices_ = original_vids_.Size();
            worker->Delegate([&]() {
#pragma omp parallel for
                for (size_t vi = 0; vi < this->num_vertices_; vi++) {
                    vid_map_.insert(original_vids_[vi], vi);
                }
            });

            this->out_index_.Append(0, false);
            auto vit = txn_.GetVertexIterator();
            for (auto vid : original_vids_) {
                vit.Goto(vid);
                for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                    size_t dst = eit.GetDst();
                    EdgeData edata;
                    if (vid_map_.contains(dst) &&
                        (!out_edge_filter_ || out_edge_filter_(eit, edata))) {
                        AdjUnit<EdgeData> out_edge;
                        out_edge.neighbour = dst;
                        if (!std::is_same<EdgeData, Empty>::value) {
                            out_edge.edge_data = edata;
                        }
                        this->out_edges_.Append(out_edge, false);
                    }
                }
                this->out_index_.Append(this->out_edges_.Size(), false);
            }
            this->num_edges_ = this->out_edges_.Size();
        }

        if (this->num_vertices_ == 0) {
            THROW_CODE(InputError, "The graph vertex cannot be empty");
        }
        if (this->num_edges_ == 0) {
            THROW_CODE(InputError, "The graph edge cannot be empty");
        }
        if (ShouldKillThisTask(task_ctx)) throw std::runtime_error("Task killed");
        if (this->num_vertices_ == 0) {
            throw std::runtime_error("The olapondb graph cannot be empty");
        }
        this->lock_array_.Resize(this->num_vertices_);
        this->lock_array_.Fill(false);
        worker->Delegate([&]() {
            if (flags_ & SNAPSHOT_UNDIRECTED) {
                this->in_degree_.Resize(this->num_vertices_);
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 1024 * thread_id; start < this->num_vertices_;
                         start += 1024 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 1024;
                        if (end > this->num_vertices_) end = this->num_vertices_;
                        for (size_t vi = start; vi < end; vi++) {
                            this->in_degree_[vi] = this->out_index_[vi + 1] - this->out_index_[vi];
                        }
                    }
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 1024 * thread_id; start < this->num_edges_;
                         start += 1024 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 1024;
                        if (end > this->num_edges_) end = this->num_edges_;
                        for (size_t ei = start; ei < end; ei++) {
                            size_t dst = this->out_edges_[ei].neighbour;
                            size_t mapped_dst = vid_map_.find(dst);
                            this->out_edges_[ei].neighbour = mapped_dst;
                            __sync_fetch_and_add(&this->in_degree_[mapped_dst], 1);
                        }
                    }
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                this->num_edges_ *= 2;
                this->in_index_.Resize(this->num_vertices_ + 1);
                this->in_index_[0] = 0;
                for (size_t vi = 0; vi < this->num_vertices_; vi++) {
                    if (vi % 1024 == 0 && ShouldKillThisTask(task_ctx)) break;
                    this->in_index_[vi + 1] = this->in_index_[vi] + this->in_degree_[vi];
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                assert(this->in_index_[this->num_vertices_] == this->num_edges_);
                this->in_edges_.Resize(this->num_edges_);
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 64 * thread_id; start < this->num_vertices_;
                         start += 64 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 64;
                        if (end > this->num_vertices_) end = this->num_vertices_;
                        for (size_t src = start; src < end; src++) {
                            size_t pos = __sync_fetch_and_add(
                                &this->in_index_[src],
                                this->out_index_[src + 1] - this->out_index_[src]);
                            memcpy(this->in_edges_.Data() + pos,
                                   this->out_edges_.Data() + this->out_index_[src],
                                   sizeof(AdjUnit<EdgeData>) *
                                       (this->out_index_[src + 1] - this->out_index_[src]));
                            for (size_t ei = this->out_index_[src]; ei < this->out_index_[src + 1];
                                 ei++) {
                                auto &out_edge = this->out_edges_[ei];
                                size_t dst = out_edge.neighbour;
                                size_t pos = __sync_fetch_and_add(&this->in_index_[dst], 1);
                                auto &in_edge = this->in_edges_[pos];
                                in_edge.neighbour = src;
                                if (!std::is_same<EdgeData, Empty>::value) {
                                    in_edge.edge_data = out_edge.edge_data;
                                }
                            }
                        }
                    }
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                memmove(this->in_index_.Data() + 1, this->in_index_.Data(),
                        sizeof(size_t) * this->num_vertices_);
                this->in_index_[0] = 0;
                this->out_degree_.Swap(this->in_degree_);
                this->out_index_.Swap(this->in_index_);
                this->out_edges_.Swap(this->in_edges_);
                this->in_degree_.Destroy();
                this->in_index_.Destroy();
                this->in_edges_.Destroy();
            } else {
                this->out_degree_.Resize(this->num_vertices_);
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 1024 * thread_id; start < this->num_vertices_;
                         start += 1024 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 1024;
                        if (end > this->num_vertices_) end = this->num_vertices_;
                        for (size_t vi = start; vi < end; vi++) {
                            this->out_degree_[vi] = this->out_index_[vi + 1] - this->out_index_[vi];
                        }
                    }
                }

                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                this->in_degree_.Resize(this->num_vertices_);
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 1024 * thread_id; start < this->num_vertices_;
                         start += 1024 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 1024;
                        if (end > this->num_vertices_) end = this->num_vertices_;
                        for (size_t vi = start; vi < end; vi++) {
                            this->in_degree_[vi] = 0;
                        }
                    }
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 1024 * thread_id; start < this->num_edges_;
                         start += 1024 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 1024;
                        if (end > this->num_edges_) end = this->num_edges_;
                        for (size_t ei = start; ei < end; ei++) {
                            size_t dst = this->out_edges_[ei].neighbour;
                            size_t mapped_dst = vid_map_.find(dst);
                            this->out_edges_[ei].neighbour = mapped_dst;
                            __sync_fetch_and_add(&this->in_degree_[mapped_dst], 1);
                        }
                    }
                }

                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                this->in_index_.Resize(this->num_vertices_ + 1);
                this->in_index_[0] = 0;
                for (size_t vi = 0; vi < this->num_vertices_; vi++) {
                    if (vi % 1024 == 0) {
                        if (ShouldKillThisTask(task_ctx)) break;
                    }
                    this->in_index_[vi + 1] = this->in_index_[vi] + this->in_degree_[vi];
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                assert(this->in_index_[this->num_vertices_] == this->num_edges_);
                this->in_edges_.Resize(this->num_edges_);
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 64 * thread_id; start < this->num_vertices_;
                         start += 64 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 64;
                        if (end > this->num_vertices_) end = this->num_vertices_;
                        for (size_t src = start; src < end; src++) {
                            for (size_t ei = this->out_index_[src]; ei < this->out_index_[src + 1];
                                 ei++) {
                                auto &out_edge = this->out_edges_[ei];
                                size_t dst = out_edge.neighbour;
                                size_t pos = __sync_fetch_and_add(&this->in_index_[dst], 1);
                                auto &in_edge = this->in_edges_[pos];
                                in_edge.neighbour = src;
                                if (!std::is_same<EdgeData, Empty>::value) {
                                    in_edge.edge_data = out_edge.edge_data;
                                }
                            }
                        }
                    }
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                memmove(this->in_index_.Data() + 1, this->in_index_.Data(),
                        sizeof(size_t) * this->num_vertices_);
                this->in_index_[0] = 0;

            SNAPSHOT_PHASE2_ABORT:
                {};
            }
        });
        if (ShouldKillThisTask(task_ctx)) throw std::runtime_error("Task killed");
    }

    void ConstructWithVid() {
        auto task_ctx = GetThreadContext();
        auto worker = Worker::SharedWorker();

        // Read from TuGraph
        if ((flags_ & SNAPSHOT_PARALLEL) && txn_.IsReadOnly()) {
            this->out_index_.Resize(this->num_vertices_ + 1, (size_t)0);
            worker->Delegate([&]() {
                int num_threads = 0;
#pragma omp parallel
                {
                    if (omp_get_thread_num() == 0) {
                        num_threads = omp_get_num_threads();
                    }
                };
                num_threads = std::min(16, num_threads);

                std::vector<size_t> out_edges_partition_offset(num_threads + 1, 0);
#pragma omp parallel num_threads(num_threads)
                {
                    ParallelVector<size_t> local_out_index(this->num_vertices_);
                    ParallelVector<AdjUnit<EdgeData>> local_out_edges(MAX_NUM_EDGES);

                    auto txn = db_->ForkTxn(txn_);
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    auto vit = txn.GetVertexIterator();

                    size_t partition_size = this->num_vertices_ / num_threads;
                    size_t partition_begin = partition_size * thread_id;
                    size_t partition_end = partition_begin + partition_size;
                    if (thread_id == num_threads - 1) {
                        partition_end = this->num_vertices_;
                        partition_size = partition_end - partition_begin;
                    }

                    for (size_t vi = partition_begin; vi < partition_end; vi++) {
                        if (vi % 64 == 0 && ShouldKillThisTask(task_ctx)) break;
                        vit.Goto(vi);
                        if (!vit.IsValid() || vit.GetNumOutEdges() == 0) {
                            continue;
                        }
                        for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                            size_t dst = eit.GetDst();
                            EdgeData edata;
                            if (!out_edge_filter_ || out_edge_filter_(eit, edata)) {
                                AdjUnit<EdgeData> out_edge;
                                out_edge.neighbour = dst;
                                if (!std::is_same<EdgeData, Empty>::value) {
                                    out_edge.edge_data = edata;
                                }
                                local_out_edges.Append(out_edge, false);
                            }
                        }
                        local_out_index.Append(local_out_edges.Size(), false);
                    }
                    out_edges_partition_offset[thread_id + 1] = local_out_edges.Size();

#pragma omp barrier

                    if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE1_ABORT;

                    if (thread_id == 0) {
                        for (int tid = 0; tid < num_threads; tid++) {
                            out_edges_partition_offset[tid + 1] += out_edges_partition_offset[tid];
                        }

                        this->num_edges_ = out_edges_partition_offset[num_threads];
                        this->out_edges_.Resize(this->num_edges_);
                        this->out_index_.Resize(this->num_vertices_ + 1);
                        this->out_index_[0] = 0;
                    }

#pragma omp barrier

                    if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE1_ABORT;

                    for (size_t ii = 0; ii < local_out_index.Size(); ii++) {
                        if (ii % 1024 == 0 && ShouldKillThisTask(task_ctx)) break;
                        local_out_index[ii] += out_edges_partition_offset[thread_id];
                    }

                    if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE1_ABORT;

                    memcpy(this->out_index_.Data() + partition_begin + 1, local_out_index.Data(),
                           sizeof(size_t) * local_out_index.Size());

                    if (thread_id == 0) {
                        this->out_index_[this->num_vertices_] = this->num_edges_;
                    }

                    if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE1_ABORT;
                    memcpy(this->out_edges_.Data() + out_edges_partition_offset[thread_id],
                           local_out_edges.Data(), this->adj_unit_size_ * local_out_edges.Size());

                SNAPSHOT_PHASE1_ABORT:
                    {};
                }
            });
        } else {
            // sequential generation
            this->out_index_.Append(0, false);
            auto vit = txn_.GetVertexIterator();
            for (size_t vid = 0; vid < this->num_vertices_; vid++) {
                if (!vit.Goto(vid)) continue;
                for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                    size_t dst = eit.GetDst();
                    EdgeData edata;
                    if (!out_edge_filter_ || out_edge_filter_(eit, edata)) {
                        AdjUnit<EdgeData> out_edge;
                        out_edge.neighbour = dst;
                        if (!std::is_same<EdgeData, Empty>::value) {
                            out_edge.edge_data = edata;
                        }
                        this->out_edges_.Append(out_edge, false);
                    }
                }
                this->out_index_.Append(this->out_edges_.Size(), false);
            }
            this->num_edges_ = this->out_edges_.Size();
        }

        if (ShouldKillThisTask(task_ctx)) throw std::runtime_error("Task killed");
        if (this->num_vertices_ == 0) {
            throw std::runtime_error("The olapondb graph cannot be empty");
        }
        this->lock_array_.Resize(this->num_vertices_);
        this->lock_array_.Fill(false);
        worker->Delegate([&]() {
            if (flags_ & SNAPSHOT_UNDIRECTED) {
                this->in_degree_.Resize(this->num_vertices_);
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 1024 * thread_id; start < this->num_vertices_;
                         start += 1024 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 1024;
                        if (end > this->num_vertices_) end = this->num_vertices_;
                        for (size_t vi = start; vi < end; vi++) {
                            this->in_degree_[vi] = this->out_index_[vi + 1] - this->out_index_[vi];
                        }
                    }
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 1024 * thread_id; start < this->num_edges_;
                         start += 1024 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 1024;
                        if (end > this->num_edges_) end = this->num_edges_;
                        for (size_t ei = start; ei < end; ei++) {
                            size_t dst = this->out_edges_[ei].neighbour;
                            __sync_fetch_and_add(&this->in_degree_[dst], 1);
                        }
                    }
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                this->num_edges_ *= 2;
                this->in_index_.Resize(this->num_vertices_ + 1);
                this->in_index_[0] = 0;
                for (size_t vi = 0; vi < this->num_vertices_; vi++) {
                    if (vi % 1024 == 0 && ShouldKillThisTask(task_ctx)) break;
                    this->in_index_[vi + 1] = this->in_index_[vi] + this->in_degree_[vi];
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                assert(this->in_index_[this->num_vertices_] == this->num_edges_);
                this->in_edges_.Resize(this->num_edges_);
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 64 * thread_id; start < this->num_vertices_;
                         start += 64 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 64;
                        if (end > this->num_vertices_) end = this->num_vertices_;
                        for (size_t src = start; src < end; src++) {
                            size_t pos = __sync_fetch_and_add(
                                &this->in_index_[src],
                                this->out_index_[src + 1] - this->out_index_[src]);
                            memcpy(this->in_edges_.Data() + pos,
                                   this->out_edges_.Data() + this->out_index_[src],
                                   sizeof(AdjUnit<EdgeData>) *
                                       (this->out_index_[src + 1] - this->out_index_[src]));
                            for (size_t ei = this->out_index_[src]; ei < this->out_index_[src + 1];
                                 ei++) {
                                auto &out_edge = this->out_edges_[ei];
                                size_t dst = out_edge.neighbour;
                                size_t pos = __sync_fetch_and_add(&this->in_index_[dst], 1);
                                auto &in_edge = this->in_edges_[pos];
                                in_edge.neighbour = src;
                                if (!std::is_same<EdgeData, Empty>::value) {
                                    in_edge.edge_data = out_edge.edge_data;
                                }
                            }
                        }
                    }
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                memmove(this->in_index_.Data() + 1, this->in_index_.Data(),
                        sizeof(size_t) * this->num_vertices_);
                this->in_index_[0] = 0;
                this->out_degree_.Swap(this->in_degree_);
                this->out_index_.Swap(this->in_index_);
                this->out_edges_.Swap(this->in_edges_);
                this->in_degree_.Destroy();
                this->in_index_.Destroy();
                this->in_edges_.Destroy();
            } else {
                this->out_degree_.Resize(this->num_vertices_);
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 1024 * thread_id; start < this->num_vertices_;
                         start += 1024 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 1024;
                        if (end > this->num_vertices_) end = this->num_vertices_;
                        for (size_t vi = start; vi < end; vi++) {
                            this->out_degree_[vi] = this->out_index_[vi + 1] - this->out_index_[vi];
                        }
                    }
                }

                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                this->in_degree_.Resize(this->num_vertices_);
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 1024 * thread_id; start < this->num_vertices_;
                         start += 1024 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 1024;
                        if (end > this->num_vertices_) end = this->num_vertices_;
                        for (size_t vi = start; vi < end; vi++) {
                            this->in_degree_[vi] = 0;
                        }
                    }
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 1024 * thread_id; start < this->num_edges_;
                         start += 1024 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 1024;
                        if (end > this->num_edges_) end = this->num_edges_;
                        for (size_t ei = start; ei < end; ei++) {
                            size_t dst = this->out_edges_[ei].neighbour;
                            __sync_fetch_and_add(&this->in_degree_[dst], 1);
                        }
                    }
                }

                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                this->in_index_.Resize(this->num_vertices_ + 1);
                this->in_index_[0] = 0;
                for (size_t vi = 0; vi < this->num_vertices_; vi++) {
                    if (vi % 1024 == 0) {
                        if (ShouldKillThisTask(task_ctx)) break;
                    }
                    this->in_index_[vi + 1] = this->in_index_[vi] + this->in_degree_[vi];
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                assert(this->in_index_[this->num_vertices_] == this->num_edges_);
                this->in_edges_.Resize(this->num_edges_);
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 64 * thread_id; start < this->num_vertices_;
                         start += 64 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 64;
                        if (end > this->num_vertices_) end = this->num_vertices_;
                        for (size_t src = start; src < end; src++) {
                            for (size_t ei = this->out_index_[src]; ei < this->out_index_[src + 1];
                                 ei++) {
                                auto &out_edge = this->out_edges_[ei];
                                size_t dst = out_edge.neighbour;
                                size_t pos = __sync_fetch_and_add(&this->in_index_[dst], 1);
                                auto &in_edge = this->in_edges_[pos];
                                in_edge.neighbour = src;
                                if (!std::is_same<EdgeData, Empty>::value) {
                                    in_edge.edge_data = out_edge.edge_data;
                                }
                            }
                        }
                    }
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                memmove(this->in_index_.Data() + 1, this->in_index_.Data(),
                        sizeof(size_t) * this->num_vertices_);
                this->in_index_[0] = 0;

            SNAPSHOT_PHASE2_ABORT:
                {};
            }
        });

        if (ShouldKillThisTask(task_ctx)) throw std::runtime_error("Task killed");
    }

    void ConstructWithDegree() {
        auto task_ctx = GetThreadContext();
        auto worker = Worker::SharedWorker();

        this->out_index_.Resize(this->num_vertices_ + 1, (size_t)0);
        this->out_degree_.Resize(this->num_vertices_, (size_t)0);
        if (flags_ & SNAPSHOT_UNDIRECTED) {
            this->in_index_.Destroy();
            this->in_degree_.Destroy();
            this->in_edges_.Destroy();
        } else {
            this->in_index_.Resize(this->num_vertices_ + 1, (size_t)0);
            this->in_degree_.Resize(this->num_vertices_, (size_t)0);
        }

        if ((flags_ & SNAPSHOT_PARALLEL) && txn_.IsReadOnly()) {
            worker->Delegate([&]() {
                int num_threads = 0;
#pragma omp parallel
                {
                    if (omp_get_thread_num() == 0) {
                        num_threads = omp_get_num_threads();
                    }
                };

                std::vector<size_t> out_edges_partition_offset(num_threads + 1, 0);
                std::vector<size_t> in_edges_partition_offset(num_threads + 1, 0);
#pragma omp parallel
                {
                    auto txn = db_->ForkTxn(txn_);
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    auto vit = txn.GetVertexIterator();

                    size_t partition_size = this->num_vertices_ / num_threads;
                    size_t partition_begin = partition_size * thread_id;
                    size_t partition_end = partition_begin + partition_size;
                    if (thread_id == num_threads - 1) {
                        partition_end = this->num_vertices_;
                        partition_size = partition_end - partition_begin;
                    }

                    for (size_t vi = partition_begin; vi < partition_end; vi++) {
                        if (vi % 64 == 0 && ShouldKillThisTask(task_ctx)) break;
                        if (!vit.Goto(vi)) continue;
                        this->out_degree_[vi] = vit.GetNumOutEdges();
                        if (flags_ & SNAPSHOT_UNDIRECTED) {
                            this->out_degree_[vi] += vit.GetNumInEdges();
                        } else {
                            this->in_degree_[vi] = vit.GetNumInEdges();
                            in_edges_partition_offset[thread_id + 1] += this->in_degree_[vi];
                        }
                        out_edges_partition_offset[thread_id + 1] += this->out_degree_[vi];
                    }

#pragma omp barrier

                    if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE1_ABORT;

                    if (thread_id == 0) {
                        for (int tid = 0; tid < num_threads; tid++) {
                            out_edges_partition_offset[tid + 1] += out_edges_partition_offset[tid];
                        }
                        if (!(flags_ & SNAPSHOT_UNDIRECTED)) {
                            for (int tid = 0; tid < num_threads; tid++) {
                                in_edges_partition_offset[tid + 1] +=
                                    in_edges_partition_offset[tid];
                            }
                        }

                        this->num_edges_ = out_edges_partition_offset[num_threads];
                        this->out_edges_.Resize(this->num_edges_);
                        if (!(flags_ & SNAPSHOT_UNDIRECTED)) {
                            this->in_edges_.Resize(this->num_edges_);
                        }
                    }

#pragma omp barrier

                    if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE1_ABORT;

                    this->out_index_[partition_begin] = out_edges_partition_offset[thread_id];
                    for (size_t ii = partition_begin; ii < partition_end; ii++) {
                        if (ii % 1024 == 0 && ShouldKillThisTask(task_ctx)) break;
                        this->out_index_[ii + 1] = this->out_index_[ii] + this->out_degree_[ii];
                    }

                    if (!(flags_ & SNAPSHOT_UNDIRECTED)) {
                        this->in_index_[partition_begin] = in_edges_partition_offset[thread_id];
                        for (size_t ii = partition_begin; ii < partition_end; ii++) {
                            if (ii % 1024 == 0 && ShouldKillThisTask(task_ctx)) break;
                            this->in_index_[ii + 1] = this->in_index_[ii] + this->in_degree_[ii];
                        }
                    }

                    if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE1_ABORT;

#pragma omp barrier

                    for (size_t vi = partition_begin; vi < partition_end; vi++) {
                        if (vi % 64 == 0 && ShouldKillThisTask(task_ctx)) break;
                        if (!vit.Goto(vi)) continue;
                        size_t pos = this->out_index_[vi];
                        for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                            size_t dst = eit.GetDst();
                            this->out_edges_[pos].neighbour = dst;
                            pos++;
                        }
                        if (flags_ & SNAPSHOT_UNDIRECTED) {
                            for (auto eit = vit.GetInEdgeIterator(); eit.IsValid(); eit.Next()) {
                                size_t src = eit.GetSrc();
                                this->out_edges_[pos].neighbour = src;
                                pos++;
                            }
                        } else {
                            pos = this->in_index_[vi];
                            for (auto eit = vit.GetInEdgeIterator(); eit.IsValid(); eit.Next()) {
                                size_t src = eit.GetSrc();
                                this->in_edges_[pos].neighbour = src;
                                pos++;
                            }
                        }
                    }

#pragma omp barrier

                    assert(this->out_index_[this->num_vertices_] == this->num_edges_);

                SNAPSHOT_PHASE1_ABORT:
                    {};
                }
            });
            //        } else {
            //            auto vit = txn_.GetVertexIterator();
            //            for (size_t vid = 0; vid < this->num_vertices_; vid++) {
            //                if (!vit.Goto(vid)) continue;
            //                for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
            //                    size_t dst = eit.GetDst();
            //                    AdjUnit<EdgeData> out_edge;
            //                    out_edge.neighbour = dst;
            //                    this->out_edges_.Append(out_edge, false);
            //                }
            //                if (flags_ & SNAPSHOT_UNDIRECTED) {
            //                    for (auto eit = vit.GetInEdgeIterator(); eit.IsValid();
            //                    eit.Next()) {
            //                        size_t src = eit.GetSrc();
            //                        AdjUnit<EdgeData> in_edge;
            //                        in_edge.neighbour = src;
            //                        this->out_edges_.Append(in_edge, false);
            //                    }
            //                } else {
            //                    for (auto eit = vit.GetInEdgeIterator(); eit.IsValid();
            //                    eit.Next()) {
            //                        size_t src = eit.GetSrc();
            //                        AdjUnit<EdgeData> in_edge;
            //                        in_edge.neighbour = src;
            //                        this->in_edges_.Append(in_edge, false);
            //                    }
            //                    this->in_index_[vid + 1] = this->in_edges_.Size();
            //                    this->in_degree_[vid] = this->in_index_[vid + 1] -
            //                    this->in_index_[vid];
            //                }
            //                this->out_index_[vid + 1] = this->out_edges_.Size();
            //                this->out_degree_[vid] = this->out_index_[vid + 1] -
            //                this->out_index_[vid];
            //            }
            //            this->num_edges_ = this->out_edges_.Size();
            //            this->out_edges_.Resize(this->num_edges_);
        }
        this->lock_array_.Resize(this->num_vertices_);
        this->lock_array_.Fill(false);
    }

 public:
    /**
     * @brief Generate a graph with LightningGraph. For V1/V2 Procedures
     */
    OlapOnDB(GraphDB *db, Transaction &txn, size_t flags = 0,
             std::function<bool(VertexIterator &)> vertex_filter = nullptr,
             std::function<bool(OutEdgeIterator &, EdgeData &)> out_edge_filter = nullptr)
        : db_(db),
          txn_(txn),
          flags_(flags),
          vertex_filter_(vertex_filter),
          out_edge_filter_(out_edge_filter) {
        if (db_ == nullptr && flags_ & SNAPSHOT_PARALLEL) {
            LOG_WARN() << "SNAPSHOT_PARALLEL needs to pass in the db parameter";
            flags_ -= SNAPSHOT_PARALLEL;
        }
        if (txn.GetNumVertices() == 0) {
            throw std::runtime_error("The graph cannot be empty");
        }
//        if (vertex_filter != nullptr) {
//            flags_ |= SNAPSHOT_IDMAPPING;
//        }
        flags_ |= SNAPSHOT_IDMAPPING;
        Init(txn.GetNumVertices());

        /*if (flags_ & SNAPSHOT_IDMAPPING) {
            Construct();
        } else {
            if ((out_edge_filter == nullptr) && (flags_ & SNAPSHOT_PARALLEL) && txn_.IsReadOnly()) {
                ConstructWithDegree();
            } else {
                ConstructWithVid();
            }
        }*/
        Construct();
    }

    /**
     * @brief Generate a graph with LightningGraph. For V1 Procedures
     *
     * @exception std::runtime_error  Raised when a runtime error condition
     *                                occurs.
     *
     * @param [in,out]    db              The GraphDB instance.
     * @param [in,out]    txn             The transaction.
     * @param             flags           (Optional) The generation flags.
     * @param [in,out]    vertex_filter   (Optional) A function filtering
     *                                    vertices.
     * @param [in,out]    out_edge_filter (Optional) A function filtering out
     *                                    edges.
     *
     *                                    Note: read-write transactions are
     *                                    not recommended here for safety, e.g.
     *                                    some vertices might be removed causing
     *                                    inconsistencies of the analysis, and
     *                                    vertex data extraction may not work for
     *                                    deleted vertices). The constructed
     *                                    graph should contain all vertices
     *                                    whose vertex_filter calls return true
     *                                    and all edges *sourced from* these
     *                                    vertices whose out_edge_filter calls
     *                                    return true. If SNAPSHOT_UNDIRECTED is
     *                                    specified, the graph will be made
     *                                    symmetric (i.e. reversed edges are also
     *                                    added to the graph).
     */
    OlapOnDB(GraphDB &db, Transaction &txn, size_t flags = 0,
             std::function<bool(VertexIterator &)> vertex_filter = nullptr,
             std::function<bool(OutEdgeIterator &, EdgeData &)> out_edge_filter = nullptr)
        : OlapOnDB(&db, txn, flags, vertex_filter, out_edge_filter) {}

    // Filter subgraphs based on a set of triples of point labels, edge labels, and point labels
    OlapOnDB(GraphDB &db, Transaction &txn, std::vector<std::vector<std::string>> label_list,
             size_t flags = 0)
        : db_(&db), txn_(txn), flags_(flags) {
        if (txn.GetNumVertices() == 0) {
            throw std::runtime_error("The graph cannot be empty");
        }
        flags_ |= SNAPSHOT_IDMAPPING;
        Init(txn.GetNumVertices());
        std::vector<std::vector<size_t>> label_id_list;
        for (auto &labels : label_list) {
            std::vector<size_t> label_id;
            label_id.push_back(txn.GetVertexLabelId(labels[0]));
            label_id.push_back(txn.GetEdgeLabelId(labels[1]));
            label_id.push_back(txn.GetVertexLabelId(labels[2]));
            label_id_list.push_back(label_id);
        }
        original_vids_.ReAlloc(this->num_vertices_);
        vid_map_.reserve(this->num_vertices_);
        auto task_ctx = GetThreadContext();
        auto worker = Worker::SharedWorker();
        if ((flags_ & SNAPSHOT_PARALLEL) && txn_.IsReadOnly()) {
            // parallel generation
            worker->Delegate([&]() {
                int num_threads = 0;
#pragma omp parallel
                {
                    if (omp_get_thread_num() == 0) {
                        num_threads = omp_get_num_threads();
                    }
                };

                std::vector<size_t> partition_offset(num_threads + 1, 0);
                std::vector<size_t> out_edges_partition_offset(num_threads + 1, 0);
#pragma omp parallel
                {
                    ParallelVector<size_t> local_original_vids(this->num_vertices_);
                    ParallelVector<size_t> local_out_index(this->num_vertices_ + 1);
                    ParallelVector<AdjUnit<EdgeData>> local_out_edges(MAX_NUM_EDGES);

                    local_out_index.Append(0, false);

                    auto txn = db_->ForkTxn(txn_);
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    auto vit = txn.GetVertexIterator();
                    for (size_t start = 64 * thread_id; start < this->num_vertices_;
                         start += 64 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 64;
                        if (end > this->num_vertices_) end = this->num_vertices_;
                        for (vit.Goto(start, true); vit.IsValid(); vit.Next()) {
                            size_t original_vid = vit.GetId();
                            if (original_vid >= end) break;
                            auto label_id = vit.GetLabelId();
                            bool keepVertex = false;
                            for (auto &labels : label_id_list) {
                                if (label_id == labels[0]) {
                                    OutEdgeIterator eit = vit.GetOutEdgeIterator();
                                    while (eit.IsValid()) {
                                        auto dst = eit.GetDst();
                                        vit.Goto(dst);
                                        if (size_t(eit.GetLabelId()) == labels[1] &&
                                            vit.GetLabelId() == labels[2]) {
                                            keepVertex = true;
                                            AdjUnit<EdgeData> out_edge;
                                            out_edge.neighbour = dst;
                                            local_out_edges.Append(out_edge, false);
                                        }
                                        eit.Next();
                                    }
                                }
                                if (!keepVertex && (label_id == labels[2])) {
                                    InEdgeIterator eit = vit.GetInEdgeIterator();
                                    while (eit.IsValid()) {
                                        auto src = eit.GetSrc();
                                        vit.Goto(src);
                                        if (size_t(eit.GetLabelId()) == labels[1] &&
                                            vit.GetLabelId() == labels[0]) {
                                            keepVertex = true;
                                            break;
                                        }
                                        eit.Next();
                                    }
                                }
                            }
                            vit.Goto(original_vid);
                            if (keepVertex) {
                                local_original_vids.Append(original_vid, false);
                            }
                            local_out_index.Append(local_out_edges.Size(), false);
                        }
                    }

                    partition_offset[thread_id + 1] = local_original_vids.Size();

#pragma omp barrier

                    if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE1_ABORT;

                    if (thread_id == 0) {
                        for (int thread_id = 0; thread_id < num_threads; thread_id++) {
                            partition_offset[thread_id + 1] += partition_offset[thread_id];
                        }
                        this->num_vertices_ = partition_offset[num_threads];
                        original_vids_.Resize(this->num_vertices_);
                    }
#pragma omp barrier

                    memcpy(original_vids_.Data() + partition_offset[thread_id],
                           local_original_vids.Data(), sizeof(size_t) * local_original_vids.Size());
                    // local_out_index.Append(0, false);

#pragma omp barrier

                    for (size_t vi = partition_offset[thread_id];
                         vi < partition_offset[thread_id + 1]; vi++) {
                        if (vi % 1024 == 0 && ShouldKillThisTask(task_ctx)) break;
                        vid_map_.insert(original_vids_[vi], vi);
                    }

#pragma omp barrier

                    if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE1_ABORT;
                    out_edges_partition_offset[thread_id + 1] = local_out_edges.Size();

#pragma omp barrier

                    if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE1_ABORT;

                    if (thread_id == 0) {
                        for (int thread_id = 0; thread_id < num_threads; thread_id++) {
                            out_edges_partition_offset[thread_id + 1] +=
                                out_edges_partition_offset[thread_id];
                        }
                        this->num_edges_ = out_edges_partition_offset[num_threads];
                        this->out_edges_.Resize(this->num_edges_);
                        this->out_index_.Resize(this->num_vertices_ + 1);
                    }

#pragma omp barrier

                    if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE1_ABORT;

                    for (size_t ii = 0; ii < local_out_index.Size(); ii++) {
                        if (ii % 1024 == 0 && ShouldKillThisTask(task_ctx)) break;
                        local_out_index[ii] += out_edges_partition_offset[thread_id];
                    }
                    if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE1_ABORT;
                    memcpy(this->out_index_.Data() + partition_offset[thread_id],
                           local_out_index.Data(), sizeof(size_t) * local_out_index.Size());

                    if (thread_id == 0) {
                        this->out_index_[this->num_vertices_] = this->num_edges_;
                    }

                    if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE1_ABORT;
                    memcpy(this->out_edges_.Data() + out_edges_partition_offset[thread_id],
                           local_out_edges.Data(),
                           sizeof(AdjUnit<EdgeData>) * local_out_edges.Size());

                SNAPSHOT_PHASE1_ABORT:
                    {};
                }
            });
        }

        if (this->num_vertices_ == 0) {
            THROW_CODE(InputError, "The graph vertex cannot be empty");
        }
        if (this->num_edges_ == 0) {
            THROW_CODE(InputError, "The graph edge cannot be empty");
        }
        if (ShouldKillThisTask(task_ctx)) throw std::runtime_error("Task killed");
        if (this->num_vertices_ == 0) {
            throw std::runtime_error("The olapondb graph cannot be empty");
        }
        this->lock_array_.Resize(this->num_vertices_);
        this->lock_array_.Fill(false);
        worker->Delegate([&]() {
            if (flags_ & SNAPSHOT_UNDIRECTED) {
                this->in_degree_.Resize(this->num_vertices_);
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 1024 * thread_id; start < this->num_vertices_;
                         start += 1024 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 1024;
                        if (end > this->num_vertices_) end = this->num_vertices_;
                        for (size_t vi = start; vi < end; vi++) {
                            this->in_degree_[vi] = this->out_index_[vi + 1] - this->out_index_[vi];
                        }
                    }
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 1024 * thread_id; start < this->num_edges_;
                         start += 1024 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 1024;
                        if (end > this->num_edges_) end = this->num_edges_;
                        for (size_t ei = start; ei < end; ei++) {
                            size_t dst = this->out_edges_[ei].neighbour;
                            size_t mapped_dst = vid_map_.find(dst);
                            this->out_edges_[ei].neighbour = mapped_dst;
                            __sync_fetch_and_add(&this->in_degree_[mapped_dst], 1);
                        }
                    }
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                this->num_edges_ *= 2;
                this->in_index_.Resize(this->num_vertices_ + 1);
                this->in_index_[0] = 0;
                for (size_t vi = 0; vi < this->num_vertices_; vi++) {
                    if (vi % 1024 == 0 && ShouldKillThisTask(task_ctx)) break;
                    this->in_index_[vi + 1] = this->in_index_[vi] + this->in_degree_[vi];
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                assert(this->in_index_[this->num_vertices_] == this->num_edges_);
                this->in_edges_.Resize(this->num_edges_);
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 64 * thread_id; start < this->num_vertices_;
                         start += 64 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 64;
                        if (end > this->num_vertices_) end = this->num_vertices_;
                        for (size_t src = start; src < end; src++) {
                            size_t pos = __sync_fetch_and_add(
                                &this->in_index_[src],
                                this->out_index_[src + 1] - this->out_index_[src]);
                            memcpy(this->in_edges_.Data() + pos,
                                   this->out_edges_.Data() + this->out_index_[src],
                                   sizeof(AdjUnit<EdgeData>) *
                                       (this->out_index_[src + 1] - this->out_index_[src]));
                            for (size_t ei = this->out_index_[src]; ei < this->out_index_[src + 1];
                                 ei++) {
                                auto &out_edge = this->out_edges_[ei];
                                size_t dst = out_edge.neighbour;
                                size_t pos = __sync_fetch_and_add(&this->in_index_[dst], 1);
                                auto &in_edge = this->in_edges_[pos];
                                in_edge.neighbour = src;
                                if (!std::is_same<EdgeData, Empty>::value) {
                                    in_edge.edge_data = out_edge.edge_data;
                                }
                            }
                        }
                    }
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                memmove(this->in_index_.Data() + 1, this->in_index_.Data(),
                        sizeof(size_t) * this->num_vertices_);
                this->in_index_[0] = 0;
                this->out_degree_.Swap(this->in_degree_);
                this->out_index_.Swap(this->in_index_);
                this->out_edges_.Swap(this->in_edges_);
                this->in_degree_.Destroy();
                this->in_index_.Destroy();
                this->in_edges_.Destroy();
            } else {
                this->out_degree_.Resize(this->num_vertices_);
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 1024 * thread_id; start < this->num_vertices_;
                         start += 1024 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 1024;
                        if (end > this->num_vertices_) end = this->num_vertices_;
                        for (size_t vi = start; vi < end; vi++) {
                            this->out_degree_[vi] = this->out_index_[vi + 1] - this->out_index_[vi];
                        }
                    }
                }

                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                this->in_degree_.Resize(this->num_vertices_);
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 1024 * thread_id; start < this->num_vertices_;
                         start += 1024 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 1024;
                        if (end > this->num_vertices_) end = this->num_vertices_;
                        for (size_t vi = start; vi < end; vi++) {
                            this->in_degree_[vi] = 0;
                        }
                    }
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 1024 * thread_id; start < this->num_edges_;
                         start += 1024 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 1024;
                        if (end > this->num_edges_) end = this->num_edges_;
                        for (size_t ei = start; ei < end; ei++) {
                            size_t dst = this->out_edges_[ei].neighbour;
                            size_t mapped_dst = vid_map_.find(dst);
                            this->out_edges_[ei].neighbour = mapped_dst;
                            __sync_fetch_and_add(&this->in_degree_[mapped_dst], 1);
                        }
                    }
                }

                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                this->in_index_.Resize(this->num_vertices_ + 1);
                this->in_index_[0] = 0;
                for (size_t vi = 0; vi < this->num_vertices_; vi++) {
                    if (vi % 1024 == 0) {
                        if (ShouldKillThisTask(task_ctx)) break;
                    }
                    this->in_index_[vi + 1] = this->in_index_[vi] + this->in_degree_[vi];
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                assert(this->in_index_[this->num_vertices_] == this->num_edges_);
                this->in_edges_.Resize(this->num_edges_);
#pragma omp parallel
                {
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    for (size_t start = 64 * thread_id; start < this->num_vertices_;
                         start += 64 * num_threads) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 64;
                        if (end > this->num_vertices_) end = this->num_vertices_;
                        for (size_t src = start; src < end; src++) {
                            for (size_t ei = this->out_index_[src]; ei < this->out_index_[src + 1];
                                 ei++) {
                                auto &out_edge = this->out_edges_[ei];
                                size_t dst = out_edge.neighbour;
                                size_t pos = __sync_fetch_and_add(&this->in_index_[dst], 1);
                                auto &in_edge = this->in_edges_[pos];
                                in_edge.neighbour = src;
                                if (!std::is_same<EdgeData, Empty>::value) {
                                    in_edge.edge_data = out_edge.edge_data;
                                }
                            }
                        }
                    }
                }
                if (ShouldKillThisTask(task_ctx)) goto SNAPSHOT_PHASE2_ABORT;
                memmove(this->in_index_.Data() + 1, this->in_index_.Data(),
                        sizeof(size_t) * this->num_vertices_);
                this->in_index_[0] = 0;

            SNAPSHOT_PHASE2_ABORT:
                {};
            }
        });
        if (ShouldKillThisTask(task_ctx)) throw std::runtime_error("Task killed");
    }

    /**
     * @brief Generate a graph without LightningGraph. For V2 Procedures
     *
     * @exception std::runtime_error  Raised when a runtime error condition
     *                                occurs.
     *
     * @param [in,out]    txn             The transaction.
     * @param             flags           (Optional) The generation flags.
     * @param [in,out]    vertex_filter   (Optional) A function filtering
     *                                    vertices.
     * @param [in,out]    out_edge_filter (Optional) A function filtering out
     *                                    edges.
     **/
    OlapOnDB(Transaction &txn, size_t flags = 0,
             std::function<bool(VertexIterator &)> vertex_filter = nullptr,
             std::function<bool(OutEdgeIterator &, EdgeData &)> out_edge_filter = nullptr)
        : OlapOnDB(nullptr, txn, flags, vertex_filter, out_edge_filter) {}

    OlapOnDB() = delete;

    OlapOnDB(const OlapOnDB<EdgeData> &rhs) = delete;

    OlapOnDB(OlapOnDB<EdgeData> &&rhs) = default;
    OlapOnDB<EdgeData> &operator=(OlapOnDB<EdgeData> &&rhs) {
        printf("OlapOnDB assigment\n");
        return *this;
    }

    virtual ~OlapOnDB() {}

    /**
     * @brief    Extract a vertex array from the graph.
     *
     * @param    extract    The function describing the extraction logic.
     *
     * @return   A ParallelVector containing each vertex's extracted data.
     */
    template <typename VertexData>
    ParallelVector<VertexData> ExtractVertexData(
        std::function<void(VertexIterator &, VertexData &)> extract) {
        auto task_ctx = GetThreadContext();
        ParallelVector<VertexData> a(this->num_vertices_, this->num_vertices_);
        if (txn_.IsReadOnly() && db_ != nullptr) {
            auto worker = Worker::SharedWorker();
            worker->Delegate([&]() {
#pragma omp parallel
                {
                    auto txn = db_->ForkTxn(txn_);
                    int thread_id = omp_get_thread_num();
                    int num_threads = omp_get_num_threads();
                    size_t start = this->num_vertices_ / num_threads * thread_id;
                    size_t end = this->num_vertices_ / num_threads * (thread_id + 1);
                    if (thread_id == num_threads - 1) end = this->num_vertices_;
                    auto vit = txn.GetVertexIterator();
                    if (flags_ & SNAPSHOT_IDMAPPING) {
                        for (size_t vi = start; vi < end; vi++) {
                            if (vi % 1024 == 0 && ShouldKillThisTask(task_ctx)) break;
                            size_t original_vid = original_vids_[vi];
                            vit.Goto(original_vid);
                            extract(vit, a[vi]);
                        }
                    } else {
                        for (size_t vi = start; vi < end; vi++) {
                            if (vi % 1024 == 0 && ShouldKillThisTask(task_ctx)) break;
                            vit.Goto(vi);
                            extract(vit, a[vi]);
                        }
                    }
                }
            });
        } else {
            auto vit = txn_.GetVertexIterator();
            if (flags_ & SNAPSHOT_IDMAPPING) {
                for (size_t vi = 0; vi < this->num_vertices_; vi++) {
                    size_t original_vid = original_vids_[vi];
                    vit.Goto(original_vid);
                    extract(vit, a[vi]);
                }
            } else {
                for (size_t vi = 0; vi < this->num_vertices_; vi++) {
                    vit.Goto(vi);
                    extract(vit, a[vi]);
                }
            }
        }
        if (ShouldKillThisTask(task_ctx)) throw std::runtime_error("Task killed");
        return a;
    }

    /**
     * @brief    Write vertex data to a file.
     *
     * @param    vertex_data    The parallel vector storing the vertex data.
     * @param    output_file    The path to the output file.
     *
     */
    template <typename VertexData>
    void WriteToFile(ParallelVector<VertexData> &vertex_data, const std::string &output_file,
                     std::function<bool(size_t vid, VertexData &vdata)> output_filter = nullptr) {
        fma_common::OutputFmaStream fout;
        fout.Open(output_file, 64 << 20);
        for (size_t i = 0; i < this->num_vertices_; ++i) {
            if (output_filter != nullptr && !output_filter(i, vertex_data[i])) {
                continue;
            }
            std::string line =
                fma_common::StringFormatter::Format("{} {}\n", OriginalVid(i), vertex_data[i]);
            fout.Write(line.c_str(), line.size());
        }
    }

    /**
     * @brief    Write vertex data(include label、primary_field、field_data) to a file.
     *
     * @param    detail_output  always true
     * @param    vertex_data    The parallel vector storing the vertex data.
     * @param    output_file    The path to the output file.
     *
     */
    template <typename VertexData>
    void WriteToFile(bool detail_output, ParallelVector<VertexData> &vertex_data,
                     const std::string &output_file,
                     std::function<bool(size_t vid, VertexData &vdata)> output_filter = nullptr) {
        if (!detail_output) {
            THROW_CODE(InputError, "Just support deatail output!");
        }
        fma_common::OutputFmaStream fout;
        fout.Open(output_file, 64 << 20);
        for (size_t i = 0; i < this->num_vertices_; ++i) {
            if (output_filter != nullptr && !output_filter(i, vertex_data[i])) {
                continue;
            }
            auto vit = txn_.GetVertexIterator();
            vit.Goto(OriginalVid(i));
            if (vit.IsValid()) {
                auto vit_label = vit.GetLabel();
                auto primary_field = txn_.GetVertexPrimaryField(vit_label);
                auto field_data = vit.GetField(primary_field);
                json curJson;
                curJson["vid"] = OriginalVid(i);
                curJson["label"] = vit_label;
                curJson["primary_field"] = primary_field;
                curJson["field_data"] = field_data.ToString();
                curJson["result"] = vertex_data[i];
                auto content = curJson.dump() + "\n";
                fout.Write(content.c_str(), content.size());
            }
        }
    }

    /**
     * @brief    Write vertex data to the graph database.
     *
     * @param [in,out]   vertex_data    The parallel vector storing the vertex data.
     * @param [in]       vertex_field   The name of the vertex field.
     *
     */
    template <typename VertexData>
    void WriteToGraphDB(ParallelVector<VertexData> &vertex_data, const std::string &vertex_field) {
        if (db_ == nullptr) {
            throw std::runtime_error("can't write to graph because db is null");
        }
        auto write_txn = db_->CreateWriteTxn();
        auto vit = write_txn.GetVertexIterator();
        for (size_t i = 0; i < this->num_vertices_; i++) {
            FieldData local_distance(std::to_string(vertex_data[i]));
            vit.Goto(OriginalVid(i));
            if (vit.IsValid()) {
                vit.SetField(vertex_field, local_distance);
            }
        }
        write_txn.Commit();
    }

    /**
     * @brief    Get the original vertex id (in LightningGraph) of some vertex.
     *
     * @param    vid     The vertex id (in the graph) to access.
     *
     * @return   The original id of the specified vertex in the graph.
     */

    int64_t OriginalVid(size_t vid) {
        if (flags_ & SNAPSHOT_IDMAPPING) {
            return (int64_t)original_vids_[vid];
        } else {
            return (int64_t)vid;
        }
    }

    /**
     * @brief    Get the mapped vertex id (in the graph) of some vertex.
     *
     * @param    original_vid    The original vertex id (in LightningGraph) to
     *                           access.
     *
     * @return   The mapped id of the specified vertex (in LightningGraph).
     */
    size_t MappedVid(size_t original_vid) {
        if (flags_ & SNAPSHOT_IDMAPPING) {
            return vid_map_.find(original_vid);
        } else {
            return original_vid;
        }
    }

    Transaction &GetTransaction() { return txn_; }
};

/**
 * \brief   Default Parser for weighted edges for graph.
 *
 * \return  Edge is converted into graph or not.
 */
template <typename EdgeData>
std::function<bool(OutEdgeIterator &, EdgeData &)> edge_convert_default =
    [](OutEdgeIterator &eit, EdgeData &edge_data) -> bool {
    edge_data = 1;
    return true;
};

/**
 * \brief   Example parser for extract from edge property "weight"
 *
 * \return  Edge is converted into graph or not.
 */
template <typename EdgeData>
std::function<bool(OutEdgeIterator &, EdgeData &)> edge_convert_weight =
    [](OutEdgeIterator &eit, EdgeData &edge_data) -> bool {
    edge_data = eit.GetField("weight").real();
    return true;
};

}  // namespace olap
}  // namespace lgraph_api
