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

#include "core/lightning_graph.h"
#include "import/import_online.h"
#include "import/import_client.h"
#include "core/blob_manager.h"

using namespace lgraph;
class BlobBuffer {
    std::mutex mtx_;
    std::vector<std::pair<BlobManager::BlobKey, Value>> blobs_;
    BlobManager::BlobKey next_key_;

 public:
    explicit BlobBuffer(lgraph::Transaction& txn) : next_key_(txn._GetNextBlobKey()) {}

    BlobManager::BlobKey AddBlob(Value&& blob) {
        std::lock_guard<std::mutex> l(mtx_);
        BlobManager::BlobKey key = next_key_++;
        blobs_.emplace_back(key, std::move(blob));
        return key;
    }

    std::vector<std::pair<BlobManager::BlobKey, Value>>& GetBlobs() { return blobs_; }
};

struct OnlineImportEdge {
    // for out edge, vid1=src, vid2=dst
    // for inref, vid1=dst, vid2=src
    VertexId vid1;
    LabelId lid;
    VertexId vid2;
    TemporalId tid;
    Value prop;
    bool is_out_edge;
    OnlineImportEdge(VertexId vid1_, LabelId lid_, VertexId vid2_, TemporalId tid_,
                     Value&& prop, bool is_out)
        : vid1(vid1_), lid(lid_), vid2(vid2_), tid(tid_), prop(std::move(prop))
          , is_out_edge(is_out) {}

    OnlineImportEdge(VertexId vid1_, LabelId lid_, VertexId vid2_, TemporalId tid_,
                     const Value& prop, bool is_out)
        : vid1(vid1_), lid(lid_), vid2(vid2_), tid(tid_), prop(prop), is_out_edge(is_out) {}

    bool operator<(const OnlineImportEdge& rhs) const {
        return vid1 < rhs.vid1 || (vid1 == rhs.vid1 && lid < rhs.lid);
    }
};
typedef lgraph::Transaction::EdgeDataForTheSameVertex OnlineImportEdgesOfSameVertex;

// the message of a package of edges sent from master to workers in HA mode
struct OnlineEdgePackage {
    bool continue_on_error;
    // edges are grouped by the vertex which it belongs to
    std::vector<OnlineImportEdgesOfSameVertex> data;
};

// type Func should be like function<void(size_t,size_t)>
// where the parameters are (start_i,end_i)
// work may throw anything, which will be rethrowed by this function
template <typename Func>
static void DoMultiThreadWork(size_t n, const Func& work, size_t n_thread = 0) {
    if (n_thread == 0) {
        n_thread = std::thread::hardware_concurrency();
        if (n_thread <= 0) n_thread = 1;
    }

    auto nothrow_work = [&work](std::exception_ptr* exception_store, size_t begin, size_t end) {
        try {
            work(begin, end);
        } catch (...) {
            *exception_store = std::current_exception();
        }
    };

    std::vector<std::exception_ptr> exceptions(n_thread);
    std::vector<std::thread> threads;
    size_t n_per_thread = (n + n_thread - 1) / n_thread;
    for (size_t i = 0; i < n_thread; i++)
        threads.emplace_back(nothrow_work, &exceptions[i], n_per_thread * i,
                             std::min(n_per_thread * (i + 1), n));
    for (auto& thread : threads) thread.join();
    for (auto& exception : exceptions)
        if (exception) std::rethrow_exception(exception);
}

