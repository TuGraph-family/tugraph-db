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

//
// Created by wt on 6/15/18.
//
#pragma once

#include "core/lightning_graph.h"
#include "cypher/cypher_types.h"
#include "cypher/cypher_exception.h"
#include "cypher/graph/common.h"

namespace lgraph {

// TODO(anyone) using native lgraph iterator
class LabelVertexIterator {
    lgraph::Transaction *_txn = nullptr;
    lgraph::graph::VertexIterator *_it = nullptr;
    std::string _label;
    bool _valid = false;

 public:
    LabelVertexIterator(lgraph::Transaction *txn, const std::string &label)
        : _txn(txn), _label(label) {
        _it = new lgraph::graph::VertexIterator(_txn->GetVertexIterator());
        while (_it->IsValid()) {
            if (_txn->GetVertexLabel(*_it) == _label) {
                _valid = true;
                break;
            }
            _it->Next();
        }
    }

    LabelVertexIterator(LabelVertexIterator &&rhs) noexcept
        : _txn(rhs._txn), _it(rhs._it), _label(std::move(rhs._label)), _valid(rhs._valid) {
        rhs._txn = nullptr;
        rhs._it = nullptr;
        rhs._valid = false;
    }

    DISABLE_COPY(LabelVertexIterator);

    ~LabelVertexIterator() {
        delete _it;
        _it = nullptr;
    }

    bool Next() {
        _valid = false;
        while (_it->IsValid()) {
            _it->Next();
            if (_it->IsValid() && _txn->GetVertexLabel(*_it) == _label) {
                _valid = true;
                break;
            }
        }
        return _valid;
    }

    bool IsValid() const { return _valid; }

    lgraph::VertexId GetId() const {
        if (!IsValid()) THROW_CODE(InternalError, "get id of invalid LabelVertexIterator");
        return _it->GetId();
    }

    bool Goto(lgraph::VertexId vid) {
        _it->Goto(vid);
        _valid = false;
        while (_it->IsValid()) {
            if (_txn->GetVertexLabel(*_it) == _label) {
                _valid = true;
                break;
            }
            _it->Next();
        }
        return _valid;
    }
};

class WeakIndexIterator {
    lgraph::Transaction *_txn = nullptr;
    lgraph::graph::VertexIterator *_it = nullptr;
    std::vector<lgraph::VertexIndexIterator *> _iit;
    size_t _iit_idx = 0;
    std::string _label;
    std::string _field_name;
    FieldData _value;
    bool _valid = false;

    void SeekIndex(const std::string &l) {
        if (_txn->IsIndexed(l, _field_name)) {
            auto iit = _txn->GetVertexIndexIterator(l, _field_name, _value, _value);
            if (iit.IsValid()) {
                _iit.emplace_back(new lgraph::VertexIndexIterator(std::move(iit)));
            }
        }
    }

 public:
    WeakIndexIterator(lgraph::Transaction *txn, const std::string &label, const std::string &field,
                      const FieldData &value)
        : _txn(txn), _label(label), _field_name(field), _value(value) {
        if (!_label.empty()) {
            SeekIndex(_label);
        } else {
            auto v_labels = _txn->GetAllLabels(true);
            for (auto &l : v_labels) SeekIndex(l);
        }
        if (!_iit.empty()) {
            _iit_idx = 0;
            _valid = true;
        } else {
            // TODO(anyone) optimize, use label iterator
            _it = new lgraph::graph::VertexIterator(_txn->GetVertexIterator());
            while (_it->IsValid()) {
                if ((_label.empty() || _txn->GetVertexLabel(*_it) == _label) &&
                    _txn->GetVertexField(*_it, _field_name) == value) {
                    _valid = true;
                    break;
                }
                _it->Next();
            }
        }
    }

    WeakIndexIterator(WeakIndexIterator &&rhs) noexcept
        : _txn(rhs._txn),
          _it(rhs._it),
          _iit(std::move(rhs._iit)),
          _iit_idx(rhs._iit_idx),
          _label(std::move(rhs._label)),
          _field_name(std::move(rhs._field_name)),
          _value(rhs._value),
          _valid(rhs._valid) {
        rhs._txn = nullptr;
        rhs._it = nullptr;
        rhs._iit.clear();
        rhs._valid = false;
    }

    DISABLE_COPY(WeakIndexIterator);

    ~WeakIndexIterator() {
        delete _it;
        _it = nullptr;
        for (auto i : _iit) {
            delete i;
        }
    }

    bool Next() {
        _valid = false;
        if (_it) {
            while (_it->IsValid()) {
                _it->Next();
                if (_it->IsValid() && (_label.empty() || _txn->GetVertexLabel(*_it) == _label) &&
                    _txn->GetVertexField(*_it, _field_name) == _value) {
                    _valid = true;
                    break;
                }
            }
        } else {
            auto iit = _iit[_iit_idx];
            while (iit->IsValid()) {
                iit->Next();
                if (iit->IsValid() && (_label.empty() || _txn->GetVertexLabel(*_it) == _label) &&
                    _txn->GetVertexField(iit->GetVid(), _field_name) == _value) {
                    _valid = true;
                    break;
                }
            }
            if (!_valid) {
                if (_iit_idx < _iit.size() - 1) {
                    _iit_idx++;
                    _valid = true;
                }
            }
        }
        return _valid;
    }

