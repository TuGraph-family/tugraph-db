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
from cython.cimports.olap_on_disk import *
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


@cython.ccall
def Standalone(input_dir: str, root: size_t = 0):
    # Standalone为Standalone模式下插件入口，用cython.ccall修饰
    # 可以任意设置参数，相应修改 procedures/algo_cython/run_standalone.py即可
    cost = time.time()
    graph = OlapOnDisk[Empty]()
    config = ConfigBase[Empty]()
    config.input_dir = input_dir.encode("utf-8")
    # config为C++类，config.input_dir为std::string，Python str需要encode才能传给std::string
    graph.Load(config, DUAL_DIRECTION)
    cost = time.time() - cost
    printf("load_cost = %lf s\n", cython.cast(cython.double, cost))

    cost = time.time()
    a = BFSCore()
    count = a.run(cython.address(graph), root)
    # cython.address(graph)，取址，类似C++中&graph
    cost = time.time() - cost
    printf("core_cost = %lf s\n", cython.cast(cython.double, cost))
    print("found_vertices = {}".format(count))
