
//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once

#include <string>
#include <vector>

#include "lgraph/olap_on_db.h"

namespace lgraph_api {
using namespace olap;

namespace traversal {

/**
 * @brief   Retrieve all vertices passing the specified filter.  Note that if the transaction is
 *          not read-only, parallel will be ignored (i.e. parallelism will not be available).
 *
 * @param [in,out]  db          The GraphDB instance.
 * @param [in,out]  txn         The transaction.
 * @param [in,out]  filter      The user-defined filter function.
 * @param           parallel    (Optional) Enable parallelism or not.
 *
 * @returns The corresponding list of vertices.
 */
ParallelVector<size_t> FindVertices(GraphDB &db, Transaction &txn,
                                    std::function<bool(VertexIterator &)> filter,
                                    bool parallel = false);

/**
 * @brief   Extract data from specified vertices. Note that if the transaction is not read-only,
 *          parallel will be ignored (i.e. parallelism will not be available).
 *
 * @exception   std::runtime_error  Raised when a runtime error condition occurs.
 *
 * @tparam  VertexData  Type of the vertex data.
 * @param [in,out]  db          The database.
 * @param [in,out]  txn         The transaction.
 * @param [in,out]  frontier    The vertices to extract data from.
 * @param [in,out]  extract     The user-defined extract function.
 * @param           parallel    (Optional) Enable parallelism or not.
 *
 * @returns The corresponding extracted data.
 */
template <typename VertexData>
static ParallelVector<VertexData> ExtractVertexData(
    GraphDB &db, Transaction &txn, ParallelVector<size_t> &frontier,
    std::function<void(VertexIterator &, VertexData &)> extract, bool parallel = false) {
    auto task_ctx = GetThreadContext();
    Transaction &txn_ = txn;
    ParallelVector<VertexData> data(frontier.Size(), frontier.Size());
    if (parallel && txn.IsReadOnly()) {
        auto worker = Worker::SharedWorker();
        worker->Delegate([&]() {
#pragma omp parallel
            {
                auto txn = db.ForkTxn(txn_);
                int thread_id = omp_get_thread_num();
                int num_threads = omp_get_num_threads();
                size_t start = frontier.Size() / num_threads * thread_id;
                size_t end = frontier.Size() / num_threads * (thread_id + 1);
                if (thread_id == num_threads - 1) end = frontier.Size();
                auto vit = txn.GetVertexIterator();
                for (size_t i = start; i < end; i++) {
                    if (i % 1024 == 0 && ShouldKillThisTask(task_ctx)) break;
                    size_t vid = frontier[i];
                    vit.Goto(vid);
                    extract(vit, data[i]);
                }
            }
        });
    } else {
        auto vit = txn.GetVertexIterator();
        for (size_t i = 0; i < frontier.Size(); i++) {
            size_t vid = frontier[i];
            vit.Goto(vid);
            extract(vit, data[i]);
        }
    }
    if (ShouldKillThisTask(task_ctx)) throw std::runtime_error("Task killed");
    return data;
}

/**
 * @brief   The default filter you may use.
 *
 * @tparam  IT  Type of the iterator.
 * @param [in,out]  it  An iterator class.
 *
 * @returns Always true.
 */
template <class IT>
bool DefaultFilter(IT &it) {
    return true;
}

/**
 * @brief   Function closure filtering by label.
 *
 * @tparam  IT  Type of the iterator.
 * @param   label   An iterator class.
 *
 * @returns The filter function.
 */
template <class IT>
std::function<bool(IT &)> LabelEquals(const std::string &label) {
    return [&](IT &it) { return it.GetLabel() == label; };
}

/**
 * @brief   Function closure filtering by label id.
 *
 * @tparam  IT  Type of the iterator.
 * @param   label_id    An iterator class.
 *
 * @returns The filter function.
 */
template <class IT>
std::function<bool(IT &)> LabelEquals(size_t label_id) {
    return [&](IT &it) { return it.GetLabelId() == label_id; };
}

// The allowed maximum size of a result set.
static constexpr size_t MAX_RESULT_SIZE = 1ul << 36;

// The available flags for traversal descriptions.
static constexpr size_t TRAVERSAL_PARALLEL = 1ul << 0;
// Enforce parallelism when traversing
static constexpr size_t TRAVERSAL_ALLOW_REVISITS = 1ul << 1;
// Whether each vertex can be revisited or not

/**
 * @brief   FrontierTraversal provides the most common way to traverse graphs. You can start from
 *          a single vertex or a set of vertices (known as the initial frontier), and expand them
 *          frontier by frontier, each time visiting neighboring vertices of the current frontier
 *          and make those matching specified conditions the new frontier. One powerful feature
 *          of FrontierTraversal is that the traversal can be performed in parallel, accelerating
 *          those *deep* queries significantly.
 */
class FrontierTraversal {
    GraphDB &db_;
    Transaction &txn_;
    size_t flags_;
    size_t num_vertices_;
    ParallelVector<size_t> curr_frontier_;
    ParallelVector<size_t> next_frontier_;
    ParallelBitset visited_;

