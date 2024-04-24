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

#include <random>
#include <unordered_map>
#include "lgraph/olap_base.h"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;

size_t choice(unsigned seed, std::unordered_map<size_t, double> *probability_distributions) {
    std::vector<double> probability(probability_distributions->size());
    std::vector<size_t> community(probability_distributions->size());
    size_t idx = 0;
    for (auto it : *probability_distributions) {
        community[idx] = it.first;
        probability[idx] = it.second;
        idx++;
    }
    std::default_random_engine generator{seed};
    std::discrete_distribution<size_t> distribution{probability.begin(), probability.end()};
    return community[distribution(generator)];
}

class LeidenGraph {
    OlapBase<double> *graph;
    bool if_base = false;
    ParallelBitset active;
    ParallelBitset well_connected;
    ParallelVector<double> k;
    ParallelVector<double> e_tot;
    ParallelVector<size_t> label_size;
    double m;
    size_t real_nodes;
    size_t num_community;
    size_t num_vertices;
    double Q;
    unsigned random_seed;
    double theta;
    double gamma;
    size_t threshold;

    ParallelVector<size_t> sub_index;
    ParallelVector<size_t> label;
    ParallelVector<size_t> sub_label;
    OlapBase<double> *sub_graph = nullptr;
    LeidenGraph *sub_leiden = nullptr;

 public:
    LeidenGraph(OlapBase<double> *myGraph, ParallelVector<size_t> &_label,
        unsigned _random_seed = 0, double _theta = 0.1, double _gamma = 0.2, size_t _threshold = 0)
        : active(myGraph->AllocVertexSubset()),
          well_connected(myGraph->AllocVertexSubset()),
          k(myGraph->AllocVertexArray<double>()),
          e_tot(myGraph->AllocVertexArray<double>()),
          label_size(myGraph->AllocVertexArray<size_t>()),
          sub_index(myGraph->AllocVertexArray<size_t>()),
          label(1),
          sub_label(1) {
        graph = myGraph;
        theta = _theta;
        gamma = _gamma;
        random_seed = _random_seed;
        if (_threshold <= 0) {
            threshold = graph->NumVertices();
        } else {
            threshold = _threshold;
        }
        label.Swap(_label);
        active.Fill();
        num_vertices = graph->NumVertices();

        real_nodes = graph->ProcessVertexInRange<size_t>(
            [&](size_t v) {
                k[v] = 0;
                label_size[v] = 0;
                if (graph->OutDegree(v) > 0) {
                    return 1;
                }
                return 0;
            },
            0, num_vertices);

        m = graph->ProcessVertexInRange<double>(
            [&](size_t v) {
                for (auto &e : graph->OutEdges(v)) {
                    k[v] += e.edge_data;
                }
                return k[v];
            },
            0, num_vertices);

        double normalized = m / graph->NumEdges();
        m = graph->ProcessVertexInRange<double>(
                [&](size_t v) {
                    for (auto &e : graph->OutEdges(v)) {
                        e.edge_data /= normalized;
                    }
                    k[v] /= normalized;
                    return k[v];
                },
                0, num_vertices) /
            2;

        std::cout << "m = " << m << std::endl;

        min_label();
        update_e_tot();
        update_num_community(true);
        update_label_size();
        update_Q(true);
        std::cout << "init" << std::endl;
    }

    ~LeidenGraph() {
        // if (!if_base) {
        //     delete sub_graph;
        // }
    }

    ParallelVector<size_t> &get_label() { return label; }

    void set_base(bool _if_base) { if_base = _if_base; }

    void min_label() {
        auto min_label = graph->AllocVertexArray<size_t>();
        min_label.Fill(-1);
        graph->ProcessVertexInRange<size_t>(
            [&](size_t v) {
                write_min(&min_label[label[v]], v);
                return 0;
            },
            0, num_vertices);
        graph->ProcessVertexInRange<size_t>(
            [&](size_t v) {
                label[v] = min_label[label[v]];
                return 0;
            },
            0, num_vertices);
    }

    void update_label_size(ParallelVector<size_t> *l = nullptr) {
        if (l == nullptr) {
            l = &label;
        }
        label_size.Fill(0);
        graph->ProcessVertexInRange<int>(
            [&](size_t v) {
                write_add(&label_size[(*l)[v]], 1);
                return 0;
            },
            0, num_vertices);
    }

