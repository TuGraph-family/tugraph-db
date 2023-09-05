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

#include <unordered_map>
#include "lgraph/olap_base.h"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;

const int max_hold_iterations = 5;
const int max_springback_iterations = 20;

class LouvainGraph {
    OlapBase<double> *graph;
    ParallelVector<size_t> label;
    ParallelVector<double> k;
    ParallelVector<double> e_tot;
    double m;
    size_t real_nodes;
    size_t active_threshold;
    size_t num_community;
    double Q;
    int is_sync;

    OlapBase<double> *sub_graph = nullptr;
    LouvainGraph *sub_louvain_graph = nullptr;

 public:
    LouvainGraph(OlapBase<double> *myGraph, ParallelVector<size_t> &_label, int _is_sync = 0)
        : k(myGraph->AllocVertexArray<double>()),
          e_tot(myGraph->AllocVertexArray<double>()) {
            graph = myGraph;
            is_sync = _is_sync;
            label.Swap(_label);
        }

    void init(size_t threshold) {
        if (threshold <= 0) {
            active_threshold = graph->NumVertices() / 1000;
        } else {
            active_threshold = threshold;
        }
        active_threshold = (active_threshold > 0) ? active_threshold : 1;
        k.Fill(0.0);
        e_tot.Fill(0.0);

        real_nodes = graph->ProcessVertexInRange<size_t>(
            [&](size_t vi) {
                label[vi] = vi;
                if (graph->OutDegree(vi) > 0) {
                    return 1;
                }
                return 0;
            },
            0, graph->NumVertices());

        m = graph->ProcessVertexInRange<double>(
                [&](size_t vi) {
                    for (auto e : graph->OutEdges(vi)) {
                        k[vi] += e.edge_data;
                    }
                    e_tot[vi] = k[vi];
                    return k[vi];
                },
                0, graph->NumVertices()) / 2;
        std::cout << "m = " << m << std::endl;
    }

    size_t update_num_community() {
        num_community = graph->ProcessVertexInRange<size_t>(
            [&](size_t vi) {
                if (e_tot[vi] == 0.0) {
                    return 0;
                } else {
                    return 1;
                }
            },
            0, graph->NumVertices());
        std::cout << "number of communities is " << num_community << std::endl;
        return num_community;
    }

    size_t get_num_community() { return num_community; }

    void update_e_tot() {
        e_tot.Fill(0.0);
        graph->ProcessVertexInRange<size_t>(
            [&](size_t v) {
                write_add(&e_tot[label[v]], k[v]);
                return 0;
            },
        0, graph->NumVertices());
    }

    double update_Q() {
        Q = graph->ProcessVertexInRange<double>(
                [&](size_t vi) {
                    double q = 0.0;
                    for (auto e : graph->OutEdges(vi)) {
                        size_t nbr = e.neighbour;
                        if (label[vi] == label[nbr]) {
                            q += e.edge_data;
                        }
                    }
                    q -= 1.0 * k[vi] * e_tot[label[vi]] / (2.0 * m);
                    return q;
                },
                0, graph->NumVertices()) / (2.0 * m);
        std::cout << "Q = " << Q << std::endl;
        return Q;
    }

    double get_Q() {return Q;}

    void update_all() {
        update_e_tot();
        update_num_community();
        update_Q();
    }

    size_t async_louvain() {
        size_t active_vertices = graph->ProcessVertexInRange<size_t>(
            [&](size_t vi) {
                std::unordered_map<size_t, double> count;
                for (auto e : graph->OutEdges(vi)) {
                    if (vi == e.neighbour) {
                        continue;
                    }
                    size_t nbr_label = label[e.neighbour];
                    auto it = count.find(nbr_label);
                    if (it == count.end()) {
                        count[nbr_label] = e.edge_data;
                    } else {
                        it->second += e.edge_data;
                    }
                }
                size_t old_label = label[vi];
                double k_in_out = 0.0;
                if (count.find(old_label) != count.end()) {
                    k_in_out = count[old_label];
                }
                double delta_in = 1.0 * k[vi] * (e_tot[old_label] - k[vi]) - 2.0 * k_in_out * m;

                double delta_in_max = -delta_in;
                size_t label_max = old_label;
                for (auto ele : count) {
                    size_t new_label = ele.first;
                    if (old_label == new_label) continue;
                    double k_in_in = ele.second;
                    double delta_in = 2.0 * k_in_in * m - 1.0 * k[vi] * (e_tot[new_label]);
                    if (delta_in > delta_in_max) {
                        delta_in_max = delta_in;
                        label_max = new_label;
                    } else if (delta_in == delta_in_max && new_label < label_max) {
                        delta_in_max = delta_in;
                        label_max = new_label;
                    }
                }

                if (label_max != old_label) {
                    double v_k = k[vi];
                    graph->AcquireVertexLock(old_label);
                    e_tot[old_label] -= v_k;
                    graph->ReleaseVertexLock(old_label);
                    label[vi] = label_max;
                    graph->AcquireVertexLock(label_max);
                    e_tot[label_max] += v_k;
                    graph->ReleaseVertexLock(label_max);
                    return 1;
                }
                return 0;
            },
            0, graph->NumVertices());
        return active_vertices;
    }

