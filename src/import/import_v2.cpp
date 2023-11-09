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

#include "core/defs.h"
#include "db/db.h"
#include "db/galaxy.h"
#include "import/import_v2.h"
#include "import/blob_writer.h"
#include "import/import_utils.h"

using lgraph::import::LabelGraph;

lgraph::import_v2::Importer::Importer(const Config& config)
    : config_(config),
      intermediate_file_(config.intermediate_dir, config.intermediate_buf_size),
      next_vid_(0),
      next_blob_key(0) {}

lgraph::AccessControlledDB lgraph::import_v2::Importer::OpenGraph(Galaxy& galaxy, bool empty_db) {
    if (empty_db) {
        config_.user = lgraph::_detail::DEFAULT_ADMIN_NAME;
        config_.password = lgraph::_detail::DEFAULT_ADMIN_PASS;
    }
    try {
        if (galaxy.GetUserToken(config_.user, config_.password).empty()) {
            throw AuthError("Bad user/password.");
        }
        if (!galaxy.IsAdmin(config_.user))
            throw AuthError("Non-admin users are not allowed to perform offline import.");
    } catch (...) {
        std::throw_with_nested(AuthError("Error validating user/password"));
    }

    const std::map<std::string, lgraph::DBConfig>& graphs = galaxy.ListGraphs(config_.user);
    if (graphs.find(config_.graph) != graphs.end()) {
        if (!empty_db) {
            if (!config_.delete_if_exists) {
                throw std::runtime_error(
                    "Graph already exists. If you want to overwrite the graph, use --overwrite "
                    "true.");
            } else {
                FMA_LOG() << "Graph already exists, all the data in the graph will be overwritten.";
                AccessControlledDB db = galaxy.OpenGraph(config_.user, config_.graph);
                db.DropAllData();
            }
        }
    } else {
        galaxy.CreateGraph(config_.user, config_.graph, lgraph::DBConfig());
    }
    galaxy.SetRaftLogIndexBeforeWrite(1);
    return galaxy.OpenGraph(config_.user, config_.graph);
}

