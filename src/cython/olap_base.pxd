# distutils: language = c++

ctypedef const void* const_p_void
from lgraph_db cimport *

# from libcpp.tuple cimport tuple

cdef extern from "<tuple>" namespace "std":
    cdef cppclass tuple[T1, T2]:
        pass


cdef extern from "<functional>" namespace "std":
    cdef cppclass function[T]:
        pass

cdef extern from "../../include/lgraph/olap_base.h" namespace "lgraph_api::olap" :
    cdef enum EdgeDirectionPolicy:
        DUAL_DIRECTION
        MAKE_SYMMETRIC
        INPUT_SYMMETRIC

    cdef struct Empty:
        pass

    cdef cppclass AdjUnit[EdgeData]:
        size_t neighbour
        EdgeData edge_data

    cdef cppclass EdgeUnit[EdgeData]:
        size_t src
        size_t dst
        EdgeData edge_data

    cdef cppclass AdjList[EdgeData]:
        AdjList() nogil
        AdjList(AdjUnit[EdgeData] *begin, AdjUnit[EdgeData] *end) nogil
        AdjUnit[EdgeData]* begin () nogil
        AdjUnit[EdgeData]* end() nogil
        AdjUnit[EdgeData]& operator[](size_t i) nogil

    cdef cppclass ParallelVector[T]:
        ParallelVector(size_t capacity) nogil
        ParallelVector(size_t capacity, size_t size) nogil
        ParallelVector(T *data, size_t size) nogil
        ParallelVector() nogil
        void Destroy() nogil
        T & operator[](size_t i) nogil
        T *begin() nogil
        T *end() nogil
        T & Back() nogil
        T *Data() nogil
        size_t Size() nogil
        void Resize(size_t size) nogil
        void Resize(size_t size, const T & elem) nogil
        void Clear() nogil
        void ReAlloc(size_t capacity) nogil
        void Fill(T elem) nogil
        void Append(const T & elem) nogil
        void Append(const T & elem, bint atomic) nogil
        void Append(T *buf, size_t count) nogil
        void Append(T *buf, size_t count, bint atomic) nogil
        void Append(ParallelVector[T] & other, bint atomic) nogil
        void Append(ParallelVector[T] & other) nogil
        void Swap(ParallelVector[T]& other) nogil
        ParallelVector[T] Copy() nogil


    cdef cppclass ParallelBitset:
        ParallelBitset(size_t size) nogil
        ParallelBitset() nogil
        void Clear() nogil
        void Fill() nogil
        bint Has(size_t i) nogil
        bint Add(size_t i) nogil
        void Swap(ParallelBitset & other) nogil
        uint64_t *Data() nogil
        size_t Size() nogil

    cdef cppclass VertexLockGuard:
        VertexLockGuard(bint *lock)

    cdef ReducedSum reduce_plus[ReducedSum](ReducedSum a, ReducedSum b) nogil

    cdef cppclass OlapBase[EdgeData]:
        OlapBase() nogil
        size_t OutDegree(size_t vid) nogil
        size_t InDegree(size_t vid) nogil
        AdjList[EdgeData] OutEdges(size_t vid) nogil
        AdjList[EdgeData] InEdges(size_t vid) nogil
        void Transpose() nogil
        size_t NumVertices() nogil
        size_t NumEdges() nogil
        ParallelVector[VertexData] AllocVertexArray[VertexData]() nogil
        ParallelBitset AllocVertexSubset() nogil
        void AcquireVertexLock(size_t vid) nogil
        void ReleaseVertexLock(size_t vid) nogil
        # VertexLockGuard GuardVertexLock(size_t vid) nogil
        void LoadFromArray(char * edge_array, size_t input_vertices, size_t input_edges, int edge_direction_policy) nogil
        ReducedSum ProcessVertexActive[ReducedSum, Algorithm](ReducedSum(&work)(Algorithm, size_t) nogil, ParallelBitset& active_vertices, Algorithm algorithm) nogil
        ReducedSum ProcessVertexActive[ReducedSum, Algorithm](ReducedSum(& work)(Algorithm, size_t) nogil, ParallelBitset& active_vertices, Algorithm algorithm, ReducedSum zero) nogil
        ReducedSum ProcessVertexActive[ReducedSum, Algorithm](ReducedSum(& work)(Algorithm, size_t) nogil, ParallelBitset& active_vertices, Algorithm algorithm, ReducedSum zero, ReducedSum(*reduce)(ReducedSum, ReducedSum)) nogil
        ReducedSum ProcessVertexInRange[ReducedSum, Algorithm](ReducedSum(& work)(Algorithm, size_t) nogil, size_t lower, size_t upper, Algorithm algorithm) nogil
        ReducedSum ProcessVertexInRange[ReducedSum, Algorithm](ReducedSum(& work)(Algorithm, size_t) nogil, size_t lower, size_t upper, Algorithm algorithm, ReducedSum zero) nogil
        ReducedSum ProcessVertexInRange[ReducedSum, Algorithm](ReducedSum(& work)(Algorithm, size_t) nogil, size_t lower, size_t upper, Algorithm algorithm, ReducedSum zero, ReducedSum(*reduce)(ReducedSum, ReducedSum)) nogil


