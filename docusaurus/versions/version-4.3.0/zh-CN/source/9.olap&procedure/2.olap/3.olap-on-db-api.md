# OlapOnDB API

> 此文档主要详细介绍了OlapOnDB API的使用说明


## 1. 简介

一般用户需要自己实现的只是将需要分析的子图抽取出来的过程。用户也可以通过使用TuGraph中丰富的辅助接口实现自己的图分析算法。

该文档主要介绍Procedure及Embed的接口设计，并介绍部分常用接口，具体的接口信息参见include/lgraph/olap_on_db.h文件。

## 2. 模型

Procedure及Embed使用到的辅助函数主要包含在OlapOnDB类，还有一些使用频率较高的函数都会逐一介绍

在TuGraph中OLAP相关的有以下常用的数据结构：

1. DB图分析类 `OlapOnDB<EdgeData>`
2. 点数组`ParallelVector<VertexData>`
3. 点集合`ParallelBitset`
4. 边数据结构`AdjUnit/AdjUnit<Empty>`
5. 边集合数据结构`AdjList<EdgeData>`

### 2.1 基于快照的存储结构

TuGraph中的OlapOnDB类能够提供数据“快照”，即建立一个对指定数据集的完全可用拷贝，该拷贝包括相应数据在某个时间点（拷贝开始的时间点）的镜像。由于OLAP的操作仅涉及读操作而不涉及写操作，OlapOnDB会以一种更紧凑的方式对数据进行排布，在节省空间的同时，提高数据访问的局部性。

### 2.2 BSP计算模型

TuGraph在计算的过程中使用了BSP（Bulk Synchronous Parallel）模型，使得该过程能够并行执行，极大的提高了程序运行效率。

BSP计算模型的核心思路为超步（Super Step）的提出和使用。在OlapOnDB创建后，在该数据上的计算分为多个超步，比如PageRank，分为多轮迭代，每轮迭代就是一个超步。不同超步之间用存在显式同步，从而保证所有线程在完成同一超步后同时进入下一个超步。在一个超步内部，所有的线程异步执行，利用并行提升计算效率。

利用BSP计算模型能够有效避免死锁，通过障碍同步的方式能够以硬件方式实现粗粒度的全局同步，使得图计算能够并行化执行，而程序员无需在同步互斥上大费周章。

## 3. 算法举例

在这里对PageRank算法分块做解释，大体上分为主函数`Process`和PageRank算法流程`PageRank`函数

### 3.1 主函数

主函数输入有三个参数，`TuGraph`数据库参数`db`，从网页端获取的请求`request`，给网页端的返回值`response`，整体流程可以分为一下几步：

1. 相关参数的获取
2. 快照类的创建
3. PageRank算法主流程
4. 网页端返回值的获取和发送

```C++
extern "C" bool Process(GraphDB & db, const std::string & request, std::string & response) {
    
    // 从网页端请求中获取迭代次数（num_iterations），
    int num_iterations = 20;
    try {
        json input = json::parse(request);
        num_iterations = input["num_iterations"].get<int>();
    } catch (std::exception & e) {
        throw std::runtime_error("json parse error");
        return false;
    }

    // 读事务的创建以及快照类的创建
    auto txn = db.CreateReadTxn();
    OlapOnDB<Empty> olapondb(
        db,
        txn,
        SNAPSHOT_PARALLEL
    );
	
    // 创建pr数组用于存储每个节点的pr值
    ParallelVector<double> pr = olapondb.AllocVertexArray<double>();
    // pagerank算法主流程，获取每个节点的pagerank值
    PageRankCore(olapondb, num_iterations, pr);
    
    auto all_vertices = olapondb.AllocVertexSubset();
    all_vertices.Fill();
    /*
        函数用途：从所有节点中获取pagerank值最大的节点编号
    
        函数流程描述：该函数对点集合all_vertices中所有为1的位对应的节点vi（又称为活跃点）执行Func A，再将Func A的返回值作为Func B的第二个输入参数，得到局部最大值（因为第一个输入参数为0，因此实际上返回值就是每个节点的pagerank值），最后再将所有线程的返回值汇总，再次 执行Func B得到全局返回值，并存入max_pr_vi变量中
    */
    size_t max_pr_vi = olapondb.ProcessVertexActive<size_t>(
        
        //Func A
        [&](size_t vi) {
            return vi;
        },
        all_vertices,
        0,
        
        //Func B
        [&](size_t a, size_t b) {
            return pr[a] > pr[b] ? a : b;
        }
    );
    
    // 网页端返回值的获取和发送
    json output;
    output["max_pr_vid"] = olapondb.OriginalVid(max_pr_vi);
    output["max_pr_val"] = pr[max_pr_vi];
    response = output.dump();
    return true;
}
```

### 3.2 PageRank算法流程

`pagerank`主流程有两个输入参数，快照类（子图）还有迭代次数，整体流程可以分为以下几步：

1. 相关数据结构的初始化
1. 每个节点pagerank值的初始化
1. 每个节点pagerank值的计算，活跃点为所有点（意味着所有点都需要计算pagerank值）
1. 得到每个节点经过`num_iterations`次迭代后的pagerank值

