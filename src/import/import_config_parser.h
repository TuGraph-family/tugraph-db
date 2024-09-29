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

#include <experimental/filesystem>
#include <utility>
#include <vector>

#include "fma-common/fma_stream.h"
#include "lgraph/lgraph_types.h"
#include "core/data_type.h"
#include "core/field_data_helper.h"
#include "core/lightning_graph.h"
#include "import/import_exception.h"

namespace lgraph {
namespace import_v2 {

static constexpr const size_t ONLINE_IMPORT_LIMIT_SOFT = 16ll << 20;
static constexpr const size_t ONLINE_IMPORT_LIMIT_HARD = 17ll << 20;

// the keywords for import tool
enum KeyWord {
    BOOL,
    INT8,
    INT16,
    INT32,
    INT64,
    FLOAT,
    DOUBLE,
    DATE,
    DATETIME,
    STRING,
    BLOB,
    POINT,
    LINESTRING,
    POLYGON,
    FLOAT_VECTOR,
    LABEL,  // VERTEX_LABEL or EDGE_LABEL
    VERTEX_LABEL,
    EDGE_LABEL,
    ID,  // = INDEX + UNIQUE
    INDEX,
    UNIQUE,
    SRC_ID,
    DST_ID,
    SKIP,
    HEADER,
    OPTIONAL_,  // OPTIONAL is a macro in windows SDK
    FORMAT,
    ASC,        // TemporalFieldOrder::ASC
    DESC        // TemporalFieldOrder::DESC
};

class KeyWordFunc {
    static const std::map<KeyWord, std::string>& KeyWordToStrMap() {
        static std::map<KeyWord, std::string> m = {
            {KeyWord::BOOL, "BOOL"},
            {KeyWord::INT8, "INT8"},
            {KeyWord::INT16, "INT16"},
            {KeyWord::INT32, "INT32"},
            {KeyWord::INT64, "INT64"},
            {KeyWord::FLOAT, "FLOAT"},
            {KeyWord::DOUBLE, "DOUBLE"},
            {KeyWord::DATE, "DATE"},
            {KeyWord::DATETIME, "DATETIME"},
            {KeyWord::STRING, "STRING"},
            {KeyWord::BLOB, "BLOB"},
            {KeyWord::POINT, "POINT"},
            {KeyWord::LINESTRING, "LINESTRING"},
            {KeyWord::POLYGON, "POLYGON"},
            {KeyWord::FLOAT_VECTOR, "FLOAT_VECTOR"},
            {KeyWord::LABEL, "LABEL"},
            {KeyWord::VERTEX_LABEL, "VERTEX_LABEL"},
            {KeyWord::EDGE_LABEL, "EDGE_LABEL"},
            {KeyWord::ID, "ID"},
            {KeyWord::INDEX, "INDEX"},
            {KeyWord::UNIQUE, "UNIQUE"},
            {KeyWord::SRC_ID, "SRC_ID"},
            {KeyWord::DST_ID, "DST_ID"},
            {KeyWord::SKIP, "SKIP"},
            {KeyWord::HEADER, "HEADER"},
            {KeyWord::OPTIONAL_, "OPTIONAL"},
            {KeyWord::FORMAT, "FORMAT"},
            {KeyWord::ASC, "ASC"},
            {KeyWord::DESC, "DESC"},
        };
        return m;
    }

    static const std::map<std::string, KeyWord>& StrToKeyWordMap() {
        static auto construct = []() {
            auto& k2s = KeyWordToStrMap();
            std::map<std::string, KeyWord> s2k;
            for (auto& kv : k2s) s2k[kv.second] = kv.first;
            return s2k;
        };
        static std::map<std::string, KeyWord> m = construct();
        return m;
    }

    static const std::map<KeyWord, FieldType>& KeyWordToFieldTypeMap() {
        static auto construct = []() {
            auto& k2s = KeyWordToStrMap();
            std::map<KeyWord, FieldType> ret;
            for (auto& kv : k2s) {
                if (kv.first > KeyWord::FLOAT_VECTOR) break;
                FieldType type = FieldType::NUL;
                if (!field_data_helper::TryGetFieldType(kv.second, type)) {
                    LOG_ERROR() << FMA_FMT("Keyword string [{}] is invalid type string", kv.second);
                }
                ret[kv.first] = type;
            }
            return ret;
        };

        static std::map<KeyWord, FieldType> m = construct();
        return m;
    }

    static const std::map<FieldType, KeyWord>& FieldTypeToKeyWordMap() {
        static auto construct = []() {
            std::map<FieldType, KeyWord> ret;
            for (auto& k2ft : KeyWordToFieldTypeMap()) {
                ret[k2ft.second] = k2ft.first;
            }
            return ret;
        };

        static std::map<FieldType, KeyWord> m = construct();
        return m;
    }

 public:
    static KeyWord GetKeyWordFromStr(const std::string& s) {
        const std::map<std::string, KeyWord>& map = StrToKeyWordMap();
        auto it = map.find(s);
        if (it == map.end()) {
            throw std::runtime_error("unknown keyword str: [" + s + "]");
        }
        return it->second;
    }

    static KeyWord GetKeyWordFromFieldType(const FieldType& ft) {
        const std::map<FieldType, KeyWord>& map = FieldTypeToKeyWordMap();
        auto it = map.find(ft);
        if (it == map.end()) {
            throw std::runtime_error(FMA_FMT("unknown field type [{}]", ft));
        }
        return it->second;
    }

