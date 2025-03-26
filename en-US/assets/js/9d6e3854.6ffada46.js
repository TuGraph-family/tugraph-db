"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[75581],{28453:(e,t,a)=>{a.d(t,{R:()=>o,x:()=>s});var r=a(96540);const n={},i=r.createContext(n);function o(e){const t=r.useContext(i);return r.useMemo((function(){return"function"==typeof e?e(t):{...t,...e}}),[t,e])}function s(e){let t;return t=e.disableParentContext?"function"==typeof e.components?e.components(n):e.components||n:o(e.components),r.createElement(i.Provider,{value:t},e.children)}},75163:(e,t,a)=>{a.r(t),a.d(t,{assets:()=>c,contentTitle:()=>o,default:()=>l,frontMatter:()=>i,metadata:()=>s,toc:()=>d});var r=a(74848),n=a(28453);const i={},o="Performance Oriented",s={id:"introduction/characteristics/performance-oriented",title:"Performance Oriented",description:"This document mainly introduces TuGraph's performance-oriented design philosophy as an open-source graph database.",source:"@site/versions/version-4.2.0/en-US/source/2.introduction/5.characteristics/1.performance-oriented.md",sourceDirName:"2.introduction/5.characteristics",slug:"/introduction/characteristics/performance-oriented",permalink:"/tugraph-db/en-US/en/4.2.0/introduction/characteristics/performance-oriented",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:1,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"TuGraph schema  Instructions",permalink:"/tugraph-db/en-US/en/4.2.0/introduction/schema"},next:{title:"Multi Level Interfaces",permalink:"/tugraph-db/en-US/en/4.2.0/introduction/characteristics/multi-level-Interfaces"}},c={},d=[{value:"1.Introduction",id:"1introduction",level:2},{value:"2.Characteristics of Graph Operations",id:"2characteristics-of-graph-operations",level:2},{value:"3.Storage Data Structures",id:"3storage-data-structures",level:2},{value:"4.Data Encoding",id:"4data-encoding",level:2}];function h(e){const t={blockquote:"blockquote",h1:"h1",h2:"h2",header:"header",p:"p",strong:"strong",...(0,n.R)(),...e.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(t.header,{children:(0,r.jsx)(t.h1,{id:"performance-oriented",children:"Performance Oriented"})}),"\n",(0,r.jsxs)(t.blockquote,{children:["\n",(0,r.jsx)(t.p,{children:"This document mainly introduces TuGraph's performance-oriented design philosophy as an open-source graph database."}),"\n"]}),"\n",(0,r.jsx)(t.h2,{id:"1introduction",children:"1.Introduction"}),"\n",(0,r.jsx)(t.p,{children:"TuGraph is currently the fastest graph database in the world, ranking first in the LDBC SNB Interactive graph database standard evaluation (March 2023). The design of TuGraph is based on performance optimization, and is committed to building a high-performance single-machine graph database. This document describes the core design of TuGraph's storage layer based on performance optimization."}),"\n",(0,r.jsx)(t.h2,{id:"2characteristics-of-graph-operations",children:"2.Characteristics of Graph Operations"}),"\n",(0,r.jsx)(t.p,{children:"Operations on property graphs involve reading, writing, and their attributes, and the access patterns of some special attributes such as timestamps can also affect overall performance. Here, by summarizing the regularities of some graph operation characteristics, we can guide the final performance."}),"\n",(0,r.jsx)(t.p,{children:"We observe that many graph applications have similar data access patterns. For example, in credit risk control, we use recursive path filtering to search for many-to-one patterns to find suspicious credit fraud users and behaviors. For online gambling, we can identify gambling-related fund accounts by recognizing multiple fund transfers in a short period of time. The equity penetration scenario recursively calculates the equity relationship between entities. These scenarios have common patterns such as multi-hop entity and relationship access, time window constraints, and read/write transactions.\nFurthermore, from the discussion in the introduction and the analysis of graph workloads, the following characteristics can be summarized:"}),"\n",(0,r.jsxs)(t.p,{children:[(0,r.jsx)(t.strong,{children:"Characteristic 1"})," KHop is the most typical operation in a graph, based on the data access pattern of the graph topology of points and edges, and has essential differences from relational databases. The typicality of KHop is not only reflected in the different data access patterns, but it is also the performance point that graph databases need to pay attention to the most."]}),"\n",(0,r.jsxs)(t.p,{children:[(0,r.jsx)(t.strong,{children:"Characteristic 2"})," Data access of graph workloads has a certain degree of locality on the topology, and the edges of the same point are usually accessed simultaneously. When the labels of these edges are the same, there is a greater probability of simultaneous access."]}),"\n",(0,r.jsxs)(t.p,{children:[(0,r.jsx)(t.strong,{children:"Characteristic 3"})," When accessing points and edges in graph workloads, their corresponding attributes are usually accessed as a traversal filter condition."]}),"\n",(0,r.jsxs)(t.p,{children:[(0,r.jsx)(t.strong,{children:"Characteristic 4"})," In time-based graph workloads, filtering of points and edges is usually within a certain time range, such as the past week."]}),"\n",(0,r.jsxs)(t.p,{children:[(0,r.jsx)(t.strong,{children:"Characteristic 5"})," Write operations may be accompanied by a large number of read operations, which need to be processed in a single transaction cycle."]}),"\n",(0,r.jsx)(t.p,{children:"Through the analysis of actual online graph applications, the read/write ratio of graph workloads is about 20:1, although the scenarios are limited to the financial field and the number of clusters is also limited, the scale of the data involved and the number of users are very large, which is representative to some extent. The 20:1 read/write ratio of graph workloads indicates that the impact of read workload on overall performance is greater, and the performance of write workload cannot be ignored either."}),"\n",(0,r.jsx)(t.h2,{id:"3storage-data-structures",children:"3.Storage Data Structures"}),"\n",(0,r.jsx)(t.p,{children:"TuGraph uses B+ trees as the underlying data structure to support real-time transactional operations for insertion, deletion, querying, and updating."}),"\n",(0,r.jsx)(t.p,{children:"In sorting tree data structures, B+ trees and LSM trees are the main representatives. B+ trees use a split and merge method to update sorted data in tree nodes, while LSM trees append updates to logs for delayed data merging. B+ trees were originally used in file system implementations, solving the problem of differences in performance between sequential and random operations on hard disks by storing data in adaptive-length leaf nodes, and have a relatively balanced read/write performance. The main advantage of LSM trees is to use WAL (Write Ahead Log) to perform updates, which turns update operations into sequential operations, especially in the case of small key-value pairs. WAL means that the update of data is deferred until it is merged in batches, which can improve the comprehensive efficiency of batch updates, but also makes the system scheduling more complicated. If the update merge is not completed, and the data is read again, LSM trees need to read several levels of locally merged logs, which will cause read amplification and space amplification, thereby affecting read efficiency."}),"\n",(0,r.jsx)(t.p,{children:"To sum up, B+ trees have better sequential read/write performance, while LSM trees have advantages in random write data. In addition, the way of LSM trees using background merging makes the performance fluctuation difficult to predict, and the correlation between performance fluctuation and upper-level storage and computation is weak, which increases the overall design cost. Considering the above factors, TuGraph chooses B+ trees as the implementation for read performance optimization."}),"\n",(0,r.jsx)(t.h2,{id:"4data-encoding",children:"4.Data Encoding"}),"\n",(0,r.jsx)(t.p,{children:"For property graph models, in addition to graph topology encoding, property data also greatly affects functionality and performance. We first discuss how property data can be encoded together with topology data. From current research, there are two ways of encoding properties: discrete encoding, which stores property data separately based on pointer indexes, and compact encoding, which packs property data and topology data together. Discrete encoding can store each property separately or pack each edge's properties separately, and the following discussion applies to both situations."}),"\n",(0,r.jsx)(t.p,{children:"Vertex Query: Property encoding mainly focuses on edges and does not involve vertex queries."}),"\n",(0,r.jsx)(t.p,{children:"Single-Edge Query: Discrete encoding locates edges through pointers, while compact encoding requires binary search to locate the edge position. Discrete encoding has a slight advantage."}),"\n",(0,r.jsx)(t.p,{children:"Edge Traversal: Discrete encoding requires constant random access through pointer jumps during edge traversal, while compact encoding arranges data together in advance, greatly improving efficiency due to its sequential access feature. Since edge traversal operations are common according to Rule 3, compact encoding has obvious advantages in edge traversal."}),"\n",(0,r.jsx)(t.p,{children:"Single-Edge Update: Discrete encoding only requires finding the corresponding pointer position and modifying the pointer's location before and after inserting data. Compact encoding needs to re-encode the compactly arranged data and rewrite the entire edge value, resulting in significant overhead compared to discrete encoding."}),"\n",(0,r.jsx)(t.p,{children:"Batch Edge Update: Batch updates can pre-build all edge properties for a vertex in memory and write them in one encoding, making both discrete encoding and compact encoding equivalent. However, compact encoding does not require storing pointer variables, so it has higher storage space efficiency."}),"\n",(0,r.jsx)(t.p,{children:"The performance issues of discrete encoding and compact encoding for a certain type of query discussed above can be alleviated through optimization. Overall, due to the 20:1 read-write load of graphs and the characteristics of property access revealed by Rule 3, TuGraph tends to use compact encoding to ensure read performance. Its main weakness is the overhead of re-encoding during single-edge updates, which can be solved using adaptive mapping technology."})]})}function l(e={}){const{wrapper:t}={...(0,n.R)(),...e.components};return t?(0,r.jsx)(t,{...e,children:(0,r.jsx)(h,{...e})}):h(e)}}}]);