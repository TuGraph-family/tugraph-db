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

#include <atomic>
#include <memory>

#include "core/graph_vertex_iterator.h"
#include "core/kv_store.h"

namespace lgraph {
class Transaction;
namespace import_v2 {
class Importer;
class ImportOnline;
}  // namespace import_v2
namespace graph {

/**
 * A graph stored in kv-store.
 *
 * For each vertex, we first create a PACKED_DATA node, in which the key is the vid and
 * PackType::PACKED_DATA.
 *
 * If the packed data grows too large, we split it into three different nodes: a VERTEX_ONLY
 * node, a IN_EDGE node and a OUT_EDGE node.
 *
 * The VERTEX_ONLY node has vid and PackType::VERTEX_ONLY as the key.
 *
 * The IN_EDGE node has the (vid, PackType::IN_EDGE, lid, src, eid) as key, in which
 * (lid, src, eid) is the LabelId, source VertexId and EdgeId of the last edge stored
 * in each OutEdge node.
 *
 * And OUT_EDGE node has (vid, PackType::OUT_EDGE, lid, dst, eid) as the key.
 *
 * Both IN_EDGE and OUT_EDGE nodes can further be split when their size grows too large. To find
 * the node in which a specific edge resides, search for the node with ClosestKey() and then
 * search for the that key in the edge node.
 */
class Graph {
    typedef EdgeIteratorImpl<PackType::OUT_EDGE> OutIteratorImpl;
    typedef EdgeIteratorImpl<PackType::IN_EDGE> InIteratorImpl;
    typedef VertexIteratorImpl VIteratorImpl;

    friend class ::lgraph::Transaction;
    friend class ::lgraph::import_v2::Importer;
    friend class ::lgraph::import_v2::ImportOnline;

    /*
     * NOTE: This class is not locked. The user of this class must guarantee
     * that there is at most one writer.
     */
    std::unique_ptr<KvTable> table_;
    std::shared_ptr<KvTable> meta_table_;

 public:
    /**
     * Constructor
     *
     * \param [in,out]  txn             The transaction.
     * \param           t               A kvstore::Table used to store the graph data.
     * \param           meta_table      Pointer to meta table.
     * Otherwise, the user is responsible for deleting incoming edges manually.
     */
    Graph(KvTransaction& txn, std::unique_ptr<KvTable> t, std::shared_ptr<KvTable> meta_table)
        : table_(std::move(t)), meta_table_(std::move(meta_table)) {
    }

    /**
     * Opens or create the graph table. Use this to open the table before you can construct the
     * Graph object.
     *
     * \param [in,out]  txn     The transaction.
     * \param [in,out]  store   The kv-store.
     * \param           name    The name of the graph table.
     *
     * \return  A kvstore::Table.
     */
    static std::unique_ptr<KvTable> OpenTable(
        KvTransaction& txn, KvStore& store, const std::string& name) {
        return store.OpenTable(txn, name, true, ComparatorDesc::DefaultComparator());
    }

    void RefreshNextVid(KvTransaction& txn) {
        // try getting next vid, if not exist, write one
        auto git = table_->GetIterator(txn);
        git->GotoFirstKey();
        VertexId nvid = 0;
        if (!git->IsValid()) {
            // graph empty, write 0
            nvid = 0;
        } else {
            git->GotoLastKey();
            nvid = KeyPacker::GetFirstVid(git->GetKey()) + 1;
        }
        auto key = Value::ConstRef(lgraph::_detail::NEXT_VID_KEY);
        auto it = meta_table_->GetIterator(txn, key);
        if (!it->IsValid() || it->GetValue().AsType<VertexId>() != nvid) {
            meta_table_->SetValue(txn, key, Value::ConstRef(nvid));
        }
    }

    /**
     * Gets vertex iterator
     *
     * \param [in,out]  txn The transaction.
     *
     * \return  The vertex iterator.
     */
    VertexIterator GetVertexIterator(::lgraph::Transaction* txn) {
        return VertexIterator(txn, *table_, 0, true);
    }

    VertexIterator GetUnmanagedVertexIterator(KvTransaction* txn) {
        return VertexIterator(txn, *table_, 0, true);
    }

    /**
     * Gets vertex iterator
     *
     * \param [in,out]  txn The transaction.
     * \param           src Source for the.
     *
     * \return  The vertex iterator.
     */
    VertexIterator GetVertexIterator(::lgraph::Transaction* txn, VertexId src,
                                     bool nearest = false) {
        return VertexIterator(txn, *table_, src, nearest);
    }