    static FieldType GetFieldTypeFromKeyWord(const KeyWord& kw) {
        const std::map<KeyWord, FieldType>& map = KeyWordToFieldTypeMap();
        auto it = map.find(kw);
        if (it == map.end())
            throw std::runtime_error(
                FMA_FMT("keyword [{}] is not a FieldType", KeyWordToStrMap().at(kw)));
        return it->second;
    }

    static FieldType GetFieldTypeFromStr(const std::string& s) {
        KeyWord kw = GetKeyWordFromStr(s);
        FieldType ft = GetFieldTypeFromKeyWord(kw);
        return ft;
    }

    static TemporalFieldOrder GetTemporalFieldOrderFromStr(const std::string& s) {
        KeyWord kw = GetKeyWordFromStr(s);
        if (kw == KeyWord::ASC) {
            return TemporalFieldOrder::ASC;
        } else if (kw == KeyWord::DESC) {
            return TemporalFieldOrder::DESC;
        } else {
            throw std::runtime_error(FMA_FMT("keyword [{}] is not a TemporalFieldOrder", s));
        }
    }

    static const std::string& GetStrFromKeyWord(const KeyWord& kw) {
        return KeyWordToStrMap().at(kw);
    }

    static const std::string& GetStrFromFieldType(const FieldType& ft) {
        return GetStrFromKeyWord(GetKeyWordFromFieldType(ft));
    }
};

struct ColumnSpec {
    std::string name;                 // name of the field, if not skip
    FieldType type = FieldType::NUL;  // type of the data
    bool optional = false;
    bool index = false;
    IndexType idxType = IndexType::NonuniqueIndex;
    bool primary = false;
    bool temporal = false;
    TemporalFieldOrder temporal_order = TemporalFieldOrder::ASC;
    bool fulltext = false;

    inline bool CheckValid() const {
        if (primary && !(index && idxType == IndexType::GlobalUniqueIndex)) THROW_CODE(InputError,
            "primary {} should be index and unique", name);
        if ((idxType == IndexType::GlobalUniqueIndex || idxType == IndexType::PairUniqueIndex)
            && optional) THROW_CODE(InputError,
            FMA_FMT("unique/primary index {} should not be optional", name));
        if (type == FieldType ::BLOB && index) THROW_CODE(InputError,
            FMA_FMT("BLOB field {} should not be indexed", name));
        if (type != FieldType::STRING && fulltext) THROW_CODE(InputError,
            FMA_FMT("fulltext index {} only supports STRING type", name));
        if (type != FieldType::INT64 && temporal) THROW_CODE(InputError,
            FMA_FMT("edge label [{}] temporal field [{}] should be INT64", name));
        return true;
    }

    ColumnSpec()
        : name(""),
          type(FieldType::NUL),
          optional(false),
          index(false),
          idxType(IndexType::NonuniqueIndex),
          primary(false),
          temporal(false),
          temporal_order(TemporalFieldOrder::ASC),
          fulltext(false) {}
    ColumnSpec(std::string name_, FieldType type_, bool is_id_, bool optional_ = false,
               bool index_ = false, IndexType idxType_ = IndexType::NonuniqueIndex,
               bool temporal_ = false, TemporalFieldOrder temporal_order_ = TemporalFieldOrder::ASC,
               bool fulltext_ = false)
        : name(std::move(name_)),
          type(type_),
          optional(optional_),
          index(index_),
          idxType(idxType_),
          primary(is_id_),
          temporal(temporal_),
          temporal_order(temporal_order_),
          fulltext(fulltext_) {}

    bool operator==(const ColumnSpec& rhs) const {
        // do not compare is_id/skip
        return name == rhs.name && type == rhs.type && optional == rhs.optional &&
               index == rhs.index && idxType == rhs.idxType &&
               temporal == rhs.temporal && fulltext == rhs.fulltext;
    }

    bool operator<(const ColumnSpec& rhs) const { return name < rhs.name; }

    FieldSpec GetFieldSpec() const {
        FieldSpec fs(name, type, optional);
        return fs;
    }

    std::string ToString() const {
        nlohmann::json conf;
        conf["name"] = this->name;
        conf["primary"] = this->primary;
        conf["index"] = this->index;
        conf["temporal"] = this->temporal;
        conf["optional"] = this->optional;
        switch (idxType) {
        case IndexType::GlobalUniqueIndex:
            conf["indexType"] = "GlobalUniqueIndex";
            break;
        case IndexType::PairUniqueIndex:
            conf["indexType"] = "PairUniqueIndex";
            break;
        case IndexType::NonuniqueIndex:
            conf["indexType"] = "NonuniqueIndex";
            break;
        }
        return conf.dump(4);
    }

    template <class StreamT>
    void DumpTo(StreamT& out) const {
        out << this->ToString();
    }
};

struct LabelDesc {
    std::string name;
    std::vector<ColumnSpec> columns;
    EdgeConstraints edge_constraints;
    bool is_vertex;
    bool detach_property{true};
    LabelDesc() {}
    std::string ToString() const {
        std::string prefix = is_vertex ? "vertex" : "edge";
        return FMA_FMT("label type:[{}] name:[{}], columns:[{}]", prefix, name, columns);
    }

