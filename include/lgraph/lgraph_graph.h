/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

/**
 * This is an implementation of the TuGraph graph analytics engine. The graph analytics engine
 * is a general-purpose processing engine useful for implementing various graph analytics
 * algorithms such as PageRank, ShortestPath, etc..
 */

#pragma once

#include <assert.h>
#include <omp.h>
#include <string.h>
#include <sys/mman.h>

#include <algorithm>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>
#include <iostream>

#include "lgraph/lgraph.h"
#include "lgraph/lgraph_atomic.h"
#include "lgraph/lgraph_utils.h"

namespace lgraph_api {
namespace olap {

#define THREAD_WORKING 0
#define THREAD_STEALING 1

struct ThreadState {
    size_t curr;
    size_t end;
    int state;
};

/**
 * @brief   All the parallel tasks should be delegated through Worker to
 *          prevent a huge number of threads being populated via OpenMP.
 */
class Worker {
    bool stopping_;
    bool has_work_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::shared_ptr<std::packaged_task<void()> > task_;
    std::thread worker_;

 public:
    /**
     * @brief   Get the global (shared) worker.
     *
     * @return  A shared pointer to the global Worker instance.
     */
    static std::shared_ptr<Worker> &SharedWorker();

    Worker();

    ~Worker();

    /**
     * @brief   Send some work to the Worker instance.
     *
     * @param   work    The function describing the work.
     *
     *                  Exceptions can be thrown in the work function if necessary.
     *                  Note that Delegate cannot be nested.
     */
    void Delegate(const std::function<void()> &work);
};

/**
 * @brief   Empty is used for representing unweighted graphs.
 */
struct Empty {};

/**
 * @brief   AdjUnit<EdgeData> represents an adjacent edge with EdgeData
 *          as the weight type.
 *
 * @tparam  EdgeData    Type of the edge data.
 */
template <typename EdgeData>
struct AdjUnit {
    size_t neighbour;
    EdgeData edge_data;
} __attribute__((packed));

template <>
struct AdjUnit<Empty> {
    union {
        size_t neighbour;
        Empty edge_data;
    } __attribute__((packed));
} __attribute__((packed));

/**
 * @brief   EdgeUnit<EdgeData> represents an edge with EdgeData as the weight type.
 *
 * @tparam  EdgeData    Type of the edge data.
 */

template <typename EdgeData>
struct EdgeUnit {
    size_t src;
    size_t dst;
    EdgeData edge_data;
} __attribute__((packed));

template <>
struct EdgeUnit<Empty> {
    size_t src;
    union {
        size_t dst;
        Empty edge_data;
    } __attribute__((packed));
} __attribute__((packed));

/**
 * @brief   Default parser for unweighted edges.
 * @tparam              EdgeData    Type of the edge data.
 * @param               p           Beginning pointer of input edge.
 * @param               end         Ending pointer of input edge.
 * @param   [in,out]    e           Edge to store the result.
 * @return
 */
template <typename EdgeData>
bool parse_line_unweighted(const char *p, const char *end, EdgeUnit<EdgeData> &e) {
    const char *orig = p;
    int64_t t = 0;
    p += lgraph_api::ParseInt64(p, end, t);
    e.src = t;
    while (p != end && (*p == ' ' || *p == '\t' || *p == ',')) p++;
    p += lgraph_api::ParseInt64(p, end, t);
    e.dst = t;
    while (p != end && *p != '\n') p++;
    return true;
}

/**
 * @brief   Parser for weighted edges.
 * @tparam              EdgeData    Type of the edge data.
 * @param               p           Beginning pointer of input edge.
 * @param               end         Ending pointer of input edge.
 * @param   [in,out]    e           Edge to store the result.
 * @returns True if parsing is successful, false otherwise.
 */

template <typename EdgeData>
bool parse_line_weight(const char *p, const char *end, EdgeUnit<EdgeData> &e) {
    const char *orig = p;
    int64_t t = 0;
    p += lgraph_api::ParseInt64(p, end, t);
    e.src = t;
    while (p != end && (*p == ' ' || *p == '\t' || *p == ',')) p++;
    p += lgraph_api::ParseInt64(p, end, t);
    e.dst = t;
    double w = 0.;
    while (p != end && (*p == ' ' || *p == '\t' || *p == ',')) p++;
    p += lgraph_api::ParseDouble(p, end, w);
    e.edge_data = w;
    while (p != end && *p != '\n') p++;
    return true;
}

/**
 * @brief   Define the edge direction policy of graph
 *          The policy determines the graph symmetric and undirected feature.
 */
enum EdgeDirectionPolicy {
    /**
     * The graph is asymmetric.
     * The edges from input files are outgoing edges.
     * The reversed edges form incoming edges.
     */
    DUAL_DIRECTION,
    /**
     * The graph is symmetric but the input files are asymmetric.
     * The outgoing and incoming edges are identical.
     */
    MAKE_SYMMETRIC,
    /**
     * Both the graph and the input files are symmetric.
     * The outgoing and incoming edges are identical.
     */
    INPUT_SYMMETRIC
};

/**
 * @brief   AdjList<EdgeData> allows range-based for-loop over
 *          AdjUnit<EdgeData>.
 *
 * @tparam  EdgeData    Type of the edge data.
 */

template <typename EdgeData>
class Graph;

template <typename EdgeData>
class AdjList {
    friend class Graph<EdgeData>;
    AdjUnit<EdgeData> *begin_;
    AdjUnit<EdgeData> *end_;
    AdjList(AdjUnit<EdgeData> *begin, AdjUnit<EdgeData> *end) : begin_(begin), end_(end) {}

