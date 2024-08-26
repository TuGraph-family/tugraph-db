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

#include "plugin/plugin_manager.h"

lgraph::SingleLanguagePluginManager::SingleLanguagePluginManager(
    const std::string& language, const std::string& graph_name, const std::string& plugin_dir,
    const std::string& table_name, KvTransaction& txn, KvStore& store,
    std::unique_ptr<PluginManagerImplBase>&& impl) {
    impl_ = std::move(impl);
    language_ = language;
    graph_name_ = graph_name;
    plugin_dir_ = plugin_dir;
    auto& fs = fma_common::FileSystem::GetFileSystem(plugin_dir);
    if (!fs.IsDir(plugin_dir) && !fs.Mkdir(plugin_dir)) {
        THROW_CODE(InternalError, "Failed to create plugin dir [{}].", plugin_dir);
    }
    table_ = store.OpenTable(txn, table_name, true, ComparatorDesc::DefaultComparator());
    LoadAllPlugins(txn);
}

lgraph::SingleLanguagePluginManager::SingleLanguagePluginManager(
    const std::string& language, LightningGraph* db, const std::string& graph_name,
    const std::string& plugin_dir, const std::string& table_name,
    std::unique_ptr<PluginManagerImplBase>&& impl)
    : db_(db), graph_name_(graph_name) {
    impl_ = std::move(impl);
    language_ = language;
    plugin_dir_ = plugin_dir;
    auto& fs = fma_common::FileSystem::GetFileSystem(plugin_dir);
    if (!fs.IsDir(plugin_dir) && !fs.Mkdir(plugin_dir)) {
        THROW_CODE(InternalError, "Failed to create plugin dir [{}].", plugin_dir);
    }
    auto txn = db_->CreateWriteTxn();
    table_ = db_->GetStore().OpenTable(txn.GetTxn(), table_name, true,
                                       ComparatorDesc::DefaultComparator());
    LoadAllPlugins(txn.GetTxn());
    txn.Commit();
}

lgraph::SingleLanguagePluginManager::~SingleLanguagePluginManager() { UnloadAllPlugins(); }

void lgraph::SingleLanguagePluginManager::DeleteAllPlugins(const std::string& user,
                                                           KvTransaction& txn) {
    auto& fs = fma_common::FileSystem::GetFileSystem(plugin_dir_);
    for (auto& kv : procedures_) {
        std::string error;
        impl_->UnloadPlugin(user, kv.first, kv.second);
        fs.Remove(impl_->GetPluginPath(kv.first));
        delete kv.second;
    }
    procedures_.clear();
    table_->Drop(txn);
}

void lgraph::SingleLanguagePluginManager::DeleteAllPlugins(const std::string& user) {
    auto txn = db_->CreateWriteTxn();
    DeleteAllPlugins(user, txn.GetTxn());
    txn.Commit();
}

std::string lgraph::SingleLanguagePluginManager::SignatureToJsonString(
    const lgraph_api::SigSpec& spec) {
    nlohmann::json body, input, output;
    for (auto& param : spec.input_list) {
        input.push_back({{"name", param.name}, {"type", to_string(param.type)}});
    }
    for (auto& param : spec.result_list) {
        output.push_back({{"name", param.name}, {"type", to_string(param.type)}});
    }
    body["input"] = input;
    body["output"] = output;

    return body.dump();
}

std::vector<lgraph::PluginDesc> lgraph::SingleLanguagePluginManager::ListPlugins(
    const std::string& user) {
    AutoReadLock lock(lock_, GetMyThreadId());
    auto txn = db_->CreateReadTxn();
    std::vector<PluginDesc> plugins;
    for (auto it = procedures_.begin(); it != procedures_.end(); it++) {
        std::string signature;
        auto& spec = it->second->sig_spec;
        if (spec) {
            signature = std::move(SignatureToJsonString(*spec));
        } else {
            signature = "";
        }
        std::string code_type;
        {
            std::string name = it->first;
            std::string zip_key = GetZipKey(name);
            std::string cpp_key = GetCppKey(name);
            std::string so_key = GetSoKey(name);
            std::string cython_key = GetCythonKey(name);
            if (table_->HasKey(txn.GetTxn(), Value::ConstRef(zip_key))) {
                auto zip_it = table_->GetIterator(txn.GetTxn(), Value::ConstRef(zip_key));
                const std::string& zip = zip_it->GetValue().AsString();
                code_type = "zip";
            } else if (table_->HasKey(txn.GetTxn(), Value::ConstRef(cpp_key))) {
                auto cpp_it = table_->GetIterator(txn.GetTxn(), Value::ConstRef(cpp_key));
                const std::string& cpp = cpp_it->GetValue().AsString();
                code_type = "cpp";
            } else if (table_->HasKey(txn.GetTxn(), Value::ConstRef(cython_key))) {
                auto cython_it = table_->GetIterator(txn.GetTxn(), Value::ConstRef(cython_key));
                const std::string& cython = cython_it->GetValue().AsString();
                code_type = "py";
            } else {
                auto so_it = table_->GetIterator(txn.GetTxn(), Value::ConstRef(so_key));
                const std::string& so = so_it->GetValue().AsString();
                code_type = language_ == plugin::PLUGIN_LANG_TYPE_CPP ? "so" : "py";
            }
        }
        plugins.emplace_back(FromInternalName(it->first), code_type, it->second->desc,
                             it->second->version, it->second->read_only, signature);
    }
    txn.Commit();
    return plugins;
}

