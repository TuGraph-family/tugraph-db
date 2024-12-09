# Traversal API

> 此文档主要讲解 TuGraph 的存储过程中的Traversal API

## 1. 简介

TuGraph 强大的在线分析处理（OLAP）能力是其区别于其它图数据库的一个重要特性。
借助 C++ OLAP API（olap_on_db.h），用户可以快速地导出一个需要进行复杂分析的子图，然后在其上运行诸如 PageRank、连通分量、社区发现等迭代式图计算过程，最后根据结果做出相应决策。
导出和计算的过程都可以通过并行处理的方式进行加速，从而实现几乎实时的分析处理，避免了传统解决方案需要将数据导出、转换、再导入（ETL）到专门的分析系统进行离线处理的冗长步骤。

TuGraph 内置了大量常用的图分析算法和丰富的辅助接口，因此用户几乎不需要自己来实现具体的图计算过程，只需在实现自己的存储过程时将相应算法库的头文件（.h 文件）包含到自己程序中，并在编译时链接相应的动态库文件（.so）即可。
一般情况下，用户需要自己实现的只有将需要分析的子图抽取出来的过程。

目前 Traversal API 仅支持 C++。

## 2. 接口说明

### 2.1. Snapshot

C++ OLAP API 中的 Snapshot 模版类用于表示抽取出来的静态子图，其中 EdgeData 用来表示该子图上每条边所用权值的数据类型（如果边不需要权值，使用 Empty 作为 EdgeData 即可）。

抽取的子图通过 Snapshot 类的构造函数来描述：

```c
Snapshot::Snapshot(
    GraphDB & db,
    Transaction & txn,
    size_t flags = 0,
    std::function<bool(VertexIterator &)> vertex_filter = nullptr,
    std::function<bool(OutEdgeIterator &, EdgeData &)> out_edge_filter = nullptr
);
```

其中，db 为数据库句柄，txn 为事务句柄，flags 为生成时使用的选项，可选值包括以下的组合：SNAPSHOT_PARALLEL 表示导出时使用多个线程进行并行；SNAPSHOT_UNDIRECTED 表示需要将导出的图变为无向图。
vertex_filter 是面向点的用户自定义过滤函数，返回值为 true 表示该点需要被包含到待抽取的子图中，反之则表示需要被排除。
out_edge_filter 是面向边的用户自定义过滤函数，返回值为 true 表示该边需要被包含到待抽取的子图中，反之则表示需要被排除。
当过滤函数为缺省值时，则表示需要将所有点/边都包含进来。

Snapshot 类提供的其它方法请参考详细的 C++ API 文档（olap_on_db.h）。

### 2.2. Traversal

图数据库中十分常见的一大类分析是基于一个或多个点出发，逐层地拓展并访问邻居。
尽管这类分析也可以使用 Cypher 完成，但是当访问的层数较深时，其性能会受到串行解释执行的限制。
使用 C++ Core API 编写存储过程尽管避免了解释执行，但依然受限于单个线程的处理能力。
为了让用户能够方便地通过并行处理的方式加速这一类应用场景，我们基于 C++ OLAP API 封装了一个 Traversal 框架，用户可以直接使用其中的 FrontierTraversal 和 PathTraversal 类来完成这种逐层遍历的分析任务，具体的使用方法可以参考相应的 C++ API 文档（lgraph_traversal.h）。

```c
ParallelVector<size_t> FindVertices(
    GraphDB & db,
    Transaction & txn,
    std::function<bool(VertexIterator &)> filter,
    bool parallel = false
);
```

该方法可用于找到所有满足条件（filter 返回 true）的点，当 parallel 为 true 时则会并行该查找过程。

```c
template <typename VertexData>
ParallelVector<VertexData> ExtractVertexData(
    GraphDB & db,
    Transaction & txn,
    ParallelVector<size_t> & frontier,
    std::function<void(VertexIterator &, VertexData &)> extract,
    bool parallel = false
);
```

