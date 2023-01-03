/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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

void CppPluginManagerImpl::DoCall(const std::string& user,
                                  AccessControlledDB* db_with_access_control,
                                  const std::string name, const PluginInfoBase* pinfo,
                                  const std::string& request, double timeout, bool in_process,
                                  std::string& output) {
    if (timeout > 0) {
        // TODO: schedule a timer event to kill this task // NOLINT
    }

    // TODO: support in_process // NOLINT
    const PluginInfo* info = dynamic_cast<const PluginInfo*>(pinfo);
    PluginFunc* procedure = info->func;
    lgraph_api::GraphDB db(db_with_access_control, info->read_only);
    bool r = procedure(db, request, output);
    if (!r) throw InputError(FMA_FMT("Plugin returned false. Output: {}.", output));
}

void CppPluginManagerImpl::LoadPlugin(const std::string& user, const std::string& name,
                                      PluginInfoBase* pinfo) {
    using namespace lgraph::dll;
    PluginInfo* info = dynamic_cast<PluginInfo*>(pinfo);
    std::string path = GetPluginPath(name);
    info->lib_handle = LoadDynamicLibrary(path);
    if (!info->lib_handle) throw InputError("Failed to load the DLL: " + GetLastErrorMsg());
    info->func = GetDllFunction<PluginFunc*>(info->lib_handle, "Process");
    if (!info->func) {
        UnloadDynamicLibrary(info->lib_handle);
        throw InputError("Failed to get Process() function in the DLL: " + GetLastErrorMsg());
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
