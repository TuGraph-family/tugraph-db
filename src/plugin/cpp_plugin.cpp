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
#include "tools/lgraph_log.h"
#include "fma-common/binary_buffer.h"
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

void CppPluginManagerImpl::OpenDynamicLib(const PluginInfoBase* pinfo, DynamicLibinfo &dinfo) {
    using namespace lgraph::dll;
    const auto* info = dynamic_cast<const PluginInfo*>(pinfo);
    dinfo.lib_handle = LoadDynamicLibrary(info->path);
    if (!dinfo.lib_handle) {
        auto errMsg = GetLastErrorMsg();
        if (errMsg.find("cannot allocate memory in static TLS block") != std::string::npos) {
            errMsg += ". Wait for other tasks to finish and try again";
        }
        THROW_CODE(InputError, "Failed to load the DLL: " + errMsg);
    }
    if (info->has_func) {
        dinfo.func = GetDllFunction<PluginFunc*>(dinfo.lib_handle, "Process");
        if (!dinfo.func) {
            UnloadDynamicLibrary(dinfo.lib_handle);
            THROW_CODE(InputError,
                       "Failed to get Process() function in the DLL: " + GetLastErrorMsg());
        }
    } else {
        dinfo.get_sig_spec = GetDllFunction<SignatureGetter*>(dinfo.lib_handle, "GetSignature");
        dinfo.func_txn = GetDllFunction<PluginFuncInTxn*>(dinfo.lib_handle, "ProcessInTxn");
        if (!dinfo.func_txn) {
            UnloadDynamicLibrary(dinfo.lib_handle);
            THROW_CODE(InputError,
                "Failed to get ProcessInTxn() function in the DLL: "
                + GetLastErrorMsg());
        }
    }
}

void CppPluginManagerImpl::CloseDynamicLib(DynamicLibinfo &dinfo) {
    using namespace lgraph::dll;
    if (!UnloadDynamicLibrary(dinfo.lib_handle))
        THROW_CODE(InternalError, "Failed to unload library.");
}

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
    DynamicLibinfo info;
    OpenDynamicLib(pinfo, info);
    if (info.func) {
        PluginFunc* procedure = info.func;
        lgraph_api::GraphDB db(db_with_access_control, pinfo->read_only);
        r = procedure(db, request, output);
    } else if (info.func_txn && txn != nullptr) {
        PluginFuncInTxn * procedure = info.func_txn;
        Result re;
        r = procedure(*txn, request, re);
        output = re.Dump();
    }
    CloseDynamicLib(info);

    if (!r) THROW_CODE(InputError, "Plugin returned false. Output: {}.", output);
}

void CppPluginManagerImpl::DoCallV2(lgraph_api::Transaction* txn,
                                    const std::string& user,
                                    AccessControlledDB* db_with_access_control,
                                    const std::string name, const PluginInfoBase* pinfo,
                                    const std::string& request,
                                    double timeout, bool in_process, Result& output) {
    if (timeout > 0) {
        // TODO: schedule a timer event to kill this task // NOLINT
    }

    // TODO: support in_process // NOLINT
    bool r = false;
    DynamicLibinfo info;
    OpenDynamicLib(pinfo, info);
    if (info.func) {
        CloseDynamicLib(info);
        THROW_CODE(InputError, "Only support the V2 version procedure");
    } else if (info.func_txn && txn != nullptr) {
        PluginFuncInTxn * procedure = info.func_txn;
        r = procedure(*txn, request, output);
    }
    CloseDynamicLib(info);
    if (!r) THROW_CODE(InputError,
                       FMA_FMT("Plugin returned false. Output: {}.", output.Dump(false)));
}

void CppPluginManagerImpl::LoadPlugin(const std::string& user, const std::string& name,
                                      PluginInfoBase* pinfo) {
    using namespace lgraph::dll;
    PluginInfo* info = dynamic_cast<PluginInfo*>(pinfo);
    info->path = GetPluginPath(name);
    auto lib_handle = LoadDynamicLibrary(info->path);
    if (!lib_handle) {
        auto errMsg = GetLastErrorMsg();
        if (errMsg.find("cannot allocate memory in static TLS block") != std::string::npos) {
            errMsg += ". Wait for other tasks to finish and try again";
        }
        THROW_CODE(InputError, "Failed to load the DLL: " + errMsg);
    }
    {
        auto get_sig_spec = GetDllFunction<SignatureGetter*>(lib_handle, "GetSignature");
        // it's ok for plugin which DOES NOT have `GetSignature` function.
        // Plugins without `GetSignature` are not guaranteed to call safely in InQueryCall context.
        if (!get_sig_spec) {
            auto func = GetDllFunction<PluginFunc*>(lib_handle, "Process");
            if (!func) {
                UnloadDynamicLibrary(lib_handle);
                THROW_CODE(InputError, "Failed to get Process() function in the DLL: " +
                                 GetLastErrorMsg());
            }
            info->sig_spec = nullptr;
            info->has_func = true;
        } else {
            auto func_txn = GetDllFunction<PluginFuncInTxn*>(lib_handle, "ProcessInTxn");
            if (!func_txn) {
                UnloadDynamicLibrary(lib_handle);
                THROW_CODE(InputError, "Failed to get Process() function in the DLL: " +
                                 GetLastErrorMsg());
            }
            auto sig_spec = std::make_unique<lgraph_api::SigSpec>();
            bool r = get_sig_spec(*sig_spec);
            if (!r) THROW_CODE(InputError, "Failed to get Signature");
            info->sig_spec = std::move(sig_spec);
            info->has_func = false;
        }
    }
    if (!UnloadDynamicLibrary(lib_handle)) {
        THROW_CODE(InputError, "Failed to unload DLL: " + GetLastErrorMsg());
    }
}

void CppPluginManagerImpl::UnloadPlugin(const std::string& user, const std::string& name,
                                        PluginInfoBase* pinfo) {}
}  // namespace lgraph
