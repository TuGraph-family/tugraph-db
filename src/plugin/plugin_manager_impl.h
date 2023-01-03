/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include <stdexcept>
#include <string>

#include "fma-common/binary_buffer.h"
#include "fma-common/binary_read_write_helper.h"

#include "core/defs.h"

namespace lgraph {
class AccessControlledDB;

struct PluginInfoBase {
    std::string desc;
    bool read_only;

    virtual ~PluginInfoBase() {}

    virtual size_t Serialize(fma_common::BinaryBuffer& stream) const {
        return fma_common::BinaryWrite(stream, desc) + fma_common::BinaryWrite(stream, read_only);
    }

    virtual size_t Deserialize(fma_common::BinaryBuffer& stream) {
        size_t s, t;
        if ((s = fma_common::BinaryRead(stream, desc)) != 0) {
            if ((t = fma_common::BinaryRead(stream, read_only)) != 0) {
                return s + t;
            }
        }
        throw std::runtime_error("Failed to read plugin info from stream, bad content");
        return 0;
    }
};

class PluginManagerImplBase {
 public:
    virtual ~PluginManagerImplBase() {}

    /**
     * Loads a plugin and sets contents in pinfo accordingly. Before calling this function, the file
     * has already been written properly into file system, so the overload of this function in child
     * classes just need to set its own memory state. For example, in CppPluginManager, this
     * function just checks if name.so has the right plugin function and then stores the function
     * pointer in pinfo.
     */
    virtual void LoadPlugin(const std::string& user, const std::string& name,
                            PluginInfoBase* pinfo) = 0;

    // unload a plugin
    virtual void UnloadPlugin(const std::string& user, const std::string& name,
                              PluginInfoBase* pinfo) = 0;

    /**
     * Creates plugin information. Different PluginManager will use different PluginInfo structures.
     *
     * @return  Null if it fails, else the new plugin information.
     */
    virtual PluginInfoBase* CreatePluginInfo() = 0;

    /**
     * Gets plugin path given its name.
     *
     * @param name  The name.
     *
     * @return  The plugin path.
     */
    virtual std::string GetPluginPath(const std::string& name) = 0;

    virtual std::string GetPluginDir() = 0;

    /**
     * Gets task name
     *
     * @param name  The plugin name.
     *
     * @return  The task name.
     */
    virtual std::string GetTaskName(const std::string& name) = 0;

    /**
     * Executes the call operation.
     *
     * @param [in,out] pinfo        If non-null, the pinfo.
     * @param          request      The request.
     * @param          timeout      The timeout.
     * @param          in_process   True to in process.
     * @param [in,out] output       The output.
     *
     */
    virtual void DoCall(const std::string& user, AccessControlledDB* db_with_access_control,
                        const std::string name, const PluginInfoBase* pinfo,
                        const std::string& request, double timeout, bool in_process,
                        std::string& output) = 0;
};
}  // namespace lgraph
