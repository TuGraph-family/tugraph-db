# cython: language_level=3, cpp_locals=True, boundscheck=False, wraparound=False, initializedcheck=False
# distutils: language = c++

import cython
from cython.cimports.olap_base import *
from cython.cimports.lgraph_db import *
from cython.parallel import parallel
from cython.cimports.openmp import omp_get_num_threads
from cython.cimports.libcpp.random import random_device, mt19937, uniform_real_distribution
from cython.cimports.cpython import array
import numpy as np
import lgraph_db_python
from cython.cimports.libc.stdio import printf

@cython.cclass
class AliasTable:

    def __init__(self):
        self.size = 0
        self.accept = None
        self.alias = None
    
    def __reduce__(self):
        return (self.__class__, (self.size), (list(self.accept), list(self.alias)))

    def __setstate__(self, state):
        self.size = state[0]
        self.accept = state[2][0]
        self.alias = state[2][1]
    
    @cython.cfunc
    @cython.exceptval(check=False)
    def Init(self, area_ration:list) -> cython.void:
        self.size = len(area_ration)
        tmp = []
        small = []
        large = []

        for i in range(self.size):
            tmp.append(area_ration[i] * self.size)
            if area_ration[i] < 1:
                small.append(i)
            else:
                large.append(i)
        
        while small and large:
            less = small.pop()
            more = large.pop()
            self.accept[less] = tmp[less]
            self.alias[less] = more
            tmp[more] = tmp[more] - 1 + tmp[less]
            if tmp[more] < 1:
                small.append(more)
            else:
                large.append(more)
        
        while large:
            self.accept[large.pop()] = 1
        
        while small:
            self.accept[small.pop()] = 1
    
    @cython.cfunc
    @cython.exceptval(check=False)
    def Sample(self) -> cython.int:
        dev = cython.declare(random_device)
        rng = mt19937(dev())
        dis = uniform_real_distribution[cython.double](0, 1.0)

        rand1 = dis(rng)
        rand2 = dis(rng)
        index = cython.cast(cython.int, rand2 * self.size)
        return index if rand1 < self.accept[index] else self.alias[index]

@cython.cclass
class Node2VecSample:
    ########### Global Variables ###########
    txn: Transaction
    num_threads: cython.int
    alias_node: dict = {}
    alias_edge: dict = {}

    ########### Input Parameters ###########
    g: cython.pointer(OlapOnDB[cython.double])
    db: cython.pointer(GraphDB)
    dimensions: size_t
    p: size_t
    q: size_t
    walk_length: size_t
    sample_node: size_t[:]

    ########### Result ###########
    node: ssize_t[:]
    feature: cython.float[:,:]
    label: ssize_t[:]
    vertex_type_string: ssize_t[:]

    @cython.cfunc
    @cython.exceptval(check=False)
    def GetAliasNodes(self, node:int) ->AliasTable:
        if node not in self.alias_node:
            # calculate the alias table
            alias_table = AliasTable()
            unnormed_probs = []
            degree = cython.declare(size_t, self.g.OutDegree(node))
            out_edges = cython.declare(AdjList[cython.double], self.g.OutEdges(node))
            for i in range(degree):
                weight = out_edges[i].edge_data
                unnormed_probs.append(weight)
            
            norm_const = sum(unnormed_probs)
            norm_probs = [float(prob) / norm_const for prob in unnormed_probs]
            alias_table.Init(norm_probs)

            self.alias_node[node] = alias_table
        return self.alias_node[node]
    
    @cython.cfunc
    @cython.exceptval(check=False)
    def GetAliasEdges(self, t:int, v:int) ->AliasTable:
        if (t, v) not in self.alias_edge:
            # calculate the alias table
            alias_table = AliasTable()
            unnormed_probs = []
            degree = cython.declare(size_t, self.g.OutDegree(v))
            out_edges = cython.declare(AdjList[cython.double], self.g.OutEdges(v))
            for i in range(degree):
                x = out_edges[i].neighbour
                weight = out_edges[i].edge_data
                if x == t:
                    unnormed_probs.append(weight / self.p)
                else:
                    x_in_degree = cython.declare(size_t, self.g.InDegree(x))
                    x_inlist = cython.declare(AdjList[cython.double], self.g.InEdges(x))
                    is_find = False
                    for j in range(x_in_degree):
                        if x_inlist[j].neighbour == t:
                            is_find = True
                            break
                    if is_find:
                        unnormed_probs.append(weight)
                    else:
                        unnormed_probs.append(weight / self.q)
            
                norm_const = sum(unnormed_probs)
                norm_probs = [float(prob) / norm_const for prob in unnormed_probs]
                alias_table.Init(norm_probs)
                self.alias_edge[(t, v)] = alias_table
        return self.alias_edge[(t, v)]
        
    @cython.cfunc
    @cython.exceptval(check=False)
    def Walk(self, node:size_t) -> str:
        path = [cython.cast(size_t, node)]
        while cython.cast(size_t, len(path)) < self.walk_length:
            cur = cython.cast(size_t, int(path[len(path)-1]))  # 使用 int() 确保类型一致

            degree = cython.declare(size_t, self.g.OutDegree(cur))
            if degree > 0:
                out_edges = cython.declare(AdjList[cython.double], self.g.OutEdges(cur))
                if len(path) == 1:
                    a_nodes = self.GetAliasNodes(cur)
                    sample_idx = a_nodes.Sample()
                    path.append(out_edges[sample_idx].neighbour)
                else:
                    prev = path[len(path)-2]
                    a_edgs = self.GetAliasEdges(prev, cur)
                    sample_idx = a_edgs.Sample()
                    path.append(out_edges[sample_idx].neighbour)
            else:
                break
        print(path)

    @cython.cfunc
    @cython.exceptval(check=False)
    def Compute(self) -> cython.void:
        # 1. Walk
        path_list = []
        begin = 0
        end = self.sample_node.shape[0]
        for i in range(begin, end):
            cur_node = self.sample_node[i]
            path_list.append(self.Walk(cur_node))



    @cython.cfunc
    @cython.exceptval(check=False)
    def run(self, db: cython.pointer(GraphDB), olapondb:cython.pointer(OlapOnDB[cython.double]), dimensions: size_t, p: size_t, q: size_t, walk_length: size_t, sample_node:list, NodeInfo: list, EdgeInfo: list):
        self.txn = db.CreateReadTxn()
        with cython.nogil, parallel():
            self.num_threads = omp_get_num_threads()

        self.g = olapondb
        self.db = db
        self.dimensions = cython.cast(size_t, dimensions)
        self.p = cython.cast(size_t, p)
        self.q = cython.cast(size_t, q)
        self.walk_length = cython.cast(size_t, walk_length)
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
        EdgeInfo.append(np.zeros((sample_edge_num,), dtype=np.intp)) #edge_type_list

@cython.ccall
def Process(db_: lgraph_db_python.PyGraphDB, olapondb:lgraph_db_python.PyOlapOnDB, feature_num: size_t, p:size_t, q:size_t, walk_length: size_t, sample_node:list, NodeInfo: list, EdgeInfo: list):
    addr = cython.declare(cython.Py_ssize_t, db_.get_pointer())
    olapondb_addr = cython.declare(cython.Py_ssize_t, olapondb.get_pointer())
    a = Node2VecSample()
    printf("all_cost = %lf s\n", cython.cast(cython.double, 12312321321))
    a.run(cython.cast(cython.pointer(GraphDB), addr), cython.cast(cython.pointer(OlapOnDB[cython.double]), olapondb_addr), feature_num, p, q, walk_length, sample_node, NodeInfo, EdgeInfo)