```C++
void PageRankCore(OlapBase<Empty>& graph, int num_iterations, ParallelVector<double>& curr) {
    
    // 相关数据结构的初始化
    auto all_vertices = olapondb.AllocVertexSubset();
    all_vertices.Fill();
    auto curr = olapondb.AllocVertexArray<double>();
    auto next = olapondb.AllocVertexArray<double>();
    size_t num_vertices = olapondb.NumVertices();
    double one_over_n = (double)1 / num_vertices;

    // 每个节点pagerank值的初始化，和该节点的出度成反比
    double delta = graph.ProcessVertexActive<double>(
        [&](size_t vi) {
            curr[vi] = one_over_n;
            if (olapondb.OutDegree(vi) > 0) {
                curr[vi] /= olapondb.OutDegree(vi);
            }
            return one_over_n;
        },
        all_vertices);

    // 总迭代过程
    double d = (double)0.85;
        for (int ii = 0;ii < num_iterations;ii ++) {
        printf("delta(%d)=%lf\n", ii, delta);
        next.Fill((double)0);

        /*
            函数用途：计算所有节点的pagerank值

            函数流程描述：该函数用于计算所有节点的pagerank值，对all_vertices中所有为1的位对应的节点vi执行Func C，得到本轮迭代中vi的pagerank值，并返回vi节点的pagerank变化值，最终经过函数内部处理汇总所有活跃节点的总变化值并返回，该值被存储在delta变量中
        */
        delta = graph.ProcessVertexActive<double>(
            // Func C
            [&](size_t vi) {
                double sum = 0;

                // 从邻居中获取当前节点的pagerank值
                for (auto & edge : olapondb.InEdges(vi)) {
                    size_t src = edge.neighbour;
                    sum += curr[src];
                }
                next[vi] = sum;

                // pagerank值计算核心公式
                next[vi] = (1 - d) * one_over_n + d * next[vi];
                if (ii == num_iterations - 1) {
                    return (double)0;
                } else {
    
                    // 相关中间变量统计
                    if (olapondb.OutDegree(vi) > 0) {
                        next[vi] /= olapondb.OutDegree(vi);
                        return fabs(next[vi] - curr[vi]) * olapondb.OutDegree(vi);
                    } else {
                        return fabs(next[vi] - curr[vi]);
                    }
                }
            },
            all_vertices
        );

        // 将本轮迭代得到的pagerank值输出作为下一轮迭代的输入
        curr.Swap(next);
    }
}
```

## 4. 其他常用函数功能描述

### 4.1 事务的创建

```C++
//读事务的创建
auto txn = db.CreateReadTxn();

//写事务的创建
auto txn = db.CreateWriteTxn();
```

### 4.2 并行化创建有向图

```C++
OlapOnDB<Empty> olapondb(
    db,
    txn,
    SNAPSHOT_PARALLEL
)
```

### 4.3 并行化创建无向图

```C++
OlapOnDB<Empty> olapondb(
    db,
    txn,
    SNAPSHOT_PARALLEL | SNAPSHOT_UNDIRECTED
)
```

### 4.4 获取出度

```C++
size_t OutDegree(size_t vid)
```

### 4.5 获取入度

```C++
size_t InDegree(size_t vid)
```

### 4.6 获取出边集合

```C++
/*
    函数名称：AdjList<EdgeData> OutEdges(size_t vid)
    数据结构:
        AdjList 可以理解为类型为AdjUnit结构体的数组
        AdjUnit 有两个成员变量： 1. size_t neighbour 2. edge_data，其中neighbour表示该出边指向的目标节点编号，如果为有权图，则edge_data数据类型和输入文件中边的权重值相同，否则数据类型为Empty

    使用示例：输出节点vid的所有出度邻居
*/
for (auto & edge : olapondb.OutEdges(vid)) {
    size_t dst = edge.neighbour;
    printf("src = %lu,dst = %lu\n",vid,dst);
}
```

### 4.7 获取入边集合

```C++
AdjList<EdgeData> InEdges(size_t vid)

// 使用示例：输出节点vid的所有入度邻居
for (auto & edge : olapondb.InEdges(vid)) {
    size_t dst = edge.neighbour;
    printf("src = %lu,dst = %lu\n",vid,dst);
}
```

### 4.8 获取TuGraph中节点对应OlapOnDB的节点编号

```C++
size_t OriginalVid(size_t vid)

// 备注： TuGraph中输入的节点编号可以是非数字，比如人名等，在生成OlapOnDB子图的时候，会将人名等转化为数字进行后续处理，因此该方法可能不适用于某些特定场景
```

### 4.9 获取OlapOnDB中节点对应TuGraph的节点编号

```C++
size_t MappedVid(size_t original_vid)
```

### 4.10 活跃点的描述

活跃点指的是在批处理函数中需要处理的点，在本例子中只是输出了活跃点的编号，并且汇总活跃点的数量：

```C++
ParallelBitset temp = 000111;	//当前活跃点为3，4，5号点

size_t delta = ForEachActiveVertex<double>(
    //void c
    [&](size_t vi) {
        printf("active_vertexId = %lu\n",vi);
        return 1;
    },
    all_vertices
);
```

函数的运行结果显而易见，因为多线程的关系，一下输出顺序可能有所变化：

```
active_vertexId = 3
active_vertexId = 4
active_vertexId = 5
```

局部返回值均为1，该函数会在保证线程安全的情况下将所有的局部返回值累加得到最终的返回值，并存储在`delta`变量中，该值最终为3