bool lgraph::SingleLanguagePluginManager::GetPluginSigSpec(const std::string& user,
                                                           const std::string& name_,
                                                           lgraph_api::SigSpec** sig_spec) {
    std::string name = ToInternalName(name_);
    AutoReadLock lock(lock_, GetMyThreadId());
    auto it = procedures_.find(name);
    if (it == procedures_.end()) return false;
    *sig_spec = it->second->sig_spec.get();
    return true;
}

bool lgraph::SingleLanguagePluginManager::GetPluginCode(const std::string& user,
                                                        const std::string& name_,
                                                        lgraph::PluginCode& ret) {
    std::string name = ToInternalName(name_);
    AutoReadLock lock(lock_, GetMyThreadId());
    auto it = procedures_.find(name);
    if (it == procedures_.end()) return false;
    ret.name = name_;
    ret.desc = it->second->desc;
    ret.read_only = it->second->read_only;
    auto txn = db_->CreateReadTxn();
    {
        std::string zip_key = GetZipKey(name);
        std::string cpp_key = GetCppKey(name);
        std::string so_key = GetSoKey(name);
        std::string cython_key = GetCythonKey(name);
        if (table_->HasKey(txn.GetTxn(), Value::ConstRef(zip_key))) {
            auto zip_it = table_->GetIterator(txn.GetTxn(), Value::ConstRef(zip_key));
            const std::string& zip = zip_it->GetValue().AsString();
            ret.code = zip;
            ret.code_type = "zip";
        } else if (table_->HasKey(txn.GetTxn(), Value::ConstRef(cpp_key))) {
            auto cpp_it = table_->GetIterator(txn.GetTxn(), Value::ConstRef(cpp_key));
            const std::string& cpp = cpp_it->GetValue().AsString();
            ret.code = cpp;
            ret.code_type = "cpp";
        } else if (table_->HasKey(txn.GetTxn(), Value::ConstRef(cython_key))) {
            auto cython_it = table_->GetIterator(txn.GetTxn(), Value::ConstRef(cython_key));
            const std::string& cython = cython_it->GetValue().AsString();
            ret.code = cython;
            ret.code_type = "py";
        } else {
            auto so_it = table_->GetIterator(txn.GetTxn(), Value::ConstRef(so_key));
            const std::string& so = so_it->GetValue().AsString();
            ret.code = so;
            ret.code_type = "so_or_py";
        }
    }
    txn.Commit();
    return true;
}

static std::string GetSuffix(const std::string& s) {
    size_t pos = s.rfind(".");
    if (pos == s.npos) return "";
    std::string suffix = s.substr(pos + 1, s.length() - pos - 1);
    return suffix;
}

void lgraph::SingleLanguagePluginManager::UpdateSoToKvStore(KvTransaction& txn,
                                                            const std::string& name,
                                                            const std::string& so) {
    // write runnable code
    std::string so_key = GetSoKey(name);
    table_->SetValue(txn, Value::ConstRef(so_key), Value::ConstRef(so));
    // write hash
    std::string hash_key = GetHashKey(name);
    table_->SetValue(txn, Value::ConstRef(hash_key), Value::ConstRef(GIT_COMMIT_HASH));
}

void lgraph::SingleLanguagePluginManager::UpdateZipToKvStore(KvTransaction& txn,
                                                             const std::string& name,
                                                             const std::string& zip) {
    // write .zip
    std::string zip_key = GetZipKey(name);
    table_->SetValue(txn, Value::ConstRef(zip_key), Value::ConstRef(zip));
}