 public:
    AdjUnit<EdgeData> *begin() { return begin_; }
    AdjUnit<EdgeData> *end() { return end_; }
};

/**
 * @brief   ParallelVector<T> aims to mimic std::vector<T>.
 *          Note that the deletions other than clearing are not supported.
 */
template <typename T>
class ParallelVector {
    bool destroyed_;
    size_t capacity_;
    T *data_;
    size_t size_;

 public:
    /**
     * @brief   Construct a ParallelVector<T>.
     *
     * @exception   std::runtime_error  Raised when a runtime error condition
     *                                  occurs.
     *
     * @param   capacity    The capacity of the vector.
     */
    explicit ParallelVector(size_t capacity)
        : destroyed_(false), capacity_(capacity), data_(nullptr), size_(0) {
        if (capacity == 0) throw std::runtime_error("capacity cannot be 0");
#if USE_VALGRIND
        data_ = (T *)malloc(sizeof(T) * capacity_);
        if (!data_) throw std::bad_alloc();
#else
        data_ = (T *)mmap(nullptr, sizeof(T) * capacity_, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
        if (data_ == MAP_FAILED) throw std::runtime_error("memory allocation failed");
#endif
    }

    /**
     * @brief   Construct a ParallelVector<T>.
     *
     * @exception   std::runtime_error  Raised when a runtime error condition
     *                                  occurs.
     *
     * @param   capacity    The capacity of the vector.
     * @param   size        The initial size of the vector.
     */
    explicit ParallelVector(size_t capacity, size_t size)
        : destroyed_(false), capacity_(capacity), data_(nullptr), size_(size) {
        if (capacity == 0) throw std::runtime_error("capacity cannot be 0");
#if USE_VALGRIND
        data_ = (T *)malloc(sizeof(T) * capacity_);
        if (!data_) throw std::bad_alloc();
#else
        data_ = (T *)mmap(nullptr, sizeof(T) * capacity_, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
        if (data_ == MAP_FAILED) throw std::runtime_error("memory allocation failed");
#endif
        for (size_t i = 0; i < size_; i++) {
            new (data_ + i) T();
        }
    }

    /**
     * @brief   Construct a ParallelVector<T>
     *
     * @param   data    The initial data of the vector.
     * @param   size    The initial size of the vector.
     *                  And the initial capacity equals initial size.
     */
    ParallelVector(T *data, size_t size)
        : destroyed_(false), capacity_(size), size_(size), data_(data) {}

    ParallelVector(const ParallelVector<T> &rhs) = delete;

    ParallelVector(ParallelVector<T> &&rhs);

    /**
     * @brief   Default constructor of ParallelVector<T>.
     */
    ParallelVector() : destroyed_(true), capacity_(0), data_(nullptr), size_(0) {}

    /**
     * @brief   Destroy ParallelVector<T>.
     */
    void Destroy() {
        if (destroyed_) return;
        if (size_ > 0) Clear();
#if USE_VALGRIND
        free(data_);
#else
        int error = munmap(data_, sizeof(T) * capacity_);
        if (error != 0) {
            fprintf(stderr, "warning: potential memory leak!\n");
        }
#endif
        destroyed_ = true;
        capacity_ = 0;
    }

    ~ParallelVector() {
        if (!destroyed_ && data_ != nullptr && data_ != MAP_FAILED) {
            Destroy();
        }
    }

    T &operator[](size_t i) { return data_[i]; }

    T *begin() { return data_; }

    T *end() { return data_ + size_; }

    T &Back() { return data_[size_ - 1]; }

    T *Data() { return data_; }

    size_t Size() { return size_; }

    /**
     * @brief   Change ParallelVector size.
     *          Note the new size should be larger than or equal to elder size.
     *
     * @param   size    Value of new size.
     */
    void Resize(size_t size) {
        if (size < size_) {
            throw std::runtime_error("The new size is smaller than the current one.");
        }
        if (size > capacity_) {
            throw std::runtime_error("out of capacity.");
        }
        for (size_t i = size_; i < size; i++) {
            new (data_ + i) T();
        }
        size_ = size;
    }

    /**
     * @brief   Change ParallelVector size and initialize the new element with elem.
     *          Note the new size should be larger than or equal to elder size.
     *
     * @param   size    Value of new size.
     * @param   elem    Initial value of new-added element.
     */
    void Resize(size_t size, const T &elem) {
        if (size < size_) {
            throw std::runtime_error("The new size is smaller than the current one.");
        }
        if (size > capacity_) {
            throw std::runtime_error("out of capacity.");
        }
        for (size_t i = size_; i < size; i++) {
            new (data_ + i) T(elem);
        }
        size_ = size;
    }

    /**
     * @brief   Clear all data and change size to 0.
     */
    void Clear() {
        if (size_ > 0 && std::is_destructible<T>::value && !std::is_scalar<T>::value) {
            for (size_t i = 0; i < size_; i++) {
                data_[i].~T();
            }
        }
        size_ = 0;
    }

    /**
     * @brief   Destroy elder data and allocate with new capacity.
     *
     * @param   capacity    New capacity value.
     */
    void ReAlloc(size_t capacity) {
        if (capacity < capacity_) {
            throw std::runtime_error("The new capacity is smaller than the current one.");
        }
        if (capacity == 0) {
            throw std::runtime_error("Capacity cannot be 0");
        }
        if (capacity_ == 0) {
#if USE_VALGRIND
            data_ = (T*)malloc(sizeof(T) * capacity);
            if (!data_) throw std::bad_alloc();
#else
            data_ = (T*)mmap(nullptr, sizeof(T) * capacity, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
            if (data_ == MAP_FAILED) throw std::runtime_error("memory alloction failed");
#endif
        } else {
#if USE_VALGRIND
            data_ = (T*)realloc(data_, sizeof(T) * capacity);
            if (!data_) throw std::bad_alloc();
#else
            T* new_data = (T *)mmap(nullptr, sizeof(T) * capacity, PROT_READ | PROT_WRITE,
                                    MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
            if (new_data == MAP_FAILED) throw std::runtime_error("memory realloc failed");
            if (size_ != 0) {
                memcpy(new_data, data_, size_);
            }
            if (capacity_ != 0) {
                int error = munmap(data_, sizeof(T) * capacity_);
                if (error != 0) {
                    fprintf(stderr, "warning: potential memory leak!\n");
                }
            }
            data_ = new_data;
#endif
        }
        capacity_ = capacity;
        destroyed_ = false;
    }

    /**
     * @brief   Assign the vector's elements with a common value.
     *
     * @param   elem    The common value.
     *
     *          This action is performed in parallel, so you should not call
     *          it inside another parallel region (via Worker::Delegate).
     */
    void Fill(T elem) {
        for (size_t i = 0; i < size_; i++) {
            data_[i] = elem;
        }
    }

    /**
     * @brief   Append an element to the vector.
     *
     * @exception   std::runtime_error  Raised when a runtime error condition
     *                                  occurs.
     *
     * @param   elem    The element.
     * @param   atomic  (Optional) Whether atomic instructions should be used or
     *                  not.
     */
    void Append(const T &elem, bool atomic = true) {
        if (atomic) {
            size_t size = __sync_fetch_and_add(&size_, 1);
            if (size + 1 > capacity_) throw std::runtime_error("out of capacity");
            new (data_ + size) T(elem);
        } else {
            if (size_ + 1 > capacity_) throw std::runtime_error("out of capacity");
            new (data_ + size_) T(elem);
            size_ += 1;
        }
    }

    /**
     * @brief   Append an array of elements to the vector.
     *
     * @exception   std::runtime_error  Raised when a runtime error condition
     *                                  occurs.
     *
     * @param   [in,out]    buf     The array pointer.
     * @param               count   The array length.
     * @param               atomic  (Optional) True to atomic.
     */
    void Append(T *buf, size_t count, bool atomic = true) {
        if (atomic) {
            size_t size = __sync_fetch_and_add(&size_, count);
            if (size + count > capacity_) throw std::runtime_error("out of capacity");
            for (size_t i = 0; i < count; i++) {
                new (data_ + size + i) T(buf[i]);
            }
        } else {
            if (size_ + count > capacity_) throw std::runtime_error("out of capacity");
            for (size_t i = 0; i < count; i++) {
                new (data_ + size_ + i) T(buf[i]);
            }
            size_ += count;
        }
    }

    /**
     * @brief   Append another vector of elements to this.
     *
     * @exception   std::runtime_error  Raised when a runtime error condition
     *                                  occurs.
     *
     * @param   [in,out]    other   The other vector.
     * @param               atomic  (Optional) True to atomic.
     */
    void Append(ParallelVector<T> &other, bool atomic = true) {
        if (atomic) {
            size_t size = __sync_fetch_and_add(&size_, other.size_);
            if (size + other.size_ > capacity_) throw std::runtime_error("out of capacity");
            for (size_t i = 0; i < other.size_; i++) {
                new (data_ + size + i) T(other.data_[i]);
            }
        } else {
            if (size_ + other.size_ > capacity_) throw std::runtime_error("out of capacity");
            for (size_t i = 0; i < other.size_; i++) {
                new (data_ + size_ + i) T(other.data_[i]);
            }
            size_ += other.size_;
        }
    }

    /**
     * @brief   Swap the current vector with another one.
     *
     * @param   [in,out]    other   The other vector to swap with.
     */
    void Swap(ParallelVector<T> &other) {
        std::swap(destroyed_, other.destroyed_);
        std::swap(capacity_, other.capacity_);
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
    }

    /**
     * @brief   Copy the current vector.
     *
     * @return  A new vector with the same copied content.
     */
    ParallelVector<T> Copy() {
        ParallelVector<T> copy(capacity_, size_);
        for (size_t i = 0; i < size_; i++) {
            copy.data_[i] = data_[i];
        }
        return copy;
    }
};

/**
 * @brief   ParallelBitset implements the concurrent bitset data structure,
 *          which is usually used to represent active vertex sets.
 */
class ParallelBitset {
#define WORD_OFFSET(i) ((i) >> 6)
#define BIT_OFFSET(i) ((i)&0x3f)

    uint64_t *data_;
    size_t size_;

 public:
    /**
     * @brief   Construct a ParallelBitset.
     *
     * @param   size    The size of the bitset (i.e. the number of bits).
     */
    explicit ParallelBitset(size_t size);

    ParallelBitset(const ParallelBitset &rhs) = delete;

    ParallelBitset(ParallelBitset &&rhs) = default;

    ~ParallelBitset();

    /**
     * @brief Clear the bitset.
     */
    void Clear();

    /**
     * @brief Fill the bitset.
     */
    void Fill();

    /**
     * @brief   Test a specified bit.
     *
     * @param   i   The bit to test.
     *
     * @return  Whether the bit is set or not.
     */
    bool Has(size_t i);

    /**
     * @brief   Set a specified bit.
     *
     * @param   i   The bit to set.
     *
     * @return  Whether the operation is a true addition or not.
     */
    bool Add(size_t i);

    /**
     * @brief   Swap the current bitset with another one.
     *
     * @param   [in,out]    other   The other bitset to swap with.
     */
    void Swap(ParallelBitset &other);

    uint64_t *Data() { return data_; }

    size_t Size() { return size_; }
};

/**
 * @brief   The VertexLockGuard automatically acquires the lock on
 *          construction and releases the lock on destruction.
 */
class VertexLockGuard {
    volatile bool *lock_;

 public:
    explicit VertexLockGuard(volatile bool *lock);
    VertexLockGuard(const VertexLockGuard &rhs) = delete;
    VertexLockGuard(VertexLockGuard &&rhs) = default;
    ~VertexLockGuard();
};

/**
 * The default reduce function which uses the plus operator.
 */
template <typename ReducedSum>
static ReducedSum reduce_plus(ReducedSum a, ReducedSum b) {
    return a + b;
}

/**
 * @brief   The maximum number of edges. Change this value if needed.
 */
#if USE_VALGRIND
static constexpr size_t MAX_NUM_EDGES = 1ul << 22;
#else
static constexpr size_t MAX_NUM_EDGES = 1ul << 36;
#endif

/**
 * @brief   Graph
 * @tparam  EdgeData
 */

/**
 * @brief   Graph instances represent static (sub)graphs loaded from txt file.
 *          The internal organization uses compressed sparse
 *          matrix formats which are optimized for read-only accesses.
 *
 *          EdgeData is used for representing edge weights (the default type
 *          is Empty which is used for unweighted graphs).
 *
 * @tparam  EdgeData    Type of the edge data.
 */
template <typename EdgeData>
class Graph {
 protected:
    size_t num_vertices_;
    size_t num_edges_;
    size_t edge_data_size_;
    size_t adj_unit_size_;
    size_t edge_unit_size_;
    EdgeDirectionPolicy edge_direction_policy_;

    EdgeUnit<EdgeData> *edge_list_;
    ParallelVector<size_t> out_degree_;
    ParallelVector<size_t> in_degree_;
    ParallelVector<size_t> out_index_;
    ParallelVector<size_t> in_index_;
    ParallelVector<AdjUnit<EdgeData> > out_edges_;
    ParallelVector<AdjUnit<EdgeData> > in_edges_;
    ParallelVector<bool> lock_array_;

    void Construct() {
        if (num_vertices_ == 0 || num_edges_ == 0) {
            throw std::runtime_error("Construct empty graph");
        }

        lock_array_.ReAlloc(num_vertices_);
        lock_array_.Resize(num_vertices_, false);

        bool dual = true;
        if (edge_direction_policy_ == MAKE_SYMMETRIC) {
            out_edges_.ReAlloc(num_edges_ * 2);
            out_edges_.Resize(num_edges_ * 2);
        } else {
            out_edges_.ReAlloc(num_edges_);
            out_edges_.Resize(num_edges_);
        }
        out_degree_.ReAlloc(num_vertices_);
        out_degree_.Resize(num_vertices_, (size_t)0);
        out_index_.ReAlloc(num_vertices_ + 1);
        out_index_.Resize(num_vertices_ + 1, (size_t)0);

        if (edge_direction_policy_ == DUAL_DIRECTION) {
            in_edges_.ReAlloc(num_edges_);
            in_edges_.Resize(num_edges_);
            in_degree_.ReAlloc(num_vertices_);
            in_degree_.Resize(num_vertices_, (size_t)0);
            in_index_.ReAlloc(num_vertices_ + 1);
            in_index_.Resize(num_vertices_ + 1, (size_t)0);
        }

        if (edge_direction_policy_ == DUAL_DIRECTION) {
            auto worker = Worker::SharedWorker();
            worker->Delegate([&]() {
#pragma omp parallel for default(none)
              for (size_t ei = 0; ei < num_edges_; ei++) {
                  size_t src = edge_list_[ei].src;
                  size_t dst = edge_list_[ei].dst;
                  __sync_fetch_and_add(&out_degree_[src], (size_t)1);
                  __sync_fetch_and_add(&in_degree_[dst], (size_t)1);
              }

              memcpy(out_index_.Data() + 1, out_degree_.Data(), sizeof(size_t) * num_vertices_);
              out_index_[0] = 0;
              if (dual) {
                  memcpy(in_index_.Data() + 1, in_degree_.Data(), sizeof(size_t) * num_vertices_);
                  in_index_[0] = 0;
              }

              for (size_t vi = 0; vi < num_vertices_; vi++) {
                  out_index_[vi + 1] += out_index_[vi];
              }
              if (dual) {
                  for (size_t vi = 0; vi < num_vertices_; vi++) {
                      in_index_[vi + 1] += in_index_[vi];
                  }
              }

#pragma omp parallel for
              for (size_t ei = 0; ei < num_edges_; ei++) {
                  size_t src = edge_list_[ei].src;
                  size_t dst = edge_list_[ei].dst;

                  size_t pos = __sync_fetch_and_add(&out_index_[src], (size_t)1);
                  out_edges_[pos].neighbour = dst;
                  if (edge_data_size_ != 0) {
                      out_edges_[pos].edge_data = edge_list_[ei].edge_data;
                  }

                  if (dual) {
                      pos = __sync_fetch_and_add(&in_index_[dst], (size_t)1);
                      in_edges_[pos].neighbour = src;
                      if (edge_data_size_ != 0) {
                          in_edges_[pos].edge_data = edge_list_[ei].edge_data;
                      }
                  }
              }

              memmove(out_index_.Data() + 1, out_index_.Data(), sizeof(size_t) * num_vertices_);
              out_index_[0] = 0;
              if (dual) {
                  memmove(in_index_.Data() + 1, in_index_.Data(), sizeof(size_t) * num_vertices_);
                  in_index_[0] = 0;
              }
            });
        } else {
            auto worker = Worker::SharedWorker();
            worker->Delegate([&]() {
#pragma omp parallel for default(none)
              for (size_t ei = 0; ei < num_edges_; ei++) {
                  size_t src = edge_list_[ei].src;
                  size_t dst = edge_list_[ei].dst;
                  __sync_fetch_and_add(&out_degree_[src], (size_t)1);
                  if (edge_direction_policy_ == MAKE_SYMMETRIC) {
                      __sync_fetch_and_add(&out_degree_[dst], (size_t)1);
                  }
              }

              memcpy(out_index_.Data() + 1, out_degree_.Data(), sizeof(size_t) * num_vertices_);
              out_index_[0] = 0;
              for (size_t vi = 0; vi < num_vertices_; vi++) {
                  out_index_[vi + 1] += out_index_[vi];
              }

#pragma omp parallel for
              for (size_t ei = 0; ei < num_edges_; ei++) {
                  size_t src = edge_list_[ei].src;
                  size_t dst = edge_list_[ei].dst;

                  size_t pos = __sync_fetch_and_add(&out_index_[src], (size_t)1);
                  out_edges_[pos].neighbour = dst;
                  if (edge_data_size_ != 0) {
                      out_edges_[pos].edge_data = edge_list_[ei].edge_data;
                  }

                  if (edge_direction_policy_ == MAKE_SYMMETRIC) {
                      pos = __sync_fetch_and_add(&out_index_[dst], (size_t)1);
                      out_edges_[pos].neighbour = src;
                      if (edge_data_size_ != 0) {
                          out_edges_[pos].edge_data = edge_list_[ei].edge_data;
                      }
                  }
              }

              memmove(out_index_.Data() + 1, out_index_.Data(), sizeof(size_t) * num_vertices_);
              out_index_[0] = 0;

              if (edge_direction_policy_ == MAKE_SYMMETRIC) {
                  num_edges_ *= 2;
              }
            });
        }
    }

    size_t GetMaxVertexId() {
        auto worker = Worker::SharedWorker();
        size_t max_vertex_id = 0;
        worker->Delegate([&]() {
            thread_local size_t local_max_vertex_id = 0;
#pragma omp parallel
            {
                int thread_id = omp_get_thread_num();
                int num_threads = omp_get_num_threads();
                size_t thread_length = num_edges_ / num_threads;
                size_t start_index = thread_length * thread_id;
                size_t end_index =
                    (thread_id == num_threads - 1) ? num_edges_ : (start_index + thread_length);
                for (size_t ei = start_index; ei < end_index; ei++) {
                    local_max_vertex_id = std::max(local_max_vertex_id, edge_list_[ei].src);
                    local_max_vertex_id = std::max(local_max_vertex_id, edge_list_[ei].dst);
                }
            }
#pragma omp parallel
            {
#pragma omp critical
                max_vertex_id = std::max(max_vertex_id, local_max_vertex_id);
            };
        });

        return max_vertex_id;
    }

 public:
    /**
     * @brief   Constructor of Graph.
     */
    Graph() {
        num_vertices_ = 0;
        num_edges_ = 0;
        edge_data_size_ = std::is_same<EdgeData, Empty>::value ? 0 : sizeof(EdgeData);
        adj_unit_size_ = edge_data_size_ + sizeof(size_t);
        edge_unit_size_ = adj_unit_size_ + sizeof(size_t);
    }

    /**
     * @brief     Access the out-degree of some vertex.
     *
     * @param     vid The vertex id (in the Graph) to access.
     *
     * @return    The out-degree of the specified vertex in the Graph.
     */
    size_t OutDegree(size_t vid) { return out_degree_[vid]; }

    /**
     * @brief   Access the in-degree of some vertex.
     *
     * @param   vid The vertex id (in the Graph) to access.
     *
     * @return  The in-degree of the specified vertex in the Graph.
     */
    size_t InDegree(size_t vid) {
        if (edge_direction_policy_ == DUAL_DIRECTION) {
            return in_degree_[vid];
        } else {
            return out_degree_[vid];
        }
    }

    /**
     * @brief     Access the outgoing edges of some vertex.
     *
     * @param     vid The vertex id (in the Graph) to access.
     *
     * @return    The outgoing edges of the specified vertex in the Graph.
     */
    AdjList<EdgeData> OutEdges(size_t vid) {
        return AdjList<EdgeData>(out_edges_.Data() + out_index_[vid],
                                 out_edges_.Data() + out_index_[vid + 1]);
    }

    /**
     * @brief   Access the incoming edges of some vertex.
     *
     * @param   vid The vertex id (in the Graph) to access.
     *
     * @return  The incoming edges of the specified vertex in the Graph.
     */
    AdjList<EdgeData> InEdges(size_t vid) {
        if (edge_direction_policy_ == DUAL_DIRECTION) {
            return AdjList<EdgeData>(in_edges_.Data() + in_index_[vid],
                                     in_edges_.Data() + in_index_[vid + 1]);
        } else {
            return AdjList<EdgeData>(out_edges_.Data() + out_index_[vid],
                                     out_edges_.Data() + out_index_[vid + 1]);
        }
    }

    /**
     * @brief   Transpose the graph.
     */
    void Transpose() {
        if (edge_direction_policy_ != DUAL_DIRECTION) return;
        out_degree_.Swap(in_degree_);
        out_index_.Swap(in_index_);
        out_edges_.Swap(in_edges_);
    }

    /**
     * @brief   Get number of vertices of the Graph.
     *
     * @return  Number of vertices of the Graph.
     */
    size_t NumVertices() { return num_vertices_; }

    /**
     * @brief   Get number of edges of the Graph.
     *
     * @return  Number of edges of the Graph.
     */
    size_t NumEdges() { return num_edges_; }

    /**
     * @brief   Allocate a vertex array with type VertexData.
     *
     * @tparam  VertexData  Type of the vertex data.
     *
     * @return  A ParallelVector with type VertexData.
     */
    template <typename VertexData>
    ParallelVector<VertexData> AllocVertexArray() {
        return ParallelVector<VertexData>(num_vertices_, num_vertices_);
    }

    /**
     * @brief   Allocate a vertex subset represented with ParallelBitset.
     *
     * @return  A ParallelBitset sized |V| of the Graph.
     */
    ParallelBitset AllocVertexSubset() { return ParallelBitset(num_vertices_); }

    /**
     * @brief   Lock some vertex to ensure correct concurrent updates.
     *
     * @param   vid     The vertex id (in the Graph) to lock.
     */
    void AcquireVertexLock(size_t vid) {
        volatile bool *v_lock = lock_array_.Data() + vid;
        do {
            while (*v_lock) std::this_thread::yield();
        } while (__sync_lock_test_and_set(v_lock, 1));
    }

    /**
     * @brief   Unlock some vertex to ensure correct concurrent updates.
     *
     * @param   vid     The vertex id (in the Graph) to unlock.
     */
    void ReleaseVertexLock(size_t vid) {
        volatile bool *v_lock = lock_array_.Data() + vid;
        __sync_lock_release(v_lock);
    }

    /**
     * @brief   Get a VertexLockGuard of some vertex.
     *
     * @param   vid     The vertex id (in the Graph) to lock/unlock.
     *
     * @return  A VertexLockGuard corresponding to the specified vertex.
     */
    VertexLockGuard GuardVertexLock(size_t vid) {
        volatile bool *v_lock = lock_array_.Data() + vid;
        return VertexLockGuard(v_lock);
    }

    /**
     * @brief   Load edge_list from txt file and construct graph.
     *
     * @param   input_file              Input edge list file.
     * @param   edge_direction_policy   Edge direction policy of graph.
     * @param   parse_line              Parse function of input edge list file.
     *                                  Default parse function is parse_line_weighted.
     */
    void Load(std::string input_file, EdgeDirectionPolicy edge_direction_policy = DUAL_DIRECTION,
              std::function<bool(const char *, const char *, EdgeUnit<EdgeData> &)> parse_line =
                  parse_line_unweighted<EdgeData>) {
        if (num_vertices_ != 0 || num_edges_ != 0) {
            throw std::runtime_error("Graph should not be loaded twice!");
        }

        edge_direction_policy_ = edge_direction_policy;
        edge_list_ = (EdgeUnit<EdgeData> *)alloc_buffer(edge_unit_size_ * MAX_NUM_EDGES);
        auto worker = Worker::SharedWorker();
        double cost = -get_time();

        char *line_buffer = new char[1024];
        size_t line_length;
        FILE *f = fopen(input_file.c_str(), "r");

        while (getline(&line_buffer, &line_length, f) != -1) {
            if (parse_line(line_buffer, line_buffer + line_length, edge_list_[num_edges_])) {
                num_edges_++;
            }
        }

        fclose(f);
        delete[] line_buffer;

        num_vertices_ = GetMaxVertexId() + 1;
        cost += get_time();
        std::printf("  single pass read cost: %.2lf(s)\n", cost);

        Construct();
        dealloc_buffer(edge_list_, edge_unit_size_ * MAX_NUM_EDGES);
    }

    bool IfSparse(ParallelBitset &active_vertices) {
        size_t active_edges = ProcessVertexInRange<size_t>(
            [&](size_t vtx) { return (size_t)out_degree_[vtx]; }, active_vertices);
        return (active_edges < num_edges_ / 20);
    }

    /**
     * @brief   Execute a parallel-for loop in the range [lower, upper).
     *
     * @exception   std::runtime_error  Raised when a runtime error condition
     *                                  occurs.
     *
     * @tparam  ReducedSum  Type of the reduced sum.
     * @param   work    The function describing the work.
     * @param   lower   The lower bound of the range (inclusive).
     * @param   upper   The upper bound of the range (exclusive).
     * @param   zero    (Optional) The initial value for reduction.
     * @param   reduce  (Optional) The function describing the reduction logic.
     *
     * @return  A reduction value.
     */
    template <typename ReducedSum>
    ReducedSum ProcessVertexInRange(
        std::function<ReducedSum(size_t)> work, size_t lower, size_t upper, ReducedSum zero = 0,
        std::function<ReducedSum(ReducedSum, ReducedSum)> reduce = reduce_plus<ReducedSum>) {
        auto task_ctx = GetThreadContext();
        auto worker = Worker::SharedWorker();
        ReducedSum sum = zero;
        int num_threads = 0;
#pragma omp parallel
        {
            if (omp_get_thread_num() == 0) {
                num_threads = omp_get_num_threads();
            }
        };
        ThreadState **thread_state;
        thread_state = new ThreadState *[num_threads];
        for (int t_i = 0; t_i < num_threads; t_i++) {
#if USE_VALGRIND
            thread_state[t_i] = (ThreadState *)malloc(sizeof(ThreadState));
            if (!thread_state[t_i]) {
                throw std::bad_alloc();
            }
#else
            thread_state[t_i] =
                (ThreadState *)mmap(NULL, sizeof(ThreadState), PROT_READ | PROT_WRITE,
                                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if ((void *)thread_state[t_i] == MAP_FAILED) {
                throw std::runtime_error("memory allocation failed");
            }
#endif
        }
        size_t vertices = upper - lower;
        for (int t_i = 0; t_i < num_threads; t_i++) {
            thread_state[t_i]->curr = lower + vertices / num_threads / 64 * 64 * t_i;
            thread_state[t_i]->end = lower + vertices / num_threads / 64 * 64 * (t_i + 1);
            if (t_i == num_threads - 1) {
                thread_state[t_i]->end = lower + vertices;
            }
            thread_state[t_i]->state = THREAD_WORKING;
        }
        worker->Delegate([&]() {
            thread_local ReducedSum local_sum;
#pragma omp parallel
            { local_sum = zero; }
#pragma omp parallel
            {
                int thread_id = omp_get_thread_num();
                while (true) {
                    size_t start = __sync_fetch_and_add(&thread_state[thread_id]->curr, 64);
                    if (start >= thread_state[thread_id]->end) break;
                    if (ShouldKillThisTask(task_ctx)) break;
                    size_t end = start + 64;
                    if (end > thread_state[thread_id]->end) end = thread_state[thread_id]->end;
                    for (size_t i = start; i < end; i++) {
                        local_sum = reduce(local_sum, work(i));
                    }
                }
                thread_state[thread_id]->state = THREAD_STEALING;
                for (int t_offset = 1; t_offset < num_threads; t_offset++) {
                    thread_id = (thread_id + t_offset) % num_threads;
                    if (thread_state[thread_id]->state == THREAD_STEALING) continue;
                    while (true) {
                        size_t start = __sync_fetch_and_add(&thread_state[thread_id]->curr, 64);
                        if (start >= thread_state[thread_id]->end) break;
                        if (ShouldKillThisTask(task_ctx)) break;
                        size_t end = start + 64;
                        if (end > thread_state[thread_id]->end) end = thread_state[thread_id]->end;
                        for (size_t i = start; i < end; i++) {
                            local_sum = reduce(local_sum, work(i));
                        }
                    }
                }
            }
#pragma omp parallel
            {
#pragma omp critical
                sum = reduce(sum, local_sum);
            }
        });

        for (int t_i = 0; t_i < num_threads; t_i++) {
#if USE_VALGRIND
            free(thread_state[t_i]);
#else
            int error = munmap(thread_state[t_i], sizeof(ThreadState));
            if (error != 0) {
                fprintf(stderr, "warning: potential memory leak!\n");
            }
#endif
        }
        delete [] thread_state;
        if (ShouldKillThisTask(task_ctx)) throw std::runtime_error("Task killed");
        return sum;
    }

    /**
     * @brief   Process a set of active vertices in parallel.
     *
     * @exception   std::runtime_error  Raised when a runtime error condition
     *                                  occurs.
     *
     * @tparam  ReducedSum  Type of the reduced sum.
     * @param           work            The function describing each vertex's
     *                                  work.
     * @param [in,out]  active_vertices The active vertex set.
     * @param           zero            (Optional) The initial value for
     *                                  reduction.
     * @param           reduce          (Optional) The function describing the
     *                                  reduction logic.
     *
     * @return  A reduction value.
     */
    template <typename ReducedSum>
    ReducedSum ProcessVertexActive(
        std::function<ReducedSum(size_t)> work, ParallelBitset &active_vertices,
        ReducedSum zero = 0,
        std::function<ReducedSum(ReducedSum, ReducedSum)> reduce = reduce_plus<ReducedSum>) {
        auto task_ctx = GetThreadContext();
        auto worker = Worker::SharedWorker();
        size_t num_vertices = active_vertices.Size();
        ReducedSum sum = zero;

        int num_threads = 0;
#pragma omp parallel
        {
            if (omp_get_thread_num() == 0) {
                num_threads = omp_get_num_threads();
            }
        };
        ThreadState **thread_state;
        thread_state = new ThreadState *[num_threads];
        for (int t_i = 0; t_i < num_threads; t_i++) {
#if USE_VALGRIND
            thread_state[t_i] = (ThreadState *)malloc(sizeof(ThreadState));
            if (!thread_state[t_i]) {
                throw std::bad_alloc();
            }
#else
            thread_state[t_i] =
                (ThreadState *)mmap(NULL, sizeof(ThreadState), PROT_READ | PROT_WRITE,
                                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if ((void *)thread_state[t_i] == MAP_FAILED) {
                throw std::runtime_error("memory allocation failed");
            }
#endif
        }
        size_t vertices = num_vertices;
        for (int t_i = 0; t_i < num_threads; t_i++) {
            thread_state[t_i]->curr = vertices / num_threads / 64 * 64 * t_i;
            thread_state[t_i]->end = vertices / num_threads / 64 * 64 * (t_i + 1);
            if (t_i == num_threads - 1) {
                thread_state[t_i]->end = vertices;
            }
            thread_state[t_i]->state = THREAD_WORKING;
        }

        worker->Delegate([&]() {
            thread_local ReducedSum local_sum;
#pragma omp parallel
            { local_sum = zero; }
#pragma omp parallel
            {
                int thread_id = omp_get_thread_num();
                while (true) {
                    size_t vi = __sync_fetch_and_add(&thread_state[thread_id]->curr, 64);
                    if (vi >= thread_state[thread_id]->end) break;
                    if (ShouldKillThisTask(task_ctx)) break;
                    uint64_t word = active_vertices.Data()[WORD_OFFSET(vi)];
                    size_t vi_copy = vi;
                    while (word != 0) {
                        if (word & 1) {
                            local_sum = reduce(local_sum, work(vi_copy));
                        }
                        vi_copy += 1;
                        word >>= 1;
                    }
                }
                thread_state[thread_id]->state = THREAD_STEALING;
                for (int t_offset = 1; t_offset < num_threads; t_offset++) {
                    thread_id = (thread_id + t_offset) % num_threads;
                    if (thread_state[thread_id]->state == THREAD_STEALING) continue;
                    while (true) {
                        size_t vi = __sync_fetch_and_add(&thread_state[thread_id]->curr, 64);
                        if (vi >= thread_state[thread_id]->end) break;
                        if (ShouldKillThisTask(task_ctx)) break;
                        uint64_t word = active_vertices.Data()[WORD_OFFSET(vi)];
                        size_t vi_copy = vi;
                        while (word != 0) {
                            if (word & 1) {
                                local_sum = reduce(local_sum, work(vi_copy));
                            }
                            vi_copy += 1;
                            word >>= 1;
                        }
                    }
                }
            }
#pragma omp parallel
            {
#pragma omp critical
                sum = reduce(sum, local_sum);
            }
        });

        for (int t_i = 0; t_i < num_threads; t_i++) {
#if USE_VALGRIND
            free(thread_state[t_i]);
#else
            int error = munmap(thread_state[t_i], sizeof(ThreadState));
            if (error != 0) {
                fprintf(stderr, "warning: potential memory leak!\n");
            }
#endif
        }
        delete [] thread_state;
        if (ShouldKillThisTask(task_ctx)) throw std::runtime_error("Task killed");
        return sum;
    }
};

#include <sys/syscall.h>
#include <unistd.h>

#ifndef __APPLE__
template <typename T>
T ForEachVertex(GraphDB &db, Transaction &txn, std::vector<Worker> &workers,
                const std::vector<int64_t> vertices,
                std::function<void(Transaction &, VertexIterator &, T &)> work,
                std::function<void(const T &, T &)> reduce, size_t parallel_factor = 8) {
    T results;
    static thread_local size_t wid = syscall(__NR_gettid) % workers.size();
    auto& worker = workers[wid];
    worker.Delegate([&]() {
#pragma omp parallel num_threads(parallel_factor)
        {
            T local_results;
            auto txn_ = db.ForkTxn(txn);
            int tid = omp_get_thread_num();
            int num_th = omp_get_num_threads();
            size_t start = vertices.size() / num_th * tid;
            size_t end = vertices.size() / num_th * (tid + 1);
            if (tid == num_th - 1) end = vertices.size();
            auto vit = txn_.GetVertexIterator();
            for (size_t i = start; i < end; i++) {
                vit.Goto(vertices[i]);
                work(txn_, vit, local_results);
            }
#pragma omp critical
            {
                reduce(local_results, results);
            }
        }
    });
    return results;
}

template <typename T>
std::vector<T> ForEachVertex(GraphDB &db, Transaction &txn, std::vector<Worker> &workers,
                             const std::vector<int64_t> vertices,
                             std::function<T(Transaction &, VertexIterator &, size_t)> work,
                             size_t parallel_factor = 8) {
    std::vector<T> results(vertices.size());
    static thread_local size_t wid = syscall(__NR_gettid) % workers.size();
    auto& worker = workers[wid];
    worker.Delegate([&]() {
#pragma omp parallel num_threads(parallel_factor)
        {
            auto txn_ = db.ForkTxn(txn);
            int tid = omp_get_thread_num();
            int num_th = omp_get_num_threads();
            size_t start = vertices.size() / num_th * tid;
            size_t end = vertices.size() / num_th * (tid + 1);
            if (tid == num_th - 1) end = vertices.size();
            auto vit = txn_.GetVertexIterator();
            for (size_t i = start; i < end; i++) {
                vit.Goto(vertices[i]);
                results[i] = work(txn_, vit, i);
            }
        }
    });
    return results;
}
#endif

}  // namespace olap
}  // namespace lgraph_api
