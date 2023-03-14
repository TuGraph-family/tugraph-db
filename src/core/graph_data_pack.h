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

#include "core/data_type.h"
#include "core/value.h"

#ifdef _WIN32
#include <intrin.h>
/**
 * Gets number of bits needed to represent the integer i. For example, 0 bit
 * is needed for 0. 1 bit is needed for 1. 2 bits for 2 and 3, etc.
 *
 * @param   i   The integer
 *
 * @return  Number of bits needed
 */
inline size_t _NeededBits(int64_t i) {
    unsigned long ind;  // NOLINT
    return _BitScanReverse64(&ind, i) ? static_cast<size_t>(ind + 1) : 0;
}
#else
#include <limits.h>
inline size_t _NeededBits(int64_t i) { return i == 0 ? 0 : 64 - __builtin_clzll(i); }
#endif

int TestGraphDataPack(int, char**);

namespace lgraph {
namespace graph {
/**
 * Type of a key-value pair. It could be a vertex data, an incoming edge or an outgoing edge.
 * Since key is always 2-byte aligned, we use [5-byte vid]+[1-byte PackType] as the key of
 * vertex_only and packed_data.
 *
 * When a vertex is initially added, it is a PACKED_DATA node. As we add more edges to the
 * vertex, the packed data will grow. If it grows beyond a limit (currently 1KB), we split the
 * vertex into three nodes: one VERTEX_ONLY node, one OUT_EDGE node and one IN_EDGE node. As we
 * add more and more edges, the OUT_EDGE and IN_EDGE nodes can still be split into multiple edge
 * nodes. The order of the different nodes is determined by the key, which is defined in
 * KeyPacker.
 */
enum PackType {
    // PackType is stored as one byte, so the value cannot exceed 255
    PACKED_DATA = 0,  // packs vertex, in-coming and out-going edges
    VERTEX_ONLY = 1,  // contains only vertex data
    OUT_EDGE = 2,     // contains out-going edges
    IN_EDGE = 3       // contains in-coming edges
};

/** Data in the graph is stored as key-value pairs in kv-store, sorted by key. The KeyPacker packs
 * keys for different types of key-value pairs.
 *
 * PACKED_DATA: key is [5-bytes vid] + [1-byte PACKED_DATA]
 * VERTEX_ONLY: key is [5-bytes vid] + [1-byte VERTEX_ONLY]
 * OUT_EDGE:    key is [5-byte src_id] + [1-byte OUT_EDGE] + [2-byte label_id] +
 *                  [8-byte temporal_id] + [5-byte dst_id] + [4-byte eid]
 * IN_EDGE:     key is [5-byte dst_id] + [1-byte IN_EDGE] + [2-byte label_id] +
 *                  [8-byte temporal_id] + [5-byte src_id] + [4-byte eid]
 */
class KeyPacker {
    static const size_t FID_OFF = 0;                                      // first vid
    static const size_t PT_OFF = FID_OFF + ::lgraph::_detail::VID_SIZE;   // pair type
    static const size_t LID_OFF = PT_OFF + 1;                             // label
    static const size_t TID_OFF = LID_OFF + ::lgraph::_detail::LID_SIZE;  // primary id
    static const size_t SID_OFF = TID_OFF + ::lgraph::_detail::TID_SIZE;  // second vid
    static const size_t EID_OFF = SID_OFF + ::lgraph::_detail::VID_SIZE;  // edge id
    static const size_t EDGE_KEY_SIZE = EID_OFF + ::lgraph::_detail::EID_SIZE;

    /**
     * Gets pack type given the pointer to key.
     *
     * @param   p   Pointer to memory storing the PackType.
     *
     * @return  The pack type.
     */
    static inline PackType GetPackType(const char* p) { return PackType(*(uint8_t*)p); }

    /**
     * Stores pack type into memory buffer pointed to by p
     *
     * @param [in,out]  p   Pointer to memory buffer where PackType is
     *                      to be stored.
     * @param           pt  The PackType
     */
    static inline void SetPackType(char* p, PackType pt) {
        *(uint8_t*)p = static_cast<uint8_t>(pt);
    }

#if BYTE_ORDER == LITTLE_ENDIAN
    // integer stored in reverse byte order
    template <int N>
    static inline int64_t GetNByteIntId(const void* p) {
        int64_t r = 0;
        uint8_t* src = (uint8_t*)p;
        std::reverse_copy(src, src + N, (uint8_t*)&r);
        return r;
    }

    template <int N>
    static inline void SetNByteIntId(void* p, int64_t i) {
        uint8_t* src = (uint8_t*)&i;
        std::reverse_copy(src, src + N, (uint8_t*)p);
    }
#else
    template <int N>
    static inline int64_t GetNByteIntId(const void* p) {
        int64_t r = 0;
        uint8_t* dst = (uint8_t*)&r;
        uint8_t* src = (uint8_t*)p;
        std::copy(src, src + N, dst + (8 - N));
        return r;
    }

    template <int N>
    static inline void SetNByteIntId(void* p, int64_t i) {
        uint8_t* src = ((uint8_t*)&i) + 8 - N;
        std::copy(src, src + N, (uint8_t*)p);
    }
#endif

 public:
    /**
     * Gets the first vid stored in the key.
     *
     * @param   v   A Value storing the key.
     *
     * @return  The first vid.
     */
    static VertexId GetFirstVid(const Value& v) {
        return GetNByteIntId<::lgraph::_detail::VID_SIZE>(v.Data());
    }

