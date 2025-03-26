"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[77435],{1732:(e,n,t)=>{t.r(n),t.d(n,{assets:()=>l,contentTitle:()=>o,default:()=>h,frontMatter:()=>r,metadata:()=>s,toc:()=>d});var i=t(74848),a=t(28453);const r={},o="OlapOnDisk API",s={id:"developer-manual/interface/olap/olap-on-disk-api",title:"OlapOnDisk API",description:"This document mainly introduces the usage instructions of OlapOnDisk API in detail",source:"@site/versions/version-3.6.0/en-US/source/5.developer-manual/6.interface/2.olap/4.olap-on-disk-api.md",sourceDirName:"5.developer-manual/6.interface/2.olap",slug:"/developer-manual/interface/olap/olap-on-disk-api",permalink:"/tugraph-db/en-US/en/3.6.0/developer-manual/interface/olap/olap-on-disk-api",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:4,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"OlapOnDB API",permalink:"/tugraph-db/en-US/en/3.6.0/developer-manual/interface/olap/olap-on-db-api"},next:{title:"Python Olap API",permalink:"/tugraph-db/en-US/en/3.6.0/developer-manual/interface/olap/python-api"}},l={},d=[{value:"Table of contents",id:"table-of-contents",level:2},{value:"1.Introduction",id:"1introduction",level:3},{value:"2.Algorithm example",id:"2algorithm-example",level:3},{value:"2.1.Head file",id:"21head-file",level:4},{value:"2.2.Configuration class MyConfig",id:"22configuration-class-myconfig",level:4},{value:"2.3.main function",id:"23main-function",level:4},{value:"2.4.bfs algorithm process",id:"24bfs-algorithm-process",level:4},{value:"3.Description of other commonly used functions",id:"3description-of-other-commonly-used-functions",level:3},{value:"3.1.Graph load",id:"31graph-load",level:4},{value:"3.2.Graph write",id:"32graph-write",level:4},{value:"3.3.graph parse function",id:"33graph-parse-function",level:4}];function c(e){const n={a:"a",blockquote:"blockquote",code:"code",edgedata:"edgedata",h1:"h1",h2:"h2",h3:"h3",h4:"h4",header:"header",li:"li",ol:"ol",p:"p",pre:"pre",ul:"ul",...(0,a.R)(),...e.components};return(0,i.jsxs)(i.Fragment,{children:[(0,i.jsx)(n.header,{children:(0,i.jsx)(n.h1,{id:"olapondisk-api",children:"OlapOnDisk API"})}),"\n",(0,i.jsxs)(n.blockquote,{children:["\n",(0,i.jsx)(n.p,{children:"This document mainly introduces the usage instructions of OlapOnDisk API in detail"}),"\n"]}),"\n",(0,i.jsx)(n.h2,{id:"table-of-contents",children:"Table of contents"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:(0,i.jsx)(n.a,{href:"#1introduction",children:"1. Introduction"})}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.a,{href:"#2algorithm-example",children:"2. Algorithm Example"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:(0,i.jsx)(n.a,{href:"#21head-file",children:"2.1 header files"})}),"\n",(0,i.jsx)(n.li,{children:(0,i.jsx)(n.a,{href:"#22configuration-class-myconfig",children:"2.2 Configuration class MyConfig"})}),"\n",(0,i.jsx)(n.li,{children:(0,i.jsx)(n.a,{href:"#23main-function",children:"2.3 Main function"})}),"\n",(0,i.jsx)(n.li,{children:(0,i.jsx)(n.a,{href:"#24bfs-algorithm-process",children:"2.4 bfs algorithm flow"})}),"\n"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.a,{href:"#3description-of-other-commonly-used-functions",children:"3. Function description of other commonly used functions"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:(0,i.jsx)(n.a,{href:"#31graph-load",children:"3.1 Image Loading"})}),"\n",(0,i.jsx)(n.li,{children:(0,i.jsx)(n.a,{href:"#32graph-write",children:"3.2 Image writing"})}),"\n",(0,i.jsx)(n.li,{children:(0,i.jsx)(n.a,{href:"#33graph-parse-function",children:"3.3 Graph Analysis Function"})}),"\n"]}),"\n"]}),"\n"]}),"\n",(0,i.jsx)(n.h3,{id:"1introduction",children:"1.Introduction"}),"\n",(0,i.jsx)(n.p,{children:"The Standalone mode of TuGraph can be used to load graph data files, where the sources of graph data files can include text files, BINARY_FILE binary files, and ODPS sources. In this mode, TuGraph can quickly load multiple data sources into a graph, and then run iterative algorithms such as BFS, WCC, SSSP, etc. on the graph, and output the final result to the terminal."}),"\n",(0,i.jsx)(n.p,{children:"In TuGraph, the export and calculation process can be accelerated through parallel processing in memory, so as to achieve near real-time processing and analysis. Compared with traditional methods, it avoids the overhead of data export and storage, and can use compact Graph data structures achieve desirable performance for computation."}),"\n",(0,i.jsx)(n.p,{children:"TuGraph has built-in a large number of common graph analysis algorithms and rich auxiliary interfaces, so users hardly need to implement the specific graph calculation process by themselves. They only need to include the header file (.h) of the corresponding algorithm library when implementing their own stored procedures. To your own program, and link your own dynamic library files in the compilation phase."}),"\n",(0,i.jsx)(n.p,{children:"This document mainly introduces the common interfaces of Standalone, and the auxiliary functions used are mainly contained in the OlapOnDB class. At the same time, in order to help users understand and facilitate, the BFS algorithm is illustrated with examples."}),"\n",(0,i.jsx)(n.h3,{id:"2algorithm-example",children:"2.Algorithm example"}),"\n",(0,i.jsxs)(n.p,{children:["Here, the BFS algorithm is explained in blocks, which are roughly divided into the main function ",(0,i.jsx)(n.code,{children:"main"}),", the BFS algorithm process ",(0,i.jsx)(n.code,{children:"BFSCore"})," function and the configuration class MyConfig."]}),"\n",(0,i.jsx)(n.h4,{id:"21head-file",children:"2.1.Head file"}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-C++",children:'#include "olap/olap_on_disk.h"   \n#include "tools/json.hpp"      //Header files to include when using TuGraph\n#include "./algo.h"   //A header file containing various algorithmic logic functions\n'})}),"\n",(0,i.jsx)(n.p,{children:"When using TuGraph to realize the calculation application of graph data files, generally, the StandaloneGraph class object graph is first created, the graph file data is loaded into the graph, and then the graph calculation process is realized by calling the graph logic function, and finally the result of the graph calculation is printed out."}),"\n",(0,i.jsx)(n.h4,{id:"22configuration-class-myconfig",children:"2.2.Configuration class MyConfig"}),"\n",(0,i.jsxs)(n.p,{children:["The MyConfig configuration class function is used to provide the configuration information required for the algorithm logic calculation, inherited from ConfigBase",(0,i.jsx)(n.edgedata,{children:", where EdgeDate can choose Empty (unweighted graph), int (the weight of the weighted graph is an integer) or double (the weight of the weighted graph is double) type."})]}),"\n",(0,i.jsx)(n.p,{children:"The MyConfig configuration class generally depends on the algorithm, and additional configuration information is required as follows:"}),"\n",(0,i.jsxs)(n.ol,{children:["\n",(0,i.jsx)(n.li,{children:"Parameters required by the algorithm"}),"\n",(0,i.jsx)(n.li,{children:"Algorithm name"}),"\n",(0,i.jsx)(n.li,{children:"Configure the Print function in the class\nOther common members inherit from ConfigBase, please refer to src/olap/olap_config.h for reference."}),"\n"]}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-C++",children:'class MyConfig : public ConfigBase<Empty> {\n public:\n\n    // The parameters required by the algorithm are initialized\n    size_t root = 0;\n    std::string name = std::string("bfs");\n    void AddParameter(fma_common::Configuration & config) {\n        ConfigBase<Empty>::AddParameter(config);\n        config.Add(root, "root", true)\n                .Comment("the root of bfs");\n    }\n    void Print() {\n        ConfigBase<Empty>::Print();\n        std::cout << "  name: " << name << std::endl;\n        if (root != size_t(-1)) {\n            std::cout << "  root: " << root << std::endl;\n        } else {\n            std::cout << "  root: UNSET" << std::endl;\n        }\n    }\n    // The configuration file accepts command line parameters. This use case will sequentially read the parameters when calling the algorithm from the command line. The value specified by the user is preferred. If the user does not specify it, the default parameter is selected.\n    MyConfig(int &argc, char** &argv): ConfigBase<Empty>(argc, argv) {\n        fma_common::Configuration config;\n        AddParameter(config);\n        config.ExitAfterHelp(true);\n        config.ParseAndFinalize(argc, argv);\n        Print();\n    }\n};\n'})}),"\n",(0,i.jsx)(n.h4,{id:"23main-function",children:"2.3.main function"}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-C++",children:'int main(int argc, char** argv) {\n    double start_time;\n    // Statistical memory consumption class MemUsage instantiation\n    MemUsage memUsage;\n    memUsage.startMemRecord();\n\n    // prepare\n    start_time = get_time();\n    // Configuration class MyConfig instantiation\n    MyConfig config(argc, argv);\n    size_t root_vid = config.root;\n    // OlapOnDisk class instantiation\n    OlapOnDisk<Empty> graph;\n    graph.Load(config, DUAL_DIRECTION);\n    memUsage.print();\n    memUsage.reset();\n    // Statistical graph loading time consumption\n    auto prepare_cost = get_time() - start_time;\n    printf("prepare_cost = %.2lf(s)\\n", prepare_cost);\n\n    // core\n    start_time = get_time();\n    // Create an array to count whether a node has been traversed\n    auto parent = graph.AllocVertexArray<size_t>();\n    // Breadth-first search algorithm, returns the number of nodes connected to the root_vid root node in the graph\n    size_t count = BFSCore(graph, root_vid, parent);\n    memUsage.print();\n    memUsage.reset();\n    auto core_cost = get_time() - start_time;\n    printf("core_cost = %.2lf(s)\\n", core_cost);\n\n    // output\n    start_time = get_time();\n    // Print relevant information to the terminal\n    printf("found_vertices = %ld\\n", count);\n    auto output_cost = get_time() - start_time;\n    printf("output_cost = %.2lf(s)\\n", output_cost);\n\n    printf("total_cost = %.2lf(s)\\n", prepare_cost + core_cost + output_cost);\n    printf("DONE.");\n\n    return 0;\n}\n'})}),"\n",(0,i.jsx)(n.h4,{id:"24bfs-algorithm-process",children:"2.4.bfs algorithm process"}),"\n",(0,i.jsxs)(n.p,{children:["The main process of ",(0,i.jsx)(n.code,{children:"bfs"})," has two input parameters, the snapshot class (subgraph) and the number of iterations. The overall process can be divided into the following steps:"]}),"\n",(0,i.jsxs)(n.ol,{children:["\n",(0,i.jsx)(n.li,{children:"Relevant definitions and initialization of data structures"}),"\n",(0,i.jsx)(n.li,{children:"Use the batch function to perform cyclic calculations on each node, find all nodes adjacent to the current node in each round, and exchange them when the round ends."}),"\n",(0,i.jsx)(n.li,{children:"Until all nodes are found, return the number of nodes discovered_vertices."}),"\n"]}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-C++",children:'size_t BFSCore(Graph<Empty>& graph, size_t root_vid, ParallelVector<size_t>& parent){\n\n  size_t root = root_vid;\n  auto active_in = graph.AllocVertexSubset();   //Allocate an array, active_in is used to store the nodes found in the previous cycle stage\n  active_in.Add(root);            //Add the root node to the array\n  auto active_out = graph.AllocVertexSubset();  //Allocate the array active_out to store the nodes found in the current cycle stage\n  parent.Fill((size_t)-1);               //Assign a value of -1 to the node in the parent array, -1 means not found\n  parent[root] = root;\n  size_t num_activations = 1;       //Indicates the number of nodes found in the current loop phase\n  size_t discovered_vertices = 0;    //Indicates the total number of nodes found in the current cycle phase\n\n  for (int ii = 0; num_activations != 0; ii++) {       //num_activations indicates the number of nodes found in the current loop phase\n      printf("activates(%d) <= %lu\\n", ii, num_activations);\n      discovered_vertices += num_activations;         //discovered_vertices indicates the total number of nodes found in the current cycle phase\n      active_out.Clear();\n      num_activations = graph.ProcessVertexActive<size_t>(\n          [&](size_t vi) {\n              size_t num_activations = 0;\n              for (auto& edge : graph.OutEdges(vi)) {   //Each cycle starts from the root node, finds adjacent adjacent nodes, changes its parent value, and operates num_activations+1\n                  size_t dst = edge.neighbour;\n                  if (parent[dst] == (size_t)-1) {\n                      auto lock = graph.GuardVertexLock(dst);\n                      if (parent[dst] == (size_t)-1) {\n                          parent[dst] = vi;\n                          num_activations += 1;\n                          active_out.Add(dst);       //Store the nodes found in the current loop phase\n                      }\n                  }\n              }\n              return num_activations;\n          },\n          active_in);\n      active_in.Swap(active_out);\n  }\n  // return all nodes\n  return discovered_vertices;\n}\n'})}),"\n",(0,i.jsx)(n.h3,{id:"3description-of-other-commonly-used-functions",children:"3.Description of other commonly used functions"}),"\n",(0,i.jsx)(n.h4,{id:"31graph-load",children:"3.1.Graph load"}),"\n",(0,i.jsx)(n.p,{children:"TuGraph-StandaloneThe loading sources of graph data files are mainly divided into three categories: text files, binary files, and ODPS. The binary file is a file in which the binary representation of the edge data is arranged in order, which can save a lot of storage space. Its loading function is divided into three types, namely:"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"void Load(ConfigBase<EdgeData> config,EdgeDirectionPolicy edge_direction_policy = DUAL_DIRECTION)"}),"\uff1aThe loading method of the graph data file contains two parameters, and their meanings represent respectively"]}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"config"}),"\uff1aConfiguration parameters to load. This parameter saves the general information of the graph (such as data source, algorithm name, data input and output paths, number of vertices, etc.) and different information parameters configured according to different data sources and different algorithms."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"edge_direction_policy"}),"\uff1aSpecifies whether the graph is directed or undirected, including three modes: DUAL_DIRECTION, MAKE_SYMMETRIC, and INPUT_SYMMETRIC. Among them, DUAL_DIRECTION is the default graph loading method.\nDUAL_DIRECTION : The input file is an asymmetric graph and the loaded graph is an asymmetric graph.\nMAKE_SYMMETRIC : The input file is an asymmetric graph and the loaded graph is a symmetric graph.\nINPUT_SYMMETRIC : The input file is a symmetric graph and the loaded graph is a symmetric graph.\nFor details, see ",(0,i.jsx)(n.code,{children:"enum EdgeDirectionPolicy"})," in the olap_config.h file under the lgraph folder."]}),"\n"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"void LoadVertexArrayTxt<V>(V * array, std::string path, std::function<size_t(const char *, const char *, VertexUnit<V> &)> parse_line)"}),"\uff1aLoad the vertices in the file into an array in the order of their ids. The meanings of each parameter are:"]}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"array"}),"\uff1aarray of data to be read"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"path"}),"\uff1aThe path to read the file, each line in the file represents a pair of vertex"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"parse_line"}),"\uff1aA user-defined function that tells the system how to parse a line of text data into a vertex pair."]}),"\n"]}),"\n"]}),"\n"]}),"\n",(0,i.jsx)(n.h4,{id:"32graph-write",children:"3.2.Graph write"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"void Write(ConfigBase<EdgeData> & config, ParallelVector<VertexData>& array, size_t array_size, std::string name, std::function<bool(VertexData &)> filter_output = filter_output_default<VertexData&>)"}),"\uff1aWrite the data in the array back to the file, and the meanings of each parameter are:\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"config"}),"\uff1aConfiguration parameters to load. This parameter saves the general information of the graph (such as data source, algorithm name, data input and output paths, number of vertices, etc.) and different information parameters configured according to different data sources and different algorithms."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"array"}),"\uff1aarray of data to be written"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"array_size"}),"\uff1aThe length of the number of data to be written"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"name"}),"\uff1aalgorithm name"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"filter_output"}),"\uff1aWrite data rule function, the data to be written needs to meet the requirements of this function."]}),"\n"]}),"\n"]}),"\n"]}),"\n",(0,i.jsx)(n.h4,{id:"33graph-parse-function",children:"3.3.graph parse function"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"std::tuple<size_t, bool> parse_line_unweighted(const char *p, const char *end, EdgeUnit<EdgeData> &e)"}),"\uff1aParse the graph data file, and load the graph as an unweighted graph."]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"std::tuple<size_t, bool> parse_line_weighted(const char* p, const char* end, EdgeUnit<EdgeData>& e)"}),"\uff1aParse the graph data file, load the graph as a weighted graph, and specify the weight data type by modifying ",(0,i.jsx)(n.edgedata,{children:"."})]}),"\n"]}),"\n"]}),"\n",(0,i.jsx)(n.p,{children:"This function can be specified through the constructor parse_line when the MyConfig class is defined."})]})}function h(e={}){const{wrapper:n}={...(0,a.R)(),...e.components};return n?(0,i.jsx)(n,{...e,children:(0,i.jsx)(c,{...e})}):c(e)}},28453:(e,n,t)=>{t.d(n,{R:()=>o,x:()=>s});var i=t(96540);const a={},r=i.createContext(a);function o(e){const n=i.useContext(r);return i.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function s(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(a):e.components||a:o(e.components),i.createElement(r.Provider,{value:n},e.children)}}}]);