void lgraph::import_v2::Importer::DoImportOffline() {
    // an Importer can only be used once
    FMA_ASSERT(!imported_);
    imported_ = true;

    double t1 = fma_common::GetTime();

    bool empty_db = !fma_common::file_system::DirExists(config_.db_dir + "/.meta");
    // Galaxy galaxy(config_.db_dir);
    std::shared_ptr<GlobalConfig> gc = std::make_shared<GlobalConfig>();
    gc->ft_index_options.enable_fulltext_index = config_.enable_fulltext_index;
    gc->ft_index_options.fulltext_analyzer = config_.fulltext_index_analyzer;
    gc->ft_index_options.fulltext_commit_interval = 0;
    gc->ft_index_options.fulltext_refresh_interval = 0;
    Galaxy galaxy(lgraph::Galaxy::Config{config_.db_dir, false, true, "fma.ai"}, true, gc);
    AccessControlledDB db = Importer::OpenGraph(galaxy, empty_db);

    std::ifstream ifs(config_.config_file);
    nlohmann::json conf;
    ifs >> conf;

    SchemaDesc schema = ImportConfParser::ParseSchema(conf);
    std::vector<CsvDesc> data_files = ImportConfParser::ParseFiles(conf);
    for (auto& file : data_files) {
        if (!file.is_vertex_file) {
            if (!schema.FindEdgeLabel(file.label)
                     .MeetEdgeConstraints(file.edge_src.label, file.edge_dst.label)) {
                throw std::runtime_error(FMA_FMT("{} not meet the edge constraints", file.path));
            }
            file.edge_src.id = schema.FindVertexLabel(file.edge_src.label).GetPrimaryField().name;
            file.edge_dst.id = schema.FindVertexLabel(file.edge_dst.label).GetPrimaryField().name;
        }
        ImportConfParser::CheckConsistent(schema, file);
    }

    if (!config_.dry_run) {
        for (auto& v : schema.label_desc) {
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
                if (v.HasTemporalField()) {
                    auto tf = v.GetTemporalField();
                    eo->temporal_field = tf.name;
                    eo->temporal_field_order = tf.temporal_order;
                }
                eo->edge_constraints = v.edge_constraints;
                options = std::move(eo);
            }
            for (auto& p : m) fds.emplace_back(p.second);
            bool ok = db.AddLabel(v.is_vertex, v.name, fds, *options);
            if (ok) {
                FMA_LOG() << FMA_FMT("Add {} label:{} success", v.is_vertex ? "vertex" : "edge",
                                     v.name);
            } else {
                throw std::runtime_error(
                    FMA_FMT("Add {} label:{} error", v.is_vertex ? "vertex" : "edge", v.name));
            }
        }
    }

    auto import_index = [&](){
        for (auto& v : schema.label_desc) {
            for (auto& spec : v.columns) {
                if (v.is_vertex && spec.index && !spec.primary &&
                    spec.idxType == lgraph::IndexType::NonuniqueIndex) {
                    // create index, ID column has creadted
                    if (db.AddVertexIndex(v.name, spec.name, spec.idxType)) {
                        FMA_LOG() << FMA_FMT("Add vertex index [label:{}, field:{}, type:{}]",
                                             v.name, spec.name, static_cast<int>(spec.idxType));
                    } else {
                        throw InputError(
                            FMA_FMT("Vertex index [label:{}, field:{}] already exists",
                                    v.name, spec.name));
                    }
                } else if (v.is_vertex && spec.index && !spec.primary &&
                           (spec.idxType == lgraph::IndexType::GlobalUniqueIndex ||
                            spec.idxType == lgraph::IndexType::PairUniqueIndex)) {
                    throw InputError(
                        FMA_FMT("offline import does not support to create a unique "
                                "index [label:{}, field:{}]. You should create an index for "
                                "an attribute column after the import is complete",
                                v.name, spec.name));
                } else if (!v.is_vertex && spec.index &&
                           spec.idxType != lgraph::IndexType::GlobalUniqueIndex) {
                    if (db.AddEdgeIndex(v.name, spec.name, spec.idxType)) {
                        FMA_LOG() << FMA_FMT("Add edge index [label:{}, field:{}, type:{}]",
                                             v.name, spec.name, static_cast<int>(spec.idxType));
                    } else {
                        throw InputError(
                            FMA_FMT("Edge index [label:{}, field:{}] already exists",
                                    v.name, spec.name));
                    }
                } else if (!v.is_vertex && spec.index &&
                           spec.idxType == lgraph::IndexType::GlobalUniqueIndex) {
                    throw InputError(
                        FMA_FMT("offline import does not support to create a unique "
                                "index [label:{}, field:{}]. You should create an index for "
                                "an attribute column after the import is complete",
                                v.name, spec.name));
                }
                if (spec.fulltext) {
                    bool ok = db.AddFullTextIndex(v.is_vertex, v.name, spec.name);
                    if (ok) {
                        FMA_LOG() << FMA_FMT("Add fulltext index [{} label:{}, field:{}]",
                                             v.is_vertex ? "vertex" : "edge", v.name, spec.name);
                    } else {
                        throw InputError(FMA_FMT(
                            "Fulltext index [{} label:{}, field:{}] already exists",
                            v.is_vertex ? "vertex" : "edge", v.name, spec.name));
                    }
                }
            }
        }
        db.GetLightningGraph()->RebuildAllFullTextIndex();
    };

    if (data_files.empty()) {
        // add index
        import_index();
        db.Flush();
        return;
    }

    auto ret = std::find_if(data_files.begin(), data_files.end(),
                            [](auto& cd) { return cd.is_vertex_file; });
    if (ret == data_files.end()) {
        throw std::runtime_error(FMA_FMT("No vertex data file found in offline importing"));
    }

    // order the files according to labels
    std::map<std::string, std::vector<CsvDesc*>> v_label_files;  // vlabel -> files
    std::map<std::string, std::vector<CsvDesc*>> e_label_files;  // elabel -> files
    std::map<std::pair<std::string, std::string>, std::map<std::string, std::vector<CsvDesc*>>>
        src_dst_files;  // pair<src,dst> -> {e_label -> files}
    std::map<std::string, size_t> v_capacity;
    {
        size_t total_file_size = 0;
        for (auto& file : data_files) {
            total_file_size += file.size;
            if (file.is_vertex_file) {
                v_label_files[file.label].push_back(&file);
                v_capacity[file.label] += file.size;
            } else {
                e_label_files[file.label].push_back(&file);
                src_dst_files[std::make_pair(file.edge_src.label, file.edge_dst.label)][file.label]
                    .push_back(&file);
                v_capacity[file.edge_src.label] += file.size;
                v_capacity[file.edge_dst.label] += file.size;
            }
        }
        FMA_LOG() << "Total file size: " << (double)total_file_size / 1024 / 1024 / 1024 << "GB";
    }

    // calculate n_buckets for each vertex label
    config_.bucket_size_mb = std::max<size_t>(1, config_.bucket_size_mb);
    size_t bucket_size = config_.bucket_size_mb * 1024 * 1024;
    for (auto& vc : v_capacity) {
        n_buckets_[vc.first] = std::max<size_t>((vc.second + bucket_size - 1) / bucket_size, 1);
    }

    // make import plan
    std::vector<std::string> vid_label;
    auto actions = MakePlan(v_label_files, src_dst_files, vid_label);
    for (auto& act : actions) {
        switch (act.type) {
        case PlanExecutor::Action::LOAD_VERTEX:
            {
                const std::string& vlabel = vid_label[act.vid];
                FMA_LOG() << "Load vertex label " << vlabel;
                if (!config_.dry_run) {
                    LoadVertexFiles(db.GetLightningGraph(), v_label_files[vlabel],
                                    schema.FindVertexLabel(vlabel));
                }
                break;
            }
        case PlanExecutor::Action::LOAD_EDGE:
            {
                const std::string& src_label = vid_label[act.edge.first];
                const std::string& dst_label = vid_label[act.edge.second];
                FMA_LOG() << "Load edges from vertex " << src_label << " to " << dst_label;
                if (!config_.dry_run) {
                    std::map<std::string, std::vector<CsvDesc*>>& elabel_files =
                        src_dst_files[std::make_pair(src_label, dst_label)];
                    for (auto& kv : elabel_files) {
                        LoadEdgeFiles(db.GetLightningGraph(), src_label, dst_label,
                                      schema.FindEdgeLabel(kv.first), kv.second);
                    }
                }
                break;
            }
        case PlanExecutor::Action::DUMP_VERTEX:
            {
                const std::string& vlabel = vid_label[act.vid];
                FMA_LOG() << "Write vertex label " << vlabel;
                const ColumnSpec& key_spec = schema.FindVertexLabel(vlabel).GetPrimaryField();
                if (!config_.dry_run) {
                    WriteVertex(db.GetLightningGraph(), vlabel, key_spec.name, key_spec.type);
                }
                break;
            }
        case PlanExecutor::Action::DONE:
            break;
        }
    }
    // persist
    FMA_LOG() << "Persisting data...";
    db.Flush();
    if (!config_.keep_intermediate_files) {
        FMA_DBG() << "Deleting tmp files...";
        intermediate_file_.CleanTempFiles();
    }
    import_index();
    FMA_LOG() << "Import finished in " << fma_common::GetTime() - t1 << " seconds.";
}

