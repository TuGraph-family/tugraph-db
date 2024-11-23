/**
 * @Function: Compute the k-hop algorithm.
 * @param:
 *
 *
 *
 *
 graph:
 The graph to compute on.
 root_vid: Identifier of root node.
 result:
 Set of reachable nodes in K-hop.
 value_k: Number of search layers.
 * @return: Number of nodes in K_hop.
 */
// #include <cstddef>
#include "lgraph/olap_base.h"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;
size_t k_hop(OlapBase<Empty>& graph, size_t root_vid, ParallelVector<size_t>&result, size_t k)
{
    size_t root = root_vid;
    auto active_in = graph.AllocVertexSubset();
    active_in.Add(root);
    auto active_out = graph.AllocVertexSubset();
    auto parent=graph.AllocVertexArray<size_t>();
    parent.Fill(0);
    parent[root]=root;
    size_t num_activations = 1;
    size_t discovered_vertices, j = 0;
    for (size_t ii = 0; ii < k ; ii++)
    {
        active_out.Clear();
        num_activations = graph.ProcessVertexActive<size_t>(
            [&](size_t vi) {
                size_t num_activations = 0;
                for (auto& edge : graph.OutEdges(vi))
                {
                    size_t dst = edge.neighbour;
                    if (parent[dst] == 0)
                    {
                        auto lock = graph.GuardVertexLock(dst);
                        if(parent[dst] == 0)
                        {
                            parent[dst] = vi;
                            num_activations += 1;
                            active_out.Add(dst);
                            result[j++]=dst;
                        }
                    }
                }
                return num_activations;
            },
            active_in
        );
        printf("activates(%lu) <= %lu \n", ii+1, num_activations);
        discovered_vertices += num_activations;
        active_in.Swap(active_out);
    }
    return discovered_vertices;
}
