# cython: language_level=3, cpp_locals=True, boundscheck=False, wraparound=False, initializedcheck=False
# distutils: language = c++

import cython
from cython.cimports.olap_base import *
from cython.cimports.lgraph_db import *
from cython.parallel import parallel
from cython.cimports.openmp import omp_get_num_threads
from cython.cimports.libcpp.random import random_device, mt19937, uniform_real_distribution
from cython.cimports.cpython import array
from cython.cimports.libc.string import memcpy
import numpy as np
import lgraph_db_python
from gensim.models import Word2Vec

@cython.cclass
class AliasTable:
    size: int
    accept: list
    alias: list

    def __init__(self):
        self.size = 0
        self.accept = []
        self.alias = []
    
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
        self.accept = [0.0] * self.size
        self.alias = [0] * self.size
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
    model : Word2Vec

    ########### Input Parameters ###########
    g: cython.pointer(OlapOnDB[cython.double])
    db: cython.pointer(GraphDB)
    dimensions: size_t
    p: cython.double
    q: cython.double
    walk_length: size_t
    num_walks: size_t
    sample_node: size_t[:]

    ########### Result ###########
    label_key: string
    feature_key: string
    node: ssize_t[:]
    feature: cython.float[:,:]
    label: ssize_t[:]
    vertex_type_string: ssize_t[:]
    src_list: ssize_t[:]
    dst_list: ssize_t[:]
    local_node: list
    local_src_list: list
    local_dst_list: list

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
                if weight == 0:
                    weight = 1
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
                if weight == 0:
                    weight = 1
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
    def Walk(self, node:size_t) -> list:
        path = [cython.cast(size_t, node)]
        while cython.cast(size_t, len(path)) < self.walk_length:
            cur = cython.cast(size_t, int(path[len(path)-1]))
            degree = cython.declare(size_t, self.g.OutDegree(cur))
            if degree > 0:
                out_edges = cython.declare(AdjList[cython.double], self.g.OutEdges(cur))
                if len(path) == 1:
                    a_nodes = self.GetAliasNodes(cur)
                    sample_idx = a_nodes.Sample()
                    self.local_node.append(cur)
                    self.local_node.append(out_edges[sample_idx].neighbour)
                    self.local_src_list.append(cur)
                    self.local_dst_list.append(out_edges[sample_idx].neighbour)
                    path.append(out_edges[sample_idx].neighbour)
                else:
                    prev = path[len(path)-2]
                    a_edgs = self.GetAliasEdges(prev, cur)
                    sample_idx = a_edgs.Sample()
                    self.local_node.append(cur)
                    self.local_node.append(out_edges[sample_idx].neighbour)
                    self.local_src_list.append(cur)
                    self.local_dst_list.append(out_edges[sample_idx].neighbour)
                    path.append(out_edges[sample_idx].neighbour)
            else:
                break
        return [str(node) for node in path]

    @cython.cfunc
    @cython.exceptval(check=False)
    def Compute(self) -> cython.void:
        # 1. Walk
        path_list = []
        for _ in range(self.num_walks):
            for i in range(self.sample_node.shape[0]):
                cur_node = self.sample_node[i]
                path_list.append(self.Walk(cur_node))

        # 2. Word2Vec
        self.model = Word2Vec(sentences=path_list, vector_size=self.dimensions, window=2, min_count=1, sg=1, workers=self.num_threads, epochs=10)
    
    @cython.cfunc
    @cython.exceptval(check=False)
    def CopyResult(self) -> cython.void:
        vit = self.txn.GetVertexIterator()
        for i in range(len(self.local_node)):
            cur_node = self.local_node[i]
            self.node[i] = cur_node
            vec = self.model.wv[str(cur_node)]
            for j in range(self.dimensions):
                self.feature[i, j] = vec[j]
            self.g.AcquireVertexLock(self.local_node[i])
            vit.Goto(self.g.OriginalVid(cur_node))
            for l in range(self.txn.GetVertexSchema(vit.GetLabel()).size()):
                if self.txn.GetVertexSchema(vit.GetLabel())[l].name == self.feature_key:
                    self.label[i] = vit.GetField(self.label_key).AsInt64()
            vertex_type = vit.GetLabelId()
            self.vertex_type_string[i] = vertex_type
            self.g.ReleaseVertexLock(self.local_node[i])
        for i in range(len(self.local_src_list)):
            self.src_list[i] = self.local_src_list[i]
            self.dst_list[i] = self.local_dst_list[i]
    
    
    @cython.cfunc
    @cython.exceptval(check=False)
    def run(self, db: cython.pointer(GraphDB), olapondb:cython.pointer(OlapOnDB[cython.double]), dimensions: size_t, p: cython.double, q: cython.double, walk_length: size_t, num_walks:size_t ,sample_node:list, NodeInfo: list, EdgeInfo: list):
        self.txn = db.CreateReadTxn()
        with cython.nogil, parallel():
            self.num_threads = omp_get_num_threads()

        self.g = olapondb
        self.db = db
        self.dimensions = cython.cast(size_t, dimensions)
        self.p = cython.cast(cython.double, p)
        self.q = cython.cast(cython.double, q)
        self.walk_length = cython.cast(size_t, walk_length)
        self.num_walks = cython.cast(size_t, num_walks)
        self.sample_node = array.array('L', sample_node)


        self.feature_key = "feature_float".encode('utf-8')
        self.label_key = "label".encode('utf-8')
        self.local_node = []
        self.local_src_list = []
        self.local_dst_list = []

        self.Compute()

        sample_node_num = cython.declare(ssize_t, len(self.local_node))
        sample_edge_num = cython.declare(ssize_t, len(self.local_src_list))
        self.node = np.zeros((sample_node_num,), dtype=np.intp)
        self.feature = np.zeros((sample_node_num, dimensions), dtype=np.float32)
        self.label = np.zeros((sample_node_num,), dtype=np.intp)
        self.vertex_type_string = np.zeros((sample_node_num,), dtype=np.intp)
        self.src_list = np.zeros((sample_edge_num,), dtype=np.intp)
        self.dst_list = np.zeros((sample_edge_num,), dtype=np.intp)

        self.CopyResult()

        NodeInfo.append(np.asarray(self.node))
        NodeInfo.append(np.asarray(self.feature))
        NodeInfo.append(np.asarray(self.label))
        NodeInfo.append(np.asarray(self.vertex_type_string))
        EdgeInfo.append(np.asarray(self.src_list))
        EdgeInfo.append(np.asarray(self.dst_list))
        EdgeInfo.append(np.zeros((sample_edge_num,), dtype=np.intp)) #edge_type_list

@cython.ccall
def Process(db_: lgraph_db_python.PyGraphDB, olapondb:lgraph_db_python.PyOlapOnDB, feature_num: size_t, p:cython.double , q:cython.double, walk_length: size_t, num_walks: size_t, sample_node:list, NodeInfo: list, EdgeInfo: list):
    addr = cython.declare(cython.Py_ssize_t, db_.get_pointer())
    olapondb_addr = cython.declare(cython.Py_ssize_t, olapondb.get_pointer())
    a = Node2VecSample()
    a.run(cython.cast(cython.pointer(GraphDB), addr), cython.cast(cython.pointer(OlapOnDB[cython.double]), olapondb_addr), feature_num, p, q, walk_length, num_walks, sample_node, NodeInfo, EdgeInfo)