    bool Goto(lgraph::VertexId vid) {
        _valid = false;
        if (_it) {
            while (_it->IsValid()) {
                _it->GotoClosestVertex(vid);
                if (_it->IsValid() && (_label.empty() || _txn->GetVertexLabel(*_it) == _label) &&
                    _txn->GetVertexField(*_it, _field_name) == _value) {
                    _valid = true;
                    break;
                }
            }
        } else {
            auto iit = _iit[_iit_idx];
            while (iit->IsValid()) {
                iit->Goto(vid);
                if (iit->IsValid() && _txn->GetVertexField(iit->GetVid(), _field_name) == _value) {
                    _valid = true;
                    break;
                }
            }
            if (!_valid) {
                if (_iit_idx < _iit.size() - 1) {
                    _iit_idx++;
                    _valid = true;
                }
            }
        }
        return _valid;
    }

    bool IsValid() const { return _valid; }

    lgraph::VertexId GetId() const {
        if (_it && _valid) {
            return _it->GetId();
        } else if (_iit[_iit_idx] && _valid) {
            return _iit[_iit_idx]->GetVid();
        } else {
            return -1;
        }
    }
};

// wrapper of all kinds of vertex iterators
class VIter {
 public:
    enum IteratorType {
        VERTEX_ITER,
        INDEX_ITER,
        WEAK_INDEX_ITER,
        LABEL_VERTEX_ITER,
        NA,
    };

    VIter() = default;

    VIter(lgraph::Transaction *txn, IteratorType type) { Initialize(txn, type); }

    VIter(lgraph::Transaction *txn, IteratorType type, lgraph::VertexId vid) {
        Initialize(txn, type, vid);
    }

    VIter(lgraph::Transaction *txn, IteratorType type, const std::string &label,
          const std::string &field, const FieldData &key_start, const FieldData &key_end) {
        Initialize(txn, type, label, field, key_start, key_end);
    }

    VIter(lgraph::Transaction *txn, IteratorType type, const std::string &label) {
        Initialize(txn, type, label);
    }

    ~VIter() { FreeIter(); }

    void FreeIter() {
        switch (_type) {
        case VERTEX_ITER:
            delete _vit;
            break;
        case INDEX_ITER:
            delete _iit;
            break;
        case WEAK_INDEX_ITER:
            delete _wit;
            break;
        case LABEL_VERTEX_ITER:
            delete _lvit;
            break;
        default:
            break;
        }
        _vit = nullptr;
    }

    void Initialize(lgraph::Transaction *txn, IteratorType type) {
        FreeIter();
        _txn = txn;
        _type = type;
        if (_type == VERTEX_ITER) {
            _vit = new lgraph::graph::VertexIterator(_txn->GetVertexIterator());
        } else {
            throw lgraph::CypherException("VIter constructor type error.");
        }
        _vid = _vit->GetId();
    }

    void Initialize(lgraph::Transaction *txn, IteratorType type, lgraph::VertexId vid) {
        FreeIter();
        _txn = txn;
        _type = type;
        if (_type == VERTEX_ITER) {
            _vit = new lgraph::graph::VertexIterator(_txn->GetVertexIterator(vid));
        } else {
            throw lgraph::CypherException("VIter constructor type error.");
        }
        _vid = vid;
    }

    void Initialize(lgraph::Transaction *txn, IteratorType type, const std::string &label,
                    const std::string &field, const FieldData &key_start,
                    const FieldData &key_end) {
        FreeIter();
        _txn = txn;
        _type = type;
        if (_type == INDEX_ITER) {
            _iit = new lgraph::VertexIndexIterator(
                _txn->GetVertexIndexIterator(label, field, key_start, key_end));
        } else {
            throw lgraph::CypherException("VIter constructor type error.");
        }
        _label = label;
        _field = field;
        _key_start = key_start;
        _key_end = key_end;
    }

    void Initialize(lgraph::Transaction *txn, const std::string &label, const std::string &field,
                    const FieldData &value) {
        FreeIter();
        _txn = txn;
        _type = WEAK_INDEX_ITER;
        _wit = new lgraph::WeakIndexIterator(_txn, label, field, value);
        _field = field;
        _key_start = value;
    }

    void Initialize(lgraph::Transaction *txn, IteratorType type, const std::string &label) {
        FreeIter();
        _txn = txn;
        _type = type;
        _label = label;
        if (_type == LABEL_VERTEX_ITER) {
            _lvit = new lgraph::LabelVertexIterator(_txn, _label);
        } else {
            throw lgraph::CypherException("VIter constructor type error.");
        }
    }

