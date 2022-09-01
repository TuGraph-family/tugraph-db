/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include <atomic>
#include <mutex>

#include "fma-common/pipeline.h"
#include "fma-common/timed_task.h"
#include "fma-common/type_traits.h"
#include "fma-common/utils.h"

#include "core/thread_id.h"

#define GC_DEBUG 0

#if GC_DEBUG
#define GC_DBG_REF(o) FMA_LOG() << "Referencing " << (o);
#define GC_DBG_DEREF(o) FMA_LOG() << "Dereferencing " << (o);
#define GC_DBG_DEL(o) FMA_LOG() << "Deleting " << (o);
#else
#define GC_DBG_REF(o)
#define GC_DBG_DEREF(o)
#define GC_DBG_DEL(o)
#endif

namespace lgraph {

namespace _detail {
// Reference counted object
// NOTE: References are kept in Thread-Local-Storage, so Reference(tid) and Dereference(tid)
// must be paired with the same tid.
// Do NOT use this class directly unless you are pretty sure about what
// you are doing. Use GabageCollectedObject and ScopedRef instead.
template <typename T>
class RefCountedObj {
    // number of managers currently referencing this object
    // two GabageCollectedObject can use the same RefCountedObj when one is copy constructed
    // When GabageCollectedObject is destructed, it will try to delete this object
    // only when manager_count_==1
    std::atomic<int64_t> manager_count_;
    T* obj_;
    std::vector<fma_common::PadForCacheLine<uint64_t>> references_;

 public:
    explicit RefCountedObj(T* obj, size_t max_threads = LGRAPH_MAX_THREADS)
        : manager_count_(1), obj_(obj),
            references_(max_threads, fma_common::PadForCacheLine<uint64_t>(0)) {}

    ~RefCountedObj() {
        FMA_ASSERT(!HasReference());
        delete obj_;
    }

    void Reference(size_t tid) { references_[tid]++; }

    void Dereference(size_t tid) { references_[tid]--; }

    bool HasReference() {
        for (auto& r : references_) {
            if (r) return true;
        }
        return false;
    }

    T* Get() { return obj_; }

    // increase manager count, used only when copying a GabageCollectedObject
    void IncManagerRef() { manager_count_.fetch_add(1, std::memory_order_acq_rel); }

    int64_t DecManagerRef() { return manager_count_.fetch_sub(1, std::memory_order_acq_rel); }

    int64_t GetManagerRef() { return manager_count_.load(std::memory_order_acquire); }
};
}  // namespace _detail

// A scoped reference to a RefCountedObj
// NOTE: This can NOT be used cross-thread, it works only in a single thread.
// RefCountedObj keeps thread-local references, so passing a ScopedRef to another
// thread results in wrong references.
template <typename T>
class ScopedRef {
    _detail::RefCountedObj<T>* obj_;
    size_t tid_;

 public:
    DISABLE_COPY(ScopedRef);
    ScopedRef(_detail::RefCountedObj<T>* obj, size_t tid) : obj_(obj), tid_(tid) {
        GC_DBG_REF(obj_);
        if (obj) obj->Reference(tid_);
    }

    ScopedRef(ScopedRef&& rhs) {
        obj_ = rhs.obj_;
        rhs.obj_ = nullptr;
        tid_ = rhs.tid_;
    }

    ScopedRef& operator=(ScopedRef&& rhs) {
        if (this == &rhs) return *this;
        if (obj_) obj_->Dereference(tid_);
        obj_ = rhs.obj_;
        rhs.obj_ = nullptr;
        tid_ = rhs.tid_;
        return *this;
    }

    ~ScopedRef() {
        GC_DBG_DEREF(obj_);
        if (obj_) obj_->Dereference(tid_);
    }

    void Release() {
        if (obj_) obj_->Dereference(tid_);
        obj_ = nullptr;
    }

    T& operator*() { return *obj_->Get(); }

    T* operator->() { return obj_->Get(); }

    T* Get() const { return obj_ ? obj_->Get() : nullptr; }

    operator bool() { return obj_->Get() != nullptr; }
};

/**
 * A shared pointer that maintains a ref-counted object. Different from
 * std::shared_ptr, this class uses thread-local storage to store the
 * reference counts. As a result, referencing the object is efficient,
 * which required a thread-local write. However, checking all references
 * requires a scan over all thread-local storage, and is slow.
 * This should be used for long-living objects with few construct/destruct
 * and a lot of references.
 * Multiple GCRefCountedPtr can point to the same object when they are
 * copy-constructed. In this case the last GCRefCountedPtr is responsible
 * for destroying the object.
 */
template <typename T>
class GCRefCountedPtr {
    std::atomic<_detail::RefCountedObj<T>*> obj_;