    template <class StreamT>
    void DumpTo(StreamT& out) const {
        if (is_vertex)
            out << KeyWordFunc::GetStrFromKeyWord(KeyWord::VERTEX_LABEL) << '=' << name;
        else
            out << KeyWordFunc::GetStrFromKeyWord(KeyWord::EDGE_LABEL) << '=' << name;

        for (size_t i = 0; i < columns.size(); ++i) {
            if (i) out << ',';
            columns[i].DumpTo(out);
        }
    }

    bool MeetEdgeConstraints(const std::string& src, const std::string& dst) {
        if (edge_constraints.empty()) {
            return true;
        }
        for (auto& item : edge_constraints) {
            if (item.first == src && item.second == dst) {
                return true;
            }
        }
        return false;
    }

    bool IsVertex() const { return is_vertex; }

    size_t FindIdx(std::string field_name) const {
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i].name == field_name) return i;
        }
        throw std::runtime_error(
            FMA_FMT("field name [{}] not found in label [{}]", field_name, name));
    }

    std::vector<ColumnSpec> GetColumnSpecs() const {
        return columns;
    }

    ColumnSpec Find(std::string field_name) const {
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i].name == field_name) return columns[i];
        }
        throw std::runtime_error(
            FMA_FMT("field name [{}] not found in label [{}]", field_name, name));
    }

    ColumnSpec GetPrimaryField() const {
        if (!is_vertex) throw std::runtime_error("No primary column found");
        for (auto it = columns.begin(); it != columns.end(); ++it) {
            if (it->primary) return *it;
        }
        throw std::runtime_error("No primary column found");
    }

    bool HasPrimaryField() const {
        if (!is_vertex) return false;
        bool ret = false;
        for (auto it = columns.begin(); it != columns.end(); ++it) {
            if (it->primary) return true;
        }
        return ret;
    }

    ColumnSpec GetTemporalField() const {
        if (is_vertex) throw std::runtime_error("No temporal column found");
        for (auto it = columns.begin(); it != columns.end(); ++it) {
            if (it->temporal) return *it;
        }
        throw std::runtime_error("No temporal column found");
    }

    bool HasTemporalField() const {
        if (is_vertex) return false;
        bool ret = false;
        for (auto it = columns.begin(); it != columns.end(); ++it) {
            if (it->temporal) return true;
        }
        return ret;
    }

    std::vector<FieldSpec> GetFieldSpecs(std::vector<std::string>& names) const {
        std::vector<FieldSpec> fs;
        for (const auto& n : names) {
            fs.push_back(this->GetFieldSpec(n));
        }
        return fs;
    }

    std::vector<FieldSpec> GetAllFieldSpecs() const {
        std::vector<FieldSpec> fs;
        for (const auto& spec : columns) {
            fs.push_back(spec.GetFieldSpec());
        }
        return fs;
    }

    FieldSpec GetFieldSpec(std::string name) const {
        ColumnSpec cs = Find(name);
        return cs.GetFieldSpec();
    }

    bool operator==(const LabelDesc& rhs) const {
        if (!(name == rhs.name && is_vertex == rhs.is_vertex &&
              columns.size() == rhs.columns.size()))
            return false;

        std::vector<ColumnSpec> x = columns;
        std::vector<ColumnSpec> y = rhs.columns;
        std::sort(x.begin(), x.end());
        std::sort(y.begin(), y.end());

        return x == y;
    }

    std::map<std::string, lgraph::FieldSpec> GetSchemaDef() const {
        std::map<std::string, FieldSpec> ret;
        for (size_t i = 0; i < columns.size(); i++) {
            auto& c = columns[i];
            // at this point, there should not be any column with empty name
            FMA_DBG_ASSERT(!c.name.empty());
            ret[c.name] = FieldSpec(c.name, c.type, c.optional);
        }
        return ret;
    }

    bool CheckValidId(std::string field_name) const {
        if (!is_vertex) {
            LOG_INFO() << "edge label does not have primary";
            return true;
        }
        for (auto it = columns.begin(); it != columns.end(); ++it) {
            if (it->name == field_name) {
                if (it->primary)
                    return true;
                else
                    THROW_CODE(InputError,
                        "[{}] is not primary field of label [{}]", field_name, name);
            }
        }
        THROW_CODE(InputError, "[{}] is not valid field name of label [{}]", field_name, name);
        return false;
    }

    bool CheckValid() const {
        bool find_primary = false;
        for (auto it = columns.begin(); it != columns.end(); ++it) {
            it->CheckValid();
            if (it->primary && is_vertex) {
                if (find_primary) {
                    THROW_CODE(InputError,
                        "vertex label [{}] should has no more than one primary.", name);
                }
                find_primary = true;
            }
        }
        if (is_vertex && !find_primary) {
            THROW_CODE(InputError, "vertex label [{}] has no primary", name);
        }
        if (is_vertex && !edge_constraints.empty()) {
            THROW_CODE(InputError,
                "label [{}] type is vertex, but edge_constraints is not empty", name);
        }
        return true;
    }
};

struct CsvDesc {
    struct EdgeIDField {
        std::string label;
        std::string id;

        EdgeIDField() {}
        EdgeIDField(std::string label_, std::string id_) : label(label_), id(id_) {}
    };

    CsvDesc() {}
    explicit CsvDesc(const std::string& path_) : path(path_) {}

