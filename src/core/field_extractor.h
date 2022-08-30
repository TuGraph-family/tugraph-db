/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include "core/blob_manager.h"
#include "core/field_data_helper.h"
#include "core/vertex_index.h"
#include "core/edge_index.h"
#include "core/schema_common.h"

namespace lgraph {
class Schema;

namespace _detail {

#define ENABLE_IF_FIXED_FIELD(_TYPE_, _RT_) \
    template <typename _TYPE_>              \
    typename std::enable_if<                \
        std::is_integral<_TYPE_>::value || std::is_floating_point<_TYPE_>::value, _RT_>::type

/** A field extractor can be used to get/set a field in the record. */
class FieldExtractor {
    friend class lgraph::Schema;
    // type information
    FieldSpec def_;
    // layout
    size_t field_id_ = 0;
    bool is_vfield_ = false;
    union {
        size_t data_off = 0;
        struct {
            size_t idx;  // index of this field in all the vfields
            size_t v_offs;
            size_t last_idx;
        };
    } offset_;
    size_t nullable_array_off_ = 0;  // offset of nullable array in record
    size_t null_bit_off_ = 0;
    // index
    std::unique_ptr<VertexIndex> vertex_index_;
    std::unique_ptr<EdgeIndex> edge_index_;
    // fulltext index
    bool fulltext_indexed_ = false;

 public:
    FieldExtractor() : null_bit_off_(0), vertex_index_(nullptr), edge_index_(nullptr) {}

    ~FieldExtractor() {}

    FieldExtractor(const FieldExtractor& rhs) {
        def_ = rhs.def_;
        field_id_ = rhs.field_id_;
        is_vfield_ = rhs.is_vfield_;
        offset_ = rhs.offset_;
        nullable_array_off_ = rhs.nullable_array_off_;
        null_bit_off_ = rhs.null_bit_off_;
        vertex_index_.reset(rhs.vertex_index_ ? new VertexIndex(*rhs.vertex_index_) : nullptr);
        edge_index_.reset(rhs.edge_index_ ? new EdgeIndex(*rhs.edge_index_) : nullptr);
        fulltext_indexed_ = rhs.fulltext_indexed_;
    }

    FieldExtractor& operator=(const FieldExtractor& rhs) {
        if (this == &rhs) return *this;
        def_ = rhs.def_;
        field_id_ = rhs.field_id_;
        is_vfield_ = rhs.is_vfield_;
        offset_ = rhs.offset_;
        null_bit_off_ = rhs.null_bit_off_;
        nullable_array_off_ = rhs.nullable_array_off_;
        vertex_index_.reset(rhs.vertex_index_ ? new VertexIndex(*rhs.vertex_index_) : nullptr);
        edge_index_.reset(rhs.edge_index_ ? new EdgeIndex(*rhs.edge_index_) : nullptr);
        fulltext_indexed_ = rhs.fulltext_indexed_;
        return *this;
    }

    FieldExtractor(FieldExtractor&& rhs) noexcept {
        def_ = std::move(rhs.def_);
        field_id_ = rhs.field_id_;
        is_vfield_ = rhs.is_vfield_;
        offset_ = rhs.offset_;
        null_bit_off_ = rhs.null_bit_off_;
        nullable_array_off_ = rhs.nullable_array_off_;
        vertex_index_ = std::move(rhs.vertex_index_);
        edge_index_ = std::move(rhs.edge_index_);
        rhs.vertex_index_ = nullptr;
        rhs.edge_index_ = nullptr;
        fulltext_indexed_ = rhs.fulltext_indexed_;
    }

    FieldExtractor& operator=(FieldExtractor&& rhs) noexcept {
        if (this == &rhs) return *this;
        def_ = std::move(rhs.def_);
        field_id_ = rhs.field_id_;
        is_vfield_ = rhs.is_vfield_;
        offset_ = rhs.offset_;
        null_bit_off_ = rhs.null_bit_off_;
        nullable_array_off_ = rhs.nullable_array_off_;
        vertex_index_ = std::move(rhs.vertex_index_);
        edge_index_ = std::move(rhs.edge_index_);
        fulltext_indexed_ = rhs.fulltext_indexed_;
        return *this;
    }

