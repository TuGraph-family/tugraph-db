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

#include "lgraph/lgraph_traversal.h"

namespace lgraph_api {

namespace traversal {

ParallelVector<size_t> FindVertices(GraphDB &db, Transaction &txn,
                                    std::function<bool(VertexIterator &)> filter, bool parallel) {
    auto task_ctx = GetThreadContext();
    GraphDB &db_ = db;
    Transaction &txn_ = txn;
    size_t num_vertices_ = txn.GetNumVertices();
    ParallelVector<size_t> frontier(num_vertices_);
    if (parallel && txn.IsReadOnly()) {
        auto worker = Worker::SharedWorker();
        worker->Delegate([&]() {
#pragma omp parallel
            {
                constexpr size_t LOCAL_BUFFER_SIZE = 1024;
                ParallelVector<size_t> local_frontier(LOCAL_BUFFER_SIZE);
                auto txn = db_.ForkTxn(txn_);
                int thread_id = omp_get_thread_num();
                int num_threads = omp_get_num_threads();
                size_t start = num_vertices_ / num_threads * thread_id;
                size_t end = num_vertices_ / num_threads * (thread_id + 1);
                if (thread_id == num_threads - 1) end = num_vertices_;
                auto vit = txn.GetVertexIterator(start, true);
                for (size_t i = 0; vit.IsValid(); vit.Next(), i++) {
                    if (i % 1024 == 0 && ShouldKillThisTask(task_ctx)) break;
                    size_t vid = vit.GetId();
                    if (vid >= end) break;
                    if (filter(vit)) {
                        local_frontier.Append(vid, false);
                        if (local_frontier.Size() == LOCAL_BUFFER_SIZE) {
                            frontier.Append(local_frontier);
                            local_frontier.Clear();
                        }
                    }
                }
                if (local_frontier.Size() != 0) {
                    frontier.Append(local_frontier);
                    local_frontier.Clear();
                }
            }
        });
    } else {
        for (auto vit = txn.GetVertexIterator(); vit.IsValid(); vit.Next()) {
            if (filter(vit)) frontier.Append(vit.GetId());
        }
    }
    if (ShouldKillThisTask(task_ctx)) throw std::runtime_error("Task killed");
    return frontier;
}

FrontierTraversal::FrontierTraversal(GraphDB &db, Transaction &txn, size_t flags, size_t capacity)
    : db_(db),
      txn_(txn),
      flags_(flags),
      num_vertices_(txn.GetNumVertices()),
      curr_frontier_(capacity),
      next_frontier_(capacity),
      visited_(num_vertices_) {}

void FrontierTraversal::Reset() {
    curr_frontier_.Clear();
    visited_.Clear();
}

void FrontierTraversal::ResetVisited() { visited_.Clear(); }

ParallelVector<size_t> &FrontierTraversal::GetFrontier() { return curr_frontier_; }

void FrontierTraversal::SetFrontier(size_t root_vid) {
    Reset();
    curr_frontier_.Append(root_vid, false);
    if (flags_ & TRAVERSAL_ALLOW_REVISITS) return;
    visited_.Add(root_vid);
}

void FrontierTraversal::SetFrontier(ParallelVector<size_t> &root_vids) {
    Reset();
    curr_frontier_.Append(root_vids, false);
    if (flags_ & TRAVERSAL_ALLOW_REVISITS) return;
    for (auto root_vid : root_vids) {
        visited_.Add(root_vid);
    }
}

void FrontierTraversal::SetFrontier(std::function<bool(VertexIterator &)> root_vertex_filter) {
    auto task_ctx = GetThreadContext();
    Reset();
    if ((flags_ & TRAVERSAL_PARALLEL) && txn_.IsReadOnly()) {
        auto worker = Worker::SharedWorker();
        worker->Delegate([&]() {
#pragma omp parallel
            {
                constexpr size_t LOCAL_BUFFER_SIZE = 1024;
                ParallelVector<size_t> local_frontier(LOCAL_BUFFER_SIZE);
                auto txn = db_.ForkTxn(txn_);
                int thread_id = omp_get_thread_num();
                int num_threads = omp_get_num_threads();
                size_t start = num_vertices_ / num_threads * thread_id;
                size_t end = num_vertices_ / num_threads * (thread_id + 1);
                if (thread_id == num_threads - 1) end = num_vertices_;
                auto vit = txn.GetVertexIterator(start, true);
                for (size_t i = 0; vit.IsValid(); vit.Next(), i++) {
                    if (i % 1024 == 0 && ShouldKillThisTask(task_ctx)) break;
                    size_t vid = vit.GetId();
                    if (vid >= end) break;
                    if (root_vertex_filter(vit)) {
                        local_frontier.Append(vid, false);
                        if (local_frontier.Size() == LOCAL_BUFFER_SIZE) {
                            curr_frontier_.Append(local_frontier);
                            local_frontier.Clear();
                        }
                        if ((flags_ & TRAVERSAL_ALLOW_REVISITS) == 0) {
                            visited_.Add(vid);
                        }
                    }
                }
                if (local_frontier.Size() != 0) {
                    curr_frontier_.Append(local_frontier);
                    local_frontier.Clear();
                }
            }
        });
    } else {
        for (auto vit = txn_.GetVertexIterator(); vit.IsValid(); vit.Next()) {
            if (root_vertex_filter(vit)) {
                size_t vid = vit.GetId();
                curr_frontier_.Append(vid, false);
                if ((flags_ & TRAVERSAL_ALLOW_REVISITS) == 0) {
                    visited_.Add(vid);
                }
            }
        }
    }
    if (ShouldKillThisTask(task_ctx)) throw std::runtime_error("Task killed");
}

void FrontierTraversal::ExpandOutEdges(std::function<bool(OutEdgeIterator &)> out_edge_filter,
                                       std::function<bool(VertexIterator &)> out_neighbour_filter) {
    auto task_ctx = GetThreadContext();
    constexpr size_t PARALLEL_THRESHOLD = 256;
    next_frontier_.Clear();
    if ((flags_ & TRAVERSAL_PARALLEL) && txn_.IsReadOnly() &&
        curr_frontier_.Size() > PARALLEL_THRESHOLD) {
        auto worker = Worker::SharedWorker();
        worker->Delegate([&]() {
            int num_threads = 0;
#pragma omp parallel
            {
                if (omp_get_thread_num() == 0) {
                    num_threads = omp_get_num_threads();
                }
            };
            size_t *start = new size_t[num_threads];
            size_t *end = new size_t[num_threads];
            for (int thread_id = 0; thread_id < num_threads; thread_id++) {
                start[thread_id] = curr_frontier_.Size() / num_threads * thread_id;
                end[thread_id] = curr_frontier_.Size() / num_threads * (thread_id + 1);
                if (thread_id == num_threads - 1) end[thread_id] = curr_frontier_.Size();
            }
            constexpr size_t LOCAL_BUFFER_SIZE = 1024;
            constexpr size_t WORK_BATCH = 64;
#pragma omp parallel
            {
                ParallelVector<size_t> local_frontier(LOCAL_BUFFER_SIZE);
                auto txn = db_.ForkTxn(txn_);
                int thread_id = omp_get_thread_num();
                auto vit = txn.GetVertexIterator();
                for (int thread_offset = 0; thread_offset < num_threads; thread_offset++) {
                    if (ShouldKillThisTask(task_ctx)) break;
                    while (true) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t batch_start = __sync_fetch_and_add(&start[thread_id], WORK_BATCH);
                        if (batch_start >= end[thread_id]) break;
                        size_t batch_end = batch_start + WORK_BATCH;
                        if (batch_end > end[thread_id]) batch_end = end[thread_id];
                        for (size_t i = batch_start; i < batch_end; i += 1) {
                            size_t vid = curr_frontier_[i];
                            vit.Goto(vid);
                            for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                                if (!out_edge_filter || out_edge_filter(eit)) {
                                    size_t nbr = eit.GetDst();
                                    bool ok = out_neighbour_filter == nullptr;
                                    if (!ok) {
                                        auto nbr_vit = txn.GetVertexIterator(nbr);
                                        ok = out_neighbour_filter(nbr_vit);
                                    }
                                    if (ok) {
                                        if (flags_ & TRAVERSAL_ALLOW_REVISITS) {
                                            local_frontier.Append(nbr, false);
                                        } else {
                                            if (!visited_.Has(nbr) && visited_.Add(nbr)) {
                                                local_frontier.Append(nbr, false);
                                            }
                                        }
                                        if (local_frontier.Size() == LOCAL_BUFFER_SIZE) {
                                            next_frontier_.Append(local_frontier);
                                            local_frontier.Clear();
                                        }
                                    }
                                }
                            }
                        }
                    }
                    thread_id = (thread_id + 1) % num_threads;
                }
                if (local_frontier.Size() != 0) {
                    next_frontier_.Append(local_frontier);
                    local_frontier.Clear();
                }
            }
            delete[] start;
            delete[] end;
        });
    } else {
        auto vit = txn_.GetVertexIterator();
        for (auto vid : curr_frontier_) {
            vit.Goto(vid);
            for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                if (!out_edge_filter || out_edge_filter(eit)) {
                    size_t nbr = eit.GetDst();
                    bool ok = out_neighbour_filter == nullptr;
                    if (!ok) {
                        auto nbr_vit = txn_.GetVertexIterator(nbr);
                        ok = out_neighbour_filter(nbr_vit);
                    }
                    if (ok) {
                        if (flags_ & TRAVERSAL_ALLOW_REVISITS) {
                            next_frontier_.Append(nbr, false);
                        } else {
                            if (!visited_.Has(nbr) && visited_.Add(nbr)) {
                                next_frontier_.Append(nbr, false);
                            }
                        }
                    }
                }
            }
        }
    }
    curr_frontier_.Swap(next_frontier_);
    if (ShouldKillThisTask(task_ctx)) throw std::runtime_error("Task killed");
}

