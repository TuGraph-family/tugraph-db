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
#include "core/iterator_base.h"
#include "core/kv_store.h"
#include "core/schema_manager.h"

int TestOutEdgeIterator(int, char**);
int TestInRefIterator(int, char**);
int TestPerfGraphNoncontinuous(bool track_incoming, bool durable);

namespace lgraph {
class Transaction;

namespace graph {
class OutEdgeIterator;
class InEdgeIterator;
class VertexIterator;
class VertexIteratorImpl;
class Graph;

namespace _detail {
inline void StorePackedNode(KvIterator& it, VertexId vid, const PackedDataValue& pdv) {
    bool r = it.AddKeyValue(pdv.CreateKey(vid), pdv.GetBuf());
    FMA_DBG_ASSERT(r);
}

inline void StoreVertexOnlyNode(KvIterator& it, VertexId vid, const VertexValue& vov) {
    bool r = it.AddKeyValue(KeyPacker::CreateVertexOnlyKey(vid), vov.GetBuf(), false);
    FMA_DBG_ASSERT(r);
}

inline void StoreEdgeNode(PackType pt, KvIterator& it, VertexId vid1, const EdgeValue& ev) {
    if (ev.GetEdgeCount() == 0) return;
    bool r = it.AddKeyValue(ev.CreateKey(pt, vid1), ev.GetBuf(), false);
    FMA_DBG_ASSERT(r);
}
}  // namespace _detail

template <PackType ET>
class EdgeIterator;

namespace _detail {
template <PackType PT>
inline EdgeValue GetEdgeValue(const PackedDataValue& pdv) {
    return EdgeValue();
}

template <>
inline EdgeValue GetEdgeValue<PackType::IN_EDGE>(const PackedDataValue& pdv) {
    return pdv.GetInEdge();
}

template <>
inline EdgeValue GetEdgeValue<PackType::OUT_EDGE>(const PackedDataValue& pdv) {
    return pdv.GetOutEdge();
}

template <PackType PT>
inline void ThrowVertexNotExist() {}

template <>
inline void ThrowVertexNotExist<PackType::IN_EDGE>() {
    throw InputError("Destination vertex does not exist");
}

template <>
inline void ThrowVertexNotExist<PackType::OUT_EDGE>() {
    throw InputError("Source vertex does not exist");
}
}  // namespace _detail

struct EdgeConstraintsChecker {
    explicit EdgeConstraintsChecker(
        const std::unordered_map<LabelId, std::unordered_set<LabelId>>& _constraints)
        : constraints(_constraints) {}
    const std::unordered_map<LabelId, std::unordered_set<LabelId>>& constraints;
    const std::unordered_set<LabelId>* dst_lids = nullptr;

    void Check(PackType pt, LabelId lid) {
        if (PackType::OUT_EDGE == pt) {
            auto iter = constraints.find(lid);
            if (iter == constraints.end()) {
                throw InputError("Does not meet the edge constraints");
            } else {
                dst_lids = &(iter->second);
            }
        } else if (PackType::IN_EDGE == pt) {
            if (!dst_lids->count(lid)) {
                throw InputError("Does not meet the edge constraints");
            }
        }
    }

    void Check(LabelId src_lid, LabelId dst_lid) {
        auto iter = constraints.find(src_lid);
        if (iter == constraints.end()) {
            throw InputError("Does not meet the edge constraints");
        }
        if (!iter->second.count(dst_lid)) {
            throw InputError("Does not meet the edge constraints");
        }
    }
};

template <PackType ET>  // ET is either IN_EDGE or OUT_EDGE
class EdgeIteratorImpl {
    friend class Graph;
    friend class OutEdgeIterator;
    friend class InEdgeIterator;
    friend class VertexIterator;
    friend class Transaction;
    friend class EdgeIterator<ET>;

    KvIterator* it_;   // iterator pointing to the data node
    EdgeValue ev_;     // the node containing multiple edges
    VertexId vid1_{};  // first vid, vid1 for out-edge, vid2 for in-edge
    LabelId lid_{};    // label id
    TemporalId tid_{};
    VertexId vid2_{};     // second vid, vid2 for out-edge, vid1 for in-edge
    EdgeId eid_{};        // edge id
    const char* prop_{};  // property
    size_t psize_{};      // property size

    size_t pos_{};  // position in the edge node
    bool valid_{};  // is this iterator valid?

    static void CheckPropSize(const Value& prop) {
        if (prop.Size() > ::lgraph::_detail::MAX_PROP_SIZE)
            throw InputError("Edge property size is too big.");
    }

