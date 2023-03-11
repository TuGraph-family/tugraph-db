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
    const std::string& graph_name, const std::string& plugin_dir, const std::string& table_name,
    KvTransaction& txn, KvStore& store, std::unique_ptr<PluginManagerImplBase>&& impl) {
    impl_ = std::move(impl);
    graph_name_ = graph_name;
    plugin_dir_ = plugin_dir;
    auto& fs = fma_common::FileSystem::GetFileSystem(plugin_dir);
    if (!fs.IsDir(plugin_dir) && !fs.Mkdir(plugin_dir)) {
        throw InternalError("Failed to create plugin dir [{}].", plugin_dir);
    }
    table_ = store.OpenTable(txn, table_name, true, ComparatorDesc::DefaultComparator());
    LoadAllPlugins(txn);
}

lgraph::SingleLanguagePluginManager::SingleLanguagePluginManager(
    LightningGraph* db, const std::string& graph_name, const std::string& plugin_dir,
    const std::string& table_name, std::unique_ptr<PluginManagerImplBase>&& impl)
    : db_(db), graph_name_(graph_name) {
    impl_ = std::move(impl);
    plugin_dir_ = plugin_dir;
    auto& fs = fma_common::FileSystem::GetFileSystem(plugin_dir);
    if (!fs.IsDir(plugin_dir) && !fs.Mkdir(plugin_dir)) {
        throw InternalError("Failed to create plugin dir [{}].", plugin_dir);
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
    }
    procedures_.clear();
    table_.Drop(txn);
}

void lgraph::SingleLanguagePluginManager::DeleteAllPlugins(const std::string& user) {
    auto txn = db_->CreateWriteTxn();
    DeleteAllPlugins(user, txn.GetTxn());
    txn.Commit();
}