    /**
     * Gets node type of the key. Valid even for PackedData, which does
     * not actually store the PackType.
     *
     * @param   v   A Value storing the key.
     *
     * @return  The node type.
     */
    static PackType GetNodeType(const Value& v) {
        if (v.Size() == ::lgraph::_detail::VID_SIZE) return PackType::PACKED_DATA;
        return GetPackType(v.Data() + PT_OFF);
    }

    /**
     * Gets the label stored in the key. Valid only for edge nodes.
     *
     * @param   v   A Value storing the key.
     *
     * @return  The label.
     */
    static LabelId GetLabel(const Value& v) {
        return (LabelId)GetNByteIntId<::lgraph::_detail::LID_SIZE>(v.Data() + LID_OFF);
    }

    /**
     * Gets second vid stored in the key. Valid only for edge nodes.
     *
     * @param   v   A Value storing the key.
     *
     * @return  The second vid.
     */
    static VertexId GetSecondVid(const Value& v) {
        return GetNByteIntId<::lgraph::_detail::VID_SIZE>(v.Data() + SID_OFF);
    }

    /**
     * Gets the edge id stored in the key. Valid only for edge nodes.
     *
     * @param   v   A Value storing the key.
     *
     * @return  The eid.
     */
    static EdgeId GetEid(const Value& v) {
        return GetNByteIntId<::lgraph::_detail::EID_SIZE>(v.Data() + EID_OFF);
    }

    /**
     * Creates a key for PackedData node.
     *
     * @param   vid  VertexId of this PackedData.
     *
     * @return  The key.
     */
    static Value CreatePackedDataKey(VertexId vid) {
        Value v(::lgraph::_detail::VID_SIZE);
        SetNByteIntId<::lgraph::_detail::VID_SIZE>(v.Data(), vid);
        return v;
    }

    /**
     * Creates a key for VertexOnly node.
     *
     * @param   vid  VertexId of this VertexOnly node.
     *
     * @return  The key.
     */
    static Value CreateVertexOnlyKey(VertexId vid) {
        Value v(::lgraph::_detail::VID_SIZE + 1);
        SetNByteIntId<::lgraph::_detail::VID_SIZE>(v.Data(), vid);
        SetPackType(v.Data() + PT_OFF, PackType::VERTEX_ONLY);
        return v;
    }

    /**
     * Creates key for either InEdge node or OutEdge node.
     *
     * @param   et      PackType, either IN_EDGE or OUT_EDGE.
     * @param   euid    EdgeId of InEdge or OutEdge
     *
     * @return  The key.
     */
    static Value CreateEdgeKey(PackType et, EdgeUid euid) {
        Value v(EDGE_KEY_SIZE);
        SetNByteIntId<::lgraph::_detail::VID_SIZE>(v.Data(), euid.src);
        SetPackType(v.Data() + PT_OFF, et);
        SetNByteIntId<::lgraph::_detail::LID_SIZE>(v.Data() + LID_OFF, euid.lid);
        SetNByteIntId<::lgraph::_detail::TID_SIZE>(v.Data() + TID_OFF, euid.tid);
        SetNByteIntId<::lgraph::_detail::VID_SIZE>(v.Data() + SID_OFF, euid.dst);
        SetNByteIntId<::lgraph::_detail::EID_SIZE>(v.Data() + EID_OFF, euid.eid);
        return v;
    }

    /**
     * Creates key for an InEdge node.
     *
     * @param   euid    EdgeUid of InEdge node.
     *
     * @return  The key.
     */
    static Value CreateInEdgeKey(EdgeUid euid) {
        return CreateEdgeKey(PackType::IN_EDGE, euid);
    }

    /**
     * Creates key for an OutEdge node.
     *
     * @param   euid    EdgeUid of an OutEdge node.
     *
     * @return  The new out edge key.
     */
    static Value CreateOutEdgeKey(EdgeUid euid) {
        return CreateEdgeKey(PackType::OUT_EDGE, euid);
    }

#if USELESS_CODE
    /**
     * Key compare function for two keys.
     *
     * @param   a   A MDB_val storing a key.
     * @param   b   A MDB_val storing a key.
     *
     * @return  -1 if a < b, 0 if a == b, 1 if a > b.
     */
    static int KeyCompareFunc(const MDB_val* a, const MDB_val* b) {
        const char* pa = (const char*)a->mv_data;
        const char* pb = (const char*)b->mv_data;
        VertexId ai = ::lgraph::_detail::GetVid(pa);
        VertexId bi = ::lgraph::_detail::GetVid(pb);
        if (ai > bi) return 1;
        if (ai < bi) return -1;
        // first element is the same
        if (a->mv_size == ::lgraph::_detail::VID_SIZE) {
            return (b->mv_size == ::lgraph::_detail::VID_SIZE) ? 0 : -1;
        }
        if (b->mv_size == ::lgraph::_detail::VID_SIZE) return 1;
        // from now on, the nodes are either VertexOnly or EdgeNode
        pa += ::lgraph::_detail::VID_SIZE;
        pb += ::lgraph::_detail::VID_SIZE;
        uint8_t at = *(uint8_t*)pa++;
        uint8_t bt = *(uint8_t*)pb++;
        if (at > bt) return 1;
        if (at < bt) return -1;
        // same src and type
        if (at == PackType::VERTEX_ONLY) {
            return 0;  // packed type does not have second id or eid
        }
        // two edge keys with same src and type, now compare (label, second_vid, eid)
        LabelId al = ::lgraph::_detail::GetLabelId(pa);
        LabelId bl = ::lgraph::_detail::GetLabelId(pb);
        if (al > bl) return 1;
        if (al < bl) return -1;
        // second vid
        pa += ::lgraph::_detail::LB_SIZE;
        pb += ::lgraph::_detail::LB_SIZE;
        ai = ::lgraph::_detail::GetVid(pa);
        bi = ::lgraph::_detail::GetVid(pb);
        if (ai > bi) return 1;
        if (ai < bi) return -1;
        // eid
        pa += ::lgraph::_detail::VID_SIZE;
        pb += ::lgraph::_detail::VID_SIZE;
        ai = ::lgraph::_detail::GetEid(pa);
        bi = ::lgraph::_detail::GetEid(pb);
        if (ai > bi) return 1;
        if (ai < bi) return -1;
        return 0;
    }
#endif
};

/** VertexValue stores only the vertex property. */
class VertexValue {
    // stores:
    //  vertex_property
    Value v_;