inline std::string LimitedLengthStr(const std::string& s, size_t max_size = 1024) {
    return s.size() > max_size ? s.substr(0, max_size) + "..." : s;
}

/**
 * Loads vertex file located in path and write into intermediate file.
 *
 * @param path  Full pathname of the file.
 * @param desc  File description
 */
void lgraph::import_v2::Importer::LoadVertexFiles(LightningGraph* db,
                                                  const std::vector<CsvDesc*>& files,
                                                  const LabelDesc& ld) {
    FMA_ASSERT(!files.empty());
    const std::string& label = ld.name;
    FieldType type = ld.GetPrimaryField().type;
    // the whole process is organized as a pipeline of multiple stages
    //   ColumnParser -> [std::pair<VidType,
    //   std::vector<std::vector<FieldData>>>]
    //      -> Tailer -> [std::vector<VertexDataWithVid>]
    //      -> DataFileWriter -> void
    VidType first_vid = next_vid_;
    vid_tables_.StartVidTable(label, type, next_vid_);
    VidTableInterface* vid_table = vid_tables_.GetVidTable(label);
    intermediate_file_.StartWritingVertex(label, next_vid_);
    BufferedBlobWriter blob_writer(db, 1 << 20);
    for (auto& desc : files) {
        CsvDesc fd = *desc;
        FMA_LOG() << "\tFile [" << desc->path << "]";
        FMA_ASSERT(desc->is_vertex_file);
        std::vector<FieldSpec> fts;
        {
            auto txn = db->CreateReadTxn();
            fd.GenFieldSpecs(txn, fts);
            txn.Abort();
        }
        int64_t max_err_msgs = config_.quiet ? 0 : 100;
        std::unique_ptr<BlockParser> parser;
        if (desc->data_format == "CSV") {
            parser.reset(new ColumnParser(
                desc->path, fts, config_.parse_block_size, config_.n_parser_threads,
                desc->n_header_line, config_.continue_on_error, config_.delimiter, max_err_msgs));
        } else {
            parser.reset(new JsonLinesParser(desc->path, fts, config_.parse_block_size,
                                             config_.n_parser_threads, desc->n_header_line,
                                             config_.continue_on_error, max_err_msgs));
        }
        // ColumnParser parser(desc->path, fts, config_.parse_block_size, config_.n_parser_threads,
        //                    desc->n_header_line, config_.continue_on_error, config_.delimiter,
        //                    max_err_msgs);
        fma_common::PipelineStage<std::vector<ImportDataFile::VertexDataWithVid>, void> data_writer(
            [this](const std::vector<ImportDataFile::VertexDataWithVid>& vs) {
                intermediate_file_.WriteVertexes(vs);
            },
            nullptr,  // threadpool
            0,        // priority
            1,        // max active tasks
            1);       // buffer size
        // stitch the FieldData into a binary block and get vid, then pass to file
        // writer
        size_t key_col_id = desc->FindIdxExcludeSkip(ld.GetPrimaryField().name);
        bool id_is_string = ld.GetPrimaryField().type == FieldType::STRING;

        Schema schema;
        std::vector<size_t> field_ids;
        std::vector<std::string> prop_names;  // property names
        for (auto v : fd.columns)
            if (v != KeyWordFunc::GetStrFromKeyWord(KeyWord::SKIP)) prop_names.push_back(v);

        {
            auto txn = db->CreateReadTxn();
            size_t label_id = txn.GetLabelId(true, label);
            field_ids = txn.GetFieldIds(true, label_id, prop_names);
            schema = *txn.GetSchemaInfo().v_schema_manager.GetSchema(label_id);
            txn.Abort();
        }

        // input is pair<start_vid, vertex fields>, output is vertex data
        fma_common::PipelineStage<std::pair<VidType, std::vector<std::vector<FieldData>>>,
                                  std::vector<ImportDataFile::VertexDataWithVid>>
            stitch(
                [&](std::pair<VidType, std::vector<std::vector<FieldData>>>&& start_vid_vs)
                    -> std::vector<ImportDataFile::VertexDataWithVid> {
                    VidType start_vid = start_vid_vs.first;
                    auto& vs = start_vid_vs.second;
                    std::vector<ImportDataFile::VertexDataWithVid> ret;
                    ret.reserve(vs.size());
                    std::function<Value(std::vector<FieldData> &&)> create_record_func;
                    if (schema.HasBlob()) {
                        create_record_func = [&](std::vector<FieldData>&& vs) {
                            return schema.CreateRecordWithBlobs(
                                field_ids.size(), field_ids.data(), vs.data(),
                                [&](const Value& blob) {
                                    return blob_writer.AddBlob(blob.MakeCopy());
                                });
                        };
                    } else {
                        create_record_func = [&](std::vector<FieldData>&& vs) {
                            return schema.CreateRecord(field_ids.size(), field_ids.data(),
                                                       vs.data());
                        };
                    }
                    {
                        for (auto& v : vs) {
                            const FieldData& key = v[key_col_id];
                            // check null key
                            if (key.IsNull() || key.is_empty_buf()) {
                                OnMissingUidOffline(desc->path, key.ToString(), fts, v,
                                                    config_.continue_on_error);
                                continue;
                            }
                            // check super long key
                            if (id_is_string &&
                                key.AsString().size() > lgraph::_detail::MAX_KEY_SIZE) {
                                OnErrorOffline(
                                    "Id string too long: " + key.AsString().substr(0, 1024),
                                    config_.continue_on_error);
                                continue;
                            }
                            // try to assign a vid, check if duplicate
                            if (!vid_table->AddKey(key, start_vid)) {
                                if (!config_.quiet || !config_.continue_on_error) {
                                    OnErrorOffline(FMA_FMT("When parsing file [{}]:\n"
                                                           "\tDuplicate vertex uid: [{}].",
                                                           desc->path, key.ToString()),
                                                   config_.continue_on_error);
                                }
                            } else {
                                try {
                                    // stitch properties together
                                    Value record = create_record_func(std::move(v));
                                    ret.emplace_back(record.AsString(), start_vid);
                                    start_vid++;
                                } catch (std::exception& e) {
                                    OnErrorOffline(fma_common::StringFormatter::Format(
                                                       "Failed to pack data fields into a record: "
                                                       "{}.\nField data is:\n[{}]",
                                                       e.what(),
                                                       LimitedLengthStr(fma_common::ToString(v))),
                                                   config_.continue_on_error);
                                }
                            }
                        }
                    }
                    return ret;
                },
                nullptr,             // threadpool
                0,                   // priority
                config_.n_stichers,  // max active tasks
                config_.n_stichers,  // buffer size
                false,               // out of core
                &data_writer);
        // now pull data from parser and send it to the stitchers
        std::vector<std::vector<FieldData>> block;
        try {
            double t1 = fma_common::GetTime();
            size_t nblock = 0;
            size_t nlines = 0;
            while (parser->ReadBlock(block)) {
                nlines += block.size();
                VidType start_vid = 0;
                {
                    start_vid = next_vid_;
                    next_vid_ += (VidType)block.size();
                }
                stitch.Push(std::make_pair(start_vid, std::move(block)));
                if (++nblock % 10 == 0) {
                    double t2 = fma_common::GetTime();
                    double percent = (double)(nblock * config_.parse_block_size) * 100 /
                                     std::max<size_t>(desc->size, 1);
                    FMA_LOG() << "\t " << percent << "% - Read " << nlines << " lines at "
                              << (double)nlines / (t2 - t1) / 1000 << " KLine/S";
                }
            }
        } catch (const std::exception& e) {
            std::throw_with_nested(
                InputError("Failed to parse file [" + desc->path + "]," + e.what()));
        } catch (...) {
            std::throw_with_nested(InputError("Failed to parse file [" + desc->path + "]"));
        }
        // wait for finish
        stitch.WaitTillClear();
        data_writer.WaitTillClear();
    }
    size_t n_bucket = n_buckets_[label];
    FMA_CHECK_GE(next_vid_, first_vid);
    intermediate_file_.EndWritingVertexAndSetBucketSize(
        label, next_vid_, (next_vid_ - first_vid + n_bucket - 1) / n_bucket);
    vid_tables_.SealVidTable(label, next_vid_);
}