    static void UpdatePackedNode(KvIterator* it, EdgeValue& ev) {
        PackedDataValue pdv(it->GetValue());
        const VertexValue& vov = pdv.GetVertexData();
        Value newv;
        if (ET == PackType::OUT_EDGE) {
            PackedDataValue::PackData(vov, ev, pdv.GetInEdge(), newv);
        } else {
            PackedDataValue::PackData(vov, pdv.GetOutEdge(), ev, newv);
        }
        it->SetValue(newv);
    }

    /** Refresh content (that is, ev_, pos_ and valid_), assuming it_,
     *  first_vid_, lid_, tid_, second_vid_, and eid_ are set properly.
     *  it_ must be positioned at the right PackedDataNode or EdgeNode.
     *
     *  If closest==true, get the first edge that sorts after (lid_, tid_, vid2_, eid_)
     */
    void LoadContentFromIt(bool closest) {
        valid_ = false;
        PackType pt = KeyPacker::GetNodeType(it_->GetKey());
        if (pt == PackType::PACKED_DATA) {
            ev_ = _detail::GetEdgeValue<ET>(PackedDataValue(it_->GetValue()));
        } else {
            FMA_DBG_CHECK_EQ(pt, ET);
            ev_ = EdgeValue(it_->GetValue());
        }
        // no edge, this can only happend in PackedDataNode with no edge
        if (ev_.GetEdgeCount() == 0) {
            FMA_DBG_CHECK_EQ(pt, PackType::PACKED_DATA);
            return;
        }
        // get first edge
        if (lid_ == 0 && tid_ == 0 && vid2_ == 0 && eid_ == 0) {
            pos_ = 0;
            auto edge = ev_.GetNthEdgeData(pos_);
            if (closest || (edge.lid == 0 && edge.tid == 0 && edge.vid == 0 && edge.eid == 0)) {
                if (closest) {
                    lid_ = edge.lid;
                    tid_ = edge.tid;
                    vid2_ = edge.vid;
                    eid_ = edge.eid;
                }
                valid_ = true;
                prop_ = edge.prop;
                psize_ = edge.psize;
            }
            return;
        }
        // search within an out-edge node
        pos_ = ev_.SearchEdge(lid_, tid_, vid2_, eid_, valid_);
        // found the edge
        if (valid_) {
            // get property
            ev_.ParseNthEdge(pos_, lid_, tid_, vid2_, eid_, prop_, psize_);
            return;
        }
        if (pos_ >= ev_.GetEdgeCount()) {
            // search passed the end, no such edge, and no edge after this one
            return;
        } else {
            // may have (lid, tid, vid, eid) bigger than this one
            auto edge = ev_.GetNthEdgeData(pos_);
            if (closest) {
                valid_ = true;
                lid_ = edge.lid;
                tid_ = edge.tid;
                vid2_ = edge.vid;
                eid_ = edge.eid;
                prop_ = edge.prop;
                psize_ = edge.psize;
                return;
            }
        }
    }
    DISABLE_COPY(EdgeIteratorImpl);

 public:
    explicit EdgeIteratorImpl(KvIterator& it) : it_(&it) {}

    EdgeIteratorImpl& operator=(EdgeIteratorImpl&& rhs) = delete;

    EdgeIteratorImpl(EdgeIteratorImpl&& rhs)
        : it_(rhs.it_),
          ev_(std::move(rhs.ev_)),
          vid1_(rhs.vid1_),
          tid_(rhs.tid_),
          lid_(rhs.lid_),
          vid2_(rhs.vid2_),
          eid_(rhs.eid_),
          prop_(rhs.prop_),
          psize_(rhs.psize_),
          pos_(rhs.pos_),
          valid_(rhs.valid_) {
        // since rhs.ev_ is moved, it becomes invalid
        rhs.valid_ = false;
        rhs.it_ = nullptr;
    }

    void RefreshIteratorAndContent() {
        if (!valid_) return;
        Goto(vid1_, lid_, tid_, vid2_, eid_, true);
    }

    void SetItPtr(KvIterator* it) {
        // used by the move constructor of EdgeIterator
        // Since the iterator is held as object in EdgeIterator, when
        // moving an EdgeIterator, the KvIt* must also change.
        // However, since the content of the KvIt is the same as the
        // moved object, we do not need to refresh contents like vid1_,
        // vid2_, etc.
        it_ = it;
    }