void FrontierTraversal::ExpandInEdges(std::function<bool(InEdgeIterator &)> in_edge_filter,
                                      std::function<bool(VertexIterator &)> in_neighbour_filter) {
    auto task_ctx = GetThreadContext();
    constexpr size_t PARALLEL_THRESHOLD = 256;
    next_frontier_.Clear();
    if ((flags_ & TRAVERSAL_PARALLEL) && txn_.IsReadOnly() &&
        curr_frontier_.Size() > PARALLEL_THRESHOLD) {
        auto worker = Worker::SharedWorker();
        worker->Delegate([&]() {
            int num_threads = 0;
#pragma omp parallel
            {
                if (omp_get_thread_num() == 0) {
                    num_threads = omp_get_num_threads();
                }
            };
            size_t *start = new size_t[num_threads];
            size_t *end = new size_t[num_threads];
            for (int thread_id = 0; thread_id < num_threads; thread_id++) {
                start[thread_id] = curr_frontier_.Size() / num_threads * thread_id;
                end[thread_id] = curr_frontier_.Size() / num_threads * (thread_id + 1);
                if (thread_id == num_threads - 1) end[thread_id] = curr_frontier_.Size();
            }
            constexpr size_t LOCAL_BUFFER_SIZE = 1024;
            constexpr size_t WORK_BATCH = 64;
#pragma omp parallel
            {
                ParallelVector<size_t> local_frontier(LOCAL_BUFFER_SIZE);
                auto txn = db_.ForkTxn(txn_);
                int thread_id = omp_get_thread_num();
                auto vit = txn.GetVertexIterator();
                for (int thread_offset = 0; thread_offset < num_threads; thread_offset++) {
                    if (ShouldKillThisTask(task_ctx)) break;
                    while (true) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t batch_start = __sync_fetch_and_add(&start[thread_id], WORK_BATCH);
                        if (batch_start >= end[thread_id]) break;
                        size_t batch_end = batch_start + WORK_BATCH;
                        if (batch_end > end[thread_id]) batch_end = end[thread_id];
                        for (size_t i = batch_start; i < batch_end; i += 1) {
                            size_t vid = curr_frontier_[i];
                            vit.Goto(vid);
                            for (auto eit = vit.GetInEdgeIterator(); eit.IsValid(); eit.Next()) {
                                if (!in_edge_filter || in_edge_filter(eit)) {
                                    size_t nbr = eit.GetSrc();
                                    bool ok = in_neighbour_filter == nullptr;
                                    if (!ok) {
                                        auto nbr_vit = txn.GetVertexIterator(nbr);
                                        ok = in_neighbour_filter(nbr_vit);
                                    }
                                    if (ok) {
                                        if (flags_ & TRAVERSAL_ALLOW_REVISITS) {
                                            local_frontier.Append(nbr, false);
                                        } else {
                                            if (!visited_.Has(nbr) && visited_.Add(nbr)) {
                                                local_frontier.Append(nbr, false);
                                            }
                                        }
                                        if (local_frontier.Size() == LOCAL_BUFFER_SIZE) {
                                            next_frontier_.Append(local_frontier);
                                            local_frontier.Clear();
                                        }
                                    }
                                }
                            }
                        }
                    }
                    thread_id = (thread_id + 1) % num_threads;
                }
                if (local_frontier.Size() != 0) {
                    next_frontier_.Append(local_frontier);
                    local_frontier.Clear();
                }
            }
            delete[] start;
            delete[] end;
        });
    } else {
        auto vit = txn_.GetVertexIterator();
        for (auto vid : curr_frontier_) {
            vit.Goto(vid);
            for (auto eit = vit.GetInEdgeIterator(); eit.IsValid(); eit.Next()) {
                if (!in_edge_filter || in_edge_filter(eit)) {
                    size_t nbr = eit.GetSrc();
                    bool ok = in_neighbour_filter == nullptr;
                    if (!ok) {
                        auto nbr_vit = txn_.GetVertexIterator(nbr);
                        ok = in_neighbour_filter(nbr_vit);
                    }
                    if (ok) {
                        if (flags_ & TRAVERSAL_ALLOW_REVISITS) {
                            next_frontier_.Append(nbr, false);
                        } else {
                            if (!visited_.Has(nbr) && visited_.Add(nbr)) {
                                next_frontier_.Append(nbr, false);
                            }
                        }
                    }
                }
            }
        }
    }
    curr_frontier_.Swap(next_frontier_);
    if (ShouldKillThisTask(task_ctx)) throw std::runtime_error("Task killed");
}

