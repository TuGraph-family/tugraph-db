
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

#include <iostream>
#include "fma-common/configuration.h"
#include "fma-common/fma_stream.h"
#include "core/field_data_helper.h"
#include "lgraph/lgraph.h"
#include "tools/json.hpp"

// csv file HEAD, will be ignored while importing
const int HEADER = 2;

struct EdgeFileKey {
    size_t e_lid;
    size_t src_lid;
    size_t dst_lid;

    EdgeFileKey(size_t elid, size_t slid, size_t dlid)
        : e_lid(elid), src_lid(slid), dst_lid(dlid) {}

    std::hash<size_t> hasher_;

    bool operator==(const EdgeFileKey& rhs) const {
        return e_lid == rhs.e_lid && src_lid == rhs.src_lid && dst_lid == rhs.dst_lid;
    }
};

namespace std {
template <>
struct hash<EdgeFileKey> {
    size_t operator()(const EdgeFileKey& key) const {
        return hash<size_t>()(key.e_lid) ^ hash<size_t>()(key.src_lid) ^
               hash<size_t>()(key.dst_lid);
    }
};
}  // namespace std

static std::string& AppendFieldDataString(const lgraph_api::FieldData& fd, std::string& buf) {
    if (!fd.is_null()) {
        if (fd.type == lgraph_api::FieldType::STRING) {
            // string need to be processed:
            // \n need to be replaced with \\n
            // " need to be replaced with ""
            // need to be escaped with "
            buf.push_back('"');
            for (auto c : fd.AsString()) {
                switch (c) {
                case '\n':
                    buf.append("\\n");
                    break;
                case '"':
                    buf.append("\"\"");
                    break;
                default:
                    buf.push_back(c);
                }
            }
            buf.push_back('"');
        } else {
            buf.append(fd.ToString());
        }
    }
    return buf;
}

struct FieldsDumper {
    std::vector<size_t> fids;

    explicit FieldsDumper(size_t n_fields) {
        fids.resize(n_fields);
        for (size_t i = 0; i < fids.size(); i++) fids[i] = i;
    }

    template <typename IT>
    void AppendToBuf(const IT& it, std::string& buf, const std::string& delimiter) const {
        std::vector<lgraph_api::FieldData> fds = it.GetFields(fids);
        for (size_t i = 0; i < fds.size(); i++) {
            AppendFieldDataString(fds[i], buf);
            if (i != fds.size() - 1) buf.append(delimiter);
        }
    }
    template <typename IT>
    void AppendToBufInJson(const IT& it, std::string& buf) const {
        std::vector<lgraph_api::FieldData> fds = it.GetFields(fids);
        nlohmann::json line;
        for (size_t i = 0; i < fds.size(); i++) {
            line.push_back(fds[i].ToString());
        }
        buf.append(line.dump());
    }
};

class VertexDumper {
    std::string label_;
    std::vector<lgraph_api::FieldSpec> schema_;
    size_t primary_field_;

    FieldsDumper dumper_;
    std::string file_path_;
    fma_common::OutputFmaStream file_;
    std::string delimiter_;
    bool export_in_json_;

    std::string tmp_buf_;

 public:
    VertexDumper(const std::string& lbl, const std::vector<lgraph_api::FieldSpec>& schema,
                 size_t primary_field, const std::string& dir, const std::string& delimiter,
                 const std::string& format)
        : label_(lbl),
          schema_(schema),
          primary_field_(primary_field),
          dumper_(schema.size()),
          delimiter_(delimiter),
          export_in_json_(format == "json" ? true : false) {
        file_path_ = dir + "/" + label_;
        if (export_in_json_) {
            file_path_.append(".jsonl");
        } else {
            file_path_.append(".csv");
        }
        file_.Open(file_path_);
        if (!file_.Good()) FMA_ERR() << "Error opening file [" << file_path_ << "] for write.";
        tmp_buf_.reserve(1024);
    }

    const std::string& FilePath() const { return file_path_; }