cdef extern from "../../src/olap/olap_on_disk.h" namespace "lgraph_api::olap" :
    cdef cppclass OlapOnDisk[EdgeData](OlapBase[EdgeData]) nogil:
        OlapOnDisk() nogil
        void Load(ConfigBase[EdgeData] config, int edge_direction_policy) nogil

cdef extern from "../../src/olap/olap_config.h" namespace "lgraph_api::olap" :
    tuple[size_t, bint] parse_line_weighted[EdgeData](const char* p, const char* end, EdgeUnit[EdgeData]& e) nogil
    tuple[size_t, bint] parse_line_unweighted[EdgeData](const char* p, const char* end, EdgeUnit[EdgeData]& e) nogil
    cdef cppclass ConfigBase[EdgeData] nogil:
        ConfigBase() nogil
        ConfigBase(int &argc, char** &argv) nogil
        void Print() nogil
        string input_dir
        string output_dir
        tuple[size_t, bint] parse_line(const char* p, const char* end, EdgeUnit[EdgeData]& e)

cdef extern from "../../include/lgraph/lgraph_atomic.h" namespace "lgraph_api" :
    bint cas[T](T* ptr, T oldv, T newv) nogil
    bint write_min[T](T* a, T b) nogil
    bint write_max[T](T* a, T b) nogil
    void write_add[T](T* a, T b) nogil
    void write_add(uint64_t *a, uint64_t b) nogil
    void write_add(uint32_t *a, uint32_t b) nogil
    void write_add(int64_t *a, int64_t b) nogil
    void write_add(int32_t *a, int32_t b) nogil
    void write_sub[T](T* a, T b) nogil
    void write_sub(uint64_t *a, uint64_t b) nogil
    void write_sub(uint32_t *a, uint32_t b) nogil
    void write_sub(int64_t *a, int64_t b) nogil
    void write_sub(int32_t *a, int32_t b) nogil


cdef extern from "../../include/lgraph/olap_on_db.h" namespace "lgraph_api::olap" :
    size_t SNAPSHOT_PARALLEL
    size_t SNAPSHOT_UNDIRECTED
    size_t SNAPSHOT_IDMAPPING
    size_t SNAPSHOT_OUT_DEGREE
    size_t SNAPSHOT_IN_DEGREE
    size_t SNAPSHOT_OUT_EDGES
    size_t SNAPSHOT_IN_EDGES

    bint edge_convert_default[EdgeData](OutEdgeIterator &, EdgeData&) nogil

    cdef cppclass OlapOnDB[EdgeData](OlapBase[EdgeData]) nogil:
        OlapOnDB(GraphDB & db, Transaction & txn, size_t flags, bint(*vertex_filter)(VertexIterator &), bint(*out_edge_filter)(OutEdgeIterator &, EdgeData &))
        # OlapOnDB(GraphDB & db, Transaction & txn, size_t flags, bint(&vertex_filter)(VertexIterator &), bint(&out_edge_filter)(OutEdgeIterator &, EdgeData&))
        # OlapOnDB(GraphDB & db, Transaction & txn, size_t flags, bint(VertexIterator &)vertex_filter)
        OlapOnDB(GraphDB & db, Transaction & txn, size_t flags)
        OlapOnDB(GraphDB & db, Transaction & txn)
        # OlapOnDB(OlapOnDB[EdgeData]&& rhs)
        # ParallelVector[VertexData] ExtractVertexData(void(VertexIterator &, VertexData &) extract)