void FrontierTraversal::ExpandEdges(std::function<bool(OutEdgeIterator &)> out_edge_filter,
                                    std::function<bool(InEdgeIterator &)> in_edge_filter,
                                    std::function<bool(VertexIterator &)> out_neighbour_filter,
                                    std::function<bool(VertexIterator &)> in_neighbour_filter) {
    auto task_ctx = GetThreadContext();
    constexpr size_t PARALLEL_THRESHOLD = 256;
    next_frontier_.Clear();
    if ((flags_ & TRAVERSAL_PARALLEL) && txn_.IsReadOnly() &&
        curr_frontier_.Size() > PARALLEL_THRESHOLD) {
        auto worker = Worker::SharedWorker();
        worker->Delegate([&]() {
            int num_threads = 0;
#pragma omp parallel
            {
                if (omp_get_thread_num() == 0) {
                    num_threads = omp_get_num_threads();
                }
            };
            size_t *start = new size_t[num_threads];
            size_t *end = new size_t[num_threads];
            for (int thread_id = 0; thread_id < num_threads; thread_id++) {
                start[thread_id] = curr_frontier_.Size() / num_threads * thread_id;
                end[thread_id] = curr_frontier_.Size() / num_threads * (thread_id + 1);
                if (thread_id == num_threads - 1) end[thread_id] = curr_frontier_.Size();
            }
            constexpr size_t LOCAL_BUFFER_SIZE = 1024;
            constexpr size_t WORK_BATCH = 64;
#pragma omp parallel
            {
                ParallelVector<size_t> local_frontier(LOCAL_BUFFER_SIZE);
                auto txn = db_.ForkTxn(txn_);
                int thread_id = omp_get_thread_num();
                auto vit = txn.GetVertexIterator();
                for (int thread_offset = 0; thread_offset < num_threads; thread_offset++) {
                    if (ShouldKillThisTask(task_ctx)) break;
                    while (true) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t batch_start = __sync_fetch_and_add(&start[thread_id], WORK_BATCH);
                        if (batch_start >= end[thread_id]) break;
                        size_t batch_end = batch_start + WORK_BATCH;
                        if (batch_end > end[thread_id]) batch_end = end[thread_id];
                        for (size_t i = batch_start; i < batch_end; i += 1) {
                            size_t vid = curr_frontier_[i];
                            vit.Goto(vid);
                            for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                                if (!out_edge_filter || out_edge_filter(eit)) {
                                    size_t nbr = eit.GetDst();
                                    bool ok = out_neighbour_filter == nullptr;
                                    if (!ok) {
                                        auto nbr_vit = txn.GetVertexIterator(nbr);
                                        ok = out_neighbour_filter(nbr_vit);
                                    }
                                    if (ok) {
                                        if (flags_ & TRAVERSAL_ALLOW_REVISITS) {
                                            local_frontier.Append(nbr, false);
                                        } else {
                                            if (!visited_.Has(nbr) && visited_.Add(nbr)) {
                                                local_frontier.Append(nbr, false);
                                            }
                                        }
                                        if (local_frontier.Size() == LOCAL_BUFFER_SIZE) {
                                            next_frontier_.Append(local_frontier);
                                            local_frontier.Clear();
                                        }
                                    }
                                }
                            }
                            for (auto eit = vit.GetInEdgeIterator(); eit.IsValid(); eit.Next()) {
                                if (!in_edge_filter || in_edge_filter(eit)) {
                                    size_t nbr = eit.GetSrc();
                                    bool ok = in_neighbour_filter == nullptr;
                                    if (!ok) {
                                        auto nbr_vit = txn.GetVertexIterator(nbr);
                                        ok = in_neighbour_filter(nbr_vit);
                                    }
                                    if (ok) {
                                        if (flags_ & TRAVERSAL_ALLOW_REVISITS) {
                                            local_frontier.Append(nbr, false);
                                        } else {
                                            if (!visited_.Has(nbr) && visited_.Add(nbr)) {
                                                local_frontier.Append(nbr, false);
                                            }
                                        }
                                        if (local_frontier.Size() == LOCAL_BUFFER_SIZE) {
                                            next_frontier_.Append(local_frontier);
                                            local_frontier.Clear();
                                        }
                                    }
                                }
                            }
                        }
                    }
                    thread_id = (thread_id + 1) % num_threads;
                }
                if (local_frontier.Size() != 0) {
                    next_frontier_.Append(local_frontier);
                    local_frontier.Clear();
                }
            }
            delete[] start;
            delete[] end;
        });
    } else {
        auto vit = txn_.GetVertexIterator();
        for (auto vid : curr_frontier_) {
            vit.Goto(vid);
            for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                if (!out_edge_filter || out_edge_filter(eit)) {
                    size_t nbr = eit.GetDst();
                    bool ok = out_neighbour_filter == nullptr;
                    if (!ok) {
                        auto nbr_vit = txn_.GetVertexIterator(nbr);
                        ok = out_neighbour_filter(nbr_vit);
                    }
                    if (ok) {
                        if (flags_ & TRAVERSAL_ALLOW_REVISITS) {
                            next_frontier_.Append(nbr, false);
                        } else {
                            if (!visited_.Has(nbr) && visited_.Add(nbr)) {
                                next_frontier_.Append(nbr, false);
                            }
                        }
                    }
                }
            }
            for (auto eit = vit.GetInEdgeIterator(); eit.IsValid(); eit.Next()) {
                if (!in_edge_filter || in_edge_filter(eit)) {
                    size_t nbr = eit.GetSrc();
                    bool ok = in_neighbour_filter == nullptr;
                    if (!ok) {
                        auto nbr_vit = txn_.GetVertexIterator(nbr);
                        ok = in_neighbour_filter(nbr_vit);
                    }
                    if (ok) {
                        if (flags_ & TRAVERSAL_ALLOW_REVISITS) {
                            next_frontier_.Append(nbr, false);
                        } else {
                            if (!visited_.Has(nbr) && visited_.Add(nbr)) {
                                next_frontier_.Append(nbr, false);
                            }
                        }
                    }
                }
            }
        }
    }
    curr_frontier_.Swap(next_frontier_);
    if (ShouldKillThisTask(task_ctx)) throw std::runtime_error("Task killed");
}

