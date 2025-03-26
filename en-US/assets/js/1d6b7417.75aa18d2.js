"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[75233],{28453:(e,t,r)=>{r.d(t,{R:()=>s,x:()=>i});var n=r(96540);const a={},o=n.createContext(a);function s(e){const t=n.useContext(o);return n.useMemo((function(){return"function"==typeof e?e(t):{...t,...e}}),[t,e])}function i(e){let t;return t=e.disableParentContext?"function"==typeof e.components?e.components(a):e.components||a:s(e.components),n.createElement(o.Provider,{value:t},e.children)}},74742:(e,t,r)=>{r.r(t),r.d(t,{assets:()=>l,contentTitle:()=>s,default:()=>h,frontMatter:()=>o,metadata:()=>i,toc:()=>d});var n=r(74848),a=r(28453);const o={},s="Traversal API",i={id:"olap&procedure/procedure/traversal",title:"Traversal API",description:"This document mainly explains the Traversal API in the stored procedure of TuGraph.",source:"@site/versions/version-4.2.0/en-US/source/9.olap&procedure/1.procedure/2.traversal.md",sourceDirName:"9.olap&procedure/1.procedure",slug:"/olap&procedure/procedure/traversal",permalink:"/tugraph-db/en-US/en/4.2.0/olap&procedure/procedure/traversal",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:2,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"TuGraph Stored Procedure Guide",permalink:"/tugraph-db/en-US/en/4.2.0/olap&procedure/procedure/"},next:{title:"C++-procedure",permalink:"/tugraph-db/en-US/en/4.2.0/olap&procedure/procedure/C++-procedure"}},l={},d=[{value:"2. Interface",id:"2-interface",level:2},{value:"2.1. Snapshot",id:"21-snapshot",level:3},{value:"2.2. Traversal",id:"22-traversal",level:3}];function c(e){const t={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",li:"li",ol:"ol",p:"p",pre:"pre",...(0,a.R)(),...e.components};return(0,n.jsxs)(n.Fragment,{children:[(0,n.jsx)(t.header,{children:(0,n.jsx)(t.h1,{id:"traversal-api",children:"Traversal API"})}),"\n",(0,n.jsxs)(t.blockquote,{children:["\n",(0,n.jsx)(t.p,{children:"This document mainly explains the Traversal API in the stored procedure of TuGraph."}),"\n"]}),"\n",(0,n.jsxs)(t.ol,{children:["\n",(0,n.jsx)(t.li,{children:"Introduction\nThe powerful online analytical processing (OLAP) capability of TuGraph is an important feature that sets it apart from other graph databases. With the help of the C++ OLAP API (olap_on_db.h), users can quickly export a subgraph that needs to be analyzed, and then run iterative graph computing processes such as PageRank, connected components, and community detection on it, and then make corresponding decisions based on the results. The export and computation processes can be accelerated through parallel processing, achieving almost real-time analysis and processing, and avoiding the lengthy steps of traditional solutions that require exporting, transforming, and reimporting (ETL) data into dedicated analytical systems for offline processing."}),"\n"]}),"\n",(0,n.jsx)(t.p,{children:"TuGraph has built-in many commonly used graph analysis algorithms and rich auxiliary interfaces, so users hardly need to implement specific graph computing processes themselves. They just need to include the header files (.h files) of the corresponding algorithm library in their own programs when implementing their own storage procedures, and link the corresponding dynamic library files (.so) during compilation. In general, the only process that users need to implement themselves is the extraction of the subgraph to be analyzed."}),"\n",(0,n.jsx)(t.p,{children:"Currently, the Traversal API only supports C++."}),"\n",(0,n.jsx)(t.h2,{id:"2-interface",children:"2. Interface"}),"\n",(0,n.jsx)(t.h3,{id:"21-snapshot",children:"2.1. Snapshot"}),"\n",(0,n.jsx)(t.p,{children:"The Snapshot template class in C++ OLAP API is used to represent extracted static subgraphs. The EdgeData is used to represent the data type of the weight used for each edge in the subgraph. If the edges do not require weights, Empty is used as the EdgeData."}),"\n",(0,n.jsx)(t.p,{children:"The extracted subgraph can be described using the constructor of the Snapshot class."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-c",children:"Snapshot::Snapshot(\n    GraphDB & db,\n    Transaction & txn,\n    size_t flags = 0,\n    std::function<bool(VertexIterator &)> vertex_filter = nullptr,\n    std::function<bool(OutEdgeIterator &, EdgeData &)> out_edge_filter = nullptr\n);\n"})}),"\n",(0,n.jsx)(t.p,{children:'In the above text, "db" represents the database handle, "txn" represents the transaction handle, and "flags" represents the options used during generation, with the optional values including the following combinations: SNAPSHOT_PARALLEL indicates that multiple threads are used for parallel extraction during export; SNAPSHOT_UNDIRECTED indicates that the exported graph needs to be transformed into an undirected graph.'}),"\n",(0,n.jsx)(t.p,{children:'"vertex_filter" is a user-defined filtering function for vertices, where a return value of true indicates that the vertex needs to be included in the extracted subgraph, and vice versa.'}),"\n",(0,n.jsx)(t.p,{children:'"out_edge_filter" is a user-defined filtering function for edges, where a return value of true indicates that the edge needs to be included in the extracted subgraph, and vice versa.'}),"\n",(0,n.jsx)(t.p,{children:"When the filtering functions are set to default values, it means that all vertices/edges should be included."}),"\n",(0,n.jsx)(t.p,{children:"For other methods provided by the Snapshot class, please refer to the detailed C++ API documentation (olap_on_db.h)."}),"\n",(0,n.jsx)(t.h3,{id:"22-traversal",children:"2.2. Traversal"}),"\n",(0,n.jsx)(t.p,{children:"A common type of analysis in graph databases is to start from one or more vertices and iteratively expand and access their neighbors. Although this type of analysis can be done using Cypher, its performance is limited by the serial interpretation and execution when the depth of traversal is large. Writing stored procedures using the C++ Core API eliminates the overhead of interpretation but is still limited by the processing power of a single thread."}),"\n",(0,n.jsx)(t.p,{children:"In order to enable users to accelerate these types of analysis tasks through parallel processing, we have wrapped a Traversal framework based on the C++ OLAP API. Users can directly use the FrontierTraversal and PathTraversal classes in this framework to perform iterative traversal analysis tasks. For specific usage instructions, please refer to the corresponding C++ API documentation (lgraph_traversal.h)."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-c",children:"ParallelVector<size_t> FindVertices(\n    GraphDB & db,\n    Transaction & txn,\n    std::function<bool(VertexIterator &)> filter,\n    bool parallel = false\n);\n"})}),"\n",(0,n.jsx)(t.p,{children:'This method can be used to find all vertices that satisfy a certain condition (when the filter returns true). When the "parallel" parameter is set to true, the search process will be executed in parallel.'}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-c",children:"template <typename VertexData>\nParallelVector<VertexData> ExtractVertexData(\n    GraphDB & db,\n    Transaction & txn,\n    ParallelVector<size_t> & frontier,\n    std::function<void(VertexIterator &, VertexData &)> extract,\n    bool parallel = false\n);\n"})}),"\n",(0,n.jsx)(t.p,{children:'This method can be used to extract (VertexData type) properties from a specified set of vertices (frontier) using the extract method. When the "parallel" parameter is set to true, the extraction process will be executed in parallel.'}),"\n",(0,n.jsx)(t.p,{children:"FrontierTraversal is suitable for cases where only the traversed set of vertices is of interest. When a user needs to access information along the path (vertices/edges along the path) in the traversal process or result, PathTraversal needs to be used. Both types of traversal have four parameters in their constructor, namely the database handle db, transaction handle txn, options flags and capacity. The available options include the following combinations: TRAVERSAL_PARALLEL indicates that multiple threads are used for parallel traversal; TRAVERSAL_ALLOW_REVISITS indicates that vertices can be visited repeatedly during traversal (PathTraversal implicitly includes this option).\ncapacity indicates the capacity of the path collection during initialization."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-c",children:"void SetFrontier(size_t root_vid);\nvoid SetFrontier(ParallelVector<size_t> & root_vids);\nvoid SetFrontier(std::function<bool(VertexIterator &)> root_vertex_filter);\n"})}),"\n",(0,n.jsx)(t.p,{children:"Both types of traversal have three ways to set the starting vertex/vertex set for traversal. The first two methods directly specify the vertex ID, while the last method is similar to FindVertices."}),"\n",(0,n.jsx)(t.p,{children:"Both types of traversal start from the set of vertices in the current layer. They use the extension function to access each outgoing edge/incoming edge/outgoing and incoming edges, and use a user-defined filter function to determine if the extension is successful. If successful, the neighboring vertex/appended path with the edge is added to the set of vertices/paths in the next layer."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-c",children:"void ExpandOutEdges(\n    std::function<bool(OutEdgeIterator &)> out_edge_filter = nullptr,\n    std::function<bool(VertexIterator &)> out_neighbour_filter = nullptr\n);\nvoid ExpandInEdges(\n    std::function<bool(InEdgeIterator &)> in_edge_filter = nullptr,\n    std::function<bool(VertexIterator &)> in_neighbour_filter = nullptr\n);\nvoid ExpandEdges(\n    std::function<bool(OutEdgeIterator &)> out_edge_filter = nullptr,\n    std::function<bool(InEdgeIterator &)> in_edge_filter = nullptr,\n    std::function<bool(VertexIterator &)> out_neighbour_filter = nullptr,\n    std::function<bool(VertexIterator &)> in_neighbour_filter = nullptr\n);\n"})}),"\n",(0,n.jsx)(t.p,{children:"The above describes the three traversal methods of FrontierTraversal. It starts from the current set of vertices and, for each vertex in the set, iterates through each outgoing edge/incoming edge/outgoing and incoming edges. If the edge or neighbor vertex satisfies the user-defined filter conditions (where edge_filter is the filter function for edges and neighbour_filter is the filter function for neighbor vertices), the neighbor vertex is added to the new set of vertices."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-c",children:"ParallelVector<size_t> & GetFrontier();\n"})}),"\n",(0,n.jsx)(t.p,{children:"After the expansion of the current set of vertices is finished, the new set of vertices can be obtained using the above method."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-c",children:"void ExpandOutEdges(\n    std::function<bool(OutEdgeIterator &, Path &, IteratorHelper &)> out_edge_filter = nullptr,\n    std::function<bool(VertexIterator &, Path &, IteratorHelper &)> out_neighbour_filter = nullptr\n);\nvoid ExpandInEdges(\n    std::function<bool(InEdgeIterator &, Path &, IteratorHelper &)> in_edge_filter = nullptr,\n    std::function<bool(VertexIterator &, Path &, IteratorHelper &)> in_neighbour_filter = nullptr\n);\nvoid ExpandEdges(\n    std::function<bool(OutEdgeIterator &, Path &, IteratorHelper &)> out_edge_filter = nullptr,\n    std::function<bool(InEdgeIterator &, Path &, IteratorHelper &)> in_edge_filter = nullptr,\n    std::function<bool(VertexIterator &, Path &, IteratorHelper &)> out_neighbour_filter = nullptr,\n    std::function<bool(VertexIterator &, Path &, IteratorHelper &)> in_neighbour_filter = nullptr\n);\n"})}),"\n",(0,n.jsx)(t.p,{children:"The three traversal methods of PathTraversal are similar to FrontierTraversal, except that the user-defined filter function adds two additional parameters: Path, which contains the path before the new edge is expanded, and IteratorHelper, which can be used to convert the vertices/edges in the path to corresponding iterators in the database. The relevant documentation can be found in the corresponding C++ API documentation."})]})}function h(e={}){const{wrapper:t}={...(0,a.R)(),...e.components};return t?(0,n.jsx)(t,{...e,children:(0,n.jsx)(c,{...e})}):c(e)}}}]);