std::string lgraph::import_v2::ImportOnline::ImportVertexes(
    LightningGraph* db, Transaction& txn, const CsvDesc& fd,
    std::vector<std::vector<FieldData>>&& data, const Config& config) {
    std::string error;
    std::mutex err_mtx;
    FMA_DBG_ASSERT(fd.is_vertex_file);

    const std::string& label = fd.label;
    size_t label_id = txn.GetLabelId(true, label);
    std::vector<std::string> field_names;
    for (size_t i = 0; i < fd.columns.size(); i++) {
        auto& c = fd.columns[i];
        if (c != KeyWordFunc::GetStrFromKeyWord(KeyWord::SKIP)) field_names.push_back(c);
    }
    std::vector<size_t> field_ids = txn.GetFieldIds(true, label_id, field_names);
    auto& primary = txn.GetVertexPrimaryField(label);
    size_t kcol = fd.FindIdxExcludeSkip(primary);
    // sort the vertices just read by key column
    LGRAPH_PSORT(data.begin(), data.end(),
                 [kcol](const std::vector<FieldData>& lhs, const std::vector<FieldData>& rhs) {
                     return lhs[kcol] < rhs[kcol];
                 });
    // pack into values
    std::vector<Value> values;
    values.resize(data.size());
    Schema* schema = txn.curr_schema_->v_schema_manager.GetSchema(label_id);
    BlobBuffer blob_buffer(txn);
    auto create_record_in_range = [&](size_t beg, size_t end) {
        for (size_t i = beg; i < end; i++) {
            try {
                if (schema->HasBlob())
                    values[i] = schema->CreateRecordWithBlobs(
                        field_ids.size(), field_ids.data(), data[i].data(),
                        [&](const Value& blob) { return blob_buffer.AddBlob(blob.MakeCopy()); });
                else
                    values[i] =
                        schema->CreateRecord(field_ids.size(), field_ids.data(), data[i].data());
            } catch (std::exception& e) {
                std::string msg = FMA_FMT("When importing vertex label [{}]:\n{}\n", label,
                                          PrintNestedException(e, 1));
                if (!config.continue_on_error) throw InputError(msg);
                std::lock_guard<std::mutex> lg(err_mtx);
                error.append(msg);
            }
        }
    };
    DoMultiThreadWork(data.size(), create_record_in_range, config.n_threads);
    // write to db
    error += txn._OnlineImportBatchAddVertexes(schema, values, blob_buffer.GetBlobs(),
                                               config.continue_on_error);
    return error;
}