void lgraph::import_v2::Importer::LoadEdgeFiles(LightningGraph* db, std::string src_label,
                                                std::string dst_label, const LabelDesc ld,
                                                const std::vector<CsvDesc*>& files) {
    FMA_ASSERT(!files.empty());
    const std::string& label = ld.name;

    VidTableInterface* src_table = vid_tables_.GetVidTable(src_label);
    VidTableInterface* dst_table = vid_tables_.GetVidTable(dst_label);
    intermediate_file_.StartWritingEdge(src_label, dst_label);
    BufferedBlobWriter blob_writer(db);
    for (auto& desc : files) {
        CsvDesc fd = *desc;
        FMA_LOG() << "\tFile [" << desc->path << "]";
        FMA_ASSERT(!desc->is_vertex_file);
        std::vector<FieldSpec> fts;
        {
            auto txn = db->CreateReadTxn();
            fd.GenFieldSpecs(txn, fts);
            txn.Abort();
        }
        int64_t max_err_msgs = config_.quiet ? 0 : 100;

        std::unique_ptr<BlockParser> parser;
        if (desc->data_format == "CSV") {
            parser.reset(new ColumnParser(
                desc->path, fts, config_.parse_block_size, config_.n_parser_threads,
                desc->n_header_line, config_.continue_on_error, config_.delimiter, max_err_msgs));
        } else {
            parser.reset(new JsonLinesParser(desc->path, fts, config_.parse_block_size,
                                             config_.n_parser_threads, desc->n_header_line,
                                             config_.continue_on_error, max_err_msgs));
        }

        // ColumnParser parser(desc->path, fts, config_.parse_block_size, config_.n_parser_threads,
        //                    desc->n_header_line, config_.continue_on_error, config_.delimiter,
        //                    max_err_msgs);
        // data file writer writes the parsed EdgeData into intermediate files
        fma_common::PipelineStage<
            std::pair<std::vector<ImportDataFile::EdgeData>, std::vector<ImportDataFile::EdgeData>>,
            void>
            data_writer(
                [this](const std::pair<std::vector<ImportDataFile::EdgeData>,
                                       std::vector<ImportDataFile::EdgeData>>& vs) {
                    intermediate_file_.WriteEdges(vs.first, vs.second);
                },
                nullptr,  // threadpool
                0,        // priority
                1,        // max active tasks
                1,        // buffer size
                false);   // out of order
                          // stitch the FieldData into a binary block and get src_id, dst_id, then
                          // pass to file writer
        std::vector<std::string> prop_names;  // property names
        size_t src_id_pos = fd.GetEdgeSrcColumnIDExcludeSkip();
        size_t dst_id_pos = fd.GetEdgeDstColumnIDExcludeSkip();
        for (auto v : fd.columns)
            if (v != KeyWordFunc::GetStrFromKeyWord(KeyWord::SKIP) &&
                v != KeyWordFunc::GetStrFromKeyWord(KeyWord::SRC_ID) &&
                v != KeyWordFunc::GetStrFromKeyWord(KeyWord::DST_ID)) {
                prop_names.push_back(v);
            }

        size_t first_id_pos = std::min(src_id_pos, dst_id_pos);
        size_t second_id_pos = std::max(src_id_pos, dst_id_pos);
        size_t label_id;
        bool has_tid = ld.HasTemporalField();
        TemporalFieldOrder tid_order = TemporalFieldOrder::ASC;
        int primary_id_pos = -1;
        if (has_tid) {
            auto tf = ld.GetTemporalField();
            tid_order = tf.temporal_order;
            primary_id_pos = (int)fd.FindIdxExcludeSkip(tf.name);
        }
        std::vector<size_t> field_ids;
        Schema schema;
        {
            auto txn = db->CreateReadTxn();
            label_id = txn.GetLabelId(false, label);
            field_ids = txn.GetFieldIds(false, label_id, prop_names);
            schema = *txn.GetSchemaInfo().e_schema_manager.GetSchema(label_id);
            txn.Abort();
        }
        std::vector<size_t> unique_index_pos;
        for (auto & cs : ld.GetColumnSpecs()) {
            if (cs.index && (cs.idxType == lgraph::IndexType::PairUniqueIndex)) {
                unique_index_pos.push_back(desc->FindIdxExcludeSkip(cs.name));
            }
        }
        cuckoohash_map<std::string, char> unique_index_keys;
        // stitcher takes vectors of vector<FieldData>, which contains field
        // values, and returns a pair of (vectors of out-edges) and (vector of
        // in-edges)
        fma_common::PipelineStage<
            std::vector<std::vector<FieldData>>,
            std::pair<std::vector<ImportDataFile::EdgeData>, std::vector<ImportDataFile::EdgeData>>>
            stitch(
                [&](std::vector<std::vector<FieldData>>&& edges) {
                    std::pair<std::vector<ImportDataFile::EdgeData>,
                              std::vector<ImportDataFile::EdgeData>>
                        ret;
                    ret.first.reserve(edges.size());
                    ret.second.reserve(edges.size());
                    std::function<Value(std::vector<FieldData> &&)> create_record_func;
                    if (schema.HasBlob()) {
                        create_record_func = [&](std::vector<FieldData>&& vs) {
                            return schema.CreateRecordWithBlobs(
                                field_ids.size(), field_ids.data(), vs.data(),
                                [&](const Value& blob) {
                                    return blob_writer.AddBlob(blob.MakeCopy());
                                });
                        };
                    } else {
                        create_record_func = [&](std::vector<FieldData>&& vs) {
                            return schema.CreateRecord(field_ids.size(), field_ids.data(),
                                                       vs.data());
                        };
                    }
                    for (auto& edge : edges) {
                        const FieldData& src_id = edge[src_id_pos];
                        const FieldData& dst_id = edge[dst_id_pos];
                        TemporalId tid = 0;
                        if (has_tid) {
                            tid = Transaction::ParseTemporalId(edge[primary_id_pos], tid_order);
                        }
                        VidType src_vid, dst_vid;
                        bool resolve_success = true;
                        // try to translate src and dst id into vid
                        {
                            if (!src_table->GetVid(src_id, src_vid)) {
                                resolve_success = false;
                                OnMissingUidOffline(desc->path, src_id.ToString(), fts, edge,
                                                    config_.continue_on_error);
                            }
                            if (!dst_table->GetVid(dst_id, dst_vid)) {
                                resolve_success = false;
                                OnMissingUidOffline(desc->path, dst_id.ToString(), fts, edge,
                                                    config_.continue_on_error);
                            }
                        }
                        {
                            for (const auto& pos : unique_index_pos) {
                                const FieldData& unique_index_col = edge[pos];
                                if (unique_index_col.IsNull() ||
                                    unique_index_col.is_empty_buf()) {
                                    OnErrorOffline("Invalid unique index key",
                                                   config_.continue_on_error);
                                    continue;
                                }
                                if (unique_index_col.IsString() &&
                                    unique_index_col.string().size() >
                                        lgraph::_detail::MAX_KEY_SIZE) {
                                    OnErrorOffline("Unique index string key is too long: "
                                                   + unique_index_col.string().substr(0, 1024),
                                                   config_.continue_on_error);
                                    continue;
                                }
                                std::string unique_key;
                                unique_key.append((const char*)&pos, sizeof(pos));
                                unique_key.append(
                                    (const char*)&(src_vid < dst_vid ? src_vid : dst_vid),
                                    sizeof(VertexId));
                                unique_key.append(
                                    (const char*)&(src_vid > dst_vid ? src_vid : dst_vid),
                                    sizeof(VertexId));
                                lgraph::import_v3::AppendFieldData(unique_key, unique_index_col);
                                if (!unique_index_keys.insert(unique_key, 0)) {
                                    OnErrorOffline("Duplicate unique index field: " +
                                                   unique_index_col.ToString(),
                                                   config_.continue_on_error);
                                    resolve_success = false;
                                    break;
                                }
                            }
                        }
                        if (resolve_success) {
                            // remove src_id and dst_id from field_data
                            for (size_t i = first_id_pos; i < second_id_pos - 1; i++) {
                                edge[i] = std::move(edge[i + 1]);
                            }
                            for (size_t i = second_id_pos - 1; i < edge.size() - 2; i++) {
                                edge[i] = std::move(edge[i + 2]);
                            }
                            edge.resize(edge.size() - 2);
                            FMA_DBG_CHECK_EQ(edge.size(), field_ids.size());
                            try {
                                // stitch properties together
                                Value record = create_record_func(std::move(edge));
                                ret.first.emplace_back(src_vid, static_cast<LabelId>(label_id), tid,
                                                       dst_vid, record.AsString());
                                ret.second.emplace_back(dst_vid, static_cast<LabelId>(label_id),
                                                        tid, src_vid, record.AsString());
                            } catch (std::exception& e) {
                                OnErrorOffline(fma_common::StringFormatter::Format(
                                                   "Failed to pack data fields into a record: "
                                                   "{}.\nField data is:\n[{}]",
                                                   e.what(),
                                                   LimitedLengthStr(fma_common::ToString(edge))),
                                               config_.continue_on_error);
                            }
                        }
                    }
                    return ret;
                },
                nullptr,             // threadpool
                0,                   // priority
                config_.n_stichers,  // max active tasks
                config_.n_stichers,  // buffer size
                false,               // out of core
                &data_writer);
        // now pull data from parser and send it to the stitchers
        std::vector<std::vector<FieldData>> block;
        try {
            double t1 = fma_common::GetTime();
            size_t nblock = 0;
            size_t nlines = 0;
            while (parser->ReadBlock(block)) {
                nlines += block.size();
                stitch.Push(std::move(block));
                if (++nblock % 10 == 0) {
                    double t2 = fma_common::GetTime();
                    double percent = (double)(nblock * config_.parse_block_size) * 100 / desc->size;
                    FMA_LOG() << "\t " << percent << "% - Read " << nlines << " lines at "
                              << (double)nlines / (t2 - t1) / 1000 << " KLine/S";
                }
            }
        } catch (...) {
            std::throw_with_nested(InputError("Failed to parse file [" + desc->path + "]"));
        }
        // wait for finish
        stitch.WaitTillClear();
        data_writer.WaitTillClear();
    }
    intermediate_file_.EndWritingEdge();
}