 public:
    /**
     * @brief   Construct the FrontierTraversal object. Note that the transaction must be read-
     *          only if you want to perform the traversals in parallel (i.e. TRAVERSAL_PARALLEL
     *          is specified in flags). Be careful when TRAVERSAL_ALLOW_REVISITS is used, as each
     *          vertex may be visited more than once, making the result set huge.
     *
     * @param [in,out]  db      The GraphDB instance.
     * @param [in,out]  txn     The transaction.
     * @param           flags   (Optional) The options used during traversals.
     */
    FrontierTraversal(GraphDB &db, Transaction &txn, size_t flags = 0,
        size_t capacity = MAX_RESULT_SIZE);

    /**
     * @brief   Retrieve the current (i.e. latest) frontier.
     *
     * @returns The frontier.
     */
    ParallelVector<size_t> &GetFrontier();

    /**
     * @brief   Set the (initial) frontier to contain a single vertex.
     *
     * @param   root_vid    The identifer for the starting vertex.
     */
    void SetFrontier(size_t root_vid);

    /**
     * @brief   Set the (initial) frontier to contain a set of vertices.
     *
     * @param [in,out]  root_vids   The starting vertex identifiers.
     */
    void SetFrontier(ParallelVector<size_t> &root_vids);

    /**
     * @brief   Set the (initial) frontier by using a filter function. Each vertex will be
     *          checked against the specified filter.
     *
     * @param [in,out]  root_vertex_filter  The filter function.
     */
    void SetFrontier(std::function<bool(VertexIterator &)> root_vertex_filter);

    /**
     * @brief   Expand the current frontier through outgoing edges using filters. Note that the
     *          default value for the two filters (nullptr) means all the expansions of this
     *          level should succeed and  enables some optimizations.
     *
     * @param [in,out]  out_edge_filter         (Optional) The filter for an outgoing edge.
     * @param [in,out]  out_neighbour_filter    (Optional) The filter for an destination vertex.
     */
    void ExpandOutEdges(std::function<bool(OutEdgeIterator &)> out_edge_filter = nullptr,
                        std::function<bool(VertexIterator &)> out_neighbour_filter = nullptr);

    /**
     * @brief   Expand the current frontier through incoming edges using filters. Note that the
     *          default value for the two filters (nullptr) means all the expansions of this
     *          level should succeed and enables some optimizations.
     *
     * @param [in,out]  in_edge_filter      (Optional) The filter for an incoming edge.
     * @param [in,out]  in_neighbour_filter (Optional) The filter for an source vertex.
     */
    void ExpandInEdges(std::function<bool(InEdgeIterator &)> in_edge_filter = nullptr,
                       std::function<bool(VertexIterator &)> in_neighbour_filter = nullptr);

    /**
     * @brief   Expand the current frontier through both directions using filters. Note that the
     *          default value for the two filters (nullptr) means all the expansions of this
     *          level should succeed and enables some optimizations.
     *
     * @param [in,out]  out_edge_filter         (Optional) The filter for an outgoing edge.
     * @param [in,out]  in_edge_filter          (Optional) The filter for an incoming edge.
     * @param [in,out]  out_neighbour_filter    (Optional) The filter for an destination vertex.
     * @param [in,out]  in_neighbour_filter     (Optional) The filter for an source vertex.
     */
    void ExpandEdges(std::function<bool(OutEdgeIterator &)> out_edge_filter = nullptr,
                     std::function<bool(InEdgeIterator &)> in_edge_filter = nullptr,
                     std::function<bool(VertexIterator &)> out_neighbour_filter = nullptr,
                     std::function<bool(VertexIterator &)> in_neighbour_filter = nullptr);

    /** @brief   Reset the traversal. */
    void Reset();

    /** @brief   Reset only the visited flags. */
    void ResetVisited();
};

/** @brief   Represent a vertex. */
class Vertex {
    friend class Path;
    friend class IteratorHelper;
    friend class PathTraversal;

    size_t vid_;

 public:
    /**
     * @brief   Constructor
     *
     * @param   vid The vid.
     */
    explicit Vertex(size_t vid);