    bool Goto(VertexId vid1, LabelId lid, TemporalId tid, VertexId vid2, EdgeId eid, bool closest) {
        valid_ = false;
        vid1_ = vid1;
        lid_ = lid;
        tid_ = tid;
        vid2_ = vid2;
        eid_ = eid;
        bool r = it_->GotoClosestKey(KeyPacker::CreatePackedDataKey(vid1_));
        if (!r) return false;  // no such vertex
        const Value& k = it_->GetKey();
        if (KeyPacker::GetFirstVid(k) != vid1_) return false;  // no such vertex
        // now we are sure the vertex exist, check whether it is a packed vertex
        PackType pt = KeyPacker::GetNodeType(k);
        if (pt == PackType::VERTEX_ONLY) {
            if (lid_ == 0 && tid_ == 0 && vid2_ == 0 && eid_ == 0 &&
                ET == PackType::OUT_EDGE) {  // get the first one
                r = it_->Next();
            } else {
                r = it_->GotoClosestKey(
                    KeyPacker::CreateEdgeKey(ET, EdgeUid(vid1_, vid2_, lid_, tid_, eid_)));
            }
            if (!r) return false;  // searched till end of db, no such edge
            pt = KeyPacker::GetNodeType(it_->GetKey());
            if (pt != ET) return false;
        }
        LoadContentFromIt(closest);
        return valid_;
    }

 public:
    void Close() {
        valid_ = false;
        it_->Close();
    }

    bool Goto(const EdgeUid& euid, bool closest) {
        return ET == PackType::OUT_EDGE
                   ? Goto(euid.src, euid.lid, euid.tid, euid.dst, euid.eid, closest)
                   : Goto(euid.dst, euid.lid, euid.tid, euid.src, euid.eid, closest);
    }

    /**
     * Move to the next edge
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool Next() {
        if (!valid_) return false;
        if (pos_ < ev_.GetEdgeCount() - 1) {
            pos_++;
            ev_.ParseNthEdge(pos_, lid_, tid_, vid2_, eid_, prop_, psize_);
            return true;
        }
        // reached the end of current block
        valid_ = false;
        bool r = it_->Next();
        if (!r) return false;  // no more data
        const Value& k = it_->GetKey();
        if (KeyPacker::GetNodeType(k) != ET) return false;  // reached another type
        ev_ = EdgeValue(it_->GetValue());
        FMA_DBG_ASSERT(ev_.GetEdgeCount() > 0);
        valid_ = true;
        pos_ = 0;
        ev_.ParseNthEdge(pos_, lid_, tid_, vid2_, eid_, prop_, psize_);
        return true;
    }

    bool Prev() {
        valid_ = TryPrev();
        return valid_;
    }

    // get first vid, i.e., vid1 for out-edge, vid2 for in-edge
    VertexId GetVid1() const { return vid1_; }

    EdgeUid GetUid() const {
        return ET == PackType::OUT_EDGE ? EdgeUid(vid1_, vid2_, lid_, tid_, eid_)
                                        : EdgeUid(vid2_, vid1_, lid_, tid_, eid_);
    }

    // get label id of the edge
    LabelId GetLabelId() const { return lid_; }

    // get Temporal id of the edge
    TemporalId GetTemporalId() const { return tid_; }

    // get second vid, i.e. vid2 for out-edge, vid1 for in-edge
    VertexId GetVid2() const { return vid2_; }

    // get edge id
    EdgeId GetEdgeId() const { return eid_; }

    /**
     * Gets the property of the current edge
     *
     * \return  The edge property.
     */
    Value GetProperty() const { return Value(prop_, psize_); }

    /**
     * Query if this iterator is valid, i.e. the Id and Property can be queried.
     *
     * \return  True if valid, false if not.
     */
    bool IsValid() const { return valid_; }

    /**
     * Sets the property of the current edge.
     *
     * \param   prop    The new property.
     */
    void SetProperty(const Value& prop) {
        CheckPropSize(prop);
        UpdateEdge(prop);
    }