void lgraph::SingleLanguagePluginManager::UpdateCppToKvStore(KvTransaction& txn,
                                                             const std::string& name,
                                                             const std::string& cpp) {
    // write cpp file
    std::string cpp_key = GetCppKey(name);
    table_->SetValue(txn, Value::ConstRef(cpp_key), Value::ConstRef(cpp));
}

void lgraph::SingleLanguagePluginManager::UpdateCythonToKvStore(KvTransaction& txn,
                                                                const std::string& name,
                                                                const std::string& cython) {
    // write cython file
    std::string cython_key = GetCythonKey(name);
    table_->SetValue(txn, Value::ConstRef(cython_key), Value::ConstRef(cython));
}

void lgraph::SingleLanguagePluginManager::UpdateInfoToKvStore(KvTransaction& txn,
                                                              const std::string& name,
                                                              fma_common::BinaryBuffer& info) {
    table_->SetValue(txn, Value::ConstRef(name), Value(info.GetBuf(), info.GetSize()));
}

static void ExecuteCommand(const std::string& cmd, size_t timeout_ms,
                           const std::string& msg_timeout, const std::string& msg_fail) {
    lgraph::SubProcess proc(cmd, false);
    if (!proc.Wait(lgraph::_detail::MAX_UNZIP_TIME_MS))
        THROW_CODE(InputError, "{} \nStdout:----\n{}\nStderr:----\n{}", msg_timeout,
                                         proc.Stdout(), proc.Stderr());
    if (proc.GetExitCode())
        THROW_CODE(InputError, "{} \nStdout:----\n{}\nStderr:----\n{}", msg_fail,
                                         proc.Stdout(), proc.Stderr());
}

static inline std::string GenUniqueTempDir(const std::string& base, const std::string& name) {
    return FMA_FMT("{}{}{}{}_{}.tmp", base,
                   fma_common::FileSystem::GetFileSystem(base).PathSeparater(), name, rand(),
                   std::chrono::high_resolution_clock::now().time_since_epoch().count());
}

static inline void WriteWholeFile(const std::string& path, const std::string& code,
                                  const std::string& file_desc) {
    fma_common::OutputFmaStream ofs(path);
    if (!ofs.Good()) THROW_CODE(InternalError, "Failed to write {} [{}].", file_desc, path);
    ofs.Write(code.data(), code.size());
}

static inline std::string ReadWholeFile(const std::string& path, const std::string& file_desc) {
    fma_common::InputFmaStream ifs(path, 0);
    if (!ifs.Good()) THROW_CODE(InternalError, "Failed to open {} [{}].", file_desc, path);
    size_t sz = ifs.Size();
    std::string code;
    code.resize(sz);
    size_t ssz = ifs.Read(&code[0], sz);
    if (ssz != sz) THROW_CODE(InternalError, "Failed to read {} [{}].", file_desc, path);
    return code;
}

std::string lgraph::SingleLanguagePluginManager::CompilePluginFromCython(
    const std::string& name, const std::string& cython) {
#ifdef _WIN32
    THROW_CODE(InputError, "Compiling cython is not supported on Windows.");
#endif
    std::string base_dir = impl_->GetPluginDir();
    auto& fs = fma_common::FileSystem::GetFileSystem(base_dir);
    std::string tmp_dir = GenUniqueTempDir(base_dir, name);
    std::string cython_file_path = tmp_dir + fs.PathSeparater() + name + ".py";
    std::string cpp_file_path = tmp_dir + fs.PathSeparater() + name + ".cpp";
    std::string plugin_path = tmp_dir + fs.PathSeparater() + name + ".so";

    AutoCleanDir tmp_dir_cleaner(tmp_dir);

    WriteWholeFile(cython_file_path, cython, "plugin source file");

    // cython
    std::string exec_dir = fma_common::FileSystem::GetExecutablePath().Dir();
    std::string cmd = FMA_FMT(
        "cython {} -+ -3 -I{}/../../include/cython/ "
        " -I/usr/local/include/cython/ "
        " -o {} --module-name {} ",
        cython_file_path, exec_dir, cpp_file_path, name);
    ExecuteCommand(cmd, _detail::MAX_COMPILE_TIME_MS, "Timeout while translate cython to c++.",
                   "Failed to translated cython. cmd: " + cmd);

    // compile
    std::string CFLAGS = FMA_FMT(
        " -I/usr/local/include "
        " -I{}/../../include "
        " -I/usr/include/python3.6m "
        " -I/usr/local/include/python3.6m ",
        exec_dir);
    //    std::string LDFLAGS = FMA_FMT("-llgraph -L{}/ -L/usr/local/lib64/ "
    //                                  "-L/usr/lib64/ -lpython3.6m", exec_dir);
    std::string LDFLAGS = FMA_FMT(
        " -llgraph -L{} -L/usr/local/lib64/ "
        " -L/usr/lib64/ ",
        exec_dir);
#ifndef __clang__
    cmd = FMA_FMT(
        "g++ -fno-gnu-unique -fPIC -g --std=c++17 {} -rdynamic -O3 -fopenmp -o {} {} {} -shared",
        CFLAGS, plugin_path, cpp_file_path, LDFLAGS);
#elif __APPLE__
    THROW_CODE(InputError, "Compiling cython is not supported on APPLE.");
#else
    THROW_CODE(InputError, "Compiling cython is not supported on clang.");
#endif
    ExecuteCommand(cmd, _detail::MAX_COMPILE_TIME_MS, "Timeout while compiling plugin.",
                   "Failed to compile plugin.");

    // return
    return ReadWholeFile(plugin_path, "plugin binary file");
}