Vertex::Vertex(size_t vid) : vid_(vid) {}

size_t Vertex::GetId() const { return vid_; }

Edge::Edge(size_t start, uint16_t lid, uint64_t tid, size_t end, size_t eid, bool forward)
    : start_(start), end_(end), eid_(eid), lid_(lid), tid_(tid), forward_(forward) {}

Vertex Edge::GetStartVertex() const { return Vertex(start_); }

Vertex Edge::GetEndVertex() const { return Vertex(end_); }

uint16_t Edge::GetLabelId() const { return lid_; }

size_t Edge::GetEdgeId() const { return eid_; }

uint64_t Edge::GetTemporalId() const { return tid_; }

bool Edge::IsForward() const { return forward_; }

Vertex Edge::GetSrcVertex() const { return Vertex(forward_ ? start_ : end_); }

Vertex Edge::GetDstVertex() const { return Vertex(forward_ ? end_ : start_); }

bool operator==(const Vertex &lhs, const Vertex &rhs) { return lhs.GetId() == rhs.GetId(); }

bool operator!=(const Vertex &lhs, const Vertex &rhs) { return lhs.GetId() != rhs.GetId(); }

bool operator==(const Edge &lhs, const Edge &rhs) {
    return lhs.GetSrcVertex() == rhs.GetSrcVertex() && lhs.GetDstVertex() == rhs.GetDstVertex() &&
           lhs.GetEdgeId() == rhs.GetEdgeId();
}