    size_t series_louvain() {
        size_t active_vertices = 0;
        for (size_t v = 0; v < graph->NumVertices(); v++) {
            if (graph->OutDegree(v) == 0) {
                continue;
            }
            std::unordered_map<size_t, double> count;
            for (auto e : graph->OutEdges(v)) {
                if (v == e.neighbour) continue;
                size_t nbr_label = label[e.neighbour];
                auto it = count.find(nbr_label);
                if (it == count.end()) {
                    count[nbr_label] = e.edge_data;
                } else {
                    it->second += e.edge_data;
                }
            }
            size_t old_label = label[v];
            double k_in_out = 0.0;
            if (count.find(old_label) != count.end()) {
                k_in_out = count[old_label];
            }
            double delta_out = k[v] * (e_tot[old_label] - k[v]) - 2.0 * k_in_out * m;
            double delta_in_max = -delta_out;
            size_t label_max = old_label;
            for (auto & ele : count) {
                size_t new_label = ele.first;
                if (old_label == new_label) continue;
                double k_in_in = ele.second;
                double delta_in = 2.0 * k_in_in * m - k[v] * (e_tot[new_label]);
                if (delta_in > delta_in_max) {
                    delta_in_max = delta_in;
                    label_max = new_label;
                } else if (delta_in == delta_in_max && new_label < label_max) {
                    delta_in_max = delta_in;
                    label_max = new_label;
                }
            }
            if (label_max != old_label) {
                e_tot[old_label] -= k[v];
                label[v] = label_max;
                e_tot[label_max] += k[v];
                active_vertices += 1;
            }
        }
        return active_vertices;
    }

    bool louvain_propagate() {
        int iters = 0;
        int recur_iters = 0;
        size_t active_vertices = graph->NumVertices();
        size_t old_active_vertices = active_vertices;
        size_t min_active_vertices = graph->NumVertices();
        int springback_iterations = 0;

        if (is_sync == 0) {
            while (active_vertices >= active_threshold) {
                old_active_vertices = active_vertices;
                active_vertices = async_louvain();
                std::cout << "active_vettices(" << iters << ") = " << active_vertices << std::endl;
                iters++;
                if (old_active_vertices == active_vertices) {
                    recur_iters++;
                } else {
                    recur_iters = 0;
                }
                if (recur_iters >= max_hold_iterations) {
                    break;
                }
                if (active_vertices < min_active_vertices) {
                    min_active_vertices = active_vertices;
                    springback_iterations = 0;
                } else {
                    springback_iterations++;
                }
                if (springback_iterations >= max_springback_iterations) {
                    break;
                }
            }
        }
        if (active_vertices >= active_threshold) {
            std::cout << "into series louvain process" << std::endl;
            while (active_vertices >= active_threshold) {
                active_vertices = series_louvain();
                std::cout << "active_vertices(" << iters << ") = " << active_vertices << std::endl;
                iters++;
            }
        }
        update_num_community();
        update_Q();
        return true;
    }

    void min_label() {
        auto min_label = graph->AllocVertexArray<size_t>();
        min_label.Fill(graph->NumVertices());
        graph->ProcessVertexInRange<size_t>(
            [&](size_t vi) {
                size_t v_label = label[vi];
                auto lock = graph->GuardVertexLock(v_label);
                if (min_label[v_label] > vi) {
                    min_label[v_label] = vi;
                }
                return 0;
            },
            0, graph->NumVertices());
        graph->ProcessVertexInRange<size_t>(
            [&](size_t vi) {
                label[vi] = min_label[label[vi]];
                return 0;
            },
        0, graph->NumVertices());
    }