 public:
    VertexValue() = default;
    explicit VertexValue(Value&& v) : v_(std::move(v)) {}
    explicit VertexValue(const Value& v) : v_(Value::ConstRef(v)) {}

    const Value& GetVertexProperty() const { return v_; }

    const Value& GetBuf() const { return v_; }

    void CopyBuffer() { v_.Resize(v_.Size()); }

    ENABLE_RVALUE(void) SetProperty(VALUE&& prop) { v_ = std::forward<VALUE>(prop); }
};

/** EdgeValue stores a batch of edges sorted by (lid, tid, vid2, eid). */
class EdgeValue {
    friend int ::TestGraphDataPack(int, char**);

 public:
    // stores:
    //  uint8_t out_edge_count
    //  [PackDataOffset offsets]*
    //  [{1-byte size indicator}
    //  {0 to 2-byte label_id} {0 or 8-type tid} {1 to 5-byte vid} {0 to 2-byte eid} {edge_prop}]*

    // Size indicator:
    // 2-bit label_id size: 0, 1, or 2
    // 1-bit tid size: 0 or 8
    // 3-bit vid size: 0 to 5
    // 2-bit eid size: 0 to 3
    struct SizeIndicator {
        uint8_t lid_size : 2;
        uint8_t tid_indicator_size : 1;
        uint8_t vid_size : 3;
        uint8_t eid_size : 2;

        SizeIndicator() = default;
        SizeIndicator(size_t l, size_t p, size_t v, size_t e)
            : lid_size((uint8_t)l), vid_size((uint8_t)v), eid_size((uint8_t)e) {
            tid_indicator_size = p == 0 ? 0 : 1;
        }
        operator uint8_t() const { return *(uint8_t*)this; }

        uint8_t PidSize() const { return tid_indicator_size == 0 ? 0 : 8; }

        size_t TotalSize() const { return lid_size + PidSize() + vid_size + eid_size; }
    };

    /** EdgeHeader stores the identifier for an edge, that is, label, vid2 and eid. */
    struct EdgeHeader {
        LabelId lid;
        TemporalId tid;
        VertexId vid;
        EdgeId eid;
    };

    /** EdgeData represents the information of an edge as stored in
     *  EdgeValue, that is, edge identifier plus edge property.
     */
    struct EdgeData : public EdgeHeader {
        const char* prop;
        size_t psize;
    };

    /**
     * Gets header size for the edge stored in memory pointed by p.
     *
     * @param   p   Pointer to memory buffer storing an edge.
     *
     * @return  The header size.
     */
    static inline size_t GetHeaderSize(const char* p) {
        return ((SizeIndicator*)p)->TotalSize() + 1;
    }

    /**
     * Gets number of bytes required to store the LabelId given in lid.
     *
     * @param   lid The LabelId.
     *
     * @return  Number of bytes required.
     */
    static inline size_t GetLidSizeRequired(LabelId lid) {
        return lid == 0 ? 0 : lid < 0x100 ? 1 : 2;
    }

    /**
     * Gets number of bytes required to store the PrimaryId given in tid.
     *
     * @param   tid The TemporalId.
     *
     * @return  Number of bytes required.
     */
    static inline size_t GetPidSizeRequired(TemporalId tid) {
        return tid == 0 ? 0 : sizeof(TemporalId);
    }

    /**
     * Gets number of bytes required to store the VertexId given in vid.
     *
     * @param   vid The VertexId.
     *
     * @return  Number of bytes required.
     */
    static inline size_t GetVidSizeRequired(VertexId vid) { return (_NeededBits(vid) + 7) / 8; }

    /**
     * Gets number of bytes required to store the EdgeId given in eid.
     *
     * @param   eid The EdgeId.
     *
     * @return  Number of bytes required.
     */
    static inline size_t GetEidSizeRequired(EdgeId eid) { return (_NeededBits(eid) + 7) / 8; }

    /**
     * Gets number of bytes required to store the header. That is, number
     * of bytes required to store (lid, vid, eid) plus one byte to store
     * the indicator.
     *
     * @param   lid         LabelId.
     * @param   tid         The tid.
     * @param   vid         The vid.
     * @param   eid         The eid.
     *
     * @return  Number of bytes required.
     */
    static size_t GetHeaderSizeRequired(LabelId lid, TemporalId tid, VertexId vid, EdgeId eid) {
        return (size_t)1  // indicator
               + GetLidSizeRequired(lid) + GetPidSizeRequired(tid) + GetVidSizeRequired(vid) +
               GetEidSizeRequired(eid);
    }

