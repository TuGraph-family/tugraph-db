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

#include <string>
#include <vector>

#include "fma-common/binary_buffer.h"
#include "fma-common/fma_stream.h"
#include "fma-common/rw_lock.h"
#include "fma-common/timed_task.h"

#include "core/defs.h"
#include "core/kv_store.h"
#include "core/killable_rw_lock.h"
#include "core/lightning_graph.h"

#include "db/db.h"

#include "plugin/cpp_plugin.h"
#include "plugin/python_plugin.h"
#include "plugin/plugin_context.h"

namespace lgraph {
/** SingleLanguagePluginManager manages the plugins of one language.
 *  Currently, we support Python and C++. */
class SingleLanguagePluginManager {
 protected:
    KillableRWLock lock_;
    LightningGraph* db_ = nullptr;
    std::string language_;
    std::string graph_name_;
    std::string plugin_dir_;
    std::unique_ptr<KvTable> table_;
    std::map<std::string, PluginInfoBase*> procedures_;
    std::unique_ptr<PluginManagerImplBase> impl_;

 public:
    SingleLanguagePluginManager(const std::string& language, const std::string& graph_name,
                                const std::string& plugin_dir, const std::string& table_name,
                                KvTransaction& txn, KvStore& store,
                                std::unique_ptr<PluginManagerImplBase>&& impl);

    SingleLanguagePluginManager(const std::string& language, LightningGraph* db,
                                const std::string& graph_name, const std::string& plugin_dir,
                                const std::string& table_name,
                                std::unique_ptr<PluginManagerImplBase>&& impl);

    virtual ~SingleLanguagePluginManager();

    void DeleteAllPlugins(const std::string& user, KvTransaction& txn);

    void DeleteAllPlugins(const std::string& user);

    /**
     * List all plugins
     *
     * @return  A std::vector&lt;PluginManager::PluginDesc&gt;
     */
    virtual std::vector<PluginDesc> ListPlugins(const std::string& user);

    // get plugin signature specification
    // return true if success, false if plugin does not exist
    virtual bool GetPluginSigSpec(const std::string& user, const std::string& name,
                                  lgraph_api::SigSpec** sig_spec);

    // get plugin code
    // return true if success, false if plugin does not exist
    virtual bool GetPluginCode(const std::string& user, const std::string& name, PluginCode& ret);

    // load plugin from code
    // return true if success, false if plugin already exists
    // throws on error
    virtual bool LoadPluginFromCode(const std::string& user, const std::string& name,
                                    const std::vector<std::string>& code,
                                    const std::vector<std::string>& filename,
                                    plugin::CodeType code_type,
                                    const std::string& desc, bool read_only,
                                    const std::string& version);

    // delete plugin
    // return true if success, false if plugin does not exist
    // throws on error
    virtual bool DelPlugin(const std::string& user, const std::string& name);

    virtual bool IsReadOnlyPlugin(const std::string& user, const std::string& name_);

    // calls plugin
    // returns true if success, false if no such plugin
    // TODO(jinyejun.jyj): Split into two overloaded function, one takes txn and the other one
    // takes db_with_access_control
    virtual bool Call(lgraph_api::Transaction* txn, const std::string& user,
                      AccessControlledDB* db_with_access_control, const std::string& name_,
                      const std::string& request, double timeout, bool in_process,
                      std::string& output);

    virtual bool CallV2(lgraph_api::Transaction* txn, const std::string& user,
                        AccessControlledDB* db_with_access_control, const std::string& name_,
                        const std::string& request, double timeout, bool in_process,
                        Result& output);

 protected:
    void LoadAllPlugins(KvTransaction& txn);

    inline std::string ToInternalName(const std::string& name) const { return "_fma_" + name; }

    inline std::string FromInternalName(const std::string& name) const {
        FMA_DBG_ASSERT(name.size() > 5);
        return name.substr(5);
    }

    inline std::string GetSoKey(const std::string& name) const { return "@so_" + name; }

    inline std::string GetZipKey(const std::string& name) const { return "@zip_" + name; }

    inline std::string GetCppKey(const std::string& name) const { return "@cpp_" + name; }

