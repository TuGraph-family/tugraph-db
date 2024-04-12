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
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "fma-common/bounded_queue.h"
#include "tiny-process-library/process.hpp"

#include "core/defs.h"
#include "core/kv_store.h"

#include "plugin/plugin_manager_impl.h"
#include "plugin/plugin_context.h"

namespace lgraph {
#if LGRAPH_ENABLE_PYTHON_PLUGIN
class PythonWorkerProcess {
    std::string p2c_name_;
    std::string c2p_name_;
    std::unique_ptr<boost::interprocess::message_queue> p2c_;
    std::unique_ptr<boost::interprocess::message_queue> c2p_;
    std::unique_ptr<TinyProcessLib::Process> process_;
    mutable std::mutex err_lock_;
    std::string err_;

    std::atomic<bool> should_kill_;
    std::atomic<bool> killed_;
    int exit_code_ = 0;

    std::chrono::steady_clock::time_point last_used_;
    std::chrono::steady_clock::time_point started_at_;

    void PrintMessageToLog(const char* bytes, size_t n) {
        std::lock_guard<std::mutex> _l(err_lock_);
        LOG_WARN() << "[python plugin] " << std::string(bytes, bytes + n);
        err_.append(bytes, n);
    }

    void PrintMessageToWarn(const char* bytes, size_t n) {
        std::lock_guard<std::mutex> _l(err_lock_);
        LOG_WARN() << "[python plugin] " << std::string(bytes, bytes + n);
        err_.append(bytes, n);
    }

    static std::string GeneratePipeName(const std::string& prefix);

    static std::string GenerateCommand(const std::string& p2c, const std::string& c2p,
                                       const std::string& db_dir) {
        return fma_common::StringFormatter::Format(
#ifdef _WIN32
            "python {}/lgraph_task_runner.py {} {} {}",
#else
            "python3 {}/lgraph_task_runner.py {} {} {}",
#endif
            fma_common::FileSystem::GetExecutablePath().Dir(), p2c, c2p, db_dir);
    }

    DISABLE_COPY(PythonWorkerProcess);
    DISABLE_MOVE(PythonWorkerProcess);

 public:
    explicit PythonWorkerProcess(const std::string& db_dir)
        : should_kill_(false), killed_(false),
          last_used_(std::chrono::steady_clock::now()),
          started_at_(std::chrono::steady_clock::now()) {
        p2c_name_ = GeneratePipeName("p2c");
        c2p_name_ = GeneratePipeName("c2p");
        boost::interprocess::message_queue::remove(p2c_name_.c_str());
        p2c_.reset(new boost::interprocess::message_queue(
            boost::interprocess::create_only, p2c_name_.c_str(), 10,
            python_plugin::MessageQueueUtils::MAX_MSG_SIZE()));
        boost::interprocess::message_queue::remove(c2p_name_.c_str());
        c2p_.reset(new boost::interprocess::message_queue(
            boost::interprocess::create_only, c2p_name_.c_str(), 10,
            python_plugin::MessageQueueUtils::MAX_MSG_SIZE()));
        process_.reset(new TinyProcessLib::Process(
            GenerateCommand(p2c_name_, c2p_name_, db_dir), "./",
            [this](const char* b, size_t n) { PrintMessageToLog(b, n); },
            [this](const char* b, size_t n) { PrintMessageToWarn(b, n); }));
        if (process_->get_id() == 0) {
            throw std::runtime_error(fma_common::StringFormatter::Format(
                "Error spawning Python process: {}", process_->get_exit_status()));
        }
    }

    ~PythonWorkerProcess() {
        Kill();
        process_.reset();
        boost::interprocess::message_queue::remove(p2c_name_.c_str());
        boost::interprocess::message_queue::remove(c2p_name_.c_str());
    }

    bool Killed() const { return killed_.load(std::memory_order_acquire); }

    void MarkToKill() { should_kill_.store(true, std::memory_order_release); }

    bool ShouldKill() const { return should_kill_.load(std::memory_order_acquire); }

    //====================
    // The following three methods all try to modify the process handle.
    // So only one successful invocation is allowed. Otherwise, later
    // invocation results in undefined behavior.
    void Kill(bool force = false) {
        if (killed_) return;
        if (force) {
            process_->kill(false);
            while (!process_->try_get_exit_status(exit_code_)) {
                fma_common::SleepS(0.1);
                process_->kill(true);
            }
        } else {
            process_->kill(false);
        }
    }

    int Wait() {
        if (killed_) return exit_code_;
        killed_ = true;
        return process_->get_exit_status();
    }

    bool IsAlive() {
        if (killed_) return false;
        bool r = !process_->try_get_exit_status(exit_code_);
        if (!r) killed_ = true;
        return r;
    }
    //====================

    void ClearOutput() {
        std::lock_guard<std::mutex> _l(err_lock_);
        err_.clear();
    }

    std::string Stderr() const {
        std::lock_guard<std::mutex> _l(err_lock_);
        return err_;
    }

    int GetExitCode() const { return exit_code_; }

    boost::interprocess::message_queue& GetSendMQ() { return *p2c_; }

    boost::interprocess::message_queue& GetRecvMQ() { return *c2p_; }

    size_t GetIdleTimeInSeconds() const {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() -
                                                                last_used_)
            .count();
    }

    size_t GetLiveTimeInSeconds() const {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() -
                                                                started_at_)
            .count();
    }

    void UpdateLastUsedTime() { last_used_ = std::chrono::steady_clock::now(); }
};

class PythonPluginManagerImpl : public PluginManagerImplBase {
 protected:
    std::string graph_name_;
    std::string db_dir_;
    std::string plugin_dir_;

