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

#include "lgraph/olap_base.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;

/**
 * @brief    Estimate the betweenness centrality of all vertices in the graph.
 *
 * @param[in]   graph    The graph to compute on.
 * @param[in]   samples    The number of samples.
 * @param[in]   score    The vector to store betweenness centrality.
 *
 * @return    return the graph node of max betweenness centrality.
 */
size_t BCCore(OlapBase<Empty>& graph, size_t samples, ParallelVector<double>& score);

/**
 * \brief   Perform a BFS from root vertex.
 *
 * \param               graph     The graph to compute on.
 * \param               root_vid  The root vertex id to start bfs from.
 * \param   [in,out]    parent    The ParallelVector to judge whether have visited.
 *
 * \return  The number of visited vertices of graph.
 */
size_t BFSCore(OlapBase<Empty>& graph, size_t root_vid, ParallelVector<size_t>& parent);

/**
 * @brief    Compute the closeness centrality of all vertices in the graph.
 *
 * @param[in]       graph      The graph to compute on.
 * @param[in]       samples    The number of samples.
 * @param[in,out]   score      The vector to store closeness centrality.
 * @param[in,out]   path_num   The vector to store the num of path.
 */
void CLCECore(OlapBase<double>& graph, size_t samples, ParallelVector<double>& score,
              ParallelVector<size_t>& path_num);

/**
 * @brief    Compute the common neighbours of all input pairs.
 *
 * @param[in]    graph          The graph to compute on.
 * @param[in]    search_pair    The node pair to be calculated.
 *
 * @result   The number of common neighbours.
 */
size_t CNCore(OlapBase<Empty>& graph, std::pair<size_t, size_t> search_pair);

/**
 * \brief   Compute the PageRank value.
 *
 * \param               graph    The graph to compute on.
 * \param               num_iterations  The number of iterations.
 * \param   [in,out]    curr     The ParallelVector to store pr value.
 *
 */
void PageRankCore(OlapBase<Empty>& graph, int num_iterations, ParallelVector<double>& curr);

/**
 * \brief   Perform a SSSP from root vertex and return the result.
 *
 * \param               graph    The graph to compute on.
 * \param               root    The root vertex id to start sssp from.
 * \param   [in,out]    distance   The ParallelVector to store distance.
 */
void SSSPCore(OlapBase<double>& graph, size_t root, ParallelVector<double>& distance);

/**
 * \brief   Compute the weakly connected components.
 *
 * \param               graph   The graph to compute on, should be an *undirected* graph.
 * \param   [in,out]    label   the ParallelVector to store wcc_label.
 */
void WCCCore(OlapBase<Empty>& graph, ParallelVector<size_t>& label);

/**
 * \brief    Comoute the clustering coefficient of all vertices in the graph.
 *
 * \param    graph    The graph to compute on.
 * \param    score    The vector to store clustering coefficient.
 *
 * \return    return the clustering coefficient of graph.
 */
double LCCCore(OlapBase<Empty>& graph, ParallelVector<double>& score);

/**
 * \brief    Comoute the label propagation algorithm.
 *
 * \param    graph    The graph to compute on.
 * \param    num_iterations    The iterations of label propagation algorithm.
 * \param    sync_flag    synchronous mode -> 1, Asynchronous mode -> 0.
 *
 * \return    The value of modularity.
 */
double LPACore(OlapBase<Empty>& graph, int num_iterations, bool sync_flag);

/**
 * @brief    Compute the degree correlation.
 *
 * @param[in]    graph    The graph to compute on.
 *
 * @return    return degree correlation of graph.
 */
double DCCore(OlapBase<Empty>& graph);

/**
 * @brief    Compute the Dimension Estimation algorithm.
 *
 * @param[in]    graph    The graph to compute on.
 * @param[in]    roots    The root vertex id to start de from.
 *
 * @return    return dimension of graph.
 */
size_t DECore(OlapBase<Empty>& graph, std::set<size_t>& roots);

/**
 * @brief    Compute the EgoNet algorithm.
 *
 * @param[in]    graph          The graph to compute on.
 * @param[in]    root           The root vertex id to start.
 * @param[in]    hop_value      The hop of EgoNet.
 *
 * @return    return the number of nodes in EgoNet.
 */