    /**
     * Parse header, get the ids and return pointer right after header.
     *
     * @param        p   Pointer to the edge.
     * @param [out]  lid The lid.
     * @param [out]  tid The tid.
     * @param [out]  vid The vid.
     * @param [out]  eid The eid.
     *
     * @return  Pointer right after the header.
     */
    static const char* ParseHeader(const char* p, LabelId& lid, TemporalId& tid, VertexId& vid,
                                   EdgeId& eid) {
        SizeIndicator indicator = *(SizeIndicator*)p;
        p++;
        // get label id
        size_t lid_size = indicator.lid_size;
        FMA_DBG_ASSERT(lid_size <= 2);
        if (lid_size == 0) {
            lid = 0;
        } else {
            lid = (LabelId)::lgraph::_detail::GetNByteIdFromBuf(p, lid_size);
            p += lid_size;
        }
        // get primary id
        size_t tid_size = indicator.PidSize();
        FMA_DBG_ASSERT(tid_size == 8 || tid_size == 0);
        if (tid_size == 0) {
            tid = 0;
        } else {
            tid = ::lgraph::_detail::GetNByteIdFromBuf(p, tid_size);
            p += tid_size;
        }
        // get vertex id
        size_t vid_size = indicator.vid_size;
        FMA_DBG_ASSERT(vid_size <= 5);
        if (vid_size == 0) {
            vid = 0;
        } else {
            vid = ::lgraph::_detail::GetNByteIdFromBuf(p, vid_size);
            p += vid_size;
        }
        // get edge id
        size_t eid_size = indicator.eid_size;
        FMA_DBG_ASSERT(eid_size <= 3);
        if (eid_size == 0) {
            eid = 0;
        } else {
            eid = ::lgraph::_detail::GetNByteIdFromBuf(p, eid_size);
            p += eid_size;
        }
        return p;
    }

    /**
     * Set header to memory pointed by p, return a pointer right after the header The memory must
     * have been properly allocated before calling this function.
     *
     * @param [in,out]  p   Pointer to memory buffer, which must have been allocated properly.
     * @param           lid The lid.
     * @param           tid The tid.
     * @param           vid The vid.
     * @param           eid The eid.
     *
     * @return  Pointer right after header.
     */
    static char* SetHeader(char* p, LabelId lid, TemporalId tid, VertexId vid, EdgeId eid) {
        // get indicator
        size_t lid_size = GetLidSizeRequired(lid);
        size_t tid_size = GetPidSizeRequired(tid);
        size_t vid_size = GetVidSizeRequired(vid);
        size_t eid_size = GetEidSizeRequired(eid);
        SizeIndicator indicator(lid_size, tid_size, vid_size, eid_size);
        *(uint8_t*)p = indicator;
        p++;
        // label id
        if (lid_size != 0) {
            ::lgraph::_detail::SetNByteIdToBuf(p, lid_size, lid);
            p += lid_size;
        }
        // tid
        if (tid_size != 0) {
            ::lgraph::_detail::SetNByteIdToBuf(p, tid_size, tid);
            p += tid_size;
        }
        // vid
        if (vid_size != 0) {
            ::lgraph::_detail::SetNByteIdToBuf(p, vid_size, vid);
            p += vid_size;
        }
        // eid
        if (eid_size != 0) {
            ::lgraph::_detail::SetNByteIdToBuf(p, eid_size, eid);
            p += eid_size;
        }
        return p;
    }

 private:
    Value v_;
    size_t n_{};

    /** Loads the number of edges. */
    void LoadN() { n_ = static_cast<size_t>(*(uint8_t*)v_.Data()); }

    void StoreN(size_t n) {
        n_ = n;
        *(uint8_t*)v_.Data() = static_cast<uint8_t>(n_);
    }

    char* Offsets() const { return v_.Data() + 1; }

    size_t NOffsets() const { return NOffsets(n_); }

    static size_t NOffsets(size_t n) { return n <= 1 ? 0 : n - 1; }

    static size_t WriteEdge(LabelId lid, TemporalId tid, VertexId vid, VertexId eid,
                            const Value& prop, char* buf) {
        char* p = SetHeader(buf, lid, tid, vid, eid);
        memcpy(p, prop.Data(), prop.Size());
        return p + prop.Size() - buf;
    }

    DISABLE_COPY(EdgeValue);

 public:
    EdgeValue() : v_(1) { StoreN(0); }

    explicit EdgeValue(Value&& v) : v_(std::move(v)) { LoadN(); }

    explicit EdgeValue(const Value& v) : v_(Value::ConstRef(v)) { LoadN(); }

    EdgeValue(EdgeValue&& rhs) noexcept : v_(std::move(rhs.v_)), n_(rhs.n_) { rhs.n_ = 0; }

    EdgeValue& operator=(EdgeValue&& rhs) {
        if (&rhs == this) return *this;
        n_ = rhs.n_;
        v_ = std::move(rhs.v_);
        rhs.n_ = 0;
        return *this;
    }