    /**
     * @brief   Copy constructor
     *
     * @param   rhs The right hand side.
     */
    Vertex(const Vertex &rhs) = default;

    /**
     * @brief   Get the Id of this vertex.
     *
     * @returns The Id.
     */
    size_t GetId() const;
};

/** @brief   Represent an edge. */
class Edge {
    friend class Path;
    friend class IteratorHelper;
    friend class PathTraversal;

    size_t start_;
    size_t end_;
    size_t eid_;
    uint16_t lid_;
    int64_t tid_;
    bool forward_;

 public:
    /**
     * @brief   Constructor
     *
     * @param   start   The start.
     * @param   lid     The lid.
     * @param   tid     The tid.
     * @param   end     The end.
     * @param   eid     The eid.
     * @param   forward True to forward.
     */
    Edge(size_t start, uint16_t lid, uint64_t tid, size_t end, size_t eid, bool forward);

    /**
     * @brief   Copy constructor
     *
     * @param   rhs The right hand side.
     */
    Edge(const Edge &rhs) = default;

    /**
     * @brief   Get the start vertex of this edge.
     *
     * @returns The start vertex.
     */
    Vertex GetStartVertex() const;

    /**
     * @brief   Get the end vertex of this edge.
     *
     * @returns The end vertex.
     */
    Vertex GetEndVertex() const;

    /**
     * @brief   Get the label ID.
     *
     * @returns The label ID.
     */
    uint16_t GetLabelId() const;

    /**
     * @brief   Get the Temporal ID.
     *
     * @returns The temporal ID.
     */
    uint64_t GetTemporalId() const;

    /**
     * @brief   Get the edge ID.
     *
     * @returns The edge ID.
     */
    size_t GetEdgeId() const;

    /**
     * @brief   Get the direction of this edge.
     *
     * @returns true : forward; false: backward.
     */
    bool IsForward() const;

    /**
     * @brief   Get the source vertex of this edge.
     *
     * @returns The source vertex.
     */
    Vertex GetSrcVertex() const;

    /**
     * @brief   Get the destination vertex of this edge.
     *
     * @returns The destination vertex.
     */
    Vertex GetDstVertex() const;
};

bool operator==(const Vertex &lhs, const Vertex &rhs);
bool operator!=(const Vertex &lhs, const Vertex &rhs);
bool operator==(const Edge &lhs, const Edge &rhs);
bool operator!=(const Edge &lhs, const Edge &rhs);

/** @brief   Represent a path. */
class Path {
    std::vector<bool> dirs_;
    std::vector<uint16_t> lids_;
    std::vector<size_t> ids_;

 public:
    /**
     * @brief   Constructor
     *
     * @param   start   The start.
     */
    explicit Path(const Vertex &start);

    /**
     * @brief   Copy constructor
     *
     * @param   rhs The right hand side.
     */
    Path(const Path &rhs);

    /**
     * @brief   Move constructor
     *
     * @param [in,out]  rhs The right hand side.
     */
    Path(Path &&rhs) = delete;

    /**
     * @brief   Assignment operator
     *
     * @param   rhs The right hand side.
     *
     * @returns A shallow copy of this object.
     */
    Path &operator=(const Path &rhs);

    /**
     * @brief   Move assignment operator
     *
     * @param [in,out]  rhs The right hand side.
     *
     * @returns A shallow copy of this object.
     */
    Path &operator=(Path &&rhs) = delete;

    /**
     * @brief   Get the length of this path.
     *
     * @returns The path length.
     */
    size_t Length() const;

    /**
     * @brief   Append an edge to the path. Note that the edge's start vertex should match the
     *          current path's end vertex.
     *
     * @param   edge    edge to append.
     */
    void Append(const Edge &edge);

    /** @brief   Get the start vertex of this path. */
    Vertex GetStartVertex() const;

    /** @brief   Get the end vertex of this path. */
    Vertex GetEndVertex() const;

    /** @brief   Get the last edge of this path. */
    Edge GetLastEdge() const;

    /** @brief   Get the Nth edge of this path. The available range of N is [0, Length). */
    Edge GetNthEdge(size_t n) const;

    /** @brief   Get the Nth vertex of this path. The available range of N is [0, Length]. */
    Vertex GetNthVertex(size_t n) const;
};

/**
 * @brief   IteratorHelper provides some useful methods, such as casting Vertex/Edge objects to
 *          their iterator forms.
 */
class IteratorHelper {
    Transaction &txn_;

 public:
    /**
     * @brief   Constructor
     *
     * @param [in,out]  txn The transaction.
     */
    explicit IteratorHelper(Transaction &txn);

