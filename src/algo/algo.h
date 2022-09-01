/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include "core/transaction.h"

namespace lgraph {
/**
 * Peer-to-peer unweighted shortest path. Gets the shortest path from source to destination.
 *
 * \param [in,out]  txn         The transaction.
 * \param           vid_from    The source vertex.
 * \param           vid_to      The destination vertex.
 * \param           max_hops    The maximum hops.
 * \param [in,out]  path_vids   Non-null. Returns vids in the path.
 * \param [in,out]  vprops      If non-null, returns properties of the vertexes in the path.
 * \param [in,out]  eprops      If non-null, returns properties of the edges in the path.
 */
void P2PUnweightedShortestPath(Transaction& txn, VertexId vid_from, VertexId vid_to,
                               size_t max_hops, std::vector<VertexId>* path_vids,
                               std::vector<std::vector<std::pair<std::string, FieldData>>>* vprops,
                               std::vector<std::vector<std::pair<std::string, FieldData>>>* eprops);

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
                 std::vector<std::vector<VertexId>>* vids, size_t n_limit = 0,
                 std::vector<std::string>* fields = nullptr,
                 std::vector<std::vector<std::vector<FieldData>>>* field_data = nullptr);
}  // namespace lgraph