std::string lgraph::SingleLanguagePluginManager::CompilePluginFromZip(const std::string& name,
                                                                      const std::string& zip) {
#ifdef _WIN32
    THROW_CODE(InputError, "Compiling ZIP is not supported on Windows.");
#endif
    std::string base_dir = impl_->GetPluginDir();
    auto& fs = fma_common::FileSystem::GetFileSystem(base_dir);
    std::string tmp_dir = GenUniqueTempDir(base_dir, name);
    std::string zip_path = tmp_dir + fs.PathSeparater() + name + ".zip";
    std::string app_dir = tmp_dir + fs.PathSeparater() + name;

    AutoCleanDir tmp_dir_cleaner(tmp_dir);

    // write zip and unzip
    WriteWholeFile(zip_path, zip, "plugin source package");
    std::string cmd = FMA_FMT("unzip -o \"{}\" -d \"{}\"", zip_path, app_dir);
    ExecuteCommand(cmd, _detail::MAX_UNZIP_TIME_MS, "Timeout while unzipping plugin file.",
                   "Failed to unzip plugin file.");

    // compile
    std::string exec_dir = fma_common::FileSystem::GetExecutablePath().Dir();
    std::string CFLAGS = FMA_FMT(" -I{}/../../include -I/usr/local/include", exec_dir);
    std::string LDFLAGS =
        FMA_FMT("-L{}/../../liblgraph.so -L/usr/local/lib64/liblgraph.so", exec_dir);
    cmd = fma_common::StringFormatter::Format("cd \"{}\" && make CFLAGS=\"{}\" LDFLAGS=\"{}\"",
                                              app_dir, CFLAGS, LDFLAGS);
    ExecuteCommand(cmd, _detail::MAX_COMPILE_TIME_MS, "Timeout while compiling plugin.",
                   "Failed to compile plugin.");

    // return
    std::string plugin_file = "";
    auto files = fs.ListFiles(app_dir, nullptr);
    for (auto it = files.begin(); it != files.end(); ++it) {
        if (GetSuffix(*it) == "so") {
            plugin_file = *it;
            break;
        }
    }
    if (plugin_file == "") THROW_CODE(InputError,
                                      "Failed to find any .so file in compiled directory.");
    return ReadWholeFile(plugin_file, "plugin binary file");
}

