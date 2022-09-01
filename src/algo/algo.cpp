/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "algo/algo.h"

namespace lgraph {

typedef std::vector<std::pair<std::string, FieldData>> FieldValues;

static void FetchPath(Transaction& txn, size_t hops, std::unordered_map<VertexId, VertexId>& parent,
                      std::unordered_map<VertexId, VertexId>& child, VertexId vid_from,
                      VertexId vid_a, VertexId vid_b, VertexId vid_to, std::vector<VertexId>* vids,
                      std::vector<FieldValues>* vprops, std::vector<FieldValues>* eprops) {
    // if all are empty, then we just need a confirmation
    if (!vids && !vprops && !eprops) return;

    std::vector<VertexId> path_vids;
    path_vids.reserve(hops + 1);
    VertexId vid;

    vid = vid_a;
    while (vid != vid_from) {
        path_vids.push_back(vid);
        vid = parent[vid];
    }
    path_vids.push_back(vid);
    std::reverse(path_vids.begin(), path_vids.end());
    vid = vid_b;
    while (vid != vid_to) {
        path_vids.push_back(vid);
        vid = child[vid];
    }
    path_vids.push_back(vid);
    FMA_CHECK_EQ(path_vids.size(), hops + 1);

    if (vids) *vids = std::move(path_vids);
    // need just the vids
    if (!vprops && !eprops) return;

    std::vector<FieldValues> vertices;
    vertices.reserve(path_vids.size());
    std::vector<FieldValues> edges;
    edges.reserve(path_vids.size() - 1);
    for (size_t i = 0; i < path_vids.size(); i++) {
        VertexId vid = path_vids[i];
        auto vit = txn.GetVertexIterator(vid);
        FMA_ASSERT(vit.IsValid());
        vertices.emplace_back(txn.GetVertexFields(vit));
        if (eprops && i != path_vids.size() - 1) {
            VertexId dst = path_vids[i + 1];
            auto eit = vit.GetOutEdgeIterator(EdgeUid(vid, dst, 0, 0, 0), true);
            if (eit.IsValid() && eit.GetDst() == dst) {
                edges.emplace_back(txn.GetEdgeFields(eit));
                continue;
            } else {
                for (auto it = vit.GetOutEdgeIterator(); it.IsValid(); it.Next()) {
                    if (it.GetDst() == dst) edges.emplace_back(txn.GetEdgeFields(it));
                }
            }
        }
    }

    if (vprops) *vprops = std::move(vertices);
    if (eprops) *eprops = std::move(edges);
}

/**
 * Peer-to-peer unweighted shortest path. Gets the shortest path from source to destination.
 *
 * \param           vid_from    The source vertex.
 * \param           vid_to      The destination vertex.
 * \param           max_hops    The maximum hops.
 * \param [in,out]  path_vids   Non-null. Returns vids in the path.
 * \param [in,out]  vprops      If non-null, returns properties of the vertexes in the path.
 * \param [in,out]  eprops      If non-null, returns properties of the edges in the path.
 */
void P2PUnweightedShortestPath(Transaction& txn, VertexId vid_from, VertexId vid_to,
                               size_t max_hops, std::vector<VertexId>* path_vids,
                               std::vector<FieldValues>* vprops, std::vector<FieldValues>* eprops) {
    static fma_common::Logger& logger = fma_common::Logger::Get("algo.p2puwssp");

    path_vids->clear();
    if (vprops) vprops->clear();
    if (eprops) eprops->clear();
    if (vid_from == vid_to) {
        path_vids->push_back(vid_from);
        return;
    }

    std::unordered_map<VertexId, VertexId> parent;
    std::unordered_map<VertexId, VertexId> child;
    std::vector<VertexId> forward_q, backward_q;
    parent[vid_from] = vid_from;
    child[vid_to] = vid_to;
    forward_q.push_back(vid_from);
    backward_q.push_back(vid_to);
    size_t hops = 0;
    while (hops++ < max_hops) {
        std::vector<VertexId> new_front;
        // decide which way to search first
        if (forward_q.size() <= backward_q.size()) {
            // search forward
            for (VertexId vid : forward_q) {
                auto vit = txn.GetVertexIterator(vid);
                FMA_DBG_ASSERT(vit.IsValid());
                bool edge_left = false;
                std::vector<VertexId> dstIds =
                    vit.ListDstVids(nullptr, nullptr, 10000, &edge_left, nullptr);
                if (edge_left) {
                    FMA_WARN_STREAM(logger) << "Result trimmed down because " << vid
                                            << " has more than 10000 out neighbours";
                }
                for (VertexId dst : dstIds) {
                    if (child.find(dst) != child.end()) {
                        // found the path
                        FetchPath(txn, hops, parent, child, vid_from, vid, dst, vid_to, path_vids,
                                  vprops, eprops);
                        return;
                    }
                    auto it = parent.find(dst);
                    if (it == parent.end()) {
                        parent.emplace_hint(it, dst, vid);
                        new_front.push_back(dst);
                    }
                }
            }
            if (new_front.empty()) break;
            forward_q = std::move(new_front);
        } else {
            for (VertexId vid : backward_q) {
                auto vit = txn.GetVertexIterator(vid);
                FMA_DBG_ASSERT(vit.IsValid());
                bool edge_left = false;
                std::vector<VertexId> srcIds =
                    vit.ListSrcVids(nullptr, nullptr, 10000, &edge_left, nullptr);
                if (edge_left) {
                    FMA_WARN_STREAM(logger) << "Result trimmed down because " << vid
                                            << " has more than 10000 in neighbours";
                }
                for (VertexId src : srcIds) {
                    if (parent.find(src) != parent.end()) {
                        // found the path
                        FetchPath(txn, hops, parent, child, vid_from, src, vid, vid_to, path_vids,
                                  vprops, eprops);
                        return;
                    }
                    auto it = child.find(src);
                    if (it == child.end()) {
                        child.emplace_hint(it, src, vid);
                        new_front.push_back(src);
                    }
                }
            }
            if (new_front.empty()) break;
            backward_q = std::move(new_front);
        }
    }
}

/**
 * BFS neighbour-finding
 *
 * \param [in,out]  txn         The transaction.
 * \param           k           A size_t to process.
 * \param           start       The id of the starting vertex.
 * \param [in,out]  vids        Returns vids of the neighbours. vids[i] contains neighbours that
 *                              are i step from start. So vids[0] contains start itself.
 * \param           n_limit     (Optional) Maximum number of neighbours.
 * \param [in,out]  fields      (Optional) If non-null, we returns the fields of the neighbours
 *                              in field_data.
 * \param [in,out]  field_data  (Optional) If non-null, information describing the field.
 */
void KNeighbours(Transaction& txn, size_t k, VertexId start,
                 std::vector<std::vector<VertexId>>* vids, size_t n_limit,
                 std::vector<std::string>* fields,
                 std::vector<std::vector<std::vector<FieldData>>>* field_data) {
    if (!vids) throw InputError("Invalid pointer");
    if (fields && !field_data)
        throw InputError("Invalid parameter: fields is not null but field_data is null");
    std::vector<std::vector<VertexId>>& result = *vids;
    result.resize(k);
    if (fields) field_data->resize(k);

    std::unordered_set<VertexId> known_vids;
    std::vector<VertexId> curr_level;
    curr_level.push_back(start);
    known_vids.insert(start);
    n_limit = (n_limit == 0) ? std::numeric_limits<size_t>::max() : n_limit;
    size_t level = 0;
    while (level < k) {
        std::vector<VertexId> next_level;
        for (VertexId vid : curr_level) {
            result[level].push_back(vid);
            const graph::VertexIterator& vit = txn.GetVertexIterator(vid);
            if (fields) (*(field_data))[level].emplace_back(txn.GetVertexFields(vit, *fields));
            for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                if (known_vids.size() >= n_limit) break;
                VertexId v = eit.GetDst();
                auto it = known_vids.find(v);
                if (it == known_vids.end()) {
                    next_level.push_back(*it);
                    known_vids.insert(it, v);
                }
            }
        }
        if (next_level.empty()) break;
        curr_level.swap(next_level);
        level++;
    }
}
}  // namespace lgraph
