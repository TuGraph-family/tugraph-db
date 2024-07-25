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

#include <stdexcept>
#include <string>

#include "fma-common/binary_buffer.h"
#include "fma-common/binary_read_write_helper.h"

#include "core/defs.h"



namespace fma_common {

template<>
inline size_t BinaryWrite(fma_common::BinaryBuffer& stream, const lgraph_api::Parameter& param) {
    return fma_common::BinaryWrite(stream, param.name) +
           fma_common::BinaryWrite(stream, param.index) +
           fma_common::BinaryWrite(stream, param.type);
}

template<>
inline size_t BinaryRead(fma_common::BinaryBuffer& stream, lgraph_api::Parameter& param) {
    size_t s, t, u;
    if ((s = fma_common::BinaryRead(stream, param.name)) != 0) {
        if ((t = fma_common::BinaryRead(stream, param.index)) != 0) {
            if ((u = fma_common::BinaryRead(stream, param.type)) != 0) {
                return s + t + u;
            }
        }
    }
    throw std::runtime_error("Failed to read parameter from stream, bad content");
}

template<>
inline size_t BinaryWrite(fma_common::BinaryBuffer& stream, const lgraph_api::SigSpec& sig_spec) {
    return fma_common::BinaryWrite(stream, sig_spec.input_list) +
           fma_common::BinaryWrite(stream, sig_spec.result_list);
}


template<>
inline size_t BinaryRead(fma_common::BinaryBuffer& stream, lgraph_api::SigSpec& sig_spec) {
    size_t s, t;
    if ((s = fma_common::BinaryRead(stream, sig_spec.input_list)) != 0) {
        if ((t = fma_common::BinaryRead(stream, sig_spec.result_list)) != 0) {
            return s + t;
        }
    }
    throw std::runtime_error("Failed to read SigSpec from stream, bad content");
}
}  // namespace fma_common

namespace lgraph {
class AccessControlledDB;

struct PluginInfoBase {
    std::string desc;
    bool read_only;
    std::string language;
    std::string version;
    /// SigSpec is an optional field.
    /// It is nullptr if the plugin doesn't have `GetSignature` function
    /// Plugins whose SigSpec is nullptr are not guaranteed to call safely
    /// in InQueryCall.
    std::unique_ptr<lgraph_api::SigSpec> sig_spec;

    virtual ~PluginInfoBase() {}

    virtual size_t Serialize(fma_common::BinaryBuffer& stream) const {
        size_t s = fma_common::BinaryWrite(stream, desc)
                   + fma_common::BinaryWrite(stream, read_only)
                   + fma_common::BinaryWrite(stream, language)
                   + fma_common::BinaryWrite(stream, version);
        if (sig_spec) {
            s += fma_common::BinaryWrite(stream, *sig_spec);
        }
        return s;
    }

    virtual size_t Deserialize(fma_common::BinaryBuffer& stream) {
        size_t s, t, u, v, w;
        if ((s = fma_common::BinaryRead(stream, desc)) != 0) {
            if ((t = fma_common::BinaryRead(stream, read_only)) != 0) {
                if ((u = fma_common::BinaryRead(stream, language)) != 0) {
                    if ((v = fma_common::BinaryRead(stream, version)) != 0) {
                        if (stream.GetSize() <= 0) {
                            return s + t + u + v;
                        }
                        auto _sig_spec = std::make_unique<lgraph_api::SigSpec>();
                        if ((w = fma_common::BinaryRead(stream, *_sig_spec)) != 0) {
                            sig_spec = std::move(_sig_spec);
                            return s + t + u + v + w;
                        }
                    }
                }
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
    virtual void DoCall(lgraph_api::Transaction* txn,
                        const std::string& user,
                        AccessControlledDB* db_with_access_control,
                        const std::string name,
                        const PluginInfoBase* pinfo,
                        const std::string& request,
                        double timeout,
                        bool in_process,
                        std::string& output) = 0;

    virtual void DoCallV2(lgraph_api::Transaction* txn,
                          const std::string& user,
                          AccessControlledDB* db_with_access_control,
                          const std::string name,
                          const PluginInfoBase* pinfo,
                          const std::string& request,
                          double timeout,
                          bool in_process,
                          Result& output) = 0;
};
}  // namespace lgraph
