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

#include <stdexcept>
#include "fma-common/binary_buffer.h"
#include "fma-common/logger.h"
#include "fma-common/file_system.h"
#include "fma-common/fma_stream.h"

#include "core/lightning_graph.h"
#include "core/task_tracker.h"

#include "plugin/cpp_plugin.h"
#include "plugin/load_library.h"

namespace lgraph {
using namespace fma_common;

CppPluginManagerImpl::CppPluginManagerImpl(LightningGraph* db, const std::string& graph_name,
                                           const std::string& plugin_dir)
    : db_(db), graph_name_(graph_name), plugin_dir_(plugin_dir) {}

CppPluginManagerImpl::~CppPluginManagerImpl() {}

void CppPluginManagerImpl::DoCall(lgraph_api::Transaction* txn,
                                  const std::string& user,
                                  AccessControlledDB* db_with_access_control,
                                  const std::string name, const PluginInfoBase* pinfo,
                                  const std::string& request, double timeout, bool in_process,
                                  std::string& output) {
    if (timeout > 0) {
        // TODO: schedule a timer event to kill this task // NOLINT
    }

    // TODO: support in_process // NOLINT
    bool r = false;
    const PluginInfo* info = dynamic_cast<const PluginInfo*>(pinfo);
    if (info->func) {
        PluginFunc* procedure = info->func;
        lgraph_api::GraphDB db(db_with_access_control, info->read_only);
        r = procedure(db, request, output);
    } else if (info->func_txn && txn != nullptr) {
        PluginFuncInTxn * procedure = info->func_txn;
        r = procedure(*txn, request, output);
    }
    if (!r) throw InputError(FMA_FMT("Plugin returned false. Output: {}.", output));
}

void CppPluginManagerImpl::LoadPlugin(const std::string& user, const std::string& name,
                                      PluginInfoBase* pinfo) {
    using namespace lgraph::dll;
    PluginInfo* info = dynamic_cast<PluginInfo*>(pinfo);
    std::string path = GetPluginPath(name);
    info->lib_handle = LoadDynamicLibrary(path);
    if (!info->lib_handle) throw InputError("Failed to load the DLL: " + GetLastErrorMsg());
    info->get_sig_spec = GetDllFunction<SignatureGetter*>(info->lib_handle, "GetSignature");
    // it's ok for plugin which DOES NOT have `GetSignature` function.
    // Plugins without `GetSignature` are not guaranteed to call safely in InQueryCall context.
    if (!info->get_sig_spec) {
        info->func = GetDllFunction<PluginFunc*>(info->lib_handle, "Process");
        if (!info->func) {
            UnloadDynamicLibrary(info->lib_handle);
            throw InputError("Failed to get Process() function in the DLL: " + GetLastErrorMsg());
        }
        info->sig_spec = nullptr;
        return;
    } else {
        info->func_txn = GetDllFunction<PluginFuncInTxn*>(info->lib_handle, "ProcessInTxn");
        if (!info->func_txn) {
            UnloadDynamicLibrary(info->lib_handle);
            throw InputError("Failed to get Process() function in the DLL: " + GetLastErrorMsg());
        }
        auto sig_spec = std::make_unique<lgraph_api::SigSpec>();
        bool r = info->get_sig_spec(*sig_spec);
        if (!r) throw InputError(FMA_FMT("Failed to get Signature"));
        info->sig_spec = std::move(sig_spec);
    }
}

void CppPluginManagerImpl::UnloadPlugin(const std::string& user, const std::string& name,
                                        PluginInfoBase* pinfo) {
    using namespace lgraph::dll;
    PluginInfo* info = dynamic_cast<PluginInfo*>(pinfo);
    if (!UnloadDynamicLibrary(info->lib_handle))
        throw InternalError("Failed to unload library [{}].", name);
}
}  // namespace lgraph
