# OlapBase API

> 此文档主要详细介绍了OlapBase API的使用说明


## 1. 概述

本手册将介绍使用TuGraph图计算系统需要的简单配置，同时结合代码对TuGraph中几个共同的重要文件和接口进行解释。

## 2. 配置要求

如果要使用TuGraph图计算编写以及编译自己的应用程序，需要的配置要求为：

- linux操作系统，目前在Ubuntu16.04, Ubuntu18.04, Ubuntu20.04和Centos7, Centos8系统上可成功运行。
- 支持C++17的编译器，要求GCC版本为8.4.0或更新的版本。

## 3. 原子操作

TuGraph使用了多线程技术进行批处理操作，在这种情况下可能会出现访存冲突现象。为了保证并行计算时修改操作的正确性，TuGraph实现了原子操作。代码部分见lgraph文件夹下的lgraph_atomic.cpp文件。
TuGraph还自定义了4个常用的原子操作。当我们需要在多线程模式下修改点的数据时，我们都应该使用原子操作来确保并行环境下修改操作的正确性。除了这4个原子操作外，用户也可以使用“cas”来构建自己的原子操作函数。

- `bool cas(T * ptr, T oldv, T newv)`：如果ptr指向的值等于oldv，则将ptr指向的值赋为newv并返回true，否则返回false
- `bool write_min(T *a, T b)`：如果b比a指向的值更小，那么将a指向的值赋为b并返回true，否则返回false。
- `bool write_max(T *a, T b)`：如果b比a指向的值更大，那么将a指向的值赋为b并返回true，否则返回false。
- `void write_add(T *a, T b)`：将b的值加到a指向的值上。
- `void write_sub(T *a, T b)`：将a指向的值减去b的值。

## 4. 点集合类ParallelBitset

在使用TuGraph进行批处理操作时，需要使用点集合来表示需要处理的点。ParallelBitset实现了点集合类，以bit为单位表示点，因此能够节省大量内存。对应的代码见lgraph文件夹下的olap_base.h文件。

### 4.1 ParallelBitset类成员

- `size_t Size()`：表示Bitmap中的点个数。
- `ParallelBitset(size_t size)`：初始化size和data，data长度为(size >> 6)+1
- `void Clear()`：清空集合
- `void Fill()`：将所有点加入集合
- `bool Has(size_t i)`：检查点i是否在集合中
- `bool Add(size_t i)`：将点i加入集合中
- `void Swap(ParallelBitset &other)`：和另一组ParallelBitset集合交换元素

## 5. 点数组类ParallelVector

在使用TuGraph进行批处理操作时，需要使用点数组来表示对点的处理结果。ParallelVector实现了点数组类。对应的代码见lgraph文件夹下的olap_base.h文件。

### 5.1 ParallelVector类成员

- `ParallelVector(size_t capacity)` 构建ParallelVector，capacity为点数组的初始容量大小
- `T &operator[](size_t i)`：下标为i的数据
- `T *begin()`：ParallelVector的起始指针
- `T *end()`：ParallelVector的结束指针。begin和end的用法类似于vector容器的begin和end指针，可以使用这两个指针对数组进行顺序访问
- `T &Back()`：ParallelVector最后一个数据
- `T *Data()`：表示数组本身数据
- `void Destroy()`：清空ParallelVector数组内数据并删除数组
- `size_t Size()`：表示ParallelVector中的数据个数
- `size_t Capacity()`：表示ParallelVector的容量大小
- `void Resize(size_t size)`：更改ParallelVector为size大小，该size应大于等于更改前的大小且小于capacity
- `void Clear()`：清空ParallelVector内数据
- `void ReAlloc(size_t capacity)`：给ParallelVector分配新的容量大小，若数组有数据则将数据迁移至新内存
- `void Fill(T elem)`：为ParallelVector的全部数据赋值为elem
- `void Append(const T &elem, bool atomic = true)`：向ParallelVector结尾添加一个数据
- `void Swap(ParallelVector<T> &other)`：和其他的ParallelVector交换数据
- `ParallelVector<T> Copy()`：复制当前的ParallelVector数据存至Copy数组中

## 6. 自定义数据结构

### 6.1 基本数据类型

我们自定义了点和边的数据结构表示，用于在覆盖所有点的同时节省内存空间：

- `Empty`：内容为空的特殊数据类型。

### 6.2 组合数据结构

为了便于计算，我们根据计算场景不同，定义了几种点和边数据的数据结构，分别是：

- `EdgeUnit<EdgeData>`：表示权值类型为EdgeData的边，用于解析输入文件，包含三个成员变量：
  - `size_t src`：边的起始点
  - `size_t dst`：边的终点
  - `EdgeData edge_data`：边的权值
- `AdjUnit<EdgeData>`：表示权值类型为EdgeData的边，用于批处理计算过程中，包含两个成员变量：
  - `size_t neighbour`：边的邻居点
  - `EdgeData edge_data`：边的权值
- `AdjList<EdgeData>`：权值类型为EdgeData的点的邻接表，常用于表示点的入边和出边集合，包含两个成员变量：
  - `AdjUnit<T> * begin`：列表的起始指针
  - `AdjUnit<T> * end`：列表的结束指针。begin和end的用法类似于vector容器的begin和end指针，可以使用这两个指针对邻接表进行循环访问。