    // move constructor
    VIter(VIter &&rhs) noexcept {
        _type = rhs._type;
        _vit = rhs._vit;
        rhs._vit = nullptr;
        _txn = rhs._txn;
        rhs._txn = nullptr;
        _vid = rhs._vid;
        _label = std::move(rhs._label);
        _field = std::move(rhs._field);
        _key_start = std::move(rhs._key_start);
        _key_end = std::move(rhs._key_end);
    }

    VIter &operator=(VIter &&rhs) = delete;

    bool Initialized() const { return _vit != nullptr; }

    bool IsValid() const {
        switch (_type) {
        case VERTEX_ITER:
            return (_vit && _vit->IsValid());
        case INDEX_ITER:
            return (_iit && _iit->IsValid());
        case WEAK_INDEX_ITER:
            return (_wit && _wit->IsValid());
        case LABEL_VERTEX_ITER:
            return (_lvit && _lvit->IsValid());
        default:
            return false;
        }
    }

    bool Next() {
        switch (_type) {
        case VERTEX_ITER:
            return (_vit && _vit->Next());
        case INDEX_ITER:
            return (_iit && _iit->Next());
        case WEAK_INDEX_ITER:
            return (_wit && _wit->Next());
        case LABEL_VERTEX_ITER:
            return (_lvit && _lvit->Next());
        default:
            return false;
        }
    }

    bool Goto(lgraph::VertexId vid) {
        switch (_type) {
        case VERTEX_ITER:
            return (_vit && _vit->Goto(vid));
        case LABEL_VERTEX_ITER:
            return (_lvit && _lvit->Goto(vid));
        case WEAK_INDEX_ITER:
            return (_wit && _wit->Goto(vid));
        case INDEX_ITER:
            return (_iit && _iit->Goto(vid));
        default:
            CYPHER_TODO();
            return false;
        }
    }

    lgraph::VertexId GetId() const {
        static const int64_t INVALID_VERTEX_ID = -1;
        switch (_type) {
        case VERTEX_ITER:
            return _vit ? _vit->GetId() : INVALID_VERTEX_ID;
        case INDEX_ITER:
            return _iit ? _iit->GetVid() : INVALID_VERTEX_ID;
        case WEAK_INDEX_ITER:
            return _wit ? _wit->GetId() : INVALID_VERTEX_ID;
        case LABEL_VERTEX_ITER:
            return _lvit ? _lvit->GetId() : INVALID_VERTEX_ID;
        default:
            break;
        }
        return INVALID_VERTEX_ID;
    }

    FieldData GetField(const std::string &fd) {
        switch (_type) {
        case VERTEX_ITER:
            return _txn->GetVertexField(*_vit, fd);
        case INDEX_ITER:
            return _txn->GetVertexField(_iit->GetVid(), fd);
        case WEAK_INDEX_ITER:
            return _txn->GetVertexField(_wit->GetId(), fd);
        case LABEL_VERTEX_ITER:
            return _txn->GetVertexField(_lvit->GetId(), fd);
        default:
            break;
        }
        return FieldData();
    }

    std::string GetLabel() const {
        static const std::string EMPTY_LABEL;
        switch (_type) {
        case VERTEX_ITER:
            return _vit ? _txn->GetVertexLabel(*_vit) : EMPTY_LABEL;
        case INDEX_ITER:
            return _iit ? _txn->GetVertexLabel(_txn->GetVertexIterator(_iit->GetVid()))
                        : EMPTY_LABEL;
        case WEAK_INDEX_ITER:
            return _wit ? _txn->GetVertexLabel(_txn->GetVertexIterator(_wit->GetId()))
                        : EMPTY_LABEL;
        case LABEL_VERTEX_ITER:
            return _lvit ? _txn->GetVertexLabel(_txn->GetVertexIterator(_lvit->GetId()))
                         : EMPTY_LABEL;
        default:
            break;
        }
        return EMPTY_LABEL;
    }

    std::vector<std::pair<std::string, FieldData>> GetFields() {
        return _txn->GetVertexFields(*_vit);
    }

    std::string Properties() const {
        static const std::string EMPTY_PROPERTIES;
        switch (_type) {
        case VERTEX_ITER:
            return _vit ? Properties(*_vit) : EMPTY_PROPERTIES;
        case INDEX_ITER:
            return _iit ? Properties(_txn->GetVertexIterator(_iit->GetVid())) : EMPTY_PROPERTIES;
        case WEAK_INDEX_ITER:
            return _wit ? Properties(_txn->GetVertexIterator(_wit->GetId())) : EMPTY_PROPERTIES;
        case LABEL_VERTEX_ITER:
            return _lvit ? Properties(_txn->GetVertexIterator(_lvit->GetId())) : EMPTY_PROPERTIES;
        default:
            break;
        }
        return EMPTY_PROPERTIES;
    }