    inline std::string GetCythonKey(const std::string& name) const { return "@cython_" + name; }

    inline std::string GetHashKey(const std::string& name) const { return "@hash_" + name; }

    inline bool IsSoKey(const std::string& key) const {
        return fma_common::StartsWith(key, "@so_");
    }

    inline bool IsZipKey(const std::string& key) const {
        return fma_common::StartsWith(key, "@zip_");
    }

    inline bool IsCppKey(const std::string& key) const {
        return fma_common::StartsWith(key, "@cpp_");
    }

    inline bool IsCythonKey(const std::string& key) const {
        return fma_common::StartsWith(key, "@cython_");
    }

    inline bool IsHashKey(const std::string& key) const {
        return fma_common::StartsWith(key, "@hash_");
    }

    inline bool IsNameKey(const std::string& key) const {
        return !fma_common::StartsWith(key, "@");
    }

    inline bool IsValidPluginName(const std::string& name) const {
        if (name.empty() || name.size() >= 64) return false;
        for (auto& c : name) {
            if ((uint8_t)c < 128 && !fma_common::TextParserUtils::IsValidNameCharacter(c))
                return false;
        }
        return true;
    }

    void UnloadAllPlugins();

    void LoadPlugin(const std::string& user, KvTransaction& txn, const std::string& name,
                    const std::string& exe, const std::string& desc, bool read_only,
                    const std::string& version);

    // compile plugin and return binary code
    std::string CompilePluginFromCython(const std::string& name, const std::string& cython);

    // compile plugin and return binary code
    std::string CompilePluginFromZip(const std::string& name, const std::string& zip);

    // compile plugin and return binary code
    std::string CompilePluginFromCpp(const std::string& name, const std::string& cpp);

    void UpdateSoToKvStore(KvTransaction& txn, const std::string& name, const std::string& exe);

    void UpdateZipToKvStore(KvTransaction& txn, const std::string& name, const std::string& zip);

    void UpdateCppToKvStore(KvTransaction& txn, const std::string& name, const std::string& cpp);

    void UpdateCythonToKvStore(KvTransaction& txn, const std::string& name,
                               const std::string& cython);

    void UpdateInfoToKvStore(KvTransaction& txn, const std::string& name,
                             fma_common::BinaryBuffer& info);

    bool isHashUpTodate(KvTransaction& txn, std::string name);

    std::string SignatureToJsonString(const lgraph_api::SigSpec& spec);

    inline std::string MergeCodeFiles(const std::vector<std::string>& code,
                               const std::vector<std::string>& filename, const std::string& name) {
        std::string all_codes;
        if (filename.empty()) {
            all_codes += FMA_FMT("//{}\n{}{}", name + ".cpp", code[0],
                                 lgraph::plugin::PLUGIN_CODE_DELIMITER);
            return all_codes;
        }
        for (size_t i = 0; i < code.size(); i++) {
            all_codes += FMA_FMT("//{}\n{}{}", filename[i], code[i],
                                 lgraph::plugin::PLUGIN_CODE_DELIMITER);
        }
        return all_codes;
    }

    inline void SplitCode(std::vector<std::string>& code,
                          std::vector<std::string>& filename, const std::string& all_code) {
        std::string::size_type startPos = 0;
        std::string::size_type delimiterPos;
        std::string::size_type filenamePos;
        code.clear();
        filename.clear();
        while ((delimiterPos = all_code.find(lgraph::plugin::PLUGIN_CODE_DELIMITER, startPos)) !=
               std::string::npos) {
            filenamePos = all_code.find('\n', startPos);
            filename.emplace_back(all_code.substr(startPos + 2, filenamePos - startPos - 2));
            startPos = filenamePos + 1;
            code.emplace_back(all_code.substr(startPos, delimiterPos - startPos));
            startPos = delimiterPos + std::string(lgraph::plugin::PLUGIN_CODE_DELIMITER).size();
        }
    }
};

class PluginManager {
    std::unique_ptr<SingleLanguagePluginManager> cpp_manager_;
    std::unique_ptr<SingleLanguagePluginManager> python_manager_;

 public:
    typedef plugin::Type PluginType;