该方法可用于从指定点集（frontier）中（通过 extract 方法）抽取（类型为 VertexData 的）属性，当 parallel 为 true 时会并行该抽取过程。

FrontierTraversal 适用于只关注遍历扩展到的点集的情况；当用户在遍历过程或是结果中需要访问路径上的信息（路径上的点/边）时，则需要使用 PathTraversal。
两类 Traversal 的构造函数均有四个参数，分别为数据库句柄 db、事务句柄 txn、选项 flags 和 初始化数组容量 capacity。
选项的可选值包括以下的组合：TRAVERSAL_PARALLEL 表示遍历时使用多个线程并行；TRAVERSAL_ALLOW_REVISITS 表示遍历时允许重复地访问点（PathTraversal 隐含了该选项）。capacity 表示初始化时路径集合的容量。

```c
void SetFrontier(size_t root_vid);
void SetFrontier(ParallelVector<size_t> & root_vids);
void SetFrontier(std::function<bool(VertexIterator &)> root_vertex_filter);
```

两类 Traversal 设置遍历的起始点/点集有上述三种方式，前两种通过点 ID 直接指定，最后一种方式则类似于 FindVertices。

两类 Traversal 的遍历都是从当前层的点集合出发，根据使用的扩展函数访问每条出边/入边/出边和入边，通过用户自定义的过滤函数决定扩展是否成功，若成功则将邻居点/追加了该条边的路径加入下一层的点/路径集合。

```c
void ExpandOutEdges(
    std::function<bool(OutEdgeIterator &)> out_edge_filter = nullptr,
    std::function<bool(VertexIterator &)> out_neighbour_filter = nullptr
);
void ExpandInEdges(
    std::function<bool(InEdgeIterator &)> in_edge_filter = nullptr,
    std::function<bool(VertexIterator &)> in_neighbour_filter = nullptr
);
void ExpandEdges(
    std::function<bool(OutEdgeIterator &)> out_edge_filter = nullptr,
    std::function<bool(InEdgeIterator &)> in_edge_filter = nullptr,
    std::function<bool(VertexIterator &)> out_neighbour_filter = nullptr,
    std::function<bool(VertexIterator &)> in_neighbour_filter = nullptr
);
```

上述为 FrontierTraversal 的三种遍历方式，即从当前的点集合出发，对集合中的每个点，依次访问每条出边/入边/出边和入边，若满足用户自定义的过滤条件（其中，edge_filter 为面向边的过滤函数，neighbour_filter 则为面向邻居点的过滤函数），则将邻居点加入新的点集合。

```c
ParallelVector<size_t> & GetFrontier();
```

当前点集合的扩展结束后，新的点集合可以通过上述方法取得。

```c
void ExpandOutEdges(
    std::function<bool(OutEdgeIterator &, Path &, IteratorHelper &)> out_edge_filter = nullptr,
    std::function<bool(VertexIterator &, Path &, IteratorHelper &)> out_neighbour_filter = nullptr
);
void ExpandInEdges(
    std::function<bool(InEdgeIterator &, Path &, IteratorHelper &)> in_edge_filter = nullptr,
    std::function<bool(VertexIterator &, Path &, IteratorHelper &)> in_neighbour_filter = nullptr
);
void ExpandEdges(
    std::function<bool(OutEdgeIterator &, Path &, IteratorHelper &)> out_edge_filter = nullptr,
    std::function<bool(InEdgeIterator &, Path &, IteratorHelper &)> in_edge_filter = nullptr,
    std::function<bool(VertexIterator &, Path &, IteratorHelper &)> out_neighbour_filter = nullptr,
    std::function<bool(VertexIterator &, Path &, IteratorHelper &)> in_neighbour_filter = nullptr
);
```

PathTraversal 的三种遍历方式与 FrontierTraversal 类似，只是用户自定义的过滤函数中增加了两个参数，其中：Path 包含了到新扩展的这条边之前的路径，IteratorHelper 可用于将路径中的点/边转为数据库中对应的迭代器，相关文档可参考对应的 C++ API 文档。