    std::vector<std::string> Keys() const {
        static const std::vector<std::string> EMPTY_KEYS;
        switch (_type) {
        case VERTEX_ITER:
            return _vit ? Keys(*_vit) : EMPTY_KEYS;
        case INDEX_ITER:
            return _iit ? Keys(_txn->GetVertexIterator(_iit->GetVid())) : EMPTY_KEYS;
        case WEAK_INDEX_ITER:
            return _wit ? Keys(_txn->GetVertexIterator(_wit->GetId())) : EMPTY_KEYS;
        case LABEL_VERTEX_ITER:
            return _lvit ? Keys(_txn->GetVertexIterator(_lvit->GetId())) : EMPTY_KEYS;
        default:
            break;
        }
        return EMPTY_KEYS;
    }

    void Reset() {
        switch (_type) {
        case VERTEX_ITER:
            _vit->Goto(_vid);
            break;
        case INDEX_ITER:
            // TODO(anyone): use goto method
            delete _iit;
            _iit = new lgraph::VertexIndexIterator(
                _txn->GetVertexIndexIterator(_label, _field, _key_start, _key_end));
            break;
        case WEAK_INDEX_ITER:
            delete _wit;
            _wit = new lgraph::WeakIndexIterator(_txn, _label, _field, _key_start);
            break;
        case LABEL_VERTEX_ITER:
            _lvit->Goto(0);
        default:
            break;
        }
    }

 private:
    IteratorType _type = NA;
    union {
        lgraph::graph::VertexIterator *_vit = nullptr;
        lgraph::VertexIndexIterator *_iit;
        lgraph::WeakIndexIterator *_wit;
        lgraph::LabelVertexIterator *_lvit;
    };
    lgraph::Transaction *_txn = nullptr;
    lgraph::VertexId _vid = 0;  // initial vid, for vit reset
    std::string _label;         // for iit reset
    std::string _field;
    FieldData _key_start;
    FieldData _key_end;

    std::string Properties(const lgraph::graph::VertexIterator &it) const {
        std::string p;
        if (!it.IsValid()) return p;
        p.append("{");
        // _LABEL_
        p.append("\"_LABEL_\":\"").append(_txn->GetVertexLabel(it)).append("\"");
        p.append(",");
        // _VID_
        p.append("\"_VID_\":").append(std::to_string(it.GetId()));
        auto fields = it.GetTxn()->GetVertexFields(it);
        for (size_t i = 0; i < fields.size(); i++) {
            auto f = fields[i];
            p.append(",");
            p.append("\"").append(f.first).append("\":");
            if (f.second.IsString())
                p.append("\"").append(f.second.ToString()).append("\"");
            else
                p.append(f.second.ToString());
        }
        p.append("}");
        return p;
    }

    std::vector<std::string> Keys(const lgraph::graph::VertexIterator &it) const {
        std::vector<std::string> keys;
        auto schema = it.GetTxn()->GetVertexSchema(it);
        for (auto &s : schema) keys.emplace_back(s.name);
        return keys;
    }
};

// Labeled Edge Iterator
// TODO(anyone): Is there an optimization opportunity? considering the edges is sorted by labelId
// now.
template <typename EIT>
class TypeEdgeIterator {
    lgraph::Transaction *_txn = nullptr;
    EIT *_eit = nullptr;
    /* Unlike vertex, if we’d like to describe some data such that
     * the relationship could have any one of a set of types, then
     * they can all be listed in the pattern like this:
     * (a)-[r:TYPE1|TYPE2]->(b)  */
    std::set<std::string> _types;
    bool _valid = false;

 public:
    TypeEdgeIterator(lgraph::Transaction *txn, EIT *eit, const std::set<std::string> &types)
        : _txn(txn), _eit(eit), _types(types) {
        // TODO(anyone) optimize with eit.goto label_id
        while (_eit->IsValid()) {
            if (_types.find(_txn->GetEdgeLabel(*_eit)) != _types.end()) {
                _valid = true;
                break;
            }
            _eit->Next();
        }
    }

    TypeEdgeIterator(TypeEdgeIterator &&rhs) noexcept
        : _txn(rhs._txn), _eit(rhs._eit), _types(std::move(rhs._types)), _valid(rhs._valid) {
        rhs._txn = nullptr;
        rhs._eit = nullptr;
        rhs._valid = false;
    }

    ~TypeEdgeIterator() {
        delete _eit;
        _eit = nullptr;
    }

    EIT *Eit() const { return _eit; }

    const std::set<std::string> &Types() const { return _types; }

    bool Next() {
        _valid = false;
        while (_eit->IsValid()) {
            _eit->Next();
            if (_eit->IsValid() && _types.find(_txn->GetEdgeLabel(*_eit)) != _types.end()) {
                _valid = true;
                break;
            }
        }
        return _valid;
    }

    bool IsValid() const { return _valid; }

    lgraph::EdgeUid GetUid() const {
        if (!IsValid()) THROW_CODE(InternalError, "get uid of invalid EIter");
        return _eit->GetUid();
    }