    void DumpVertex(const lgraph_api::VertexIterator& vit) {
        tmp_buf_.clear();
        if (export_in_json_) {
            dumper_.AppendToBufInJson(vit, tmp_buf_);
        } else {
            dumper_.AppendToBuf(vit, tmp_buf_, delimiter_);
        }
        tmp_buf_.push_back('\n');
        file_.Write(tmp_buf_.data(), tmp_buf_.size());
    }
    void AppendConf(const std::vector<lgraph_api::IndexSpec>& vertex_indexs, nlohmann::json& conf) {
        nlohmann::json properties;
        for (size_t i = 0; i < schema_.size(); i++) {
            const lgraph_api::FieldSpec& f = schema_[i];
            nlohmann::json item;
            item["name"] = f.name;
            item["type"] = lgraph::field_data_helper::FieldTypeName(f.type);
            if (i != primary_field_) {
                if (f.optional) {
                    item["optional"] = true;
                }
                auto iter = std::find_if(vertex_indexs.begin(), vertex_indexs.end(),
                                         [this, &f](const auto& item) {
                                             return item.label == label_ && item.field == f.name;
                                         });
                if (iter != vertex_indexs.end()) {
                    item["index"] = true;
                    bool unique = false, pair_unique = false;
                    switch (iter->type) {
                    case lgraph::IndexType::GlobalUniqueIndex:
                        unique = true;
                        break;
                    case lgraph::IndexType::PairUniqueIndex:
                        pair_unique = true;
                        break;
                    case lgraph::IndexType::NonuniqueIndex:
                        // just to pass the compilation
                        break;
                    }
                    if (unique) {
                        item["unique"] = true;
                    } else if (pair_unique) {
                        item["pair_unique"] = true;
                    }
                }
            }
            properties.push_back(item);
        }
        nlohmann::json s_item;
        s_item["label"] = label_;
        s_item["type"] = "VERTEX";
        s_item["properties"] = properties;
        s_item["primary"] = schema_[primary_field_].name;
        conf["schema"].push_back(s_item);

        nlohmann::json file;
        file["path"] = file_path_;
        if (export_in_json_) {
            file["format"] = "JSON";
        } else {
            file["format"] = "CSV";
        }
        file["label"] = label_;
        for (const auto& f : schema_) {
            file["columns"].push_back(f.name);
        }
        conf["files"].push_back(file);
    }

    size_t PrimaryField() const { return primary_field_; }
};

class EdgeDumper {
    std::string label_;
    std::string src_label_;
    std::string src_id_field_;
    std::string dst_label_;
    std::string dst_id_field_;
    std::vector<lgraph_api::FieldSpec> schema_;
    size_t dst_fid_;

    FieldsDumper dumper_;
    std::string file_path_;
    fma_common::OutputFmaStream file_;
    std::string delimiter_;
    bool export_in_json_;

    std::string tmp_buf_;

 public:
    EdgeDumper(const std::string& lbl, const std::string& slabel, const std::string& dlabel,
               const std::vector<lgraph_api::FieldSpec>& schema, size_t dfid,
               const std::string& dir, const std::string& delimiter, const std::string& format)
        : label_(lbl),
          src_label_(slabel),
          dst_label_(dlabel),
          schema_(schema),
          dst_fid_(dfid),
          dumper_(schema.size()),
          delimiter_(delimiter),
          export_in_json_(format == "json" ? true : false) {
        file_path_ =
            fma_common::StringFormatter::Format("{}/{}_{}_{}", dir, label_, src_label_, dst_label_);
        if (export_in_json_) {
            file_path_.append(".jsonl");
        } else {
            file_path_.append(".csv");
        }
        file_.Open(file_path_);
        if (!file_.Good()) FMA_ERR() << "Error opening file [" << file_path_ << "] for write.";
        tmp_buf_.reserve(1024);
    }

    const std::string& FilePath() const { return file_path_; }