bool operator!=(const Edge &lhs, const Edge &rhs) {
    return lhs.GetSrcVertex() != rhs.GetSrcVertex() || lhs.GetDstVertex() != rhs.GetDstVertex() ||
           lhs.GetEdgeId() != rhs.GetEdgeId();
}

Path::Path(const Vertex &start) : dirs_(), lids_(), ids_() {
    ids_.push_back(start.vid_);
    // vid, eid, vid, ..., eid, vid
}

Path::Path(const Path &rhs) : dirs_(), lids_(), ids_() {
    dirs_.resize(rhs.dirs_.size());
    for (size_t i = 0; i < rhs.dirs_.size(); i++) {
        dirs_[i] = rhs.dirs_[i];
    }
    lids_.resize(rhs.lids_.size());
    for (size_t i = 0; i < rhs.lids_.size(); i++) {
        lids_[i] = rhs.lids_[i];
    }
    ids_.resize(rhs.ids_.size());
    for (size_t i = 0; i < rhs.ids_.size(); i++) {
        ids_[i] = rhs.ids_[i];
    }
}

Path &Path::operator=(const Path &rhs) {
    dirs_.resize(rhs.dirs_.size());
    for (size_t i = 0; i < rhs.dirs_.size(); i++) {
        dirs_[i] = rhs.dirs_[i];
    }
    lids_.resize(rhs.lids_.size());
    for (size_t i = 0; i < rhs.lids_.size(); i++) {
        lids_[i] = rhs.lids_[i];
    }
    ids_.resize(rhs.ids_.size());
    for (size_t i = 0; i < rhs.ids_.size(); i++) {
        ids_[i] = rhs.ids_[i];
    }
    return *this;
}

size_t Path::Length() const { return dirs_.size(); }

void Path::Append(const Edge &edge) {
    if (ids_.back() != edge.start_) {
        throw std::runtime_error("The edge's start doesn't match the path's end.");
    }
    dirs_.push_back(edge.forward_);
    lids_.push_back(edge.lid_);
    ids_.push_back(edge.eid_);
    ids_.push_back(edge.end_);
}

Vertex Path::GetStartVertex() const { return Vertex(ids_.front()); }

Vertex Path::GetEndVertex() const { return Vertex(ids_.back()); }

Edge Path::GetLastEdge() const {
    size_t length = dirs_.size();
    if (length == 0) {
        throw std::runtime_error("The path contains only a single vertex.");
    }
    return Edge(ids_[0 + (length - 1) * 2], lids_.back(), 0, ids_[2 + (length - 1) * 2],
                ids_[1 + (length - 1) * 2], dirs_.back());
}

Edge Path::GetNthEdge(size_t n) const {
    size_t length = dirs_.size();
    if (n >= length) {
        throw std::runtime_error("Access out of range.");
    }
    return Edge(ids_[0 + n * 2], lids_[n], 0, ids_[2 + n * 2], ids_[1 + n * 2], dirs_[n]);
}

Vertex Path::GetNthVertex(size_t n) const {
    size_t length = dirs_.size();
    if (n > length) {
        throw std::runtime_error("Access out of range.");
    }
    return Vertex(ids_[0 + n * 2]);
}

IteratorHelper::IteratorHelper(Transaction &txn) : txn_(txn) {}

VertexIterator IteratorHelper::Cast(const Vertex &vertex) {
    return txn_.GetVertexIterator(vertex.vid_);
}

OutEdgeIterator IteratorHelper::Cast(const Edge &edge) {
    if (edge.forward_) {
        return txn_.GetOutEdgeIterator(
            EdgeUid(edge.start_, edge.end_, edge.lid_, edge.tid_, edge.eid_));
    } else {
        return txn_.GetOutEdgeIterator(
            EdgeUid(edge.end_, edge.start_, edge.lid_, edge.tid_, edge.eid_));
    }
}

PathTraversal::PathTraversal(GraphDB &db, Transaction &txn, size_t flags, size_t capacity)
    : db_(db),
      txn_(txn),
      flags_(flags),
      num_vertices_(txn.GetNumVertices()),
      curr_frontier_(capacity),
      next_frontier_(capacity) {}

void PathTraversal::Reset() { curr_frontier_.Clear(); }

ParallelVector<Path> &PathTraversal::GetFrontier() { return curr_frontier_; }

void PathTraversal::SetFrontier(size_t root_vid) {
    Reset();
    curr_frontier_.Append(Path(Vertex(root_vid)), false);
}

void PathTraversal::SetFrontier(ParallelVector<size_t> &root_vids) {
    Reset();
    for (size_t root_vid : root_vids) {
        curr_frontier_.Append(Path(Vertex(root_vid)), false);
    }
}

