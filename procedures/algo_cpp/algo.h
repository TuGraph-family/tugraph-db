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

#include <unordered_set>
#include "lgraph/olap_base.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;

/**
 * @brief    Compute the shortest path length between any two vertices in the graph.
 *
 * @param[in]       graph       The graph to compute on.
 * @param[in,out]   result      The vector to store the shortest path result in the graph.
 * @param[in]       max_tuple   The tuple to store maximum and shortest path information.
 */
void APSPCore(OlapBase<double>& graph, std::vector<std::tuple<size_t, size_t, double>> result,
              std::tuple<size_t, size_t, double>& max_tuple);

/**
 * @brief    Estimate the betweenness centrality of all vertices in the graph.
 *
 * @param[in]   graph    The graph to compute on.
 * @param[in]   samples  The number of samples.
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
 * @brief    Compute the Personalized PageRank algorithm.
 *
 * @param[in]        graph          The graph to compute on.
 * @param[in,out]    curr           The ParallelVector to store ppr value.
 * @param[in]        root           The id of the vertex for which the ppr is to be calculated.
 * @param[in]        iterations     The iterations of Personalized PageRank algorithm.
 *
 */
void PPRCore(OlapBase<Empty>& graph, ParallelVector<double>& curr, size_t root, size_t iterations);

/**
 * \brief   Perform a SSSP from root vertex and return the result.
 *
 * \param               graph     The graph to compute on.
 * \param               root      The root vertex id to start sssp from.
 * \param   [in,out]    distance  The ParallelVector to store distance.
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
 * \brief    Compute the clustering coefficient of all vertices in the graph.
 *
 * \param    graph    The graph to compute on.
 * \param    score    The vector to store clustering coefficient.
 *
 * \return    return the clustering coefficient of graph.
 */
double LCCCore(OlapBase<Empty>& graph, ParallelVector<double>& score);

/**
 * @brief    Compute the leiden algorithm.
 *
 * @param[in]       graph           The graph to compute on.
 * @param[in,out]   label           The community of vertex.
 * @param[in]       random_seed     The number of random seed for leiden algorithm.
 * @param[in]       theta           Determine the degree of randomness in the selection
 *                                  of a community and within a range of roughly [0.0005, 0.1]
 * @param[in]       gamma           The resolution parameter.
 *                                  Higher resolutions lead to more communities
 * @param[in]       threshold       Terminate when active_vertices < num_vertices / threshold
 */
void LeidenCore(OlapBase<double>& graph, ParallelVector<size_t>& label, unsigned random_seed,
                double theta, double gamma, size_t threshold);

/**
 * @brief    Compute the number of rings of length k.
 *
 * @param[in]   graph    The graph to compute on, should be an *directed* graph.
 * @param[in]   k        The length of rings.
 *
 * @return   the number of rings of length k.
 */
size_t LocateCycleCore(OlapBase<Empty>& graph, size_t k);

/**
 * \brief    Compute the label propagation algorithm.
 *
 * \param    graph             The graph to compute on.
 * \param    label             The ParallelVector to store label information.
 * \param    num_iterations    The iterations of label propagation algorithm.
 * \param    sync_flag         Synchronous mode -> 1, Asynchronous mode -> 0.
 *
 * \return    The value of modularity.
 */
double LPACore(OlapBase<Empty>& graph, ParallelVector<size_t>& label,
        int num_iterations, bool sync_flag);

/**
 * @brief    Compute the Maximal Independent Set Algorithm.
 *
 * @param[in]        graph          The graph to compute on.
 * @param[in, out]   mis            The ParallelVector to store MIS value.
 * @param[in, out]   mis_size       The size of MIS.
 *
 */
void MISCore(OlapBase<Empty>& graph, ParallelVector<bool>& mis, size_t& mis_size);

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
 * @brief    Compute the k-cliques algorithm.
 *
 * @param[in]    graph           The graph to compute on.
 * @param[in]    value_k         The size of to cliques.
 * @param[in]    cliques_cnt     The number of to cliques of each vertex.
 *
 * @return    return the number of cliques.
 */
size_t KCliquesCore(OlapBase<Empty>& graph, int value_k, ParallelVector<size_t>& cliques_cnt);

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
 * @brief    Compute the k-truss algorithm.
 *
 * @param[in]    graph           The graph to compute on.
 * @param[in]    value_k         The size of truss.
 * @param[in]    sub_neighbours  The neighbours in k-truss subgraph.
 *
 * @return    return number of edges in k-truss subgraph.
 */