    std::string path;
    std::string data_format = "CSV";
    bool is_vertex_file = false;
    size_t size = 0;
    size_t n_header_line = 0;

    std::string label;
    EdgeIDField edge_src;
    EdgeIDField edge_dst;

    std::vector<std::string> columns;

    size_t FindIdx(std::string s) const {
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i] == s) return i;
        }
        throw std::runtime_error(FMA_FMT("No id name [{}] found", s));
    }

    size_t FindIdxExcludeSkip(std::string s) const {
        size_t pos = 0;
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i] == KeyWordFunc::GetStrFromKeyWord(KeyWord::SKIP)) {
                continue;
            }
            if (columns[i] == s) {
                return pos;
            }
            pos++;
        }
        throw std::runtime_error(FMA_FMT("No id name [{}] found", s));
    }

    bool CheckValid() const {
        bool find_src_id = false;
        bool find_dst_id = false;
        std::set<std::string> set;
        for (auto& col : columns) {
            if (col.empty()) throw std::runtime_error("column field cannot be empty");
            if (col == KeyWordFunc::GetStrFromKeyWord(KeyWord::SKIP)) {
                continue;
            } else if (col == KeyWordFunc::GetStrFromKeyWord(KeyWord::SRC_ID)) {
                if (find_src_id) throw std::runtime_error("SRC_ID defined more than once");
                find_src_id = true;
            } else if (col == KeyWordFunc::GetStrFromKeyWord(KeyWord::DST_ID)) {
                if (find_dst_id) throw std::runtime_error("DST_ID defined more than once");
                find_dst_id = true;
            } else {
                if (set.find(col) != set.end())
                    throw std::runtime_error(FMA_FMT("field [{}] appear more than once", col));
                set.insert(col);
            }
        }
        if (!find_src_id && !is_vertex_file)
            throw std::runtime_error("SRC_ID should be defined in edge file");
        if (!find_dst_id && !is_vertex_file)
            throw std::runtime_error("DST_ID should be defined in edge file");
        return true;
    }

    size_t GetEdgeSrcColumnIDExcludeSkip() const {
        bool find = false;
        size_t pos = 0;
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i] == KeyWordFunc::GetStrFromKeyWord(KeyWord::SKIP)) {
                continue;
            }
            if (columns[i] == KeyWordFunc::GetStrFromKeyWord(KeyWord::SRC_ID)) {
                find = true;
                break;
            }
            pos++;
        }
        if (!find) throw std::runtime_error("SRD_ID not found in columns");
        return pos;
    }

    size_t GetEdgeDstColumnIDExcludeSkip() const {
        bool find = false;
        size_t pos = 0;
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i] == KeyWordFunc::GetStrFromKeyWord(KeyWord::SKIP)) {
                continue;
            }
            if (columns[i] == KeyWordFunc::GetStrFromKeyWord(KeyWord::DST_ID)) {
                find = true;
                break;
            }
            pos++;
        }
        if (!find) throw std::runtime_error("SRD_ID not found in columns");
        return pos;
    }

    bool IsSkip(size_t pos) const {
        if (pos >= columns.size()) throw std::runtime_error(FMA_FMT("pos [{}] out of range", pos));
        return columns[pos] == KeyWordFunc::GetStrFromKeyWord(KeyWord::SKIP);
    }

    void GenFieldSpecs(Transaction& txn, std::vector<FieldSpec>& field_specs) const {
        std::string src_str = KeyWordFunc::GetStrFromKeyWord(KeyWord::SRC_ID);
        std::string dst_str = KeyWordFunc::GetStrFromKeyWord(KeyWord::DST_ID);
        std::string skip_str = KeyWordFunc::GetStrFromKeyWord(KeyWord::SKIP);

        // edge src/dst column
        FieldSpec src_spec;
        FieldSpec dst_spec;
        // FieldSpec skip_spec(skip_str, FieldType::NUL, true);
        FieldSpec skip_spec("", FieldType::NUL, true);
        if (!is_vertex_file) {
            src_spec = GetFieldSpec(txn.GetSchema(true, edge_src.label), edge_src.id);
            src_spec.name = src_str;

            dst_spec = GetFieldSpec(txn.GetSchema(true, edge_dst.label), edge_dst.id);
            dst_spec.name = dst_str;
        }

        // vertex/edge field column
        LabelDesc ld;
        auto field_specs_candidate = txn.GetSchema(is_vertex_file, label);
        for (const auto& name : columns) {
            if (name == skip_str) {
                field_specs.push_back(skip_spec);
            } else if (name == src_str) {
                field_specs.push_back(src_spec);
            } else if (name == dst_str) {
                field_specs.push_back(dst_spec);
            } else {
                auto fs = GetFieldSpec(field_specs_candidate, name);
                field_specs.push_back(fs);
            }
        }
    }

    std::string ToString() const {
        return fma_common::StringFormatter::Format("path={}, is_vertex_file={}, size={}", path,
                                                   is_vertex_file, size);
    }

    std::string Dump(bool has_path = false) const {
        nlohmann::json conf;
        if (has_path) {
            conf["files"][0]["path"] = this->path;
        }
        conf["files"][0]["header"] = this->n_header_line;
        conf["files"][0]["format"] = this->data_format;
        conf["files"][0]["label"] = this->label;
        conf["files"][0]["columns"] = this->columns;
        if (!is_vertex_file) {
            conf["files"][0]["SRC_ID"] = this->edge_src.label;
            conf["files"][0]["DST_ID"] = this->edge_dst.label;
        }
        return conf.dump(4);
    }

 private:
    FieldSpec GetFieldSpec(const std::vector<FieldSpec>& field_specs,
                           const std::string& name) const {
        for (const auto& fs : field_specs) {
            if (fs.name == name) {
                return fs;
            }
        }
        throw std::runtime_error(FMA_FMT("field [{}] not found in FieldSpec vector", name));
    }
};

