# GeminiLite Tutorial

### 1 图计算简介

图（graph）是一种用于表示对象之间联结关系的抽象数据结构，通常用G=(V,E)的形式表示，V和E分别表示顶点和边的集合：顶点表示对象，边表示对象之间的关系。而对于可抽象成用图表示的数据，我们通常称之为图状结构数据，或简称图数据。例如在社交网络中，可以把每个人看做一个顶点，把人与人之间的好友关系看做边，这样社交网络中的所有人构成了点集V，所有的好友关系构成了边集E，生成的图数据G=（V，E）能够完整的表达出社交网络中的关联性，对社交网络的研究就可以抽象为在图数据上进行的操作。

图计算（graph computing）就是基于图数据的一种计算模式。相比于传统的个体数据分析，图计算能够深入挖掘个体之间的关联性，进而获得隐藏在数据背后的逻辑和结果。例如在上述的社交网络图数据中，我们可以对图数据进行，对图数据的聚类分析，可以挖掘出社交网络中实际存在的社区。图计算可以应用在多种多样的场景中，如反欺诈、马甲识别、黑/白名单用户筛选等。

### 2 GeminiLite简介

GeminiLite是由蚂蚁图平台团队开发的一款高性能图计算系统，包括了常用的pagerank、wcc（连通分量）、bfs（广度优先搜索）等工具。GeminiLite在性能上能够领先GraphX和GraphLab等其他图计算工具一到两个数量级别，占用内存资源是GraphX的1/20以下，能够轻松处理多达千亿级顶点的大规模图计算任务。

#### 2.1 基础算法介绍

#### bfs
Breadth-first search（广度优先搜索），从根顶点开始，沿着图的宽度遍历所有可访问顶点。返回结果为遍历顶点个数。
#### pagerank
pagerank（网页排序）算法，该算法根据图中边和边权值计算所有顶点的重要性排名。PageRank值越高，表示该顶点在图中的重要性越高。
#### sssp
single-source shortest path（单源最短路径），根据给定的源顶点，计算从该源顶点出发到其他任意顶点的最短路径长度。
#### wcc
weakly connected components（弱连通分量），该算法会计算图中所有的弱连通分量。弱连通分量是图的一个子图，子图中任意两点之间均存在可达路径。

#### 2.2 配置要求

GeminiLite需要在linux环境下运行，目前已在Ubuntu16.04.5和CentOS 7环境下正确运行。用户在官网下载了可执行程序后，就可以直接使用该程序进行相应的图计算了。

#### 2.3 图数据输入格式

在GeminiLite中，我们以从0开始的连续整数来代表图中的顶点，如果最大的输入顶点编号为v_max，那么图中的顶点编号为0到v_max的所有整数，共v_max+1个顶点。边则由起始顶点src、终点dst以及边权值edgeWeight组成，无权图的edgeWeight通常省略。

在GeminiLite中，图数据通常通过文本文件、二进制文件或odps导入，文件中需包含图中所有的边即可，这里我们将介绍文本文件的格式。在输入图文件中，我们将每条边作为一行输入，即每条边由src、dst和edgeWeight依次组成，数据之间由空格隔开，无权图中的edgeWeight可省略。src和dst为0到v_max之间的整数，而edgeWeight通常为整数或浮点数。输入文件每行格式示例：

```
0 1 0.1
```

该行数据表示一条从顶点0指向顶点1的边，边权值为0.1。

当图中的顶点个数较多时，通常图数据需要分成多个文件存储，因此我们在读取图数据时，以文件夹为单位，读取某文件夹下的所有文件。用户在使用时，需要先创建一个文件夹input_dir，然后将所有的输入图文件放到input_dir下，在运行程序时将input_dir作为参数输入即可读取图文件。在读取文件时，GeminiLite会读取input_dir下的所有文件，因此input_dir下只能包含输入图文件，不能包含其他文件。

#### 2.4 输入指令参数介绍

我们以bfs程序为例，介绍GeminiLite应用程序在输入文本文件和输入odps数据情况下的指令参数。

#### 2.4.1 输入文本文件
用户首先从官网下载bfs可执行程序，在bfs程序所在目录下执行

```
./bfs --type text -h
```