    lgraph::VertexId GetSrc() const {
        if (!IsValid()) THROW_CODE(InternalError, "get src of invalid EIter");
        return _eit->GetSrc();
    }

    lgraph::VertexId GetDst() const {
        if (!IsValid()) THROW_CODE(InternalError, "get dst of invalid EIter");
        return _eit->GetDst();
    }

    lgraph::LabelId GetLabel() const {
        if (!IsValid()) THROW_CODE(InternalError, "get label of invalid EIter");
        return _eit->GetLabelId();
    }

    lgraph::EdgeId GetEdgeId() const {
        if (!IsValid()) THROW_CODE(InternalError, "get eid of invalid EIter");
        return _eit->GetEdgeId();
    }
};

// wrapper of all kinds of edge iterators
class EIter {
 public:
    enum IteratorType {
        OUT_EDGE,
        IN_EDGE,
        TYPE_OUT_EDGE,
        TYPE_IN_EDGE,
        BI_EDGE,
        BI_TYPE_EDGE,
        NA,
    };

    EIter() = default;

    ~EIter() { FreeIter(); }

    void FreeIter() {
        switch (_type) {
        case OUT_EDGE:
            delete _oeit;
            break;
        case IN_EDGE:
            delete _ieit;
            break;
        case TYPE_OUT_EDGE:
            delete _toeit;
            break;
        case TYPE_IN_EDGE:
            delete _tieit;
            break;
        case BI_EDGE:
            if (_is_out) {
                delete _oeit;
            } else {
                delete _ieit;
            }
            break;
        case BI_TYPE_EDGE:
            if (_is_out) {
                delete _toeit;
            } else {
                delete _tieit;
            }
            break;
        default:
            break;
        }
        _oeit = nullptr;
    }

    void Initialize(lgraph::Transaction *txn, IteratorType type, lgraph::VertexId vid,
                    const std::set<std::string> &relp_types, std::vector<cypher::Property> props) {
        FreeIter();
        _txn = txn;
        _type = type;
        _properties = std::move(props);
        switch (_type) {
        case OUT_EDGE:
            _oeit = new lgraph::graph::OutEdgeIterator(_txn->GetOutEdgeIterator(vid));
            break;
        case IN_EDGE:
            _ieit = new lgraph::graph::InEdgeIterator(_txn->GetInEdgeIterator(vid));
            break;
        case BI_EDGE:
            {
                auto it = _txn->GetOutEdgeIterator(vid);
                if (it.IsValid()) {
                    _oeit = new lgraph::graph::OutEdgeIterator(std::move(it));
                    _is_out = true;
                } else {
                    _ieit = new lgraph::graph::InEdgeIterator(_txn->GetInEdgeIterator(vid));
                    _is_out = false;
                }
                break;
            }
        case TYPE_OUT_EDGE:
            _toeit = new TypeEdgeIterator<lgraph::graph::OutEdgeIterator>(
                _txn, new lgraph::graph::OutEdgeIterator(_txn->GetOutEdgeIterator(vid)),
                relp_types);
            break;
        case TYPE_IN_EDGE:
            _tieit = new TypeEdgeIterator<lgraph::graph::InEdgeIterator>(
                _txn, new lgraph::graph::InEdgeIterator(_txn->GetInEdgeIterator(vid)), relp_types);
            break;
        case BI_TYPE_EDGE:
            {
                _toeit = new TypeEdgeIterator<lgraph::graph::OutEdgeIterator>(
                    _txn, new lgraph::graph::OutEdgeIterator(_txn->GetOutEdgeIterator(vid)),
                    relp_types);
                if (_toeit->IsValid()) {
                    _is_out = true;
                } else {
                    delete _toeit;
                    _tieit = new TypeEdgeIterator<lgraph::graph::InEdgeIterator>(
                        _txn, new lgraph::graph::InEdgeIterator(_txn->GetInEdgeIterator(vid)),
                        relp_types);
                    _is_out = false;
                }
                break;
            }
        default:
            throw lgraph::CypherException("EIter constructor type error.");
        }
        if (IsValid() && !MatchPropertyFilter()) {
            Next();
        }
    }

    void Initialize(lgraph::Transaction *txn, const lgraph::EdgeUid &euid) {
        FreeIter();
        _txn = txn;
        _type = OUT_EDGE;
        _oeit = new lgraph::graph::OutEdgeIterator(_txn->GetOutEdgeIterator(euid, false));
    }

    EIter(EIter &&rhs) noexcept {
        _type = rhs._type;
        _oeit = rhs._oeit;
        rhs._oeit = nullptr;
        _txn = rhs._txn;
        rhs._txn = nullptr;
        _is_out = rhs._is_out;
    }

    EIter &operator=(EIter &&rhs) = delete;

    bool operator==(const EIter &rhs) const {
        return IsValid() && rhs.IsValid() && GetUid() == rhs.GetUid();
    }