    // for test only
    explicit FieldExtractor(const FieldSpec& d) noexcept : def_(d) {
        is_vfield_ = !field_data_helper::IsFixedLengthFieldType(d.type);
        vertex_index_ = nullptr;
        edge_index_ = nullptr;
        null_bit_off_ = 0;
        if (is_vfield_) SetVLayoutInfo(d.optional ? 1 : 0, 1, 0);
    }

    const FieldSpec& GetFieldSpec() const { return def_; }

    bool GetIsNull(const Value& record) const {
        if (!def_.optional) {
            return false;
        } else {
            // get the Kth bit from NullArray
            char* arr = GetNullArray(record);
            return arr[null_bit_off_ / 8] & (0x1 << (null_bit_off_ % 8));
        }
    }

    /**
     * Extract a field from record into data of type T. T must be fixed-length
     * type.
     *
     * \param   record  The record in which fields are stored.
     * \param   data    Place where the extracted data will be stored.
     *
     * Assert fails if data is corrupted.
     */
    ENABLE_IF_FIXED_FIELD(T, void) GetCopy(const Value& record, T& data) const {
        FMA_DBG_ASSERT(field_data_helper::FieldTypeSize(def_.type) == sizeof(T));
        FMA_DBG_ASSERT(offset_.data_off + field_data_helper::FieldTypeSize(def_.type) <=
                       record.Size());
        memcpy(&data, (char*)record.Data() + offset_.data_off, sizeof(T));
    }

    /**
     * Extracts a copy of field into the string.
     *
     * \param           record  The record.
     * \param [in,out]  data    The result data.
     *
     * Assert fails if data is corrupted.
     */
    void GetCopy(const Value& record, std::string& data) const {
        FMA_DBG_ASSERT(Type() != FieldType::BLOB);
        data.resize(GetDataSize(record));
        GetCopyRaw(record, &data[0], data.size());
    }

    /**
     * Extracts field data from the record
     *
     * \param           record  The record.
     * \param [in,out]  data    The result.
     *
     * Assert fails if data is corrupted.
     */
    void GetCopy(const Value& record, Value& data) const {
        data.Resize(GetDataSize(record));
        GetCopyRaw(record, data.Data(), data.Size());
    }

    // Gets a const reference of the field.
    // Formatted data is returned for blob, which means [is_large_blob] [blob_data | blob_key]
    Value GetConstRef(const Value& record) const {
        if (GetIsNull(record)) return Value();
        return Value((char*)GetFieldPointer(record), GetDataSize(record));
    }

    // gets a const ref to the blob content
    // get_blob_by_key is a function that accepts BlobKey and returns Value containing blob content
    template <typename GetBlobByKeyFunc>
    Value GetBlobConstRef(const Value& record, const GetBlobByKeyFunc& get_blob_by_key) const {
        FMA_DBG_ASSERT(Type() == FieldType::BLOB);
        if (GetIsNull(record)) return Value();
        Value v((char*)GetFieldPointer(record), GetDataSize(record));
        if (BlobManager::IsLargeBlob(v)) {
            return get_blob_by_key(BlobManager::GetLargeBlobKey(v));
        } else {
            return BlobManager::GetSmallBlobContent(v);
        }
    }

    // parse a string as input and then set field in record
    // cannot be used for blobs since they need formatting
    void ParseAndSet(Value& record, const std::string& data) const;

    // get FieldData as input and then set field in record
    // used for blobs *only* in case of AlterLabel, when we need to
    // copy old data into new format
    void ParseAndSet(Value& record, const FieldData& data) const;

    // parse and set a blob
    // data can be string or FieldData
    // store_blob is a function of type std::function<BlobKey(const Value&)>
    template <typename DataT, typename StoreBlobAndGetKeyFunc>
    void ParseAndSetBlob(Value& record, const DataT& data,
                         const StoreBlobAndGetKeyFunc& store_blob) const {
        FMA_DBG_ASSERT(Type() == FieldType::BLOB);
        bool is_null;
        Value v = ParseBlob(data, is_null);
        SetIsNull(record, is_null);
        if (is_null) return;
        if (v.Size() <= _detail::MAX_IN_PLACE_BLOB_SIZE) {
            _SetVariableLengthValue(record, BlobManager::ComposeSmallBlobData(v));
        } else {
            BlobManager::BlobKey key = store_blob(v);
            v.Clear();
            _SetVariableLengthValue(record, BlobManager::ComposeLargeBlobData(key));
        }
    }