    void DumpEdge(const std::string& src_uid, const lgraph_api::OutEdgeIterator& eit,
                  const lgraph_api::VertexIterator& dst_it) {
        tmp_buf_.clear();
        if (export_in_json_) {
            nlohmann::json line;
            line.push_back(src_uid);
            line.push_back(dst_it.GetField(dst_fid_).ToString());
            if (!schema_.empty()) {
                std::vector<lgraph_api::FieldData> fds = eit.GetFields(dumper_.fids);
                for (size_t i = 0; i < fds.size(); i++) {
                    line.push_back(fds[i].ToString());
                }
            }
            tmp_buf_.append(line.dump());
        } else {
            tmp_buf_.append(src_uid);
            tmp_buf_.append(delimiter_);
            // get dst_uid
            AppendFieldDataString(dst_it.GetField(dst_fid_), tmp_buf_);
            // write fields
            if (!schema_.empty()) {
                tmp_buf_.append(delimiter_);
                dumper_.AppendToBuf(eit, tmp_buf_, delimiter_);
            }
        }
        tmp_buf_.push_back('\n');
        file_.Write(tmp_buf_.data(), tmp_buf_.size());
    }
    void AppendConf(const std::vector<lgraph_api::IndexSpec>& edge_indexs, nlohmann::json& conf) {
        nlohmann::json properties;
        for (size_t i = 0; i < schema_.size(); i++) {
            nlohmann::json item;
            const lgraph_api::FieldSpec& f = schema_[i];
            item["name"] = f.name;
            item["type"] = lgraph::field_data_helper::FieldTypeName(f.type);
            if (f.optional) {
                item["optional"] = true;
            }
            auto iter = std::find_if(edge_indexs.begin(), edge_indexs.end(),
                                     [this, &f](const auto& item) {
                                         return item.label == label_ && item.field == f.name;
                                     });
            if (iter != edge_indexs.end()) {
                item["index"] = true;
                bool unique = false, pair_unique = false;
                switch (iter->type) {
                case lgraph::IndexType::GlobalUniqueIndex:
                    unique = true;
                    break;
                case lgraph::IndexType::PairUniqueIndex:
                    pair_unique = true;
                    break;
                case lgraph::IndexType::NonuniqueIndex:
                    // just to pass the compilation
                    break;
                }
                if (unique) {
                    item["unique"] = true;
                } else if (pair_unique) {
                    item["pair_unique"] = true;
                }
            }
            properties.push_back(item);
        }
        nlohmann::json s_item;
        s_item["label"] = label_;
        s_item["type"] = "EDGE";
        if (properties.size() > 0) {
            s_item["properties"] = properties;
        }
        nlohmann::json constraints, constraints_item;
        constraints_item.push_back(src_label_);
        constraints_item.push_back(dst_label_);
        constraints.push_back(constraints_item);
        s_item["constraints"] = constraints;
        auto iter = std::find_if(conf["schema"].begin(),
                                 conf["schema"].end(),
                                 [this](const auto& item) {
                                     return item["label"] == label_ && item["type"] == "EDGE";
                                 });
        if (iter == conf["schema"].end()) {
            conf["schema"].push_back(s_item);
        } else {
            (*iter)["constraints"].push_back(constraints_item);
        }

        nlohmann::json file;
        file["path"] = file_path_;
        if (export_in_json_) {
            file["format"] = "JSON";
        } else {
            file["format"] = "CSV";
        }
        file["label"] = label_;
        file["SRC_ID"] = src_label_;
        file["DST_ID"] = dst_label_;
        file["columns"].push_back("SRC_ID");
        file["columns"].push_back("DST_ID");
        for (const auto& f : schema_) {
            file["columns"].push_back(f.name);
        }
        conf["files"].push_back(file);
    }
};