    /**
     * Insert multiple edges with the same vid1. Type InputIt must point to a type
     * like std::tuple<LabelId, VertexId, Value>.
     */
    template <class InputIt>
    static void InsertEdges(
        VertexId vid1, InputIt begin, InputIt end, KvIterator& it,
        std::function<void(int64_t, int64_t, uint16_t, int64_t, int64_t, const Value&)>
            add_fulltext_index) {
        // @TODO: optimize
        for (InputIt i = begin; i != end; ++i) {
            auto eid =
                InsertEdge(EdgeSid(vid1, std::get<1>(*i), std::get<0>(*i), 0),
                           std::get<2>(*i), it);
            if (ET == PackType::OUT_EDGE && add_fulltext_index) {
                add_fulltext_index(vid1, std::get<1>(*i), std::get<0>(*i), 0, eid, std::get<2>(*i));
            }
        }
    }
    /**
     * Insert a new edge. When eid is not null, no need to get eid from
     * previous edge.
     */
    static EdgeId InsertEdge(EdgeSid esid, const Value& prop, KvIterator& it,
                             EdgeId* eid_ptr = nullptr,
                             EdgeConstraintsChecker* ec = nullptr) {
        CheckPropSize(prop);
        it.GotoClosestKey(KeyPacker::CreatePackedDataKey(esid.src));
        if (!it.IsValid()) _detail::ThrowVertexNotExist<ET>();
        const Value& k = it.GetKey();
        if (KeyPacker::GetFirstVid(k) != esid.src) _detail::ThrowVertexNotExist<ET>();
        // ok, source exists
        PackType pt = KeyPacker::GetNodeType(k);
        EdgeValue ev;
        if (pt == PackType::PACKED_DATA) {
            ev = _detail::GetEdgeValue<ET>(PackedDataValue(it.GetValue()));
            if (ec) {
                auto lid = SchemaManager::GetRecordLabelId(
                    PackedDataValue(it.GetValue()).GetVertexData().GetVertexProperty());
                ec->Check(ET, lid);
            }
        } else {
            FMA_DBG_CHECK_EQ(pt, PackType::VERTEX_ONLY);
            if (ec) {
                auto lid =
                    SchemaManager::GetRecordLabelId(VertexValue(it.GetValue()).GetVertexProperty());
                ec->Check(ET, lid);
            }
            if (eid_ptr == nullptr) {
                // When inserting a new edge, we first insert out edge
                // then in edge. The EdgeId is obtained when inserting
                // out edge. So we need to get the edge right before
                // the inserted one to decide the right EdgeId.
                //
                // Another case is when we use "InsertEdgeRaw" and insert
                // a bunch of in-edges without first inserting the out edges.
                // In this case, eid is also unknown.

                // find the edge node
                it.GotoClosestKey(KeyPacker::CreateEdgeKey(
                    ET, EdgeUid(esid.src, esid.dst + 1, esid.lid, esid.tid, 0)));
                if (!it.IsValid() || KeyPacker::GetNodeType(it.GetKey()) != ET) {
                    // no edge node here, get previous node
                    it.Prev();
                    // it is either at the right position or we need a new node
                    if (KeyPacker::GetNodeType(it.GetKey()) == ET) {
                        // right position
                        ev = EdgeValue(it.GetValue());
                    }
                    // need a new edge node after it
                } else {
                    // got an edge node
                    ev = EdgeValue(it.GetValue());
                    FMA_DBG_ASSERT(ev.GetEdgeCount() > 0);
                    auto header = ev.GetNthEdgeHeader(0);
                    if (header.lid > esid.lid ||
                        (header.lid == esid.lid && header.tid > esid.tid) ||
                        (header.lid == esid.lid && header.tid == esid.tid &&
                         header.vid > esid.dst)) {
                        // this edge might belong to the previous node
                        it.Prev();
                        if (KeyPacker::GetNodeType(it.GetKey()) != ET) {
                            // no edge node before this one
                            it.Next();
                        } else {
                            // ok, got the right position
                            ev = EdgeValue(it.GetValue());
                        }
                    }
                }
            } else {
                // eid not not null, so we just go to the EdgeNode where
                // this edge should belong
                it.GotoClosestKey(KeyPacker::CreateEdgeKey(ET, esid.ConstructEdgeUid(*eid_ptr)));
                if (!it.IsValid() || KeyPacker::GetNodeType(it.GetKey()) != ET) {
                    it.Prev();
                    if (KeyPacker::GetNodeType(it.GetKey()) == ET) {
                        // right position
                        ev = EdgeValue(it.GetValue());
                    }
                } else {
                    // got the right edge node
                    ev = EdgeValue(it.GetValue());
                }
            }
        }
        // now it is at the right position, and ev is either a new node
        // or the right EdgeValue for it
        bool found;
        size_t pos = ev.SearchEdge(esid.lid, esid.tid, esid.dst + 1, 0, found);
        EdgeId eid;
        if (eid_ptr) {
            eid = *eid_ptr;
        } else {
            // need to decide the right eid
            if (pos == 0) {
                // edge goes at the front
                eid = 0;
            } else {
                auto header = ev.GetNthEdgeHeader(pos - 1);
                if (header.lid == esid.lid && header.tid == esid.tid && header.vid == esid.dst) {
                    // already have (label, tid, vid, eid)
                    eid = header.eid + 1;
                } else {
                    eid = 0;
                }
            }
            if (eid >= ::lgraph::_detail::MAX_EID)
                throw std::runtime_error("Too many edges from src to dst with the same label");
        }
        int64_t size_diff = ev.InsertAtPos(pos, esid.lid, esid.tid, esid.dst, eid, prop);
        const Value& v = it.GetValue();
        if (pt == PackType::PACKED_DATA) {
            if (it.GetValue().Size() + size_diff < ::lgraph::_detail::NODE_SPLIT_THRESHOLD) {
                // pack back and store
                UpdatePackedNode(&it, ev);
            } else {
                // packed data, but too large
                SplitPackedDataNode(esid.src, ev, pos, it);
            }
        } else {
            if (ev.GetEdgeCount() == 1) {
                // newly created edge node
                bool r = it.AddKeyValue(ev.CreateKey(ET, esid.src), ev.GetBuf(), false);
                FMA_DBG_ASSERT(r);
            } else {
                it.DeleteKey();
                SplitAndStoreEdgeNode(esid.src, ev, pos, it);
            }
        }
        return eid;
    }