    void CopyDataRaw(Value& dst_record, const Value& src_record, const FieldExtractor* extr) const {
        if (extr->GetIsNull(src_record)) {
            SetIsNull(dst_record, true);
            return;
        }
        SetIsNull(dst_record, false);
        if (is_vfield_) {
            _SetVariableLengthValue(dst_record, extr->GetConstRef(src_record));
        } else {
            _SetFixedSizeValueRaw(dst_record, extr->GetConstRef(src_record));
        }
    }

    const std::string& Name() const { return def_.name; }

    FieldType Type() const { return def_.type; }

    size_t TypeSize() const { return field_data_helper::FieldTypeSize(def_.type); }

    size_t DataSize(const Value& record) const { return GetDataSize(record); }

    bool IsOptional() const { return def_.optional; }

    /**
     * Print the string representation of the field. For digital types, it prints
     * it into ASCII string; for NBytes and String, it just copies the content of
     * the field into the string.
     *
     * \param   record  The record.
     *
     * \return  String representation of the field.
     */
    std::string FieldToString(const Value& record) const;

    VertexIndex* GetVertexIndex() const { return vertex_index_.get(); }

    EdgeIndex* GetEdgeIndex() const { return edge_index_.get(); }

    bool FullTextIndexed() const { return fulltext_indexed_; }

    size_t GetFieldId() const { return field_id_; }

 private:
    void SetVertexIndex(VertexIndex* index) { vertex_index_.reset(index); }

    void SetEdgeIndex(EdgeIndex* edgeindex) { edge_index_.reset(edgeindex); }
    void SetFullTextIndex(bool fulltext_indexed) { fulltext_indexed_ = fulltext_indexed; }

    void SetFixedLayoutInfo(size_t offset) {
        is_vfield_ = false;
        offset_.data_off = offset;
    }

    void SetVLayoutInfo(size_t voff, size_t nv, size_t idx) {
        is_vfield_ = true;
        offset_.v_offs = voff;
        offset_.last_idx = nv - 1;
        offset_.idx = idx;
    }

    void SetNullableOff(size_t offset) { null_bit_off_ = offset; }

    void SetNullableArrayOff(size_t offset) { nullable_array_off_ = offset; }

    void SetFieldId(size_t n) { field_id_ = n; }

    //-----------------------
    // record accessors

    // get a const ref of raw blob data
    inline Value ParseBlob(const FieldData& fd, bool& is_null) const {
        if (fd.type == FieldType::NUL) {
            is_null = true;
            return Value();
        }
        is_null = false;
        if (fd.type == FieldType::BLOB) {
            return Value::ConstRef(*fd.data.buf);
        }
        if (fd.type == FieldType::STRING) {
            std::string decoded;
            const std::string& s = *fd.data.buf;
            if (!::lgraph_api::base64::TryDecode(s.data(), s.size(), decoded))
                throw ParseStringException(Name(), s, Type());
            return Value(decoded);
        } else {
            throw ParseIncompatibleTypeException(Name(), fd.type, FieldType::BLOB);
            return Value();
        }
    }

    inline Value ParseBlob(const std::string& str, bool& is_null) const {
        // string input is always seen as non-NULL
        is_null = false;
        // decode str as base64
        std::string decoded;
        if (!::lgraph_api::base64::TryDecode(str.data(), str.size(), decoded))
            throw ParseStringException(Name(), str, Type());
        return Value(decoded);
    }

    template <FieldType FT>
    void _ParseStringAndSet(Value& record, const std::string& data) const;

    /**
     * Sets the value of the field in the record, assuming it is not a null value.
     * data should not be empty for fixed field
     *
     * \param [in,out]  record  The record.
     * \param           data    The data.
     *
     * \return  ErrorCode::OK if succeeds, or
     *          FIELD_CANNOT_BE_NULL
     *          DATA_SIZE_TOO_LARGE
     */
    void _SetVariableLengthValue(Value& record, const Value& data) const;