template <lgraph::FieldType FT>
void lgraph::import_v2::Importer::DumpIndexOutOfCore(LightningGraph* db, const std::string& label,
                                                     const std::string& field) {
    typedef typename FieldType2VidType<FT>::type KeyT;
    SingleVidTable<FT>* vtab = dynamic_cast<SingleVidTable<FT>*>(vid_tables_.GetVidTable(label));
    VidType start_id = vid_tables_.GetStartVid(label);
    VidType end_id = vid_tables_.GetEndVid(label);
    std::vector<KeyVidValid<KeyT>> key_vids;
    key_vids.resize(end_id - start_id);
    auto& tables = vtab->GetTables();
    for (auto& tab : tables) {
        cuckoohash_map_foreach(tab, [&](const std::pair<KeyT, VidType>& p) {
            auto& kv = key_vids[p.second - start_id];
            kv.key = p.first;
            kv.vid = p.second;
            kv.is_valid = true;
        });
    }
    FMA_DBG() << "Sorting by unique id";
    LGRAPH_PSORT(key_vids.begin(), key_vids.end());
    FMA_DBG() << "Dumping index";
    Transaction txn = db->CreateWriteTxn();
    auto index = txn.GetVertexIndex(label, field);
    for (auto& kv : key_vids) {
        if (kv.is_valid) {
            index->_AppendVertexIndexEntry(txn.GetTxn(), GetConstRef(kv.key), (VertexId)kv.vid);
        }
    }
    txn.Commit();
}

