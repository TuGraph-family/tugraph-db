# cython: language_level=3, cpp_locals=True, boundscheck=False, wraparound=False, initializedcheck=False
# distutils: language = c++

import cython
from cython.cimports.olap_base import *
from cython.cimports.libc.stdio import printf
import time
import json
import lgraph_db_python


@cython.cclass
class WCCCore:
    graph: cython.pointer(OlapBase[Empty])
    label: ParallelVector[size_t]
    active_in: ParallelBitset
    active_out: ParallelBitset

    @cython.cfunc
    @cython.nogil
    @cython.exceptval(check=False)
    def Work(self, vi: size_t) -> size_t:
        degree = cython.declare(size_t, self.graph.OutDegree(vi))
        out_edges = cython.declare(AdjList[Empty], self.graph.OutEdges(vi))
        i = cython.declare(size_t, 0)
        local_num_activations = cython.declare(size_t, 0)
        dst: size_t
        for i in range(degree):
            dst = out_edges[i].neighbour
            if self.label[dst] > self.label[vi]:
                self.graph.AcquireVertexLock(dst)
                if self.label[dst] > self.label[vi]:
                    self.label[dst] = self.label[vi]
                    self.active_out.Add(dst)
                    local_num_activations += 1
                self.graph.ReleaseVertexLock(dst)
        return local_num_activations

    @cython.cfunc
    @cython.nogil
    @cython.exceptval(check=False)
    def run(self, g: cython.pointer(OlapBase[Empty])) -> cython.void:
        self.graph = g
        num_vertices = cython.declare(size_t, self.graph.NumVertices())
        printf("num_vertices = %lu\n", num_vertices)
        self.active_in = g.AllocVertexSubset()
        self.active_in.Fill()
        self.active_out = g.AllocVertexSubset()
        self.label = g.AllocVertexArray[size_t]()
        vi: cython.size_t
        for vi in range(num_vertices):
            self.label[vi] = vi
        num_activations = cython.declare(size_t, num_vertices)
        while num_activations > 0:
            self.active_out.Clear()
            num_activations = g.ProcessVertexActive[size_t, WCCCore](self.Work, self.active_in, self)
            self.active_out.Swap(self.active_in)
            printf("num_activations = %lu\n", num_activations)


@cython.cfunc
def procedure_process(db: cython.pointer(GraphDB), request: dict, response: dict) -> cython.bint:
    cost = time.time()
    txn = db.CreateReadTxn()
    olapondb = OlapOnDB[Empty](db[0], txn, SNAPSHOT_PARALLEL | SNAPSHOT_UNDIRECTED)
    cost = time.time() - cost
    printf("prepare_cost = %lf s\n", cython.cast(cython.double, cost))

    cost = time.time()
    a = WCCCore()
    a.run(cython.address(olapondb))
    cost = time.time() - cost
    printf("core_cost = %lf s\n", cython.cast(cython.double, cost))

    component = [0] * int(olapondb.NumVertices())
    i: size_t
    for i in range(olapondb.NumVertices()):
        component[a.label[i]] += 1
    max_component = 0
    num_components = 0
    for i in range(olapondb.NumVertices()):
        max_component = max(max_component, component[i])
        if component[i] > 0:
            num_components += 1
    response["max_component"] = max_component
    response["num_components"] = num_components
    response["num_vertices"] = olapondb.NumVertices()
    response["num_edges"] = olapondb.NumEdges()
    return True


@cython.ccall
def Process(db: lgraph_db_python.PyGraphDB, inp: bytes):
    _inp = inp.decode("utf-8")
    request = json.loads(_inp)
    response = {}
    addr = cython.declare(cython.Py_ssize_t, db.get_pointer())
    procedure_process(cython.cast(cython.pointer(GraphDB), addr),
                      request, response)
    return (True, json.dumps(response))