## 7. 图类OlapBase

图类OlapBase是TuGraph用于加载图以及进行图计算操作的主类，常用OlapBase<EdgeData>表示权值类型为EdgeData的图，代码部分见lgraph文件夹下的olap_base.hpp。本章将介绍Graph类中常用的类型和API接口。上文介绍Procedure、Embed及Standalone功能所使用的类均为该类的子类。

### 7.1 基本信息

- `size_t NumVertices()`：获取点数
- `size_t NumEdges()`：获取边数
- `size_t OutDegree(size_t vid)`：点vid的出度
- `size_t InDegree(size_t vid)`：点vid的入度

### 7.2 点集和边集及其相关操作

- `ParallelVector<VertexData> AllocVertexArray<VertexData>()`：分配一个类型为VertexData的数组，大小为点个数
- `void fill_vertex_array<V>(V * array, V value)`：将数组array中的所有元素赋值为value
- `ParallelBitset AllocVertexSubset()`：分配一个ParallelBitset集合，用于表示所有点的状态是否激活
- `AdjList<EdgeData> OutEdges(size_t vid)`：获取点v的所有出边集合
- `AdjList<EdgeData> InEdges(size_t vid)`：获取点v的所有入边集合
- `void Transpose()`：对有向图进行图反转
- `LoadFromArray(char * edge_array, VertexId input_vertices, EdgeId input_edges,  EdgeDirectionPolicy edge_direction_policy)`：从数组中加载图数据，包含四个参数，其含义分别表示：
  - `edge_array`：将该数组中的数据读入图，一般情况下该数组包含多条边。
  - `input_vertices`：指定数组读入图的点个数。
  - `input_edges`：指定数组读入图的边的条数。
  - `edge_direction_policy`：指定图为有向或无向，包含三种模式，分别为DUAL_DIRECTION、MAKE_SYMMETRIC以及INPUT_SYMMETRIC。对应的详细介绍见include/lgraph/olap_base.h文件的`enum EdgeDirectionPolicy`。

### 7.3 锁机制

TuGraph实现了一对锁机制，来控制程序对于点数据的访存权限。分别是：

- `void AcquireVertexLock(size_t vid)`：对点vid加锁，禁止其它线程对该锁对应的点数据进行访存
- `void ReleaseVertexLock(size_t vid)`：对点vid解锁，所有线程均可访存该锁对应的点数据
- `VertexLockGuard GuardVertexLock(size_t vid)`：在对vid操作时，对点vid加锁，退出作用域时时自动释放锁

### 7.4 批处理操作

TuGraph提供了两个批处理操作来并行地进行以点为中心的批处理过程。分别是：

```c++
/*
    函数名称:ReducedSum ProcessVertexInRange(std::function<ReducedSum(size_t)> work, size_t lower, size_t upper,
                ReducedSum zero = 0,std::function<ReducedSum(ReducedSum, ReducedSum)> reduce =reduce_plus<ReducedSum>)
				
    函数用途:对Graph中节点编号介于lower和upper之间的节点执行work函数。第四个参数表示累加的基数，默认为0；
    第五个参数表示对每个work处理后的节点返回值进行迭代reduce函数操作，默认为累加操作。
    具体实现请参考include/lgraph/olap_base.h中具体代码

    使用示例:统计数组parent数组中有出边的点个数
*/

auto vertex_num = graph.ProcessVertexInRange<size_t>(
    [&](size_t i) {
        if (graph.OutDegree(parent[i]) > 0) {
            return 1;
        }
    },
    0, parent.Size()
);
printf("the number is %lu\n",vertex_num);
```

其中graph为图类OlapBase的实例化对象

```C++
/*
    函数名称:ReducedSum ProcessVertexActive(std::function<ReducedSum(size_t)> work, ParallelBitset &active_vertices,
                ReducedSum zero = 0,std::function<ReducedSum(ReducedSum, ReducedSum)> reduce =reduce_plus<ReducedSum>)
				
    函数用途:对active_vertices中对应为1的节点执行work函数，第三个参数表示累加的基数，默认为0；
    第四个参数表示对每个work处理后的节点返回值进行迭代reduce函数操作，默认为累加操作。
    具体实现请参考/include/lgraph/olap_base.h中具体代码
    
    使用示例:输出Graph中节点1，2，3的所有出度邻居，并统计这三个节点的总出度
*/

auto active_in = graph.AllocVertexSubset();
active_in.Add(1);
active_in.Add(2);
active_in.Add(3);
auto total_outdegree = graph.ProcessVertexActive<size_t>(
    [&](size_t vi) {
        size_t local_outdegree = 0;
        for (auto & edge : graph.OutEdges(vi)) {
            size_t dst = edge.neighbour;
            printf("node %lu has neighbour %lu\n",vi,dst);
            local_outdegree += 1;
        }
        return local_outdegree;
    },
    active_in
);
printf("total outdegree of node1,2,3 is %lu\n",total_outdegree);
```