void lgraph::import_v2::Importer::WriteVertex(LightningGraph* db, const std::string& label,
                                              const std::string& key_field, FieldType key_type) {
    // Writes a vertex label into db, including vertex data and edges, and
    // indexes index is build first, so we can delete vid_table before sorting
    // buckets to reduce memory pressure
    //
    // writing data is organized as a pipeline of:
    //     [bucket_id] -> reader reads vertex and edge data -> [VertexBucket]
    //         -> sorter sorts data and pack them into kvs ->
    //         [vector<PackedKVs>]
    //             -> committer writes to db

    // build index
    FMA_DBG() << "Building index for " << label;
    // make index
    db->_AddEmptyIndex(label, key_field, lgraph::IndexType::GlobalUniqueIndex, true);
    switch (key_type) {
    case FieldType::NUL:
        FMA_ASSERT(false);
    case FieldType::BOOL:
        DumpIndexOutOfCore<FieldType::BOOL>(db, label, key_field);
        break;
    case FieldType::INT8:
        DumpIndexOutOfCore<FieldType::INT8>(db, label, key_field);
        break;
    case FieldType::INT16:
        DumpIndexOutOfCore<FieldType::INT16>(db, label, key_field);
        break;
    case FieldType::INT32:
        DumpIndexOutOfCore<FieldType::INT32>(db, label, key_field);
        break;
    case FieldType::INT64:
        DumpIndexOutOfCore<FieldType::INT64>(db, label, key_field);
        break;
    case FieldType::FLOAT:
        DumpIndexOutOfCore<FieldType::FLOAT>(db, label, key_field);
        break;
    case FieldType::DOUBLE:
        DumpIndexOutOfCore<FieldType::DOUBLE>(db, label, key_field);
        break;
    case FieldType::DATE:
        DumpIndexOutOfCore<FieldType::DATE>(db, label, key_field);
        break;
    case FieldType::DATETIME:
        DumpIndexOutOfCore<FieldType::DATETIME>(db, label, key_field);
        break;
    case FieldType::STRING:
        DumpIndexOutOfCore<FieldType::STRING>(db, label, key_field);
        break;
    case FieldType::BLOB:
        FMA_ASSERT(false) << "Blob fields cannot be indexed.";
        break;
    default:
        FMA_ASSERT(false);
    }

    FMA_DBG() << "Writing vertex data";
    VidType start_id = vid_tables_.GetStartVid(label);
    VidType end_id = vid_tables_.GetEndVid(label);
    vid_tables_.DeleteVidTable(label);
    // start dumping data
    size_t total_committed = 0;
    // load back the vertex data and edge data
    intermediate_file_.StartReading(label, start_id, end_id, config_.n_ireaders, config_.n_sorters);
    size_t n_vertices = std::max<size_t>(end_id - start_id, 1);
    // committer writes the kvs directly to db
    fma_common::PipelineStage<PackedKVs, void> committer(
        [&](PackedKVs&& data) {
            double t1 = fma_common::GetTime();
            Transaction txn = db->CreateWriteTxn();
            for (size_t i = 0; i < data.size(); i++) {
                auto& d = data[i];
                for (auto& kv : d) {
                    txn.ImportAppendDataRaw(kv.first, kv.second);
                }
            }
            txn.RefreshNextVid();
            txn.Commit();
            total_committed += data.size();
            data.clear();
            double t2 = fma_common::GetTime();
            double percent = (double)total_committed * 100 / n_vertices;
            FMA_LOG() << "\t " << percent << "% - Committed " << total_committed
                      << ", time to write batch=" << t2 - t1;
        },
        nullptr, 0, 1, 1);
    // packer packs vertex property, out-edges and in-edges into KVs
    fma_common::PipelineStage<ImportDataFile::BucketData, PackedKVs> packer(
        [&](ImportDataFile::BucketData&& data) -> PackedKVs {
            std::vector<ImportDataFile::VertexDataWithVid>& vds = data.vdata;
            PackedKVs packed_data;
            packed_data.reserve(vds.size());
            if (vds.size() == 0) {
                return packed_data;
            }
            FMA_DBG() << "start packing " << vds.front().vid;
            auto& oes = data.oes;
            auto& ies = data.ins;
            auto oeit = oes.begin();
            auto ieit = ies.begin();
            for (size_t i = 0; i < vds.size(); i++) {
                VidType vid = vds[i].vid;
                auto& vprop = vds[i].data;
                std::deque<std::tuple<LabelId, TemporalId, VertexId, DenseString>> outs;
                std::deque<std::tuple<LabelId, TemporalId, VertexId, DenseString>> ins;
                if (oeit != oes.end() && oeit->vid1 < vid) {
                    OnMissingVertexOffline(label, oeit->vid1, config_.continue_on_error);
                }
                if (ieit != ies.end() && ieit->vid1 < vid) {
                    OnMissingVertexOffline(label, ieit->vid1, config_.continue_on_error);
                }
                while (oeit != oes.end() && oeit->vid1 == vid) {
                    outs.emplace_back(oeit->lid, oeit->tid, oeit->vid2, std::move(oeit->prop));
                    oeit++;
                }
                while (ieit != ies.end() && ieit->vid1 == vid) {
                    ins.emplace_back(ieit->lid, ieit->tid, ieit->vid2, std::move(ieit->prop));
                    ieit++;
                }
                packed_data.emplace_back(
                    MakeDataKvs(vid, std::move(vprop), std::move(outs), std::move(ins)));
            }
            FMA_DBG_ASSERT(oeit == oes.end());
            FMA_DBG_ASSERT(ieit == ies.end());
            FMA_DBG() << "end packing " << vds.front().vid;
            vds.clear();
            ies.clear();
            oes.clear();
            return packed_data;
        },
        nullptr, 0, config_.n_packers, config_.n_packers, false, &committer);
    // bucket reader reads a bucket and send it to sorter
    ImportDataFile::BucketData bucket;
    while (intermediate_file_.ReadBucket(bucket)) {
        packer.Push(std::move(bucket));
    }
    // now push tasks and wait
    packer.WaitTillClear();
    committer.WaitTillClear();
    intermediate_file_.EndReadingVertex();
}

