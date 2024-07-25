# cython: language_level=3, cpp_locals=True, boundscheck=False, wraparound=False, initializedcheck=False
# distutils: language = c++

# 注释作用如下：
# language_level=3: 使用Python3
# cpp_locals=True: 需要c++17，使用std::optional管理Python代码中的C++对象，可以避免C++对象的拷贝构造
# boundscheck=False: 关闭索引的边界检查
# wraparound=False: 关闭负数下标的处理（类似Python List）
# initializedcheck=False: 关闭检查内存是否初始化，关闭检查后运行性能更快
# language = c++: 将此py文件翻译为C++而不是C文件，TuGraph使用大量模板函数，所以都应该使用C++

import json

import cython
from cython.cimports.olap_base import *
from cython.cimports.libc.stdio import printf
import time
import lgraph_db_python


@cython.cclass
class BFSCore:
# cython.cclass 表示BFSCore为C类型的Class
    graph: cython.pointer(OlapBase[Empty])
    # cython.pointer(OlapBase[Empty])表示OlapBase[Empty]的指针，类似C++中OlapBase[Empty]*
    # cython提供了常见类型的指针，如cython.p_int, cython.p_char等，表示int*, char*, ...
    parent: ParallelVector[size_t]
    active_in: ParallelBitset
    active_out: ParallelBitset
    root: size_t
    # root: size_t 声明root为C++ size_t类型变量，等效于root = cython.declare(size_t)
    # 不声明类型的变量为Python object类型
    # 声明变量类型会大幅提高性能，同时在多线程部分，只有C/C++类型的变量可以访问

    @cython.cfunc
    @cython.nogil
    @cython.exceptval(check=False)
    def Work(self, vi: size_t) -> size_t:
    # cython.cfunc 表示Work为C类型的函数，参数与返回值应声明
    # cfunc性能好，能接受C/C++对象为参数、返回值，但是不能在其他python文件中调用
    # 类似的有cython.ccall，如Standalone函数，可以在其他python文件中调用
    # cython.nogil 表示释放Python全局解释锁，在nogil修饰的部分，不能访问Python对象
    # 在多线程部分，都应有nogil修饰器
    # cython.exceptval(check=False) 表示禁用异常传播，将忽略函数内部引发的Python异常
        degree = cython.declare(size_t, self.graph.OutDegree(vi))
        out_edges = cython.declare(AdjList[Empty], self.graph.OutEdges(vi))
        i = cython.declare(size_t, 0)
        local_num_activations = cython.declare(size_t, 0)
        dst: size_t
        for i in range(degree):
            dst = out_edges[i].neighbour
            if self.parent[dst] == cython.cast(size_t, -1):
                # parent[dst] == -1 表示dst没有被bfs访问过
                if self.active_out.Add(dst):
                    # 将dst设置为为活跃节点；ParallelBitmap.Add为原子操作，防止重复计算
                    self.parent[dst] = vi
                    local_num_activations += 1
        return local_num_activations

    @cython.cfunc
    @cython.nogil
    @cython.exceptval(check=False)
    def run(self, g: cython.pointer(OlapBase[Empty]), r: size_t) -> cython.size_t:
        self.graph = g
        self.root = r
        self.active_in = g.AllocVertexSubset()
        self.active_out = g.AllocVertexSubset()
        self.parent = g.AllocVertexArray[size_t]()
        self.parent.Fill(-1)
        num_vertices = cython.declare(size_t, self.graph.NumVertices())
        printf("num_vertices = %lu\n", num_vertices)
        self.parent[self.root] = self.root
        num_activations = cython.declare(size_t, 1)
        discovered_vertices = cython.declare(size_t, num_activations)
        self.active_in.Add(self.root)
        while num_activations > 0:
            self.active_out.Clear()
            num_activations = g.ProcessVertexActive[size_t, BFSCore](self.Work, self.active_in, self)
            discovered_vertices += num_activations
            self.active_out.Swap(self.active_in)
            printf("num_activations = %lu\n", num_activations)
        return discovered_vertices


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
    olapondb = OlapOnDB[Empty](db[0], txn, SNAPSHOT_PARALLEL)
    # 并行创建OlapOnDB
    # Cython不支持如 *db 的解引用操作，通过db[0]来解引用
    root_vid = txn.GetVertexIndexIterator(
        label.encode('utf-8'), field.encode('utf-8'),
        root_id.encode('utf-8'), root_id.encode('utf-8')
    ).GetVid()
    # 通过 GetVertexIndexIterator 根据root节点label名和filed名与filed值（root_id）
    # 获取root节点的迭代器，通过迭代器获取vid，在无ID_MAPPING时，该vid与OlapOnDB中的id相同
    cost = time.time() - cost
    printf("prepare_cost = %lf s\n", cython.cast(cython.double, cost))
    a = BFSCore()
    cost = time.time()
    count = a.run(cython.address(olapondb), root_vid)
    cost = time.time() - cost
    printf("core_cost = %lf s\n", cython.cast(cython.double, cost))
    response["found_vertices"] = count
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