    void update_e_tot(ParallelVector<size_t> *l = nullptr) {
        if (l == nullptr) {
            l = &label;
        }
        e_tot.Fill(0);
        graph->ProcessVertexInRange<int>(
            [&](size_t v) {
                write_add(&e_tot[(*l)[v]], k[v]);
                return 0;
            },
            0, num_vertices);
    }

    size_t update_num_community(bool is_print = true) {
        auto bit = graph->AllocVertexSubset();

        num_community = graph->ProcessVertexInRange<size_t>(
            [&](size_t v) {
                if (e_tot[v] < 1e-5) {
                    return 0;
                } else {
                    return 1;
                }
                return 0;
            },
            0, num_vertices);
        if (is_print) {
            std::cout << "number of communities is " << num_community << std::endl;
        }
        return num_community;
    }

    size_t get_num_community() { return num_community; }

    double update_Q(bool is_print = true) {
        Q = graph->ProcessVertexInRange<double>(
                [&](size_t v) {
                    double q = 0.0;
                    for (auto e : graph->OutEdges(v)) {
                        size_t nbr = e.neighbour;
                        if (label[v] == label[nbr]) q += e.edge_data;
                    }
                    q -= 1.0 * gamma * k[v] * e_tot[label[v]] / (2 * m);
                    return q;
                },
                0, num_vertices) /
            (2.0 * m);
        if (is_print) {
            std::cout << "Q = " << Q << std::endl;
        }
        return Q;
    }

    double get_Q() { return Q; }

    ParallelVector<size_t> singleton_partition() {
        auto l = graph->AllocVertexArray<size_t>();
        graph->ProcessVertexInRange<int>(
            [&](size_t vtx) {
                l[vtx] = vtx;
                return 0;
            },
            0, num_vertices);
        update_e_tot(&l);
        update_label_size(&l);
        return l;
    }

    void aggregate_graph(ParallelVector<size_t> &l) {
        std::cout << std::endl;
        size_t num_sub_vertices = 0;
        auto label_bitmap = graph->AllocVertexSubset();
        sub_index.Fill((size_t)-1);
        for (size_t v_i = 0; v_i < num_vertices; v_i++) {
            if (label_bitmap.Has(l[v_i])) {
                sub_index[v_i] = sub_index[l[v_i]];
            } else {
                sub_index[l[v_i]] = num_sub_vertices;
                sub_index[v_i] = num_sub_vertices;
                num_sub_vertices++;
                label_bitmap.Add(l[v_i]);
            }
        }

        std::vector<std::unordered_map<size_t, double> *> sub_edges(num_sub_vertices);
        for (auto &m_ptr : sub_edges) {
            m_ptr = new std::unordered_map<size_t, double>();
        }

        for (size_t v_i = 0; v_i < num_vertices; v_i++) {
            for (auto &e : graph->OutEdges(v_i)) {
                auto v_sub_node = sub_index[v_i];
                auto neighbour_sub_node = sub_index[e.neighbour];
                auto edge_data = e.edge_data;
                auto pair_it = sub_edges[v_sub_node]->find(neighbour_sub_node);
                if (pair_it == sub_edges[v_sub_node]->end()) {
                    sub_edges[v_sub_node]->insert({neighbour_sub_node, edge_data});
                } else {
                    pair_it->second += edge_data;
                }
            }
        }

        size_t num_sub_edges = 0;
        for (size_t i = 0; i < num_sub_vertices; i++) {
            num_sub_edges += sub_edges[i]->size();
        }

        EdgeUnit<double> *sub_edge_array = new EdgeUnit<double>[num_sub_edges];
        size_t sub_edge_index = 0;
        for (size_t v_i = 0; v_i < num_sub_vertices; v_i++) {
            for (auto &edge_pair : *sub_edges[v_i]) {
                sub_edge_array[sub_edge_index].src = v_i;
                sub_edge_array[sub_edge_index].dst = edge_pair.first;
                sub_edge_array[sub_edge_index].edge_data = edge_pair.second;
                sub_edge_index++;
            }
            delete sub_edges[v_i];
        }
        assert(sub_edge_index == num_sub_edges);

        sub_graph = new OlapBase<double>;
        sub_graph->LoadFromArray((char*)sub_edge_array, num_sub_vertices,
                                        num_sub_edges, INPUT_SYMMETRIC);
        delete[] sub_edge_array;

        sub_graph->AllocVertexArray<size_t>().Swap(sub_label);
        sub_label.Fill(0);

        graph->ProcessVertexActive<int>(
            [&](size_t v_i) {
                sub_label[sub_index[v_i]] = sub_index[label[v_i]];
                return 1;
            },
            label_bitmap);
    }

