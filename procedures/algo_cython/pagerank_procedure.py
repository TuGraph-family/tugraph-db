# cython: language_level=3, cpp_locals=True, boundscheck=False, wraparound=False, initializedcheck=False
# distutils: language = c++

# 注释作用如下：
# language_level=3: 使用Python3
# cpp_locals=True: 需要c++17，使用std::optional管理Python代码中的C++对象，可以避免C++对象的拷贝构造
# boundscheck=False: 关闭索引的边界检查
# wraparound=False: 关闭负数下标的处理（类似Python List）
# initializedcheck=False: 关闭检查内存是否初始化，关闭检查后运行性能更快
# language = c++: 将此py文件翻译为C++而不是C文件，TuGraph使用大量模板函数，所以都应该使用C++


import cython
from cython.cimports.olap_base import *
from cython.cimports.libc.stdio import printf
from cython.cimports.libc.math import fabs
import time
import json
import lgraph_db_python


@cython.cclass
class PageRankCore:
# cython.cclass 表示BFSCore为C类型的Class
    graph: cython.pointer(OlapBase[Empty])
    # cython.pointer(OlapBase[Empty])表示OlapBase[Empty]的指针，类似C++中OlapBase[Empty]*
    # cython提供了常见类型的指针，如cython.p_int, cython.p_char等，表示int*, char*, ...
    pr_curr: ParallelVector[cython.double]
    pr_next: ParallelVector[cython.double]
    d: cython.double
    one_over_all: cython.double
    iteration: cython.int
    num_iterations: cython.int
    dangling: cython.double
    # dangling: cython.double 声明root为C++ double类型变量，等效于dangling = cython.declare(cython.double)
    # 不声明类型的变量为Python object类型
    # 声明变量类型会大幅提高性能，同时在多线程部分，只有C/C++类型的变量可以访问

    @cython.cfunc
    @cython.nogil
    @cython.exceptval(check=False)
    def Work(self, vi: size_t) -> cython.double:
    # cython.cfunc 表示Work为C类型的函数，参数与返回值应声明
    # cfunc性能好，能接受C/C++对象为参数、返回值，但是不能在其他python文件中调用
    # 类似的有cython.ccall，如Standalone函数，可以在其他python文件中调用
    # cython.nogil 表示释放Python全局解释锁，在nogil修饰的部分，不能访问Python对象
    # 在多线程部分，都应有nogil修饰器
    # cython.exceptval(check=False) 表示禁用异常传播，将忽略函数内部引发的Python异常
        degree = cython.declare(size_t, self.graph.InDegree(vi))
        in_edges = cython.declare(AdjList[Empty], self.graph.InEdges(vi))
        i = cython.declare(size_t, 0)
        src: size_t
        pr = cython.declare(double, 0)
        for i in range(degree):
            src = in_edges[i].neighbour
            pr += self.pr_curr[src]
        self.pr_next[vi] = pr * self.d + self.one_over_all * (1.0 - self.d) + self.d * self.dangling
        if self.iteration == self.num_iterations - 1:
            return 0.0
        else:
            if self.graph.OutDegree(vi) > 0:
                self.pr_next[vi] /= self.graph.OutDegree(vi)
                return fabs(self.pr_curr[vi] - self.pr_next[vi]) * self.graph.OutDegree(vi)
            else:
                return fabs(self.pr_curr[vi] - self.pr_next[vi])

    @cython.cfunc
    @cython.nogil
    @cython.exceptval(check=False)
    def InitPR(self, vi: size_t) -> cython.double:
        self.pr_curr[vi] = self.one_over_all
        if self.graph.OutDegree(vi) > 0:
            self.pr_curr[vi] /= self.graph.OutDegree(vi)
        return 0.0

    @cython.cfunc
    @cython.nogil
    @cython.exceptval(check=False)
    def Dangling(self, vi: size_t) -> cython.double:
        if self.graph.OutDegree(vi) > 0:
            return 0.0
        return self.pr_curr[vi]


    @cython.cfunc
    @cython.nogil
    @cython.exceptval(check=False)
    def run(self, g: cython.pointer(OlapBase[Empty]), num_iterations: size_t) -> cython.void:
        self.d = 0.85
        num_vertices = cython.declare(size_t, g.NumVertices())
        self.one_over_all = 1.0 / num_vertices
        self.graph = g
        self.num_iterations = num_iterations
        self.pr_curr = g.AllocVertexArray[cython.double]()
        self.pr_next = g.AllocVertexArray[cython.double]()
        printf("num_vertices = %lu\n", num_vertices)
        delta = cython.declare(cython.double, 1.0)
        self.iteration = 0
        g.ProcessVertexInRange[cython.double, PageRankCore](self.InitPR, 0, num_vertices, self)
        while self.iteration < self.num_iterations:
            self.dangling = g.ProcessVertexInRange[cython.double, PageRankCore](self.Dangling, 0, num_vertices, self)
            self.dangling /= num_vertices
            delta = g.ProcessVertexInRange[cython.double, PageRankCore](self.Work, 0, num_vertices, self)
            self.pr_next.Swap(self.pr_curr)
            printf("delta = %f\n", delta)
            self.iteration += 1


@cython.cfunc
def procedure_process(db: cython.pointer(GraphDB), request: dict, response: dict) -> cython.bint:
    cost = time.time()
    iteration = 20
    if "iteration" in request:
        iteration = request["iteration"]

    txn = db.CreateReadTxn()
    olapondb = OlapOnDB[Empty](db[0], txn, SNAPSHOT_PARALLEL)
    # 并行创建OlapOnDB
    # Cython不支持如 *db 的解引用操作，通过db[0]来解引用
    cost = time.time() - cost
    printf("prepare_cost = %lf s\n", cython.cast(cython.double, cost))

    cost = time.time()
    a = PageRankCore()
    a.run(cython.address(olapondb), iteration)
    cost = time.time() - cost
    printf("core_cost = %lf s\n", cython.cast(cython.double, cost))

    max_pr = cython.declare(cython.double, 0)
    max_pr_vi = cython.declare(size_t, 0)
    for i in range(olapondb.NumVertices()):
        if max_pr < a.pr_curr[i]:
            max_pr = a.pr_curr[i]
            max_pr_vi = i
    response["max_pr_vi"] = max_pr_vi
    response["max_pr"] = max_pr
    response["num_vertices"] = olapondb.NumVertices()
    response["num_edges"] = olapondb.NumEdges()
    return True


@cython.ccall
def Process(db: lgraph_db_python.PyGraphDB, inp: bytes):
    # Process为embed模式和procedure模式下插件入口，用cython.ccall修饰
    # Process函数必须名为Process，参数为lgraph_db_python.PyGraphDB与bytes
    # 返回值必须为(bool, str)
    _inp = inp.decode("utf-8")
    request = json.loads(_inp)
    response = {}
    addr = cython.declare(cython.Py_ssize_t, db.get_pointer())
    # 获取PyGraphDB中GraphDB对象的地址，转换为指针后传递
    procedure_process(cython.cast(cython.pointer(GraphDB), addr),
                      request, response)
    return (True, json.dumps(response))