void PathTraversal::SetFrontier(std::function<bool(VertexIterator &)> root_vertex_filter) {
    auto task_ctx = GetThreadContext();
    Reset();
    if ((flags_ & TRAVERSAL_PARALLEL) && txn_.IsReadOnly()) {
        auto worker = Worker::SharedWorker();
        worker->Delegate([&]() {
#pragma omp parallel
            {
                constexpr size_t LOCAL_BUFFER_SIZE = 1024;
                ParallelVector<Path> local_frontier(LOCAL_BUFFER_SIZE);
                auto txn = db_.ForkTxn(txn_);
                int thread_id = omp_get_thread_num();
                int num_threads = omp_get_num_threads();
                size_t start = num_vertices_ / num_threads * thread_id;
                size_t end = num_vertices_ / num_threads * (thread_id + 1);
                if (thread_id == num_threads - 1) end = num_vertices_;
                auto vit = txn.GetVertexIterator(start, true);
                for (size_t i = 0; vit.IsValid(); vit.Next(), i++) {
                    if (i % 1024 == 0 && ShouldKillThisTask(task_ctx)) break;
                    size_t vid = vit.GetId();
                    if (vid >= end) break;
                    if (root_vertex_filter(vit)) {
                        local_frontier.Append(Path(Vertex(vid)), false);
                        if (local_frontier.Size() == LOCAL_BUFFER_SIZE) {
                            // TODO: avoid copy construction of Path // NOLINT
                            curr_frontier_.Append(local_frontier);
                            local_frontier.Clear();
                        }
                    }
                }
                if (local_frontier.Size() != 0) {
                    // TODO: avoid copy construction of Path // NOLINT
                    curr_frontier_.Append(local_frontier);
                    local_frontier.Clear();
                }
            }
        });
    } else {
        for (auto vit = txn_.GetVertexIterator(); vit.IsValid(); vit.Next()) {
            if (root_vertex_filter(vit)) {
                size_t vid = vit.GetId();
                curr_frontier_.Append(Path(Vertex(vid)), false);
            }
        }
    }
    if (ShouldKillThisTask(task_ctx)) throw std::runtime_error("Task killed");
}

void PathTraversal::ExpandOutEdges(
    std::function<bool(OutEdgeIterator &, Path &, IteratorHelper &)> out_edge_filter,
    std::function<bool(VertexIterator &, Path &, IteratorHelper &)> out_neighbour_filter) {
    auto task_ctx = GetThreadContext();
    constexpr size_t PARALLEL_THRESHOLD = 256;
    next_frontier_.Clear();
    if ((flags_ & TRAVERSAL_PARALLEL) && txn_.IsReadOnly() &&
        curr_frontier_.Size() > PARALLEL_THRESHOLD) {
        auto worker = Worker::SharedWorker();
        worker->Delegate([&]() {
            int num_threads = 0;
#pragma omp parallel
            {
                if (omp_get_thread_num() == 0) {
                    num_threads = omp_get_num_threads();
                }
            };
            size_t *start = new size_t[num_threads];
            size_t *end = new size_t[num_threads];
            for (int thread_id = 0; thread_id < num_threads; thread_id++) {
                start[thread_id] = curr_frontier_.Size() / num_threads * thread_id;
                end[thread_id] = curr_frontier_.Size() / num_threads * (thread_id + 1);
                if (thread_id == num_threads - 1) end[thread_id] = curr_frontier_.Size();
            }
            constexpr size_t LOCAL_BUFFER_SIZE = 1024;
            constexpr size_t WORK_BATCH = 64;
#pragma omp parallel
            {
                ParallelVector<Path> local_frontier(LOCAL_BUFFER_SIZE);
                auto txn = db_.ForkTxn(txn_);
                IteratorHelper helper(txn_);
                int thread_id = omp_get_thread_num();
                auto vit = txn.GetVertexIterator();
                for (int thread_offset = 0; thread_offset < num_threads; thread_offset++) {
                    if (ShouldKillThisTask(task_ctx)) break;
                    while (true) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t batch_start = __sync_fetch_and_add(&start[thread_id], WORK_BATCH);
                        if (batch_start >= end[thread_id]) break;
                        size_t batch_end = batch_start + WORK_BATCH;
                        if (batch_end > end[thread_id]) batch_end = end[thread_id];
                        for (size_t i = batch_start; i < batch_end; i += 1) {
                            Path &path = curr_frontier_[i];
                            size_t vid = path.GetEndVertex().GetId();
                            vit.Goto(vid);
                            for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                                if (!out_edge_filter || out_edge_filter(eit, path, helper)) {
                                    size_t nbr = eit.GetDst();
                                    bool ok = out_neighbour_filter == nullptr;
                                    if (!ok) {
                                        auto nbr_vit = txn.GetVertexIterator(nbr);
                                        ok = out_neighbour_filter(nbr_vit, path, helper);
                                    }
                                    if (ok) {
                                        local_frontier.Append(path, false);
                                        local_frontier.Back().Append(Edge(vid, eit.GetLabelId(),
                                                                          eit.GetTemporalId(), nbr,
                                                                          eit.GetEdgeId(), true));
                                        if (local_frontier.Size() == LOCAL_BUFFER_SIZE) {
                                            next_frontier_.Append(local_frontier);
                                            local_frontier.Clear();
                                        }
                                    }
                                }
                            }
                        }
                    }
                    thread_id = (thread_id + 1) % num_threads;
                }
                if (local_frontier.Size() != 0) {
                    next_frontier_.Append(local_frontier);
                    local_frontier.Clear();
                }
            }
            delete[] start;
            delete[] end;
        });
    } else {
        IteratorHelper helper(txn_);
        auto vit = txn_.GetVertexIterator();
        for (auto &path : curr_frontier_) {
            size_t vid = path.GetEndVertex().GetId();
            vit.Goto(vid);
            for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                if (!out_edge_filter || out_edge_filter(eit, path, helper)) {
                    size_t nbr = eit.GetDst();
                    bool ok = out_neighbour_filter == nullptr;
                    if (!ok) {
                        auto nbr_vit = txn_.GetVertexIterator(nbr);
                        ok = out_neighbour_filter(nbr_vit, path, helper);
                    }
                    if (ok) {
                        next_frontier_.Append(path, false);
                        next_frontier_.Back().Append(Edge(vid, eit.GetLabelId(),
                                                          eit.GetTemporalId(),
                                                          nbr, eit.GetEdgeId(), true));
                    }
                }
            }
        }
    }
    curr_frontier_.Swap(next_frontier_);
    if (ShouldKillThisTask(task_ctx)) throw std::runtime_error("Task killed");
}