std::string lgraph::SingleLanguagePluginManager::CompilePluginFromCpp(
    const std::string& name, const std::string& all_codes) {
#ifdef _WIN32

#endif
    std::string base_dir = impl_->GetPluginDir();
    auto& fs = fma_common::FileSystem::GetFileSystem(base_dir);
    std::string tmp_dir = GenUniqueTempDir(base_dir, name);
    std::string file_path = tmp_dir + fs.PathSeparater() + "/";
    std::string plugin_path = tmp_dir + fs.PathSeparater() + name + ".so";

    AutoCleanDir tmp_dir_cleaner(tmp_dir);

    // compile
    std::vector<std::string> filename;
    std::vector<std::string> code;
    SplitCode(code, filename, all_codes);
    std::string source_files = "";
        for (size_t i = 0; i < filename.size(); i++) {
        WriteWholeFile(file_path + filename[i], code[i], "plugin source file-" + std::to_string(i));
        source_files += FMA_FMT(" {}/{}", file_path, filename[i]);
    }
    std::string exec_dir = fma_common::FileSystem::GetExecutablePath().Dir();
    std::string CFLAGS = FMA_FMT("-I{}/../../include -I/usr/local/include", exec_dir);
    std::string LDFLAGS = FMA_FMT("-llgraph -L{}/ -L/usr/local/lib64/", exec_dir);
#ifndef __clang__
#ifdef __SANITIZE_ADDRESS__
    std::string cmd = FMA_FMT(
        "g++ -fno-gnu-unique "
        " -fPIC -g --std=c++17 {} -Wl,-z,nodelete "
        " -rdynamic -O3 -fopenmp -o {} {} {} -shared ",
        CFLAGS, plugin_path, source_files, LDFLAGS);
#else
    std::string cmd = FMA_FMT(
        "g++ -fno-gnu-unique -fPIC -g "
        " --std=c++17 {} -rdynamic -O3 -fopenmp -o {} {} {} -shared",
        CFLAGS, plugin_path, source_files, LDFLAGS);
#endif
#elif __APPLE__
    std::string cmd = FMA_FMT(
        "clang++ -stdlib=libc++ "
        " -fPIC -g --std=c++17 {} -rdynamic -O3 -Xpreprocessor -fopenmp -o "
        "{} {} {} -shared",
        CFLAGS, plugin_path, source_files, LDFLAGS);
#else
    std::string cmd = FMA_FMT(
        "clang++ -stdlib=libc++ "
        " -fPIC -g --std=c++17 {} -rdynamic -O3 -fopenmp -o {} {} {} -shared",
        CFLAGS, plugin_path, source_files, LDFLAGS);
#endif
    ExecuteCommand(cmd, _detail::MAX_COMPILE_TIME_MS, "Timeout while compiling plugin.",
                   "Failed to compile plugin.");

    // return
    return ReadWholeFile(plugin_path, "plugin binary file");
}

void lgraph::SingleLanguagePluginManager::LoadPlugin(const std::string& user, KvTransaction& txn,
                                                     const std::string& name,
                                                     const std::string& exe,
                                                     const std::string& desc, bool read_only,
                                                     const std::string& version) {
    std::string plugin_path = impl_->GetPluginPath(name);
    // write so
    WriteWholeFile(plugin_path, exe, "plugin binary file");

    // load dll
    std::unique_ptr<PluginInfoBase> pinfo_(impl_->CreatePluginInfo());
    PluginInfoBase* pinfo = pinfo_.get();
    pinfo->language = language_;
    pinfo->desc = desc;
    pinfo->read_only = read_only;
    pinfo->version = version;
    impl_->LoadPlugin(user, name, pinfo);
    procedures_.emplace(name, pinfo_.release());

    // update kv and memory status
    fma_common::BinaryBuffer info;
    fma_common::BinaryWrite(info, *pinfo);
    UpdateInfoToKvStore(txn, name, info);
    UpdateSoToKvStore(txn, name, exe);
}

bool lgraph::SingleLanguagePluginManager::LoadPluginFromCode(
    const std::string& user, const std::string& name_,
    const std::vector<std::string>& code,
    const std::vector<std::string>& filename,
    plugin::CodeType code_type, const std::string& desc, bool read_only,
    const std::string& version) {
    // check input
    bool empty_code = code.empty();
    for (auto& c : code) {
        if (c.empty()) {
            empty_code = true;
            break;
        }
    }
    if (empty_code) THROW_CODE(InputError, "Code cannot be empty.");
    if (!IsValidPluginName(name_)) throw InvalidPluginNameException(name_);
    if (version != plugin::PLUGIN_VERSION_1 && version != plugin::PLUGIN_VERSION_2) {
        throw InvalidPluginVersionException(version);
    }

    std::string name = ToInternalName(name_);
    // check if plugin already exist, remove if overwrite
    int tid = GetMyThreadId();
    AutoReadLock rlock(lock_, tid);
    auto it = procedures_.find(name);
    if (it != procedures_.end()) return false;

    // undo the changes if there is any problem
    AutoCleanupAction rollback([&]() {
        auto it = procedures_.find(name);
        if (it != procedures_.end()) {
            // unload plugin
            try {
                impl_->UnloadPlugin(user, name, it->second);
            } catch (...) {
            }
            // delete pinfo
            delete it->second;
            procedures_.erase(it);
        }
        // delete file
        fma_common::file_system::RemoveFile(impl_->GetPluginPath(name));
        // exception can only occure before txn commit, so no need to revert kv state
    });

    // load plugin from different type
    std::string exe;
    std::string all_codes;
    if (code_type == plugin::CodeType::CPP) {
        all_codes = MergeCodeFiles(code, filename, name);
    }
    switch (code_type) {
    case plugin::CodeType::SO:
        break;
    case plugin::CodeType::PY:
        if (code.size() != 1) {
            THROW_CODE(InternalError,
                       FMA_FMT("code_type [{}] only supports uploading a single file.", code_type));
        }
        exe = CompilePluginFromCython(name, code[0]);
        break;
    case plugin::CodeType::CPP:
        exe = CompilePluginFromCpp(name, all_codes);
        break;
    case plugin::CodeType::ZIP:
        if (code.size() != 1) {
            THROW_CODE(InternalError,
                       FMA_FMT("code_type [{}] only supports uploading a single file.", code_type));
        }
        exe = CompilePluginFromZip(name, code[0]);
        break;
    default:
        THROW_CODE(InternalError, "Unhandled code_type [{}].", code_type);
        return false;
    }

    // upgrade to write lock
    AutoWriteLock wlock(lock_, tid);
    auto txn = db_->CreateWriteTxn();
    switch (code_type) {
    case plugin::CodeType::PY:
        LoadPlugin(user, txn.GetTxn(), name, exe, desc, read_only, version);
        UpdateCythonToKvStore(txn.GetTxn(), name, code[0]);
        break;
    case plugin::CodeType::SO:
        LoadPlugin(user, txn.GetTxn(), name, code[0], desc, read_only, version);
        break;
    case plugin::CodeType::CPP:
        LoadPlugin(user, txn.GetTxn(), name, exe, desc, read_only, version);

        UpdateCppToKvStore(txn.GetTxn(), name, all_codes);
        break;
    case plugin::CodeType::ZIP:
        LoadPlugin(user, txn.GetTxn(), name, exe, desc, read_only, version);
        UpdateZipToKvStore(txn.GetTxn(), name, code[0]);
        break;
    default:
        THROW_CODE(InternalError, "Unhandled code_type [{}].", code_type);
        return false;
    }
    // commit
    txn.Commit();
    rollback.Cancel();
    return true;
}

