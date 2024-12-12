# OlapOnDisk API

> 此文档主要详细介绍了OlapOnDisk API的使用说明

    
## 1. 简介

TuGraph的Standalone模式可用于加载图数据文件，其中图数据文件来源可包含text文本文件、BINARY_FILE二进制文件和ODPS源。在该模式下，TuGraph可实现多数据来源快速加载成图，然后在该图上运行如BFS、WCC、SSSP等迭代式算法，并输出最终结果至终端。

在TuGraph中，导出和计算过程均可以通过在内存中并行处理的方式进行加速，从而达到近乎实时的处理分析，和传统方法相比，即避免了数据导出落盘的开销，又能使用紧凑的图数据结构获得计算的理想性能。

TuGraph内置了大量的常见图分析算法和丰富的辅助接口，因此用户几乎不需要自己实现具体的图计算过程，只需要在实现自己的存储过程的时候将相应算法库的头文件(.h)包含到自己的程序中，并在编译阶段链接自己的动态库文件即可。

该文档主要介绍了Standalone的常用接口，使用到的辅助函数主要包含在OlapOnDB类。同时为帮助用户理解方便，对BFS算法进行举例说明。

## 2. 算法举例

在这里对BFS算法分块做解释，大体上分为主函数`main`、BFS算法流程`BFSCore`函数和配置类MyConfig。

### 2.1 头文件

```C++
#include "olap/olap_on_disk.h"   
#include "tools/json.hpp"      //使用 TuGraph 时需要包含的头文件
#include "./algo.h"   //包含各种算法逻辑函数的头文件
```

在使用 TuGraph 实现图数据文件计算应用时，一般首先建立StandaloneGraph类对象graph，将图文件数据加载进graph中，之后通过调用图逻辑函数实现图计算过程，最后对图计算的结果进行打印输出。

### 2.2 配置类MyConfig

MyConfig配置类函数用于提供算法逻辑计算时所需的配置信息，继承于ConfigBase<EdgeData>,其中EdgeDate可根据加载图类型不同选择Empty（无权图）、int（带权图权重为整数）或者double（带权图权重为double）类型。

MyConfig配置类一般根据算法不同，需要额外配置信息如下：

1.算法所需参数
2.算法名称
3.配置类内Print函数
其余公用成员继承与ConfigBase，可参考src/olap/olap_config.h查阅。

```C++
class MyConfig : public ConfigBase<Empty> {
 public:

    // 算法所需参数初始化
    size_t root = 0;
    std::string name = std::string("bfs");
    void AddParameter(fma_common::Configuration & config) {
        ConfigBase<Empty>::AddParameter(config);
        config.Add(root, "root", true)
                .Comment("the root of bfs");
    }
    void Print() {
        ConfigBase<Empty>::Print();
        std::cout << "  name: " << name << std::endl;
        if (root != size_t(-1)) {
            std::cout << "  root: " << root << std::endl;
        } else {
            std::cout << "  root: UNSET" << std::endl;
        }
    }
    // 配置文件接受命令行参数，该用例会顺次读取命令行调用算法时的参数，优先使用用户指定数值，若用户并未指定则选择默认参数。
    MyConfig(int &argc, char** &argv): ConfigBase<Empty>(argc, argv) {
        fma_common::Configuration config;
        AddParameter(config);
        config.ExitAfterHelp(true);
        config.ParseAndFinalize(argc, argv);
        Print();
    }
};
```

### 2.3 主函数

```C++
int main(int argc, char** argv) {
    double start_time;
    // 统计内存消耗类MemUsage实例化
    MemUsage memUsage;
    memUsage.startMemRecord();

    // prepare
    start_time = get_time();
    // 配置类MyConfig实例化
    MyConfig config(argc, argv);
    size_t root_vid = config.root;
    // OlapOnDisk类实例化
    OlapOnDisk<Empty> graph;
    graph.Load(config, DUAL_DIRECTION);
    memUsage.print();
    memUsage.reset();
    // 统计图加载消耗时间
    auto prepare_cost = get_time() - start_time;
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);

    // core
    start_time = get_time();
    // 创建数组用于统计某节点是否遍历过
    auto parent = graph.AllocVertexArray<size_t>();
    // 宽度优先搜索算法，返回图内root_vid根结点连接的节点个数
    size_t count = BFSCore(graph, root_vid, parent);
    memUsage.print();
    memUsage.reset();
    auto core_cost = get_time() - start_time;
    printf("core_cost = %.2lf(s)\n", core_cost);

    // output
    start_time = get_time();
    // 打印相关信息至终端
    printf("found_vertices = %ld\n", count);
    auto output_cost = get_time() - start_time;
    printf("output_cost = %.2lf(s)\n", output_cost);

    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("DONE.");

    return 0;
}
```

### 2.4 bfs算法流程

`bfs`主流程有两个输入参数，快照类（子图）还有迭代次数，整体流程可以分为以下几步：

1. 相关定义、数据结构的初始化
2. 使用批处理函数对每个节点进行循环计算，每一轮找到与当前节点相邻的全部节点，并在该轮次终止时进行交换。
3. 直到找到全部节点，返回节点个数discovered_vertices。