    VertexIterator GetUnmanagedVertexIterator(KvTransaction* txn, VertexId src,
                                              bool nearest = false) {
        return VertexIterator(txn, *table_, src, nearest);
    }

    OutEdgeIterator GetOutEdgeIterator(::lgraph::Transaction* txn, const EdgeUid& euid,
                                       bool closest) {
        return OutEdgeIterator(txn, *table_, euid, closest);
    }

    OutEdgeIterator GetUnmanagedOutEdgeIterator(KvTransaction* txn, const EdgeUid& euid,
                                                bool closest) {
        return OutEdgeIterator(txn, *table_, euid, closest);
    }

    InEdgeIterator GetInEdgeIterator(::lgraph::Transaction* txn, const EdgeUid& euid,
                                     bool closest) {
        return InEdgeIterator(txn, *table_, euid, closest);
    }

    InEdgeIterator GetUnmanagedInEdgeIterator(KvTransaction* txn, const EdgeUid& euid,
                                              bool closest) {
        return InEdgeIterator(txn, *table_, euid, closest);
    }

    /**
     * Adds a new vertex
     *
     * \param [in,out]  txn     The transaction.
     * \param           prop    The property.
     *
     * \return  The id of the newly added vertex
     */
    VertexId AddVertex(KvTransaction& txn, const Value& prop) {
        VertexId vid = GetAndIncNextVid(txn);
        if (vid >= ::lgraph::_detail::MAX_VID) {
            throw std::runtime_error("Max vertex id reached. DB can only hold 1<<40 vertcies.");
        }
        VertexValue vov(prop);
        if (prop.Size() > ::lgraph::_detail::NODE_SPLIT_THRESHOLD) {
            // create a vertex-only node
            table_->AppendKv(txn, KeyPacker::CreateVertexOnlyKey(vid), vov.GetBuf());
        } else {
            // create a packed data node
            Value v;
            PackedDataValue::PackData(vov, EdgeValue(), EdgeValue(), v);
            table_->AppendKv(txn, KeyPacker::CreatePackedDataKey(vid), v);
        }
        return vid;
    }

    /**
     * Adds an edge
     *
     * \param [in,out]  txn     The transaction.
     * \param           src     Source of the edge.
     * \param           lid     The label id.
     * \param           dst     Destination of the edge.
     * \param           prop    The edge property.
     * \param           constraints edge constraints
     *
     * \return  The id of the newly added edge. The edge id of the first src->dst edge is 0, and
     *          increments by one whenever a new src->dst edge is added.
     */
    EdgeUid AddEdge(KvTransaction& txn, VertexId src, LabelId lid, VertexId dst, const Value& prop,
                    const std::unordered_map<LabelId, std::unordered_set<LabelId>>&
                        constraints = {}) {
        return AddEdge(txn, EdgeSid(src, dst, lid, 0), prop, constraints);
    }

    /**
     * Adds an edge
     *
     * \param [in,out]  txn     The transaction.
     * \param           esid    Edge id without eid.
     * \param           prop    The edge property.
     * \param           constraints edge constraints
     *
     * \return  The id of the newly added edge. The edge id of the first src->dst edge is 0, and
     *          increments by one whenever a new src->dst edge is added.
     */
    EdgeUid AddEdge(KvTransaction& txn, EdgeSid esid, const Value& prop,
                    const std::unordered_map<LabelId, std::unordered_set<LabelId>>&
                        constraints) {
        // add out-going edge
        auto it = table_->GetIterator(txn);
        std::unique_ptr<EdgeConstraintsChecker> ec;
        if (!constraints.empty()) {
            ec = std::make_unique<EdgeConstraintsChecker>(constraints);
        }
        EdgeId eid = OutIteratorImpl::InsertEdge(esid, prop, *it, nullptr, ec.get());
        EdgeSid esid_reverse = esid;
        esid_reverse.Reverse();
        EdgeId ee = InIteratorImpl::InsertEdge(esid_reverse, prop, *it, &eid, ec.get());
        FMA_DBG_CHECK_EQ(ee, eid);
        return esid.ConstructEdgeUid(eid);
    }