    /**
     * construct an EdgeValue from a sorted array of edges. The edges must have been sorted by
     * (LabelId, TemporalId, VertexId)
     *
     * @tparam  IT  Type of the iterator.
     * @param           beg             The first iterator.
     * @param           end             The end iterator.
     * @param           total_edge_size Total number of bytes required to store all the edges.
     * @param [in,out]  last_lid        The last lid seen, will be changed by this function.
     * @param [in,out]  last_tid        The last tid seen, will be changed by this function.
     * @param [in,out]  last_vid        The last vid seen, will be changed by this function.
     * @param [in,out]  last_eid        The last eid seen, will be changed by this function.
     */
    template <typename IT>
    EdgeValue(const IT& beg,  // is a sequential iterator of std::tuple<LabelId, TemporalId
                              // VertexId, DenseString>
              const IT& end, LabelId& last_lid, TemporalId& last_tid, VertexId& last_vid,
              EdgeId& last_eid, typename std::decay<IT>::type& next_begin, bool no_split = false) {
        size_t edge_size = 0;
        LabelId clid = last_lid;
        TemporalId ctid = last_tid;
        VertexId cvid = last_vid;
        EdgeId ceid = last_eid;
        size_t n_edges = 0;
        // calculate size and set next_beg
        for (next_begin = beg; next_begin != end; next_begin++) {
            LabelId lid = std::get<0>(*next_begin);
            TemporalId tid = std::get<1>(*next_begin);
            VertexId vid = std::get<2>(*next_begin);
            const auto& prop = std::get<3>(*next_begin);
            size_t hsize;
            EdgeId eid = (lid != clid || vid != cvid || tid != ctid) ? 0 : ceid + 1;
            hsize = GetHeaderSizeRequired(lid, tid, vid, eid);
            if (!no_split && next_begin != beg &&           // at least include one edge
                edge_size                      // size so far
                        + hsize + prop.size()  // new edge size
                        + NOffsets(n_edges + 1) * sizeof(PackDataOffset) +
                        1  // overhead for EdgeValue
                    > ::lgraph::_detail::NODE_SPLIT_THRESHOLD) {
                break;
            }
            clid = lid;
            ctid = tid;
            cvid = vid;
            ceid = eid;
            edge_size += hsize + prop.size();
            n_edges++;
        }
        // allocate space for edges and the offsets
        v_.Resize(edge_size                                     // all edges
                  + NOffsets(n_edges) * sizeof(PackDataOffset)  // offset size
                  + 1);                                         // n_
        StoreN(n_edges);
        // write edges
        char* offsets = v_.Data() + 1;
        size_t eoff = GetNthEdgeOffset(0);
        char* buf = v_.Data() + eoff;
        for (auto it = beg; it != next_begin; ++it) {
            if (it != beg) {
                // first edge does not need offset
                ::lgraph::_detail::SetOffset(offsets, (it - beg) - 1, buf - v_.Data());
            }
            // write edge
            LabelId lid = std::get<0>(*it);
            TemporalId tid = std::get<1>(*it);
            VertexId vid = std::get<2>(*it);
            const auto& prop = std::get<3>(*it);
            last_eid = (lid != last_lid || vid != last_vid || tid != last_tid) ? 0 : last_eid + 1;
            last_lid = lid;
            last_tid = tid;
            last_vid = vid;
            buf = SetHeader(buf, lid, tid, vid, last_eid);
            memcpy(buf, prop.data(), prop.size());
            buf += prop.size();
        }
        FMA_DBG_ASSERT(buf == v_.Data() + v_.Size());
    }

    size_t GetEdgeCount() const { return n_; }

    /**
     * Parse the content of the n-th edge.
     *
     * @param           n           VertexIndex of the edge.
     * @param [in,out]  lid         The lid.
     * @param [in,out]  tid         The tid.
     * @param [in,out]  vid         The vid.
     * @param [in,out]  eid         The eid.
     * @param           prop        The property.
     * @param [in,out]  prop_size   Size of the property.
     */
    void ParseNthEdge(size_t n, LabelId& lid, TemporalId& tid, VertexId& vid, EdgeId& eid,
                      const char*& prop, size_t& prop_size) const {
        const char* p = GetNthEdge(n);
        prop = ParseHeader(p, lid, tid, vid, eid);
        prop_size = GetNthEdge(n + 1) - prop;
    }

    /**
     * Gets nth edge data
     *
     * @param   n   VertexIndex of the edge.
     *
     * @return  The nth edge data.
     */
    EdgeData GetNthEdgeData(size_t n) const {
        EdgeData ed;
        ParseNthEdge(n, ed.lid, ed.tid, ed.vid, ed.eid, ed.prop, ed.psize);
        return ed;
    }

    /**
     * Gets nth edge header
     *
     * @param   n   VertexIndex of the edge.
     *
     * @return  The nth edge header.
     */
    EdgeHeader GetNthEdgeHeader(size_t n) const {
        EdgeHeader eh;
        const char* p = GetNthEdge(n);
        ParseHeader(p, eh.lid, eh.tid, eh.vid, eh.eid);
        return eh;
    }

    /**
     * Search for an edge in the EdgeValue.
     *
     * @param           lid     The lid.
     * @param           tid     The tid.
     * @param           vid     The vid.
     * @param           eid     The eid.
     * @param [in,out]  found   Set to true if the edge is found, false otherwise.
     *
     * @return  The index of the edge if found, otherwise the position where the
     *          specified edge should belong, which can be used in InsertAtPos().
     */
    size_t SearchEdge(LabelId lid, TemporalId tid, VertexId vid, EdgeId eid, bool& found) const {
        found = false;
        size_t beg = 0;
        size_t end = n_;
        while (beg < end) {
            size_t p = (beg + end) / 2;
            LabelId ll;
            TemporalId pp;
            VertexId vv;
            EdgeId ee;
            const char* prop;
            size_t prop_size;
            ParseNthEdge(p, ll, pp, vv, ee, prop, prop_size);
            int cmp;
            // compare (lid, vid, eid)
            if (lid < ll)
                cmp = -1;
            else if (lid > ll)
                cmp = 1;
            else if (tid < pp)
                cmp = -1;
            else if (tid > pp)
                cmp = 1;
            else if (vid < vv)
                cmp = -1;
            else if (vid > vv)
                cmp = 1;
            else
                cmp = eid < ee ? -1 : (eid > ee ? 1 : 0);
            // binary search
            if (cmp == 0) {
                found = true;
                return p;
            } else if (cmp == -1) {
                end = p;
            } else {
                beg = p + 1;
            }
        }
        return end;
    }