bool lgraph::SingleLanguagePluginManager::DelPlugin(const std::string& user,
                                                    const std::string& name_) {
    if (!IsValidPluginName(name_)) throw InvalidPluginNameException(name_);
    std::string name = ToInternalName(name_);
    std::string path = impl_->GetPluginPath(name);
    AutoReadLock rlock(lock_, GetMyThreadId());
    auto it = procedures_.find(name);
    if (it == procedures_.end()) return false;
    // plugin exists
    AutoWriteLock wlock(lock_, GetMyThreadId());
    auto db_txn = db_->CreateWriteTxn();
    KvTransaction& txn = db_txn.GetTxn();
    table_->DeleteKey(txn, Value::ConstRef(name));
    std::string exe_key = GetSoKey(name);
    table_->DeleteKey(txn, Value::ConstRef(exe_key));
    std::string zip_key = GetZipKey(name);
    std::string cpp_key = GetCppKey(name);
    std::string cython_key = GetCythonKey(name);
    table_->DeleteKey(txn, Value::ConstRef(zip_key));
    table_->DeleteKey(txn, Value::ConstRef(cpp_key));
    table_->DeleteKey(txn, Value::ConstRef(cython_key));

    std::unique_ptr<PluginInfoBase> old_pinfo(std::move(it->second));
    procedures_.erase(it);
    impl_->UnloadPlugin(user, name, old_pinfo.get());
    AutoCleanupAction rollback([&]() {
        // exception can only occur before txn commit,
        // so no need to revert kv state
        // file is deleted after commit, so no need to write file again
        impl_->LoadPlugin(user, name, old_pinfo.get());
        procedures_.emplace(name, old_pinfo.release());
    });
    db_txn.Commit();
    rollback.Cancel();
    fma_common::file_system::RemoveFile(path);
    return true;
}

bool lgraph::SingleLanguagePluginManager::IsReadOnlyPlugin(const std::string& user,
                                                           const std::string& name_) {
    if (!IsValidPluginName(name_)) {
        throw InvalidPluginNameException(name_);
    }
    std::string name = ToInternalName(name_);
    AutoReadLock lock(lock_, GetMyThreadId());
    auto it = procedures_.find(name);
    if (it == procedures_.end()) THROW_CODE(InputError, "Plugin [{}] does not exist.", name);
    return it->second->read_only;
}

bool lgraph::SingleLanguagePluginManager::Call(lgraph_api::Transaction* txn,
                                               const std::string& user,
                                               AccessControlledDB* db_with_access_control,
                                               const std::string& name_, const std::string& request,
                                               double timeout, bool in_process,
                                               std::string& output) {
    std::string name = ToInternalName(name_);
    AutoReadLock lock(lock_, GetMyThreadId());
    auto it = procedures_.find(name);
    if (it == procedures_.end()) return false;
    impl_->DoCall(txn, user, db_with_access_control, name, it->second, request, timeout, in_process,
                  output);
    return true;
}

