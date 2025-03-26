"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[23925],{28453:(e,n,t)=>{t.d(n,{R:()=>s,x:()=>d});var i=t(96540);const r={},a=i.createContext(r);function s(e){const n=i.useContext(a);return i.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function d(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(r):e.components||r:s(e.components),i.createElement(a.Provider,{value:n},e.children)}},98474:(e,n,t)=>{t.r(n),t.d(n,{assets:()=>o,contentTitle:()=>s,default:()=>c,frontMatter:()=>a,metadata:()=>d,toc:()=>l});var i=t(74848),r=t(28453);const a={},s="What is TuGraph",d={id:"introduction/what-is-tugraph",title:"What is TuGraph",description:"This document mainly introduces the main features and characteristics of TuGraph Community Edition, as well as the differences between TuGraph Enterprise Edition and Community Edition.",source:"@site/versions/version-4.0.1/en-US/source/2.introduction/3.what-is-tugraph.md",sourceDirName:"2.introduction",slug:"/introduction/what-is-tugraph",permalink:"/tugraph-db/en-US/en/4.0.1/introduction/what-is-tugraph",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:3,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"What is a graph database",permalink:"/tugraph-db/en-US/en/4.0.1/introduction/what-is-gdbms"},next:{title:"Performance Oriented",permalink:"/tugraph-db/en-US/en/4.0.1/introduction/characteristics/performance-oriented"}},o={},l=[{value:"1.Introduction",id:"1introduction",level:2},{value:"2.TuGraph Community Edition",id:"2tugraph-community-edition",level:2},{value:"3.TuGraph features",id:"3tugraph-features",level:2},{value:"4.The data model",id:"4the-data-model",level:2},{value:"4.1.Graph model",id:"41graph-model",level:3},{value:"4.2.The data type",id:"42the-data-type",level:3},{value:"4.3.Index",id:"43index",level:3},{value:"5.TuGraph Enterprise Edition",id:"5tugraph-enterprise-edition",level:2}];function h(e){const n={blockquote:"blockquote",code:"code",div:"div",em:"em",h1:"h1",h2:"h2",h3:"h3",header:"header",li:"li",p:"p",pre:"pre",strong:"strong",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",ul:"ul",...(0,r.R)(),...e.components};return(0,i.jsxs)(i.Fragment,{children:[(0,i.jsx)(n.header,{children:(0,i.jsx)(n.h1,{id:"what-is-tugraph",children:"What is TuGraph"})}),"\n",(0,i.jsxs)(n.blockquote,{children:["\n",(0,i.jsx)(n.p,{children:"This document mainly introduces the main features and characteristics of TuGraph Community Edition, as well as the differences between TuGraph Enterprise Edition and Community Edition."}),"\n"]}),"\n",(0,i.jsx)(n.h2,{id:"1introduction",children:"1.Introduction"}),"\n",(0,i.jsx)(n.p,{children:"Graph database is a non-relational database that stores data by vertices and edges. It can be used to store complex data network models, such as social networks and transaction networks. TuGraph is a graph database developed by Ant Group. This manual introduces the functions and usage of TuGraph."}),"\n",(0,i.jsx)(n.h2,{id:"2tugraph-community-edition",children:"2.TuGraph Community Edition"}),"\n",(0,i.jsx)(n.p,{children:"The Community Edition is a fully functional version of TuGraph, suitable for single-instance deployment. It provides complete basic fuctions of graph database, such as ACID-compatible transactions, programming APIs, and associated tools. It is ideal for learning TuGraph and implementing small projects."}),"\n",(0,i.jsx)(n.h2,{id:"3tugraph-features",children:"3.TuGraph features"}),"\n",(0,i.jsx)(n.p,{children:"TuGraph is a large-scale graph computing system independently developed by Ant Group, providing graph database engine and graph analysis engine. Its main features are large data storage and computation, high throughput, and flexible API, while supporting efficient online transaction processing (OLTP) and online analytical processing (OLAP). LightGraph and GeaGraph are former names of TuGraph."}),"\n",(0,i.jsx)(n.p,{children:"The main functional features include:"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"Labeled property graph model"}),"\n",(0,i.jsx)(n.li,{children:"Support multiple Graphs"}),"\n",(0,i.jsx)(n.li,{children:"Full ACID transaction processing"}),"\n",(0,i.jsx)(n.li,{children:"Built-in 34 graph analysis algorithm"}),"\n",(0,i.jsx)(n.li,{children:"Graph visualization tool based on Web client"}),"\n",(0,i.jsx)(n.li,{children:"RESTful API and RPC are supported"}),"\n",(0,i.jsx)(n.li,{children:"OpenCypher graph query language"}),"\n",(0,i.jsx)(n.li,{children:"Stored procedure based on C++/Python/Java"}),"\n",(0,i.jsx)(n.li,{children:"The Traversal API for efficient graph algorithm development"}),"\n"]}),"\n",(0,i.jsx)(n.p,{children:"Performance and scalability features include:"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"TB large capacity"}),"\n",(0,i.jsx)(n.li,{children:"High throughput of ten million vertices per second"}),"\n",(0,i.jsx)(n.li,{children:"High Availability Support (Enterprise Edition)"}),"\n",(0,i.jsx)(n.li,{children:"High-performance Batch Import"}),"\n",(0,i.jsx)(n.li,{children:"Online/offline backup"}),"\n"]}),"\n",(0,i.jsx)(n.h2,{id:"4the-data-model",children:"4.The data model"}),"\n",(0,i.jsx)(n.h3,{id:"41graph-model",children:"4.1.Graph model"}),"\n",(0,i.jsx)(n.p,{children:"TuGraph is a strong schema, directed property graph database with multi-graph capability."}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"Graph Project: Each database service can host multiple graph projects (multi-graphs), and each graph project can have its own access control configuration. The database administrator can create or delete specified graph projects."}),"\n",(0,i.jsxs)(n.li,{children:["Vertex: Refers to entity, generally used to express real-world entities, such as a movie or an actor.\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"Primary Key: User-defined vertex data primary key, unique in the corresponding graph project and vertex type."}),"\n",(0,i.jsx)(n.li,{children:"VID: Refers to the auto-generated unique ID of the vertex, which cannot be modified by the user."}),"\n",(0,i.jsx)(n.li,{children:"Upper Limit: Each graph project can store up to 2^(40) vertex data."}),"\n"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["Edge: Used to express the relationship between vertexs, such as an actor appears in a movie.\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"Directed Edge: The edge is a directed edge. If you want to simulate an undirected edge, the user can create two edges with opposite directions."}),"\n",(0,i.jsx)(n.li,{children:"Duplicate Edge: TuGraph currently supports duplicate edges. If you want to ensure the uniqueness of the edge, you need to implement it through business policies."}),"\n",(0,i.jsx)(n.li,{children:"Upper Limit: Up to 2^(32) edge data can be stored between two vertex data."}),"\n"]}),"\n"]}),"\n",(0,i.jsx)(n.li,{children:"Property Graph: vertexs and edges can have properties associated with them, and each property can have a different type."}),"\n",(0,i.jsxs)(n.li,{children:["Strong-typed: Each vertex and edge has only one label, and after creating a label, there is a cost to modify the number and type of attributes.\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"Specify the starting/ending vertex type of the edge: You can limit the starting and ending vertex types of the edge, and support different vertex types of the starting and ending vertexs of the same type of edge, such as individuals transferring money to companies, companies transferring money to companies. After specifying the starting/ending vertex type of the edge, you can add multiple sets of starting/ending vertex types, but you cannot delete the restricted starting/ending vertex types."}),"\n",(0,i.jsx)(n.li,{children:"Unrestricted Mode: Supports creating edge data of this type between any two vertex types without specifying the starting and ending vertex types of the edge. Note: After specifying the starting/ending vertex type of the edge, the unrestricted mode cannot be used again."}),"\n"]}),"\n"]}),"\n"]}),"\n",(0,i.jsx)(n.h3,{id:"42the-data-type",children:"4.2.The data type"}),"\n",(0,i.jsx)(n.p,{children:"TuGraph Supports a variety of data types that can be used as attributes, the specific supported data types are as follows:"}),"\nTable 1. TuGraph supported data types\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,i.jsxs)(n.table,{children:[(0,i.jsx)(n.thead,{children:(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Type"})}),(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Min"})}),(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Max"})}),(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Description"})})]})}),(0,i.jsxs)(n.tbody,{children:[(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"BOOL"}),(0,i.jsx)(n.td,{children:"false"}),(0,i.jsx)(n.td,{children:"true"}),(0,i.jsx)(n.td,{children:"Boolean"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"INT8"}),(0,i.jsx)(n.td,{children:"-128"}),(0,i.jsx)(n.td,{children:"127"}),(0,i.jsx)(n.td,{children:"8-bit int"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"INT16"}),(0,i.jsx)(n.td,{children:"-32768"}),(0,i.jsx)(n.td,{children:"32767"}),(0,i.jsx)(n.td,{children:"16-bit int"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"INT32"}),(0,i.jsx)(n.td,{children:"- 2^31"}),(0,i.jsx)(n.td,{children:"2^31 - 1"}),(0,i.jsx)(n.td,{children:"32-bit int"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"INT64"}),(0,i.jsx)(n.td,{children:"- 2^63"}),(0,i.jsx)(n.td,{children:"2^63 - 1"}),(0,i.jsx)(n.td,{children:"64-bit int"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"DATE"}),(0,i.jsx)(n.td,{children:"0000-00-00"}),(0,i.jsx)(n.td,{children:"9999-12-31"}),(0,i.jsx)(n.td,{children:'"YYYY-MM-DD" Date of format'})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"DATETIME"}),(0,i.jsx)(n.td,{children:"0000-00-00 00:00:00.000000"}),(0,i.jsx)(n.td,{children:"9999-12-31 23:59:59.999999"}),(0,i.jsxs)(n.td,{children:['"YYYY-MM-DD hh:mm',(0,i.jsx)(n.div,{children:".ffffff"}),'"Format of the date and time']})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"FLOAT"}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{children:"32-bit float"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"DOUBLE"}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{children:"64-bit float"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"STRING"}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{children:"A string of variable length"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"BLOB"}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{children:"Binary data"})]})]})]}),"\n",(0,i.jsx)(n.p,{children:(0,i.jsx)(n.em,{children:"BLOB data is BASE64 encoded in input and output"})}),"\n",(0,i.jsx)(n.h3,{id:"43index",children:"4.3.Index"}),"\n",(0,i.jsx)(n.p,{children:"TuGraph supports indexing vertex fields."}),"\n",(0,i.jsx)(n.p,{children:"Indexes can be unique or non-unique. If a unique index is created for a vertex label, TuGraph will perform a data integrity check to ensure the uniqueness of the index before modifying the vertex of the label."}),"\n",(0,i.jsx)(n.p,{children:"Each index built on a single field of a label, and multiple fields can be indexed using the same label."}),"\n",(0,i.jsx)(n.p,{children:"BLOB fields cannot be indexed."}),"\n",(0,i.jsx)(n.p,{children:"TuGraph supports creating indexes on properties of vertexs or edges to improve query efficiency."}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"The index can be a unique or non-unique index."}),"\n",(0,i.jsx)(n.li,{children:"If a unique index is created for a vertex label, when modifying the vertex of the label, data integrity check will be performed first to ensure the uniqueness of the index."}),"\n",(0,i.jsx)(n.li,{children:"Each index is created based on a property of a vertex or edge, and indexes can be created on multiple properties of the same vertex or edge."}),"\n",(0,i.jsx)(n.li,{children:"An index cannot be created on a BLOB-type property."}),"\n"]}),"\n",(0,i.jsx)(n.h2,{id:"5tugraph-enterprise-edition",children:"5.TuGraph Enterprise Edition"}),"\n",(0,i.jsx)(n.p,{children:"The Enterprise Edition has more comprehensive support for commercial features, including distributed cluster architecture, a one-stop graph platform covering exploration, research and development, service and operation and maintenance throughout the lifecycle, online, near-line, and offline graph computing engines, support for streaming and big data data sources, multi-site and multi-center deployment, making it an ideal choice for commercial solutions."}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-text",children:"    If you need to implement production-level high-availability cluster architecture, operation and maintenance and other enterprise-level services, please contact us to obtain commercial support:\n    Email:tugraph@service.alipay.com\n    Tel:0571-85022088,extension number 83789993#\n"})})]})}function c(e={}){const{wrapper:n}={...(0,r.R)(),...e.components};return n?(0,i.jsx)(n,{...e,children:(0,i.jsx)(h,{...e})}):h(e)}}}]);