    /**
     * Updates the nth edge and return size diff
     *
     * @param   p       VertexIndex of the edge.
     * @param   prop    The new property.
     *
     * @return  Number of bytes grown (positive) or shrinked (negative).
     */
    int64_t UpdateNthEdge(size_t p, const Value& prop) {
        size_t eoff = GetNthEdgeOffset(p);
        size_t next_off = GetNthEdgeOffset(p + 1);
        size_t hsize = GetHeaderSize(v_.Data() + eoff);
        size_t orig_size = (next_off - eoff) - hsize;
        int64_t offset_diff = (int64_t)prop.Size() - orig_size;
        if (offset_diff == 0) {
            // simply replace original property
            v_.Resize(v_.Size());
            memcpy(v_.Data() + eoff + hsize, prop.Data(), prop.Size());
        } else {
            Value newv;
            newv.Resize(v_.Size() + offset_diff);
            char* ptr = newv.Data();
            memcpy(ptr, v_.Data(), eoff + hsize);
            ptr += eoff + hsize;
            memcpy(ptr, prop.Data(), prop.Size());
            ptr += prop.Size();
            memcpy(ptr, v_.Data() + next_off, v_.Size() - next_off);
            // adjust the offsets
            char* new_offsets = newv.Data() + 1;
            for (size_t i = p + 1; i < n_; i++) {
                size_t off = ::lgraph::_detail::GetOffset(new_offsets, i - 1);
                ::lgraph::_detail::SetOffset(new_offsets, i - 1, off + offset_diff);
            }
            v_ = std::move(newv);
        }
        return offset_diff;
    }

    /**
     * Update or insert an edge. If the edge already exists, update it, otherwise insert it. exists
     * and pos are set accordingly.
     *
     * @param        lid     The lid.
     * @param        tid     The tid.
     * @param        vid     The vid.
     * @param        eid     The eid.
     * @param        prop    The new property.
     * @param [out]  exists  True if the edge already exists.
     * @param [out]  pos     The index of the edge.
     *
     * @return  number of bytes grown (positive) or shrinked (negative)
     */
    int64_t UpsertEdge(LabelId lid, TemporalId tid, VertexId vid, EdgeId eid, const Value& prop,
                       bool& exists, size_t& pos) {
        pos = SearchEdge(lid, tid, vid, eid, exists);
        if (exists)
            return UpdateNthEdge(pos, prop);
        else
            return InsertAtPos(pos, lid, tid, vid, eid, prop);
    }

    /**
     * Delete an edge if it exists
     *
     * @param   lid The lid.
     * @param   tid The tid.
     * @param   vid The vid.
     * @param   eid The eid.
     *
     * @return  True if edge exists and is deleted. False otherwise.
     */
    bool DeleteEdgeIfExist(LabelId lid, TemporalId tid, VertexId vid, EdgeId eid) {
        bool exists;
        size_t p = SearchEdge(lid, tid, vid, eid, exists);
        if (!exists) return false;
        DeleteNthEdge(p);
        return true;
    }

    /**
     * Gets the buffer for EdgeValue.
     *
     * @return  The buffer.
     */
    const Value& GetBuf() const { return v_; }

    Value& GetBuf() { return v_; }

    /**
     * Splits the node into two parts of similar size and return the left half. The right half is
     * kept in *this. Left part is guaranteed to be: either smaller than lhs_limit, or has only
     * one huge edge.
     *
     * \return  An OutEdgeValue storing the left half.
     *
     * @param   lhs_limit   (Optional) The left hand side limit.
     *
     * @return  The left part.
     */
    EdgeValue SplitEven(size_t lhs_limit = ::lgraph::_detail::NODE_SPLIT_THRESHOLD) {
        FMA_DBG_ASSERT(n_ >= 1);
        size_t target_size = std::min<size_t>(v_.Size() / 2, lhs_limit);
        char* offsets = Offsets();
        size_t p = 1;
        for (p = 1; p < n_; p++) {
            if ((size_t)::lgraph::_detail::GetOffset(offsets, p - 1) > target_size) break;
        }
        if (p != 1) p--;
        return SplitAtPos(p);
    }

    /**
     * Creates key for OutEdge node
     *
     * @param   src Source VertexId.
     *
     * @return  The key.
     */
    Value CreateOutEdgeKey(VertexId src) const {
        char* e = GetNthEdge(GetEdgeCount() - 1);
        LabelId lid;
        TemporalId tid;
        VertexId vid;
        EdgeId eid;
        ParseHeader(e, lid, tid, vid, eid);
        return KeyPacker::CreateOutEdgeKey(EdgeUid(src, vid, lid, tid, eid));
    }

    /**
     * Creates key for InEdge node
     *
     * @param   dst Destination VertexId.
     *
     * @return  The key.
     */
    Value CreateInEdgeKey(VertexId dst) const {
        char* e = GetNthEdge(GetEdgeCount() - 1);
        LabelId lid;
        TemporalId tid;
        VertexId vid;
        EdgeId eid;
        ParseHeader(e, lid, tid, vid, eid);
        return KeyPacker::CreateInEdgeKey(EdgeUid(dst, vid, lid, tid, eid));
    }

    /**
     * Creates a key for InEdge node or OutEdge node, depending on et.
     *
     * @param   et      The edge type, either IN_EDGE or OUT_EDGE.
     * @param   vid1    The vid1.
     *
     * @return  The new key.
     */
    Value CreateKey(PackType et, VertexId vid1) const {
        char* e = GetNthEdge(GetEdgeCount() - 1);
        LabelId lid;
        TemporalId tid;
        VertexId vid;
        EdgeId eid;
        ParseHeader(e, lid, tid, vid, eid);
        return KeyPacker::CreateEdgeKey(et, EdgeUid(vid1, vid, lid, tid, eid));
    }