size_t KTrussCore(OlapBase<Empty>& graph, size_t value_k,
                  std::vector<std::vector<size_t>>& sub_neighbours);

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
 * @param[in]       graph          The graph to compute on.
 * @param[in]       root           The set of root vertex.
 * @param[in,out]   distance       The ParallelVector to store shostest distance.
 */
void MSSPCore(OlapBase<double>& graph, std::vector<size_t> roots, ParallelVector<double>& distance);

/**
 * @brief    Compute the Subgraph Isomorphism Algorithm.
 *
 * @param[in]        graph          The graph to compute on.
 * @param[in]        query          The outgoing adjacency list of subgraph.
 * @param[in, out]   counts         The ParallelVector to store number of subgraph.
 *
 * @return   return total number of subgraph.
 */
size_t SubgraphIsomorphismCore(OlapBase<Empty>& graph,
                               std::vector<std::unordered_set<size_t>>& query,
                               ParallelVector<size_t>& counts);

/**
 * @brief    Compute the Sybil Rank Algorithm.
 *
 * @param[in]        graph          The graph to compute on.
 * @param[in]        trust_seeds    The ParallelVector for trusted nodes.
 * @param[in, out]   curr           The ParallelVector to store sybil rank value.
 *
 */
void SybilRankCore(OlapBase<Empty>& graph, ParallelVector<size_t>& trust_seeds,
                   ParallelVector<double>& curr);

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
 * @brief    Compute the Triangle Counting algorithm with Range Algorithm (KDD'21).
 *
 * @param[in]    graph           The graph to compute on.
 * @param[in]    num_triangle    The ParallelVector to store the number of triangles of each vertex.
 *
 * @return   return the number of triangles of the whole graph.
 */
size_t RangeTriangleCore(OlapBase<Empty>& graph, ParallelVector<size_t>& num_triangle);

/**
 * @brief    Compute the strongly connected components algorithm.
 *
 * @param[in]        graph       The graph to compute on.
 * @param[in,out]    label       The community of vertex.
 */
void SCCCore(OlapBase<Empty>& graph, ParallelVector<size_t>& label);

/**
 * @brief    Compute the Speaker-listener Label Propagation Algorithm.
 *
 * @param[in]    graph          The graph to compute on.
 * @param[in]    label_map      The ParallelVector to store label information.
 * @param[in]    num_iterations The iterations of slpa algorithm.
 * @param[in]    threshold      The minimum number of times the node's label that meets the
 *                              requirements appears.
 * @return   return the Modularity of graph.
 */
double SLPACore(OlapBase<Empty>& graph,
                ParallelVector<std::vector<std::pair<size_t, size_t>>>& label_map,
                int num_iterations, size_t threshold);

/**
 * @brief    Compute the Single Pair Shortest Path algorithm.
 *
 * @param[in]    graph           The graph to compute on.
 * @param[in]    search_pair     The vertex pair to compute.
 *
 * @return    return the shortest path distance of search pair.
 */
size_t SPSPCore(OlapBase<Empty>& graph, std::pair<size_t, size_t> search_pair);

/**
 * @brief    Compute the Trustrank algorithm.
 *
 * @param[in]        graph          The graph to compute on.
 * @param[in]        iterations     The iterations of Trustrank algorithm.
 * @param[in,out]    curr           The ParallelVector to store trustrank.
 * @param[in]        trust_list     Trust user input vertices.
 *
 */
void TrustrankCore(OlapBase<Empty>& graph, size_t iterations, ParallelVector<double>& curr,
                   std::vector<size_t> trust_list);

/**
 * @brief    Compute the Weighted Label Propagation Algorithm.
 *
 * @param[in]    graph          The graph to compute on.
 * @param[in]    num_iterations The iterations of wlpa algorithm.
 *
 * @return   return the Modularity of graph.
 */
double WLPACore(OlapBase<double>& graph, int num_iterations);

/**
 * @brief    Compute the Weighted Pagerank Algorithm.
 *
 * @param[in]        graph          The graph to compute on.
 * @param[in]        num_iterations The iterations of Weighted Pagerank Algorithm.
 * @param[in,out]    curr           The ParallelVector to store pagerank value.
 *
 */
void WPageRankCore(OlapBase<double>& graph, int num_iterations, ParallelVector<double>& curr);
