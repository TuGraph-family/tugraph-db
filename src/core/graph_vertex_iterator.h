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

#include "core/graph_data_pack.h"
#include "core/graph_edge_iterator.h"
#include "core/iterator_base.h"
#include "core/kv_store.h"

int TestPerfGraphNoncontinuous(bool track_incoming, bool durable);

namespace lgraph {
class Transaction;
class LightningGraph;

namespace graph {
class VertexIterator;
class Graph;

/** A vertex iterator. */
class VertexIteratorImpl {
    friend class Graph;
    friend class VertexIterator;
    friend class Transaction;
    friend class LightningGraph;

    KvIterator* it_;
    VertexId id_ = 0;
    VertexValue vov_;
    bool valid_ = false;
    bool track_incoming_ = false;

    void RefreshIteratorAndContent() {
        // don't refresh if the iterator is already invalid
        if (!valid_) return;
        // now refresh iterator
        if (it_->RefreshAfterModify()) {
            // if it_ still points to the same vertex node, we don't need to do Goto
            Value key = it_->GetKey();
            VertexId vid = KeyPacker::GetFirstVid(key);
            PackType pt = KeyPacker::GetNodeType(key);
            if (vid == id_) {
                if (pt == PackType::VERTEX_ONLY) {
                    vov_ = VertexValue(it_->GetValue());
                    valid_ = true;
                    return;
                } else if (pt == PackType::PACKED_DATA) {
                    vov_ = PackedDataValue(it_->GetValue()).GetVertexData();
                    valid_ = true;
                    return;
                }
            }
        }
        // it_ is either invalid, or it does not point to the right position
        // In this case, we call Goto to position it correctly
        Goto(id_, true);
    }

    ENABLE_RVALUE(void) SetVertexProperty(VALUE&& prop) {
        PackType pt = KeyPacker::GetNodeType(it_->GetKey());
        const Value& oldv = it_->GetValue();
        if (pt == PackType::PACKED_DATA) {
            if (oldv.Size() - vov_.GetVertexProperty().Size() + prop.Size() >
                ::lgraph::_detail::NODE_SPLIT_THRESHOLD) {
                // split into three nodes: vertex-only, out-edges and in-edges
                PackedDataValue pack(Value::MakeCopy(it_->GetValue()));
                vov_.SetProperty(std::forward<VALUE>(prop));
                it_->DeleteKey();
                _detail::StoreEdgeNode(PackType::OUT_EDGE, *it_, id_, pack.GetOutEdge());
                _detail::StoreEdgeNode(PackType::IN_EDGE, *it_, id_, pack.GetInEdge());
                _detail::StoreVertexOnlyNode(*it_, id_, vov_);
                vov_ = VertexValue(it_->GetValue());
            } else {
                PackedDataValue pack(oldv);
                Value newv;
                PackedDataValue::PackData(VertexValue(std::forward<VALUE>(prop)), pack.GetOutEdge(),
                                          pack.GetInEdge(), newv);
                it_->SetValue(newv);
                vov_ = PackedDataValue(it_->GetValue()).GetVertexData();
            }
        } else if (pt == PackType::VERTEX_ONLY) {
            it_->SetValue(VertexValue(std::forward<VALUE>(prop)).GetBuf());
            vov_ = VertexValue(it_->GetValue());
        }
    }

    /** Refresh contents (id_, vov_, valid_) from correctly positioned iterator */
    void LoadContentFromIt() {
        // if we delete all vertices, the KvIterator would become invalid
        if (!it_->IsValid()) {
            valid_ = false;
            return;
        }
        const Value& key = it_->GetKey();
        id_ = KeyPacker::GetFirstVid(key);
        PackType pt = KeyPacker::GetNodeType(key);
        if (pt == PackType::VERTEX_ONLY) {
            vov_ = VertexValue(it_->GetValue());
        } else {
            FMA_DBG_ASSERT(pt == PackType::PACKED_DATA);
            vov_ = PackedDataValue(it_->GetValue()).GetVertexData();
        }
        valid_ = true;
    }

 public:
    typedef OutEdgeIterator OutIterator;
    typedef InEdgeIterator InIterator;

    explicit VertexIteratorImpl(KvIterator& it) : it_(&it) {}

    DISABLE_COPY(VertexIteratorImpl);

    VertexIteratorImpl(VertexIteratorImpl&& rhs)
        : it_(rhs.it_),
          id_(rhs.id_),
          vov_(std::move(rhs.vov_)),
          valid_(rhs.valid_),
          track_incoming_(rhs.track_incoming_) {
        rhs.valid_ = false;
    }

