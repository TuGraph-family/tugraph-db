# cython: language_level=3, cpp_locals=True, boundscheck=False, wraparound=False, initializedcheck=False
# distutils: language = c++

import cython
from cython.cimports.olap_base import *
from cython.cimports.libc.stdio import printf
from cython.cimports.libc.stdlib import qsort
import time
import json
import lgraph_db_python


@cython.cfunc
@cython.nogil
@cython.exceptval(check=False)
def count_common(num_triangle: cython.pointer(ParallelVector[uint64_t]),
                 list_a: AdjList[Empty], list_b: AdjList[Empty],
                 a: size_t, b: size_t) -> size_t:
    local_count = cython.declare(size_t, 0)
    ptr_a = cython.declare(cython.pointer(AdjUnit[Empty]), list_a.begin())
    ptr_b = cython.declare(cython.pointer(AdjUnit[Empty]), list_b.begin())
    pre_a = cython.declare(size_t, -1)
    pre_b = cython.declare(size_t, -1)
    threshold: size_t
    if a < b:
        threshold = a
    else:
        threshold = b

    while ptr_a != list_a.end() and ptr_b != list_b.end() and ptr_a.neighbour < threshold and ptr_b.neighbour < threshold:
        if pre_a == ptr_a.neighbour:
            ptr_a += 1
            continue
        if pre_b == ptr_b.neighbour:
            ptr_b += 1
            continue
        if ptr_a.neighbour < ptr_b.neighbour:
            pre_a = ptr_a.neighbour
            ptr_a += 1
        elif ptr_a.neighbour > ptr_b.neighbour:
            pre_b = ptr_b.neighbour
            ptr_b += 1
        else:
            pre_a = ptr_a.neighbour
            pre_b = ptr_b.neighbour
            write_add(num_triangle.begin() + a, 1)
            write_add(num_triangle.begin() + b, 1)
            write_add(num_triangle.begin() + pre_a, 1)
            local_count += 1
            ptr_a += 1
            ptr_b += 1
    return local_count


@cython.cfunc
@cython.nogil
@cython.exceptval(check=False)
def compare(a: const_p_void, b: const_p_void) -> cython.int:
    ptr_a = cython.cast(cython.pointer(AdjUnit[Empty]), a)
    ptr_b = cython.cast(cython.pointer(AdjUnit[Empty]), b)
    return (ptr_a.neighbour - ptr_b.neighbour)

@cython.cclass
class LCCCore:
    graph: cython.pointer(OlapBase[Empty])
    score: ParallelVector[cython.double]
    num_triangle: ParallelVector[uint64_t]

    @cython.cfunc
    @cython.nogil
    @cython.exceptval(check=False)
    def Count(self, src: size_t) -> size_t:
        src_adj = self.graph.OutEdges(src)
        pre = cython.declare(size_t, -1)
        src_degree = self.graph.OutDegree(src)
        if src_degree < 2:
            return 0
        dst: size_t
        for i in range(src_degree):
            dst = src_adj[i].neighbour
            if pre == dst:
                continue
            else:
                pre = dst
            if src < dst:
                neighbour_adj = self.graph.OutEdges(dst)
                count_common(cython.address(self.num_triangle), src_adj, neighbour_adj, src, dst)
        return 0

    @cython.cfunc
    @cython.nogil
    @cython.exceptval(check=False)
    def SortEdge(self, vi: size_t) -> size_t:
        edges = cython.declare(AdjList[Empty], self.graph.OutEdges(vi))
        qsort(edges.begin(), self.graph.OutDegree(vi), cython.sizeof(AdjUnit[Empty]), cython.address(compare))
        return 0

    @cython.cfunc
    @cython.nogil
    @cython.exceptval(check=False)
    def ComputeScore(self, vi: size_t) -> cython.double:
        valid_degree = cython.declare(size_t, 0)
        prev_nbr = cython.declare(size_t, -1)
        degree = self.graph.OutDegree(vi)
        edges = self.graph.OutEdges(vi)
        nbr: size_t
        for i in range(degree):
            nbr = edges[i].neighbour
            if nbr == prev_nbr or nbr == vi:
                continue
            else:
                valid_degree += 1
                prev_nbr = nbr
        if valid_degree < 2:
            return 0.0
        self.score[vi] = 2.0 * self.num_triangle[vi] / valid_degree / (valid_degree - 1)
        return self.score[vi]



    @cython.cfunc
    @cython.nogil
    @cython.exceptval(check=False)
    def run(self, g: cython.pointer(OlapBase[Empty])) -> cython.double:
        self.graph = g
        num_vertices = cython.declare(size_t, self.graph.NumVertices())
        printf("[cython]: num_vertices = %lu\n", num_vertices)
        self.score = g.AllocVertexArray[cython.double]()
        self.score.Fill(0.0)
        self.num_triangle = g.AllocVertexArray[uint64_t]()
        self.num_triangle.Fill(0)
        g.ProcessVertexInRange[size_t, LCCCore](self.SortEdge, 0, num_vertices, self)
        g.ProcessVertexInRange[size_t, LCCCore](self.Count, 0, num_vertices, self)
        sum_clco: cython.double
        sum_clco = g.ProcessVertexInRange[cython.double, LCCCore](self.ComputeScore, 0, num_vertices, self)
        return sum_clco / num_vertices


@cython.cfunc
def procedure_process(db: cython.pointer(GraphDB), request: dict, response: dict) -> cython.bint:
    cost = time.time()
    txn = db.CreateReadTxn()
    olapondb = OlapOnDB[Empty](db[0], txn, SNAPSHOT_PARALLEL | SNAPSHOT_UNDIRECTED)
    cost = time.time() - cost
    printf("prepare_cost = %lf s\n", cython.cast(cython.double, cost))
    a = LCCCore()
    cost = time.time()
    average_lcc = a.run(cython.address(olapondb))
    cost = time.time() - cost
    printf("core_cost = %lf s\n", cython.cast(cython.double, cost))
    response["average_lcc"] = average_lcc
    response["num_vertices"] = olapondb.NumVertices()
    response["num_edges"] = olapondb.NumEdges()
    return True


@cython.ccall
def Standalone(input_dir: str):
    cost = time.time()
    graph = OlapOnDisk[Empty]()
    config = ConfigBase[Empty]()
    config.input_dir = input_dir.encode("utf-8")
    graph.Load(config, MAKE_SYMMETRIC)
    cost = time.time() - cost
    printf("load_cost = %lf s\n", cython.cast(cython.double, cost))

    cost = time.time()
    a = LCCCore()
    average_lcc = a.run(cython.address(graph))
    cost = time.time() - cost
    printf("core_cost = %lf s\n", cython.cast(cython.double, cost))
    print("average_lcc = {}".format(average_lcc))



@cython.ccall
def Process(db: lgraph_db_python.PyGraphDB, inp: bytes):
    _inp = inp.decode("utf-8")
    request = json.loads(_inp)
    response = {}
    addr = cython.declare(cython.Py_ssize_t, db.get_pointer())
    procedure_process(cython.cast(cython.pointer(GraphDB), addr),
                      request, response)
    return (True, json.dumps(response))