struct SchemaDesc {
    SchemaDesc() = default;

    std::vector<LabelDesc> label_desc;

    size_t Size() const { return label_desc.size(); }

    LabelDesc FindVertexLabel(const std::string& label) const {
        for (const auto& it : label_desc)
            if (it.is_vertex && it.name == label) {
                return it;
            }
        throw std::runtime_error(FMA_FMT("vertex label [{}] not found in schema", label));
    }

    LabelDesc FindEdgeLabel(const std::string& label) const {
        for (const auto& it : label_desc)
            if (!it.is_vertex && it.name == label) {
                return it;
            }
        throw std::runtime_error(FMA_FMT("edge label [{}] not found in schema", label));
    }

    std::string ToString() const {
        std::string s = "Graph Schema: ";
        for (const auto& it : label_desc) s.append("  " + it.ToString() + "\n");
        return s;
    }

    std::string Dump() const {
        std::stringstream out;
        for (const auto& ld : label_desc) {
            ld.DumpTo(out);
            out << "\n";
        }
        return out.str();
    }

    bool CheckValid() {
        std::unordered_set<std::string> set_vertex, set_edge;
        for (const auto& ld : label_desc) {
            ld.CheckValid();
            if (ld.IsVertex()) {
                if (set_vertex.find(ld.name) != set_vertex.end())
                    THROW_CODE(InputError, "Vertex label [{}] appears more than once!", ld.name);
                set_vertex.insert(ld.name);
            } else {
                if (set_edge.find(ld.name) != set_edge.end())
                    THROW_CODE(InputError, "Edge label [{}] appears more than once!", ld.name);
                set_edge.insert(ld.name);
            }
        }
        return true;
    }

    void Clear() { label_desc.clear(); }

    SchemaDesc DifferenceSet(const SchemaDesc& rhs) {
        SchemaDesc schema;
        for (auto& ld : this->label_desc) {
            LabelDesc x;
            bool find = false;
            for (const auto& rld : rhs.label_desc)
                if (ld.name == rld.name && ld.is_vertex == rld.is_vertex) {
                    x = rld;
                    find = true;
                    break;
                }
            if (!find) {
                // not find, add to result
                schema.label_desc.push_back(ld);
            } else {
                // find, check if equal
                if (!(x == ld)) {
                    THROW_CODE(InputError, "{} and {} not comparable in two schema", x.ToString(),
                                             ld.ToString());
                }
            }
        }
        return schema;
    }

    void ConstructFromDB(Transaction& txn) {
        for (const bool& is_vertex : {false, true}) {
            auto labels = txn.GetAllLabels(is_vertex);
            std::string prefix = is_vertex ? "vertex" : "edge";
            LOG_INFO() << prefix + " label size: " << labels.size();
            for (const auto& name : labels) {
                LabelDesc ld;
                {
                    // fields
                    ld.name = name;
                    ld.is_vertex = is_vertex;
                    for (const auto& fs : txn.GetSchema(is_vertex, name)) {
                        ColumnSpec spec(fs.name, fs.type, false, fs.optional);
                        ld.columns.push_back(spec);
                    }
                }
                if (is_vertex) {
                    // index
                    const auto& primary = txn.GetVertexPrimaryField(name);
                    FMA_ASSERT(!primary.empty());
                    auto indexes = txn.ListVertexIndexByLabel(name);
                    for (const auto& index : indexes) {
                        size_t col = ld.FindIdx(index.field);
                        ld.columns[col].index = true;
                        ld.columns[col].idxType = index.type;
                        if (index.field == primary) ld.columns[col].primary = true;
                    }
                } else {
                    auto edge_indexes = txn.ListEdgeIndexByLabel(name);
                    for (const auto& index : edge_indexes) {
                        size_t col = ld.FindIdx(index.field);
                        ld.columns[col].index = true;
                        ld.columns[col].idxType = index.type;
                    }
                    ld.edge_constraints = txn.GetEdgeConstraints(name);
                }
                ld.CheckValid();
                // LOG_INFO() << "construct label: " << ld.name;
                label_desc.push_back(ld);
            }
        }
        this->CheckValid();
    }
};