    /**
     * Deletes the vertex
     *
     * \param [in,out]  txn     The transaction.
     * \param           vid     The vid.
     * \param           on_edge_deleted  Callback function when deleting edges
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool DeleteVertex(KvTransaction& txn, VertexId vid,
                      const std::function<void(bool, const EdgeValue&)>&
                          on_edge_deleted = nullptr) {
        auto it = table_->GetIterator(txn);
        it->GotoClosestKey(KeyPacker::CreatePackedDataKey(vid));
        if (!it->IsValid() || KeyPacker::GetFirstVid(it->GetKey()) != vid)
            return false;  // no such vertex
        DeleteVertexInternal(txn, *it, on_edge_deleted);
        return true;
    }

    void DeleteVertex(KvTransaction& txn, VertexIterator& vertex_it,
                      const std::function<void(bool, const EdgeValue&)>&
                          on_edge_deleted = nullptr) {
        if (!vertex_it.IsValid()) return;
        KvIterator& it = vertex_it.GetIt();
        DeleteVertexInternal(txn, it, on_edge_deleted);
        vertex_it.LoadContentFromIt();
    }

    /**
     * Delete an edge
     *
     * \param [in,out]  txn The transaction.
     * \param           uid The Edge unique id.
     *
     * \return  True if the edge is deleted successfully. False if the edge does not exist. Raises
     *          exception if other errors occur.
     */
    bool DeleteEdge(KvTransaction& txn, const EdgeUid& uid) {
        auto it = table_->GetIterator(txn);
        OutIteratorImpl oeit(*it);
        oeit.Goto(uid, false);
        if (!oeit.IsValid()) return false;
        oeit.Delete();
        InIteratorImpl ieit(*it);
        ieit.Goto(uid, false);
        FMA_DBG_ASSERT(ieit.IsValid());
        ieit.Delete();
        return true;
    }

    void DeleteEdge(KvTransaction& txn, OutEdgeIterator& oeit) {
        EdgeUid euid(oeit.GetSrc(), oeit.GetDst(), oeit.GetLabelId(), oeit.GetTemporalId(),
                     oeit.GetEdgeId());
        oeit.Delete();
        // delete in-edge of destination vertex
        auto it = table_->GetIterator(txn);
        InIteratorImpl ieit(*it);
        ieit.Goto(euid, false);
        FMA_DBG_ASSERT(ieit.IsValid());
        ieit.Delete();
    }

    void DeleteEdge(KvTransaction& txn, InEdgeIterator& ieit) {
        EdgeUid euid(ieit.GetSrc(), ieit.GetDst(), ieit.GetLabelId(), ieit.GetTemporalId(),
                     ieit.GetEdgeId());
        ieit.Delete();
        // delete out-edge of source vertex
        auto it = table_->GetIterator(txn);
        OutIteratorImpl oeit(*it);
        oeit.Goto(euid, false);
        FMA_DBG_ASSERT(oeit.IsValid());
        oeit.Delete();
    }

    /**
     * Sets vertex property
     *
     * \param [in,out]  txn     The transaction.
     * \param           vid     The vid.
     * \param           prop    The property.
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool SetVertexProperty(KvTransaction& txn, VertexId vid, const Value& prop) {
        auto it = table_->GetIterator(txn);
        VIteratorImpl vit(*it);
        vit.Goto(vid);
        if (!vit.IsValid()) return false;
        vit.SetProperty(prop);
        return true;
    }

    /**
     * Sets edge property
     *
     * \param [in,out]  txn         The transaction.
     * \param           uid         The Edge unique id.
     * \param           new_prop    The new property.
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool SetEdgeProperty(KvTransaction& txn, const EdgeUid& uid, const Value& new_prop) {
        auto it = table_->GetIterator(txn);
        OutIteratorImpl oeit(*it);
        oeit.Goto(uid, false);
        if (!oeit.IsValid()) return false;
        oeit.SetProperty(new_prop);
        InIteratorImpl ieit(*it);
        ieit.Goto(uid, false);
        FMA_ASSERT(ieit.IsValid());
        ieit.SetProperty(new_prop);
        return true;
    }

    /**
     * Drops the entire graph.
     *
     * \param [in,out]  txn The transaction.
     */
    void Drop(KvTransaction& txn) { table_->Drop(txn); }

    /** @breif Get next available vid. */
    VertexId GetAndIncNextVid(KvTransaction& txn) {
        static Value key = Value::ConstRef(lgraph::_detail::NEXT_VID_KEY);
        auto it = meta_table_->GetIterator(txn, key);
        if (!it->IsValid()) {
            meta_table_->AddKV(txn, key, Value::ConstRef((VertexId)1));
            return 0;
        } else {
            VertexId vid = it->GetValue().AsType<VertexId>();
            if (vid == lgraph::_detail::MAX_VID)
                THROW_CODE(InputError, "Maximum number of vertex reached: ", vid);
            it->SetValue(Value::ConstRef(vid + 1));
            return vid;
        }
    }