    int max_idle_seconds_ = 600;
    int max_plugin_lifetime_seconds_ = LGRAPH_PYTHON_PLUGIN_LIFETIME_S;
    std::mutex _mtx;
    // free processes are owned by plugin manager
    std::list<std::unique_ptr<PythonWorkerProcess>> _free_processes;
    // busy processes are owned by the threads that are running tasks
    // A busy process can only be deleted by the worker thread
    // that is using the process, when the process is killed.
    // The manager may kill the process, but it cannot delete it.
    // Before deleting the process, worker thread is required to remove
    // the prointer from busy_processes from this set first.
    std::unordered_set<PythonWorkerProcess*> _busy_processes;
    fma_common::TimedTaskScheduler::TaskPtr _kill_task;
    // marked processes are owned by plugin manager.
    // After trying to kill a process, worker thread need to move it
    // into this queue.
    std::unordered_set<std::unique_ptr<PythonWorkerProcess>> _marked_processes;

    // just for test
    PythonPluginManagerImpl(const std::string& name, const std::string& db_dir, size_t db_size,
                            const std::string& plugin_dir, int max_idle_seconds = 600,
                            int max_plugin_lifetime_seconds = LGRAPH_PYTHON_PLUGIN_LIFETIME_S)
        : graph_name_(name),
          db_dir_(db_dir),
          plugin_dir_(plugin_dir),
          max_idle_seconds_(max_idle_seconds),
          max_plugin_lifetime_seconds_(max_plugin_lifetime_seconds) {
        auto& scheduler = fma_common::TimedTaskScheduler::GetInstance();
        _kill_task = scheduler.ScheduleReccurringTask(
            max_idle_seconds * 1000, [this](fma_common::TimedTask*) {
                std::lock_guard<std::mutex> l(_mtx);
                CleanUpIdleProcessesNoLock();
                auto it = _marked_processes.begin();
                while (it != _marked_processes.end()) {
                    if ((*it)->IsAlive()) {
                        (*it)->Kill();
                        it++;
                    } else {
                        it = _marked_processes.erase(it);
                    }
                }
            });
    }

 public:
    PythonPluginManagerImpl(LightningGraph* db, const std::string& graph_name,
                            const std::string& plugin_dir, int max_idle_seconds = 600,
                            int max_plugin_lifetime_seconds = LGRAPH_PYTHON_PLUGIN_LIFETIME_S);

    ~PythonPluginManagerImpl();

    /**
     * Loads a plugin and sets contents in pinfo accordingly.
     *
     * @param          name     The name.
     * @param [in,out] pinfo    If non-null, the pinfo.
     *
     * @return  The plugin.
     */
    void LoadPlugin(const std::string& user, const std::string& name,
                    PluginInfoBase* pinfo) override;

    /**
     * Unload plugin and set pinfo if necessary.
     *
     * @param          name     The name.
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
    PluginInfoBase* CreatePluginInfo() override { return new PluginInfoBase(); };

    /**
     * Gets plugin path given its name.
     *
     * @param name  The name.
     *
     * @return  The plugin path.
     */
    std::string GetPluginPath(const std::string& name) override {
        return fma_common::file_system::JoinPath(plugin_dir_, name + ".so");
    }

    std::string GetPluginDir() override { return plugin_dir_; }

    std::string GetTaskName(const std::string& name) override { return "[PYTHON_PLUGIN] " + name; }

    /**
     * Executes the call operation.
     *
     * @param          name         The name.
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
                const std::string name,
                const PluginInfoBase* pinfo,
                const std::string& request,
                double timeout,
                bool in_process,
                std::string& output) override;

    void DoCallV2(lgraph_api::Transaction* txn,
                  const std::string& user,
                  AccessControlledDB* db_with_access_control,
                  const std::string name,
                  const PluginInfoBase* pinfo,
                  const std::string& request,
                  double timeout,
                  bool in_process,
                  Result& output) override;

 protected:
    python_plugin::TaskOutput::ErrorCode CallInternal(const std::string& user,
                                                      const std::string& function,
                                                      const std::string& input, double timeout,
                                                      bool in_process, bool read_only,
                                                      std::string& output);

    void KillAllProcesses();

    // clean up processes that are idle for a long time
    void CleanUpIdleProcessesNoLock();
};
#else
class PythonPluginManagerImpl : public PluginManagerImplBase {
 public:
    PythonPluginManagerImpl(LightningGraph* db, const std::string& graph_name,
                            const std::string& plugin_dir, int max_idle) {}

    PythonPluginManagerImpl(const std::string& graph_name, const std::string& db_dir,
                            size_t db_size, const std::string& plugin_dir, int max_idle) {}

    ~PythonPluginManagerImpl() {}

    virtual void LoadPlugin(const std::string& user, const std::string& name,
                            PluginInfoBase* pinfo, std::string& error) {
        throw std::runtime_error("Python plugin is disabled in this version.");
    }

    virtual void UnloadPlugin(const std::string& user, const std::string& name,
                              PluginInfoBase* pinfo, std::string& error) {
        throw std::runtime_error("Python plugin is disabled in this version.");
    }

    virtual PluginInfoBase* CreatePluginInfo() {
        throw std::runtime_error("Python plugin is disabled in this version.");
    }

    virtual std::string GetPluginPath(const std::string& name) {
        throw std::runtime_error("Python plugin is disabled in this version.");
    }

    std::string GetTaskName(const std::string& name) override { return "[PYTHON_PLUGIN] " + name; }

    virtual void DoCall(const std::string& user, AccessControlledDB* db_with_access_control,
                        const std::string name, PluginInfoBase* pinfo, const std::string& request,
                        double timeout, bool in_process, std::string& output) {
        throw std::runtime_error("Python plugin is disabled in this version.");
    }
};
#endif
}  // namespace lgraph