class ImportConfParser {
 public:
    static SchemaDesc ParseSchema(const nlohmann::json& conf) {
        SchemaDesc sd;
        if (!conf.contains("schema")) {
            THROW_CODE(InputError, "Missing `schema` definition");
        }
        if (!conf["schema"].is_array()) {
            THROW_CODE(InputError, "\"schema\" is not array");
        }
        for (auto& s : conf["schema"]) {
            LabelDesc ld;
            if (!s.contains("label") || !s.contains("type")) {
                THROW_CODE(InputError, R"(Missing "label" or "type" definition)");
            }
            ld.name = s["label"];
            ld.is_vertex = s["type"] == "VERTEX";
            if (ld.is_vertex) {
                if (!s.contains("primary") || !s.contains("properties")) {
                    THROW_CODE(InputError,
                        R"(Label[{}]: Missing "primary" or "properties" definition)", ld.name);
                }
                if (s.contains("temporal")) {
                    THROW_CODE(InputError,
                        R"(Label[{}]: "temporal" is not supported in Vertex)", ld.name);
                }
                for (auto & p : s["properties"]) {
                    if (p.contains("pair_unique")) {
                        THROW_CODE(InputError,
                            R"(Label[{}]: "pair_unique index" is not supported in Vertex)", ld.name);
                    }
                }
            } else {
                if (s.contains("primary")) {
                    THROW_CODE(InputError,
                        R"(Label[{}]: "primary" is not supported in Edge)", ld.name);
                }
                if (s.contains("constraints")) {
                    if (!s["constraints"].is_array())
                        THROW_CODE(InputError,
                            R"(Label[{}]: "constraints" is not array)", ld.name);
                    for (auto& p : s["constraints"]) {
                        if (!p.is_array() || p.size() != 2)
                            THROW_CODE(InputError,
                                R"(Label[{}]: "constraints" element size should be 2)", ld.name);
                        ld.edge_constraints.emplace_back(p[0], p[1]);
                    }
                }
                if (s.contains("properties")) {
                    for (auto& p : s["properties"]) {
                        if (p.contains("pair_unique") && p["pair_unique"]
                            && p.contains("unique") && p["unique"]) {
                            THROW_CODE(InputError,
                                "Label[{}]: pair_unique and unique configuration cannot"
                                        " occur simultaneously)",
                                        ld.name);
                        }
                    }
                }
            }
            if (s.contains("properties")) {
                if (!s["properties"].is_array()) {
                    THROW_CODE(InputError, "Label[{}]: \"properties\" is not array", ld.name);
                }
                for (auto& p : s["properties"]) {
                    ColumnSpec cs;
                    if (!p.contains("name") || !p.contains("type")) {
                        THROW_CODE(InputError,
                            R"(Label[{}]: Missing "name" or "type" in "properties" definition)",
                            ld.name);
                    }
                    if (p["name"] == KeyWordFunc::GetStrFromKeyWord(KeyWord::SKIP) ||
                        p["name"] == KeyWordFunc::GetStrFromKeyWord(KeyWord::SRC_ID) ||
                        p["name"] == KeyWordFunc::GetStrFromKeyWord(KeyWord::DST_ID)) {
                        THROW_CODE(InputError,
                            R"(Label[{}]: Property name cannot be "SKIP" or "SRC_ID" or "DST_ID")",
                            ld.name);
                    }
                    cs.name = p["name"];
                    cs.type = KeyWordFunc::GetFieldTypeFromStr(p["type"]);
                    if (s.contains("primary") && cs.name == s["primary"]) {
                        cs.primary = true;
                        if (ld.is_vertex) {
                            cs.idxType = IndexType::GlobalUniqueIndex;
                            cs.index = true;
                        }
                    }
                    if (s.contains("temporal") && cs.name == s["temporal"]) {
                        cs.temporal = true;
                        if (s.contains("temporal_order")) {
                            cs.temporal_order =
                                KeyWordFunc::GetTemporalFieldOrderFromStr(s["temporal_order"]);
                        } else {
                            cs.temporal_order = TemporalFieldOrder::ASC;
                        }
                    }
                    if (p.contains("index")) {
                        cs.index = p["index"];
                    }
                    if (p.contains("unique")) {
                        if (p["unique"]) {
                            cs.idxType = IndexType::GlobalUniqueIndex;
                        } else {
                            cs.idxType = IndexType::NonuniqueIndex;
                        }
                    }
                    if (p.contains("pair_unique")) {
                        if (p["pair_unique"]) {
                            cs.idxType = IndexType::PairUniqueIndex;
                        } else {
                            cs.idxType = IndexType::NonuniqueIndex;
                        }
                    }
                    if (p.contains("optional")) {
                        cs.optional = p["optional"];
                    }
                    if (p.contains("fulltext")) {
                        cs.fulltext = p["fulltext"];
                    }

                    ld.columns.push_back(cs);
                }
            }
            if (s.contains("detach_property")) {
                if (!s["detach_property"].is_boolean()) {
                    THROW_CODE(InputError,
                        "Label[{}]: \"detach_property\" is not boolean", ld.name);
                }
                ld.detach_property = s["detach_property"];
            }
            sd.label_desc.push_back(ld);
        }
        sd.CheckValid();
        return sd;
    }