    bool operator!=(const EIter &rhs) const { return !(*this == rhs); }

    bool Directed() const { return _type < BI_EDGE; }

    bool Undirected() const { return _type >= BI_EDGE && _type < NA; }

    bool IsValid() const {
        switch (_type) {
        case OUT_EDGE:
            return (_oeit && _oeit->IsValid());
        case IN_EDGE:
            return (_ieit && _ieit->IsValid());
        case TYPE_OUT_EDGE:
            return (_toeit && _toeit->IsValid());
        case TYPE_IN_EDGE:
            return (_tieit && _tieit->IsValid());
        case BI_EDGE:
            return _is_out ? (_oeit && _oeit->IsValid()) : (_ieit && _ieit->IsValid());
        case BI_TYPE_EDGE:
            return _is_out ? (_toeit && _toeit->IsValid()) : (_tieit && _tieit->IsValid());
        default:
            return false;
        }
    }

    bool MatchPropertyFilter() {
        if (_properties.empty()) {
            return true;
        }
        auto fields = GetFields();
        std::unordered_map<std::string, FieldData> map;
        for (size_t i = 0; i < fields.size(); i++) {
            map.emplace(fields[i].first, std::move(fields[i].second));
        }
        bool match = true;
        for (auto &item : _properties) {
            auto iter = map.find(item.field);
            if (iter == map.end() || item.value != iter->second) {
                match = false;
                break;
            }
        }
        return match;
    }

    bool Next() {
        while (InternalNext()) {
            if (MatchPropertyFilter()) {
                return true;
            }
        }
        return false;
    }

    bool InternalNext() {
        switch (_type) {
        case OUT_EDGE:
            return (_oeit && _oeit->Next());
        case IN_EDGE:
            return (_ieit && _ieit->Next());
        case TYPE_OUT_EDGE:
            return (_toeit && _toeit->Next());
        case TYPE_IN_EDGE:
            return (_tieit && _tieit->Next());
        case BI_EDGE:
            if (_is_out) {
                CYPHER_THROW_ASSERT(_oeit && _oeit->IsValid());
                if (!_oeit) return false;
                auto id = _oeit->GetSrc();
                if (_oeit->Next()) return true;
                delete _oeit;
                _ieit = new lgraph::graph::InEdgeIterator(_txn->GetInEdgeIterator(id));
                _is_out = false;
                return _ieit->IsValid();
            } else {
                return (_ieit && _ieit->Next());
            }
        case BI_TYPE_EDGE:
            if (_is_out) {
                CYPHER_THROW_ASSERT(_toeit && _toeit->IsValid());
                if (!_toeit) return false;
                auto id = _toeit->GetSrc();
                if (_toeit->Next()) return true;
                /* NOTE: strictly speaking, the member methods should be called
                 * before _toeit becomes invalid. */
                auto types = _toeit->Types();
                delete _toeit;
                _tieit = new TypeEdgeIterator<lgraph::graph::InEdgeIterator>(
                    _txn, new lgraph::graph::InEdgeIterator(_txn->GetInEdgeIterator(id)), types);
                _is_out = false;
                return _tieit->IsValid();
            } else {
                return (_tieit && _tieit->Next());
            }
        default:
            return false;
        }
    }

    lgraph::EdgeUid GetUid() const {
        CYPHER_THROW_ASSERT(_CheckItPtr());
        switch (_type) {
        case OUT_EDGE:
            return _oeit->GetUid();
        case IN_EDGE:
            return _ieit->GetUid();
        case TYPE_OUT_EDGE:
            return _toeit->GetUid();
        case TYPE_IN_EDGE:
            return _tieit->GetUid();
        case BI_EDGE:
            return _is_out ? _oeit->GetUid() : _ieit->GetUid();
        case BI_TYPE_EDGE:
            return _is_out ? _toeit->GetUid() : _tieit->GetUid();
        default:
            return {};
        }
    }

    lgraph::LabelId GetLid() const {
        CYPHER_THROW_ASSERT(_CheckItPtr());
        switch (_type) {
        case OUT_EDGE:
            return _oeit->GetLabelId();
        case IN_EDGE:
            return _ieit->GetLabelId();
        case TYPE_OUT_EDGE:
            return _toeit->GetLabel();
        case TYPE_IN_EDGE:
            return _tieit->GetLabel();
        case BI_EDGE:
            return _is_out ? _oeit->GetLabelId() : _ieit->GetLabelId();
        case BI_TYPE_EDGE:
            return _is_out ? _toeit->GetLabel() : _tieit->GetLabel();
        default:
            return std::numeric_limits<lgraph::LabelId>::max();
        }
    }