    /**
     * Sets the value of the field in record. Valid only for fixed-length fields.
     *
     * \param   record  The record.
     * \param   data    Value to be set.
     *
     * \return   ErrorCode::OK if succeeds.
     */
    ENABLE_IF_FIXED_FIELD(T, void)
    SetFixedSizeValue(Value& record, const T& data) const {
        // "Cannot call SetField(Value&, const T&) on a variable length field";
        FMA_DBG_ASSERT(!is_vfield_);
        // "Type size mismatch"
        FMA_DBG_CHECK_EQ(sizeof(data), field_data_helper::FieldTypeSize(def_.type));
        // copy the buffer so we don't accidentally overwrite memory
        record.Resize(record.Size());
        char* ptr = (char*)record.Data() + offset_.data_off;
        ::lgraph::_detail::UnalignedSet<T>(ptr, data);
    }

    void _SetFixedSizeValueRaw(Value& record, const Value& data) const {
        // "Cannot call SetField(Value&, const T&) on a variable length field";
        FMA_DBG_ASSERT(!is_vfield_);
        // "Type size mismatch"
        FMA_DBG_CHECK_EQ(data.Size(), field_data_helper::FieldTypeSize(def_.type));
        // copy the buffer so we don't accidentally overwrite memory
        char* ptr = (char*)record.Data() + offset_.data_off;
        memcpy(ptr, data.Data(), data.Size());
    }

    // set field value to null
    void SetIsNull(Value& record, bool is_null) const {
        if (!def_.optional) {
            if (is_null) throw FieldCannotBeSetNullException(Name());
            return;
        }
        // set the Kth bit from NullArray
        char* arr = GetNullArray(record);
        if (is_null) {
            arr[null_bit_off_ / 8] |= (0x1 << (null_bit_off_ % 8));
        } else {
            arr[null_bit_off_ / 8] &= ~(0x1 << (null_bit_off_ % 8));
        }
    }

    /**
     * Extracts field data from the record to the buffer pointed to by data. This
     * is for internal use only, the size MUST match the data size.
     *
     * \param           record  The record.
     * \param [in,out]  data    If non-null, the data.
     * \param           size    Size of field, must be equal to field size.
     *
     * Assert fails if data is corrupted.
     */
    void GetCopyRaw(const Value& record, void* data, size_t size) const {
        size_t off = GetFieldOffset(record);
        FMA_DBG_ASSERT(off + size <= record.Size());
        memcpy(data, record.Data() + off, size);
    }

    char* GetNullArray(const Value& record) const { return record.Data() + nullable_array_off_; }

    size_t GetDataSize(const Value& record) const {
        if (is_vfield_) {
            return GetNextOffset(record) - GetFieldOffset(record);
        } else {
            return field_data_helper::FieldTypeSize(def_.type);
        }
    }

    size_t GetFieldOffset(const Value& record) const {
        if (is_vfield_) {
            size_t off =
                (offset_.idx == 0)
                    ? (offset_.v_offs + sizeof(DataOffset) * (offset_.last_idx))
                    : ::lgraph::_detail::UnalignedGet<DataOffset>(
                          record.Data() + offset_.v_offs + (offset_.idx - 1) * sizeof(DataOffset));
            return off;
        } else {
            return offset_.data_off;
        }
    }

    size_t GetNextOffset(const Value& record) const {
        if (is_vfield_) {
            size_t off =
                (offset_.idx == offset_.last_idx)
                    ? record.Size()
                    : ::lgraph::_detail::UnalignedGet<DataOffset>(record.Data() + offset_.v_offs +
                                                                  offset_.idx * sizeof(DataOffset));
            return off;
        } else {
            return offset_.data_off + field_data_helper::FieldTypeSize(def_.type);
        }
    }

    void* GetFieldPointer(const Value& record) const {
        return (char*)record.Data() + GetFieldOffset(record);
    }
};

}  // namespace _detail

}  // namespace lgraph