    /**
     * @brief   Casting a Vertex to a VertexIterator.
     *
     * @param   vertex  A Vertex object.
     *
     * @return  A VertexIterator corresponding to vertex.
     */
    VertexIterator Cast(const Vertex &vertex);

    /**
     * @brief   Casting an Edge to an OutEdgeIterator.
     *
     * @param   edge    An edge object.
     *
     * @return  An OutEdgeIterator corresponding to edge.
     */
    OutEdgeIterator Cast(const Edge &edge);
};

/**
 * @brief   PathTraversal behaves similar to FrontierTraversal, except that
 *          1) Each vertex could be revisited multiple times.
 *          2) Each traversed path would be stored, and the filter function has access to the
 *          path.
 */
class PathTraversal {
    GraphDB &db_;
    Transaction &txn_;
    size_t flags_;
    size_t num_vertices_;
    ParallelVector<Path> curr_frontier_;
    ParallelVector<Path> next_frontier_;

 public:
    /**
     * @brief   Construct the PathTraversal object. Note that the transaction must be read-only
     *          if you want to perform the traversals in parallel (i.e. TRAVERSAL_PARALLEL is
     *          specified in flags). Since TRAVERSAL_ALLOW_REVISITS is implied in PathTraversal
     *          and storing the Paths also takes a lot of space, the memory consumption could be
     *          very large if the filters are not very selective.
     *
     * @param [in,out]  db      The GraphDB instance.
     * @param [in,out]  txn     The transaction.
     * @param           flags   The options used during traversals.
     */
    PathTraversal(GraphDB &db, Transaction &txn, size_t flags, size_t capacity = MAX_RESULT_SIZE);

    /**
     * @brief   Retrieve the current (i.e. latest) frontier.
     *
     * @returns The frontier.
     */
    ParallelVector<Path> &GetFrontier();

    /**
     * @brief   Set the (initial) frontier to contain a single vertex.
     *
     * @param   root_vid    The identifer for the starting vertex.
     */
    void SetFrontier(size_t root_vid);

    /**
     * @brief   Set the (initial) frontier to contain a set of vertices.
     *
     * @param [in,out]  root_vids   The starting vertex identifiers.
     */
    void SetFrontier(ParallelVector<size_t> &root_vids);

    /**
     * @brief   Set the (initial) frontier by using a filter function. Each vertex will be
     *          checked against the specified filter.
     *
     * @param [in,out]  root_vertex_filter  The filter function.
     */
    void SetFrontier(std::function<bool(VertexIterator &)> root_vertex_filter);

    /**
     * @brief   Expand the current frontier through outgoing edges using filters.
     *
     * @param [in,out]  out_edge_filter         (Optional) The filter for an outgoing edge.
     * @param [in,out]  out_neighbour_filter    (Optional) The filter for an destination vertex.
     */
    void ExpandOutEdges(
        std::function<bool(OutEdgeIterator &, Path &, IteratorHelper &)> out_edge_filter = nullptr,
        std::function<bool(VertexIterator &, Path &, IteratorHelper &)> out_neighbour_filter =
            nullptr);

    /**
     * @brief   Expand the current frontier through incoming edges using filters.
     *
     * @param [in,out]  in_edge_filter      (Optional) The filter for an incoming edge.
     * @param [in,out]  in_neighbour_filter (Optional) The filter for an source vertex.
     */
    void ExpandInEdges(
        std::function<bool(InEdgeIterator &, Path &, IteratorHelper &)> in_edge_filter = nullptr,
        std::function<bool(VertexIterator &, Path &, IteratorHelper &)> in_neighbour_filter =
            nullptr);

    /**
     * @brief   Expand the current frontier through both directions using filters.
     *
     * @param [in,out]  out_edge_filter         (Optional) The filter for an outgoing edge.
     * @param [in,out]  in_edge_filter          (Optional) The filter for an incoming edge.
     * @param [in,out]  out_neighbour_filter    (Optional) The filter for an destination vertex.
     * @param [in,out]  in_neighbour_filter     (Optional) The filter for an source vertex.
     */
    void ExpandEdges(
        std::function<bool(OutEdgeIterator &, Path &, IteratorHelper &)> out_edge_filter = nullptr,
        std::function<bool(InEdgeIterator &, Path &, IteratorHelper &)> in_edge_filter = nullptr,
        std::function<bool(VertexIterator &, Path &, IteratorHelper &)> out_neighbour_filter =
            nullptr,
        std::function<bool(VertexIterator &, Path &, IteratorHelper &)> in_neighbour_filter =
            nullptr);

    /** @brief   Reset the traversal. */
    void Reset();
};

}  // namespace traversal
}  // namespace lgraph_api
