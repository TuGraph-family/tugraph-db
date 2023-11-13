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

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>

#include "fma-common/file_system.h"
#include "tools/lgraph_log.h"

#include "core/data_type.h"
#include "core/lightning_graph.h"
#include "core/task_tracker.h"
#include "core/thread_id.h"
#include "lgraph/lgraph_db.h"

#include "plugin/python_plugin.h"
#include "plugin/plugin_context.h"
#include "db/galaxy.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define GetCurrentPid GetCurrentProcessId
#else
#include <sys/types.h>
#include <unistd.h>
#define GetCurrentPid getpid
#endif

namespace lgraph {
#if LGRAPH_ENABLE_PYTHON_PLUGIN
PythonPluginManagerImpl::PythonPluginManagerImpl(LightningGraph* db, const std::string& graph_name,
                                                 const std::string& dir, int max_idle_seconds,
                                                 int max_plugin_lifetime_seconds)
    : graph_name_(graph_name),
      plugin_dir_(dir),
      max_idle_seconds_(max_idle_seconds <= 0 ? std::numeric_limits<int>::max()
                                              : max_idle_seconds),
      max_plugin_lifetime_seconds_(max_plugin_lifetime_seconds) {
    fma_common::FilePath p(db->GetConfig().dir);
    db_dir_ = p.Dir();
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

PythonPluginManagerImpl::~PythonPluginManagerImpl() {
    _kill_task->Cancel();
    KillAllProcesses();
    std::lock_guard<std::mutex> l(_mtx);
    auto it = _marked_processes.begin();
    while (it != _marked_processes.end()) {
        if ((*it)->IsAlive()) {
            // kill process force
            (*it)->Kill(true);
        }
        it++;
    }
    _marked_processes.clear();
}

void PythonPluginManagerImpl::LoadPlugin(const std::string& user, const std::string& name,
                                         PluginInfoBase* pinfo) {
    std::string output;
    auto ec = CallInternal(user, "__lgraph_load_module__", name, 0, true, false, output);
    switch (ec) {
    case python_plugin::TaskOutput::ErrorCode::SUCCESS:
        break;
    case python_plugin::TaskOutput::ErrorCode::SUCCESS_WITH_SIGNATURE:
        {
            if (!pinfo->sig_spec) {
                pinfo->sig_spec = std::make_unique<lgraph_api::SigSpec>();
            }
            // output stores the serialized lgraph_api::SigSpec
            // deserialize from output
            fma_common::BinaryBuffer buffer(output.data(), output.size());
            fma_common::BinaryRead(buffer, *pinfo->sig_spec);
            // detach buffer, since ownership of underlying data belongs to output
            void* _buffer = nullptr;
            size_t _size = 0;
            buffer.DetachBuf(&_buffer, &_size);
        }
    case python_plugin::TaskOutput::ErrorCode::INTERNAL_ERR:
        throw InternalError("Unexpected error occured when loading plugin: [{}]", output);
    case python_plugin::TaskOutput::ErrorCode::INPUT_ERR:
        throw InputError(FMA_FMT("Unexpected error occured when loading plugin: [{}]", output));
    default:
        throw InternalError("Unhandled error code [{}].", ec);
    }
}

void PythonPluginManagerImpl::UnloadPlugin(const std::string& user, const std::string& name,
                                           PluginInfoBase* pinfo) {
    // TODO: currently we do not track the modules in each process, so it is possible // NOLINT
    // for a process to not have a already-existing module.
    // std::string output;
    // auto ec = CallInternal(user, "__lgraph_del_module__", name, 0, true, false, output);
    KillAllProcesses();
}

void PythonPluginManagerImpl::DoCall(lgraph_api::Transaction* txn, const std::string& user,
                                     AccessControlledDB* db_with_access_control,
                                     const std::string name, const PluginInfoBase* pinfo,
                                     const std::string& request, double timeout, bool in_process,
                                     std::string& output) {
    auto ec = CallInternal(user, name, request, timeout, in_process, pinfo->read_only, output);
    switch (ec) {
    case python_plugin::TaskOutput::ErrorCode::SUCCESS:
        break;
    case python_plugin::TaskOutput::ErrorCode::INTERNAL_ERR:
        throw InputError(FMA_FMT("Plugin failed unexpectly_1. Stderr:\n{}", output));
    case python_plugin::TaskOutput::ErrorCode::INPUT_ERR:
        throw InputError("Plugin returned false. Look in output for more information.");
    default:
        throw InternalError("Unhandled error code [{}].", ec);
    }
}

// Run by the rest handling threads. Pushes the task to Python and wait for its finish.
python_plugin::TaskOutput::ErrorCode PythonPluginManagerImpl::CallInternal(
    const std::string& user, const std::string& function, const std::string& input, double timeout,
    bool in_process, bool read_only, std::string& output) {
    // check timeout
    if (timeout <= 0) timeout = (double)3600 * 24 * 365;
    std::unique_ptr<PythonWorkerProcess> proc(nullptr);
    AutoCleanupAction rollback(nullptr);
    {
        // pick a free process, or create a new one
        std::lock_guard<std::mutex> l(_mtx);
        if (_free_processes.empty()) {
            // no free process, create a new one
            FMA_DBG() << "Creating a new Python process";
            proc.reset(new PythonWorkerProcess(db_dir_));
        } else {
            proc.swap(_free_processes.front());
            _free_processes.pop_front();
            if (!proc->IsAlive()) proc.reset(new PythonWorkerProcess(db_dir_));
        }
        PythonWorkerProcess* ptr = proc.get();
        _busy_processes.insert(ptr);
        // on failure, proc is killed and should be removed from busy processes
        rollback.Reset([ptr, this]() {
            std::lock_guard<std::mutex> l(_mtx);
            _busy_processes.erase(ptr);
        });
    }

    proc->ClearOutput();
    // write task
    python_plugin::TaskInput task_input;
    task_input.user = user;
    task_input.graph = graph_name_;
    task_input.plugin_dir = plugin_dir_;
    task_input.function = function;
    task_input.input = input;
    task_input.read_only = read_only;
    task_input.WriteToMessageQueue(proc->GetSendMQ());
    // wait for output
    python_plugin::TaskOutput task_output;
    double start_time = fma_common::GetTime();
    python_plugin::TaskOutput::ErrorCode ec;
    while (true) {
        // if master thread is killing this process
        if (TaskTracker::GetInstance().ShouldKillCurrentTask() || proc->ShouldKill()) {
            proc->Kill();
            std::lock_guard<std::mutex> l(_mtx);
            _marked_processes.insert(std::move(proc));
            throw lgraph_api::TaskKilledException();
        }
        // if process failed, break
        if (!proc->IsAlive()) {
            output = proc->Stderr();
            throw InternalError("Plugin failed unexpectly. Stderr:\n{}", output);
        }
        bool r = task_output.ReadFromMessageQueue(proc->GetRecvMQ(), 1000);
        if (r) {
            // done with the task
            ec = task_output.error_code;
            output.swap(task_output.output);
            // task finish, mark its last used time
            proc->UpdateLastUsedTime();
            break;
        }
        if (fma_common::GetTime() - start_time >= timeout) {
            // timeout, kill process
            proc->Kill();
            std::lock_guard<std::mutex> l(_mtx);
            output = proc->Stderr();
            _marked_processes.insert(std::move(proc));
            throw TimeoutException(timeout);
        }
    }
    rollback.Cancel();
    // now, check if we need to put the process back to pool
    {
        std::lock_guard<std::mutex> l(_mtx);
        _busy_processes.erase(proc.get());
        CleanUpIdleProcessesNoLock();
        if (!proc->Killed()) {
//            _free_processes.emplace_front(proc.release());
            if (proc->GetLiveTimeInSeconds() < (size_t)max_plugin_lifetime_seconds_) {
                _free_processes.emplace_front(std::move(proc));
            } else {
                DEBUG_LOG(DEBUG) << "proc lives " << proc->GetLiveTimeInSeconds()
                                 << " s and is not reused";
                proc->Kill();
                _marked_processes.insert(std::move(proc));
            }
        }
    }
    return ec;
}

void PythonPluginManagerImpl::KillAllProcesses() {
    GENERAL_LOG(DEBUG) << "Killing all python processes";
    std::lock_guard<std::mutex> l(_mtx);
    for (auto& p : _free_processes) {
        p->Kill();
        _marked_processes.insert(std::move(p));
    }
    _free_processes.clear();
    for (auto& p : _busy_processes) {
        p->MarkToKill();
    }
    _busy_processes.clear();
}

void PythonPluginManagerImpl::CleanUpIdleProcessesNoLock() {
    auto it = _free_processes.rbegin();
    while (it != _free_processes.rend() &&
           (*it)->GetIdleTimeInSeconds() >= (size_t)max_idle_seconds_) {
        (*it)->Kill();
        _marked_processes.insert(std::move(*it));
        it++;
    }
    _free_processes.erase(it.base(), _free_processes.end());
}

std::string PythonWorkerProcess::GeneratePipeName(const std::string& prefix) {
    int64_t timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return fma_common::StringFormatter::Format("_fma_pipe_{}_{}_{}_{}_", prefix, GetCurrentPid(),
                                               GetMyThreadId(), timestamp);
}
#endif
}  // namespace lgraph