std::string lgraph::import_v2::ImportOnline::ImportEdges(LightningGraph* db, Transaction& txn,
                                                         const CsvDesc& file_desc,
                                                         std::vector<std::vector<FieldData>>&& data,
                                                         const Config& config) {
    std::string error;
    std::mutex err_mtx;
    FMA_DBG_ASSERT(!file_desc.is_vertex_file);
    auto& fd = file_desc;
    const std::string& label = fd.label;
    const std::string& src_label = fd.edge_src.label;
    const std::string& dst_label = fd.edge_dst.label;

    {
        auto schema = txn.GetSchema(label, false);
        if (!schema)
            throw InputError(FMA_FMT("Edge Label [{}] does not exist.", label));
        const auto& ec = schema->GetEdgeConstraintsLids();
        if (!ec.empty()) {
            graph::EdgeConstraintsChecker ecc(ec);
            ecc.Check(txn.GetLabelId(true, src_label),
                      txn.GetLabelId(true, dst_label));
        }
    }

    std::vector<std::string> field_names;
    size_t src_id_pos_in_parsed_fds = fd.GetEdgeSrcColumnIDExcludeSkip();
    size_t dst_id_pos_in_parsed_fds = fd.GetEdgeDstColumnIDExcludeSkip();
    int n_fds = 0;
    for (size_t i = 0; i < fd.columns.size(); i++) {
        if (fd.columns[i] != KeyWordFunc::GetStrFromKeyWord(KeyWord::SKIP) &&
            fd.columns[i] != KeyWordFunc::GetStrFromKeyWord(KeyWord::SRC_ID) &&
            fd.columns[i] != KeyWordFunc::GetStrFromKeyWord(KeyWord::DST_ID)) {
            field_names.push_back(fd.columns[i]);
            n_fds++;
        }
    }
    size_t first_id_pos = std::min(src_id_pos_in_parsed_fds, dst_id_pos_in_parsed_fds);
    size_t second_id_pos = std::max(src_id_pos_in_parsed_fds, dst_id_pos_in_parsed_fds);

    size_t temporal_id_pos;
    bool has_temporal_field = txn.HasTemporalField(label);
    if (has_temporal_field) {
        const std::string& temporal = txn.GetEdgeTemporalField(label);
        temporal_id_pos = fd.FindIdxExcludeSkip(temporal);
    }

    size_t label_id = txn.GetLabelId(false, label);
    std::vector<size_t> field_ids = txn.GetFieldIds(false, label_id, field_names);

    VertexIndex* src_index = txn.GetVertexIndex(src_label, fd.edge_src.id);
    if (!src_index || !src_index->IsReady()) {
        throw ::lgraph::InputError(fma_common::StringFormatter::Format(
            "Src {} field {} has no available index!", src_label, fd.edge_src.id));
    }

    VertexIndex* dst_index = txn.GetVertexIndex(dst_label, fd.edge_dst.id);
    if (!dst_index || !dst_index->IsReady()) {
        throw ::lgraph::InputError(fma_common::StringFormatter::Format(
            "Dst {} field {} has no available index!", dst_label, fd.edge_dst.id));
    }

    // lookup index for src vid
    size_t n = data.size();
    std::vector<VertexId> src_vids;
    src_vids.resize(n);
    BlobBuffer blob_buffer(txn);

    auto calc_src_vid_in_range = [&](size_t begin, size_t end) {
        auto txn = db->CreateReadTxn();
        for (size_t i = begin; i < end; i++) {
            auto& field_values = data[i];
            auto& src_id = field_values[src_id_pos_in_parsed_fds];
            Value src_key =
                field_data_helper::FieldDataToValueOfFieldType(src_id, src_index->KeyType());
            VertexId src_vid;
            auto src_it = src_index->GetIterator(&txn, src_key, src_key);
            if (src_it.IsValid()) {
                src_vid = src_it.GetVid();
            } else {
                std::string msg = "Missing src uid " + src_id.ToString() + "\n";
                if (!config.continue_on_error) throw InputError(msg);
                std::lock_guard<std::mutex> lg(err_mtx);
                error.append(msg);
                src_vid = -1;
            }
            src_vids[i] = src_vid;
        }
    };
    DoMultiThreadWork(n, calc_src_vid_in_range, config.n_threads);

    // lookup index for dst vid
    std::vector<VertexId> dst_vids;
    dst_vids.resize(n);
    auto calc_dst_vid_in_range = [&](size_t begin, size_t end) {
        auto txn = db->CreateReadTxn();
        for (size_t i = begin; i < end; i++) {
            auto& field_values = data[i];
            auto& dst_id = field_values[dst_id_pos_in_parsed_fds];
            Value dst_key = ::lgraph::field_data_helper::FieldDataToValueOfFieldType(
                dst_id, dst_index->KeyType());
            VertexId dst_vid;
            auto dst_it = dst_index->GetIterator(&txn, dst_key, dst_key);
            if (dst_it.IsValid()) {
                dst_vid = dst_it.GetVid();
            } else {
                std::string msg = "Missing dst uid " + dst_id.ToString() + "\n";
                if (!config.continue_on_error) throw InputError(msg);
                std::lock_guard<std::mutex> lg(err_mtx);
                error.append(msg);
                dst_vid = -1;
            }
            dst_vids[i] = dst_vid;
        }
    };
    DoMultiThreadWork(n, calc_dst_vid_in_range, config.n_threads);

    // prepare records and remove src/dst fields
    std::vector<OnlineImportEdge> edges;
    std::mutex edges_mtx;
    Schema* schema = txn.curr_schema_->e_schema_manager.GetSchema(label_id);
    auto prepare_records = [&](size_t beg, size_t end) {
        for (size_t i = beg; i < end; i++) {
            auto& field_values = data[i];
            int64_t src_vid = src_vids[i];
            int64_t dst_vid = dst_vids[i];
            TemporalId tid = has_temporal_field ? field_values[temporal_id_pos].AsInt64() : 0;
            if (src_vid == -1 || dst_vid == -1) continue;
            for (size_t i = first_id_pos; i < second_id_pos - 1; i++) {
                field_values[i] = std::move(field_values[i + 1]);
            }
            for (size_t i = second_id_pos - 1; i < field_values.size() - 2; i++) {
                field_values[i] = std::move(field_values[i + 2]);
            }
            field_values.resize(field_values.size() - 2);
            FMA_DBG_CHECK_EQ(field_values.size(), field_ids.size());
            Value prop;
            try {
                if (schema->HasBlob())
                    prop = schema->CreateRecordWithBlobs(
                        field_ids.size(), field_ids.data(), field_values.data(),
                        [&](const Value& blob) { return blob_buffer.AddBlob(blob.MakeCopy()); });
                else
                    prop = schema->CreateRecord(field_ids.size(), field_ids.data(),
                                                field_values.data());
            } catch (std::exception& e) {
                std::string msg = fma_common::StringFormatter::Format(
                    "When importing edge label [{}]:\n{}\n", label, PrintNestedException(e, 1));
                if (!config.continue_on_error) throw ::lgraph::InputError(msg);
                std::lock_guard<std::mutex> l(err_mtx);
                error.append(msg);
            }
            std::lock_guard<std::mutex> l(edges_mtx);
            edges.emplace_back(src_vid, static_cast<LabelId>(label_id), dst_vid,
                               tid, prop, true);
            edges.emplace_back(dst_vid, static_cast<LabelId>(label_id), src_vid,
                               tid, std::move(prop), false);
        }
    };
    DoMultiThreadWork(n, prepare_records, config.n_threads);

    // sort edges by vertex and merge edges of same vertex together in
    // OnlineImportEdgesOfSameVertex
    LGRAPH_PSORT(edges.begin(), edges.end());
    n = edges.size();
    std::vector<OnlineImportEdgesOfSameVertex> edges_of_vertex;
    for (size_t L = 0, R; L < n; L = R) {
        for (R = L; R < n && edges[R].vid1 == edges[L].vid1; R++) {
        }
        OnlineImportEdgesOfSameVertex oiv;
        oiv.vid = edges[L].vid1;
        for (size_t i = L; i < R; i++) {
            if (edges[i].is_out_edge) {
                oiv.outs.emplace_back(edges[i].lid, edges[i].vid2, edges[i].tid,
                                      std::move(edges[i].prop));
            } else {
                oiv.ins.emplace_back(edges[i].lid, edges[i].vid2, edges[i].tid,
                                     std::move(edges[i].prop));
            }
        }
        edges_of_vertex.emplace_back(std::move(oiv));
    }

    // write to db
    txn._OnlineImportBatchAddEdges(edges_of_vertex, blob_buffer.GetBlobs(),
                                   config.continue_on_error, schema);
    return error;
}