    /**
     * Copies the buffer, makes sure the buffer is owned by EdgeValue. This is used before modifying
     * the edges, so that we don't mess with the KvStore.
     */
    void CopyBuffer() { v_.Resize(v_.Size()); }

    // get the offset of n-th edge
    size_t GetNthEdgeOffset(size_t n) const {
        if (n == 0) return 1 + NOffsets() * sizeof(PackDataOffset);
        if (n == n_) return v_.Size();
        return ::lgraph::_detail::GetOffset(Offsets(), n - 1);
    }

    // get the memory buffer for n-th edge
    char* GetNthEdge(size_t n) const { return v_.Data() + GetNthEdgeOffset(n); }

    // insert an edge at position p, and return the number of bytes grown
    int64_t InsertAtPos(size_t p, LabelId lid, TemporalId tid, VertexId vid, EdgeId eid,
                        const Value& prop) {
        size_t hsize = GetHeaderSizeRequired(lid, tid, vid, eid);
        size_t offset_diff =
            hsize + static_cast<int>(prop.Size()) +
            (n_ == 0 ? 0 : sizeof(PackDataOffset));  // if only one edge, don't need offset
        Value newv(v_.Size() + offset_diff);
        *(uint8_t*)newv.Data() = static_cast<uint8_t>(n_ + 1);
        char* old_offsets = Offsets();
        char* new_offsets = newv.Data() + 1;
        size_t edge_off = GetNthEdgeOffset(0);
        if (p != 0) {
            for (size_t i = 1; i < p; i++) {
                size_t old_offset = ::lgraph::_detail::GetOffset(old_offsets, i - 1);
                ::lgraph::_detail::SetOffset(new_offsets, i - 1,
                                             old_offset + sizeof(PackDataOffset));
            }
            ::lgraph::_detail::SetOffset(new_offsets, p - 1,
                                         GetNthEdgeOffset(p) + sizeof(PackDataOffset));
            for (size_t i = p; i < n_; i++) {
                size_t old_off = ::lgraph::_detail::GetOffset(Offsets(), i - 1);
                ::lgraph::_detail::SetOffset(new_offsets, i, old_off + offset_diff);
            }
        } else {
            if (n_ != 0) {
                // if we insert at pos==0, the original first edge offset must be set
                ::lgraph::_detail::SetOffset(new_offsets, 0, offset_diff + edge_off);
            }
            for (size_t i = 1; i < n_; i++) {
                size_t old_off = ::lgraph::_detail::GetOffset(Offsets(), i - 1);
                ::lgraph::_detail::SetOffset(new_offsets, i, old_off + offset_diff);
            }
        }
        char* newptr = newv.Data() + edge_off + (n_ == 0 ? 0 : sizeof(PackDataOffset));
        char* oldptr = v_.Data() + edge_off;
        size_t to_copy = GetNthEdgeOffset(p) - edge_off;
        memcpy(newptr, oldptr, to_copy);
        oldptr += to_copy;
        newptr += to_copy;
        size_t r = WriteEdge(lid, tid, vid, eid, prop, newptr);
        newptr += r;
        memcpy(newptr, oldptr, v_.Data() + v_.Size() - oldptr);
        n_++;
        // replace the buffer
        v_ = std::move(newv);
        return static_cast<int64_t>(offset_diff);
    }

    /**
     * Splits at position pos and return the left half. The right half
     * is kept in *this.
     *
     * \param   pos The position.
     *
     * \return  An EdgeValue storing the left half.
     */
    EdgeValue SplitAtPos(size_t pos) {
        if (pos == 0) {
            v_.Resize(v_.Size());
            EdgeValue oev(std::move(v_));
            v_.Resize(1);
            StoreN(0);
            return oev;
        }
        if (pos == n_) {
            v_.Resize(v_.Size());
            return EdgeValue();
        }
        size_t n_lhs = pos;
        size_t n_rhs = n_ - pos;

        size_t eoff = GetNthEdgeOffset(pos);
        size_t l_offset_diff = n_rhs * sizeof(PackDataOffset);
        Value lhs(eoff - l_offset_diff);
        size_t r_offset_start = 1 + NOffsets(n_rhs) * sizeof(PackDataOffset);
        size_t r_offset_diff = eoff - r_offset_start;
        Value rhs(v_.Size() - r_offset_diff);
        // copy offsets
        char* lptr = lhs.Data();
        *(uint8_t*)lptr = static_cast<uint8_t>(n_lhs);
        lptr++;
        char* oldptr = v_.Data() + 1;
        for (size_t i = 1; i < pos; i++) {
            size_t off = ::lgraph::_detail::GetOffset(oldptr, i - 1);
            ::lgraph::_detail::SetOffset(lptr, i - 1, off - l_offset_diff);
        }
        lptr += (pos - 1) * sizeof(PackDataOffset);
        char* rptr = rhs.Data();
        *(uint8_t*)rptr = static_cast<uint8_t>(n_rhs);
        rptr++;
        // offset at pos now becomes 0 and is left out
        for (size_t i = pos + 1; i < n_; i++) {
            size_t off = ::lgraph::_detail::GetOffset(oldptr, i - 1);
            ::lgraph::_detail::SetOffset(rptr, i - pos - 1, off - r_offset_diff);
        }
        // copy edges
        rptr += (n_rhs - 1) * sizeof(PackDataOffset);
        oldptr += (n_ - 1) * sizeof(PackDataOffset);
        size_t to_copy = eoff - GetNthEdgeOffset(0);
        memcpy(lptr, oldptr, to_copy);
        oldptr += to_copy;
        memcpy(rptr, oldptr, v_.Size() - eoff);
        v_ = std::move(rhs);
        n_ = n_rhs;
        return EdgeValue(std::move(lhs));
    }