    // List peers of current vertex.
    // If ET==IN_EDGE, then this returns all V in which (V->current) \belong E
    // if ET==OUT_EDGE, then this returns all V in which (current->V) \belong E
    //
    // The function returns list of VertexIds, and set start_lid and start_vid
    // which identifies the next edge.
    //
    // it must be pointing to the vertex node
    static std::vector<VertexId> ListPeersWithVertexIt(
        KvIterator& it, VertexId vid1, LabelId& start_lid, VertexId& start_vid, size_t& n_edges,
        bool& edge_left, size_t n_limit = std::numeric_limits<size_t>::max()) {
        bool start_from_first = (start_lid == 0 && start_vid == 0);
        n_edges = 0;
        edge_left = false;
        std::unordered_set<VertexId> vids;
        PackType t = KeyPacker::GetNodeType(it.GetKey());
        if (t == PackType::PACKED_DATA) {
            const EdgeValue& ev = _detail::GetEdgeValue<ET>(PackedDataValue(it.GetValue()));
            GetPeersFromEdgeValue(ev, start_from_first, start_lid, start_vid, n_edges, edge_left,
                                  vids, n_limit);
        } else {
            KvIterator tmp(it);
            if (ET == PackType::OUT_EDGE && start_from_first)
                tmp.Next();
            else
                tmp.GotoClosestKey(
                    KeyPacker::CreateEdgeKey(ET, EdgeUid(vid1, start_vid, start_lid, 0, 0)));
            bool set_pos_to_zero = start_from_first;
            while (tmp.IsValid()) {
                const Value& k = tmp.GetKey();
                if (KeyPacker::GetFirstVid(k) != vid1 || KeyPacker::GetNodeType(k) != ET) break;
                const EdgeValue& ev(EdgeValue(tmp.GetValue()));
                GetPeersFromEdgeValue(ev, set_pos_to_zero, start_lid, start_vid, n_edges, edge_left,
                                      vids, n_limit);
                set_pos_to_zero = true;
                tmp.Next();
            }
        }
        std::vector<VertexId> ret(vids.begin(), vids.end());
        std::sort(ret.begin(), ret.end());
        return ret;
    }

    static size_t GetNumEdgesWithVertexIt(KvIterator& it, VertexId vid1,
                                          size_t limit = std::numeric_limits<size_t>::max(),
                                          bool* limit_exceeded = nullptr) {
        if (limit_exceeded) *limit_exceeded = false;
        PackType t = KeyPacker::GetNodeType(it.GetKey());
        if (t == PackType::PACKED_DATA) {
            const EdgeValue& oev = _detail::GetEdgeValue<ET>(PackedDataValue(it.GetValue()));
            size_t n = (size_t)oev.GetEdgeCount();
            if (n > limit) {
                if (limit_exceeded) *limit_exceeded = true;
                return limit;
            }
            return n;
        } else {
            size_t n = 0;
            KvIterator tmp(it);
            tmp.GotoClosestKey(KeyPacker::CreateEdgeKey(ET, EdgeUid(vid1, 0, 0, 0, 0)));
            while (tmp.IsValid()) {
                const Value& k = tmp.GetKey();
                if (KeyPacker::GetFirstVid(k) != vid1 || KeyPacker::GetNodeType(k) != ET) break;
                const EdgeValue oev(tmp.GetValue());
                n += (size_t)oev.GetEdgeCount();
                if (n > limit) {
                    if (limit_exceeded) *limit_exceeded = true;
                    return limit;
                }
                tmp.Next();
            }
            return n;
        }
    }