bool lgraph::SingleLanguagePluginManager::CallV2(lgraph_api::Transaction* txn,
                                                 const std::string& user,
                                                 AccessControlledDB* db_with_access_control,
                                                 const std::string& name_,
                                                 const std::string& request,
                                                 double timeout, bool in_process,
                                                 Result& output) {
    std::string name = ToInternalName(name_);
    AutoReadLock lock(lock_, GetMyThreadId());
    auto it = procedures_.find(name);
    if (it == procedures_.end()) return false;
    impl_->DoCallV2(txn, user, db_with_access_control,
                    name, it->second, request, timeout, in_process,
                    output);
    return true;
}

bool lgraph::SingleLanguagePluginManager::isHashUpTodate(KvTransaction& txn, std::string name) {
    std::string hash_key = GetHashKey(name);
    auto hash_it = table_->GetIterator(txn, Value::ConstRef(hash_key));
    FMA_DBG_ASSERT(hash_it->IsValid());
    const std::string& hash = hash_it->GetValue().AsString();
    return hash == GIT_COMMIT_HASH;
}

void lgraph::SingleLanguagePluginManager::LoadAllPlugins(KvTransaction& txn) {
    auto it = table_->GetIterator(txn);
    for (it->GotoFirstKey(); it->IsValid(); it->Next()) {
        if (!IsNameKey(it->GetKey().AsString())) continue;
        std::string name = it->GetKey().AsString();
        try {
            // load plugin info
            std::unique_ptr<PluginInfoBase> pinfo(impl_->CreatePluginInfo());
            Value v = it->GetValue();
            fma_common::BinaryBuffer info(v.Data(), v.Size());
            fma_common::BinaryRead(info, *pinfo);
            // write file
            bool need_write_file = false;
            {
                // check if commit changes
                if (!isHashUpTodate(txn, name)) need_write_file = true;
            }
            if (!need_write_file) {
                // check if local file exist
                std::string so_key = GetSoKey(name);
                auto so_it = table_->GetIterator(txn, Value::ConstRef(so_key));
                FMA_DBG_ASSERT(so_it->IsValid());
                std::string path = impl_->GetPluginPath(name);
                const std::string& so = so_it->GetValue().AsString();
                auto& fs = fma_common::FileSystem::GetFileSystem(path);
                if (fs.FileExists(path)) {
                    std::string file_content = ReadWholeFile(path, "plugin binary file");
                    if (file_content != so) need_write_file = true;
                } else {
                    need_write_file = true;
                }
            }
            // need_write_file might have changed
            if (need_write_file) {
                std::string zip_key = GetZipKey(name);
                std::string cpp_key = GetCppKey(name);
                std::string so_key = GetSoKey(name);
                std::string cython_key = GetCythonKey(name);
                if (table_->HasKey(txn, Value::ConstRef(zip_key))) {
                    auto zip_it = table_->GetIterator(txn, Value::ConstRef(zip_key));
                    const std::string& zip = zip_it->GetValue().AsString();
                    std::string exe = CompilePluginFromZip(name, zip);
                    WriteWholeFile(impl_->GetPluginPath(name), exe, "");
                    UpdateSoToKvStore(txn, name, exe);
                } else if (table_->HasKey(txn, Value::ConstRef(cpp_key))) {
                    auto file_it = table_->GetIterator(txn, Value::ConstRef(cpp_key));
                    const std::string& file = file_it->GetValue().AsString();
                    std::string exe = CompilePluginFromCpp(name, file);
                    WriteWholeFile(impl_->GetPluginPath(name), exe, "");
                    UpdateSoToKvStore(txn, name, exe);
                } else if (table_->HasKey(txn, Value::ConstRef(cython_key))) {
                    auto file_it = table_->GetIterator(txn, Value::ConstRef(cython_key));
                    const std::string& file = file_it->GetValue().AsString();
                    std::string exe = CompilePluginFromCython(name, file);
                    WriteWholeFile(impl_->GetPluginPath(name), exe, "");
                    UpdateSoToKvStore(txn, name, exe);
                } else if (table_->HasKey(txn, Value::ConstRef(so_key))) {
                    auto file_it = table_->GetIterator(txn, Value::ConstRef(so_key));
                    const std::string& exe = file_it->GetValue().AsString();
                    WriteWholeFile(impl_->GetPluginPath(name), exe, "");
                    UpdateSoToKvStore(txn, name, exe);
                }
                LOG_DEBUG() << "Reload plugin " << name;
            }
            // load plugin
            impl_->LoadPlugin("", name, pinfo.get());
            procedures_.emplace(std::make_pair(name, pinfo.release()));
            LOG_DEBUG() << "Loaded plugin " << name;
        } catch (const std::exception& e) {
            THROW_CODE(InternalError, "Failed to load plugin [{}], err: {}", name, e.what());
        }
    }
}

