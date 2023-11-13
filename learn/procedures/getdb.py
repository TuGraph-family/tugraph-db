# cython: language_level=3, cpp_locals=True, boundscheck=False, wraparound=False, initializedcheck=False
# distutils: language = c++

import cython
from cython.cimports.olap_base import *
from cython.cimports.lgraph_db import *
from cython.cimports.libc.stdio import printf
from cython.cimports.libcpp.string import string
from cython.cimports.cpython import array
from cython.parallel import parallel, prange
from cython.cimports.openmp import omp_set_dynamic, omp_get_num_threads, omp_get_thread_num
from cython.cimports.libc.string import memcpy
import numpy as np
import time
import lgraph_db_python

@cython.cclass
class AllDb:
    g: cython.pointer(OlapOnDB[Empty])
    active: ParallelBitset
    db: cython.pointer(GraphDB)
    feature_num: size_t
    txn: Transaction
    node: ssize_t[:]
    num_threads: cython.int
    flag: size_t[:]
    feature: cython.float[:,:]
    label: ssize_t[:]
    label_key: string
    feature_key: string
    src_list: ssize_t[:]
    s_key: string
    dst_list: ssize_t[:]
    local_node_num: size_t[:]
    local_edge_num: size_t[:]
    local_feature: cython.float[:,:,:]
    local_node: ssize_t[:,:]
    local_label: ssize_t[:,:]
    index:size_t[:]
    edge_index: size_t[:]
    local_src_list: ssize_t[:,:]
    local_dst_list: ssize_t[:,:]

    @cython.cfunc
    @cython.exceptval(check=False)
    def MergeList(self) ->cython.void:
        i: size_t
        k: cython.int
        thread_id= cython.declare(cython.int)
        with cython.nogil, parallel():
            thread_id = omp_get_thread_num()
            for k in range(thread_id):
                self.index[thread_id] += self.local_node_num[k]
                self.edge_index[thread_id] += self.local_edge_num[k]
            memcpy(cython.address(self.node[self.index[thread_id]]), cython.address(self.local_node[thread_id, 0]), self.local_node_num[thread_id] * cython.sizeof(ssize_t))
            memcpy(cython.address(self.label[self.index[thread_id]]), cython.address(self.local_label[thread_id, 0]), self.local_node_num[thread_id] * cython.sizeof(ssize_t))
            for i in range(self.local_node_num[thread_id]):
                memcpy(cython.address(self.feature[self.index[thread_id] + i, 0]), cython.address(self.local_feature[thread_id, i, 0]), self.feature_num * cython.sizeof(cython.float))
            memcpy(cython.address(self.src_list[self.edge_index[thread_id]]), cython.address(self.local_src_list[thread_id, 0]), self.local_edge_num[thread_id] * cython.sizeof(ssize_t))
            memcpy(cython.address(self.dst_list[self.edge_index[thread_id]]), cython.address(self.local_dst_list[thread_id, 0]), self.local_edge_num[thread_id] * cython.sizeof(ssize_t))

    @cython.cfunc
    @cython.nogil
    @cython.exceptval(check=False)
    def Work(self, vi: size_t) -> size_t:
        i: size_t
        thread_id= cython.declare(cython.int)

        thread_id = omp_get_thread_num()
        local_txn = self.db.ForkTxn(self.txn)
        degree = cython.declare(size_t, self.g.OutDegree(vi))
        vit = local_txn.GetVertexIterator()
        vit.Goto(vi)
        feat_string = vit.GetField(self.feature_key).ToString()
        feature_list = cython.cast(cython.p_float, feat_string.c_str())
        label_string = vit.GetField(self.label_key).AsInt32()
        memcpy(cython.address(self.local_feature[thread_id, self.local_node_num[thread_id], 0]), feature_list, self.feature_num * cython.sizeof(cython.float))
        self.local_label[thread_id, self.local_node_num[thread_id]] = label_string
        self.local_node[thread_id, self.local_node_num[thread_id]] = vi
        self.local_node_num[thread_id] += 1
        out_edges = cython.declare(AdjList[Empty], self.g.OutEdges(vi))
        for i in range(degree):
            self.local_src_list[thread_id, self.local_edge_num[thread_id]] = vi
            self.local_dst_list[thread_id, self.local_edge_num[thread_id]] = out_edges[i].neighbour
            self.local_edge_num[thread_id] += 1
        local_txn.Abort()
        return 0

    @cython.cfunc
    @cython.exceptval(check=False)
    def run(self, db: cython.pointer(GraphDB), olapondb:cython.pointer(OlapOnDB[Empty]), feature_num: size_t, NodeInfo: list, EdgeInfo: list):
        start = time.time()
        self.txn = db.CreateReadTxn()
        self.g = olapondb
        self.db = db
        self.feature_num = cython.cast(size_t, feature_num)
        self.flag = np.zeros((olapondb[0].NumVertices(),), dtype=np.uintp)
        with cython.nogil, parallel():
            self.num_threads = omp_get_num_threads()
        self.local_node_num = np.zeros((self.num_threads,), dtype=np.uintp)
        self.local_edge_num = np.zeros((self.num_threads,), dtype=np.uintp)
        self.local_feature = np.zeros((self.num_threads, olapondb[0].NumVertices(), self.feature_num), dtype = np.float32)
        self.local_node = np.zeros((self.num_threads, olapondb[0].NumVertices()), dtype=np.intp)
        self.local_label = np.zeros((self.num_threads, olapondb[0].NumVertices()), dtype=np.intp)
        self.feature_key = "feature_float".encode('utf-8')
        self.label_key = "label".encode('utf-8')
        self.local_src_list = np.zeros((self.num_threads, olapondb[0].NumEdges()), dtype=np.intp)
        self.local_dst_list = np.zeros((self.num_threads, olapondb[0].NumEdges()), dtype=np.intp)
        self.active = self.g.AllocVertexSubset()
        self.active.Fill()
        cost = time.time()
        self.g.ProcessVertexActive[size_t, AllDb](self.Work, self.active, self)
        sample_cost = time.time()
        sample_node_num = 0
        sample_edge_num = 0
        for id in range(self.num_threads):
            sample_node_num += self.local_node_num[id]
            sample_edge_num += self.local_edge_num[id]
        self.feature = np.zeros((sample_node_num, self.feature_num), dtype=np.float32)
        self.label = np.zeros((sample_node_num,), dtype=np.intp)
        self.node = np.zeros((sample_node_num,), dtype=np.intp)
        self.index = np.zeros((self.num_threads,), dtype=np.uintp)
        self.edge_index = np.zeros((self.num_threads,), dtype=np.uintp)
        self.src_list = np.zeros((sample_edge_num,), dtype=np.intp)
        self.dst_list = np.zeros((sample_edge_num,), dtype=np.intp)
        self.MergeList()

        NodeInfo.append(np.asarray(self.node))
        NodeInfo.append(np.asarray(self.feature))
        NodeInfo.append(np.asarray(self.label))
        EdgeInfo.append(np.asarray(self.src_list))
        EdgeInfo.append(np.asarray(self.dst_list))
        end_cost = time.time()
        # printf("prepare_cost = %lf s\n", cython.cast(cython.double, cost - start))
        # printf("sample_cost = %lf s\n", cython.cast(cython.double, sample_cost - cost))
        # printf("end_cost = %lf s\n", cython.cast(cython.double, end_cost - sample_cost))
        # printf("all_cost = %lf s\n", cython.cast(cython.double, end_cost - start))

@cython.ccall
def Process(db_: lgraph_db_python.PyGraphDB, olapondb:lgraph_db_python.PyOlapOnDB, feature_num: size_t, NodeInfo: list, EdgeInfo: list):
    db_addr = cython.declare(cython.Py_ssize_t, db_.get_pointer())
    olapondb_addr = cython.declare(cython.Py_ssize_t, olapondb.get_pointer())
    a = AllDb()
    a.run(cython.cast(cython.pointer(GraphDB), db_addr), cython.cast(cython.pointer(OlapOnDB[Empty]), olapondb_addr), feature_num, NodeInfo, EdgeInfo)
