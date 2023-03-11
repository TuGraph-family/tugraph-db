/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include <memory>
#include "rocksdb/sst_file_writer.h"
#include "import/import_v3.h"
#include "import/dense_string.h"
#include "import/import_config_parser.h"
#include "import/blob_writer.h"
#include "db/galaxy.h"

namespace lgraph {
namespace import_v3 {

#define InvalidVid std::numeric_limits<VertexId>::max()

Importer::Importer(Config config)
    : config_(std::move(config)), next_vid_(0), next_eid_(0), db_(nullptr) {
    sst_files_path_ = config_.intermediate_dir + "/sst";
    rocksdb_path_ = config_.intermediate_dir + "/db";
    vid_path_ = config_.intermediate_dir + "/vid";
}

void Importer::OnErrorOffline(const std::string& msg) {
    if (config_.continue_on_error && config_.quiet) return;
    FMA_WARN() << msg;
    if (!config_.continue_on_error) {
        FMA_ERR() << "If you wish to ignore the errors, use "
                     "--continue_on_error true";
        exit(-1);
    }
}

void Importer::AppendFieldData(std::string& ret, const FieldData& data) {
    switch (data.getType()) {
    case FieldType::NUL:
        FMA_ASSERT(false);
        break;
    case FieldType::BOOL:
        {
            auto val = field_data_helper::GetStoredValue<FieldType::BOOL>(data);
            encodeNumToStr(val, ret);
        }
        break;
    case FieldType::FLOAT:
        {
            auto val = field_data_helper::GetStoredValue<FieldType::FLOAT>(data);
            encodeNumToStr(val, ret);
        }
        break;
    case FieldType::DOUBLE:
        {
            auto val = field_data_helper::GetStoredValue<FieldType::DOUBLE>(data);
            encodeNumToStr(val, ret);
        }
        break;
    case FieldType::DATE:
        {
            auto val = field_data_helper::GetStoredValue<FieldType::DATE>(data);
            encodeNumToStr(val, ret);
        }
        break;
    case FieldType::DATETIME:
        {
            auto val = field_data_helper::GetStoredValue<FieldType::DATETIME>(data);
            encodeNumToStr(val, ret);
        }
        break;
    case FieldType::INT64:
        {
            auto val = field_data_helper::GetStoredValue<FieldType::INT64>(data);
            encodeNumToStr(val, ret);
        }
        break;
    case FieldType::INT32:
        {
            auto val = field_data_helper::GetStoredValue<FieldType::INT32>(data);
            encodeNumToStr(val, ret);
        }
        break;
    case FieldType::INT16:
        {
            auto val = field_data_helper::GetStoredValue<FieldType::INT16>(data);
            encodeNumToStr(val, ret);
        }
        break;
    case FieldType::INT8:
        {
            auto val = field_data_helper::GetStoredValue<FieldType::INT8>(data);
            encodeNumToStr(val, ret);
        }
        break;
    case FieldType::STRING:
        {
            const auto& val = field_data_helper::GetStoredValue<FieldType::STRING>(data);
            ret.append(val);
        }
        break;
    case FieldType::BLOB:
        {
            const auto& val = field_data_helper::GetStoredValue<FieldType::BLOB>(data);
            ret.append(val);
        }
        break;
    default:
        FMA_ASSERT(false);
        break;
    }
}

void Importer::DoImportOffline() {
    double t1 = fma_common::GetTime();
    bool empty_db = !fma_common::file_system::DirExists(config_.db_dir + "/.meta");
    std::shared_ptr<GlobalConfig> gc = std::make_shared<GlobalConfig>();
    gc->ft_index_options.enable_fulltext_index = config_.enable_fulltext_index;
    gc->ft_index_options.fulltext_analyzer = config_.fulltext_index_analyzer;
    gc->ft_index_options.fulltext_commit_interval = 0;
    gc->ft_index_options.fulltext_refresh_interval = 0;
    Galaxy galaxy(Galaxy::Config{config_.db_dir, false, true, "fma.ai"}, true, gc);
    auto db = OpenGraph(galaxy, empty_db);
    db_ = &db;
    std::ifstream ifs(config_.config_file);
    nlohmann::json conf;
    ifs >> conf;

    schemaDesc_ = import_v2::ImportConfParser::ParseSchema(conf);
    data_files_ = import_v2::ImportConfParser::ParseFiles(conf);
    for (auto& file : data_files_) {
        if (!file.is_vertex_file) {
            if (!schemaDesc_.FindEdgeLabel(file.label)
                     .MeetEdgeConstraints(file.edge_src.label, file.edge_dst.label)) {
                throw std::runtime_error(FMA_FMT("{} not meet the edge constraints", file.path));
            }
            file.edge_src.id = schemaDesc_.FindVertexLabel(
                                              file.edge_src.label).GetPrimaryColumn().name;
            file.edge_dst.id = schemaDesc_.FindVertexLabel(
                                              file.edge_dst.label).GetPrimaryColumn().name;
        }
        import_v2::ImportConfParser::CheckConsistent(schemaDesc_, file);
    }

    for (auto& v : schemaDesc_.label_desc) {
        // create labels
        auto m = v.GetSchemaDef();
        std::vector<FieldSpec> fds;
        for (auto& p : m) fds.emplace_back(p.second);
        bool ok = db_->AddLabel(v.is_vertex, v.name, fds,
                              v.HasPrimaryColumn() ? v.GetPrimaryColumn().name : "",
                              v.edge_constraints);
        if (ok) {
            FMA_LOG() << FMA_FMT("Add {} label:{} success", v.is_vertex ? "vertex" : "edge",
                                 v.name);
        } else {
            throw std::runtime_error(
                FMA_FMT("Add {} label:{} error", v.is_vertex ? "vertex" : "edge", v.name));
        }
    }

    auto& fs = fma_common::FileSystem::GetFileSystem(config_.intermediate_dir);
    fs.RemoveDir(config_.intermediate_dir);
    if (!fs.Mkdir(config_.intermediate_dir)) {
        throw std::runtime_error(FMA_FMT("mkdir {} failed", config_.intermediate_dir));
    }

    if (!data_files_.empty()) {
        VertexDataToSST();
        EdgeDataToSST();
        VertexPrimaryIndexToLmdb();
        RocksdbToLmdb();
    }

    // rm temp data
    fs.RemoveDir(config_.intermediate_dir);

    {
        // add index
        for (auto& v : schemaDesc_.label_desc) {
            for (auto& spec : v.columns) {
                if (spec.index && !spec.primary) {
                    // create index, ID column has creadted
                    bool ok = db_->AddVertexIndex(v.name, spec.name, spec.unique);
                    if (!ok)
                        throw std::runtime_error(
                            FMA_FMT("Add label [{}] index [{}] error", v.name, spec.name));
                }
                if (spec.fulltext) {
                    bool ok = db_->AddFullTextIndex(v.is_vertex, v.name, spec.name);
                    if (ok) {
                        FMA_LOG() << FMA_FMT("Add fulltext index [label:{}, field:{}] success",
                                             v.name, spec.name);
                    } else {
                        throw std::runtime_error(FMA_FMT(
                            "Add fulltext index [label:{}, field:{}] error", v.name, spec.name));
                    }
                }
            }
        }
        db_->GetLightningGraph()->RebuildAllFullTextIndex();
    }

    FMA_LOG() << "Import finished in " << fma_common::GetTime() - t1 << " seconds.";
}

void Importer::VertexDataToSST() {
    auto t1 = fma_common::GetTime();
    if (!config_.keep_vid_in_memory) {
        rocksdb::Options options;
        options.create_if_missing = true;
        options.create_missing_column_families = true;
        rocksdb::BlockBasedTableOptions table_options;
        table_options.no_block_cache = true;
        options.table_factory.reset(rocksdb::NewBlockBasedTableFactory(table_options));
        options.IncreaseParallelism();
        options.PrepareForBulkLoad();
        rocksdb::DB* db;
        auto s = rocksdb::DB::Open(options, vid_path_, &db);
        if (!s.ok()) {
            throw std::runtime_error(
                FMA_FMT("Opening DB failed, error: {}", s.ToString().c_str()));
        }
        rocksdb_vids_.reset(db);
    }
    struct KV {
        std::string key;
        VertexId vid;
        KV(std::string k, VertexId v): key(std::move(k)), vid(v) {}
    };

    parse_file_threads_ = std::make_unique<boost::asio::thread_pool>(
        config_.parse_file_threads);
    generate_sst_threads_ = std::make_unique<boost::asio::thread_pool>(
        config_.generate_sst_threads);
    struct VertexDataBlock {
        VertexId start_vid = 0;
        uint16_t key_col_id = 0;
        Schema* schema = nullptr;
        std::vector<size_t> field_ids;
        std::vector<std::vector<FieldData>> block;
        LabelId label_id = 0;
    };
    if (!fma_common::file_system::MkDir(sst_files_path_)) {
        throw std::runtime_error(FMA_FMT("failed to mkdir dir : {}", sst_files_path_));
    }
    BufferedBlobWriter blob_writer(db_->GetLightningGraph(), 1 << 20);
    std::atomic<uint64_t> pending_tasks(0);
    for (auto& file : data_files_) {
        // skip edge files
        if (!file.is_vertex_file) {
            continue;
        }
        boost::asio::post(*parse_file_threads_, [this, &blob_writer, &pending_tasks, file](){
            try {
                std::vector<FieldSpec> fts;
                {
                    auto txn = db_->CreateReadTxn();
                    file.GenFieldSpecs(txn, fts);
                    txn.Abort();
                }
                Schema* schema;
                std::vector<size_t> field_ids;
                LabelId label_id;
                std::vector<std::string> prop_names;  // property names
                for (const auto& v : file.columns)
                    if (v != import_v2::KeyWordFunc::GetStrFromKeyWord(import_v2::KeyWord::SKIP))
                        prop_names.push_back(v);
                {
                    auto txn = db_->CreateReadTxn();
                    label_id = txn.GetLabelId(true, file.label);
                    field_ids = txn.GetFieldIds(true, label_id, prop_names);
                    schema = (Schema*)txn.GetSchemaInfo().v_schema_manager.GetSchema(label_id);
                    txn.Abort();
                }
                uint16_t key_col_id = file.FindIdxExcludeSkip(
                    schemaDesc_.FindVertexLabel(file.label).GetPrimaryColumn().name);
                std::unique_ptr<import_v2::BlockParser> parser;
                if (file.data_format == "CSV") {
                    parser.reset(new import_v2::ColumnParser(
                        file.path, fts, config_.parse_block_size, config_.parse_block_threads,
                        file.n_header_line, config_.continue_on_error, config_.delimiter,
                        config_.quiet ? 0 : 100));
                } else {
                    parser.reset(new import_v2::JsonLinesParser(
                        file.path, fts, config_.parse_block_size, config_.parse_block_threads,
                        file.n_header_line, config_.continue_on_error, config_.quiet ? 0 : 100));
                }
                std::vector<std::vector<FieldData>> block;
                while (parser->ReadBlock(block)) {
                    VertexId start_vid = 0;
                    {
                        std::lock_guard<std::mutex> guard(next_vid_lock_);
                        start_vid = next_vid_;
                        next_vid_ += (VertexId)block.size();
                    }
                    auto dataBlock = std::make_unique<VertexDataBlock>();
                    dataBlock->block = std::move(block);
                    dataBlock->start_vid = start_vid;
                    dataBlock->key_col_id = key_col_id;
                    dataBlock->schema = schema;
                    dataBlock->field_ids = field_ids;
                    dataBlock->label_id = boost::endian::native_to_big(label_id);
                    while (pending_tasks > 1) {
                        fma_common::SleepUs(1000);
                    }
                    pending_tasks++;
                    boost::asio::post(*generate_sst_threads_, [this, &blob_writer, &pending_tasks,
                                                               dataBlock = std::move(dataBlock)]() {
                        try {
                            pending_tasks--;
                            std::vector<KV> vec_kvs;
                            VertexId start_vid = dataBlock->start_vid;
                            rocksdb::SstFileWriter vertex_sst_writer(rocksdb::EnvOptions(), {},
                                                                   nullptr, false);
                            auto s = vertex_sst_writer.Open(sst_files_path_ + "/vertex_" +
                                                          std::to_string(start_vid));
                            if (!s.ok()) {
                                throw std::runtime_error(
                                    FMA_FMT("Failed to open vertex_sst_writer"));
                            }
                            auto hasBlob = dataBlock->schema->HasBlob();
                            for (auto& line : dataBlock->block) {
                                const FieldData& key_col = line[dataBlock->key_col_id];
                                // check null key
                                if (key_col.IsNull() || key_col.is_empty_buf()) {
                                    OnErrorOffline("Invalid primary key");
                                    continue;
                                }
                                // check super long key
                                if (key_col.IsString() &&
                                    key_col.string().size() > lgraph::_detail::MAX_KEY_SIZE) {
                                    OnErrorOffline("Id string too long: "
                                                   + key_col.string().substr(0, 1024));
                                    continue;
                                }

                                VertexId vid = boost::endian::native_to_big(start_vid++);
                                {
                                    std::string k;
                                    k.append((const char*)&dataBlock->label_id,
                                             sizeof(dataBlock->label_id));
                                    AppendFieldData(k, key_col);
                                    if (config_.keep_vid_in_memory) {
                                        if (!key_vid_maps_.insert(std::move(k), vid)) {
                                            OnErrorOffline("Duplicate primary field: " +
                                                           key_col.ToString());
                                            continue;
                                        }
                                    } else {
                                        vec_kvs.emplace_back(std::move(k), vid);
                                    }
                                }

                                Value value;
                                if (!hasBlob) {
                                    value = dataBlock->schema->CreateRecord(
                                        dataBlock->field_ids.size(), dataBlock->field_ids.data(),
                                        line.data());
                                } else {
                                    value = dataBlock->schema->CreateRecordWithBlobs(
                                        dataBlock->field_ids.size(), dataBlock->field_ids.data(),
                                        line.data(), [&blob_writer](const Value& blob) {
                                            return blob_writer.AddBlob(blob.MakeCopy());
                                        });
                                }
                                s = vertex_sst_writer.Put({(const char*)&vid, sizeof(vid)},
                                                    {value.Data(), value.Size()});
                                if (!s.ok()) {
                                    throw std::runtime_error(
                                        FMA_FMT("vertex_sst_writer.Put error, {}", s.ToString()));
                                }
                            }
                            std::vector<std::vector<FieldData>>().swap(dataBlock->block);
                            s = vertex_sst_writer.Finish();
                            if (!s.ok()) {
                                throw std::runtime_error(
                                    FMA_FMT("vertex_sst_writer.Finish error, {}", s.ToString()));
                            }

                            if (!config_.keep_vid_in_memory) {
                                rocksdb::SstFileWriter vid_sst_writer(rocksdb::EnvOptions(), {},
                                                                         nullptr, false);
                                s = vid_sst_writer.Open(
                                    sst_files_path_ + "/vid_" +
                                    std::to_string(dataBlock->start_vid));
                                if (!s.ok()) {
                                    throw std::runtime_error(
                                        FMA_FMT("Failed to open vid_sst_writer"));
                                }
                                LGRAPH_PSORT(vec_kvs.begin(), vec_kvs.end(),
                                             [](const KV& a, const KV& b) {
                                                 return a.key < b.key;
                                             });
                                for (auto& item : vec_kvs) {
                                    s = vid_sst_writer.Put(
                                        item.key, {(const char*)&item.vid, sizeof(item.vid)});
                                    if (!s.ok()) {
                                        throw std::runtime_error(
                                            FMA_FMT("vid_sst_writer.Put error, {}", s.ToString()));
                                    }
                                }
                                std::vector<KV>().swap(vec_kvs);
                                s = vid_sst_writer.Finish();
                                if (!s.ok()) {
                                    throw std::runtime_error(
                                        FMA_FMT("vid_sst_writer.Finish error, {}", s.ToString()));
                                }
                            }
                        } catch (...) {
                            std::lock_guard<std::mutex> guard(exceptions_lock_);
                            exceptions_.push(std::current_exception());
                        }
                    });
                }
            } catch (...) {
                std::lock_guard<std::mutex> guard(exceptions_lock_);
                exceptions_.push(std::current_exception());
            }
        });
    }
    // Blocked waiting for tasks to finish
    parse_file_threads_->join();
    generate_sst_threads_->join();
    if (!exceptions_.empty()) {
        std::rethrow_exception(exceptions_.front());
    }

    if (!config_.keep_vid_in_memory) {
        auto begin = fma_common::GetTime();
        std::vector<std::string> ingest_files;
        for (const auto & entry : std::filesystem::directory_iterator(sst_files_path_)) {
            const auto& path = entry.path().string();
            if (path.find("vid_") != std::string::npos) {
                ingest_files.push_back(path);
            }
        }
        rocksdb::IngestExternalFileOptions op;
        op.move_files = true;
        auto s = rocksdb_vids_->IngestExternalFile(ingest_files, op);
        if (!s.ok()) {
            throw std::runtime_error(
                FMA_FMT("Importing vid files failed, error: {}", s.ToString().c_str()));
        }

        rocksdb::CompactRangeOptions options;
        options.max_subcompactions = 8;
        s = rocksdb_vids_->CompactRange(options, nullptr, nullptr);
        if (!s.ok()) {
            throw std::runtime_error(
                FMA_FMT("vids CompactRange failed, error: {}", s.ToString().c_str()));
        }
        FMA_LOG() << "vids CompactRange, time: " << fma_common::GetTime() - begin << "s";
    }

    auto t2 = fma_common::GetTime();
    FMA_LOG() << "Convert vertex data to sst files, time: " << t2 - t1 << "s";
}

void Importer::EdgeDataToSST() {
    auto t1 = fma_common::GetTime();
    parse_file_threads_ = std::make_unique<boost::asio::thread_pool>(
        config_.parse_file_threads);
    generate_sst_threads_ = std::make_unique<boost::asio::thread_pool>(
        config_.generate_sst_threads);
    struct EdgeDataBlock {
        std::vector<std::vector<FieldData>> block;
        uint64_t start_eid = 0;
        uint16_t src_id_pos = 0;
        uint16_t dst_id_pos = 0;
        LabelId label_id = 0;
        LabelId src_label_id = 0;
        LabelId dst_label_id = 0;
        Schema* schema = nullptr;
    };
    struct KV {
        std::string key;
        Value value;
        KV(std::string k, Value v): key(std::move(k)), value(std::move(v)) {}
    };
    BufferedBlobWriter blob_writer(db_->GetLightningGraph());
    std::atomic<uint64_t> pending_tasks(0);
    std::atomic<uint64_t> sst_file_id(0);
    for (auto& file : data_files_) {
        // skip vertex files
        if (file.is_vertex_file) {
            continue;
        }
        boost::asio::post(*parse_file_threads_,
                          [this, file, &blob_writer, &pending_tasks, &sst_file_id](){
            try {
                std::vector<FieldSpec> fts;
                {
                    auto txn = db_->CreateReadTxn();
                    file.GenFieldSpecs(txn, fts);
                    txn.Abort();
                }
                size_t src_id_pos = file.GetEdgeSrcColumnIDExcludeSkip();
                size_t dst_id_pos = file.GetEdgeDstColumnIDExcludeSkip();
                std::vector<std::string> prop_names;
                for (auto& v : file.columns)
                    if (v != import_v2::KeyWordFunc::GetStrFromKeyWord(import_v2::KeyWord::SKIP) &&
                        v !=
                            import_v2::KeyWordFunc::GetStrFromKeyWord(import_v2::KeyWord::SRC_ID) &&
                        v !=
                            import_v2::KeyWordFunc::GetStrFromKeyWord(import_v2::KeyWord::DST_ID)) {
                        prop_names.push_back(v);
                    }

                LabelId label_id;
                LabelId src_label_id, dst_label_id;
                std::vector<size_t> field_ids;
                Schema* schema;
                {
                    auto txn = db_->CreateReadTxn();
                    label_id = txn.GetLabelId(false, file.label);
                    src_label_id = txn.GetLabelId(true, file.edge_src.label);
                    dst_label_id = txn.GetLabelId(true, file.edge_dst.label);
                    field_ids = txn.GetFieldIds(false, label_id, prop_names);
                    schema = (Schema*)txn.GetSchemaInfo().e_schema_manager.GetSchema(label_id);
                    txn.Abort();
                }
                std::unique_ptr<import_v2::BlockParser> parser;
                if (file.data_format == "CSV") {
                    parser.reset(new import_v2::ColumnParser(
                        file.path, fts, config_.parse_block_size, config_.parse_block_threads,
                        file.n_header_line, config_.continue_on_error, config_.delimiter,
                        config_.quiet ? 0 : 100));
                } else {
                    parser.reset(new import_v2::JsonLinesParser(
                        file.path, fts, config_.parse_block_size, config_.parse_block_threads,
                        file.n_header_line, config_.continue_on_error, config_.quiet ? 0 : 100));
                }

                std::vector<std::vector<FieldData>> block;
                while (parser->ReadBlock(block)) {
                    uint64_t start_eid = 0;
                    {
                        std::lock_guard<std::mutex> guard(next_eid_lock_);
                        start_eid = next_eid_;
                        next_eid_ += (VertexId)block.size();
                    }
                    auto edgeDataBlock = std::make_unique<EdgeDataBlock>();
                    edgeDataBlock->block = std::move(block);
                    edgeDataBlock->start_eid = start_eid;
                    edgeDataBlock->src_id_pos = src_id_pos;
                    edgeDataBlock->dst_id_pos = dst_id_pos;
                    edgeDataBlock->label_id = boost::endian::native_to_big(label_id);
                    edgeDataBlock->src_label_id = boost::endian::native_to_big(src_label_id);
                    edgeDataBlock->dst_label_id = boost::endian::native_to_big(dst_label_id);
                    edgeDataBlock->schema = schema;
                    while (pending_tasks > 1) {
                        fma_common::SleepUs(1000);
                    }
                    pending_tasks++;
                    boost::asio::post(*generate_sst_threads_, [this, &blob_writer, &sst_file_id,
                                                               &pending_tasks, field_ids,
                                                               edgeDataBlock =
                                                                   std::move(edgeDataBlock)]() {
                        try {
                            pending_tasks--;
                            uint64_t num = ++sst_file_id;
                            rocksdb::SstFileWriter sst_file_writer(rocksdb::EnvOptions(), {},
                                                                   nullptr, false);
                            auto s = sst_file_writer.Open(sst_files_path_ + "/edge_" +
                                                          std::to_string(num));
                            if (!s.ok()) {
                                throw std::runtime_error(FMA_FMT("failed to open sst_file_writer"));
                            }
                            std::vector<KV> vec_kvs;
                            size_t first_id_pos =
                                std::min(edgeDataBlock->src_id_pos, edgeDataBlock->dst_id_pos);
                            size_t second_id_pos =
                                std::max(edgeDataBlock->src_id_pos, edgeDataBlock->dst_id_pos);
                            std::string edge_key;
                            edge_key.reserve(20);
                            std::string k;
                            k.reserve(sizeof(VertexId));
                            VertexId src_vid, dst_vid;
                            auto hasBlob = edgeDataBlock->schema->HasBlob();
                            rocksdb::ReadOptions ro;
                            std::string vid_str;
                            for (auto& line : edgeDataBlock->block) {
                                const FieldData& src_fd = line[edgeDataBlock->src_id_pos];
                                const FieldData& dst_fd = line[edgeDataBlock->dst_id_pos];
                                k.clear();
                                k.append((const char*)&edgeDataBlock->src_label_id,
                                         sizeof(edgeDataBlock->src_label_id));
                                AppendFieldData(k, src_fd);
                                if (config_.keep_vid_in_memory) {
                                    if (!key_vid_maps_.find(k, src_vid)) {
                                        OnErrorOffline("no vid for " + src_fd.ToString());
                                        continue;
                                    }
                                } else {
                                    s = rocksdb_vids_->Get(ro, k, &vid_str);
                                    if (s.ok()) {
                                        src_vid = *(VertexId*)vid_str.data();
                                    } else if (s.IsNotFound()) {
                                        OnErrorOffline("no vid for " + src_fd.ToString());
                                        continue;
                                    } else {
                                        throw std::runtime_error(
                                            FMA_FMT("Failed to get vid, error: ", s.ToString()));
                                    }
                                }
                                k.clear();
                                k.append((const char*)&edgeDataBlock->dst_label_id,
                                         sizeof(edgeDataBlock->dst_label_id));
                                AppendFieldData(k, dst_fd);
                                if (config_.keep_vid_in_memory) {
                                    if (!key_vid_maps_.find(k, dst_vid)) {
                                        OnErrorOffline("no vid for " + dst_fd.ToString());
                                        continue;
                                    }
                                } else {
                                    s = rocksdb_vids_->Get(ro, k, &vid_str);
                                    if (s.ok()) {
                                        dst_vid = *(VertexId*)vid_str.data();
                                    } else if (s.IsNotFound()) {
                                        OnErrorOffline("no vid for " + dst_fd.ToString());
                                        continue;
                                    } else {
                                        throw std::runtime_error(
                                            FMA_FMT("Failed to get vid, error: ", s.ToString()));
                                    }
                                }
                                for (size_t i = first_id_pos; i < second_id_pos - 1; i++) {
                                    line[i] = std::move(line[i + 1]);
                                }
                                for (size_t i = second_id_pos - 1; i < line.size() - 2; i++) {
                                    line[i] = std::move(line[i + 2]);
                                }
                                line.resize(line.size() - 2);
                                FMA_DBG_CHECK_EQ(line.size(), field_ids.size());
                                Value record;
                                if (!hasBlob) {
                                    record = edgeDataBlock->schema->CreateRecord(
                                        field_ids.size(), field_ids.data(), line.data());
                                } else {
                                    record = edgeDataBlock->schema->CreateRecordWithBlobs(
                                        field_ids.size(), field_ids.data(), line.data(),
                                        [&blob_writer](const Value& blob) {
                                            return blob_writer.AddBlob(blob.MakeCopy());
                                        });
                                }
                                edge_key.clear();
                                edge_key.append((const char*)&src_vid, sizeof(src_vid));
                                edge_key.append(1, 0x00);  // out edge flag
                                edge_key.append((const char*)&edgeDataBlock->label_id,
                                                sizeof(edgeDataBlock->label_id));
                                edge_key.append((const char*)&dst_vid, sizeof(dst_vid));
                                edge_key.append((const char*)&edgeDataBlock->start_eid,
                                                sizeof(edgeDataBlock->start_eid));
                                vec_kvs.emplace_back(edge_key, record);

                                edge_key.clear();
                                edge_key.append((const char*)&dst_vid, sizeof(dst_vid));
                                edge_key.append(1, 0x01);  // in edge flag
                                edge_key.append((const char*)&edgeDataBlock->label_id,
                                                sizeof(edgeDataBlock->label_id));
                                edge_key.append((const char*)&src_vid, sizeof(src_vid));
                                edge_key.append((const char*)&edgeDataBlock->start_eid,
                                                sizeof(edgeDataBlock->start_eid));
                                vec_kvs.emplace_back(std::move(edge_key), std::move(record));
                                edgeDataBlock->start_eid++;
                            }
                            // free memory
                            std::vector<std::vector<FieldData>>().swap(edgeDataBlock->block);
                            // sort
                            LGRAPH_PSORT(vec_kvs.begin(), vec_kvs.end(),
                                         [](const KV& a, const KV& b) { return a.key < b.key; });
                            for (auto& pair : vec_kvs) {
                                sst_file_writer.Put(pair.key,
                                                    {pair.value.Data(), pair.value.Size()});
                            }
                            std::vector<KV>().swap(vec_kvs);
                            sst_file_writer.Finish();
                        } catch (...) {
                            std::lock_guard<std::mutex> guard(exceptions_lock_);
                            exceptions_.push(std::current_exception());
                        }
                    });
                }
            } catch (...) {
                std::lock_guard<std::mutex> guard(exceptions_lock_);
                exceptions_.push(std::current_exception());
            }
        });
    }
    // Blocked waiting for tasks to finish
    parse_file_threads_->join();
    generate_sst_threads_->join();
    if (!exceptions_.empty()) {
        std::rethrow_exception(exceptions_.front());
    }
    auto t2 = fma_common::GetTime();
    FMA_LOG() << "Convert edge data to sst files, time: " << t2 - t1 << "s";
}

void Importer::VertexPrimaryIndexToLmdb() {
    auto t1 = fma_common::GetTime();
    struct KeyVid {
        std::string key;
        VertexId vid;
        KeyVid(std::string k, VertexId v) : key(std::move(k)), vid(v) {}
        bool operator<(const KeyVid& rhs) const { return key < rhs.key; }
    };
    std::vector<KeyVid> key_vids;
    if (config_.keep_vid_in_memory) {
        {
            auto lt = key_vid_maps_.lock_table();
            for (auto& it : lt) {
                key_vids.emplace_back(it.first, it.second);
            }
        }
        // free memory
        cuckoohash_map<std::string, VertexId>().swap(key_vid_maps_);
        // sort
        LGRAPH_PSORT(key_vids.begin(), key_vids.end());
    }

    LabelId preLabelId = std::numeric_limits<LabelId>::max();
    auto txn = db_->CreateWriteTxn();
    VertexIndex* vertexIndex = nullptr;
    auto write_index = [&](const std::string& key, VertexId vid){
        FMA_DBG_CHECK(key.size() > sizeof(LabelId));
        LabelId labelId = *((LabelId*)key.data());
        if (labelId != preLabelId) {
            preLabelId = labelId;
            txn.Commit();
            boost::endian::big_to_native_inplace(labelId);
            txn = db_->CreateReadTxn();
            auto schema = txn.GetSchemaInfo().v_schema_manager.GetSchema(labelId);
            auto& label = schema->GetLabel();
            auto& primary_field = schema->GetPrimaryField();
            txn.Abort();
            db_->GetLightningGraph()->_AddEmptyIndex(label, primary_field, true, true);
            txn = db_->CreateWriteTxn();
            vertexIndex = txn.GetVertexIndex(label, primary_field);
        }
        boost::endian::big_to_native_inplace(vid);
        const char* p = key.data() + sizeof(LabelId);
        size_t size = key.size() - sizeof(LabelId);
        switch (vertexIndex->KeyType()) {
        case FieldType::BOOL:
            {
                auto k = decodeStrToNum<int8_t>(p);
                vertexIndex->_AppendVertexIndexEntry(txn.GetTxn(), Value(&k, sizeof(k)), vid);
            }
            break;
        case FieldType::FLOAT:
            {
                auto k = decodeStrToNum<float>(p);
                vertexIndex->_AppendVertexIndexEntry(txn.GetTxn(), Value(&k, sizeof(k)), vid);
            }
            break;
        case FieldType::DOUBLE:
            {
                auto k = decodeStrToNum<double>(p);
                vertexIndex->_AppendVertexIndexEntry(txn.GetTxn(), Value(&k, sizeof(k)), vid);
            }
            break;
        case FieldType::DATE:
            {
                auto k = decodeStrToNum<int32_t>(p);
                vertexIndex->_AppendVertexIndexEntry(txn.GetTxn(), Value(&k, sizeof(k)), vid);
            }
            break;
        case FieldType::DATETIME:
            {
                auto k = decodeStrToNum<int64_t>(p);
                vertexIndex->_AppendVertexIndexEntry(txn.GetTxn(), Value(&k, sizeof(k)), vid);
            }
            break;
        case FieldType::INT64:
            {
                auto k = decodeStrToNum<int64_t>(p);
                vertexIndex->_AppendVertexIndexEntry(txn.GetTxn(), Value(&k, sizeof(k)), vid);
            }
            break;
        case FieldType::INT32:
            {
                auto k = decodeStrToNum<int32_t>(p);
                vertexIndex->_AppendVertexIndexEntry(txn.GetTxn(), Value(&k, sizeof(k)), vid);
            }
            break;
        case FieldType::INT16:
            {
                auto k = decodeStrToNum<int16_t>(p);
                vertexIndex->_AppendVertexIndexEntry(txn.GetTxn(), Value(&k, sizeof(k)), vid);
            }
            break;
        case FieldType::INT8:
            {
                auto k = decodeStrToNum<int8_t>(p);
                vertexIndex->_AppendVertexIndexEntry(txn.GetTxn(), Value(&k, sizeof(k)), vid);
            }
            break;
        case FieldType::STRING:
            {
                vertexIndex->_AppendVertexIndexEntry(txn.GetTxn(), Value(p, size), vid);
            }
            break;
        default:
            FMA_ASSERT(false);
        }
    };
    if (config_.keep_vid_in_memory) {
        for (auto& item : key_vids) {
            write_index(item.key, item.vid);
        }
    } else {
        rocksdb::ReadOptions options;
        options.ignore_range_deletions = true;
        options.background_purge_on_iterator_cleanup = true;
        options.verify_checksums = false;
        std::unique_ptr<rocksdb::Iterator> iter(rocksdb_vids_->NewIterator(options));
        for (iter->SeekToFirst(); iter->Valid(); iter->Next()) {
            auto k = iter->key();
            auto v = iter->value();
            write_index(k.ToString(), *(VertexId*)v.data());
        }
    }
    txn.Commit();
    auto t2 = fma_common::GetTime();
    FMA_LOG() << "Write vertex primary index to lmdb, time: " << t2 - t1 << "s";
    rocksdb_vids_.reset(nullptr);
}
typedef std::vector<std::tuple<LabelId, TemporalId,
                               VertexId, import_v2::DenseString>>::iterator IT;
void Importer::RocksdbToLmdb() {
    auto t1 = fma_common::GetTime();
    std::unique_ptr<rocksdb::DB> rocksdb;
    {
        rocksdb::Options options;
        options.create_if_missing = true;
        options.create_missing_column_families = true;
        rocksdb::BlockBasedTableOptions table_options;
        table_options.no_block_cache = true;
        options.table_factory.reset(rocksdb::NewBlockBasedTableFactory(table_options));
        options.IncreaseParallelism();
        options.PrepareForBulkLoad();
        rocksdb::DB* db;
        auto s = rocksdb::DB::Open(options, rocksdb_path_, &db);
        if (!s.ok()) {
            throw std::runtime_error(FMA_FMT("Opening DB failed, error: {}", s.ToString().c_str()));
        }
        rocksdb.reset(db);
    }
    std::vector<std::string> ingest_files;
    for (const auto & entry : std::filesystem::directory_iterator(sst_files_path_)) {
        ingest_files.push_back(entry.path());
    }
    rocksdb::IngestExternalFileOptions op;
    op.move_files = true;
    auto s = rocksdb->IngestExternalFile(ingest_files, op);
    if (!s.ok()) {
        throw std::runtime_error(
            FMA_FMT("Importing files failed, error: {}", s.ToString().c_str()));
    }
    if (config_.compact) {
        auto begin = fma_common::GetTime();
        rocksdb::CompactRangeOptions options;
        options.max_subcompactions = 8;
        s = rocksdb->CompactRange(options, nullptr, nullptr);
        if (!s.ok()) {
            throw std::runtime_error(
                FMA_FMT("CompactRange failed, error: {}", s.ToString().c_str()));
        }
        FMA_LOG() << "CompactRange, time: " << fma_common::GetTime() - begin << "s";
    }
    std::atomic<uint64_t> pending_tasks(0);
    std::atomic<uint16_t> stage(0);
    std::unique_ptr<boost::asio::thread_pool> lmdb_writer(
        new boost::asio::thread_pool(1));
    std::unique_ptr<boost::asio::thread_pool> rocksdb_readers(
        new boost::asio::thread_pool(config_.read_rocksdb_threads));
    for (uint16_t i = 0; i < config_.read_rocksdb_threads; i++) {
        boost::asio::post(*rocksdb_readers, [this, i, &pending_tasks,
                                             &rocksdb, &lmdb_writer, &stage]() {
            uint64_t start_vid = i * config_.vid_num_per_reading;
            uint64_t bigend_start_vid = boost::endian::native_to_big(start_vid);
            uint64_t end_vid = (i+1) * config_.vid_num_per_reading;
            uint64_t bigend_end_vid = boost::endian::native_to_big(end_vid);
            rocksdb::ReadOptions options;
            options.ignore_range_deletions = true;
            options.background_purge_on_iterator_cleanup = true;
            options.verify_checksums = false;
            std::unique_ptr<rocksdb::Iterator> iter(rocksdb->NewIterator(options));
            std::vector<std::tuple<LabelId, TemporalId, VertexId, import_v2::DenseString>> outs;
            std::vector<std::tuple<LabelId, TemporalId, VertexId, import_v2::DenseString>> ins;
            std::vector<std::pair<Value, Value>> kvs;
            size_t all_kv_size = 0;
            import_v2::DenseString vdata;
            bool split = false;
            LabelId out_last_lid = 0, in_last_lid = 0;
            TemporalId out_last_tid = -1, in_last_tid = -1;
            VertexId out_last_dst = -1, in_last_dst = -1;
            EdgeId out_last_eid = -1, in_last_eid = -1;
            size_t total_size = 0;
            VertexId pre_vid = InvalidVid;

            auto throw_kvs_to_lmdb = [&lmdb_writer, &pending_tasks, this, &stage, i]
                (std::vector<std::pair<Value, Value>> kvs){
                while (stage != i || pending_tasks > 1) {
                    fma_common::SleepUs(1000);
                }
                pending_tasks++;
                boost::asio::post(*lmdb_writer, [this, &pending_tasks, kvs = std::move(kvs)]() {
                    Transaction txn = db_->CreateWriteTxn();
                    for (auto& kv : kvs) {
                        txn.ImportAppendDataRaw(kv.first, kv.second);
                    }
                    txn.RefreshNextVid();
                    txn.Commit();
                    pending_tasks--;
                });
            };
            auto make_kvs = [&]() {
                if (!split) {
                    lgraph::graph::VertexValue vov(GetConstRef(vdata));
                    IT next_beg;
                    lgraph::graph::EdgeValue oev(outs.begin(), outs.end(), out_last_lid,
                                                 out_last_tid, out_last_dst, out_last_eid,
                                                 next_beg, true);
                    FMA_DBG_ASSERT(next_beg == outs.end());
                    outs.clear();
                    lgraph::graph::EdgeValue iev(ins.begin(), ins.end(), in_last_lid,
                                                 in_last_tid, in_last_dst, in_last_eid,
                                                 next_beg, true);
                    FMA_DBG_ASSERT(next_beg == ins.end());
                    ins.clear();
                    Value data;
                    lgraph::graph::PackedDataValue::PackData(vov, oev, iev, data);
                    kvs.emplace_back(lgraph::graph::KeyPacker::CreatePackedDataKey(pre_vid),
                                     std::move(data));
                    all_kv_size += kvs.back().first.Size() + kvs.back().second.Size();
                } else {
                    if (!outs.empty()) {
                        IT next_start;
                        lgraph::graph::EdgeValue oev(outs.begin(), outs.end(), out_last_lid,
                                                     out_last_tid, out_last_dst, out_last_eid,
                                                     next_start, true);
                        FMA_DBG_ASSERT(next_start == outs.end());
                        kvs.emplace_back(oev.CreateOutEdgeKey(pre_vid),
                                         std::move(oev.GetBuf()));
                        all_kv_size += kvs.back().first.Size() + kvs.back().second.Size();
                        outs.clear();
                    }
                    if (!ins.empty()) {
                        IT next_start;
                        lgraph::graph::EdgeValue iev(ins.begin(), ins.end(), in_last_lid,
                                                     in_last_tid, in_last_dst, in_last_eid,
                                                     next_start, true);
                        FMA_DBG_ASSERT(next_start == ins.end());
                        kvs.emplace_back(iev.CreateInEdgeKey(pre_vid),
                                         std::move(iev.GetBuf()));
                        all_kv_size += kvs.back().first.Size() + kvs.back().second.Size();
                        ins.clear();
                    }
                }
                split = false;
                total_size = 0;
                out_last_lid = 0;
                in_last_lid = 0;
                out_last_tid = -1;
                in_last_tid = -1;
                out_last_dst = -1;
                in_last_dst = -1;
                out_last_eid = -1;
                in_last_eid = -1;
            };

            while (true) {
                pre_vid = InvalidVid;
                for (iter->Seek({(const char*)&bigend_start_vid, sizeof(bigend_start_vid)});
                     iter->Valid();
                     iter->Next()) {
                    auto key = iter->key();
                    if (key.compare({(const char*)&bigend_end_vid, sizeof(bigend_end_vid)}) >= 0) {
                        make_kvs();
                        throw_kvs_to_lmdb(std::move(kvs));
                        all_kv_size = 0;
                        stage = (i + 1) % config_.read_rocksdb_threads;
                        break;
                    }

                    auto val = iter->value();
                    const char* p = key.data();
                    if (key.size() == sizeof(VertexId)) {
                        VertexId vid = (*(VertexId*)p);
                        boost::endian::big_to_native_inplace(vid);
                        if (pre_vid != vid) {
                            if (pre_vid != InvalidVid) {
                                make_kvs();
                                if (all_kv_size > config_.max_size_per_reading) {
                                    throw_kvs_to_lmdb(std::move(kvs));
                                    all_kv_size = 0;
                                }
                            }
                            pre_vid = vid;
                        }
                        vdata = import_v2::DenseString(val.data(), val.size());
                        total_size += vdata.size();
                    } else {
                        bool out_edge;
                        p += sizeof(VertexId);
                        if (*p == 0) {
                            out_edge = true;
                        } else {
                            out_edge = false;
                        }
                        p += 1;
                        LabelId labelId = *(LabelId*)p;
                        boost::endian::big_to_native_inplace(labelId);
                        p += sizeof(LabelId);
                        VertexId vertexId = *(VertexId*)p;
                        boost::endian::big_to_native_inplace(vertexId);
                        import_v2::DenseString edge_val(val.data(), val.size());
                        total_size += edge_val.size() + 16;
                        if (out_edge) {
                            outs.emplace_back(labelId, 0, vertexId, std::move(edge_val));
                        } else {
                            ins.emplace_back(labelId, 0, vertexId, std::move(edge_val));
                        }
                        if (total_size > ::lgraph::_detail::NODE_SPLIT_THRESHOLD) {
                            if (!split) {
                                kvs.emplace_back(
                                    lgraph::graph::KeyPacker::CreateVertexOnlyKey(pre_vid),
                                    Value::MakeCopy(GetConstRef(vdata)));
                                all_kv_size += kvs.back().first.Size() + kvs.back().second.Size();
                                split = true;
                            }
                            if (!outs.empty()) {
                                IT next_start;
                                lgraph::graph::EdgeValue oev(outs.begin(), outs.end(),
                                                             out_last_lid, out_last_tid,
                                                             out_last_dst, out_last_eid,
                                                             next_start, true);
                                FMA_DBG_ASSERT(next_start == outs.end());
                                kvs.emplace_back(oev.CreateOutEdgeKey(pre_vid),
                                                 std::move(oev.GetBuf()));
                                outs.clear();
                                all_kv_size += kvs.back().first.Size() + kvs.back().second.Size();
                            }
                            if (!ins.empty()) {
                                IT next_start;
                                lgraph::graph::EdgeValue iev(ins.begin(), ins.end(), in_last_lid,
                                                             in_last_tid, in_last_dst, in_last_eid,
                                                             next_start, true);
                                FMA_DBG_ASSERT(next_start == ins.end());
                                kvs.emplace_back(iev.CreateInEdgeKey(pre_vid),
                                                 std::move(iev.GetBuf()));
                                ins.clear();
                                all_kv_size += kvs.back().first.Size() + kvs.back().second.Size();
                            }

                            if (all_kv_size > config_.max_size_per_reading) {
                                throw_kvs_to_lmdb(std::move(kvs));
                                all_kv_size = 0;
                            }
                            total_size = 0;
                        }
                    }
                }
                if (iter->Valid()) {
                    start_vid += config_.read_rocksdb_threads * config_.vid_num_per_reading;
                    end_vid += config_.read_rocksdb_threads * config_.vid_num_per_reading;
                    bigend_start_vid = boost::endian::native_to_big(start_vid);
                    bigend_end_vid = boost::endian::native_to_big(end_vid);
                } else {
                    if (pre_vid != InvalidVid) {
                        make_kvs();
                        throw_kvs_to_lmdb(std::move(kvs));
                        all_kv_size = 0;
                    }
                    break;
                }
            }
        });
    }
    rocksdb_readers->join();
    lmdb_writer->join();
    auto t2 = fma_common::GetTime();
    FMA_LOG() << "Dump rocksdb into lmdb, time: " << t2 - t1 << "s";
}

AccessControlledDB Importer::OpenGraph(Galaxy& galaxy, bool empty_db) {
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

}  // namespace import_v3
}  // namespace lgraph