void PathTraversal::ExpandInEdges(
    std::function<bool(InEdgeIterator &, Path &, IteratorHelper &)> in_edge_filter,
    std::function<bool(VertexIterator &, Path &, IteratorHelper &)> in_neighbour_filter) {
    auto task_ctx = GetThreadContext();
    constexpr size_t PARALLEL_THRESHOLD = 256;
    next_frontier_.Clear();
    if ((flags_ & TRAVERSAL_PARALLEL) && txn_.IsReadOnly() &&
        curr_frontier_.Size() > PARALLEL_THRESHOLD) {
        auto worker = Worker::SharedWorker();
        worker->Delegate([&]() {
            int num_threads = 0;
#pragma omp parallel
            {
                if (omp_get_thread_num() == 0) {
                    num_threads = omp_get_num_threads();
                }
            };
            size_t *start = new size_t[num_threads];
            size_t *end = new size_t[num_threads];
            for (int thread_id = 0; thread_id < num_threads; thread_id++) {
                start[thread_id] = curr_frontier_.Size() / num_threads * thread_id;
                end[thread_id] = curr_frontier_.Size() / num_threads * (thread_id + 1);
                if (thread_id == num_threads - 1) end[thread_id] = curr_frontier_.Size();
            }
            constexpr size_t LOCAL_BUFFER_SIZE = 1024;
            constexpr size_t WORK_BATCH = 64;
#pragma omp parallel
            {
                ParallelVector<Path> local_frontier(LOCAL_BUFFER_SIZE);
                auto txn = db_.ForkTxn(txn_);
                IteratorHelper helper(txn);
                int thread_id = omp_get_thread_num();
                auto vit = txn.GetVertexIterator();
                for (int thread_offset = 0; thread_offset < num_threads; thread_offset++) {
                    if (ShouldKillThisTask(task_ctx)) break;
                    while (true) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t batch_start = __sync_fetch_and_add(&start[thread_id], WORK_BATCH);
                        if (batch_start >= end[thread_id]) break;
                        size_t batch_end = batch_start + WORK_BATCH;
                        if (batch_end > end[thread_id]) batch_end = end[thread_id];
                        for (size_t i = batch_start; i < batch_end; i += 1) {
                            Path &path = curr_frontier_[i];
                            size_t vid = path.GetEndVertex().GetId();
                            vit.Goto(vid);
                            for (auto eit = vit.GetInEdgeIterator(); eit.IsValid(); eit.Next()) {
                                if (!in_edge_filter || in_edge_filter(eit, path, helper)) {
                                    size_t nbr = eit.GetSrc();
                                    bool ok = in_neighbour_filter == nullptr;
                                    if (!ok) {
                                        auto nbr_vit = txn.GetVertexIterator(nbr);
                                        ok = in_neighbour_filter(nbr_vit, path, helper);
                                    }
                                    if (ok) {
                                        local_frontier.Append(path, false);
                                        local_frontier.Back().Append(Edge(vid, eit.GetLabelId(),
                                                                          eit.GetTemporalId(), nbr,
                                                                          eit.GetEdgeId(), false));
                                        if (local_frontier.Size() == LOCAL_BUFFER_SIZE) {
                                            next_frontier_.Append(local_frontier);
                                            local_frontier.Clear();
                                        }
                                    }
                                }
                            }
                        }
                    }
                    thread_id = (thread_id + 1) % num_threads;
                }
                if (local_frontier.Size() != 0) {
                    next_frontier_.Append(local_frontier);
                    local_frontier.Clear();
                }
            }
            delete[] start;
            delete[] end;
        });
    } else {
        IteratorHelper helper(txn_);
        auto vit = txn_.GetVertexIterator();
        for (auto &path : curr_frontier_) {
            size_t vid = path.GetEndVertex().GetId();
            vit.Goto(vid);
            for (auto eit = vit.GetInEdgeIterator(); eit.IsValid(); eit.Next()) {
                if (!in_edge_filter || in_edge_filter(eit, path, helper)) {
                    size_t nbr = eit.GetSrc();
                    bool ok = in_neighbour_filter == nullptr;
                    if (!ok) {
                        auto nbr_vit = txn_.GetVertexIterator(nbr);
                        ok = in_neighbour_filter(nbr_vit, path, helper);
                    }
                    if (ok) {
                        next_frontier_.Append(path, false);
                        next_frontier_.Back().Append(Edge(vid, eit.GetLabelId(),
                                                          eit.GetTemporalId(),
                                                          nbr, eit.GetEdgeId(), false));
                    }
                }
            }
        }
    }
    curr_frontier_.Swap(next_frontier_);
    if (ShouldKillThisTask(task_ctx)) throw std::runtime_error("Task killed");
}