    VertexIteratorImpl& operator=(VertexIteratorImpl&& rhs) = delete;

    void Close() {
        it_->Close();
        valid_ = false;
    }

    /**
     * Move to the next vertex.
     *
     * \return  True if it succeeds, otherwise false (no more vertex).
     */
    bool Next() { return Goto(id_ + 1, true); }

    /**
     * Goto the vertex with id src
     *
     * \param   vid VertexId of the vertex to go.
     *
     * \return  True if it succeeds, otherwise false (no such vertex).
     */
    bool Goto(VertexId vid, bool closest = false) {
        valid_ = false;
        it_->GotoClosestKey(KeyPacker::CreatePackedDataKey(vid));
        if (!it_->IsValid()) return false;
        const Value& key = it_->GetKey();
        if (!closest && KeyPacker::GetFirstVid(key) != vid) return false;
        LoadContentFromIt();
        return valid_;
    }

    const VertexId& GetId() const { return id_; }

    /**
     * Gets the property of the vertex
     *
     * \return  The property.
     */
    const Value& GetProperty() const { return vov_.GetVertexProperty(); }

    bool IsValid() const { return valid_; }

    /**
     * Sets the property of current vertex.
     *
     * \note    Write operation on vertex iterator will invalidate contents of any edge iterator on
     *          that vertex.
     *
     * \param   prop    The property.
     */
    void SetProperty(const Value& prop) { SetVertexProperty(prop); }

    void SetProperty(Value&& prop) { SetVertexProperty(std::move(prop)); }

    std::vector<VertexId> ListDstVids(size_t& n_edges, bool& edge_left, LabelId& start_lid,
                                      VertexId& start_vid,
                                      size_t n_limit = std::numeric_limits<size_t>::max()) {
        return EdgeIteratorImpl<PackType::OUT_EDGE>::ListPeersWithVertexIt(
            *it_, id_, start_lid, start_vid, n_edges, edge_left, n_limit);
    }

    size_t GetNumOutEdges(size_t limit = std::numeric_limits<size_t>::max(),
                          bool* limit_exceeded = nullptr) {
        return EdgeIteratorImpl<PackType::OUT_EDGE>::GetNumEdgesWithVertexIt(*it_, id_, limit,
                                                                             limit_exceeded);
    }

    std::vector<VertexId> ListSrcVids(size_t& n_edges, bool& edge_left, LabelId& start_lid,
                                      VertexId& start_vid,
                                      size_t n_limit = std::numeric_limits<size_t>::max()) {
        return EdgeIteratorImpl<PackType::IN_EDGE>::ListPeersWithVertexIt(
            *it_, id_, start_lid, start_vid, n_edges, edge_left, n_limit);
    }

    size_t GetNumInEdges(size_t limit = std::numeric_limits<size_t>::max(),
                         bool* limit_exceeded = nullptr) {
        return EdgeIteratorImpl<PackType::IN_EDGE>::GetNumEdgesWithVertexIt(*it_, id_, limit,
                                                                            limit_exceeded);
    }

 private:
    /**
     * Deletes current vertex and all out-going edges. This function does *NOT* delete in-coming
     * edges.
     */
    void DeleteVertexAndOutEdges() {
        do {
            it_->DeleteKey();
        } while (it_->IsValid() && KeyPacker::GetFirstVid(it_->GetKey()) == id_);
        if (!it_->IsValid()) {
            valid_ = false;
            return;
        }
        LoadContentFromIt();
    }

    KvIterator& GetKvIterator() { return *it_; }

    void SetItPtr(KvIterator* it) { it_ = it; }
};

class VertexIterator : public ::lgraph::IteratorBase {
    friend class Graph;
    friend class ::lgraph::Transaction;
    friend class ::lgraph::LightningGraph;
    friend int ::TestPerfGraphNoncontinuous(bool track_incoming, bool durable);

    KvIterator it_;
    VertexIteratorImpl impl_;

    DISABLE_COPY(VertexIterator);
    VertexIterator& operator=(VertexIterator&& rhs) = delete;

 protected:
    void CloseImpl() override;

 public:
    VertexIterator(::lgraph::Transaction* txn, KvTable& tbl, VertexId vid, bool closest = false);
    VertexIterator(::lgraph::KvTransaction* txn, KvTable& tbl, VertexId vid, bool closest = false);

    VertexIterator(VertexIterator&& rhs);

    /**
     * Move to the next vertex.
     *
     * \return  True if it succeeds, otherwise false (no more vertex).
     */
    bool Next() { return impl_.Next(); }