    bool TryPrev() {
        if (valid_ && pos_ > 0) {
            pos_--;
            ev_.ParseNthEdge(pos_, lid_, tid_, vid2_, eid_, prop_, psize_);
            return true;
        }
        if (!valid_) {
            // we might have passsed the end of all edges
            it_->Prev();
            if (!it_->IsValid()) return false;
            const Value& k = it_->GetKey();
            PackType pt = KeyPacker::GetNodeType(k);
            if (pt != ET && pt != PackType::PACKED_DATA) return false;
            if (pt == PackType::PACKED_DATA) {
                ev_ = _detail::GetEdgeValue<ET>(PackedDataValue(it_->GetValue()));
            } else {
                FMA_DBG_ASSERT(pt == ET);
                ev_ = EdgeValue(it_->GetValue());
            }
        } else {
            // valid && pos_ == 0
            // reached the front of current out-edge block/packed block
            // so the previous one must be another type or another vertex
            FMA_DBG_ASSERT(valid_ && pos_ == 0);
            valid_ = false;
            it_->Prev();
            if (!it_->IsValid()) {
                // no previous node/vertex, leave the cursor unchanged, and iterator valid
                it_->Next();
                valid_ = true;
                return false;
            }
            const Value& k = it_->GetKey();
            if (KeyPacker::GetFirstVid(k) != vid1_ || KeyPacker::GetNodeType(k) != ET) {
                // leave the cursor unchanged and iterator valid
                it_->Next();
                valid_ = true;
                return false;
            }
            // reached previous edge node
            ev_ = EdgeValue(it_->GetValue());
        }
        // we have moved to another node
        size_t n = ev_.GetEdgeCount();
        FMA_DBG_CHECK_GT(n, 0);
        pos_ = n - 1;
        valid_ = true;
        ev_.ParseNthEdge(pos_, lid_, tid_, vid2_, eid_, prop_, psize_);
        return true;
    }

    KvIterator& GetIt() { return *it_; }

 protected:
    /** Deletes current edge. */
    void Delete() {
        PackType nt = KeyPacker::GetNodeType(it_->GetKey());
        if (nt == PackType::PACKED_DATA) {
            ev_.DeleteNthEdge(pos_);
            if (pos_ >= ev_.GetEdgeCount()) {
                pos_ = 0;
                valid_ = false;
            } else {
                ev_.ParseNthEdge(pos_, lid_, tid_, vid2_, eid_, prop_, psize_);
            }
            UpdatePackedNode(it_, ev_);
        } else {
            FMA_DBG_ASSERT(nt == ET);
            size_t ne = ev_.GetEdgeCount();
            bool key_changed = false;
            ev_.DeleteNthEdge(pos_);
            if (ne == 1) {
                // no edge left in this node, go to next one
                it_->DeleteKey();
                key_changed = true;
                // there might be another edge node after this one
                // in that case, we point to the first edge in that node
                pos_ = 0;
            } else if (pos_ < ne - 1) {
                // deleted at the middle
                // pos_ stays the same, but need to reload the edge
                it_->SetValue(ev_.GetBuf());
            } else {
                FMA_DBG_CHECK_EQ(pos_, ne - 1);
                // deleted at the end, key will change
                it_->DeleteKey();
                bool r = it_->AddKeyValue(ev_.CreateKey(ET, vid1_), ev_.GetBuf());
                FMA_DBG_ASSERT(r);
                // go to next node and point to its first edge if possible
                it_->Next();
                key_changed = true;
                pos_ = 0;
            }
            if (key_changed && (!it_->IsValid() || KeyPacker::GetNodeType(it_->GetKey()) != ET)) {
                // no edge after the deleted one
                valid_ = false;
                return;
            }
            // reload edge, this will release the buffer in ev, which
            // was allocated during the delete
            ev_ = EdgeValue(it_->GetValue());
            valid_ = true;
            ev_.ParseNthEdge(pos_, lid_, tid_, vid2_, eid_, prop_, psize_);
        }
    }

 private:
    static void GetPeersFromEdgeValue(const EdgeValue& ev, bool start_from_first,
                                      LabelId& start_lid, VertexId& start_vid, size_t& n_edges,
                                      bool& edge_left, std::unordered_set<VertexId>& vids,
                                      size_t n_limit) {
        bool found;
        size_t start = start_from_first ? 0 : ev.SearchEdge(start_lid, 0, start_vid, 0, found);
        LabelId last_lid = -1;
        VertexId last_vid = -1;
        size_t n = ev.GetEdgeCount();
        for (size_t i = start; i < n; i++) {
            n_edges++;
            auto header = ev.GetNthEdgeHeader(i);
            if (header.lid == last_lid && header.vid == last_vid) continue;
            last_lid = header.lid;
            last_vid = header.vid;
            if (vids.size() >= n_limit) {
                edge_left = true;
                break;
            }
            vids.insert(header.vid);
        }
        start_lid = last_lid;
        start_vid = last_vid;
    }