void PathTraversal::ExpandEdges(
    std::function<bool(OutEdgeIterator &, Path &, IteratorHelper &)> out_edge_filter,
    std::function<bool(InEdgeIterator &, Path &, IteratorHelper &)> in_edge_filter,
    std::function<bool(VertexIterator &, Path &, IteratorHelper &)> out_neighbour_filter,
    std::function<bool(VertexIterator &, Path &, IteratorHelper &)> in_neighbour_filter) {
    auto task_ctx = GetThreadContext();
    constexpr size_t PARALLEL_THRESHOLD = 256;
    next_frontier_.Clear();
    if ((flags_ & TRAVERSAL_PARALLEL) && txn_.IsReadOnly() &&
        curr_frontier_.Size() > PARALLEL_THRESHOLD) {
        auto worker = Worker::SharedWorker();
        worker->Delegate([&]() {
            int num_threads = 0;
#pragma omp parallel
            {
                if (omp_get_thread_num() == 0) {
                    num_threads = omp_get_num_threads();
                }
            };
            size_t *start = new size_t[num_threads];
            size_t *end = new size_t[num_threads];
            for (int thread_id = 0; thread_id < num_threads; thread_id++) {
                start[thread_id] = curr_frontier_.Size() / num_threads * thread_id;
                end[thread_id] = curr_frontier_.Size() / num_threads * (thread_id + 1);
                if (thread_id == num_threads - 1) end[thread_id] = curr_frontier_.Size();
            }
            constexpr size_t LOCAL_BUFFER_SIZE = 1024;
            constexpr size_t WORK_BATCH = 64;
#pragma omp parallel
            {
                ParallelVector<Path> local_frontier(LOCAL_BUFFER_SIZE);
                auto txn = db_.ForkTxn(txn_);
                IteratorHelper helper(txn);
                int thread_id = omp_get_thread_num();
                auto vit = txn.GetVertexIterator();
                for (int thread_offset = 0; thread_offset < num_threads; thread_offset++) {
                    if (ShouldKillThisTask(task_ctx)) break;
                    while (true) {
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t batch_start = __sync_fetch_and_add(&start[thread_id], WORK_BATCH);
                        if (batch_start >= end[thread_id]) break;
                        size_t batch_end = batch_start + WORK_BATCH;
                        if (batch_end > end[thread_id]) batch_end = end[thread_id];
                        for (size_t i = batch_start; i < batch_end; i += 1) {
                            Path &path = curr_frontier_[i];
                            size_t vid = path.GetEndVertex().GetId();
                            vit.Goto(vid);
                            for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                                if (!out_edge_filter || out_edge_filter(eit, path, helper)) {
                                    size_t nbr = eit.GetDst();
                                    bool ok = out_neighbour_filter == nullptr;
                                    if (!ok) {
                                        auto nbr_vit = txn.GetVertexIterator(nbr);
                                        ok = out_neighbour_filter(nbr_vit, path, helper);
                                    }
                                    if (ok) {
                                        local_frontier.Append(path, false);
                                        local_frontier.Back().Append(Edge(vid, eit.GetLabelId(),
                                                                          eit.GetTemporalId(), nbr,
                                                                          eit.GetEdgeId(), true));
                                        if (local_frontier.Size() == LOCAL_BUFFER_SIZE) {
                                            next_frontier_.Append(local_frontier);
                                            local_frontier.Clear();
                                        }
                                    }
                                }
                            }
                            for (auto eit = vit.GetInEdgeIterator(); eit.IsValid(); eit.Next()) {
                                if (!in_edge_filter || in_edge_filter(eit, path, helper)) {
                                    size_t nbr = eit.GetSrc();
                                    bool ok = in_neighbour_filter == nullptr;
                                    if (!ok) {
                                        auto nbr_vit = txn.GetVertexIterator(nbr);
                                        ok = in_neighbour_filter(nbr_vit, path, helper);
                                    }
                                    if (ok) {
                                        local_frontier.Append(path, false);
                                        local_frontier.Back().Append(Edge(vid, eit.GetLabelId(),
                                                                          eit.GetTemporalId(), nbr,
                                                                          eit.GetEdgeId(), false));
                                        if (local_frontier.Size() == LOCAL_BUFFER_SIZE) {
                                            next_frontier_.Append(local_frontier);
                                            local_frontier.Clear();
                                        }
                                    }
                                }
                            }
                        }
                    }
                    thread_id = (thread_id + 1) % num_threads;
                }
                if (local_frontier.Size() != 0) {
                    next_frontier_.Append(local_frontier);
                    local_frontier.Clear();
                }
            }
            delete[] start;
            delete[] end;
        });
    } else {
        IteratorHelper helper(txn_);
        auto vit = txn_.GetVertexIterator();
        for (auto &path : curr_frontier_) {
            size_t vid = path.GetEndVertex().GetId();
            vit.Goto(vid);
            for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                if (!out_edge_filter || out_edge_filter(eit, path, helper)) {
                    size_t nbr = eit.GetDst();
                    bool ok = out_neighbour_filter == nullptr;
                    if (!ok) {
                        auto nbr_vit = txn_.GetVertexIterator(nbr);
                        ok = out_neighbour_filter(nbr_vit, path, helper);
                    }
                    if (ok) {
                        next_frontier_.Append(path, false);
                        next_frontier_.Back().Append(Edge(vid, eit.GetLabelId(),
                                                          eit.GetTemporalId(),
                                                          nbr, eit.GetEdgeId(), true));
                    }
                }
            }
            for (auto eit = vit.GetInEdgeIterator(); eit.IsValid(); eit.Next()) {
                if (!in_edge_filter || in_edge_filter(eit, path, helper)) {
                    size_t nbr = eit.GetSrc();
                    bool ok = in_neighbour_filter == nullptr;
                    if (!ok) {
                        auto nbr_vit = txn_.GetVertexIterator(nbr);
                        ok = in_neighbour_filter(nbr_vit, path, helper);
                    }
                    if (ok) {
                        next_frontier_.Append(path, false);
                        next_frontier_.Back().Append(Edge(vid, eit.GetLabelId(),
                                                          eit.GetTemporalId(),
                                                          nbr, eit.GetEdgeId(), false));
                    }
                }
            }
        }
    }
    curr_frontier_.Swap(next_frontier_);
    if (ShouldKillThisTask(task_ctx)) throw std::runtime_error("Task killed");
}

}  // namespace traversal

}  // namespace lgraph_api
