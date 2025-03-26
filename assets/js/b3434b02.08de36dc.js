"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[50213],{28453:(e,n,t)=>{t.d(n,{R:()=>d,x:()=>a});var i=t(96540);const r={},s=i.createContext(r);function d(e){const n=i.useContext(s);return i.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function a(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(r):e.components||r:d(e.components),i.createElement(s.Provider,{value:n},e.children)}},91865:(e,n,t)=>{t.r(n),t.d(n,{assets:()=>h,contentTitle:()=>d,default:()=>c,frontMatter:()=>s,metadata:()=>a,toc:()=>o});var i=t(74848),r=t(28453);const s={},d="TuGraph schema  Instructions",a={id:"introduction/schema",title:"TuGraph schema  Instructions",description:"1.The data model",source:"@site/versions/version-4.5.2/en-US/source/2.introduction/4.schema.md",sourceDirName:"2.introduction",slug:"/introduction/schema",permalink:"/tugraph-db/en/4.5.2/introduction/schema",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:4,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"What is TuGraph",permalink:"/tugraph-db/en/4.5.2/introduction/what-is-tugraph"},next:{title:"Performance Oriented",permalink:"/tugraph-db/en/4.5.2/introduction/characteristics/performance-oriented"}},h={},o=[{value:"1.The data model",id:"1the-data-model",level:2},{value:"1.1.Graph model",id:"11graph-model",level:3},{value:"1.2.The data type",id:"12the-data-type",level:3},{value:"1.3.Index",id:"13index",level:3},{value:"1.3.1 Single index",id:"131-single-index",level:4},{value:"1.3.1.1 Node index",id:"1311-node-index",level:4},{value:"1.3.1.1.1 unique index",id:"13111-unique-index",level:5},{value:"1.3.1.1.2 non_unique index",id:"13112-non_unique-index",level:5},{value:"1.3.1.2 Edge index",id:"1312-edge-index",level:4},{value:"1.3.1.2.1 unique index",id:"13121-unique-index",level:5},{value:"1.3.1.2.2 pair_unique index",id:"13122-pair_unique-index",level:5},{value:"1.3.1.2.3 non_unique index",id:"13123-non_unique-index",level:5},{value:"1.3.2 Composite index",id:"132-composite-index",level:4},{value:"1.3.2.1 Unique index",id:"1321-unique-index",level:5},{value:"1.3.2.2 non_unique index",id:"1322-non_unique-index",level:5},{value:"2. Graph Project, Vertex, Edge, and Attribute Naming Conventions and suggestions",id:"2-graph-project-vertex-edge-and-attribute-naming-conventions-and-suggestions",level:2},{value:"2.1 Naming Rules",id:"21-naming-rules",level:3},{value:"2.2 Usage Restrictions",id:"22-usage-restrictions",level:3},{value:"2.3 Naming Suggestions",id:"23-naming-suggestions",level:3}];function l(e){const n={code:"code",div:"div",em:"em",h1:"h1",h2:"h2",h3:"h3",h4:"h4",h5:"h5",header:"header",li:"li",ol:"ol",p:"p",strong:"strong",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",ul:"ul",...(0,r.R)(),...e.components};return(0,i.jsxs)(i.Fragment,{children:[(0,i.jsx)(n.header,{children:(0,i.jsx)(n.h1,{id:"tugraph-schema--instructions",children:"TuGraph schema  Instructions"})}),"\n",(0,i.jsx)(n.h2,{id:"1the-data-model",children:"1.The data model"}),"\n",(0,i.jsx)(n.h3,{id:"11graph-model",children:"1.1.Graph model"}),"\n",(0,i.jsx)(n.p,{children:"TuGraph is a strong schema, directed property graph database with multi-graph capability."}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"Graph Project: Each database service can host multiple graph projects (multi-graphs), and each graph project can have its own access control configuration. The database administrator can create or delete specified graph projects."}),"\n",(0,i.jsxs)(n.li,{children:["Vertex: Refers to entity, generally used to express real-world entities, such as a movie or an actor.\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"Primary Key: User-defined vertex data primary key, unique in the corresponding graph project and vertex type."}),"\n",(0,i.jsx)(n.li,{children:"VID: Refers to the auto-generated unique ID of the vertex, which cannot be modified by the user."}),"\n",(0,i.jsx)(n.li,{children:"Upper Limit: Each graph project can store up to 2^(40) vertex data."}),"\n"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["Edge: Used to express the relationship between vertexs, such as an actor appears in a movie.\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"Directed Edge: The edge is a directed edge. If you want to simulate an undirected edge, the user can create two edges with opposite directions."}),"\n",(0,i.jsx)(n.li,{children:"Duplicate Edge: TuGraph currently supports duplicate edges. If you want to ensure the uniqueness of the edge, you need to implement it through business policies."}),"\n",(0,i.jsx)(n.li,{children:"Upper Limit: Up to 2^(32) edge data can be stored between two vertex data."}),"\n"]}),"\n"]}),"\n",(0,i.jsx)(n.li,{children:"Property Graph: vertexs and edges can have properties associated with them, and each property can have a different type."}),"\n",(0,i.jsxs)(n.li,{children:["Strong-typed: Each vertex and edge has only one label, and after creating a label, there is a cost to modify the number and type of attributes.\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"Specify the starting/ending vertex type of the edge: You can limit the starting and ending vertex types of the edge, and support different vertex types of the starting and ending vertexs of the same type of edge, such as individuals transferring money to companies, companies transferring money to companies. After specifying the starting/ending vertex type of the edge, you can add multiple sets of starting/ending vertex types, but you cannot delete the restricted starting/ending vertex types."}),"\n",(0,i.jsx)(n.li,{children:"Unrestricted Mode: Supports creating edge data of this type between any two vertex types without specifying the starting and ending vertex types of the edge. Note: After specifying the starting/ending vertex type of the edge, the unrestricted mode cannot be used again."}),"\n"]}),"\n"]}),"\n"]}),"\n",(0,i.jsx)(n.h3,{id:"12the-data-type",children:"1.2.The data type"}),"\n",(0,i.jsx)(n.p,{children:"TuGraph Supports a variety of data types that can be used as attributes, the specific supported data types are as follows:"}),"\nTable 1. TuGraph supported data types\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,i.jsxs)(n.table,{children:[(0,i.jsx)(n.thead,{children:(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Type"})}),(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Min"})}),(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Max"})}),(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Description"})})]})}),(0,i.jsxs)(n.tbody,{children:[(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"BOOL"}),(0,i.jsx)(n.td,{children:"false"}),(0,i.jsx)(n.td,{children:"true"}),(0,i.jsx)(n.td,{children:"Boolean"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"INT8"}),(0,i.jsx)(n.td,{children:"-128"}),(0,i.jsx)(n.td,{children:"127"}),(0,i.jsx)(n.td,{children:"8-bit int"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"INT16"}),(0,i.jsx)(n.td,{children:"-32768"}),(0,i.jsx)(n.td,{children:"32767"}),(0,i.jsx)(n.td,{children:"16-bit int"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"INT32"}),(0,i.jsx)(n.td,{children:"- 2^31"}),(0,i.jsx)(n.td,{children:"2^31 - 1"}),(0,i.jsx)(n.td,{children:"32-bit int"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"INT64"}),(0,i.jsx)(n.td,{children:"- 2^63"}),(0,i.jsx)(n.td,{children:"2^63 - 1"}),(0,i.jsx)(n.td,{children:"64-bit int"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"DATE"}),(0,i.jsx)(n.td,{children:"0000-00-00"}),(0,i.jsx)(n.td,{children:"9999-12-31"}),(0,i.jsx)(n.td,{children:'"YYYY-MM-DD" Date of format'})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"DATETIME"}),(0,i.jsx)(n.td,{children:"0000-00-00 00:00:00.000000"}),(0,i.jsx)(n.td,{children:"9999-12-31 23:59:59.999999"}),(0,i.jsxs)(n.td,{children:['"YYYY-MM-DD hh:mm',(0,i.jsx)(n.div,{children:".ffffff"}),'"Format of the date and time']})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"FLOAT"}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{children:"32-bit float"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"DOUBLE"}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{children:"64-bit float"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"STRING"}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{children:"A string of variable length"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"BLOB"}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{children:"Binary data"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"POINT"}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{children:"EWKB format data of point"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"LINESTRING"}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{children:"EWKB format data of linestring"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"POLYGON"}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{children:"EWKB format data of polygon"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"FLOAT_VECTOR"}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{}),(0,i.jsx)(n.td,{children:"The dynamic vector containing 32-bit float numbers"})]})]})]}),"\n",(0,i.jsx)(n.p,{children:(0,i.jsx)(n.em,{children:"BLOB data is BASE64 encoded in input and output"})}),"\n",(0,i.jsx)(n.h3,{id:"13index",children:"1.3.Index"}),"\n",(0,i.jsx)(n.p,{children:"TuGraph supports indexing vertex fields."}),"\n",(0,i.jsx)(n.p,{children:"Indexes can be unique or non-unique. If a unique index is created for a vertex label, TuGraph will perform a data integrity check to ensure the uniqueness of the index before modifying the vertex of the label."}),"\n",(0,i.jsx)(n.p,{children:"Each index built on a single field of a label, and multiple fields can be indexed using the same label."}),"\n",(0,i.jsx)(n.p,{children:"BLOB fields cannot be indexed."}),"\n",(0,i.jsx)(n.p,{children:"TuGraph supports creating indexes on node or edge attributes to improve query efficiency. Its characteristics are as follows:"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"Indexes include single indexes and composite indexes. A single index is created based on a single property of a node or an edge, while a composite index is created based on multiple properties of a node or an edge (no more than 16). Indexes can be created on multiple sets of properties for the same node or edge."}),"\n",(0,i.jsx)(n.li,{children:"If a unique index is created for a node label, when modifying a node for that label, a data integrity check is first performed to ensure the uniqueness of the index."}),"\n",(0,i.jsx)(n.li,{children:"Attributes of type BLOB cannot be indexed."}),"\n"]}),"\n",(0,i.jsx)(n.p,{children:"There are multiple index types for TuGraph's vertices and edges. Different index types have different functions and restrictions, as follows:"}),"\n",(0,i.jsx)(n.h4,{id:"131-single-index",children:"1.3.1 Single index"}),"\n",(0,i.jsx)(n.h4,{id:"1311-node-index",children:"1.3.1.1 Node index"}),"\n",(0,i.jsx)(n.h5,{id:"13111-unique-index",children:"1.3.1.1.1 unique index"}),"\n",(0,i.jsxs)(n.p,{children:["The unique index of a node refers to a globally unique index. That is, if a unique index is set for an attribute, in the same graph, the attribute of nodes with the same label will not have the same value.\nThe maximum length of a unique index key is 480 bytes. ",(0,i.jsx)(n.strong,{children:"Unique indexes cannot be created for attributes exceeding 480 bytes"}),".\nPrimary is a special unique index, so the maximum key length is also 480 bytes."]}),"\n",(0,i.jsx)(n.h5,{id:"13112-non_unique-index",children:"1.3.1.1.2 non_unique index"}),"\n",(0,i.jsxs)(n.p,{children:["The non_unique index of a node refers to a non-global unique index, that is, if an attribute is set with a non_unique index,\nIn the same graph, nodes with the same label can have the same value for this attribute.\nSince a key in a non_unique index may be mapped to multiple values, in order to speed up search and writing,\nThe maximum value of a group of vids with the same index key is added after the user-specified key.\nEach vid is 5 bytes long, so the maximum length of a non_unique index key is 475 bytes.\nHowever, unlike unique indexes, non_unique indexes can also be established if they exceed 475 bytes.\nHowever, when indexing such an attribute, only ",(0,i.jsx)(n.strong,{children:"the first 475 bytes"})," will be intercepted as the index key (the value stored in the attribute itself will not be affected).\nMoreover, when traversing through an iterator, the first 475 bytes of the query value are automatically intercepted before traversing.\nTherefore, the results may be inconsistent with expectations and require users to filter again."]}),"\n",(0,i.jsx)(n.h4,{id:"1312-edge-index",children:"1.3.1.2 Edge index"}),"\n",(0,i.jsx)(n.h5,{id:"13121-unique-index",children:"1.3.1.2.1 unique index"}),"\n",(0,i.jsxs)(n.p,{children:["Similar to node, the unique index of an edge refers to a globally unique index. That is, if an attribute is set to a unique index, in the same graph, the attribute of the edge with the same label will not have the same value.\nThe maximum length of a unique index key is 480 bytes. ",(0,i.jsx)(n.strong,{children:"Unique indexes cannot be created for attributes exceeding 480 bytes"}),"."]}),"\n",(0,i.jsx)(n.h5,{id:"13122-pair_unique-index",children:"1.3.1.2.2 pair_unique index"}),"\n",(0,i.jsxs)(n.p,{children:["pair_unique index refers to the unique index between two nodes, that is, if an attribute sets a unique index, between the same set of the starting node and the ending node in the same graph,\nEdges with the same label will not have the same value for this attribute. In order to ensure that the pair_unique index key does not repeat between the starting node and the end node of the same group,\nThe index adds the starting and ending vids after the user-specified key. Each vid is 5 bytes in length.\nTherefore, the maximum key length is 470 bytes, and ",(0,i.jsx)(n.strong,{children:"a pair_unique index cannot be created for attributes exceeding 470 bytes"}),"."]}),"\n",(0,i.jsx)(n.h5,{id:"13123-non_unique-index",children:"1.3.1.2.3 non_unique index"}),"\n",(0,i.jsxs)(n.p,{children:["Similar to node, the non_unique index of an edge refers to a non-global unique index, that is, if an attribute sets a non_unique index,\nIn the same graph, edges with the same label can have the same value for this attribute.\nSince a key in a non_unique index may be mapped to multiple values, in order to speed up search and writing,\nThe maximum value of a group of eids with the same index key is added after the user-specified key.\nEach eid is 24 bytes in length, so the maximum length of a non_unique index key is 456 bytes.\nHowever, unlike unique indexes, non_unique indexes can also be established if they exceed 456 bytes.\nHowever, when indexing such an attribute, only ",(0,i.jsx)(n.strong,{children:"the first 456 bytes"})," will be intercepted as the index key (the value stored in the attribute itself will not be affected).\nMoreover, when traversing through an iterator, the first 475 bytes of the query value are automatically intercepted before traversing.\nTherefore, the results may be inconsistent with expectations and require users to filter again."]}),"\n",(0,i.jsx)(n.h4,{id:"132-composite-index",children:"1.3.2 Composite index"}),"\n",(0,i.jsx)(n.p,{children:"Currently, composite indexes are only supported for multiple properties of a vertex, and not supported for properties of an edge. There are two types of composite indexes: unique indexes and non-unique indexes. The requirements for creating a composite index are as follows:"}),"\n",(0,i.jsxs)(n.ol,{children:["\n",(0,i.jsx)(n.li,{children:"The number of properties for creating a composite index should be between 2 and 16 (inclusive)."}),"\n",(0,i.jsx)(n.li,{children:"For a unique composite index, the sum of the lengths of the properties cannot exceed 480 - 2 * (number of properties - 1) bytes, while for a non-unique composite index, it cannot exceed 475 - 2 * (number of properties - 1) bytes."}),"\n"]}),"\n",(0,i.jsx)(n.h5,{id:"1321-unique-index",children:"1.3.2.1 Unique index"}),"\n",(0,i.jsx)(n.p,{children:"Similar to a vertex's single unique index, a vertex's composite unique index refers to a globally unique index, meaning that for a set of properties with a unique index, there will not be another vertex with the same label in the same graph that has the same value for that group of properties.\nDue to the underlying storage design, the composite index key needs to retain the length of the properties; therefore, the maximum length for a composite unique index key is 480 - 2 * (number of properties - 1) bytes. Properties exceeding this length cannot be indexed uniquely."}),"\n",(0,i.jsx)(n.h5,{id:"1322-non_unique-index",children:"1.3.2.2 non_unique index"}),"\n",(0,i.jsx)(n.p,{children:"Similar to a vertex's single non-unique index, a vertex's non-unique index refers to a non-globally unique index, meaning that for a set of properties with a non-unique index, different vertices with the same label in the same graph may have the same value for that group of properties.\nSince a non-unique index key may map to multiple values to accelerate lookups and writing, the maximum value from a group of vertex IDs (vid) has been appended to the user-specified key where the index keys are identical. Each vid is 5 bytes in length; thus, the maximum length for a non-unique index key is 475 - 2 * (number of properties - 1) bytes. Properties exceeding this length cannot be indexed non-uniquely."}),"\n",(0,i.jsx)(n.h2,{id:"2-graph-project-vertex-edge-and-attribute-naming-conventions-and-suggestions",children:"2. Graph Project, Vertex, Edge, and Attribute Naming Conventions and suggestions"}),"\n",(0,i.jsx)(n.h3,{id:"21-naming-rules",children:"2.1 Naming Rules"}),"\n",(0,i.jsx)(n.p,{children:"Graph projects, vertices, edges, and attributes are identifiers. This section describes the allowed syntax for identifiers in TuGraph.\nThe table below describes the maximum length and allowed characters for each type of identifier."}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,i.jsxs)(n.table,{children:[(0,i.jsx)(n.thead,{children:(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Identifier"})}),(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Length"})}),(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Allowed Characters"})})]})}),(0,i.jsxs)(n.tbody,{children:[(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"User, role, graph project"}),(0,i.jsx)(n.td,{children:"1-64 characters"}),(0,i.jsx)(n.td,{children:"Chinese, letters, numbers, underscore, and the first character cannot be a number"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Vertex type, edge type, attribute"}),(0,i.jsx)(n.td,{children:"1-256 characters"}),(0,i.jsx)(n.td,{children:"Chinese, letters, numbers, underscore, and the first character cannot be a number"})]})]})]}),"\n",(0,i.jsx)(n.h3,{id:"22-usage-restrictions",children:"2.2 Usage Restrictions"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,i.jsxs)(n.table,{children:[(0,i.jsx)(n.thead,{children:(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Description"})}),(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Maximum number"})})]})}),(0,i.jsxs)(n.tbody,{children:[(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Number of users, number of roles"}),(0,i.jsx)(n.td,{children:"65536"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Number of graphs"}),(0,i.jsx)(n.td,{children:"4096"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Number of vertex and edge types per graph"}),(0,i.jsx)(n.td,{children:"4096"})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Number of attributes per type"}),(0,i.jsx)(n.td,{children:"1024"})]})]})]}),"\n",(0,i.jsx)(n.p,{children:"Note:\n1.Special characters and keywords: When using special characters or keywords, they need to be enclosed in backquotes (``) for reference;"}),"\n",(0,i.jsxs)(n.p,{children:["Example: ",(0,i.jsx)(n.code,{children:"match (`match`:match) return `match`.id limit 1"})]}),"\n",(0,i.jsx)(n.p,{children:"2.Case sensitivity: TuGraph is case-sensitive;"}),"\n",(0,i.jsx)(n.p,{children:"3.Graph project, vertex/edge, and attribute names can be reused, but attribute names under the same vertex or edge cannot be duplicated;"}),"\n",(0,i.jsx)(n.p,{children:"4.Reserved keywords for attribute names: SRC_ID / DST_ID / SKIP."}),"\n",(0,i.jsx)(n.h3,{id:"23-naming-suggestions",children:"2.3 Naming Suggestions"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,i.jsxs)(n.table,{children:[(0,i.jsx)(n.thead,{children:(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Identifier"})}),(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Description"})}),(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Suggestions"})})]})}),(0,i.jsxs)(n.tbody,{children:[(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Graph project"}),(0,i.jsx)(n.td,{children:"Start with a letter or Chinese character"}),(0,i.jsx)(n.td,{children:"Examples: graph123, project123, etc."})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Vertex/edge type"}),(0,i.jsx)(n.td,{children:"Start with a letter or Chinese character and use underscores to separate words"}),(0,i.jsx)(n.td,{children:"Examples: person, act_in, etc."})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Attribute"}),(0,i.jsx)(n.td,{children:"Letters or Chinese characters"}),(0,i.jsx)(n.td,{children:"Examples: name, age, etc."})]})]})]})]})}function c(e={}){const{wrapper:n}={...(0,r.R)(),...e.components};return n?(0,i.jsx)(n,{...e,children:(0,i.jsx)(l,{...e})}):l(e)}}}]);