    PluginManager(LightningGraph* db, const std::string& graph_name,
                  const std::string& cpp_plugin_dir, const std::string& cpp_table_name,
                  const std::string& python_plugin_dir, const std::string& python_table_name,
                  int subprocess_max_idle_seconds);

    ~PluginManager();

    std::vector<PluginDesc> ListPlugins(PluginType type, const std::string& user);

    /**
     * Loads plugin from code
     *
     * @param          type                 The language type, CPP or PYTHON.
     * @param          user                 The user.
     * @param          name                 The plugin name.
     * @param          ret                  The code structure, include desc.
     *
     * @return  true if success, false if not found the plugin.
     */
    bool GetPluginSignature(PluginType type, const std::string& user, const std::string& name,
                            lgraph_api::SigSpec** sig_spec);

    /**
     * Loads plugin from code
     *
     * @param          type                 The language type, CPP or PYTHON.
     * @param          user                 The user.
     * @param          name                 The plugin name.
     * @param          ret                  The code structure, include desc.
     *
     * @return  true if success, false if not found the plugin.
     */
    bool GetPluginCode(PluginType type, const std::string& user, const std::string& name,
                       PluginCode& ret);

    /**
     * Loads plugin from code
     *
     * @param          type                 The language type, CPP or PYTHON.
     * @param          user                 The user.
     * @param          name                 The plugin name.
     * @param          code                 The code.
     * @param          code_type            The code type, py, so, cpp or zip.
     * @param          desc                 The description.
     * @param          read_only            True to read only.
     * @param          version              v1 or v2. v1 for legacy, v2 for POG.
     *
     * @return  true if success, false if plugin already exists
     */
    virtual bool LoadPluginFromCode(PluginType type, const std::string& user,
                                    const std::string& name, const std::vector<std::string>& code,
                                    const std::vector<std::string>& filename,
                                    plugin::CodeType code_type, const std::string& desc,
                                    bool read_only, const std::string& version);

    /**
     * Deletes the plugin
     *
     * @param          type     The language type, either CPP or PYTHON.
     * @param          name     The name.
     * @param [in,out] error    The error.
     *
     * @return  A plugin::ErrorCode.
     */
    bool DelPlugin(PluginType type, const std::string& user, const std::string& name);

    /**
     * Is this plugin read-only?
     *
     * @param          type         The name.
     * @param          name_        The name.
     * @param [in,out] is_read_only True if is read only, false if not.
     *
     * @return  true if read-only, false if read-write
     */
    bool IsReadOnlyPlugin(PluginType type, const std::string& user, const std::string& name_);

    /**
     * Calls a plugin. Note that in_process is currently ignored for CPP.
     *
     * @param          type         The name.
     * @param          name_        The name.
     * @param          request      The request.
     * @param          timeout      The timeout.
     * @param          in_process   True to in process.
     * @param [in,out] output       The output.
     *
     * @return  true if success, false if no such plugin
     */
    bool Call(lgraph_api::Transaction* txn, PluginType type, const std::string& user,
              AccessControlledDB* db_with_access_control, const std::string& name_,
              const std::string& request, double timeout, bool in_process, std::string& output);

    bool CallV2(lgraph_api::Transaction* txn, PluginType type, const std::string& user,
                AccessControlledDB* db_with_access_control, const std::string& name_,
                const std::string& request, double timeout, bool in_process, Result& output);

 protected:
    inline std::unique_ptr<SingleLanguagePluginManager>& SelectManager(PluginType type) {
        return type == PluginType::CPP ? cpp_manager_ : python_manager_;
    }
};
using lgraph_api::LgraphException;
using lgraph_api::ErrorCode;
class InvalidPluginNameException : public LgraphException {
 public:
    explicit InvalidPluginNameException(const std::string& name)
        : LgraphException(ErrorCode::InvalidPluginName, "Invalid plugin name [{}].", name) {}
};

class InvalidPluginVersionException : public LgraphException {
 public:
    explicit InvalidPluginVersionException(const std::string& version)
        : LgraphException(ErrorCode::InvalidPluginVersion,
                          "Invalid plugin version [{}].", version) {}
};
}  // namespace lgraph