    /**
     * Determines if we can refresh content if kv iterator modified
     *
     * @return  True if underlying KvIterator was modified.
     */
    void RefreshContentIfKvIteratorModified() override {
        if (IsValid() && it_.IsValid() && it_.UnderlyingPointerModified()) {
            impl_.RefreshIteratorAndContent();
        }
    }

    /**
     * Goto the vertex with id src
     *
     * \param   vid VertexId of the vertex to go.
     *
     * \return  True if it succeeds, otherwise false (no such vertex).
     */
    bool Goto(VertexId vid, bool nearest = false) { return impl_.Goto(vid, nearest); }

    bool GotoClosestVertex(VertexId vid) { return Goto(vid, true); }

    const VertexId& GetId() const { return impl_.GetId(); }

    /**
     * Gets the property of the vertex
     *
     * \return  The property.
     */
    const Value& GetProperty() const { return impl_.GetProperty(); }

    /**
     * Construct an out-edge iterator
     *
     * \param   dst (Optional) Destination for the edge. If set to default (0), the iterator points
     *              to the first out edge.
     * \param   eid (Optional) The id of the edge. If set to default (0), the iterator points to the
     *              first edge src->dst.
     *
     * \return  The out edge iterator.
     */
    OutEdgeIterator GetOutEdgeIterator(const EdgeUid& euid = EdgeUid(), bool closest = true) const {
        EdgeUid e = euid;
        e.src = GetId();
        if (GetTxn())
            return OutEdgeIterator(GetTxn(), it_.GetTable(), e, closest);
        else
            return OutEdgeIterator(&it_.GetTxn(), it_.GetTable(), e, closest);
    }

    /**
     * Construct an in-edge iterator
     *
     * \param   src             (Optional) Source for the edge.
     * \param   closest_edge    (Optional) If set to true and [src->dst] does not exist, start with
     *                          the edge right next to it.
     *
     * \return  The in edge iterator.
     */
    InEdgeIterator GetInEdgeIterator(const EdgeUid& euid = EdgeUid(), bool closest = true) const {
        EdgeUid e = euid;
        e.dst = GetId();
        if (GetTxn())
            return InEdgeIterator(GetTxn(), it_.GetTable(), e, closest);
        else
            return InEdgeIterator(&it_.GetTxn(), it_.GetTable(), e, closest);
    }

    bool IsValid() const { return impl_.IsValid(); }

    std::vector<VertexId> ListDstVids(LabelId* start_lid = nullptr, VertexId* start_vid = nullptr,
                                      size_t n_limit = std::numeric_limits<size_t>::max(),
                                      bool* edge_left = nullptr, size_t* n_edges = nullptr) {
        size_t ne = 0;
        bool el = false;
        LabelId lid = 0;
        VertexId vid = 0;
        return impl_.ListDstVids(n_edges ? *n_edges : ne, edge_left ? *edge_left : el,
                                 start_lid ? *start_lid : lid, start_vid ? *start_vid : vid,
                                 n_limit);
    }

    size_t GetNumOutEdges(size_t limit = std::numeric_limits<size_t>::max(),
                          bool* limit_exceeded = nullptr) {
        return impl_.GetNumOutEdges(limit, limit_exceeded);
    }

    std::vector<VertexId> ListSrcVids(LabelId* start_lid = nullptr, VertexId* start_vid = nullptr,
                                      size_t n_limit = std::numeric_limits<size_t>::max(),
                                      bool* edge_left = nullptr, size_t* n_edges = nullptr) {
        size_t ne = 0;
        bool el = false;
        LabelId lid = 0;
        VertexId vid = 0;
        return impl_.ListSrcVids(n_edges ? *n_edges : ne, edge_left ? *edge_left : el,
                                 start_lid ? *start_lid : lid, start_vid ? *start_vid : vid,
                                 n_limit);
    }

    size_t GetNumInEdges(size_t limit = std::numeric_limits<size_t>::max(),
                         bool* limit_exceeded = nullptr) {
        return impl_.GetNumInEdges(limit, limit_exceeded);
    }

#ifdef FMA_IN_UNIT_TEST
 public:  // NOLINT
#else
 private:  // NOLINT
#endif
    void LoadContentFromIt() { impl_.LoadContentFromIt(); }

    KvIterator& GetIt() { return it_; }

    /**
     * Sets the property of current vertex.
     *
     * \note    Write operation on vertex iterator will invalidate contents of any edge iterator on
     *          that vertex.
     *
     * \param   prop    The property.
     */
    void SetProperty(const Value& prop) { impl_.SetVertexProperty(prop); }

    void SetProperty(Value&& prop) { impl_.SetVertexProperty(std::move(prop)); }
};
}  // namespace graph
}  // namespace lgraph