    VertexId GetNextVid(KvTransaction& txn) {
        static Value key = Value::ConstRef(lgraph::_detail::NEXT_VID_KEY);
        auto it = meta_table_->GetIterator(txn, key);
        if (!it->IsValid()) return 0;
        return it->GetValue().AsType<VertexId>();
    }

    VertexId GetLooseNumVertex(KvTransaction& txn) { return GetNextVid(txn); }

    void IncreaseCount(KvTransaction& txn, bool is_vertex, LabelId lid, int64_t delta) {
        std::string key;
        if (is_vertex) {
            key.append(lgraph::_detail::VERTEX_COUNT_PREFIX);
        } else {
            key.append(lgraph::_detail::EDGE_COUNT_PREFIX);
        }
        key.append((const char*)(&lid), sizeof(LabelId));
        auto k = Value::ConstRef(key);
        auto it = meta_table_->GetIterator(txn, k);
        if (!it->IsValid()) {
            int64_t count = delta;
            if (count < 0) {
                LOG_ERROR() << "Unexpected count value, is_vertex: " << is_vertex
                          << ", LabelId: " << lid << ", count: " << count;
            }
            meta_table_->AddKV(txn, k, Value::ConstRef(count));
        } else {
            int64_t count = it->GetValue().AsType<int64_t>();
            count += delta;
            if (count < 0) {
                LOG_ERROR() << "Unexpected count value, is_vertex: " << is_vertex
                          << ", LabelId: " << lid << ", count: " << count;
            }
            it->SetValue(Value::ConstRef(count));
        }
    }

    void DeleteCount(KvTransaction& txn, bool is_vertex, LabelId lid) {
        std::string key;
        if (is_vertex) {
            key.append(lgraph::_detail::VERTEX_COUNT_PREFIX);
        } else {
            key.append(lgraph::_detail::EDGE_COUNT_PREFIX);
        }
        key.append((const char*)(&lid), sizeof(LabelId));
        auto k = Value::ConstRef(key);
        auto it = meta_table_->GetIterator(txn, k);
        if (it->IsValid()) {
            it->DeleteKey();
        }
    }

    void DeleteAllCount(KvTransaction& txn) {
        std::vector<std::string> prefixes = {
            lgraph::_detail::VERTEX_COUNT_PREFIX,
            lgraph::_detail::EDGE_COUNT_PREFIX};

        for (auto& prefix : prefixes) {
            auto k = Value::ConstRef(prefix);
            for (auto it = meta_table_->GetClosestIterator(txn, k); it->IsValid();) {
                auto key = it->GetKey().AsString();
                if (!fma_common::StartsWith(key, prefix)) {
                    break;
                }
                it->DeleteKey();
            }
        }
    }

    int64_t GetCount(KvTransaction& txn, bool is_vertex, LabelId lid) {
        std::string key;
        if (is_vertex) {
            key.append(lgraph::_detail::VERTEX_COUNT_PREFIX);
        } else {
            key.append(lgraph::_detail::EDGE_COUNT_PREFIX);
        }
        key.append((const char*)(&lid), sizeof(LabelId));
        auto k = Value::ConstRef(key);
        auto it = meta_table_->GetIterator(txn, k);
        if (it->IsValid()) {
            return it->GetValue().AsType<int64_t>();
        } else {
            return 0;
        }
    }

    void AppendDataRaw(KvTransaction& txn, const Value& k, const Value& v) {
        table_->AppendKv(txn, k, v);
    }

    // returns the KvTable, used by Transaction only
    KvTable& _GetKvTable() { return *this->table_; }

    // scan and delete
    // sets n_nodes to number of nodes deleted
    // sets n_ins to number of in edges deleted
    // sets n_outs to number of out edges deleted
    void _ScanAndDelete(KvStore& store, KvTransaction& txn,
                        const std::function<bool(VertexIterator&)>& should_delete_node,
                        const std::function<bool(InEdgeIterator&)>& should_delete_in_edge,
                        const std::function<bool(OutEdgeIterator&)>& should_delete_out_edge,
                        size_t& n_modified, size_t commit_size = 100000);