size_t ENCore(OlapBase<Empty>& graph, size_t root, int hop_value);

/**
 * @brief    Compute the Fast Triangle Algorithm.
 *
 * @param[in]        graph          The graph to compute on.
 * @param[in]        num_triangle   The ParallelVector store the number of triangles of each vertex.
 *
 * @return   return  the number of triangles of the whole graph.
 */
size_t FastTriangleCore(OlapBase<Empty>& graph, ParallelVector<size_t>& num_triangle);

/**
 * @brief    Compute the Hyperlink-Induced Topic Search algorithm.
 *
 * @param[in]        graph         The graph to compute on.
 * @param[in,out]    authority     The authority of vertex.
 * @param[in,out]    hub           The hub of vertex.
 * @param[in]        iterations    The iterations of Hyperlink-Induced Topic Search algorithm.
 * @param[in]        delta         Max delta value to stop.
 */
void HITSCore(OlapBase<Empty>& graph, ParallelVector<double>& authority,
              ParallelVector<double>& hub, const int iterations, const double delta);

/**
 * @brief    Compute the Jaccard Index algorithm.
 *
 * @param[in]    graph           The graph to compute on.
 * @param[in]    search_pair     The vertex pair to compute.
 *
 * @return    return the Jaccard Index of search pair.
 */
double JiCore(OlapBase<Empty>& graph, std::pair<size_t, size_t> search_pair);

/**
 * @brief    Compute the k-core algorithm.
 *
 * @param[in]    graph      The graph to compute on.
 * @param[in]    result     Calculated vertex set.
 * @param[in]    value_k    The points in the graph need to always
 *                          satisfy the degree of value_k in the kcore operation.
 *
 * @return    return the number of result.
 */
size_t KCoreCore(OlapBase<Empty>& graph, ParallelVector<bool>& result, size_t value_k);

/**
 * @brief    Compute the Louvain Algorithm.
 *
 * @param[in]        graph              The graph to compute on.
 * @param[in,out]    label              The ParallelVector to store label information.
 * @param[in]        active_threshold   Threshold of active_vertices in original graph.
 * @param[in]        is_sync            if to run louvain in sync mode. 0 means async mode, non-zero value means sync mode.
 *
 * @return   return the modularity of graph.
 */

double LouvainCore(OlapBase<double>& graph, ParallelVector<size_t>& label,
                   size_t active_threshold = 0, int is_sync = 0);

/**
 * @brief    Compute the Motif Algorithm.
 *
 * @param[in]        graph          The graph to compute on.
 * @param[in]        motif_vertices The vertices of computation.
 * @param[in]        k              The size of subgraph.
 * @param[in,out]    motif_count    The array to store motif value.
 *
 */
void MotifCore(OlapBase<Empty>& graph, ParallelVector<size_t>& motif_vertices, int k,
               ParallelVector<std::map<uint64_t, size_t>>& motif_count);

/**
 * @brief    Compute Multiple-source Shortest Paths algorithm.
 *
 * @param[in]    graph          The graph to compute on.
 * @param[in]    root           The set of root vertex.
 * @param[in,out]    distance       The ParallelVector to store shostest distance.
 */
void MSSPCore(OlapBase<double>& graph, std::vector<size_t> roots, ParallelVector<double>& distance);

/**
 * @brief    Compute the Triangle Counting algorithm.
 *
 * @param[in]    graph           The graph to compute on.
 * @param[in]    num_triangle    The ParallelVector to store the number of triangles of each vertex.
 *
 * @return   return the number of triangles of the whole graph.
 */
size_t TriangleCore(OlapBase<Empty>& graph, ParallelVector<size_t>& num_triangle);

/**
 * @brief    Compute the strongly connected components algorithm.
 *
 * @param[in]        graph           The graph to compute on.
 * @param[in,out]    label           The community of vertex.
 */
void SCCCore(OlapBase<Empty>& graph, ParallelVector<size_t>& label);

/**
 * @brief    Compute the Single Pair Shortest Path algorithm.
 *
 * @param[in]    graph           The graph to compute on.
 * @param[in]    search_pair     The vertex pair to compute.
 *
 * @return    return the shortest path distance of search pair.
 */
size_t SPSPCore(OlapBase<Empty>& graph, std::pair<size_t, size_t> search_pair);