    static std::vector<CsvDesc> ParseFiles(const nlohmann::json& conf, bool has_path = true) {
        namespace fs = std::experimental::filesystem;
        std::vector<CsvDesc> cds;
        if (!conf.contains("files")) {
            return cds;
        }
        if (!conf["files"].is_array()) {
            THROW_CODE(InputError, "\"files\" is not array");
        }
        for (auto& item : conf["files"]) {
            if (!item.contains("format") || !item.contains("label") || !item.contains("columns")) {
                THROW_CODE(InputError,
                    R"(Missing "path" or "format" or "label" or "columns" in json {})",
                            item.dump(4));
            }
            if (!item["columns"].is_array()) {
                THROW_CODE(InputError, "\"columns\" is not array in json {}", item.dump(4));
            }

            std::vector<std::string> files;
            if (has_path) {
                if (!item.contains("path"))
                    THROW_CODE(InputError, R"(Missing "path" in json {})", item.dump(4));
                const std::string& path = item["path"];
                if (!fs::exists(path)) {
                    THROW_CODE(InputError,
                        "Path [{}] does not exist in json {}", path, item.dump(4));
                }
                if (fs::is_directory(path)) {
                    for (const auto& entry : fs::directory_iterator(path)) {
                        if (!fs::is_regular_file(entry.path())) continue;
#ifdef _MSC_VER
                        files.push_back(entry.path().string());
#else
                        files.push_back(entry.path());
#endif
                    }
                } else {
                    files.push_back(path);
                }
            } else {
                files.push_back("");
            }

            for (auto& file : files) {
                CsvDesc cd;
                if (!file.empty()) {
                    cd.path = file;
                    cd.size = fs::file_size(file);
                }
                cd.data_format = item["format"];
                if (cd.data_format != "CSV" && cd.data_format != "JSON") {
                    THROW_CODE(InputError,
                        "\"format\" value error : {}, should be CSV or JSON in json {}",
                                cd.data_format, item.dump(4));
                }
                cd.label = item["label"];
                if (item.contains("header")) {
                    cd.n_header_line = item["header"];
                }
                cd.is_vertex_file = !(item.contains("SRC_ID") || item.contains("DST_ID"));
                if (!cd.is_vertex_file) {
                    if (!item.contains("SRC_ID") || !item.contains("DST_ID")) {
                        THROW_CODE(InputError,
                            R"(Missing "SRC_ID" or "DST_ID" in json {})", item.dump(4));
                    }
                    cd.edge_src.label = item["SRC_ID"];
                    cd.edge_dst.label = item["DST_ID"];
                }
                for (auto& column : item["columns"]) {
                    const auto& data = column.get<std::string>();
                    if (data.empty()) {
                        std::throw_with_nested(std::runtime_error("Found empty filed in json " +
                                                                  item["columns"].dump(4)));
                    }
                    cd.columns.push_back(data);
                }
                cd.CheckValid();
                cds.push_back(cd);
            }
        }
        return cds;
    }

    static bool CheckConsistent(const SchemaDesc& sd, const CsvDesc& fd) {
        std::string label = fd.label;
        LabelDesc ld = fd.is_vertex_file ? sd.FindVertexLabel(label) : sd.FindEdgeLabel(label);
        std::vector<size_t> column_ids;

        // check field name in file exists and only once
        for (const auto& column : fd.columns) {
            if (column != KeyWordFunc::GetStrFromKeyWord(KeyWord::SKIP) &&
                column != KeyWordFunc::GetStrFromKeyWord(KeyWord::SRC_ID) &&
                column != KeyWordFunc::GetStrFromKeyWord(KeyWord::DST_ID))
                column_ids.push_back(ld.FindIdx(column));
        }
        std::sort(column_ids.begin(), column_ids.end());
        for (size_t i = 1; i < column_ids.size(); ++i) {
            if (column_ids[i] == column_ids[i - 1])
                throw std::runtime_error("same column name exists");
        }

        // all fields should be defined except optional
        std::set<std::string> set;
        for (const auto& column : ld.columns) {
            if (!column.optional) {
                set.insert(column.name);
            }
        }
        for (const auto& column : fd.columns) {
            if (set.find(column) != set.end()) {
                set.erase(column);
            }
        }
        if (!set.empty()) {
            std::string err;
            for (const auto& v : set) err.append(v + ",");
            THROW_CODE(InputError,
                "All fields (expect optional) should be defined, "
                        "[{}] not defined",
                        err);
        }

        // check edge SRC_ID and DST_ID valid
        if (!fd.is_vertex_file) {
            LabelDesc src_label_desc = sd.FindVertexLabel(fd.edge_src.label);
            LabelDesc dst_label_desc = sd.FindVertexLabel(fd.edge_dst.label);
            src_label_desc.CheckValidId(fd.edge_src.id);  // currently must be id
            dst_label_desc.CheckValidId(fd.edge_dst.id);  // currently must be id
        }
        return true;
    }
};

namespace comp {

struct ColumnSpec {
    bool skip = false;                // should we skip this column?
    std::string name;                 // name of the field, if not skip
    FieldType type = FieldType::NUL;  // type of the data
    bool optional = false;            // is this field optional?
    bool cannot_be_empty = false;     // ID, SRC_ID, and DST_ID cannot be empty
    bool index = false;
    bool unique = false;

    inline std::string DTToKeyWord(FieldType dt) const {
        KeyWord kw;
        switch (dt) {
        case FieldType::BOOL:
            kw = KeyWord::BOOL;
            break;
        case FieldType::INT8:
            kw = KeyWord::INT8;
            break;
        case FieldType::INT16:
            kw = KeyWord::INT16;
            break;
        case FieldType::INT32:
            kw = KeyWord::INT32;
            break;
        case FieldType::INT64:
            kw = KeyWord::INT64;
            break;
        case FieldType::FLOAT:
            kw = KeyWord::FLOAT;
            break;
        case FieldType::DOUBLE:
            kw = KeyWord::DOUBLE;
            break;
        case FieldType::DATE:
            kw = KeyWord::DATE;
            break;
        case FieldType::DATETIME:
            kw = KeyWord::DATETIME;
            break;
        case FieldType::STRING:
            kw = KeyWord::STRING;
            break;
        case FieldType::BLOB:
            kw = KeyWord::BLOB;
            break;
        default:
            throw std::runtime_error(
                fma_common::StringFormatter::Format("unexpected data type: [{}]", (int)dt));
        }
        return KeyWordFunc::GetStrFromKeyWord(kw);
    }