 private:
    void DeleteCorrespondingInEdges(KvTransaction& txn, KvIterator& tmp_it, VertexId src,
                                    const EdgeValue& oev) {
        InIteratorImpl ieit(tmp_it);
        size_t ne = oev.GetEdgeCount();
        for (size_t i = 0; i < ne; i++) {
            // TODO(hjk41): optimize this
            // when there are multiple edges with the same (src, tid, lid, dst), we can delete
            // these edges in one go
            auto e = oev.GetNthEdgeHeader(i);
            // do not delete self-loop, as they will be deleted by DeleteVertex
            if (e.vid == src) continue;
            ieit.Goto(EdgeUid(src, e.vid, e.lid, e.tid, e.eid), false);
            FMA_ASSERT(ieit.IsValid());
            ieit.Delete();
        }
    }

    void DeleteCorrespondingOutEdges(KvTransaction& txn, KvIterator& tmp_it, VertexId dst,
                                     const EdgeValue& iev) {
        OutIteratorImpl oeit(tmp_it);
        size_t ne = iev.GetEdgeCount();
        for (size_t i = 0; i < ne; i++) {
            // TODO(hjk41): optimize this
            // when there are multiple edges with the same (src, tid, lid, dst), we can delete
            // these edges in one go
            auto e = iev.GetNthEdgeHeader(i);
            // do not delete self-loop, as they will be deleted by DeleteVertex
            if (e.vid == dst) continue;
            oeit.Goto(EdgeUid(e.vid, dst, e.lid, e.tid, e.eid), false);
            FMA_ASSERT(oeit.IsValid());
            oeit.Delete();
        }
    }

    /**
     * Deletes the vertex internal
     *
     * \param [in,out]  txn     The transaction.
     * \param [in,out]  it      The iterator.
     * \param [in]      on_edge_deleted Callback function when deleting edges
     * \return  True if it succeeds, false if it fails.
     */
    void DeleteVertexInternal(KvTransaction& txn, KvIterator& it,
                              const std::function<void(bool, const EdgeValue&)>&
                                  on_edge_deleted = nullptr) {
        const Value& k = it.GetKey();
        VertexId vid = KeyPacker::GetFirstVid(k);
        PackType pt = KeyPacker::GetNodeType(k);
        if (pt == PackType::PACKED_DATA) {
            PackedDataValue pdv(Value::MakeCopy(it.GetValue()));
            // tmp_it is modified when deleting edges, do not use it for other purpose
            auto tmp_it = table_->GetIterator(txn);
            const EdgeValue& oev = pdv.GetOutEdge();
            if (on_edge_deleted) {
                on_edge_deleted(true, oev);
            }
            DeleteCorrespondingInEdges(txn, *tmp_it, vid, oev);
            // delete incoming edges
            const EdgeValue& iev = pdv.GetInEdge();
            if (on_edge_deleted) {
                on_edge_deleted(false, iev);
            }
            DeleteCorrespondingOutEdges(txn, *tmp_it, vid, iev);
            // delete vertex
            it.RefreshAfterModify();
            it.DeleteKey();
        } else {
            // delete vertex-only node
            FMA_DBG_CHECK_EQ(KeyPacker::GetNodeType(it.GetKey()), PackType::VERTEX_ONLY);
            it.DeleteKey();
            // now it points to out-edge node
            // tmp_it is modified when deleting edges, do not use it for other purpose
            auto tmp_it = table_->GetIterator(txn);
            while (it.IsValid() && KeyPacker::GetNodeType(it.GetKey()) == PackType::OUT_EDGE) {
                FMA_DBG_CHECK_EQ(KeyPacker::GetFirstVid(it.GetKey()), vid);
                EdgeValue oev(Value::MakeCopy(it.GetValue()));
                if (on_edge_deleted) {
                    on_edge_deleted(true, oev);
                }
                DeleteCorrespondingInEdges(txn, *tmp_it, vid, oev);
                it.RefreshAfterModify();
                it.DeleteKey();
            }
            // delete incoming edges
            while (it.IsValid() && KeyPacker::GetNodeType(it.GetKey()) == PackType::IN_EDGE) {
                EdgeValue iev(Value::MakeCopy(it.GetValue()));
                if (on_edge_deleted) {
                    on_edge_deleted(false, iev);
                }
                DeleteCorrespondingOutEdges(txn, *tmp_it, vid, iev);
                it.RefreshAfterModify();
                it.DeleteKey();
            }
        }
        FMA_DBG_ASSERT(!it.IsValid() || KeyPacker::GetFirstVid(it.GetKey()) != vid);
    }
};
}  // namespace graph
}  // namespace lgraph
