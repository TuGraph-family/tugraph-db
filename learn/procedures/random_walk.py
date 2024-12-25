# cython: language_level=3, cpp_locals=True, boundscheck=False, wraparound=False, initializedcheck=False
# distutils: language = c++

import cython
from cython.cimports.olap_base import *
from cython.cimports.lgraph_db import *
from cython.cimports.libc.stdio import printf
from cython.cimports.libcpp.string import string
from cython.cimports.cpython import array
from cython.cimports.libcpp.memory import shared_ptr
from cython.parallel import parallel, prange
from cython.cimports.openmp import omp_set_dynamic, omp_get_num_threads, omp_get_thread_num
from cython.cimports.libcpp.random import random_device, mt19937, uniform_int_distribution
from cython.cimports.libc.string import memcpy
import numpy as np
import time
import lgraph_db_python

@cython.cclass
class RandomWalkSample:
    g: cython.pointer(OlapOnDB[Empty])
    db: cython.pointer(GraphDB)
    feature_num: size_t
    txn: Transaction
    sample_node: size_t[:]
    step: size_t
    node: ssize_t[:]
    num_threads: cython.int
    flag: size_t[:]
    feature: cython.float[:,:]
    label: ssize_t[:]
    label_key: string
    feature_key: string
    src_list: ssize_t[:]
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
    local_vertex_type: ssize_t[:,:]
    local_edge_type: ssize_t[:,:]
    vertex_type_string: ssize_t[:]
    edge_type_string: ssize_t[:]

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
            memcpy(cython.address(self.vertex_type_string[self.index[thread_id]]), cython.address(self.local_vertex_type[thread_id, 0]), self.local_node_num[thread_id] * cython.sizeof(ssize_t))
            for i in range(self.local_node_num[thread_id]):
                memcpy(cython.address(self.feature[self.index[thread_id] + i, 0]), cython.address(self.local_feature[thread_id, i, 0]), self.feature_num * cython.sizeof(cython.float))
            memcpy(cython.address(self.src_list[self.edge_index[thread_id]]), cython.address(self.local_src_list[thread_id, 0]), self.local_edge_num[thread_id] * cython.sizeof(ssize_t))
            memcpy(cython.address(self.dst_list[self.edge_index[thread_id]]), cython.address(self.local_dst_list[thread_id, 0]), self.local_edge_num[thread_id] * cython.sizeof(ssize_t))
            memcpy(cython.address(self.edge_type_string[self.edge_index[thread_id]]), cython.address(self.local_edge_type[thread_id, 0]), self.local_edge_num[thread_id] * cython.sizeof(ssize_t))

    @cython.cfunc
    @cython.exceptval(check=False)
    def Compute(self) -> cython.void:
        dst: size_t
        k: size_t
        i: cython.int
        l: size_t
        label_string: int64_t
        feat_string: string
        vertex_type: size_t
        edge_type: size_t
        feature_list: cython.p_float
        thread_id = cython.declare(cython.int)
        begin = cython.declare(cython.int)
        end = cython.declare(cython.int)

        dev = cython.declare(random_device)
        rng = mt19937(dev())
        with cython.nogil, parallel():
            thread_id = omp_get_thread_num()
            begin = cython.cast(int, self.sample_node.shape[0] / self.num_threads) * thread_id
            end = cython.cast(int, self.sample_node.shape[0] / self.num_threads) * (thread_id + 1)
            if thread_id == self.num_threads - 1:
                end = self.sample_node.shape[0]
            for i in range(begin, end):
                degree = cython.declare(size_t,  self.g.OutDegree(self.sample_node[i]))
                if degree == 0:
                    continue
                local_txn = self.db.ForkTxn(self.txn)
                vit = local_txn.GetVertexIterator(self.g.OriginalVid(self.sample_node[i]))
                eit = vit.GetOutEdgeIterator()
                if self.flag[self.sample_node[i]] == 0:
                    self.g.AcquireVertexLock(self.sample_node[i])
                    if self.flag[self.sample_node[i]] == 0:
                        for l in range(self.txn.GetVertexSchema(vit.GetLabel()).size()):
                            if self.txn.GetVertexSchema(vit.GetLabel())[l].name == self.feature_key:
                                feat_string = vit.GetField(self.feature_key).ToString()
                                feature_list = cython.cast(cython.p_float, feat_string.c_str())
                                label_string = vit.GetField(self.label_key).AsInt64()
                                memcpy(cython.address(self.local_feature[thread_id, self.local_node_num[thread_id], 0]), feature_list, self.feature_num * cython.sizeof(cython.float))
                                self.local_label[thread_id, self.local_node_num[thread_id]] = label_string
                        vertex_type = vit.GetLabelId()
                        self.local_vertex_type[thread_id, self.local_node_num[thread_id]] = vertex_type
                        self.local_node[thread_id, self.local_node_num[thread_id]] = self.sample_node[i]
                        self.local_node_num[thread_id] += 1
                        self.flag[self.sample_node[i]] = -1
                    self.g.ReleaseVertexLock(self.sample_node[i])
                dst = self.sample_node[i]
                for k in range(self.step):
                    dst_degree = cython.declare(size_t,  self.g.OutDegree(dst))
                    dst_edges = cython.declare(AdjList[Empty], self.g.OutEdges(dst))
                    if dst_degree == 0:
                        break
                    self.local_src_list[thread_id, self.local_edge_num[thread_id]] = dst
                    dis = uniform_int_distribution[int](0, dst_degree)
                    dst = dst_edges[dis(rng)].neighbour
                    if self.flag[dst] == 0:
                        self.g.AcquireVertexLock(dst)
                        if self.flag[dst] == 0:
                            vit.Goto(self.g.OriginalVid(dst))
                            for l in range(self.txn.GetVertexSchema(vit.GetLabel()).size()):
                                if self.txn.GetVertexSchema(vit.GetLabel())[l].name == self.feature_key:
                                    feat_string = vit.GetField(self.feature_key).ToString()
                                    feature_list = cython.cast(cython.p_float, feat_string.c_str())
                                    label_string = vit.GetField(self.label_key).AsInt64()
                                    memcpy(cython.address(self.local_feature[thread_id, self.local_node_num[thread_id], 0]), feature_list, self.feature_num * cython.sizeof(cython.float))
                                    self.local_label[thread_id, self.local_node_num[thread_id]] = label_string
                            vertex_type = vit.GetLabelId()
                            self.local_vertex_type[thread_id, self.local_node_num[thread_id]] = vertex_type
                            self.local_node[thread_id, self.local_node_num[thread_id]] = dst
                            self.local_node_num[thread_id] += 1
                            self.flag[dst] = -1
                        self.g.ReleaseVertexLock(dst)
                    while eit.IsValid():
                        if eit.GetDst() == self.g.OriginalVid(dst):
                            edge_type = eit.GetLabelId()
                            self.local_edge_type[thread_id, self.local_edge_num[thread_id]] = edge_type
                            break
                        eit.Next()
                    self.local_dst_list[thread_id, self.local_edge_num[thread_id]] = dst
                    self.local_edge_num[thread_id] += 1
                local_txn.Abort()


    @cython.cfunc
    @cython.exceptval(check=False)
    def run(self, db: cython.pointer(GraphDB), olapondb:cython.pointer(OlapOnDB[Empty]), feature_num: size_t, sample_node: list, step: size_t, NodeInfo: list, EdgeInfo: list):
        start = time.time()
        self.txn = db.CreateReadTxn()
        self.g = olapondb
        self.db = db
        self.feature_num = cython.cast(size_t, feature_num)
        self.step = cython.cast(size_t, step)
        self.sample_node = array.array('L', sample_node)
        with cython.nogil, parallel():
            self.num_threads = omp_get_num_threads()
        self.local_node_num = np.zeros((self.num_threads,), dtype=np.uintp)
        self.local_edge_num = np.zeros((self.num_threads,), dtype=np.uintp)
        self.local_feature = np.zeros((self.num_threads, olapondb[0].NumVertices(), feature_num), dtype = np.float32)
        self.local_node = np.zeros((self.num_threads, olapondb[0].NumVertices()), dtype=np.intp)
        self.local_label = np.zeros((self.num_threads, olapondb[0].NumVertices()), dtype=np.intp)
        self.local_vertex_type = np.zeros((self.num_threads, olapondb[0].NumVertices()), dtype=np.intp)
        self.local_edge_type = np.zeros((self.num_threads, olapondb[0].NumEdges()), dtype=np.intp)
        self.flag = np.zeros((olapondb[0].NumVertices(),), dtype=np.uintp)
        self.feature_key = "feature_float".encode('utf-8')
        self.label_key = "label".encode('utf-8')
        self.local_src_list = np.zeros((self.num_threads, len(sample_node) * (step)), dtype=np.intp)
        self.local_dst_list = np.zeros((self.num_threads, len(sample_node) * (step)), dtype=np.intp)
        cost = time.time()
        worker = Worker.SharedWorker()
        worker.get().DelegateCompute[RandomWalkSample](self.Compute, self)
        sample_cost = time.time()
        sample_node_num = cython.declare(ssize_t, 0)
        sample_edge_num = cython.declare(ssize_t, 0)
        for id in range(self.num_threads):
            sample_node_num += self.local_node_num[id]
            sample_edge_num += self.local_edge_num[id]
        self.feature = np.zeros((sample_node_num, feature_num), dtype=np.float32)
        self.label = np.zeros((sample_node_num,), dtype=np.intp)
        self.node = np.zeros((sample_node_num,), dtype=np.intp)
        self.index = np.zeros((self.num_threads,), dtype=np.uintp)
        self.edge_index = np.zeros((self.num_threads,), dtype=np.uintp)
        self.src_list = np.zeros((sample_edge_num,), dtype=np.intp)
        self.dst_list = np.zeros((sample_edge_num,), dtype=np.intp)
        self.vertex_type_string = np.zeros((sample_node_num,), dtype=np.intp)
        self.edge_type_string = np.zeros((sample_edge_num,), dtype=np.intp)
        self.MergeList()

        NodeInfo.append(np.asarray(self.node))
        NodeInfo.append(np.asarray(self.feature)) # embedding
        NodeInfo.append(np.asarray(self.label))
        NodeInfo.append(np.asarray(self.vertex_type_string))
        EdgeInfo.append(np.asarray(self.src_list))
        EdgeInfo.append(np.asarray(self.dst_list))
        EdgeInfo.append(np.asarray(self.edge_type_string))
        end_cost = time.time()
        # printf("prepare_cost = %lf s\n", cython.cast(cython.double, cost - start))
        # printf("sample_cost = %lf s\n", cython.cast(cython.double, sample_cost - cost))
        # printf("end_cost = %lf s\n", cython.cast(cython.double, end_cost - sample_cost))
        # printf("all_cost = %lf s\n", cython.cast(cython.double, end_cost - start))
@cython.ccall
def Process(db_: lgraph_db_python.PyGraphDB, olapondb:lgraph_db_python.PyOlapOnDB, feature_num: size_t, sample_node: list, step: size_t, NodeInfo: list, EdgeInfo: list):
    addr = cython.declare(cython.Py_ssize_t, db_.get_pointer())
    olapondb_addr = cython.declare(cython.Py_ssize_t, olapondb.get_pointer())
    a = RandomWalkSample()
    a.run(cython.cast(cython.pointer(GraphDB), addr), cython.cast(cython.pointer(OlapOnDB[Empty]), olapondb_addr), feature_num, sample_node, step, NodeInfo, EdgeInfo)