```C++
size_t BFSCore(Graph<Empty>& graph, size_t root_vid, ParallelVector<size_t>& parent){

  size_t root = root_vid;
  auto active_in = graph.AllocVertexSubset();   //分配数组，active_in用于存放上一循环阶段已找到的节点
  active_in.Add(root);            //把跟节点加入数组中
  auto active_out = graph.AllocVertexSubset();  //分配数组active_out用于存放当前循环阶段找到的节点
  parent.Fill((size_t)-1);               //将parent数组中的节点赋值为-1，-1表示未被找到
  parent[root] = root;
  size_t num_activations = 1;       //表示当前循环阶段找到的节点个数
  size_t discovered_vertices = 0;    //表示当前循环阶段找到节点的总个数

  for (int ii = 0; num_activations != 0; ii++) {       //num_activations表示当前循环阶段找到的节点个数
      printf("activates(%d) <= %lu\n", ii, num_activations);
      discovered_vertices += num_activations;         //discovered_vertices表示当前循环阶段找到节点的总个数
      active_out.Clear();
      num_activations = graph.ProcessVertexActive<size_t>(
          [&](size_t vi) {
              size_t num_activations = 0;
              for (auto& edge : graph.OutEdges(vi)) {   //每一次循环从根节点出发，查找邻近的相邻节点，对其parent值改变，并num_activations+1操作
                  size_t dst = edge.neighbour;
                  if (parent[dst] == (size_t)-1) {
                      auto lock = graph.GuardVertexLock(dst);
                      if (parent[dst] == (size_t)-1) {
                          parent[dst] = vi;
                          num_activations += 1;
                          active_out.Add(dst);       //存放当前循环阶段找到的节点
                      }
                  }
              }
              return num_activations;
          },
          active_in);
      active_in.Swap(active_out);
  }
  // 返回全部节点数
  return discovered_vertices;
}
```

## 3. 其他常用函数功能描述

### 3.1 图加载

TuGraph-Standalone对于图数据文件的加载来源主要分为三大类：文本文件、二进制文件和ODPS。二进制文件为将边数据的二进制表示按顺序排列的文件，能够节省大量存储空间。其加载函数分为三种，分别是：

- `void Load(ConfigBase<EdgeData> config,EdgeDirectionPolicy edge_direction_policy = DUAL_DIRECTION)`：图数据文件的加载方式，包含两个参数，其含义分别表示：
  - `config`：需要加载的配置参数。该参数内保存了该图的一般信息（如数据来源，算法名称，数据输入、输出路径，点个数等）以及根据不同数据来源、不同算法所配置的不同信息参数。
  - `edge_direction_policy`：指定图为有向或无向，包含三种模式，分别为DUAL_DIRECTION、MAKE_SYMMETRIC以及INPUT_SYMMETRIC。其中DUAL_DIRECTION为默认的图加载方式。
  DUAL_DIRECTION : 输入文件为非对称图，加载图为非对称图。
  MAKE_SYMMETRIC : 输入文件为非对称图，加载图为对称图。
  INPUT_SYMMETRIC : 输入文件为对称图，加载图为对称图。
  对应的详细介绍见lgraph文件夹下的olap_config.h文件的`enum EdgeDirectionPolicy`。

- `void LoadVertexArrayTxt<V>(V * array, std::string path, std::function<size_t(const char *, const char *, VertexUnit<V> &)> parse_line)`：将文件中的点-数据对按照点id的顺序加载到数组中。各参数表示意义分别为：
  - `array`：待读入数据的数组
  - `path`：读取文件的路径，文件中每行表示一对点-数据对
  - `parse_line`：用户自定义函数，告诉系统如何将一行文本数据解析为一个点-数据对。


### 3.2 图写入
- `void Write(ConfigBase<EdgeData> & config, ParallelVector<VertexData>& array, size_t array_size, std::string name, std::function<bool(VertexData &)> filter_output = filter_output_default<VertexData&>)`：把array中数据写回文件中，各参数表示意义分别是：
  - `config`：需要加载的配置参数。该参数内保存了该图的一般信息（如数据来源，算法名称，数据输入、输出路径，点个数等）以及根据不同数据来源、不同算法所配置的不同信息参数。
  - `array`：待写入数据的数组
  - `array_size`：待写入数据的数字长度
  - `name`：算法名称
  - `filter_output`：写入数据规则函数，待写入数据需要满足该函数的要求。

### 3.3 图解析函数
- `std::tuple<size_t, bool> parse_line_unweighted(const char *p, const char *end, EdgeUnit<EdgeData> &e)`：对图数据文件进行解析，加载图为无权图。

- `std::tuple<size_t, bool> parse_line_weighted(const char* p, const char* end, EdgeUnit<EdgeData>& e)`：对图数据文件进行解析，加载图为有权图，权重数据类型可以通过修改<EdgeData>指定。

该函数可通过MyConfig类定义时的构造函数parse_line进行指定。