    lgraph::EdgeId GetId() const {
        CYPHER_THROW_ASSERT(_CheckItPtr());
        switch (_type) {
        case OUT_EDGE:
            return _oeit->GetEdgeId();
        case IN_EDGE:
            return _ieit->GetEdgeId();
        case TYPE_OUT_EDGE:
            return _toeit->GetEdgeId();
        case TYPE_IN_EDGE:
            return _tieit->GetEdgeId();
        case BI_EDGE:
            return _is_out ? _oeit->GetEdgeId() : _ieit->GetEdgeId();
        case BI_TYPE_EDGE:
            return _is_out ? _toeit->GetEdgeId() : _tieit->GetEdgeId();
        default:
            return -1;
        }
    }

    lgraph::VertexId GetSrc() const {
        CYPHER_THROW_ASSERT(_CheckItPtr());
        switch (_type) {
        case OUT_EDGE:
            return _oeit->GetSrc();
        case IN_EDGE:
            return _ieit->GetSrc();
        case TYPE_OUT_EDGE:
            return _toeit->GetSrc();
        case TYPE_IN_EDGE:
            return _tieit->GetSrc();
        case BI_EDGE:
            return _is_out ? _oeit->GetSrc() : _ieit->GetSrc();
        case BI_TYPE_EDGE:
            return _is_out ? _toeit->GetSrc() : _tieit->GetSrc();
        default:
            return -1;
        }
    }

    lgraph::VertexId GetDst() const {
        CYPHER_THROW_ASSERT(_CheckItPtr());
        switch (_type) {
        case OUT_EDGE:
            return _oeit->GetDst();
        case IN_EDGE:
            return _ieit->GetDst();
        case TYPE_OUT_EDGE:
            return _toeit->GetDst();
        case TYPE_IN_EDGE:
            return _tieit->GetDst();
        case BI_EDGE:
            return _is_out ? _oeit->GetDst() : _ieit->GetDst();
        case BI_TYPE_EDGE:
            return _is_out ? _toeit->GetDst() : _tieit->GetDst();
        default:
            return -1;
        }
    }

    lgraph::VertexId GetNbr(cypher::ExpandTowards direction) const {
        switch (direction) {
        case cypher::FORWARD:
            CYPHER_THROW_ASSERT(Directed());
            return GetDst();
        case cypher::REVERSED:
            CYPHER_THROW_ASSERT(Directed());
            return GetSrc();
        case cypher::BIDIRECTIONAL:
            CYPHER_THROW_ASSERT(Undirected());
            if (_type == BI_EDGE) {
                return _is_out ? _oeit->GetDst() : _ieit->GetSrc();
            } else {
                return _is_out ? _toeit->GetDst() : _tieit->GetSrc();
            }
        default:
            return -1;
        }
    }

    int64_t HashId() const;

    FieldData GetField(const std::string &fd) {
        CYPHER_THROW_ASSERT(_CheckItPtr());
        switch (_type) {
        case OUT_EDGE:
            return _txn->GetEdgeField(*_oeit, fd);
        case IN_EDGE:
            return _txn->GetEdgeField(*_ieit, fd);
        case TYPE_OUT_EDGE:
            return _txn->GetEdgeField(*(_toeit->Eit()), fd);
        case TYPE_IN_EDGE:
            return _txn->GetEdgeField(*(_tieit->Eit()), fd);
        case BI_EDGE:
            return _is_out ? _txn->GetEdgeField(*_oeit, fd) : _txn->GetEdgeField(*_ieit, fd);
        case BI_TYPE_EDGE:
            return _is_out ? _txn->GetEdgeField(*(_toeit->Eit()), fd)
                           : _txn->GetEdgeField(*(_tieit->Eit()), fd);
        default:
            break;
        }
        return {};
    }

    std::string GetLabel() const {
        CYPHER_THROW_ASSERT(_CheckItPtr());
        switch (_type) {
        case OUT_EDGE:
            return _txn->GetEdgeLabel(*_oeit);
        case IN_EDGE:
            return _txn->GetEdgeLabel(*_ieit);
        case TYPE_OUT_EDGE:
            return _txn->GetEdgeLabel(*(_toeit->Eit()));
        case TYPE_IN_EDGE:
            return _txn->GetEdgeLabel(*(_tieit->Eit()));
        case BI_EDGE:
            return _is_out ? _txn->GetEdgeLabel(*_oeit) : _txn->GetEdgeLabel(*_ieit);
        case BI_TYPE_EDGE:
            return _is_out ? _txn->GetEdgeLabel(*(_toeit->Eit()))
                           : _txn->GetEdgeLabel(*(_tieit->Eit()));
        default:
            break;
        }
        return "";
    }