// this function succeed or throw exception
std::string lgraph::import_v2::ImportOnline::HandleOnlineTextPackage(
    std::string&& desc, std::string&& data, LightningGraph* db,
    const lgraph::import_v2::ImportOnline::Config& config) {
    // parse desc
    CsvDesc fd;
    std::vector<CsvDesc> cds;
    FMA_LOG() << "desc: " << desc;
    cds = ImportConfParser::ParseFiles(nlohmann::json::parse(desc), false);
    if (cds.size() != 1)
        std::throw_with_nested(InputError(FMA_FMT("config items number error:  {}", desc)));
    fd = cds[0];
    if (!fd.is_vertex_file) {
        auto txn = db->CreateReadTxn();
        SchemaDesc schema;
        schema.ConstructFromDB(txn);
        txn.Abort();
        fd.edge_src.id = schema.FindVertexLabel(fd.edge_src.label).GetPrimaryField().name;
        fd.edge_dst.id = schema.FindVertexLabel(fd.edge_dst.label).GetPrimaryField().name;
    }

    // create txn early, to block other write during preparation
    // must be non-optimistic to ensure consistency of prepare and write
    auto txn = db->CreateWriteTxn(false);

    // parse input
    std::vector<std::vector<FieldData>> all_data;
    std::vector<FieldSpec> field_specs;
    fd.GenFieldSpecs(txn, field_specs);
    {
        std::unique_ptr<fma_common::InputFileStream> data_stream(
            new fma_common::InputMemoryFileStream(std::move(data)));
        std::unique_ptr<BlockParser> parser;
        if (fd.data_format == "CSV") {
            parser.reset(new ColumnParser(data_stream.get(), field_specs, 1 << 20, 8,
                                          fd.n_header_line, config.continue_on_error,
                                          config.delimiter));
        } else {
            parser.reset(new JsonLinesParser(std::move(data_stream), field_specs, 1 << 20, 8,
                                             fd.n_header_line, config.continue_on_error));
        }
        std::vector<std::vector<FieldData>> block;
        while (parser->ReadBlock(block)) {
            all_data.insert(all_data.end(), std::make_move_iterator(block.begin()),
                            std::make_move_iterator(block.end()));
        }
    }

    std::string errors = fd.is_vertex_file
                             ? ImportVertexes(db, txn, fd, std::move(all_data), config)
                             : ImportEdges(db, txn, fd, std::move(all_data), config);
    txn.Commit();
    return errors;
}