std::vector<lgraph::PluginDesc> lgraph::SingleLanguagePluginManager::ListPlugins(
    const std::string& user) {
    AutoReadLock lock(lock_, GetMyThreadId());
    std::vector<PluginDesc> plugins;
    for (auto it = procedures_.begin(); it != procedures_.end(); it++) {
        plugins.emplace_back(FromInternalName(it->first), it->second->desc, it->second->read_only);
    }
    return plugins;
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
        if (table_.HasKey(txn.GetTxn(), Value::ConstRef(zip_key))) {
            auto zip_it = table_.GetIterator(txn.GetTxn(), Value::ConstRef(zip_key));
            const std::string& zip = zip_it.GetValue().AsString();
            ret.code = zip;
            ret.code_type = "zip";
        } else if (table_.HasKey(txn.GetTxn(), Value::ConstRef(cpp_key))) {
            auto cpp_it = table_.GetIterator(txn.GetTxn(), Value::ConstRef(cpp_key));
            const std::string& cpp = cpp_it.GetValue().AsString();
            ret.code = cpp;
            ret.code_type = "cpp";
        } else if (table_.HasKey(txn.GetTxn(), Value::ConstRef(cython_key))) {
            auto cython_it = table_.GetIterator(txn.GetTxn(), Value::ConstRef(cython_key));
            const std::string& cython = cython_it.GetValue().AsString();
            ret.code = cython;
            ret.code_type = "py";
        } else {
            auto so_it = table_.GetIterator(txn.GetTxn(), Value::ConstRef(so_key));
            const std::string& so = so_it.GetValue().AsString();
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
    table_.SetValue(txn, Value::ConstRef(so_key), Value::ConstRef(so));
    // write hash
    std::string hash_key = GetHashKey(name);
    table_.SetValue(txn, Value::ConstRef(hash_key), Value::ConstRef(GIT_COMMIT_HASH));
}

void lgraph::SingleLanguagePluginManager::UpdateZipToKvStore(KvTransaction& txn,
                                                             const std::string& name,
                                                             const std::string& zip) {
    // write .zip
    std::string zip_key = GetZipKey(name);
    table_.SetValue(txn, Value::ConstRef(zip_key), Value::ConstRef(zip));
}

void lgraph::SingleLanguagePluginManager::UpdateCppToKvStore(KvTransaction& txn,
                                                             const std::string& name,
                                                             const std::string& cpp) {
    // write cpp file
    std::string cpp_key = GetCppKey(name);
    table_.SetValue(txn, Value::ConstRef(cpp_key), Value::ConstRef(cpp));
}

void lgraph::SingleLanguagePluginManager::UpdateCythonToKvStore(KvTransaction& txn,
                                                                const std::string& name,
                                                                const std::string& cython) {
    // write cython file
    std::string cython_key = GetCythonKey(name);
    table_.SetValue(txn, Value::ConstRef(cython_key), Value::ConstRef(cython));
}

void lgraph::SingleLanguagePluginManager::UpdateInfoToKvStore(KvTransaction& txn,
                                                              const std::string& name,
                                                              fma_common::BinaryBuffer& info) {
    table_.SetValue(txn, Value::ConstRef(name), Value(info.GetBuf(), info.GetSize()));
}

static void ExecuteCommand(const std::string& cmd, size_t timeout_ms,
                           const std::string& msg_timeout, const std::string& msg_fail) {
    lgraph::SubProcess proc(cmd, false);
    if (!proc.Wait(lgraph::_detail::MAX_UNZIP_TIME_MS))
        throw lgraph::InputError(FMA_FMT("{} \nStdout:----\n{}\nStderr:----\n{}", msg_timeout,
                                 proc.Stdout(), proc.Stderr()));
    if (proc.GetExitCode())
        throw lgraph::InputError(
            FMA_FMT("{} \nStdout:----\n{}\nStderr:----\n{}",
                    msg_fail, proc.Stdout(), proc.Stderr()));
}

static inline std::string GenUniqueTempDir(const std::string& base, const std::string& name) {
    return FMA_FMT("{}{}{}{}_{}.tmp", base,
                   fma_common::FileSystem::GetFileSystem(base).PathSeparater(), name, rand(),
                   std::chrono::high_resolution_clock::now().time_since_epoch().count());
}

static inline void WriteWholeFile(const std::string& path, const std::string& code,
                                  const std::string& file_desc) {
    fma_common::OutputFmaStream ofs(path);
    if (!ofs.Good()) throw lgraph::InternalError("Failed to write {} [{}].", file_desc, path);
    ofs.Write(code.data(), code.size());
}

static inline std::string ReadWholeFile(const std::string& path, const std::string& file_desc) {
    fma_common::InputFmaStream ifs(path, 0);
    if (!ifs.Good()) throw lgraph::InternalError("Failed to open {} [{}].", file_desc, path);
    size_t sz = ifs.Size();
    std::string code;
    code.resize(sz);
    size_t ssz = ifs.Read(&code[0], sz);
    if (ssz != sz) throw lgraph::InternalError("Failed to read {} [{}].", file_desc, path);
    return code;
}

std::string lgraph::SingleLanguagePluginManager::CompilePluginFromCython(
    const std::string& name, const std::string& cython) {
#ifdef _WIN32
    throw InputError("Compiling cython is not supported on Windows.");
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
    std::string cmd = FMA_FMT("cython {} -+ -3 -I{}/../../src/cython/ -o {}  --module-name {}",
                              cython_file_path, exec_dir, cpp_file_path, name);
    ExecuteCommand(cmd, _detail::MAX_COMPILE_TIME_MS, "Timeout while translate cython to c++.",
                   "Failed to translated cython. cmd: " + cmd);

    // compile
    std::string CFLAGS = FMA_FMT("-I{}/../../include -I/usr/local/include "
                                 "-I/usr/include/python3.6m "
                                 "-I/usr/local/include/python3.6m "
                                 "-I{}/../../deps/fma-common "
                                 "-I{}/../../src",
                                 exec_dir, exec_dir, exec_dir);
//    std::string LDFLAGS = FMA_FMT("-llgraph -L{}/ -L/usr/local/lib64/ "
//                                  "-L/usr/lib64/ -lpython3.6m", exec_dir);
    std::string LDFLAGS = FMA_FMT("-llgraph -L{}/ -L/usr/local/lib64/ "
                                  "-L/usr/lib64/ ", exec_dir);
#ifndef __clang__
    cmd = FMA_FMT(
        "g++ -fno-gnu-unique -fPIC -g --std=c++17 {} -rdynamic -O3 -fopenmp -o {} {} {} -shared",
        CFLAGS, plugin_path, cpp_file_path, LDFLAGS);
#elif __APPLE__
    throw InputError("Compiling cython is not supported on APPLE.");
#else
    throw InputError("Compiling cython is not supported on clang.");
#endif
    ExecuteCommand(cmd, _detail::MAX_COMPILE_TIME_MS, "Timeout while compiling plugin.",
                   "Failed to compile plugin.");

    // return
    return ReadWholeFile(plugin_path, "plugin binary file");
}

std::string lgraph::SingleLanguagePluginManager::CompilePluginFromZip(const std::string& name,
                                                                      const std::string& zip) {
#ifdef _WIN32
    throw InputError("Compiling ZIP is not supported on Windows.");
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
    std::string CFLAGS = FMA_FMT("-I{}/../../include -I/usr/local/include", exec_dir);
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
    if (plugin_file == "") throw InputError("Failed to find any .so file in compiled directory.");
    return ReadWholeFile(plugin_file, "plugin binary file");
}

std::string lgraph::SingleLanguagePluginManager::CompilePluginFromCpp(const std::string& name,
                                                                      const std::string& file) {
#ifdef _WIN32

#endif
    std::string base_dir = impl_->GetPluginDir();
    auto& fs = fma_common::FileSystem::GetFileSystem(base_dir);
    std::string tmp_dir = GenUniqueTempDir(base_dir, name);
    std::string file_path = tmp_dir + fs.PathSeparater() + name + ".cpp";
    std::string plugin_path = tmp_dir + fs.PathSeparater() + name + ".so";

    AutoCleanDir tmp_dir_cleaner(tmp_dir);

    // compile
    WriteWholeFile(file_path, file, "plugin source file");
    std::string exec_dir = fma_common::FileSystem::GetExecutablePath().Dir();
    std::string CFLAGS = FMA_FMT("-I{}/../../include -I/usr/local/include", exec_dir);
    std::string LDFLAGS = FMA_FMT("-llgraph -L{}/ -L/usr/local/lib64/", exec_dir);
#ifndef __clang__
    std::string cmd = FMA_FMT(
        "g++ -fno-gnu-unique -fPIC -g --std=c++14 {} -rdynamic -O3 -fopenmp -o {} {} {} -shared",
        CFLAGS, plugin_path, file_path, LDFLAGS);
#elif __APPLE__
    std::string cmd = FMA_FMT(
        "clang++ -stdlib=libc++ -fPIC -g --std=c++14 {} -rdynamic -O3 -Xpreprocessor -fopenmp -o "
        "{} {} {} -shared",
        CFLAGS, plugin_path, file_path, LDFLAGS);
#else
    std::string cmd = FMA_FMT(
        "clang++ -stdlib=libc++ -fPIC -g --std=c++14 {} -rdynamic -O3 -fopenmp -o {} {} {} -shared",
        CFLAGS, plugin_path, file_path, LDFLAGS);
#endif
    ExecuteCommand(cmd, _detail::MAX_COMPILE_TIME_MS, "Timeout while compiling plugin.",
                   "Failed to compile plugin.");

    // return
    return ReadWholeFile(plugin_path, "plugin binary file");
}

void lgraph::SingleLanguagePluginManager::LoadPluginFromPyOrSo(
    const std::string& user, KvTransaction& txn, const std::string& name, const std::string& exe,
    const std::string& desc, bool read_only) {
    std::string plugin_path = impl_->GetPluginPath(name);
    // write so
    WriteWholeFile(plugin_path, exe, "plugin binary file");

    // load dll
    std::unique_ptr<PluginInfoBase> pinfo_(impl_->CreatePluginInfo());
    PluginInfoBase* pinfo = pinfo_.get();
    pinfo->desc = desc;
    pinfo->read_only = read_only;
    impl_->LoadPlugin(user, name, pinfo);
    procedures_.emplace(name, pinfo_.release());

    // update kv and memory status
    fma_common::BinaryBuffer info;
    fma_common::BinaryWrite(info, *pinfo);
    UpdateInfoToKvStore(txn, name, info);
    UpdateSoToKvStore(txn, name, exe);
}

void lgraph::SingleLanguagePluginManager::CompileAndLoadPluginFromCython(
    const std::string& user, KvTransaction& txn, const std::string& name,
    const std::string& cython, const std::string& desc, bool read_only) {
    std::string exe = CompilePluginFromCython(name, cython);
    LoadPluginFromPyOrSo(user, txn, name, exe, desc, read_only);
    UpdateCythonToKvStore(txn, name, cython);
}

void lgraph::SingleLanguagePluginManager::CompileAndLoadPluginFromCpp(
    const std::string& user, KvTransaction& txn, const std::string& name, const std::string& cpp,
    const std::string& desc, bool read_only) {
    std::string exe = CompilePluginFromCpp(name, cpp);
    LoadPluginFromPyOrSo(user, txn, name, exe, desc, read_only);
    UpdateCppToKvStore(txn, name, cpp);
}

void lgraph::SingleLanguagePluginManager::CompileAndLoadPluginFromZip(
    const std::string& user, KvTransaction& txn, const std::string& name, const std::string& zip,
    const std::string& desc, bool read_only) {
    std::string exe = CompilePluginFromZip(name, zip);
    LoadPluginFromPyOrSo(user, txn, name, exe, desc, read_only);
    UpdateZipToKvStore(txn, name, zip);
}

bool lgraph::SingleLanguagePluginManager::LoadPluginFromCode(
    const std::string& user, const std::string& name_, const std::string& code,
    plugin::CodeType code_type, const std::string& desc, bool read_only) {
    // check input
    if (code.empty()) throw InputError("Code cannot be empty.");
    if (!IsValidPluginName(name_)) throw InvalidPluginNameException(name_);

    std::string name = ToInternalName(name_);
    // check if plugin already exist, remove if overwrite
    int tid = GetMyThreadId();
    AutoReadLock rlock(lock_, tid);
    auto it = procedures_.find(name);
    if (it != procedures_.end()) return false;

    // upgrade to write lock
    AutoWriteLock wlock(lock_, tid);
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
    auto txn = db_->CreateWriteTxn();
    switch (code_type) {
    case plugin::CodeType::PY:
        CompileAndLoadPluginFromCython(user, txn.GetTxn(), name, code, desc, read_only);
        break;
    case plugin::CodeType::SO:
        LoadPluginFromPyOrSo(user, txn.GetTxn(), name, code, desc, read_only);
        break;
    case plugin::CodeType::CPP:
        CompileAndLoadPluginFromCpp(user, txn.GetTxn(), name, code, desc, read_only);
        break;
    case plugin::CodeType::ZIP:
        CompileAndLoadPluginFromZip(user, txn.GetTxn(), name, code, desc, read_only);
        break;
    default:
        throw InternalError("Unhandled code_type [{}].", code_type);
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
    table_.DeleteKey(txn, Value::ConstRef(name));
    std::string exe_key = GetSoKey(name);
    table_.DeleteKey(txn, Value::ConstRef(exe_key));
    std::string zip_key = GetZipKey(name);
    std::string cpp_key = GetCppKey(name);
    std::string cython_key = GetCythonKey(name);
    table_.DeleteKey(txn, Value::ConstRef(zip_key));
    table_.DeleteKey(txn, Value::ConstRef(cpp_key));
    table_.DeleteKey(txn, Value::ConstRef(cython_key));

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
    if (it == procedures_.end()) throw InputError("Plugin [{}] does not exist.", name);
    return  it->second->read_only;
}

bool lgraph::SingleLanguagePluginManager::Call(const std::string& user,
                                               AccessControlledDB* db_with_access_control,
                                               const std::string& name_, const std::string& request,
                                               double timeout, bool in_process,
                                               std::string& output) {
    std::string name = ToInternalName(name_);
    AutoReadLock lock(lock_, GetMyThreadId());
    auto it = procedures_.find(name);
    if (it == procedures_.end()) return false;
    impl_->DoCall(user, db_with_access_control, name, it->second, request, timeout, in_process,
                  output);
    return true;
}

bool lgraph::SingleLanguagePluginManager::isHashUpTodate(KvTransaction& txn, std::string name) {
    std::string hash_key = GetHashKey(name);
    auto hash_it = table_.GetIterator(txn, Value::ConstRef(hash_key));
    FMA_DBG_ASSERT(hash_it.IsValid());
    const std::string& hash = hash_it.GetValue().AsString();
    return hash == GIT_COMMIT_HASH;
}

void lgraph::SingleLanguagePluginManager::LoadAllPlugins(KvTransaction& txn) {
    for (auto it = table_.GetIterator(txn); it.IsValid(); it.Next()) {
        if (!IsNameKey(it.GetKey().AsString())) continue;
        std::string name = it.GetKey().AsString();
        try {
            // load plugin info
            std::unique_ptr<PluginInfoBase> pinfo(impl_->CreatePluginInfo());
            Value v = it.GetValue();
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
                auto so_it = table_.GetIterator(txn, Value::ConstRef(so_key));
                FMA_DBG_ASSERT(so_it.IsValid());
                std::string path = impl_->GetPluginPath(name);
                const std::string& so = so_it.GetValue().AsString();
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
                if (table_.HasKey(txn, Value::ConstRef(zip_key))) {
                    auto zip_it = table_.GetIterator(txn, Value::ConstRef(zip_key));
                    const std::string& zip = zip_it.GetValue().AsString();
                    std::string exe = CompilePluginFromZip(name, zip);
                    WriteWholeFile(impl_->GetPluginPath(name), exe, "");
                    UpdateSoToKvStore(txn, name, exe);
                } else if (table_.HasKey(txn, Value::ConstRef(cpp_key))) {
                    auto file_it = table_.GetIterator(txn, Value::ConstRef(cpp_key));
                    const std::string& file = file_it.GetValue().AsString();
                    std::string exe = CompilePluginFromCpp(name, file);
                    WriteWholeFile(impl_->GetPluginPath(name), exe, "");
                    UpdateSoToKvStore(txn, name, exe);
                } else if (table_.HasKey(txn, Value::ConstRef(cython_key))) {
                    auto file_it = table_.GetIterator(txn, Value::ConstRef(cython_key));
                    const std::string& file = file_it.GetValue().AsString();
                    std::string exe = CompilePluginFromCython(name, file);
                    WriteWholeFile(impl_->GetPluginPath(name), exe, "");
                    UpdateSoToKvStore(txn, name, exe);
                } else if (table_.HasKey(txn, Value::ConstRef(so_key))) {
                    auto file_it = table_.GetIterator(txn, Value::ConstRef(so_key));
                    const std::string& exe = file_it.GetValue().AsString();
                    WriteWholeFile(impl_->GetPluginPath(name), exe, "");
                    UpdateSoToKvStore(txn, name, exe);
                }
                FMA_DBG() << "Reload plugin " << name;
            }
            // load plugin
            impl_->LoadPlugin("", name, pinfo.get());
            procedures_.emplace(std::make_pair(name, pinfo.release()));
            FMA_DBG() << "Loaded plugin " << name;
        } catch (...) {
            std::throw_with_nested(InternalError("Failed to load plugin [{}].", name));
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
    cpp_manager_.reset(new SingleLanguagePluginManager(db, graph_name, cpp_plugin_dir,
                                                       cpp_table_name, std::move(cpp_impl)));
#if LGRAPH_ENABLE_PYTHON_PLUGIN
    std::unique_ptr<PythonPluginManagerImpl> python_impl(new PythonPluginManagerImpl(
        db, graph_name, python_plugin_dir, subprocess_max_idle_seconds));
    python_manager_.reset(new SingleLanguagePluginManager(
        db, graph_name, python_plugin_dir, python_table_name, std::move(python_impl)));
#endif
}

lgraph::PluginManager::~PluginManager() {}

std::vector<lgraph::PluginDesc> lgraph::PluginManager::ListPlugins(PluginType type,
                                                                   const std::string& user) {
    return SelectManager(type)->ListPlugins(user);
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
                                               const std::string& name, const std::string& code,
                                               plugin::CodeType code_type, const std::string& desc,
                                               bool read_only) {
    return SelectManager(type)->LoadPluginFromCode(user, name, code, code_type, desc, read_only);
}

bool lgraph::PluginManager::DelPlugin(PluginType type, const std::string& user,
                                      const std::string& name) {
    return SelectManager(type)->DelPlugin(user, name);
}

bool lgraph::PluginManager::IsReadOnlyPlugin(PluginType type, const std::string& user,
                                            const std::string& name_) {
    return SelectManager(type)->IsReadOnlyPlugin(user, name_);
}

bool lgraph::PluginManager::Call(PluginType type, const std::string& user,
                                 AccessControlledDB* db_with_access_control,
                                 const std::string& name_, const std::string& request,
                                 double timeout, bool in_process, std::string& output) {
    return SelectManager(type)->Call(user, db_with_access_control, name_, request, timeout,
                                     in_process, output);
}