    void update_label(ParallelVector<size_t> &_label) {
        _label.Swap(label);
    }

    void update_by_subgraph() {
        auto sub_index = graph->AllocVertexArray<size_t>();
        size_t num_sub_vertices = 0;
        for (size_t v_i = 0; v_i < graph->NumVertices(); v_i++) {
            if (e_tot[v_i] == 0) {
                sub_index[v_i] = -1;
            } else {
                sub_index[v_i] = num_sub_vertices;
                num_sub_vertices++;
            }
        }
        std::vector<std::unordered_map<size_t, double> > sub_edges(num_sub_vertices);

        for (size_t v = 0; v < graph->NumVertices(); v++) {
            if (sub_index[label[v]] == (size_t)-1) {
                continue;
            }
            for (auto e : graph->OutEdges(v)) {
                if (v <= e.neighbour) {
                    double edouble = e.edge_data;
                    if (v == e.neighbour) {
                        edouble /= 2;
                    }
                    auto pair_it = sub_edges[
                            sub_index[label[v]]].find(sub_index[label[e.neighbour]]);
                    if (pair_it == sub_edges[sub_index[label[v]]].end()) {
                        sub_edges[sub_index[label[v]]][
                                sub_index[label[e.neighbour]]] = edouble;
                    } else {
                        pair_it->second += edouble;
                    }
                }
            }
        }

        size_t sub_edges_num = 0;
        for (size_t i = 0; i < sub_edges.size(); i++) {
            sub_edges_num += sub_edges[i].size();
        }
        EdgeUnit<double> * sub_edge_array = new EdgeUnit<double>[sub_edges_num];
        size_t sub_edge_index = 0;
        for (size_t v_i = 0; v_i < num_sub_vertices; v_i++) {
            for (auto ele : sub_edges[v_i]) {
                assert(sub_edge_index < sub_edges_num);
                sub_edge_array[sub_edge_index].src = v_i;
                sub_edge_array[sub_edge_index].dst = ele.first;
                sub_edge_array[sub_edge_index].edge_data = ele.second;
                sub_edge_index++;
            }
        }
        sub_edges.clear();

        sub_graph = new OlapBase<double>;
        sub_graph->LoadFromArray((char*)sub_edge_array,
                num_sub_vertices, sub_edges_num, MAKE_SYMMETRIC);
        delete [] sub_edge_array;

        auto sub_to_parent_label = sub_graph->AllocVertexArray<size_t>();
        sub_to_parent_label.Fill(graph->NumVertices());
        graph->ProcessVertexInRange<size_t> (
                [&] (size_t v) {
                    if (sub_index[v] == (size_t)-1) {
                        return 0;
                    }
                    sub_to_parent_label[sub_index[v]] = v;
                    return 1;
                },
                0, graph->NumVertices());

        auto sub_label = sub_graph->AllocVertexArray<size_t>();
        sub_louvain_graph = new LouvainGraph(sub_graph, sub_label, is_sync);
        sub_louvain_graph->init(1);
        sub_louvain_graph->louvain_propagate();

        if (sub_louvain_graph->get_num_community() == sub_graph->NumVertices()) {
            return;
        } else {
            sub_louvain_graph->update_by_subgraph();
        }

        graph->ProcessVertexInRange<size_t> (
                [&] (size_t v) {
                    if (sub_index[label[v]] == (size_t)-1) {
                        return 0;
                    }
                    size_t sub_index_v_comm = sub_louvain_graph->label[sub_index[label[v]]];
                    label[v] = sub_to_parent_label[sub_index_v_comm];
                    return 0;
                },
                0, graph->NumVertices());

        delete sub_louvain_graph;
    }
};

double LouvainCore(OlapBase<double>& graph, ParallelVector<size_t> &label,
                   size_t active_threshold, int is_sync) {
    LouvainGraph louvain_graph(&graph, label, is_sync);
    louvain_graph.init(active_threshold);
    louvain_graph.louvain_propagate();
    louvain_graph.update_by_subgraph();

    louvain_graph.update_all();
    louvain_graph.update_label(label);
    return louvain_graph.get_Q();
}