bool Export(lgraph_api::GraphDB& db, const std::string& exportdir, const std::string& delimiter,
            const std::string& format) {
    using namespace lgraph_api;

    auto txn = db.CreateReadTxn();
    size_t nv = txn.GetNumVertices();
    // opened files
    // label_id -> vertex file
    std::unordered_map<size_t, std::unique_ptr<VertexDumper>> vfiles;
    // (elid, src_lid, dst_lid) -> edge file
    std::unordered_map<EdgeFileKey, std::unique_ptr<EdgeDumper>> efiles;
    // opened vertex & edge file name
    size_t last_v_lid = -1;
    std::string last_vlabel = "";
    VertexDumper* vdesc = nullptr;

    size_t n_v_dumped = 0;
    size_t n_e_dumped = 0;
    auto dst_vit = txn.GetVertexIterator();
    for (auto vit = txn.GetVertexIterator(); vit.IsValid(); vit.Next()) {
        n_v_dumped++;
        if (n_v_dumped % 100000 == 0) {
            FMA_LOG() << (double)n_v_dumped / nv << " complete. Dumped " << n_v_dumped
                      << " vertexes and " << n_e_dumped << " edges.";
        }
        size_t vlid = vit.GetLabelId();
        if (vlid != last_v_lid) {
            // get current vdesc
            auto it = vfiles.find(vlid);
            if (it == vfiles.end()) {
                // construct a new VertexDumper
                const std::string label = vit.GetLabel();
                std::vector<FieldSpec> schema = txn.GetVertexSchema(label);
                // make sure schema is sorted by field id
                std::map<size_t, FieldSpec> id_fs;
                for (auto& f : schema) id_fs.emplace(txn.GetVertexFieldId(vlid, f.name), f);
                FMA_DBG_CHECK_EQ(id_fs.begin()->first, 0);
                FMA_DBG_CHECK_EQ(id_fs.rbegin()->first, schema.size() - 1);
                for (size_t i = 0; i < schema.size(); i++) schema[i] = id_fs[i];
                size_t primary_fid = txn.GetVertexFieldId(vlid, txn.GetVertexPrimaryField(label));
                it = vfiles.emplace_hint(
                    it, vlid,
                    std::unique_ptr<VertexDumper>(
                        new VertexDumper(label, schema, primary_fid,
                            exportdir, delimiter, format)));
            }
            vdesc = it->second.get();
        }
        // write vertex
        vdesc->DumpVertex(vit);
        std::string src_uid = vit.GetField(vdesc->PrimaryField()).ToString();
        // now dump edges
        for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
            size_t elid = eit.GetLabelId();
            bool r = dst_vit.Goto(eit.GetDst());
            FMA_DBG_ASSERT(r);
            EdgeFileKey efkey(elid, vlid, dst_vit.GetLabelId());
            auto efit = efiles.find(efkey);
            if (efit == efiles.end()) {
                // new edge file
                const std::string& elabel = eit.GetLabel();
                const std::string& slabel = vit.GetLabel();
                const std::string& dlabel = dst_vit.GetLabel();
                size_t dst_primary_fid = txn.GetVertexFieldId(dst_vit.GetLabelId(),
                        txn.GetVertexPrimaryField(dlabel));
                // get schema
                std::vector<FieldSpec> schema = txn.GetEdgeSchema(elabel);
                if (!schema.empty()) {
                    // make sure schema is sorted by id
                    std::map<size_t, FieldSpec> id_fds;
                    for (auto& f : schema) id_fds.emplace(txn.GetEdgeFieldId(elid, f.name), f);
                    FMA_DBG_CHECK_EQ(id_fds.begin()->first, 0);
                    FMA_DBG_CHECK_EQ(id_fds.rbegin()->first, schema.size() - 1);
                    for (size_t i = 0; i < schema.size(); i++) schema[i] = id_fds[i];
                }
                // construct new file desc
                efit = efiles.emplace_hint(efit, efkey,
                                           std::unique_ptr<EdgeDumper>(new EdgeDumper(
                                               elabel, slabel, dlabel, schema,
                                               dst_primary_fid, exportdir, delimiter, format)));
            }
            EdgeDumper* edesc = efit->second.get();
            edesc->DumpEdge(src_uid, eit, dst_vit);
            n_e_dumped++;
        }
    }
    FMA_LOG() << "100% complete. Dumped " << n_v_dumped << " vertexes and " << n_e_dumped
              << " edges.";
    // now write config file
    fma_common::OutputFmaStream config_file;
    config_file.Open(exportdir + "/import.config");
    if (!config_file.Good())
        FMA_ERR() << "Failed to open file " << config_file.Path() << " for write.";
    nlohmann::json conf;
    auto vertex_indexs = txn.ListVertexIndexes();
    auto edge_indexs = txn.ListEdgeIndexes();
    for (auto& f : vfiles) {
        VertexDumper* fdesc = f.second.get();
        fdesc->AppendConf(vertex_indexs, conf);
    }
    for (auto& f : efiles) {
        EdgeDumper* fdesc = f.second.get();
        fdesc->AppendConf(edge_indexs, conf);
    }
    const auto& str = conf.dump(4);
    config_file.Write(str.data(), str.size());
    FMA_LOG() << "Config file generated in [" << config_file.Path() << "]";
    return true;
}