std::string lgraph::import_v2::ImportOnline::HandleOnlineSchema(std::string&& desc,
                                                                AccessControlledDB& db) {
    std::string errors = "";
    FMA_LOG() << desc;
    FMA_LOG() << "----------";
    SchemaDesc schema_new = ImportConfParser::ParseSchema(nlohmann::json::parse(desc));
    auto txn = db.CreateReadTxn();
    if (txn.GetAllLabels(true).size() != 0) {
        // if db schema not empty
        SchemaDesc schema_curr;
        schema_curr.ConstructFromDB(txn);
        SchemaDesc schema_diff = schema_new.DifferenceSet(schema_curr);
        schema_new = schema_diff;
    }
    txn.Abort();

    // add vertex, edge, index
    for (auto& v : schema_new.label_desc) {
        // create labels
        auto m = v.GetSchemaDef();
        std::vector<FieldSpec> fds;
        std::unique_ptr<LabelOptions> options;
        if (v.is_vertex) {
            auto vo = std::make_unique<VertexOptions>();
            vo->primary_field = v.GetPrimaryField().name;
            options = std::move(vo);
        } else {
            auto eo = std::make_unique<EdgeOptions>();
            eo->edge_constraints = v.edge_constraints;
            if (v.HasTemporalField()) {
                auto tf = v.GetTemporalField();
                eo->temporal_field = tf.name;
                eo->temporal_field_order = tf.temporal_order;
            }
            options = std::move(eo);
        }
        options->detach_property = v.detach_property;
        for (auto& p : m) fds.emplace_back(p.second);
        bool ok = db.AddLabel(v.is_vertex, v.name, fds, *options);
        if (ok) {
            FMA_LOG() << FMA_FMT("Add {} label:{}, detach:{}", v.is_vertex ? "vertex" : "edge",
                                 v.name, options->detach_property);
        } else {
            throw InputError(
                FMA_FMT("{} label:{} already exists", v.is_vertex ? "Vertex" : "Edge", v.name));
        }

        // create index
        for (auto& spec : v.columns) {
            if (v.is_vertex && spec.index && !spec.primary) {
                if (db.AddVertexIndex(v.name, spec.name, spec.idxType)) {
                    FMA_LOG() << FMA_FMT("Add vertex index [label:{}, field:{}, type:{}]",
                                         v.name, spec.name, static_cast<int>(spec.idxType));
                } else {
                    throw InputError(
                        FMA_FMT("Vertex index [label:{}, field:{}] already exists",
                                v.name, spec.name));
                }
            } else if (!v.is_vertex && spec.index) {
                if (db.AddEdgeIndex(v.name, spec.name, spec.idxType)) {
                    FMA_LOG() << FMA_FMT("Add edge index [label:{}, field:{}, type:{}]",
                                         v.name, spec.name, static_cast<int>(spec.idxType));
                } else {
                    throw InputError(
                        FMA_FMT("Edge index [label:{}, field:{}] already exists",
                                v.name, spec.name));
                }
            }
            if (spec.fulltext) {
                if (db.AddFullTextIndex(v.is_vertex, v.name, spec.name)) {
                    FMA_LOG() << FMA_FMT("Add fulltext index [label:{}, field:{}] success",
                                         v.name, spec.name);
                } else {
                    throw InputError(FMA_FMT("Fulltext index [label:{}, field:{}] already exists",
                                                     v.name, spec.name));
                }
            }
        }
    }
    return errors;
}