    bool move_nodes_fast() {
        auto active_next = graph->AllocVertexSubset();
        int iters = 0;
        size_t active_vertices = 1;
        size_t all_vertices = 0;
        while (active_vertices != 0) {
            active_next.Clear();
            active_vertices = graph->ProcessVertexActive<size_t>(
                [&](size_t v) {
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
                    double delta_in = k[v] * (e_tot[old_label] - k[v]) * gamma - 2.0 * k_in_out * m;

                    double delta_in_max = -delta_in;
                    size_t label_max = old_label;
                    for (auto &ele : count) {
                        size_t new_label = ele.first;
                        if (old_label == new_label) continue;
                        double k_in_in = ele.second;
                        delta_in = 2.0 * k_in_in * m - k[v] * (e_tot[new_label]) * gamma;
                        if (delta_in > delta_in_max) {
                            delta_in_max = delta_in;
                            label_max = new_label;
                        } else if (delta_in == delta_in_max && new_label < label_max) {
                            delta_in_max = delta_in;
                            label_max = new_label;
                        }
                    }

                    if (delta_in_max > 0 && label_max != old_label) {
                        write_sub(&e_tot[old_label], k[v]);
                        write_sub(&label_size[old_label], 1);
                        label[v] = label_max;
                        write_add(&e_tot[label_max], k[v]);
                        write_add(&label_size[label_max], 1);
                        for (auto &edge : graph->OutEdges(v)) {
                            if (label[edge.neighbour] != label_max) active_next.Add(edge.neighbour);
                        }
                        return 1;
                    }
                    if (delta_in_max < 0) {
                        write_sub(&e_tot[old_label], k[v]);
                        write_sub(&label_size[old_label], 1);
                        label[v] = v;
                        write_add(&e_tot[v], k[v]);
                        write_add(&label_size[v], 1);
                        for (auto &edge : graph->OutEdges(v)) {
                            if (label[edge.neighbour] != v) active_next.Add(edge.neighbour);
                        }
                        return 1;
                    }
                    return 0;
                },
                active);
            std::cout << "active_vertices(" << iters << ") = " << active_vertices << std::endl;
            iters++;
            active.Swap(active_next);
            all_vertices += active_vertices;
        }
        active.Fill();
        update_num_community();
        return (all_vertices <= num_vertices / threshold);
    }

    ParallelVector<size_t> refine_partition() {
        auto l = singleton_partition();
        auto comm_size = graph->AllocVertexArray<size_t>();
        comm_size.Fill(1);
        auto comm_degree = graph->AllocVertexArray<double>();
        comm_degree.Fill(0);
        auto sub_size = graph->AllocVertexArray<size_t>();
        sub_size.Fill(0);
        auto degree_in_sub = graph->AllocVertexArray<double>();
        degree_in_sub.Fill(0);
        well_connected.Clear();

        graph->ProcessVertexInRange<int>(
            [&](size_t vtx) {
                double c_degree = 0;
                double in_degree = 0;
                write_add(&sub_size[label[vtx]], (size_t)1);
                for (auto &edge : graph->OutEdges(vtx)) {
                    size_t nbr = edge.neighbour;
                    if (nbr == vtx || label[vtx] != label[nbr]) {
                        continue;
                    }
                    in_degree += edge.edge_data;
                    if (l[vtx] != l[nbr]) {
                        c_degree += edge.edge_data;
                    }
                    write_add(&degree_in_sub[vtx], in_degree);
                    write_add(&comm_degree[l[vtx]], c_degree);
                }
                return 0;
            },
            0, num_vertices);

        graph->ProcessVertexInRange<int>(
            [&](size_t vtx) {
                if (degree_in_sub[vtx] >= gamma * (double)(sub_size[label[vtx]] - 1)) {
                    well_connected.Add(vtx);
                }
                return 0;
            },
            0, num_vertices);

        graph->ProcessVertexActive<int>(
            [&](size_t vtx) {
                if (comm_size[l[vtx]] != 1) {
                    return 0;
                }
                std::unordered_map<size_t, double> count;
                std::unordered_map<size_t, double> pr;
                std::unordered_map<size_t, double> v_degree;

                for (auto e : graph->OutEdges(vtx)) {
                    size_t nbr = e.neighbour;
                    if (vtx == nbr || label[nbr] != label[vtx]) {
                        continue;
                    }
                    size_t nbr_label = l[nbr];
                    auto it_d = v_degree.find(nbr_label);
                    if (it_d != v_degree.end()) {
                        it_d->second += e.edge_data;
                    } else {
                        v_degree[nbr_label] = e.edge_data;
                    }
                    auto it = count.find(nbr_label);
                    if (it != count.end()) {
                        it->second += e.edge_data;
                    } else if (comm_degree[nbr_label] >=
                               gamma * comm_size[nbr_label] *
                                   (sub_size[label[vtx]] - comm_size[nbr_label])) {
                        count[nbr_label] = e.edge_data;
                    }
                }

                size_t old_label = l[vtx];
                pr[old_label] = 1;
                double k_in_in = 0.0;
                double delta_in = 0.0;

                for (auto &ele : count) {
                    size_t new_label = ele.first;
                    if (old_label == new_label) continue;
                    k_in_in = ele.second;

                    delta_in = k_in_in - k[vtx] * (e_tot[new_label]) / 2.0 / m * gamma;
                    if (delta_in >= 0) {
                        pr[new_label] = std::exp(1.0 / theta * delta_in);
                    }
                }

                size_t chosen_label = choice(random_seed, &pr);
                write_add(&e_tot[old_label], -k[vtx]);
                write_add(&comm_size[old_label], -1);
                write_add(&label_size[old_label], -1);
                write_add(&comm_degree[old_label], -k[vtx] + 2 * v_degree[old_label]);
                l[vtx] = chosen_label;
                write_add(&e_tot[chosen_label], k[vtx]);
                write_add(&comm_size[chosen_label], 1);
                write_add(&label_size[chosen_label], 1);
                write_add(&comm_degree[chosen_label], k[vtx] - 2 * v_degree[chosen_label]);
                return 0;
            },
            well_connected);

        return l;
    }