    std::vector<std::pair<std::string, FieldData>> GetFields() {
        CYPHER_THROW_ASSERT(_CheckItPtr());
        std::vector<std::pair<std::string, FieldData>> EMPTY_FIELDS;
        switch (_type) {
        case OUT_EDGE:
            return _txn->GetEdgeFields(*_oeit);
        case IN_EDGE:
            return _txn->GetEdgeFields(*_ieit);
        case TYPE_OUT_EDGE:
            return _txn->GetEdgeFields(*(_toeit->Eit()));
        case TYPE_IN_EDGE:
            return _txn->GetEdgeFields(*(_tieit->Eit()));
        case BI_EDGE:
            return _is_out ? _txn->GetEdgeFields(*_oeit) : _txn->GetEdgeFields(*_ieit);
        case BI_TYPE_EDGE:
            return _is_out ? _txn->GetEdgeFields(*(_toeit->Eit()))
                           : _txn->GetEdgeFields(*(_tieit->Eit()));
        default:
            break;
        }
        return EMPTY_FIELDS;
    }

    std::string Properties() const {
        CYPHER_THROW_ASSERT(_CheckItPtr());
        switch (_type) {
        case OUT_EDGE:
            return Properties(*_oeit);
        case IN_EDGE:
            return Properties(*_ieit);
        case TYPE_OUT_EDGE:
            return Properties(*(_toeit->Eit()));
        case TYPE_IN_EDGE:
            return Properties(*(_tieit->Eit()));
        case BI_EDGE:
            return _is_out ? Properties(*_oeit) : Properties(*_ieit);
        case BI_TYPE_EDGE:
            return _is_out ? Properties(*(_toeit->Eit())) : Properties(*(_tieit->Eit()));
        default:
            break;
        }
        return "";
    }

 private:
    IteratorType _type = NA;
    union {
        lgraph::graph::OutEdgeIterator *_oeit = nullptr;
        lgraph::graph::InEdgeIterator *_ieit;
        TypeEdgeIterator<lgraph::graph::OutEdgeIterator> *_toeit;
        TypeEdgeIterator<lgraph::graph::InEdgeIterator> *_tieit;
    };
    lgraph::Transaction *_txn = nullptr;
    bool _is_out;  // for undirected edge
    std::vector<cypher::Property> _properties;

    bool _CheckItPtr() const {
        switch (_type) {
        case OUT_EDGE:
            return _oeit;
        case IN_EDGE:
            return _ieit;
        case TYPE_OUT_EDGE:
            return _toeit;
        case TYPE_IN_EDGE:
            return _tieit;
        case BI_EDGE:
            return (_is_out && _oeit) || (!_is_out && _ieit);
        case BI_TYPE_EDGE:
            return (_is_out && _toeit) || (!_is_out && _tieit);
        default:
            return false;
        }
    }

    template <typename EIT>
    typename std::enable_if<IS_EIT_TYPE(EIT), std::string>::type Properties(const EIT &eit) const {
        std::string p;
        if (!eit.IsValid()) return p;
        p.append("{");
        // _LABEL_
        p.append("\"_LABEL_\":\"").append(_txn->GetEdgeLabel(eit)).append("\"");
        p.append(",");
        // _EID_
        p.append("\"_EID_\":\"").append(eit.GetUid().ToString()).append("\"");
        auto fields = _txn->GetEdgeFields(eit);
        for (int i = 0; i < (int)fields.size(); i++) {
            auto f = fields[i];
            p.append(",");
            p.append("\"").append(f.first).append("\":");
            if (f.second.IsString())
                p.append("\"").append(f.second.ToString()).append("\"");
            else
                p.append(f.second.ToString());
        }
        p.append("}");
        return p;
    }
};

struct EuidHash {
    size_t operator()(const lgraph::EdgeUid &e) const {
        auto h0 = std::hash<decltype(e.src)>{}(e.src);
        auto h1 = std::hash<decltype(e.dst)>{}(e.dst);
        auto h2 = std::hash<decltype(e.lid)>{}(e.lid);
        auto h3 = std::hash<decltype(e.eid)>{}(e.eid);
        return h0 ^ h1 ^ h2 ^ h3;
    }
};

struct EuidHashSet {
    typedef std::unordered_set<lgraph::EdgeUid, EuidHash> EuidSet;
    EuidSet euid_hash_set;

    bool Contains(const lgraph::EIter &eit) const {
        if (!eit.IsValid()) return false;
        auto it = euid_hash_set.find(eit.GetUid());
        return it != euid_hash_set.end();
    }

    std::pair<EuidSet::iterator, bool> Add(const lgraph::EIter &eit) {
        if (!eit.IsValid()) return std::make_pair(EuidSet::iterator(), false);
        return euid_hash_set.emplace(eit.GetUid());
    }

    EuidSet::iterator Erase(const EuidSet::iterator &it) { return euid_hash_set.erase(it); }

    bool Erase(const lgraph::EIter &eit) {
        if (!eit.IsValid()) return false;
        auto it = euid_hash_set.find(eit.GetUid());
        if (it == euid_hash_set.end()) return false;
        euid_hash_set.erase(it);
        return true;
    }

    std::string Dump() const {
        std::string line = "Current Visited Edges:\n";
        for (auto &e : euid_hash_set) {
            line.append("EUID<").append(cypher::_detail::EdgeUid2String(e)).append(">\n");
        }
        return line;
    }
};
}  // namespace lgraph
