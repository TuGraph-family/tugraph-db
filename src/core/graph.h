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
    KvTable table_;
    KvTable* meta_table_;

 public:
    /**
     * Constructor
     *
     * \param [in,out]  txn             The transaction.
     * \param           t               A kvstore::Table used to store the graph data.
     * \param           meta_table      Pointer to meta table.
     * Otherwise, the user is responsible for deleting incoming edges manually.
     */
    Graph(KvTransaction& txn, const KvTable& t, KvTable* meta_table)
        : table_(t), meta_table_(meta_table) {
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
    static KvTable OpenTable(KvTransaction& txn, KvStore& store, const std::string& name) {
        return store.OpenTable(txn, name, true, ComparatorDesc::DefaultComparator());
    }

    void RefreshNextVid(KvTransaction& txn) {
        // try getting next vid, if not exist, write one
        auto git = table_.GetIterator(txn);
        VertexId nvid = 0;
        if (!git.IsValid()) {
            // graph empty, write 0
            nvid = 0;
        } else {
            git.GotoLastKey();
            nvid = KeyPacker::GetFirstVid(git.GetKey()) + 1;
        }
        auto key = Value::ConstRef(lgraph::_detail::NEXT_VID_KEY);
        auto it = meta_table_->GetIterator(txn, key);
        if (!it.IsValid() || it.GetValue().AsType<VertexId>() != nvid) {
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
        return VertexIterator(txn, table_, 0, true);
    }

    VertexIterator GetUnmanagedVertexIterator(KvTransaction* txn) {
        return VertexIterator(txn, table_, 0, true);
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
        return VertexIterator(txn, table_, src, nearest);
    }

    VertexIterator GetUnmanagedVertexIterator(KvTransaction* txn, VertexId src,
                                              bool nearest = false) {
        return VertexIterator(txn, table_, src, nearest);
    }

    OutEdgeIterator GetOutEdgeIterator(::lgraph::Transaction* txn, const EdgeUid& euid,
                                       bool closest) {
        return OutEdgeIterator(txn, table_, euid, closest);
    }

    OutEdgeIterator GetUnmanagedOutEdgeIterator(KvTransaction* txn, const EdgeUid& euid,
                                                bool closest) {
        return OutEdgeIterator(txn, table_, euid, closest);
    }

    InEdgeIterator GetInEdgeIterator(::lgraph::Transaction* txn, const EdgeUid& euid,
                                     bool closest) {
        return InEdgeIterator(txn, table_, euid, closest);
    }

    InEdgeIterator GetUnmanagedInEdgeIterator(KvTransaction* txn, const EdgeUid& euid,
                                              bool closest) {
        return InEdgeIterator(txn, table_, euid, closest);
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
            table_.AppendKv(txn, KeyPacker::CreateVertexOnlyKey(vid), vov.GetBuf());
        } else {
            // create a packed data node
            Value v;
            PackedDataValue::PackData(vov, EdgeValue(), EdgeValue(), v);
            table_.AppendKv(txn, KeyPacker::CreatePackedDataKey(vid), v);
        }
        return vid;
    }

    /**
     * Adds an edge
     *
     * \param [in,out]  txn     The transaction.
     * \param           src     Source of the edge.
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
     * \param           src     Source of the edge.
     * \param           tid     The Temporal id.
     * \param           dst     Destination of the edge.
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
        KvIterator it(txn, table_);
        std::unique_ptr<EdgeConstraintsChecker> ec;
        if (!constraints.empty()) {
            ec = std::make_unique<EdgeConstraintsChecker>(constraints);
        }
        EdgeId eid = OutIteratorImpl::InsertEdge(esid, prop, it, nullptr, ec.get());
        EdgeSid esid_reverse = esid;
        esid_reverse.Reverse();
        EdgeId ee = InIteratorImpl::InsertEdge(esid_reverse, prop, it, &eid, ec.get());
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
        KvIterator it(txn, table_);
        it.GotoClosestKey(KeyPacker::CreatePackedDataKey(vid));
        if (!it.IsValid() || KeyPacker::GetFirstVid(it.GetKey()) != vid)
            return false;  // no such vertex
        DeleteVertexInternal(txn, it, on_edge_deleted);
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
     * Delete edge from src to dst with edge_id==eid
     *
     * \param [in,out]  txn The transaction.
     * \param           src Source of the edge.
     * \param           dst Destination of the edge.
     * \param           eid Edge property.
     *
     * \return  True if the edge is deleted successfully. False if the edge does not exist. Raises
     *          exception if other errors occur.
     */
    bool DeleteEdge(KvTransaction& txn, const EdgeUid& uid) {
        KvIterator it(txn, table_);
        OutIteratorImpl oeit(it);
        oeit.Goto(uid, false);
        if (!oeit.IsValid()) return false;
        oeit.Delete();
        InIteratorImpl ieit(it);
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
        KvIterator it(txn, table_);
        InIteratorImpl ieit(it);
        ieit.Goto(euid, false);
        FMA_DBG_ASSERT(ieit.IsValid());
        ieit.Delete();
    }

    void DeleteEdge(KvTransaction& txn, InEdgeIterator& ieit) {
        EdgeUid euid(ieit.GetSrc(), ieit.GetDst(), ieit.GetLabelId(), ieit.GetTemporalId(),
                     ieit.GetEdgeId());
        ieit.Delete();
        // delete out-edge of source vertex
        KvIterator it(txn, table_);
        OutIteratorImpl oeit(it);
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
        KvIterator it(txn, table_);
        VIteratorImpl vit(it);
        vit.Goto(vid);
        if (!vit.IsValid()) return false;
        vit.SetProperty(prop);
        return true;
    }

    /**
     * Sets edge property
     *
     * \param [in,out]  txn         The transaction.
     * \param           src         Source for the.
     * \param           dst         Destination for the.
     * \param           eid         The old property.
     * \param           new_prop    The new property.
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool SetEdgeProperty(KvTransaction& txn, const EdgeUid& uid, const Value& new_prop) {
        KvIterator it(txn, table_);
        OutIteratorImpl oeit(it);
        oeit.Goto(uid, false);
        if (!oeit.IsValid()) return false;
        oeit.SetProperty(new_prop);
        InIteratorImpl ieit(it);
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
    void Drop(KvTransaction& txn) { table_.Drop(txn); }

    /** @breif Get next available vid. */
    VertexId GetAndIncNextVid(KvTransaction& txn) {
        static Value key = Value::ConstRef(lgraph::_detail::NEXT_VID_KEY);
        auto it = meta_table_->GetIterator(txn, key);
        if (!it.IsValid()) {
            meta_table_->AddKV(txn, key, Value::ConstRef((VertexId)1));
            return 0;
        } else {
            VertexId vid = it.GetValue().AsType<VertexId>();
            if (vid == lgraph::_detail::MAX_VID)
                throw InputError(FMA_FMT("Maximum number of vertex reached: ", vid));
            it.SetValue(Value::ConstRef(vid + 1));
            return vid;
        }
    }

    VertexId GetNextVid(KvTransaction& txn) {
        static Value key = Value::ConstRef(lgraph::_detail::NEXT_VID_KEY);
        auto it = meta_table_->GetIterator(txn, key);
        if (!it.IsValid()) return 0;
        return it.GetValue().AsType<VertexId>();
    }

    VertexId GetLooseNumVertex(KvTransaction& txn) { return GetNextVid(txn); }

#ifdef _USELESS_CODE
    // Adds a bunch of out-edges and in-edges for the same vertex.
    // Make sure out-edge and in-edges are inserted in the same order.
    // For example, (V->U, e1) and (V->U, e2) appears as {e1, e2} in
    // outs of V, and they must also be in the same order {e1, e2} in
    // the in-edges of U.
    void AddEdgesRaw(KvTransaction& txn, VertexId vid, LabelId lid,
                     const std::vector<VertexId>& dsts, const std::vector<Value>& props,
                     const std::vector<VertexId>& in_srcs, const std::vector<Value>& in_props) {
        KvIterator it(txn, table_);
        FMA_DBG_CHECK_EQ(dsts.size(), props.size());
        for (size_t i = 0; i < dsts.size(); i++) {
            VertexId dst = dsts[i];
            const Value& prop = props[i];
            OutIteratorImpl::InsertEdge(EdgeSid(vid, dst, lid, 0), prop, it);
        }
        FMA_DBG_CHECK_EQ(in_srcs.size(), in_props.size());
        for (size_t i = 0; i < in_srcs.size(); i++) {
            VertexId src = in_srcs[i];
            const Value& prop = in_props[i];
            InIteratorImpl::InsertEdge(EdgeSid(vid, src, lid, 0), prop, it);
        }
    }
    void ImportVertexDataRaw(KvTransaction& txn, const std::string& vdata, VertexId expected,
                             const std::vector<std::tuple<LabelId, VertexId, std::string>>& outs,
                             const std::vector<std::tuple<LabelId, VertexId, std::string>>& ins) {
        VertexId vid = AddVertex(txn, Value(vdata.data(), vdata.size()));
        FMA_DBG_ASSERT(vid == expected);
        KvIterator it(txn, table_);
        for (auto& e : outs) {
            LabelId lid = std::get<0>(e);
            VertexId dst = std::get<1>(e);
            const std::string& ep = std::get<2>(e);
            OutIteratorImpl::InsertEdge(vid, dst, Value(ep.data(), ep.size()), it, has_other_edge);
        }
        for (auto& in : inrefs) {
            int pos = 0;
            bool r = InIteratorImpl::AddInRef(in, vid, it, pos);
        }
    }
#endif

    void AppendDataRaw(KvTransaction& txn, const Value& k, const Value& v) {
        table_.AppendKv(txn, k, v);
    }

    // returns the KvTable, used by Transaction only
    KvTable& _GetKvTable() { return this->table_; }

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
    void DeleteCorrespondingInEdges(KvTransaction& txn, KvIterator& it, VertexId vid,
                                    const EdgeValue& oev) {
        KvIterator tmp_it(txn, table_);
        InIteratorImpl ieit(tmp_it);
        LabelId last_lid = -1;
        VertexId last_dst = -1;
        size_t ne = oev.GetEdgeCount();
        for (size_t i = 0; i < ne; i++) {
            auto e = oev.GetNthEdgeHeader(i);
            if (e.lid == last_lid && e.vid == last_dst) continue;
            last_lid = e.lid;
            last_dst = e.vid;
            if (e.vid == vid) continue;
            // delete all in-edges (vid->last_dst)
            ieit.Goto(EdgeUid(vid, last_dst, e.lid, e.tid, 0), true);
            while (ieit.IsValid() && ieit.GetLabelId() == last_lid && ieit.GetVid2() == vid)
                ieit.Delete();
        }
    }

    void DeleteCorrespondingOutEdges(KvTransaction& txn, KvIterator& tmp_it, VertexId vid,
                                     const EdgeValue& iev) {
        OutIteratorImpl oeit(tmp_it);
        LabelId last_lid = -1;
        VertexId last_src = -1;
        size_t ne = iev.GetEdgeCount();
        for (size_t i = 0; i < ne; i++) {
            auto e = iev.GetNthEdgeHeader(i);
            if (e.lid == last_lid && e.vid == last_src) continue;
            last_lid = e.lid;
            last_src = e.vid;
            if (e.vid == vid) continue;
            // delete all out-edges (last_src->vid)
            oeit.Goto(EdgeUid(last_src, vid, e.lid, e.tid, 0), true);
            while (oeit.IsValid() && oeit.GetLabelId() == last_lid && oeit.GetVid2() == vid)
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
            KvIterator tmp_it(txn, table_);
            const EdgeValue& oev = pdv.GetOutEdge();
            if (on_edge_deleted) {
                on_edge_deleted(true, oev);
            }
            DeleteCorrespondingInEdges(txn, tmp_it, vid, oev);
            // delete incoming edges
            const EdgeValue& iev = pdv.GetInEdge();
            if (on_edge_deleted) {
                on_edge_deleted(false, iev);
            }
            DeleteCorrespondingOutEdges(txn, tmp_it, vid, iev);
            // delete vertex
            it.RefreshAfterModify();
            it.DeleteKey();
        } else {
            // delete vertex-only node
            FMA_DBG_CHECK_EQ(KeyPacker::GetNodeType(it.GetKey()), PackType::VERTEX_ONLY);
            it.DeleteKey();
            // now it points to out-edge node
            KvIterator tmp_it(txn, table_);
            while (it.IsValid() && KeyPacker::GetNodeType(it.GetKey()) == PackType::OUT_EDGE) {
                FMA_DBG_CHECK_EQ(KeyPacker::GetFirstVid(it.GetKey()), vid);
                EdgeValue oev(Value::MakeCopy(it.GetValue()));
                if (on_edge_deleted) {
                    on_edge_deleted(true, oev);
                }
                DeleteCorrespondingInEdges(txn, tmp_it, vid, oev);
                it.RefreshAfterModify();
                it.DeleteKey();
            }
            // delete incoming edges
            while (it.IsValid() && KeyPacker::GetNodeType(it.GetKey()) == PackType::IN_EDGE) {
                EdgeValue iev(Value::MakeCopy(it.GetValue()));
                if (on_edge_deleted) {
                    on_edge_deleted(false, iev);
                }
                DeleteCorrespondingOutEdges(txn, tmp_it, vid, iev);
                it.RefreshAfterModify();
                it.DeleteKey();
            }
        }
        FMA_DBG_ASSERT(!it.IsValid() || KeyPacker::GetFirstVid(it.GetKey()) != vid);
    }
};
}  // namespace graph
}  // namespace lgraph
