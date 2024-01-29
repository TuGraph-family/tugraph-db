/**
 * Copyright 2024 AntGroup CO., Ltd.
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

#include "fma-common/rw_lock.h"

#include "core/task_tracker.h"

namespace lgraph {
namespace _detail {
class _ShouldKillTask {
 public:
    bool operator()() const { return TaskTracker::GetInstance().ShouldKillCurrentTask(); }

    operator bool() const { return true; }
};
}  // namespace _detail

typedef fma_common::InterruptableTLSRWLock<_detail::_ShouldKillTask> KillableRWLock;
typedef fma_common::TLSAutoReadLock<KillableRWLock> AutoReadLock;
typedef fma_common::TLSAutoWriteLock<KillableRWLock> AutoWriteLock;

#define _HoldReadLock(lock) ::lgraph::AutoReadLock _lock__(lock, ::lgraph::GetMyThreadId())
#define _ReleaseReadLock() _lock__.Unlock();
#define _HoldWriteLock(lock) ::lgraph::AutoWriteLock _lock__(lock, ::lgraph::GetMyThreadId())
}  // namespace lgraph
