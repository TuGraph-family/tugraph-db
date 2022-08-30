/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include "fma-common/rw_lock.h"

#include "core/task_tracker.h"

namespace lgraph {
namespace _detail {
class _ShouldKillTask {
 public:
    bool operator()() const { return TaskTracker::GetInstance().ShouldKillCurrentTask(); }
};
}  // namespace _detail

typedef fma_common::InterruptableTLSRWLock<_detail::_ShouldKillTask> KillableRWLock;
typedef fma_common::InterruptableTLSAutoReadLock<KillableRWLock> AutoReadLock;
typedef fma_common::InterruptableTLSAutoWriteLock<KillableRWLock> AutoWriteLock;

#define _HoldReadLock(lock) ::lgraph::AutoReadLock _lock__(lock, ::lgraph::GetMyThreadId())
#define _ReleaseReadLock() _lock__.Unlock();
#define _HoldWriteLock(lock) ::lgraph::AutoWriteLock _lock__(lock, ::lgraph::GetMyThreadId())
}  // namespace lgraph
