/**
 * Copyright 2024 AntGroup CO., Ltd.
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

#include "core/composite_index.h"
#include "core/transaction.h"

namespace lgraph {

Value CompositeIndexValue::CreateKey(const Value &key) const {
    int pos = GetVidCount() - 1;
    Value v(key.Size() + _detail::VID_SIZE);
    memcpy(v.Data(), key.Data(), key.Size());
    memcpy(v.Data() + key.Size(), v_.Data() + 1 + pos * _detail::VID_SIZE, _detail::VID_SIZE);
    return v;
}

CompositeIndex::CompositeIndex(std::shared_ptr<KvTable> table, std::vector<FieldType> key_types,
    CompositeIndexType type) : table_(std::move(table)), key_types(std::move(key_types)),
    ready_(false), disabled_(false), type_(type) {}

CompositeIndex::CompositeIndex(const CompositeIndex& rhs)
    : table_(rhs.table_),
      key_types(rhs.key_types),
      ready_(rhs.ready_.load()),
      disabled_(rhs.disabled_.load()),
      type_(rhs.type_) {}

std::unique_ptr<KvTable> CompositeIndex::OpenTable(KvTransaction &txn, KvStore &store,
                                                   const std::string &name,
                                                   const std::vector<FieldType> &dt,
                                                   CompositeIndexType type) {
    ComparatorDesc desc;
    switch (type) {
    case CompositeIndexType::GlobalUniqueIndex:
        {
            desc.comp_type = ComparatorDesc::COMPOSITE_KEY;
            break;
        }
    }
    desc.data_types = dt;
    return store.OpenTable(txn, name, true, desc);
}

void CompositeIndex::_AppendCompositeIndexEntry(KvTransaction& txn, const Value& k, VertexId vid) {
    FMA_DBG_ASSERT(type_ == CompositeIndexType::GlobalUniqueIndex);
    if (k.Size() >= _detail::MAX_KEY_SIZE) {
        txn.Abort();
        THROW_CODE(InternalError, "The key of the composite index is too long and exceeds the limit.");
    }
    table_->AppendKv(txn, k, Value::ConstRef(vid));
}

}  // namespace lgraph