指令，即可获取bfs程序运行参数的详细信息，如下所示：

```
    --type              Input graph file source type, can be 
                        text/odps/binary.
    --input_dir         Directory where the input graph files
                        are stored. You should only put the
                        input graph files under the directory.
    --output_dir        output dir of result. Default="". 
    --vertices          Number of vertices in the input graph.
                        When value is set to 0, the program will
                        detect the number of vertices
                        automatically; when value is set to
                        nonzero integer, this integer should be
                        larger than the largest ID in the input
                        graph file. Default=0.
    --root              Root vertex the bfs starting from.
    -h, --help          Print this help message. Default=0.
```

其中，type、input_dir、output_dir、vertices和root参数是程序运行的5个参数，"-h, --help"表示帮助选项，带有Default选项的参数可以省略。必须输入的参数是type、input_dir和root，分别表示输入图数据的来源（包含text文本文件、odps以及binary二进制文件）、输入图数据和要进行bfs的起始顶点，输出文件可在output_dir参数后给出。

#### 2.4.2 输入ODPS源

用户首先从官网下载bfs可执行程序，在bfs程序所在目录下执行

```
./bfs --type odps -h
```

指令，即可获取bfs程序运行参数的详细信息，如下所示：

```
    --type              Input graph file source type, can be 
                        text/odps/binary.
    --vertices          Number of vertices in the input graph.
                        When value is set to 0, the program will
                        detect the number of vertices
                        automatically; when value is set to
                        nonzero integer, this integer should be
                        larger than the largest ID in the input
                        graph file. Default=0.
    --input_project     odps project to download the graph edgelist. 
                        Default=alipaygdb_dev. 
    --input_table       table name of graph edgelist. 
                        Default=changqin_livejournal_unweighted. 
    --input_cols        graph table column names. Default=src,dst. 
    --output_project    odps project to upload the result. 
                        Default=alipaygdb_dev. 
    --output_table      table name of result. Default="". 
    --access_id         access_id of personal account.
    --access_key        access_key of personal account.
    --access_encrypted  is odps access info encrypted, nonzero value means 
                        encrypted. Default=0. 
    --root              the root of bfs. Default=0. 
    -h, --help          Print this help message. Default=0.
```

其中，type、vertices、root、help参数的含义和输入文本文件时相同，input_project、input_table、input_cols表示输入数据来源为odps时的项目名称、表名称以及起点终点对应的字段名，需要根据需要自行给出；output_project和output_table为输出数据的项目名称和表名，需要根据需求自行给出。access_id、access_key和access_encrypted表示个人的账号配置。

#### 2.5 输出

GeminiLite的程序输出有两种：直接输出到屏幕上或输出到文件。当需要输出到文件时，需要用户在运行程序时提供输出文件夹,并通过--output_dir参数给出。

#### 3 运行过程及结果示例

此处介绍输入数据为文本文件时的bfs程序。

bfs程序的输入为无权图，所以边数据只需要包含起始顶点src和终点dst即可。上图所对应的输入文件内容为：

```
0 2
1 4
2 3
2 5
3 5
```

在bfs程序对应的目录下创建文件夹bfs_dir，在bfs_dir下创建文件bfs_file，内容为上述5行图数据。然后执行
```leaf
./bfs --type text --input_dir bfs_dir/ --root 0
```
命令，即可运行bfs程序，对上图从顶点0开始进行bfs计算。程序输出信息如下所示：

```
set |V| to 0
|V| = 6 |E| = 5
edges loaded into memory
preprocessing used 0.11 seconds
active(0)=1
active(1)=1
active(2)=2
from root = 0, find 4 vertices
exec_time=0.057408(s)
```

前4行信息表示加载图数据的过程，表示图中有6个顶点，5条边。5-7行表示bfs的过程有3轮，分别是：

* active(0)=1表示第一轮发现1个顶点，即定顶点0；
* active(1)=1表示第二轮发现1个顶点，即顶点2；
* active(2)=2表示第三轮发现两个顶点，即顶点3和顶点5。

最后两行表示最终发现了4个顶点。exec_time表示执行时间，每次运行的执行时间会变化。

至此，通过GeminiLite对上图进行bfs运算的过程已经完成。