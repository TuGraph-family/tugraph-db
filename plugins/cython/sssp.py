# cython: language_level=3, cpp_locals=True, boundscheck=False, wraparound=False, initializedcheck=False
# distutils: language = c++

import cython
from cython.cimports.olap_base import *
from cython.cimports.lgraph_db import *
from cython.cimports.libc.stdio import printf
import time
import json
import lgraph_db_python


@cython.cclass
class SSSPCore:
    graph: cython.pointer(OlapBase[cython.double])
    distance: ParallelVector[cython.double]
    active_in: ParallelBitset
    active_out: ParallelBitset

    @cython.cfunc
    @cython.nogil
    @cython.exceptval(check=False)
    def Work(self, vi: size_t) -> size_t:
        degree = cython.declare(size_t, self.graph.OutDegree(vi))
        out_edges = cython.declare(AdjList[cython.double], self.graph.OutEdges(vi))
        i = cython.declare(size_t, 0)
        local_num_activations = cython.declare(size_t, 0)
        dst: size_t
        update_distance: cython.double
        for i in range(degree):
            dst = out_edges[i].neighbour
            update_distance = self.distance[vi] + out_edges[i].edge_data
            # lock = cython.declare(VertexLockGuard, graph.GuardVertexLock(dst))
            self.graph.AcquireVertexLock(dst)
            if self.distance[dst] > update_distance:
                self.active_out.Add(dst)
                self.distance[dst] = update_distance
                local_num_activations += 1
            self.graph.ReleaseVertexLock(dst)
        return local_num_activations

    @cython.cfunc
    @cython.nogil
    @cython.exceptval(check=False)
    def run(self, g: cython.pointer(OlapBase[cython.double]), root: size_t) -> cython.void:
        num_vertices = cython.declare(size_t, g.NumVertices())
        self.graph = g
        self.distance = g.AllocVertexArray[cython.double]()
        self.distance.Fill(2e10)
        self.distance[root] = 0
        self.active_in = g.AllocVertexSubset()
        self.active_in.Add(root)
        self.active_out = g.AllocVertexSubset()
        printf("num_vertices = %lu\n", num_vertices)
        active_vertices = cython.declare(size_t, 1)
        while active_vertices > 0:
            self.active_out.Clear()
            active_vertices = g.ProcessVertexActive[size_t, SSSPCore](self.Work, self.active_in, self)
            self.active_in.Swap(self.active_out)
            printf("num_active = %lu\n", active_vertices)


@cython.cfunc
def procedure_process(db: cython.pointer(GraphDB), request: dict, response: dict) -> cython.bint:
    cost = time.time()
    root_id = "0"
    label = "node"
    field = "id"
    if "root" in request:
        root_id = request["root"]
    if "label" in request:
        label = request["label"]
    if "field" in request:
        field = request["field"]

    txn = db.CreateReadTxn()
    olapondb = OlapOnDB[double](db[0], txn, SNAPSHOT_PARALLEL, cython.NULL, edge_convert_default[double])
    root_vid = txn.GetVertexIndexIterator(
        label.encode('utf-8'), field.encode('utf-8'),
        root_id.encode('utf-8'), root_id.encode('utf-8')
    ).GetVid()
    cost = time.time() - cost
    printf("prepare_cost = %lf s\n", cython.cast(cython.double, cost))

    cost = time.time()
    a = SSSPCore()
    a.run(cython.address(olapondb), root_vid)
    cost = time.time() - cost
    printf("core_cost = %lf s\n", cython.cast(cython.double, cost))

    max_distance = cython.declare(cython.double, 0)
    max_vi = cython.declare(size_t, 0)
    for i in range(olapondb.NumVertices()):
        if 1e10 > a.distance[i] > max_distance:
            max_distance = a.distance[i]
            max_vi = i
    response["max_distance"] = max_distance
    response["max_vi"] = max_vi
    response["num_vertices"] = olapondb.NumVertices()
    response["num_edges"] = olapondb.NumEdges()
    return True


@cython.ccall
def Standalone(input_dir: str, root: size_t = 0):
    cost = time.time()
    graph = OlapOnDisk[cython.double]()
    config = ConfigBase[cython.double]()
    config.input_dir = input_dir.encode("utf-8")
    config.parse_line = parse_line_weighted[cython.double]
    graph.Load(config, DUAL_DIRECTION)
    cost = time.time() - cost
    printf("load_cost = %lf s\n", cython.cast(cython.double, cost))

    cost = time.time()
    a = SSSPCore()
    a.run(cython.address(graph), root)
    cost = time.time() - cost
    printf("core_cost = %lf s\n", cython.cast(cython.double, cost))
    max_distance = cython.declare(cython.double, 0)
    max_vi = cython.declare(size_t, 0)
    for i in range(graph.NumVertices()):
        if 1e10 > a.distance[i] > max_distance:
            max_distance = a.distance[i]
            max_vi = i
    printf("max distance value is distance[%lu] = %f\n", max_vi, max_distance)


@cython.ccall
def Process(db: lgraph_db_python.PyGraphDB, inp: bytes):
    _inp = inp.decode("utf-8")
    request = json.loads(_inp)
    response = {}
    addr = cython.declare(cython.Py_ssize_t, db.get_pointer())
    procedure_process(cython.cast(cython.pointer(GraphDB), addr),
                      request, response)
    return (True, json.dumps(response))