    /**
     * Splits and store an edge node. Content in original iterator will
     * not be touched. So you need to delete the old data if necessary.
     * After the operation, iterator will point to the EdgeNode where
     * the original edge at position pos is, and pos will be adjusted
     * accordingly.
     *
     * @param          vid1  Source for the.
     * @param [in,out] ev   The EdgeValue.
     * @param [in,out] pos  The position.
     * @param [in,out] it   The iterator.
     */
    static void SplitAndStoreEdgeNode(VertexId vid1, EdgeValue& ev, size_t& pos, KvIterator& it) {
        FMA_DBG_CHECK_GE(ev.GetEdgeCount(), 1);
        if (ev.GetBuf().Size() < ::lgraph::_detail::NODE_SPLIT_THRESHOLD ||
            ev.GetEdgeCount() == 1) {
            // does not need split, or cannot be split
            _detail::StoreEdgeNode(ET, it, vid1, ev);
        } else {
            // SplitEven will guarantee lhs is either smaller than
            // NODE_SPLIT_THRESHOLD, or it contains only one edge.
            // So lhs can always be directly stored, while rhs might
            // need further split
            EdgeValue lhs = ev.SplitEven();
            size_t n_lhs = lhs.GetEdgeCount();
            if (pos >= n_lhs) {
                // edge is at the rhs/oev
                _detail::StoreEdgeNode(ET, it, vid1, lhs);
                pos -= n_lhs;
                SplitAndStoreEdgeNode(vid1, ev, pos, it);
            } else {
                // edge is at lhs
                size_t p = 0;
                SplitAndStoreEdgeNode(vid1, ev, p, it);
                ev = std::move(lhs);
                _detail::StoreEdgeNode(ET, it, vid1, ev);
            }
        }
    }

    /**
     * Splits packed data node. Assuming it points to the packed data node.
     * After the operation, iterator will point to the OutEdgeNode where
     * the original edge at position pos is, and pos will be adjusted
     * accordingly.
     *
     * @param          vid1  Source for the.
     * @param          pdv  The pdv.
     * @param          oev  The oev.
     * @param [in,out] pos  The position.
     * @param [in,out] it   The iterator.
     */
    static void SplitPackedDataNode(VertexId src, EdgeValue& ev, size_t& pos, KvIterator& it) {
        PackedDataValue pdv(Value::MakeCopy(it.GetValue()));
        it.DeleteKey();
        _detail::StoreVertexOnlyNode(it, src, pdv.GetVertexData());
        if (ET == PackType::OUT_EDGE) {
            _detail::StoreEdgeNode(PackType::IN_EDGE, it, src, pdv.GetInEdge());
        } else {
            _detail::StoreEdgeNode(PackType::OUT_EDGE, it, src, pdv.GetOutEdge());
        }
        SplitAndStoreEdgeNode(src, ev, pos, it);  // it will point to the OutEdgeNode
    }

    void UpdateEdge(const Value& prop) {
        FMA_DBG_ASSERT(valid_);
        CheckPropSize(prop);
        Value v = it_->GetValue();
        size_t old_prop_size = psize_;
        bool need_split =
            prop.Size() > old_prop_size &&
            v.Size() + prop.Size() - old_prop_size > ::lgraph::_detail::NODE_SPLIT_THRESHOLD;
        PackType nt = KeyPacker::GetNodeType(it_->GetKey());
        ev_.CopyBuffer();
        ev_.UpdateNthEdge(pos_, prop);
        if (nt == PackType::PACKED_DATA) {
            if (need_split) {
                SplitPackedDataNode(vid1_, ev_, pos_, *it_);
            } else {
                // still the same PackedDataNode
                UpdatePackedNode(it_, ev_);
            }
        } else {
            FMA_DBG_CHECK_EQ(nt, ET);
            if (need_split) {
                it_->DeleteKey();
                SplitAndStoreEdgeNode(vid1_, ev_, pos_, *it_);
            } else {
                // still the same EdgeNode, with the same key
                it_->SetValue(ev_.GetBuf());
            }
        }
        // update prop and psize
        ev_.ParseNthEdge(pos_, lid_, tid_, vid2_, eid_, prop_, psize_);
    }
};

template <PackType ET>
class EdgeIterator : public ::lgraph::IteratorBase {
    friend class ::lgraph::Transaction;
    friend class Graph;
    friend class VertexIterator;
    friend class VertexIteratorImpl;
    friend class OutEdgeIterator;
    friend class InEdgeIterator;
    friend int ::TestOutEdgeIterator(int, char**);
    friend int ::TestPerfGraphNoncontinuous(bool track_incoming, bool durable);

 protected:
    KvIterator it_;
    EdgeIteratorImpl<ET> impl_;

    /**
     * Get an edge iterator for euid.
     *
     * @param [in,out]  txn             If non-null, the transaction.
     * @param [in,out]  table           The table.
     * @param           euid            Edge uid.
     * @param           closest         If goto closest position.
     */
    EdgeIterator(Transaction* txn, KvTable& table, const EdgeUid& euid, bool closest);

    /**
     * Constructs an un-managed iterator, which does not register to a transaction.
     *
     * @param [in,out]  txn             If non-null, the transaction.
     * @param [in,out]  table           The table.
     * @param           euid            Edge uid.
     * @param           closest         If goto closest position.
     */
    EdgeIterator(KvTransaction* txn, KvTable& table, const EdgeUid& euid, bool closest)
        : IteratorBase(nullptr), it_(*txn, table), impl_(it_) {
        impl_.Goto(euid, closest);
    }

