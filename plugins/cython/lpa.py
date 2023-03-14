# cython: language_level=3, cpp_locals=True, boundscheck=False, wraparound=False, initializedcheck=False
# distutils: language = c++

import cython
from cython.cimports.olap_base import *
from cython.cimports.lgraph_db import *
from cython.cimports.libc.stdio import printf
from cython.cimports.libcpp.unordered_map import unordered_map
from cython.operator import dereference as deref
from cython.operator import preincrement as inc
import time
import lgraph_db_python
import json

@cython.cclass
class LPACore:
    graph: cython.pointer(OlapBase[Empty])
    label_curr: ParallelVector[size_t]
    label_next: ParallelVector[size_t]

    @cython.cfunc
    @cython.nogil
    @cython.exceptval(check=False)
    def Work(self, vi: size_t) -> size_t:
        i = cython.declare(cython.int)
        dst_label:cython.int
        degree = cython.declare(cython.int, self.graph.OutDegree(vi))
        out_edges = cython.declare(AdjList[Empty], self.graph.OutEdges(vi))
        if degree == 0:
            return 0
        src_label = self.label_curr[vi]
        labels = unordered_map[size_t, size_t]()
        labels[src_label] = 1
        for i in range(degree):
            dst_label = self.label_curr[out_edges[i].neighbour]
            if labels.count(dst_label):
                labels[dst_label] += 1
            else:
                labels[dst_label] = 1
        max_l = cython.declare(size_t, src_label)
        max_n = cython.declare(size_t, 1)
        begin = cython.declare(unordered_map[size_t, size_t].iterator, labels.begin())
        end = cython.declare(unordered_map[size_t, size_t].iterator, labels.end())
        k: size_t
        v: size_t
        while begin != end:
            k = deref(begin).first
            v = deref(begin).second
            if v > max_n or (v == max_n and k < max_l):
                max_n = v
                max_l = k
            inc(begin)
        self.label_next[vi] = max_l
        if max_l == src_label:
            return 0
        return 1


    @cython.cfunc
    @cython.nogil
    @cython.exceptval(check=False)
    def run(self, g: cython.pointer(OlapBase[Empty]), num_iteration: size_t) -> cython.double:
        self.graph = g
        num_vertices = cython.declare(size_t, self.graph.NumVertices())
        printf("num_vertices = %lu\n", num_vertices)
        self.label_curr = g.AllocVertexArray[size_t]()
        self.label_next = g.AllocVertexArray[size_t]()
        vi: cython.size_t
        for vi in range(num_vertices):
            self.label_curr[vi] = vi
        num_activations = cython.declare(size_t, num_vertices)
        iteration = cython.declare(size_t, 0)
        while iteration < num_iteration:
            num_activations = g.ProcessVertexInRange[size_t, LPACore](self.Work, 0, num_vertices, self)
            self.label_curr.Swap(self.label_next)
            printf("num_activations = %lu\n", num_activations)
            iteration += 1
        return 0.0


@cython.cfunc
def procedure_process(db: cython.pointer(GraphDB), request: dict, response: dict) -> cython.bint:
    cost = time.time()
    iteration = 20
    if "iteration" in request:
        iteration = request["iteration"]

    txn = db.CreateReadTxn()
    olapondb = OlapOnDB[Empty](db[0], txn, SNAPSHOT_PARALLEL | SNAPSHOT_UNDIRECTED)
    cost = time.time() - cost
    printf("prepare_cost = %lf s\n", cython.cast(cython.double, cost))

    cost = time.time()
    a = LPACore()
    modularity = a.run(cython.address(olapondb), iteration)
    cost = time.time() - cost
    printf("core_cost = %lf s\n", cython.cast(cython.double, cost))

    community = [0] * olapondb.NumVertices()
    i: size_t
    for i in range(olapondb.NumVertices()):
        community[a.label_curr[i]] += 1
    max_community = 0
    num_communities = 0
    for i in range(olapondb.NumVertices()):
        max_community = max(max_community, community[i])
        if community[i] > 0:
            num_communities += 1
    response["max_community"] = max_community
    response["num_communities"] = num_communities
    response["modularity"] = modularity
    response["num_vertices"] = olapondb.NumVertices()
    response["num_edges"] = olapondb.NumEdges()
    return True


@cython.ccall
def Standalone(input_dir: str, num_iteration: size_t = 20):
    cost = time.time()
    graph = OlapOnDisk[Empty]()
    config = ConfigBase[Empty]()
    config.input_dir = input_dir.encode("utf-8")
    graph.Load(config, MAKE_SYMMETRIC)
    cost = time.time() - cost
    printf("load_cost = %lf s\n", cython.cast(cython.double, cost))

    cost = time.time()
    a = LPACore()
    a.run(cython.address(graph), num_iteration)
    cost = time.time() - cost
    printf("core_cost = %lf s\n", cython.cast(cython.double, cost))

    community = [0] * graph.NumVertices()
    i: size_t
    for i in range(graph.NumVertices()):
        community[a.label_curr[i]] += 1
    max_community = 0
    num_communities = 0
    for i in range(graph.NumVertices()):
        max_community = max(max_community, community[i])
        if community[i] > 0:
            num_communities += 1
    print("max_community = " + str(max_community))
    print("num_communities = " + str(num_communities))


@cython.ccall
def Process(db: lgraph_db_python.PyGraphDB, inp: bytes):
    _inp = inp.decode("utf-8")
    request = json.loads(_inp)
    response = {}
    addr = cython.declare(cython.Py_ssize_t, db.get_pointer())
    procedure_process(cython.cast(cython.pointer(GraphDB), addr),
                      request, response)
    return (True, json.dumps(response))