    ColumnSpec() {}
    ColumnSpec(bool skip_, const std::string& name_, FieldType type_, bool opt,
               bool not_empty = false)
        : skip(skip_), name(name_), type(type_), optional(opt), cannot_be_empty(not_empty) {}

    bool operator==(const ColumnSpec& rhs) const {
        return skip == rhs.skip && name == rhs.name && type == rhs.type &&
               optional == rhs.optional && cannot_be_empty == rhs.cannot_be_empty;
    }

    std::string ToString() const {
        if (skip) return KeyWordFunc::GetStrFromKeyWord(KeyWord::SKIP);
        std::string ret;
        ret.append(name).append(" ").append(field_data_helper::FieldTypeName(type));
        if (optional) ret.append(" ").append(KeyWordFunc::GetStrFromKeyWord(KeyWord::OPTIONAL_));
        return ret;
    }

    template <class StreamT>
    void DumpTo(StreamT& out) const {
        if (skip) {
            out << KeyWordFunc::GetStrFromKeyWord(KeyWord::SKIP);
            return;
        }
        out << name << ':' << DTToKeyWord(type);
        if (optional) out << ':' << KeyWordFunc::GetStrFromKeyWord(KeyWord::OPTIONAL_);
    }
};

struct VertexFileDesc {
    std::string label;
    std::vector<ColumnSpec> columns;
    size_t key_column;

    std::string ToString() const {
        return fma_common::StringFormatter::Format(
            "[vertex file, label:{}, key_column:{}, columns:{}]", label, key_column, columns);
    }

    template <class StreamT>
    void DumpTo(StreamT& out) const {
        out << KeyWordFunc::GetStrFromKeyWord(KeyWord::LABEL) << '=' << label << '\n';

        for (size_t i = 0; i < columns.size(); i++) {
            if (i) out << ',';
            columns[i].DumpTo(out);
            if (i == key_column) out << ':' << KeyWordFunc::GetStrFromKeyWord(KeyWord::ID);
        }
        out << '\n';
    }
};

struct EdgeFileDesc {
    std::string label;
    std::string src_label;
    std::string src_id_field;
    std::string dst_label;
    std::string dst_id_field;

    std::vector<ColumnSpec> columns;
    size_t src_column;
    size_t dst_column;

    std::string ToString() const {
        return fma_common::StringFormatter::Format(
            "[edge file, label:{}, src:({}:{}), dst:({},{}), src_column:{}, "
            "dst_column:{}, columns:{}]",
            label, src_label, src_id_field, dst_label, dst_id_field, src_column, dst_column,
            columns);
    }

    template <class StreamT>
    void DumpTo(StreamT& out) const {
        out << KeyWordFunc::GetStrFromKeyWord(KeyWord::LABEL) << '=' << label << ','
            << KeyWordFunc::GetStrFromKeyWord(KeyWord::SRC_ID) << '=' << src_label << ':'
            << src_id_field << ',' << KeyWordFunc::GetStrFromKeyWord(KeyWord::DST_ID) << '='
            << dst_label << ':' << dst_id_field << '\n';

        for (size_t i = 0; i < columns.size(); i++) {
            if (i) out << ',';

            if (i == src_column)
                out << KeyWordFunc::GetStrFromKeyWord(KeyWord::SRC_ID);
            else if (i == dst_column)
                out << KeyWordFunc::GetStrFromKeyWord(KeyWord::DST_ID);
            else
                columns[i].DumpTo(out);
        }
        out << '\n';
    }
};

struct FileDesc {
    FileDesc() {}
    explicit FileDesc(const std::string& path_) : path(path_) {}

    std::string path;
    std::string data_format;
    bool is_vertex_file = false;
    size_t size = 0;
    size_t n_header_line = 0;

    VertexFileDesc vfile_desc;  // valid only if it is a vertex file
    EdgeFileDesc efile_desc;    // valid only if it is an edge file

    std::string ToString() const {
        return fma_common::StringFormatter::Format(
            "path={}, is_vertex_file={}, size={}, desc={}", path, is_vertex_file, size,
            is_vertex_file ? vfile_desc.ToString() : efile_desc.ToString());
    }

    // this will not dump the file name like [/path/to/file1.csv]
    // should be reparsed by ParseOneFile with has_file_path=false
    std::string Dump() const {
        std::stringstream ss;
        DumpTo(ss);
        return ss.str();
    }
    template <class StreamT>
    void DumpTo(StreamT& out) const {
        if (n_header_line)
            out << KeyWordFunc::GetStrFromKeyWord(KeyWord::HEADER) << '=' << n_header_line << ',';

        if (is_vertex_file)
            vfile_desc.DumpTo(out);
        else
            efile_desc.DumpTo(out);
    }
};
}  // namespace comp
}  // namespace import_v2
}  // namespace lgraph