    KvIterator& GetIt() { return it_; }

    DISABLE_COPY(EdgeIterator);
    EdgeIterator& operator=(EdgeIterator&& rhs) = delete;

    void CloseImpl() override { impl_.Close(); }

 public:
    EdgeIterator(EdgeIterator&& rhs)
        : IteratorBase(std::move(rhs)), it_(std::move(rhs.it_)), impl_(std::move(rhs.impl_)) {
        // it_ is moved, but impl_ keeps the pointer of rhs.it_, so we
        // need to reset the pointer here
        impl_.SetItPtr(&it_);
    }

    /**
     * Go to the edge specified by vid1->vid2 with eid. If closest_dst is true, then
     * go to the edge with the destination no less than and closest to vid2. If
     * closest_edge is true, then go to the edge with the edge id no less than and
     * closest to eid.
     *
     * \param   vid1            src_id for out-edge, dst_id for in_edge
     * \param   lid             LabelId of the edge
     * \param   vid2            dst_id for out-edge, src_id for in_edge
     * \param   eid             The eid.
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool Goto(const EdgeUid& euid, bool closest) { return impl_.Goto(euid, closest); }

    /**
     * Move to the next edge
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool Next() { return impl_.Next(); }

    /**
     * Move to the previous edge
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool Prev() { return impl_.Prev(); }

    EdgeUid GetUid() const { return impl_.GetUid(); }

    /**
     * Gets the LabelId of current edge
     *
     * \return  The LabelId of current edge.
     */
    LabelId GetLabelId() const { return impl_.GetLabelId(); }

    /**
     * Gets the TemporalId of current edge
     *
     * \return  The TemporalId of current edge.
     */
    TemporalId GetTemporalId() const { return impl_.GetTemporalId(); }

    /**
     * Gets edge identifier
     *
     * \return  The edge identifier.
     */
    EdgeId GetEdgeId() const { return impl_.GetEdgeId(); }

    /**
     * Gets the property of the current edge
     *
     * \return  The edge property.
     */
    Value GetProperty() const { return impl_.GetProperty(); }

    /**
     * Query if this iterator is valid, i.e. the Id and Property can be queried.
     *
     * \return  True if valid, false if not.
     */
    bool IsValid() const { return impl_.IsValid(); }

    void RefreshContentIfKvIteratorModified() override {
        if (IsValid() && it_.IsValid() && it_.UnderlyingPointerModified()) {
            impl_.RefreshIteratorAndContent();
        }
    }

    /** Deletes current edge. */
    void Delete() { impl_.Delete(); }

    /**
     * Sets the property of the current edge.
     *
     * \param   prop    The new property.
     */
    void SetProperty(const Value& prop) { impl_.SetProperty(prop); }

    /**
     * Attempts to get previous OutEdge. If fails, iterator remains unchanged.
     *
     * @return  True if it succeeds, false if it fails.
     */
    bool TryPrev() { return impl_.TryPrev(); }
};

class OutEdgeIterator : public EdgeIterator<PackType::OUT_EDGE> {
    friend class ::lgraph::Transaction;
    friend class Graph;
    friend class VertexIterator;
    friend class VertexIteratorImpl;

 public:
    OutEdgeIterator(::lgraph::Transaction* txn, KvTable& table, const EdgeUid& euid, bool closest)
        : EdgeIterator<PackType::OUT_EDGE>(txn, table, euid, closest) {}

    OutEdgeIterator(KvTransaction* txn, KvTable& table, const EdgeUid& euid, bool closest)
        : EdgeIterator<PackType::OUT_EDGE>(txn, table, euid, closest) {}

    VertexId GetSrc() const { return impl_.GetVid1(); }

    VertexId GetDst() const { return impl_.GetVid2(); }
};

class InEdgeIterator : public EdgeIterator<PackType::IN_EDGE> {
    friend class ::lgraph::Transaction;
    friend class Graph;
    friend class VertexIterator;
    friend class VertexIteratorImpl;

 public:
    InEdgeIterator(::lgraph::Transaction* txn, KvTable& table, const EdgeUid& euid, bool closest)
        : EdgeIterator<PackType::IN_EDGE>(txn, table, euid, closest) {}

    InEdgeIterator(KvTransaction* txn, KvTable& table, const EdgeUid& euid, bool closest)
        : EdgeIterator<PackType::IN_EDGE>(txn, table, euid, closest) {}

    VertexId GetSrc() const { return impl_.GetVid2(); }

    VertexId GetDst() const { return impl_.GetVid1(); }
};
}  // namespace graph
}  // namespace lgraph
