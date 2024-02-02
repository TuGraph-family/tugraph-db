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

#include "fma-common/binary_read_write_helper.h"
#include "fma-common/rw_lock.h"

#include "core/defs.h"
#include "core/kv_store.h"
#include "lgraph/lgraph.h"
#include "lgraph/lgraph_types.h"

#include "plugin/load_library.h"
#include "plugin/plugin_manager_impl.h"

namespace lgraph {
class LightningGraph;

class CppPluginManagerImpl : public PluginManagerImplBase {
 private:
    struct DynamicLibinfo {
        lgraph::dll::LibHandle lib_handle;
        lgraph_api::Process* func = nullptr;
        lgraph_api::ProcessInTxn* func_txn = nullptr;
        lgraph_api::GetSignature* get_sig_spec = nullptr;
    };
    /**
     * Open the dynamic library and find the run-time address
     * refers to of the symbol "Process".
     *
     * Opening multiple algorithm libraries at the same time will cause the problem
     * of "cannot allocate memory in static TLS block". Use OpenDynamicLib to load the
     * plugin each time before calling the plugin, and use CloseDynamicLib to unload
     * the plugin after calling the plugin.
     *
     * @param [in]      pinfo    If non-null, the pinfo.
     * @param [in,out]  dinfo    plugin handle.
     *
     * */
    void OpenDynamicLib(const PluginInfoBase* pinfo, DynamicLibinfo &dinfo);

    /**
     * Close the dynamic library.
     *
     * Opening multiple algorithm libraries at the same time will cause the problem
     * of "cannot allocate memory in static TLS block". Use OpenDynamicLib to load the
     * plugin each time before calling the plugin, and use CloseDynamicLib to unload
     * the plugin after calling the plugin.
     *
     * @param [in,out]  dinfo    plugin handle.
     * */
    void CloseDynamicLib(DynamicLibinfo &dinfo);

 protected:
    typedef lgraph_api::Process PluginFunc;
    typedef lgraph_api::ProcessInTxn PluginFuncInTxn;
    typedef lgraph_api::GetSignature SignatureGetter;

    struct PluginInfo : public PluginInfoBase {
        std::string path;
        bool has_func = true;
    };

    LightningGraph* db_;
    std::string graph_name_;
    std::string plugin_dir_;

 public:
    CppPluginManagerImpl(LightningGraph* db, const std::string& graph_name,
                         const std::string& plugin_dir);

    ~CppPluginManagerImpl();

 protected:
    /**
     * Loads a plugin and sets contents in pinfo accordingly.
     *
     * Opening multiple algorithm libraries at the same time will cause the problem
     * of "cannot allocate memory in static TLS block". Call OpenDynamicLib to verify
     * that there are no errors in the plugin file and then call CloseDynamicLib to
     * uninstall the dynamic library.
     *
     * @param          name     The name.
     * @param [in,out] pinfo    If non-null, the pinfo.
     * @param [in,out] error    The error.
     *
     * @return  The plugin.
     */
    void LoadPlugin(const std::string& user, const std::string& name,
                    PluginInfoBase* pinfo) override;

    /**
     * Unload plugin and set pinfo if necessary.
     *
     * Does nothing because the dynamic library has already been unloaded
     * in CloseDynamicLib.
     *
     * @param [in,out] pinfo    If non-null, the pinfo.
     * @param [in,out] error    The error.
     *
     * @return  A plugin::ErrorCode.
     */
    void UnloadPlugin(const std::string& user, const std::string& name,
                      PluginInfoBase* pinfo) override;

    /**
     * Creates plugin information. Different PluginManager will use different PluginInfo structures.
     *
     * @return  Null if it fails, else the new plugin information.
     */
    PluginInfoBase* CreatePluginInfo() override { return new PluginInfo(); };

    /**
     * Gets plugin path given its name.
     *
     * @param name  The name.
     *
     * @return  The plugin path.
     */
    std::string GetPluginPath(const std::string& name) override {
        return fma_common::file_system::JoinPath(plugin_dir_, name +
#ifdef _WIN32
                                                                  ".dll"
#else
                                                                  ".so"
#endif
        );  // NOLINT
    }

    std::string GetPluginDir() override { return plugin_dir_; }

    std::string GetTaskName(const std::string& name) override { return "[CPP_PLUGIN] " + name; }

    /**
     * Executes the call operation.
     *
     * @param [in,out] pinfo        If non-null, the pinfo.
     * @param          request      The request.
     * @param          timeout      The timeout.
     * @param          in_process   True to in process.
     * @param [in,out] output       The output.
     *
     * @return  A plugin::ErrorCode.
     */
    void DoCall(lgraph_api::Transaction* txn,
                const std::string& user,
                AccessControlledDB* db_with_access_control,
                const std::string name, const PluginInfoBase* pinfo, const std::string& request,
                double timeout, bool in_process, std::string& output) override;

    void DoCallV2(lgraph_api::Transaction* txn,
                  const std::string& user,
                  AccessControlledDB* db_with_access_control,
                  const std::string name, const PluginInfoBase* pinfo, const std::string& request,
                  double timeout, bool in_process, Result& output) override;
};
}  // namespace lgraph