lgraph::import_v2::Importer::DataForOneVertex lgraph::import_v2::Importer::MakeDataKvs(
    VertexId vid, const DenseString& vdata,
    std::deque<std::tuple<LabelId, TemporalId, VertexId, DenseString>>&& outs,
    std::deque<std::tuple<LabelId, TemporalId, VertexId, DenseString>>&& ins) {
    using namespace lgraph::graph;
    typedef std::deque<std::tuple<LabelId, TemporalId, VertexId, DenseString>>::iterator IT;
    // calculate total size, this is just a hint, not very accurate
    size_t total_size = vdata.size();
    for (auto& e : outs) {
        total_size += std::get<3>(e).size() + 16;
    }
    for (auto& e : ins) {
        total_size += std::get<3>(e).size() + 16;
    }

    DataForOneVertex ret;
    if (total_size < ::lgraph::_detail::NODE_SPLIT_THRESHOLD) {
        VertexValue vov(GetConstRef(vdata));
        LabelId lid = 0;
        VertexId vid2 = -1;
        TemporalId tid = 0;
        EdgeId eid = -1;
        IT next_beg;
        EdgeValue oev(outs.begin(), outs.end(), lid, tid, vid2, eid, next_beg);
        FMA_DBG_ASSERT(next_beg == outs.end());
        vid2 = -1;
        EdgeValue iev(ins.begin(), ins.end(), lid, tid, vid2, eid, next_beg);
        FMA_DBG_ASSERT(next_beg == ins.end());
        Value val;
        PackedDataValue::PackData(vov, oev, iev, val);
        ret.emplace_back(KeyPacker::CreatePackedDataKey(vid), std::move(val));
    } else {
        ret.emplace_back(KeyPacker::CreateVertexOnlyKey(vid), Value::MakeCopy(GetConstRef(vdata)));
        // make oevs
        LabelId last_lid = 0;
        TemporalId last_tid = -1;
        VertexId last_dst = -1;
        EdgeId last_eid = -1;
        IT start = outs.begin();
        IT next_start;
        while (start != outs.end()) {
            EdgeValue oev(start, outs.end(), last_lid, last_tid, last_dst, last_eid, next_start);
            ret.emplace_back(oev.CreateOutEdgeKey(vid), std::move(oev.GetBuf()));
            start = next_start;
        }
        // make ievs
        last_lid = 0;
        last_tid = -1;
        last_dst = -1;
        last_eid = -1;
        start = ins.begin();
        while (start != ins.end()) {
            EdgeValue iev(start, ins.end(), last_lid, last_tid, last_dst, last_eid, next_start);
            ret.emplace_back(iev.CreateInEdgeKey(vid), std::move(iev.GetBuf()));
            start = next_start;
        }
    }
    return ret;
}