void lgraph::SingleLanguagePluginManager::UnloadAllPlugins() {
    AutoWriteLock l(lock_, GetMyThreadId());
    for (auto& kv : procedures_) {
        impl_->UnloadPlugin("", kv.first, kv.second);
        delete kv.second;
    }
    procedures_.clear();
}

lgraph::PluginManager::PluginManager(LightningGraph* db, const std::string& graph_name,
                                     const std::string& cpp_plugin_dir,
                                     const std::string& cpp_table_name,
                                     const std::string& python_plugin_dir,
                                     const std::string& python_table_name,
                                     int subprocess_max_idle_seconds) {
    std::unique_ptr<CppPluginManagerImpl> cpp_impl(
        new CppPluginManagerImpl(db, graph_name, cpp_plugin_dir));
    cpp_manager_.reset(new SingleLanguagePluginManager(plugin::PLUGIN_LANG_TYPE_CPP, db, graph_name,
                                                       cpp_plugin_dir, cpp_table_name,
                                                       std::move(cpp_impl)));
#if LGRAPH_ENABLE_PYTHON_PLUGIN
    std::unique_ptr<PythonPluginManagerImpl> python_impl(new PythonPluginManagerImpl(
        db, graph_name, python_plugin_dir, subprocess_max_idle_seconds));
    python_manager_.reset(new SingleLanguagePluginManager(
        plugin::PLUGIN_LANG_TYPE_PYTHON, db, graph_name, python_plugin_dir, python_table_name,
        std::move(python_impl)));
#endif
}

lgraph::PluginManager::~PluginManager() {}

std::vector<lgraph::PluginDesc> lgraph::PluginManager::ListPlugins(PluginType type,
                                                                   const std::string& user) {
    auto & sm = SelectManager(type);
    if (sm) return sm->ListPlugins(user);
    return {};
}

bool lgraph::PluginManager::GetPluginSignature(lgraph::PluginManager::PluginType type,
                                               const std::string& user, const std::string& name,
                                               lgraph_api::SigSpec** sig_spec) {
    return SelectManager(type)->GetPluginSigSpec(user, name, sig_spec);
}

bool lgraph::PluginManager::GetPluginCode(PluginType type, const std::string& user,
                                          const std::string& name, lgraph::PluginCode& ret) {
    bool rt = SelectManager(type)->GetPluginCode(user, name, ret);
    if (ret.code_type == "so_or_py") {
        ret.code_type = (type == PluginType::CPP) ? "so" : "py";
    }
    return rt;
}

bool lgraph::PluginManager::LoadPluginFromCode(PluginType type, const std::string& user,
                                               const std::string& name,
                                               const std::vector<std::string>& code,
                                               const std::vector<std::string>& filename,
                                               plugin::CodeType code_type, const std::string& desc,
                                               bool read_only, const std::string& version) {
    return SelectManager(type)->LoadPluginFromCode(user, name, code, filename, code_type, desc,
                                                   read_only, version);
}

bool lgraph::PluginManager::DelPlugin(PluginType type, const std::string& user,
                                      const std::string& name) {
    return SelectManager(type)->DelPlugin(user, name);
}

bool lgraph::PluginManager::IsReadOnlyPlugin(PluginType type, const std::string& user,
                                             const std::string& name_) {
    return SelectManager(type)->IsReadOnlyPlugin(user, name_);
}

bool lgraph::PluginManager::Call(lgraph_api::Transaction* txn, PluginType type,
                                 const std::string& user,
                                 AccessControlledDB* db_with_access_control,
                                 const std::string& name_, const std::string& request,
                                 double timeout, bool in_process, std::string& output) {
    return SelectManager(type)->Call(txn, user, db_with_access_control, name_, request, timeout,
                                     in_process, output);
}

bool lgraph::PluginManager::CallV2(lgraph_api::Transaction* txn, PluginType type,
                                   const std::string& user,
                                   AccessControlledDB* db_with_access_control,
                                   const std::string& name_, const std::string& request,
                                   double timeout, bool in_process, Result& output) {
    return SelectManager(type)->CallV2(txn, user, db_with_access_control, name_, request, timeout,
                                       in_process, output);
}