    // delete edge at position p
    void DeleteNthEdge(size_t p) {
        if (n_ == 1) {
            v_.Resize(1);
            *(uint8_t*)v_.Data() = 0;
            n_ = 0;
            return;
        }
        size_t eoff = GetNthEdgeOffset(p);
        size_t next_eoff = GetNthEdgeOffset(p + 1);
        size_t esize = next_eoff - eoff;
        size_t offset_diff = esize + sizeof(PackDataOffset);
        Value newv(v_.Size() - offset_diff);
        char* newptr = newv.Data();
        *(uint8_t*)newptr = static_cast<uint8_t>(n_ - 1);
        newptr++;
        char* oldptr = v_.Data() + 1;
        if (p == 0) {
            for (size_t i = 2; i < n_; i++) {
                size_t old_offset = ::lgraph::_detail::GetOffset(oldptr, i - 1);
                ::lgraph::_detail::SetOffset(newptr, i - 2, old_offset - offset_diff);
            }
        } else {
            for (size_t i = 1; i < p; i++) {
                size_t old_offset = ::lgraph::_detail::GetOffset(oldptr, i - 1);
                ::lgraph::_detail::SetOffset(newptr, i - 1, old_offset - sizeof(PackDataOffset));
            }
            for (size_t i = p + 1; i < n_; i++) {
                size_t old_offset = ::lgraph::_detail::GetOffset(oldptr, i - 1);
                ::lgraph::_detail::SetOffset(newptr, i - 2, old_offset - offset_diff);
            }
        }
        oldptr = v_.Data() + GetNthEdgeOffset(0);
        newptr = newv.Data() + GetNthEdgeOffset(0) - sizeof(PackDataOffset);
        size_t to_copy = GetNthEdgeOffset(p) - GetNthEdgeOffset(0);
        memcpy(newptr, oldptr, to_copy);
        oldptr = v_.Data() + next_eoff;
        newptr += to_copy;
        to_copy = v_.Size() - next_eoff;
        memcpy(newptr, oldptr, to_copy);
        // replace the buffer
        v_ = std::move(newv);
        n_--;
    }
};

class PackedDataValue {
    // stores:
    //  [PackDataOffset out_edge_value_off]
    //  [PackDataOffset in_edge_value_off]
    //  VertexValue
    //  OutEdgeValue
    //  InEdgeValue
    static const size_t VertexDataOff = sizeof(PackDataOffset) * 2;

    Value v_;

    size_t GetOutEdgeValueOff() const { return ::lgraph::_detail::GetOffset(v_.Data(), 0); }

    size_t GetInEdgeValueOff() const { return ::lgraph::_detail::GetOffset(v_.Data(), 1); }

    Value GetVertexProperty() const {
        return Value(v_.Data() + VertexDataOff, GetOutEdgeValueOff() - VertexDataOff);
    }

    Value GetOutEdgeData() const {
        size_t off = GetOutEdgeValueOff();
        return Value(v_.Data() + off, GetInEdgeValueOff() - off);
    }

    Value GetInEdgeData() const {
        size_t off = GetInEdgeValueOff();
        return Value(v_.Data() + off, v_.Size() - off);
    }

 public:
    PackedDataValue() { PackData(VertexValue(), EdgeValue(), EdgeValue(), v_); }

    explicit PackedDataValue(Value&& v) : v_(std::move(v)) {}

    explicit PackedDataValue(const Value& v) : v_(Value::ConstRef(v)) {}

    VertexValue GetVertexData() const { return VertexValue(GetVertexProperty()); }

    EdgeValue GetInEdge() const { return EdgeValue(GetInEdgeData()); }

    EdgeValue GetOutEdge() const { return EdgeValue(GetOutEdgeData()); }

    static void PackData(const VertexValue& vov, const EdgeValue& oev, const EdgeValue& iev,
                         Value& buf) {
        size_t vsize = vov.GetBuf().Size();
        size_t osize = oev.GetBuf().Size();
        size_t isize = iev.GetBuf().Size();
        buf.Resize(vsize + isize + osize + VertexDataOff);
        char* ptr = buf.Data();
        size_t off = vsize + VertexDataOff;
        ::lgraph::_detail::SetOffset(ptr, 0, off);
        off += osize;
        ::lgraph::_detail::SetOffset(ptr, 1, off);
        ptr += VertexDataOff;
        memcpy(ptr, vov.GetBuf().Data(), vsize);
        ptr += vsize;
        memcpy(ptr, oev.GetBuf().Data(), osize);
        ptr += osize;
        memcpy(ptr, iev.GetBuf().Data(), isize);
    }

    static PackedDataValue PackData(const VertexValue& vov, const EdgeValue& oev,
                                    const EdgeValue& iev) {
        Value v;
        PackData(vov, oev, iev, v);
        return PackedDataValue(std::move(v));
    }

    const Value& GetBuf() const { return v_; }

    Value& GetBuf() { return v_; }

    void CopyBuffer() { v_.Resize(v_.Size()); }

    Value CreateKey(VertexId vid) const { return KeyPacker::CreatePackedDataKey(vid); }
};
}  // namespace graph
}  // namespace lgraph
