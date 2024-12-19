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
from typing import Generic, TypeVar, Dict, Optional
import numpy as np
import time
import lgraph_db_python
import threading


@cython.cclass
class Node2VecSample:
    ############ Gobal val ###################
    txn: Transaction
    num_threads: cython.int
    _map = {}
    _lock = threading.Lock()

    ############ Input Args ###################
    g: cython.pointer(OlapOnDB[Empty])
    db: cython.pointer(GraphDB)
    dimensions: size_t # dimensions of result vec
    p: size_t
    q: size_t
    sample_node: size_t[:]
    ############ Result Field ###################
    node: ssize_t[:]
    feature: cython.float[:,:]
    label: ssize_t[:]
    vertex_type_string: ssize_t[:]

    @cython.cfunc
    @cython.exceptval(check=False)
    def Compute(self) -> cython.void:
        # 1. Prepare
        path_list : cython.list[str] = []
        path_list.append("0")



    @cython.cfunc
    @cython.exceptval(check=False)
    def run(self, db: cython.pointer(GraphDB), olapondb:cython.pointer(OlapOnDB[Empty]), dimensions: size_t, p: size_t, q: size_t, sample_node: list, NodeInfo: list, EdgeInfo: list):
        self.txn = db.CreateReadTxn()
        with cython.nogil, parallel():
            self.num_threads = omp_get_num_threads()

        self.g = olapondb
        self.db = db
        self.dimensions = cython.cast(size_t, dimensions)
        self.p = cython.cast(size_t, p)
        self.q = cython.cast(size_t, q)
        self.sample_node = array.array('L', sample_node)

        self.Compute()

        sample_node_num = cython.declare(ssize_t, 0)
        sample_edge_num = cython.declare(ssize_t, 0)
        self.node = np.zeros((sample_node_num,), dtype=np.intp)
        self.feature = np.zeros((sample_node_num, dimensions), dtype=np.float32)
        self.label = np.zeros((sample_node_num,), dtype=np.intp)
        self.vertex_type_string = np.zeros((sample_node_num,), dtype=np.intp)
        NodeInfo.append(np.asarray(self.node))
        NodeInfo.append(np.asarray(self.feature))
        NodeInfo.append(np.asarray(self.label))
        NodeInfo.append(np.asarray(self.vertex_type_string))
        EdgeInfo.append(np.zeros((sample_edge_num,), dtype=np.intp)) #src_list
        EdgeInfo.append(np.zeros((sample_edge_num,), dtype=np.intp)) #dst_list
        EdgeInfo.append(np.zeros((sample_edge_num,), dtype=np.intp)) #edge_type_string

@cython.ccall
def Process(db_: lgraph_db_python.PyGraphDB, olapondb:lgraph_db_python.PyOlapOnDB, feature_num: size_t, p: size_t, q: size_t, sample_node: list, NodeInfo: list, EdgeInfo: list):
    addr = cython.declare(cython.Py_ssize_t, db_.get_pointer())
    olapondb_addr = cython.declare(cython.Py_ssize_t, olapondb.get_pointer())
    a = Node2VecSample()
    a.run(cython.cast(cython.pointer(GraphDB), addr), cython.cast(cython.pointer(OlapOnDB[Empty]), olapondb_addr), feature_num, p, q, sample_node, NodeInfo, EdgeInfo)