    void leiden() {
        bool changed = move_nodes_fast();
        if (!changed && num_community != graph->NumVertices()) {
            auto l = refine_partition();
            aggregate_graph(l);
            sub_leiden =
                new LeidenGraph(sub_graph, sub_label, random_seed, theta, gamma, threshold);
            bool flag = true;
            while (flag) {
                flag = subgraph_leiden(l);
            }

            graph->ProcessVertexInRange<int>(
                [&](size_t v) {
                    size_t sub_vtx = sub_index[v];
                    if (sub_vtx == (size_t)-1) {
                        return 0;
                    }
                    size_t sub_index_v_comm = sub_leiden->label[sub_vtx];
                    label[v] = sub_index_v_comm;
                    return 0;
                },
                0, num_vertices);
            delete sub_leiden;
        }
    }

    bool subgraph_leiden(ParallelVector<size_t> &original_l) {
        bool not_changed = sub_leiden->move_nodes_fast();
        bool ret = !not_changed && sub_leiden->num_community != sub_graph->NumVertices();
        if (ret) {
            auto l = sub_leiden->refine_partition();
            sub_leiden->aggregate_graph(l);
            LeidenGraph *sub_sub_leiden = new LeidenGraph(
                sub_leiden->sub_graph, sub_leiden->sub_label, random_seed, theta, gamma, threshold);

            graph->ProcessVertexInRange<int>(
                [&](size_t v) {
                    auto sub_v = sub_index[v];
                    sub_index[v] = sub_leiden->sub_index[sub_v];
                    return 0;
                },
                0, num_vertices);
            delete sub_leiden;
            sub_leiden = sub_sub_leiden;
        }
        return ret;
    }
};

void LeidenCore(OlapBase<double> &graph, ParallelVector<size_t> &label, unsigned random_seed = 0,
                 double theta = 0.1, double gamma = 0.2, size_t threshold = 0) {
    graph.ProcessVertexInRange<int>(
        [&](size_t v) {
            label[v] = v;
            return 0;
        },
        0, graph.NumVertices());

    LeidenGraph leiden_graph(&graph, label, random_seed, theta, gamma, threshold);
    leiden_graph.set_base(true);
    leiden_graph.leiden();

    leiden_graph.update_e_tot();
    leiden_graph.update_label_size();
    leiden_graph.update_num_community(false);
    leiden_graph.update_Q(false);
    std::cout << "final number of communities is " << leiden_graph.get_num_community() << std::endl;
    std::cout << "final Q is " << leiden_graph.get_Q() << std::endl;
    label.Swap(leiden_graph.get_label());
}
