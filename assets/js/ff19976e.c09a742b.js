"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[37030],{28453:(e,n,t)=>{t.d(n,{R:()=>a,x:()=>o});var i=t(96540);const r={},s=i.createContext(r);function a(e){const n=i.useContext(s);return i.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function o(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(r):e.components||r:a(e.components),i.createElement(s.Provider,{value:n},e.children)}},68597:(e,n,t)=>{t.r(n),t.d(n,{assets:()=>l,contentTitle:()=>a,default:()=>h,frontMatter:()=>s,metadata:()=>o,toc:()=>c});var i=t(74848),r=t(28453);const s={},a="Python Olap API",o={id:"developer-manual/interface/olap/python-api",title:"Python Olap API",description:"This document mainly introduces the API usage of OlapBase OlapOnDB and OlapOnDisk in Python",source:"@site/versions/version-3.6.0/en-US/source/5.developer-manual/6.interface/2.olap/5.python-api.md",sourceDirName:"5.developer-manual/6.interface/2.olap",slug:"/developer-manual/interface/olap/python-api",permalink:"/tugraph-db/en/3.6.0/developer-manual/interface/olap/python-api",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:5,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"OlapOnDisk API",permalink:"/tugraph-db/en/3.6.0/developer-manual/interface/olap/olap-on-disk-api"},next:{title:"TuGraph Built-in Algorithm Description",permalink:"/tugraph-db/en/3.6.0/developer-manual/interface/olap/algorithms"}},l={},c=[{value:"Table of contents",id:"table-of-contents",level:2},{value:"1 Overview",id:"1-overview",level:2},{value:"2. Configuration requirements",id:"2-configuration-requirements",level:2},{value:"3. Cython",id:"3-cython",level:2},{value:"4. Olap API",id:"4-olap-api",level:2},{value:"Atomic operations",id:"atomic-operations",level:3},{value:"Vertex collection class ParallelBitset",id:"vertex-collection-class-parallelbitset",level:3},{value:"Vertex array class ParallelVector",id:"vertex-array-class-parallelvector",level:3},{value:"Custom Data Structure",id:"custom-data-structure",level:3},{value:"Graph class OlapBase",id:"graph-class-olapbase",level:3},{value:"Graph class OlapOnDB:",id:"graph-class-olapondb",level:3},{value:"Graph class OlapOnDisk",id:"graph-class-olapondisk",level:3},{value:"ConfigBase:",id:"configbase",level:4},{value:"5. lgraph_db API",id:"5-lgraph_db-api",level:2},{value:"VertexIndexIterator",id:"vertexindexiterator",level:3},{value:"Galaxy",id:"galaxy",level:3},{value:"GraphDB:",id:"graphdb",level:3},{value:"Transaction:",id:"transaction",level:3},{value:"PyGalaxy:",id:"pygalaxy",level:3},{value:"PyGraphDB:",id:"pygraphdb",level:3},{value:"6. Algorithm plug-in example",id:"6-algorithm-plug-in-example",level:2}];function d(e){const n={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",h4:"h4",header:"header",li:"li",p:"p",pre:"pre",ul:"ul",...(0,r.R)(),...e.components};return(0,i.jsxs)(i.Fragment,{children:[(0,i.jsx)(n.header,{children:(0,i.jsx)(n.h1,{id:"python-olap-api",children:"Python Olap API"})}),"\n",(0,i.jsxs)(n.blockquote,{children:["\n",(0,i.jsx)(n.p,{children:"This document mainly introduces the API usage of OlapBase OlapOnDB and OlapOnDisk in Python"}),"\n"]}),"\n",(0,i.jsx)(n.h2,{id:"table-of-contents",children:"Table of contents"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:(0,i.jsx)(n.a,{href:"#1-overview",children:"1.Overview"})}),"\n",(0,i.jsx)(n.li,{children:(0,i.jsx)(n.a,{href:"#2-configuration-requirements",children:"2.Configuration Requirements"})}),"\n",(0,i.jsx)(n.li,{children:(0,i.jsx)(n.a,{href:"#3-cython",children:"3.Cython"})}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.a,{href:"#4-olap-api",children:"4.Olap API"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:(0,i.jsx)(n.a,{href:"#atomic-operations",children:"4.1. Atomic operations"})}),"\n",(0,i.jsx)(n.li,{children:(0,i.jsx)(n.a,{href:"#vertex-collection-class-parallelbitset",children:"4.2. Vertex collection class ParallelBitset"})}),"\n",(0,i.jsx)(n.li,{children:(0,i.jsx)(n.a,{href:"#vertex-array-class-parallelvector",children:"4.3. Vertex array class ParallelVector"})}),"\n",(0,i.jsx)(n.li,{children:(0,i.jsx)(n.a,{href:"#custom-data-structure",children:"4.4. Custom data structure"})}),"\n",(0,i.jsx)(n.li,{children:(0,i.jsx)(n.a,{href:"#graph-class-olapbase",children:"4.5.Graph class OlapBase"})}),"\n",(0,i.jsx)(n.li,{children:(0,i.jsx)(n.a,{href:"#graph-class-olapondb",children:"4.6. Graph Class OlapOnDB"})}),"\n",(0,i.jsx)(n.li,{children:(0,i.jsx)(n.a,{href:"#graph-class-olapondisk",children:"4.7.Graph class OlapOnDisk"})}),"\n"]}),"\n"]}),"\n",(0,i.jsx)(n.li,{children:(0,i.jsx)(n.a,{href:"#5-lgraph_db-api",children:"5.lgraph DB API"})}),"\n",(0,i.jsx)(n.li,{children:(0,i.jsx)(n.a,{href:"#6-algorithm-plug-in-example",children:"6.Algorithm plugin example"})}),"\n"]}),"\n",(0,i.jsx)(n.h2,{id:"1-overview",children:"1 Overview"}),"\n",(0,i.jsx)(n.p,{children:"This manual will introduce the simple configuration required to use the Python interface of the TuGraph graph computing system, and explain the TuGraph Python API in conjunction with the code. For details about the functions of class ParallelBitset, OlapBase, etc., see olap-base-api.md, olap-on-db-api.md and olap-on-disk-api.md."}),"\n",(0,i.jsx)(n.h2,{id:"2-configuration-requirements",children:"2. Configuration requirements"}),"\n",(0,i.jsx)(n.p,{children:"If you want to use TuGraph to write and compile your own applications, the required configuration requirements are:"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"Linux operating system, currently running successfully on Ubuntu16.04.2 and Centos7 systems."}),"\n",(0,i.jsx)(n.li,{children:"A compiler that supports C++17 requires GCC version 5.4.1 or later."}),"\n",(0,i.jsx)(n.li,{children:"Cython, version 3.0a1 or above is required, and the tested version is 3.0.0a11"}),"\n"]}),"\n",(0,i.jsx)(n.h2,{id:"3-cython",children:"3. Cython"}),"\n",(0,i.jsx)(n.p,{children:"Cython is an efficient programming language that is a superset of Python. Cython can translate .py files into C/C++ codes and compile them into Python extension modules, which can be called through import in Python. In TuGraph, all Python plugins are compiled into Python extension modules by Cython and then used."}),"\n",(0,i.jsx)(n.p,{children:"The main advantage of using Cython is that it combines the simplicity and ease of use of Python with the performance of C/C++. The TuGraph Python interface is implemented using Cython."}),"\n",(0,i.jsx)(n.p,{children:(0,i.jsx)(n.a,{href:"https://cython.readthedocs.io/en/latest/index.html",children:"Cython Documentation"})}),"\n",(0,i.jsx)(n.h2,{id:"4-olap-api",children:"4. Olap API"}),"\n",(0,i.jsxs)(n.p,{children:["The Olap API is used for graph computing and is implemented in C++. The usage and functions are basically the same as the C++ interface. To use the API in a Python file, the interfaces declared in plugins/cython/olap_base.pxd must be imported using ",(0,i.jsx)(n.code,{children:"cython.cimports.olap_base import *"}),". The Python file can only be run after being compiled by Cython."]}),"\n",(0,i.jsx)(n.h3,{id:"atomic-operations",children:"Atomic operations"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"cas[T](ptr: cython.pointer(T), oldv: T, newv: T)-> cython.bint"}),": If the value pointed to by ptr is equal to oldv, assign the value pointed to by ptr to newv and return True, otherwise return False."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"write_min[T](a: cython.pointer(T), b: T)->cython.bint"}),": If b is smaller than the value pointed to by a, then assign the value pointed to by a to b and return true, otherwise return false."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"write_max[T](a: cython.pointer(T), b: T)->cython.bint"}),": If b is greater than the value pointed to by a, then assign the value pointed to by a to b and return true, otherwise return false."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"write_add[T](a: cython.pointer(T), b: T)->cython.bint"}),": Add the value of b to the value pointed to by a."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"write_sub[T](a: cython.pointer(T), b: T)->cython.bint"}),": Subtract the value pointed to by a from the value of b."]}),"\n"]}),"\n",(0,i.jsx)(n.h3,{id:"vertex-collection-class-parallelbitset",children:"Vertex collection class ParallelBitset"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"Size() -> size_t"}),": Indicates the number of vertices in the Bitmap."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"ParallelBitset(size: size_t)"}),": Initializes size and data, where the length of data is (size >> 6) + 1."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"Clear() -> cython.void"}),": Empties the collection."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"Fill() -> cython.void"}),": Adds all vertices to the set."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"Has(i: size_t) -> cython.bint"}),": Checks if vertex i is in the set."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"Add(i: size_t) -> cython.bint"}),": Adds vertex i to the set."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"Swap(other: ParallelBitset) -> cython.void"}),": Exchanges elements with another ParallelBitset set."]}),"\n"]}),"\n",(0,i.jsx)(n.h3,{id:"vertex-array-class-parallelvector",children:"Vertex array class ParallelVector"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"ParallelVector[T](size_t capacity)"}),": Constructs a ParallelVector, where capacity is the initial capacity of the vertex array."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"operator[](i: size_t) -> T"}),": Returns the data at index i."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"begin() -> cython.pointer(T)"}),": Returns the start pointer of ParallelVector."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"end() -> cython.pointer(T)"}),": Returns the end pointer of ParallelVector. The usage of begin() and end() is similar to the begin and end pointers of the vector container, and these two pointers can be used to sequentially access the array."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"Back() -> T"}),": Returns the last data of ParallelVector."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"Data() -> cython.pointer(T)"}),": Indicates the data of the array itself."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"Destroy() -> cython.void"}),": Clears the data in the ParallelVector array and deletes the array."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"Size() -> size_t"}),": Indicates the number of data in ParallelVector."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"Resize(size: size_t) -> cython.void"}),": Changes ParallelVector to size, which should be greater than or equal to the size before the change and less than capacity."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"Clear() -> cython.void"}),": Clears the data in ParallelVector."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"ReAlloc(capacity: size_t) -> cython.void"}),": Allocates a new capacity size to ParallelVector. If the array has data, it migrates the data to the new memory."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"Fill(elem: T) -> cython.void"}),": Assigns elem to all data of ParallelVector."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"Append(elem: T, atomic: cython.bint = true) -> cython.void"}),": Adds a data to the end of ParallelVector."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"Swap(other: ParallelVector[T]) -> cython.void"}),": Exchanges data with another ParallelVector."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"Copy() -> ParallelVector[T]"}),": Copies the current ParallelVector data and stores it in the copy array."]}),"\n"]}),"\n",(0,i.jsx)(n.h3,{id:"custom-data-structure",children:"Custom Data Structure"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"Empty"}),": A special data type whose content is empty."]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"EdgeUnit[EdgeData]"}),": Indicates the edge whose weight type is EdgeData, used to parse the input file, including three member variables:\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"src: size_t"}),": starting vertex of the edge"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"dst: size_t"}),": end of the edge"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"edge_data: EdgeData"}),": edge weight"]}),"\n"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"AdjUnit[EdgeData]"}),": indicates the edge whose weight type is EdgeData, which is used in the batch calculation process and contains two member variables:\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"neighbor: size_t"}),": neighbor vertex of the edge"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"edge_data: EdgeData"}),": edge weight"]}),"\n"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"AdjList[EdgeData]"}),": The adjacency list of vertices whose weight type is EdgeData, often used to represent the set of incoming and outgoing edges of vertices, including three member functions:\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"begin()->cython.pointer(AdjUnit[T])"}),": the starting pointer of the list"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"end()->cython.pointer(AdjUnit[T])"}),": the end pointer of the list"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"operator[](i: size_t)-> AdjUnit[EdgeData]"}),": the data whose subscript is i"]}),"\n"]}),"\n"]}),"\n"]}),"\n",(0,i.jsx)(n.h3,{id:"graph-class-olapbase",children:"Graph class OlapBase"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"NumVertices()->size_t"}),": get the number of vertices"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"NumEdges()-> size_t"}),": get the number of edges"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"OutDegree(vid: size_t)-> size_t"}),": out-degree of vertex vid"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"InDegree(vid: size_t)->size_t"}),": in-degree of the vertex vid"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"AllocVertexArray[VertexData]() ->ParallelVector[VertexData]"}),": Allocates an array of type VertexData with size as the number of vertices"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"AllocVertexSubset()-> ParallelBitset"}),": Assigns a subset of ParallelBitsets to denote whether the state of all vertices is activated"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"OutEdges(vid: size_t)-> AdjList[EdgeData]"}),": Get the list of all outgoing edges of vertex v"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"InEdges(vid: size_t)-> AdjList[EdgeData]"}),": Get the list of all incoming edges of vertex v"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"Transpose()->cython.void"}),": transpose of a directed graph"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"LoadFromArray(edge_array: cython.p_char, input_vertices: size_t, input_edges: size_t, edge_direction_policy: EdgeDirectionPolicy)"}),": Loads the graph data from the array, contains four parameters, the meaning of which are respectively:"]}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"edge_array"})," : reads the data from the array into the graph. Normally, the array contains multiple edges."]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"input_vertices"}),": specifies the number of vertices read into the graph by the array."]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"input_edges"})," : specifies the number of edges that the array reads into the image."]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"edge_direction_policy"})," : indicates that the graph is directed or undirected. The graph can be divided into three modes: DUAL_DIRECTION, MAKE_SYMMETRIC, and INPUT_SYMMETRIC. For details, see 'enum EdgeDirectionPolicy' in the config.h file in the core folder."]}),"\n"]}),"\n"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"AcquireVertexLock(vid: size_t)-> cython.void"}),": locks a vertex vid and prohibits other threads from accessing the vertex data corresponding to this lock"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"void ReleaseVertexLock(vid: size_t)-> cython.void"}),": unlocks the vertex vid and all threads can access the vertex data corresponding to the lock"]}),"\n"]}),"\n"]}),"\n",(0,i.jsx)(n.p,{children:"TuGraph provides two batch operations to perform point-centric batch processing in parallel, which is slightly different from C++ in Python."}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-python",children:'# Function name: ProcessVertexInRange[ReducedSum, Algorithm](\n# work: (algo: Algorithm, vi: size_t)-> ReducedSum,\n# lower: size_t, upper: size_t,\n# algo: Algorithm,\n# zero: ReducedSum = 0,\n# reduce: (a: ReducedSum, b: ReducedSum)-> ReducedSum = reduce_plus[ReducedSum])\n#\n# Function purpose: Executes the work function on nodes whose node numbers are between lower and upper in the Graph. The fourth parameter indicates the base of accumulation, which is 0 by default.\n# The fifth parameter indicates that the iterative reduce function operation is performed on the node return value after each work process, and the default is the accumulation operation.\n# For specific implementation, please refer to the specific code in include/lgraph/olap_base.h\n#\n# Example usage: Count the number of vertices with edges in the parent array.\n\nimport cython\nfrom cython.cimports.olap_base import *\n\n\n@cython.cclass\nclass CountCore:\n    graph: cython.pointer(OlapBase[Empty])\n    parent: ParallelVector[size_t]\n\n    @cython.cfunc\n    @cython.nogil\n    def Work(self, vi: size_t) -> size_t:\n        if self.graph.OutDegree(self.parent[vi]) > 0:\n            return 1\n        return 0\n\n    def run(self, pointer_g: cython.pointer(OlapBase[Empty])):\n        self.graph = pointer_g\n        self.parent = self.graph.AllocVertexArray[size_t]()\n        vertex_num: size_t\n        vertex_num = self.graph.ProcessVertexInRange[size_t, CountCore](self.Work, 0, self.parent.Size(), self)\n        print("the number is", vertex_num)\n\nif __name__ == "__main__":\n    count_core = CountCore()\n    count_core.run(cython.address(g))\n'})}),"\n",(0,i.jsx)(n.p,{children:"g is the instantiated object of graph class OlapBase"}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-python",children:'# Function name: ProcessVertexActive[ReducedSum, Algorithm](\n# work: (algo: Algorithm, vi: size_t)-> ReducedSum,\n# active: ParallelBitset,\n# algo: Algorithm,\n# zero: ReducedSum = 0,\n# reduce: (a: ReducedSum, b: ReducedSum)-> ReducedSum = reduce_plus[ReducedSum])\n#\n# Function purpose: Execute the work function on the nodes corresponding to 1 in the active_vertices bitset. The third parameter indicates the base of accumulation, which is 0 by default;\n# The fourth parameter indicates that the iterative reduce function operation is performed on the node return value after each work process, and the default is the accumulation operation.\n# For specific implementation, please refer to the specific code in /include/lgraph/olap_base.h\n#\n# Usage example: Output all out-degree neighbors of nodes 1, 2, and 3 in the Graph, and count the total out-degree of these three nodes.\n\nimport cython\nfrom cython.cimports.olap_base import *\nfrom cython.cimports.libc.stdio import printf\n\n\n@cython.cclass\nclass NeighborCore:\n    graph: cython.pointer(OlapBase[Empty])\n    active_in: ParallelBitset\n\n    @cython.cfunc\n    @cython.nogil\n    def Work(self, vi: size_t) -> size_t:\n        degree = self. graph. OutDegree(vi)\n        dst: size_t\n        edges = self. graph. OutEdges(vi)\n        local_out_degree: size_t\n        for i in range(degree):\n            dst = edges[i].neighbor\n            printf("node %lu has neighbor %lu\\n", vi, dst)\n            local_out_degree += 1\n        return local_out_degree\n\n    def run(self, pointer_g: cython.pointer(OlapBase[Empty])):\n        self.graph = pointer_g\n        self.active_in = self.graph.AllocVertexSubset()\n        self. active_in. Add(1)\n        self. active_in. Add(2)\n        self. active_in. Add(3)\n        total_outdegree = cython.declare(\n            size_t,\n            self.graph.ProcessVertexActive[size_t, CountCore](self.Work, self.active_in, self))\n        printf("total outdegree of node1,2,3 is %lu\\n",total_outdegree)\n\nif __name__ == "__main__":\n    neighbor_core = NeighborCore()\n    neighbor_core.run(cython.address(g))\n'})}),"\n",(0,i.jsx)(n.p,{children:"As shown in the above two examples, ProcessVertexActive and ProcessVertexInRange in Python require an additional algorithm class pointer parameter compared to their C++ counterparts. The Work function is generally used as a member function of the algorithm class to access member variables, such as in the graph and ParallelVector parent examples shown above. When calling the batch function, the Work function and the self pointer of the algorithm class are passed to the batch function."}),"\n",(0,i.jsxs)(n.p,{children:["The Work function will be called in multiple threads, so the @cython.nogil decorator is added to release the Python global interpretation lock. In code executed by multiple threads, such as the Work function in the batch function, variables are better declared as C/C++ types using ",(0,i.jsx)(n.code,{children:"dst: type"})," or ",(0,i.jsx)(n.code,{children:"dst = cython.declare(type)"}),". This is because Python objects cannot be used in multi-threaded code."]}),"\n",(0,i.jsx)(n.h3,{id:"graph-class-olapondb",children:"Graph class OlapOnDB:"}),"\n",(0,i.jsx)(n.p,{children:"Parallelize to create a directed graph:"}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-python",children:"olapondb = OlapOnDB[Empty](db, txn, SNAPSHOT_PARALLEL)\n"})}),"\n",(0,i.jsx)(n.p,{children:"Parallelize to create an undirected graph"}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-python",children:"olapondb = OlapOnDB[Empty](db, txn, SNAPSHOT_PARALLEL | SNAPSHOT_UNDIRECTED)\n"})}),"\n",(0,i.jsx)(n.p,{children:"ID_MAPPING creates a directed graph"}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-python",children:"olapondb = OlapOnDB[Empty](db, txn, SNAPSHOT_PARALLEL | SNAPSHOT_IDMAPPING)\n"})}),"\n",(0,i.jsx)(n.h3,{id:"graph-class-olapondisk",children:"Graph class OlapOnDisk"}),"\n",(0,i.jsx)(n.h4,{id:"configbase",children:"ConfigBase:"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"ConfigBase()"}),": Constructor"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"std::string input_dir"}),": graph edge table data path"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"std::string output_dir"}),": output path"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"Load(config: ConfigBase[EdgeData], edge_direction_policy: EdgeDirectionPolicy)-> void"}),": read in graph data"]}),"\n"]}),"\n"]}),"\n",(0,i.jsx)(n.h2,{id:"5-lgraph_db-api",children:"5. lgraph_db API"}),"\n",(0,i.jsx)(n.p,{children:"Please refer to the files plugins/cython/lgraph_db.pxd and lgraph_db_python.py for the lgraph_db API."}),"\n",(0,i.jsxs)(n.p,{children:["The usage and functions of the interface in lgraph_db.pxd are basically the same as the C++ interface. The interface declared in lgraph_db.pxd is implemented in C++. In the py file, it must be imported by ",(0,i.jsx)(n.code,{children:"cython.cimports.olap_db import *"})," and compiled by the Cython file to run."]}),"\n",(0,i.jsx)(n.h3,{id:"vertexindexiterator",children:"VertexIndexIterator"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"GetVid()-> int64_t"}),": Get the vid of the vertex"]}),"\n"]}),"\n",(0,i.jsx)(n.h3,{id:"galaxy",children:"Galaxy"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"Galaxy(dir_path: std::string)"}),": constructor, dir_path is the db path"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"SetCurrentUser(user: std::string, password: std::string)-> cython.void"}),": set user"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"SetUser(user: std::string)-> cython.void"}),": set user"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"OpenGraph(graph: std::string, read_only: bint)-> GraphDB"}),": create GraphDB"]}),"\n"]}),"\n",(0,i.jsx)(n.h3,{id:"graphdb",children:"GraphDB:"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"CreateReadTxn()-> Transaction"}),": create a read-only transaction"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"CreateWriteTxn()-> Transaction"}),": create a write transaction"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"ForkTxn(txn: Transaction)-> Transaction"}),": Copy transactions, only read transactions can be copied"]}),"\n"]}),"\n",(0,i.jsx)(n.h3,{id:"transaction",children:"Transaction:"}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{children:"GetVertexIndexIterator(\n                 label: std::string,\n                 field: std::string,\n                 key_start: std::string,\n                 key_end: std::string) -> VertexIndexIterator\n"})}),"\n",(0,i.jsx)(n.p,{children:"Gets index iterator. The iterator has field value [key_start, key_end]. So key_start=key_end=v returns an iterator pointing to all vertexes that has field value v"}),"\n",(0,i.jsxs)(n.p,{children:["lgraph_db_python.py packages the C++ classes Galaxy and GraphDB from lgraph_db.pxd as Python classes. After compiling lgraph_db_python.py into a Python extension, you can directly access it in a Python file or from the Python command line by importing it with ",(0,i.jsx)(n.code,{children:"import lgraph_db_python"}),"."]}),"\n",(0,i.jsx)(n.h3,{id:"pygalaxy",children:"PyGalaxy:"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"PyGalaxy(self, dir_path: str)"}),": constructor, dir_path is the db path"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"SetCurrentUser(self, user: str password: str)-> void"}),": set user"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"SetUser(self, user: std::string)-> void"}),": set user"]}),"\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"OpenGraph(self, graph: str, read_only: bool)-> PyGraphDB"}),": create PyGraphDB"]}),"\n"]}),"\n",(0,i.jsx)(n.h3,{id:"pygraphdb",children:"PyGraphDB:"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:[(0,i.jsx)(n.code,{children:"get_pointer(self)->cython.Py_ssize_t"}),": address of C++ class GraphDB"]}),"\n"]}),"\n",(0,i.jsx)(n.h2,{id:"6-algorithm-plug-in-example",children:"6. Algorithm plug-in example"}),"\n",(0,i.jsx)(n.p,{children:"The following is a code example of the BFS algorithm implemented in Python:"}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-python",children:'# cython: language_level=3, cpp_locals=True, boundscheck=False, wraparound=False, initializedcheck=False\n# distutils: language = c++\n\n# Comments work as follows:\n# language_level=3: use Python3\n# cpp_locals=True: C++17 is required, and std::optional is used to manage C++ objects in Python code, which can avoid copy construction of C++ objects\n# boundscheck=False: Turn off bounds checking for indexes\n# wraparound=False: Turn off the processing of negative subscripts (similar to Python List)\n# initializedcheck=False: Turn off checking whether the memory is initialized, and the running performance will be faster after turning off the check\n# language = c++: translate this .py file to C++ instead of C file. TuGraph uses a lot of template functions, so C++ should be used\n\nimport json\n\nimport cython\nfrom cython.cimports.olap_base import *\nfrom cython.cimports.lgraph_db import *\n# From plugins/cython/ cimportolap_base.pxd and lgraph_db.pxd, similar to #include "xxx.h" in C++\n\nfrom cython.cimports.libc.stdio import printf\n# Similar to #include <stdio.h> in C++\n# Other common ones include cython.cimports.libcpp.unordered_map, etc.\n\nimport time\nimport lgraph_db_python\n\n@cython.cclass\n# cython.cclass indicates that BFSCore is a C-type Class\nclass BFSCore:\n    graph: cython.pointer(OlapBase[Empty])\n    # cython.pointer(OlapBase[Empty]) indicates the pointer of OlapBase[Empty], similar to OlapBase[Empty]* in C++\n    # Cython provides common types of pointers, such as cython.p_int, cython.p_char, etc.\n    parent: ParallelVector[size_t]\n    active_in: ParallelBitset\n    active_out: ParallelBitset\n    root: size_t\n    # root: size_t declares root as a C++ size_t type variable, equivalent to root = cython.declare(size_t)\n    # Variables that do not declare a type are Python object types\n    # Declaring variable types will greatly improve performance, and in the multi-threaded part, only C/C++ type variables can be accessed\n\n    @cython.cfunc\n    # cython.cfunc indicates that Work is a C-type function, and the parameters and return values should be declared\n    # cfunc has good performance and can accept C/C++ objects as parameters and return values, but it cannot be called in other python files\n    # Similar to cython.ccall, such as Standalone function, which can be called in other python files\n    @cython.nogil\n    # cython.nogil means to release the Python global interpretation lock. In the part modified by nogil, Python objects cannot be accessed\n    # In the multi-threaded part, there should be nogil decorator\n    @cython.exceptval(check=False)\n    # cython.exceptval(check=False) means that exception propagation is disabled, and Python exceptions raised inside the function will be ignored\n    def Work(self, vi: size_t) -> size_t:\n        degree = cython.declare(size_t, self.graph.OutDegree(vi))\n        out_edges = cython.declare(AdjList[Empty], self.graph.OutEdges(vi))\n        i = cython.declare(size_t, 0)\n        local_num_activations = cython.declare(size_t, 0)\n        dst: size_t\n        for i in range(degree):\n            dst = out_edges[i].neighbor\n            if self.parent[dst] == cython.cast(size_t, -1):\n                # parent[dst] == -1 means that dst has not been visited by bfs\n                if self.active_out.Add(dst):\n                    # Set dst as an active node; ParallelBitmap.Add is an atomic operation to prevent double calculation\n                    self.parent[dst] = vi\n                    local_num_activations += 1\n        return local_num_activations\n\n    @cython.cfunc\n    @cython.nogil\n    @cython.exceptval(check=False)\n    def run(self, g: cython.pointer(OlapBase[Empty]), r: size_t) -> cython. size_t:\n        self. graph = g\n        self.root = r\n        self.active_in = g.AllocVertexSubset()\n        self.active_out = g.AllocVertexSubset()\n        self.parent = g.AllocVertexArray[size_t]()\n        self. parent. Fill(-1)\n        num_vertices = cython.declare(size_t, self.graph.NumVertices())\n        printf("num_vertices = %lu\\n", num_vertices)\n        self.parent[self.root] = self.root\n        num_activations = cython.declare(size_t, 1)\n        discovered_vertices = cython.declare(size_t, num_activations)\n        self. active_in. Add(self. root)\n        while num_activations > 0:\n            self. active_out. Clear()\n            num_activations = g.ProcessVertexActive[size_t, BFSCore](self.Work, self.active_in, self)\n            discovered_vertices += num_activations\n            self. active_out. Swap(self. active_in)\n            printf("num_activations = %lu\\n", num_activations)\n        return discovered_vertices\n\n\n@cython.cfunc\ndef procedure_process(db: cython.pointer(GraphDB), request: dict, response: dict) -> cython.bint:\n    cost = time. time()\n    root_id = "0"\n    label = "node"\n    field = "id"\n    if "root" in request:\n        root_id = request["root"]\n    if "label" in request:\n        label = request["label"]\n    if "field" in request:\n        field = request["field"]\n\n    txn = db.CreateReadTxn()\n    olapondb = OlapOnDB[Empty](db[0], txn, SNAPSHOT_PARALLEL)\n    # Create OlapOnDB in parallel\n    # Cython does not support dereference operations such as *db, use db[0] to dereference\n    root_vid = txn.GetVertexIndexIterator(\n        label.encode(\'utf-8\'), field.encode(\'utf-8\'),\n        root_id.encode(\'utf-8\'), root_id.encode(\'utf-8\')\n    ).GetVid()\n    # Get the iterator of the root node through GetVertexIndexIterator based on the root node label name, field name and field value (root_id),\n    # and get the vid through the iterator. When there is no ID_MAPPING, the vid is the same as the id in OlapOnDB\n    cost = time. time() - cost\n    printf("prepare_cost = %lf s\\n", cython.cast(cython.double, cost))\n    a = BFSCore()\n    cost = time. time()\n    count = a. run(cython. address(olapondb), root_vid)\n    cost = time. time() - cost\n    printf("core_cost = %lf s\\n", cython.cast(cython.double, cost))\n    response["found_vertices"] = count\n    response["num_vertices"] = olapondb. NumVertices()\n    response["num_edges"] = olapondb. NumEdges()\n    return True\n\n\n@cython.ccall\ndef Standalone(input_dir: str, root: size_t):\n    # Standalone is the plug-in entry in Standalone mode, modified with cython.ccall\n    # You can set parameters arbitrarily, and modify plugins/cython/standalone_main.py accordingly\n    cost = time. time()\n    graph = OlapOnDisk[Empty]()\n    config = ConfigBase[Empty]()\n    config.input_dir = input_dir.encode("utf-8")\n    # config is a C++ class, config.input_dir is std::string, Python str needs to be encoded to be passed to std::string\n    graph.Load(config, DUAL_DIRECTION)\n    # read in graph\n    cost = time. time() - cost\n    printf("load_cost = %lf s\\n", cython.cast(cython.double, cost))\n\n    cost = time. time()\n    a = BFSCore()\n    count = a. run(cython. address(graph), root)\n    # cython.address(graph), address, similar to &graph in C++\n    cost = time. time() - cost\n    printf("core_cost = %lf s\\n", cython.cast(cython.double, cost))\n    print("find {} vertices". format(count))\n\n\n@cython.ccall\ndef Process(db: lgraph_db_python.PyGraphDB, inp: bytes):\n    # Process is the plug-in entry in embed mode and procedure mode, modified with cython.ccall\n    # The Process function must be named Process, and the parameters are lgraph_db_python.PyGraphDB and bytes\n    # The return value must be (bool, str)\n    _inp = inp.decode("utf-8")\n    request = json.loads(_inp)\n    response = {}\n    addr = cython.declare(cython.Py_ssize_t, db.get_pointer())\n    # Get the address of the GraphDB object in the PyGraphDB, convert it to a pointer and pass it\n    procedure_process(cython.cast(cython.pointer(GraphDB), addr),\n                      request, response)\n    return (True, json.dumps(response))\n\n'})})]})}function h(e={}){const{wrapper:n}={...(0,r.R)(),...e.components};return n?(0,i.jsx)(n,{...e,children:(0,i.jsx)(d,{...e})}):d(e)}}}]);