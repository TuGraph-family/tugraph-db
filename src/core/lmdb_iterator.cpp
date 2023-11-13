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

#if (!LGRAPH_USE_MOCK_KV)
#include "core/lmdb_iterator.h"
#include "core/wal.h"

namespace lgraph {

static const int DIR_BACKWARD = -1;
static const int DIR_FORWARD = 1;

static const int ST_MINIMUM = -1;
static const int ST_NORMAL = 0;
static const int ST_MAXIMUM = 1;

static const int IT_MAIN = 0;
static const int IT_DELTA = 1;

static const int OP_DELETE = -1;
static const int OP_GET_FOR_UPDATE = 0;
static const int OP_PUT = 1;

int LMDBKvIterator::Compare() {
    if (main_status_ != delta_status_) return main_status_ - delta_status_;
    if (main_status_ != 0) return main_status_;
    MDB_val delta_key;
    delta_key.mv_data = (void*)iter_->first.data();
    delta_key.mv_size = iter_->first.size();
    return table_->compare_key_(&key_, &delta_key);
}

void LMDBKvIterator::MoveForwardMain() {
    int ec;
    if (main_status_ == ST_MAXIMUM) return;
    if (main_status_ == ST_MINIMUM) {
        ec = mdb_cursor_get(cursor_, &key_, &value_, MDB_FIRST);
        if (ec == MDB_SUCCESS) {
            main_status_ = ST_NORMAL;
        } else if (ec == MDB_NOTFOUND) {
            main_status_ = ST_MAXIMUM;
        } else {
            THROW_ERR(ec);
        }
        return;
    }
    ec = mdb_cursor_get(cursor_, &key_, &value_, MDB_NEXT);
    if (ec == MDB_SUCCESS)
        return;
    else if (ec == MDB_NOTFOUND)
        main_status_ = ST_MAXIMUM;
    else
        THROW_ERR(ec);
}

void LMDBKvIterator::MoveForwardDelta() {
    if (delta_status_ == ST_MAXIMUM) return;
    if (delta_status_ == ST_MINIMUM) {
        iter_ = delta_->write_set_.begin();
        if (iter_ != delta_->write_set_.end()) {
            delta_status_ = ST_NORMAL;
            return;
        }
        delta_status_ = ST_MAXIMUM;
        return;
    }
    iter_++;
    if (iter_ != delta_->write_set_.end()) return;
    delta_status_ = ST_MAXIMUM;
}

void LMDBKvIterator::MoveBackwardMain() {
    int ec;
    if (main_status_ == ST_MINIMUM) return;
    if (main_status_ == ST_MAXIMUM) {
        ec = mdb_cursor_get(cursor_, &key_, &value_, MDB_LAST);
        if (ec == MDB_SUCCESS) {
            main_status_ = ST_NORMAL;
        } else if (ec == MDB_NOTFOUND) {
            main_status_ = ST_MINIMUM;
        } else {
            THROW_ERR(ec);
        }
        return;
    }
    ec = mdb_cursor_get(cursor_, &key_, &value_, MDB_PREV);
    if (ec == MDB_SUCCESS)
        return;
    else if (ec == MDB_NOTFOUND)
        main_status_ = ST_MINIMUM;
    else
        THROW_ERR(ec);
}

void LMDBKvIterator::MoveBackwardDelta() {
    if (delta_status_ == ST_MINIMUM) return;
    if (delta_status_ == ST_MAXIMUM) {
        if (!delta_->write_set_.empty()) {
            iter_ = --delta_->write_set_.end();
            delta_status_ = ST_NORMAL;
            return;
        }
        delta_status_ = ST_MINIMUM;
        return;
    }
    if (iter_ != delta_->write_set_.begin()) {
        iter_--;
        return;
    }
    delta_status_ = ST_MINIMUM;
}

void LMDBKvIterator::Fix() {
    if (current_mode_ == DIR_FORWARD) {
        while (main_status_ == ST_NORMAL && delta_status_ == ST_NORMAL) {
            int cmp_res = Compare();
            if (cmp_res != 0) {
                current_cursor_ = (cmp_res < 0) ? IT_MAIN : IT_DELTA;
                return;
            }
            int8_t op_type = *(int8_t*)(iter_->second.data() + sizeof(size_t));
            if (op_type == OP_DELETE) {
                MoveForwardMain();
                MoveForwardDelta();
            } else if (op_type == OP_GET_FOR_UPDATE) {
                MoveForwardDelta();
            } else {
                MoveForwardMain();
            }
        }
    } else {
        while (main_status_ == ST_NORMAL && delta_status_ == ST_NORMAL) {
            int cmp_res = Compare();
            if (cmp_res != 0) {
                current_cursor_ = (cmp_res > 0) ? ST_NORMAL : IT_DELTA;
                return;
            }
            int8_t op_type = *(int8_t*)(iter_->second.data() + sizeof(size_t));
            if (op_type == OP_DELETE) {
                MoveBackwardMain();
                MoveBackwardDelta();
            } else if (op_type == OP_GET_FOR_UPDATE) {
                MoveBackwardDelta();
            } else {
                MoveBackwardMain();
            }
        }
    }
    current_cursor_ = (main_status_ == 0) ? IT_MAIN : IT_DELTA;
}

LMDBKvIterator::~LMDBKvIterator() { LMDBKvIterator::Close(); }

void LMDBKvIterator::Close() {
    if (cursor_) {
        FMA_DBG_ASSERT(txn_->IsValid());
        mdb_cursor_close(cursor_);
        cursor_ = nullptr;
        valid_ = false;
    }
}

LMDBKvIterator::LMDBKvIterator(LMDBKvTransaction& txn, LMDBKvTable& table,
                               const Value& key, bool closest) {
    ThrowIfTaskKilled();
    txn_ = &txn;
    table_ = &table;
    THROW_ON_ERR(mdb_cursor_open(txn.GetTxn(), table.GetDbi(), &cursor_));
    if (!txn.read_only_ && txn.optimistic_) {
        delta_ = &txn.GetDelta(table);
    }
    if (key.Empty()) {
        LMDBKvIterator::GotoFirstKey();
    } else {
        if (closest)
            LMDBKvIterator::GotoClosestKey(key);
        else
            LMDBKvIterator::GotoKey(key);
    }
}

LMDBKvIterator::LMDBKvIterator(LMDBKvTransaction& txn, LMDBKvTable& table) {
    ThrowIfTaskKilled();
    txn_ = &txn;
    table_ = &table;
    THROW_ON_ERR(mdb_cursor_open(txn.GetTxn(), table.GetDbi(), &cursor_));
    if (!txn.read_only_ && txn.optimistic_) {
        delta_ = &txn.GetDelta(table);
        iter_ = delta_->write_set_.end();
        main_status_ = ST_MAXIMUM;
        delta_status_ = ST_MAXIMUM;
        current_mode_ = DIR_FORWARD;
        current_cursor_ = IT_MAIN;
    }
    valid_ = false;
}

LMDBKvIterator::LMDBKvIterator(const LMDBKvIterator& rhs)
    : txn_(rhs.txn_),
      table_(rhs.table_),
      cursor_(nullptr),
      valid_(rhs.valid_),
      delta_(rhs.delta_),
      iter_(rhs.iter_),
      main_status_(rhs.main_status_),
      delta_status_(rhs.delta_status_),
      current_mode_(rhs.current_mode_),
      current_cursor_(rhs.current_cursor_) {
    ThrowIfTaskKilled();
    if (valid_) {
        THROW_ON_ERR(mdb_cursor_open(txn_->GetTxn(), table_->GetDbi(), &cursor_));
        key_ = rhs.key_;
        value_ = rhs.value_;
        THROW_ON_ERR(mdb_cursor_get(cursor_, &key_, &value_, MDB_SET_KEY));
    }
}

LMDBKvIterator::LMDBKvIterator(LMDBKvIterator&& rhs) noexcept {
    cursor_ = std::move(rhs.cursor_);
    rhs.cursor_ = nullptr;
    txn_ = std::move(rhs.txn_);
    table_ = std::move(rhs.table_);
    valid_ = std::move(rhs.valid_);
    key_ = std::move(rhs.key_);
    value_ = std::move(rhs.value_);

    delta_ = std::move(rhs.delta_);
    iter_ = std::move(rhs.iter_);
    main_status_ = std::move(rhs.main_status_);
    delta_status_ = std::move(rhs.delta_status_);
    current_mode_ = std::move(rhs.current_mode_);
    current_cursor_ = std::move(rhs.current_cursor_);
}

LMDBKvIterator& LMDBKvIterator::operator=(LMDBKvIterator&& rhs) noexcept {
    if (this == &rhs) return *this;
    if (cursor_) {
        mdb_cursor_close(cursor_);
        cursor_ = nullptr;
        valid_ = false;
    }
    cursor_ = std::move(rhs.cursor_);
    rhs.cursor_ = nullptr;
    txn_ = std::move(rhs.txn_);

    table_ = std::move(rhs.table_);
    valid_ = std::move(rhs.valid_);
    key_ = std::move(rhs.key_);
    value_ = std::move(rhs.value_);

    delta_ = std::move(rhs.delta_);
    iter_ = std::move(rhs.iter_);
    main_status_ = std::move(rhs.main_status_);
    delta_status_ = std::move(rhs.delta_status_);
    current_mode_ = std::move(rhs.current_mode_);
    current_cursor_ = std::move(rhs.current_cursor_);

    return *this;
}

std::unique_ptr<KvIterator> LMDBKvIterator::Fork() {
    return std::make_unique<LMDBKvIterator>(*this);
}

bool LMDBKvIterator::UnderlyingPointerModified() {
    if (txn_->read_only_) return false;
    if (txn_->optimistic_) return true;
    return valid_ && (mdb_cursor_modified(cursor_) == 1);
}

bool LMDBKvIterator::RefreshAfterModify() {
    if (txn_->read_only_) return false;
    if (txn_->optimistic_) return true;
    if (!valid_ || !UnderlyingPointerModified()) return valid_;
    int ec = mdb_cursor_get(cursor_, &key_, &value_, MDB_GET_CURRENT);
    valid_ = (ec == MDB_SUCCESS);
    if (ec != MDB_SUCCESS && ec != MDB_NOTFOUND && ec != EINVAL) THROW_ERR(ec);
    return valid_;
}

bool LMDBKvIterator::Next() {
    ThrowIfTaskKilled();
    if (txn_->read_only_ || !txn_->optimistic_) {
        int ec = mdb_cursor_get(cursor_, &key_, &value_, MDB_NEXT);
        valid_ = (ec == MDB_SUCCESS);
        if (ec == MDB_SUCCESS || ec == MDB_NOTFOUND) return valid_;
        THROW_ERR(ec);
    }
    if (current_mode_ == DIR_BACKWARD) {
        if (current_cursor_ == IT_MAIN) {
            while (Compare() > 0) MoveForwardDelta();
        } else {
            while (Compare() < 0) MoveForwardMain();
        }
        current_mode_ = DIR_FORWARD;
    }
    if (current_cursor_ == IT_MAIN) {
        MoveForwardMain();
    } else {
        MoveForwardDelta();
    }
    Fix();
    return LMDBKvIterator::IsValid();
}

bool LMDBKvIterator::Prev() {
    ThrowIfTaskKilled();
    if (txn_->read_only_ || !txn_->optimistic_) {
        if (!valid_) return GotoLastKey();
        int ec = mdb_cursor_get(cursor_, &key_, &value_, MDB_PREV);
        valid_ = (ec == MDB_SUCCESS);
        if (ec == MDB_SUCCESS || ec == MDB_NOTFOUND) return valid_;
        THROW_ERR(ec);
    }
    if (current_mode_ == DIR_FORWARD) {
        if (current_cursor_ == IT_MAIN) {
            while (Compare() < 0) MoveBackwardDelta();
        } else {
            while (Compare() > 0) MoveBackwardMain();
        }
        current_mode_ = DIR_BACKWARD;
    }
    if (current_cursor_ == IT_MAIN) {
        MoveBackwardMain();
    } else {
        MoveBackwardDelta();
    }
    Fix();
    return LMDBKvIterator::IsValid();
}

bool LMDBKvIterator::GotoKey(const Value& key) {
    ThrowIfTaskKilled();
    key_ = key.MakeMdbVal();
    int ec;
    ec = mdb_cursor_get(cursor_, &key_, &value_, MDB_SET_KEY);
    if (ec != MDB_SUCCESS && ec != MDB_NOTFOUND) THROW_ERR(ec);
    if (txn_->read_only_ || !txn_->optimistic_) {
        valid_ = (ec == MDB_SUCCESS);
        return valid_;
    }
    main_status_ = (ec == MDB_SUCCESS) ? ST_NORMAL : ST_MAXIMUM;
    iter_ = delta_->write_set_.find(key);
    delta_status_ = (iter_ != delta_->write_set_.end()) ? IT_MAIN : ST_MAXIMUM;
    current_mode_ = DIR_FORWARD;
    if (delta_status_ == ST_NORMAL) {
        ec = mdb_cursor_get(cursor_, &key_, &value_, MDB_SET_RANGE);
        if (ec != MDB_SUCCESS && ec != MDB_NOTFOUND) THROW_ERR(ec);
        main_status_ = (ec == MDB_SUCCESS) ? ST_NORMAL : ST_MAXIMUM;
    }
    Fix();
    return LMDBKvIterator::IsValid();
}

bool LMDBKvIterator::GotoClosestKey(const Value& key) {
    ThrowIfTaskKilled();
    key_ = key.MakeMdbVal();
    int ec = mdb_cursor_get(cursor_, &key_, &value_, MDB_SET_RANGE);
    if (txn_->read_only_ || !txn_->optimistic_) {
        valid_ = (ec == MDB_SUCCESS);
        if (ec == MDB_SUCCESS || ec == MDB_NOTFOUND) return valid_;
        THROW_ERR(ec);
    }
    if (ec != MDB_SUCCESS && ec != MDB_NOTFOUND) THROW_ERR(ec);
    main_status_ = (ec == MDB_SUCCESS) ? ST_NORMAL : ST_MAXIMUM;
    iter_ = delta_->write_set_.lower_bound(key);
    delta_status_ = (iter_ != delta_->write_set_.end()) ? ST_NORMAL : ST_MAXIMUM;
    current_mode_ = DIR_FORWARD;
    Fix();
    return LMDBKvIterator::IsValid();
}

bool LMDBKvIterator::GotoLastKey() {
    ThrowIfTaskKilled();
    int ec = mdb_cursor_get(cursor_, &key_, &value_, MDB_LAST);
    if (txn_->read_only_ || !txn_->optimistic_) {
        valid_ = (ec == MDB_SUCCESS);
        if (ec == MDB_SUCCESS || ec == MDB_NOTFOUND) return valid_;
        THROW_ERR(ec);
    }
    if (ec != MDB_SUCCESS && ec != MDB_NOTFOUND) THROW_ERR(ec);
    main_status_ = (ec == MDB_SUCCESS) ? ST_NORMAL : ST_MINIMUM;
    if (!delta_->write_set_.empty()) {
        iter_ = --delta_->write_set_.end();
        delta_status_ = ST_NORMAL;
    } else {
        iter_ = delta_->write_set_.end();
        delta_status_ = ST_MINIMUM;
    }
    current_mode_ = DIR_BACKWARD;
    Fix();
    return LMDBKvIterator::IsValid();
}

bool LMDBKvIterator::GotoFirstKey() {
    ThrowIfTaskKilled();
    int ec = mdb_cursor_get(cursor_, &key_, &value_, MDB_FIRST);
    if (txn_->read_only_ || !txn_->optimistic_) {
        valid_ = (ec == MDB_SUCCESS);
        if (ec == MDB_SUCCESS || ec == MDB_NOTFOUND) return valid_;
        THROW_ERR(ec);
    }
    if (ec != MDB_SUCCESS && ec != MDB_NOTFOUND) THROW_ERR(ec);
    main_status_ = (ec == MDB_SUCCESS) ? ST_NORMAL : ST_MAXIMUM;
    iter_ = delta_->write_set_.begin();
    delta_status_ = (iter_ != delta_->write_set_.end()) ? ST_NORMAL : ST_MAXIMUM;
    current_mode_ = DIR_FORWARD;
    Fix();
    return LMDBKvIterator::IsValid();
}

Value LMDBKvIterator::GetKey() const {
    if (txn_->read_only_ || !txn_->optimistic_) {
        return Value(key_);
    } else {
        if (current_cursor_ == IT_MAIN) {
            return Value(key_);
        } else {
            return Value(iter_->first.data(), iter_->first.size());
        }
    }
}

Value LMDBKvIterator::GetValue(bool for_update) {
    if (txn_->read_only_ || !txn_->optimistic_) {
        return Value((char*)value_.mv_data + sizeof(size_t), value_.mv_size - sizeof(size_t));
    } else {
        if (current_cursor_ == IT_MAIN) {
            if (for_update) {
                size_t version = *(size_t*)(value_.mv_data);
                Value key(key_);
                delta_->GetForUpdate(key, version);
            }
            return Value((char*)value_.mv_data + sizeof(size_t), value_.mv_size - sizeof(size_t));
        } else {
            return Value(iter_->second.data() + sizeof(size_t) + sizeof(int8_t),
                         iter_->second.size() - sizeof(size_t) - sizeof(int8_t));
        }
    }
}

void LMDBKvIterator::SetValue(const Value& value) {
    ThrowIfTaskKilled();
    if (txn_->read_only_ || !txn_->optimistic_) {
        Value tmpv(sizeof(size_t) + value.Size());
        *(size_t*)(tmpv.Data()) = txn_->version_;
        memcpy(tmpv.Data() + sizeof(size_t), value.Data(), value.Size());
        value_ = tmpv.MakeMdbVal();
        Value tmpk = Value::MakeCopy(key_);
        key_ = tmpk.MakeMdbVal();
        int flags = 0;
        int ec = mdb_cursor_put(cursor_, &key_, &value_, flags);
        valid_ = (ec == MDB_SUCCESS);
        if (ec != MDB_SUCCESS) THROW_ERR(ec);
        if (txn_->GetWal())
            txn_->GetWal()->WriteKvPut(table_->GetDbi(), tmpk, tmpv);
        THROW_ON_ERR(mdb_cursor_get(cursor_, &key_, &value_, MDB_GET_CURRENT));
        return;
    }
    if (main_status_ != ST_NORMAL && delta_status_ != ST_NORMAL) {
        THROW_ERR("Failed to set value with an invalid cursor");
    }
    Value key;
    if (main_status_ == ST_NORMAL && delta_status_ == ST_NORMAL) {
        if (current_cursor_ == ST_NORMAL) {
            size_t version = *(size_t*)(value_.mv_data);
            key.Copy(key_);
            delta_->Put(key, version, value);
        } else {
            size_t version = *(size_t*)(iter_->second.data());
            key.Copy(iter_->first.data(), iter_->first.size());
            delta_->Put(key, version, value);
        }
    } else {
        if (main_status_ == ST_NORMAL) {
            size_t version = *(size_t*)(value_.mv_data);
            key.Copy(key_);
            delta_->Put(key, version, value);
        } else {
            size_t version = *(size_t*)(iter_->second.data());
            key.Copy(iter_->first.data(), iter_->first.size());
            delta_->Put(key, version, value);
        }
    }
    iter_ = delta_->write_set_.find(key);
    delta_status_ = ST_NORMAL;
    Fix();
}

bool LMDBKvIterator::AddKeyValue(const Value& key, const Value& value, bool overwrite) {
    ThrowIfTaskKilled();
    if (txn_->read_only_ || !txn_->optimistic_) {
        key_ = key.MakeMdbVal();
        Value tmpv(sizeof(size_t) + value.Size());
        *(size_t*)(tmpv.Data()) = txn_->version_;
        memcpy(tmpv.Data() + sizeof(size_t), value.Data(), value.Size());
        value_ = tmpv.MakeMdbVal();
        int flags = 0;
        if (!overwrite) flags |= MDB_NOOVERWRITE;
        int ec = mdb_cursor_put(cursor_, &key_, &value_, flags);
        valid_ = (ec == MDB_SUCCESS || ec == MDB_KEYEXIST);
        if (ec != MDB_SUCCESS && ec != MDB_KEYEXIST) THROW_ERR(ec);
        if (txn_->GetWal())
            txn_->GetWal()->WriteKvPut(table_->GetDbi(), key, tmpv);
        THROW_ON_ERR(mdb_cursor_get(cursor_, &key_, &value_, MDB_GET_CURRENT));
        return ec == MDB_SUCCESS;
    }
    auto status_value = delta_->Get(key);
    bool existing = false;
    if (status_value.first != 0) {
        if (status_value.first == OP_PUT) existing = true;
    } else {
        key_ = key.MakeMdbVal();
        int ec = mdb_cursor_get(cursor_, &key_, &value_, MDB_SET_KEY);
        if (ec == MDB_SUCCESS) existing = true;
        if (ec != MDB_SUCCESS && ec != MDB_NOTFOUND) THROW_ERR(ec);
    }
    if (!overwrite && existing) return false;
    size_t version = table_->GetVersion(*txn_, key);
    delta_->Put(key, version, value);
    LMDBKvIterator::GotoKey(key);
    return true;
}

void LMDBKvIterator::DeleteKey() {
    ThrowIfTaskKilled();
    if (txn_->read_only_ || !txn_->optimistic_) {
        Value tmpk = Value::MakeCopy(key_);
        THROW_ON_ERR(mdb_cursor_del(cursor_, 0));
        if (txn_->GetWal())
            txn_->GetWal()->WriteKvDel(table_->GetDbi(), tmpk);
        int ec = mdb_cursor_get(cursor_, &key_, &value_, MDB_GET_CURRENT);
        valid_ = (ec == MDB_SUCCESS);
        if (ec == MDB_SUCCESS || ec == MDB_NOTFOUND || ec == EINVAL) return;
        THROW_ERR(ec);
    }
    if (main_status_ != ST_NORMAL && delta_status_ != ST_NORMAL) {
        throw KvException("Failed to set value with an invalid cursor.");
    }
    Value key;
    if (main_status_ == ST_NORMAL && delta_status_ == ST_NORMAL) {
        if (current_cursor_ == IT_MAIN) {
            size_t version = *(size_t*)(value_.mv_data);
            key.Copy(key_);
            delta_->Delete(key, version);
        } else {
            size_t version = *(size_t*)(iter_->second.data());
            key.Copy(iter_->first.data(), iter_->first.size());
            delta_->Delete(key, version);
        }
    } else {
        if (main_status_ == ST_NORMAL) {
            size_t version = *(size_t*)(value_.mv_data);
            key.Copy(key_);
            delta_->Delete(key, version);
        } else {
            size_t version = *(size_t*)(iter_->second.data());
            key.Copy(iter_->first.data(), iter_->first.size());
            delta_->Delete(key, version);
        }
    }
    LMDBKvIterator::GotoClosestKey(key);
}

bool LMDBKvIterator::IsValid() const {
    if (txn_->read_only_ || !txn_->optimistic_) {
        return valid_;
    } else {
        return main_status_ == ST_NORMAL || delta_status_ == ST_NORMAL;
    }
}

}  // namespace lgraph
#endif