std::vector<lgraph::import_v2::PlanExecutor::Action> lgraph::import_v2::Importer::MakePlan(
    const std::map<std::string, std::vector<CsvDesc*>>& v_label_files,
    const std::map<std::pair<std::string, std::string>,
                   std::map<std::string, std::vector<CsvDesc*>>>& src_dst_files,
    std::vector<std::string>& vid_label) {
    vid_label.clear();
    size_t nv = v_label_files.size();
    LabelGraph graph(nv);
    std::map<std::string, size_t> label_vid;
    size_t vid = 0;
    for (auto& kv : v_label_files) {
        label_vid[kv.first] = vid++;
        vid_label.push_back(kv.first);
        auto& vlabel = kv.first;
        size_t vfsize = 0;
        for (auto& f : kv.second) {
            vfsize += f->size;
        }
        graph.AddVFileSize(label_vid[vlabel], vfsize);
    }
    for (auto& src_dst : src_dst_files) {
        auto& src = src_dst.first.first;
        auto& dst = src_dst.first.second;
        size_t src_id = label_vid[src];
        size_t dst_id = label_vid[dst];
        size_t efsize = 0;
        for (auto& edges : src_dst.second) {
            for (auto& f : edges.second) {
                efsize += f->size;
            }
        }
        graph.AddEFileSize(src_id, dst_id, efsize);
    }

    size_t max_memory;
    auto plan = graph.CalculateBestPlan(max_memory);
    PlanExecutor executor(plan, graph.GetEFileSize());
    return executor.GetActions();
}

void lgraph::import_v2::Importer::OnErrorOffline(const std::string& msg, bool continue_on_error) {
    FMA_WARN() << msg;
    if (!continue_on_error) {
        FMA_ERR() << "If you wish to ignore the errors, use "
                     "--continue_on_error true";
        exit(-1);
    }
}

void lgraph::import_v2::Importer::OnMissingUidOffline(const std::string& file,
                                                      const std::string& uid,
                                                      const std::vector<FieldSpec>& fs,
                                                      const std::vector<lgraph::FieldData>& fd,
                                                      bool continue_on_error) {
    if (config_.continue_on_error && config_.quiet) return;
    std::string msg = fma_common::StringFormatter::Format(
        "When parsing file [{}]:\n"
        "\tMissing vertex uid: [{}].\n\tError line fields: [",
        file, uid);
    for (size_t i = 0, j = 0; i < fs.size(); i++) {
        if (fs[i].type != FieldType::NUL) {
            FMA_ASSERT(i < fs.size() && j < fd.size());
            msg.append(fs[i].name).append(":").append(fd[j].ToString());
            if (j != fd.size() - 1) msg.append(",");
            j++;
        }
    }
    msg.append("]\n");
    OnErrorOffline(msg, continue_on_error);
}

void lgraph::import_v2::Importer::OnMissingVertexOffline(const std::string& label, VidType vid,
                                                         bool continue_on_error) {
    if (config_.continue_on_error && config_.quiet) return;
    OnErrorOffline(
        fma_common::StringFormatter::Format(
            "Vertex [{}] with vid={} does not exist, yet there are edges referencing this vertex.",
            label, vid),
        continue_on_error);
}
