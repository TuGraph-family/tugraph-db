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
 * \brief   Perform a BFS from root vertex.
 *
 * \param               graph    The graph to compute on.
 * \param               root_vid    The root vertex id to start bfs from.
 * \param   [in,out]    parent    The ParallelVector to judge whether have visited.
 *
 * \return  The number of visited vertices of graph.
 */
size_t BFSCore(OlapBase<Empty>& graph, size_t root_vid, ParallelVector<size_t>& parent);

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