int main(int argc, char** argv) {
    std::string db_dir = "./testdb";
    std::string export_dir = "./exportdir";
    std::string graph = "default";
    std::string user;
    std::string password;
    std::string delimiter = ",";
    std::string format = "csv";

    // param config
    fma_common::Configuration config;
    config.Add(db_dir, "d,db_dir", true).Comment("Database directory");
    config.Add(export_dir, "e,export_dir", true).Comment("Export destination directory");
    config.Add(graph, "g,graph", true).Comment("Graph to use");
    config.Add(user, "u,user", false).Comment("User name");
    config.Add(password, "p,password", false).Comment("Password");
    config.Add(delimiter, "s,separator", true).Comment("Field separator to use");
    config.Add(format, "f,format", true)
        .SetPossibleValues({"json", "csv"})
        .Comment("Export data in json or csv format");
    try {
        config.ExitAfterHelp(true);
        config.ParseAndFinalize(argc, argv);
    } catch (std::exception& e) {
        FMA_ERR() << "Failed to parse command line option: " << e.what();
        return -1;
    }

    // check db_dir and export_dir
    fma_common::FileSystem& db_fs = fma_common::FileSystem::GetFileSystem(db_dir);
    if (!db_fs.IsDir(db_dir)) {
        throw std::runtime_error("DB directory: \"" + db_dir + "\" does not exist.");
    } else {
        FMA_LOG() << "DB directory has been found.";
    }

    if (format != "csv" && format != "json") {
        throw std::runtime_error("format error, can only be \"json\" or \"csv\"");
    }

    fma_common::FileSystem& export_fs = fma_common::FileSystem::GetFileSystem(export_dir);
    if (!export_fs.IsDir(export_dir)) {
        FMA_LOG() << "Export directory does not exist, creating...";
        if (!export_fs.Mkdir(export_dir)) {
            throw std::runtime_error("Creating export directory: \"" + export_dir + "\" fail.");
        } else {
            FMA_LOG() << "Export directory has been created, now exporting...";
        }
    } else {
        FMA_LOG() << "Export directory has been found, now exporting...";
    }

    try {
        FMA_LOG() << "From dir:        " << db_dir;
        FMA_LOG() << "From graph:      " << graph;
        FMA_LOG() << "To dir:          " << export_dir;
        FMA_LOG() << "Field separator: " << delimiter;
        FMA_LOG() << "Format:          " << format;
        // export process
        lgraph_api::Galaxy galaxy(db_dir, user, password, false, false);
        lgraph_api::GraphDB db = galaxy.OpenGraph(graph, true);

        double t1 = fma_common::GetTime();
        if (!Export(db, export_dir, delimiter, format)) {
            FMA_LOG() << "Something went wrong, export failed.";
        }
        double t2 = fma_common::GetTime();
        FMA_LOG() << "Export successful in " << t2 - t1 << " seconds.";
    } catch (std::exception& e) {
        FMA_ERR() << "Error occurred during export: " << e.what();
        return -1;
    }
    return 0;
}