    void DerefPtr(_detail::RefCountedObj<T>* obj,
                  const std::function<void(T*)>& before_destroy_old_obj = nullptr,
                  const std::function<void(void)>& after_destroy_old_obj = nullptr) {
        if (obj && obj->DecManagerRef() == 1)
            CollectGabage(obj, before_destroy_old_obj, after_destroy_old_obj);
    }

    _detail::RefCountedObj<T>* GetPtr() const { return obj_.load(std::memory_order_acquire); }

    void SetPtr(_detail::RefCountedObj<T>* obj) { obj_.store(obj, std::memory_order_release); }

 public:
    GCRefCountedPtr() : obj_(nullptr) {}

    explicit GCRefCountedPtr(T* obj) { SetPtr(new _detail::RefCountedObj<T>(obj)); }

    ~GCRefCountedPtr() { DerefPtr(GetPtr()); }

    GCRefCountedPtr(const GCRefCountedPtr& rhs) {
        auto obj = rhs.GetPtr();
        SetPtr(obj);
        obj->IncManagerRef();
    }

    GCRefCountedPtr& operator=(const GCRefCountedPtr& rhs) {
        if (this == &rhs) return *this;
        DerefPtr(GetPtr());
        auto obj = rhs.GetPtr();
        SetPtr(obj);
        obj->IncManagerRef();
        return *this;
    }

    GCRefCountedPtr(GCRefCountedPtr&& rhs) {
        SetPtr(rhs.GetPtr());
        rhs.SetPtr(nullptr);
    }

    GCRefCountedPtr& operator=(GCRefCountedPtr&& rhs) {
        if (this == &rhs) return *this;
        DerefPtr(GetPtr());
        SetPtr(rhs.GetPtr());
        rhs.SetPtr(nullptr);
        return *this;
    }

    /**
     * Take ownership of the object pointed to by ptr, and manage it as a ref-counted object.
     * before_destroy_old_obj: function to call when the existing object pointed to by this
     * GCRefCountedPtr has zero reference and is about to be destroyed. The function takes
     * the pointer of the old object as a parameter.
     * after_destroy_old_obj: function to call when existing object has been deleted.
     * Both callbacks are called ONLY when existing obj is not nullptr.
     */
    void Assign(T* ptr, const std::function<void(T*)>& before_destroy_old_obj = nullptr,
                const std::function<void(void)>& after_destroy_old_obj = nullptr) {
        DerefPtr(GetPtr(), before_destroy_old_obj, after_destroy_old_obj);
        SetPtr(new _detail::RefCountedObj<T>(ptr));
    }

    ScopedRef<T> GetScopedRef() const { return ScopedRef<T>(GetPtr(), GetMyThreadId()); }

    template <typename... DT>
    void EmplaceNewVersion(DT&&... ts) {
        auto old_obj = GetPtr();
        SetPtr(new _detail::RefCountedObj<T>(new T(std::forward<DT...>(ts...))));
        DerefPtr(old_obj);
    }

    operator bool() const { return GetPtr() != nullptr; }

 private:
    void CollectGabage(_detail::RefCountedObj<T>* obj,
                       const std::function<void(T*)>& before_destroy = nullptr,
                       const std::function<void(void)>& after_destroy = nullptr) {
        if (!obj) return;
        if (!obj->HasReference()) {
            FMA_DBG_ASSERT(obj->GetManagerRef() == 0);
            if (before_destroy) before_destroy(obj->Get());
            delete obj;
            if (after_destroy) after_destroy();
            return;
        }
        // start a background task to gc the obj
        auto& scheduler = fma_common::TimedTaskScheduler::GetInstance();
        scheduler.ScheduleReccurringTask(
            1000, [this, obj, before_destroy, after_destroy](fma_common::TimedTask* self) {
                if (!obj) {
                    self->Cancel();
                    return;
                }
                // check if all ref counts have become zero
                if (!obj->HasReference()) {
                    FMA_DBG_ASSERT(obj->GetManagerRef() == 0);
                    if (before_destroy) before_destroy(obj->Get());
                    delete obj;
                    if (after_destroy) after_destroy();
                    self->Cancel();
                }
            });
    }
};
}  // namespace lgraph
