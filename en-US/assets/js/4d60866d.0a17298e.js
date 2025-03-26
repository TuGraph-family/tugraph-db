"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[21581],{28453:(e,n,r)=>{r.d(n,{R:()=>c,x:()=>i});var d=r(96540);const s={},t=d.createContext(s);function c(e){const n=d.useContext(t);return d.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function i(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(s):e.components||s:c(e.components),d.createElement(t.Provider,{value:n},e.children)}},87775:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>o,contentTitle:()=>c,default:()=>h,frontMatter:()=>t,metadata:()=>i,toc:()=>a});var d=r(74848),s=r(28453);const t={},c="Vector index",i={id:"query/vector_index",title:"Vector index",description:"\u521b\u5efa\u5411\u91cf\u7d22\u5f15",source:"@site/versions/version-4.5.2/zh-CN/source/8.query/3.vector_index.md",sourceDirName:"8.query",slug:"/query/vector_index",permalink:"/tugraph-db/en-US/zh/4.5.2/query/vector_index",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:3,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"ISO GQL",permalink:"/tugraph-db/en-US/zh/4.5.2/query/gql"},next:{title:"Procedure API",permalink:"/tugraph-db/en-US/zh/4.5.2/olap&procedure/procedure/"}},o={},a=[{value:"\u521b\u5efa\u5411\u91cf\u7d22\u5f15",id:"\u521b\u5efa\u5411\u91cf\u7d22\u5f15",level:2},{value:"\u5411\u91cf\u67e5\u8be2",id:"\u5411\u91cf\u67e5\u8be2",level:2},{value:"KnnSearch",id:"knnsearch",level:3},{value:"RangeSearch",id:"rangesearch",level:3}];function l(e){const n={code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",p:"p",pre:"pre",...(0,s.R)(),...e.components};return(0,d.jsxs)(d.Fragment,{children:[(0,d.jsx)(n.header,{children:(0,d.jsx)(n.h1,{id:"vector-index",children:"Vector index"})}),"\n",(0,d.jsx)(n.h2,{id:"\u521b\u5efa\u5411\u91cf\u7d22\u5f15",children:"\u521b\u5efa\u5411\u91cf\u7d22\u5f15"}),"\n",(0,d.jsxs)(n.p,{children:["\u5982\u4e0bjson\u5b9a\u4e49\u4e86\u4e00\u4e2a\u70b9\u7c7b\u578b\uff0c\u540d\u5b57\u662f",(0,d.jsx)(n.code,{children:"person"}),", \u91cc\u9762\u6709\u4e2a\u5b57\u6bb5\u662f",(0,d.jsx)(n.code,{children:"embedding"}),"\uff0c\u7c7b\u578b\u662f",(0,d.jsx)(n.code,{children:"FLOAT_VECTOR"}),"\uff0c\u7528\u6765\u5b58\u50a8\u5411\u91cf\u6570\u636e\u3002\n\u76ee\u524d\u5411\u91cf\u6570\u636e\u53ea\u80fd\u5728\u70b9\u4e0a\u521b\u5efa\u3002"]}),"\n",(0,d.jsx)(n.pre,{children:(0,d.jsx)(n.code,{className:"language-json",children:'{\n\t"label": "person",\n\t"primary": "id",\n\t"type": "VERTEX",\n\t"properties": [{\n\t\t"name": "id",\n\t\t"type": "INT32",\n\t\t"optional": false\n\t}, {\n\t\t"name": "age",\n\t\t"type": "INT32",\n\t\t"optional": false\n\t}, {\n\t\t"name": "embedding",\n\t\t"type": "FLOAT_VECTOR",\n\t\t"optional": false\n\t}]\n}\n\n'})}),"\n",(0,d.jsx)(n.p,{children:"\u628a\u4e0a\u9762\u8fd9\u4e2ajson\u5e8f\u5217\u5316\u6210\u5b57\u7b26\u4e32\uff0c\u4f5c\u4e3a\u53c2\u6570\u4f20\u5165\uff0c\u5efa\u8bae\u4f7f\u7528\u9a71\u52a8\u7684\u53c2\u6570\u5316\u7279\u6027\uff0c\u907f\u514d\u81ea\u5df1\u62fc\u63a5\u8bed\u53e5\u3002"}),"\n",(0,d.jsx)(n.pre,{children:(0,d.jsx)(n.code,{children:"CALL db.createVertexLabelByJson($json_data)\n"})}),"\n",(0,d.jsxs)(n.p,{children:["\u7ed9",(0,d.jsx)(n.code,{children:"embedding"}),"\u5b57\u6bb5\u6dfb\u52a0\u5411\u91cf\u7d22\u5f15\uff0c\u7b2c\u4e09\u4e2a\u53c2\u6570\u662f\u4e2amap\uff0c\u91cc\u9762\u53ef\u4ee5\u8bbe\u7f6e\u4e00\u4e9b\u5411\u91cf\u7d22\u5f15\u7684\u914d\u7f6e\u53c2\u6570\uff0c\u5982\u4e0b\uff0c",(0,d.jsx)(n.code,{children:"dimension"}),"\u8bbe\u7f6e\u5411\u91cf\u7ef4\u5ea6\u662f4"]}),"\n",(0,d.jsx)(n.pre,{children:(0,d.jsx)(n.code,{children:"CALL db.addVertexVectorIndex('person','embedding', {dimension: 4});\n"})}),"\n",(0,d.jsxs)(n.p,{children:["\u518d\u5b9a\u4e49\u4e00\u4e2a\u8fb9\uff0c\u7528\u6765\u6d4b\u8bd5\uff0c\u5982\u4e0bjson\u5b9a\u4e49\u4e86\u4e00\u4e2a\u8fb9\u7c7b\u578b\uff0c\u540d\u5b57\u662f",(0,d.jsx)(n.code,{children:"like"}),"\u3002"]}),"\n",(0,d.jsx)(n.pre,{children:(0,d.jsx)(n.code,{className:"language-json",children:'{\n  "label": "like",\n  "type": "EDGE",\n  "constraints": [\n    ["person", "person"]\n  ],\n  "properties": []\n}\n'})}),"\n",(0,d.jsx)(n.p,{children:"\u628a\u4e0a\u9762\u8fd9\u4e2ajson\u5e8f\u5217\u5316\u6210\u5b57\u7b26\u4e32\uff0c\u4f5c\u4e3a\u53c2\u6570\u4f20\u5165\u3002"}),"\n",(0,d.jsx)(n.pre,{children:(0,d.jsx)(n.code,{children:"CALL db.createEdgeLabelByJson($json_data)\n"})}),"\n",(0,d.jsx)(n.p,{children:"\u5199\u5165\u51e0\u6761\u6d4b\u8bd5\u6570\u636e"}),"\n",(0,d.jsx)(n.pre,{children:(0,d.jsx)(n.code,{children:"CREATE (n1:person {id:1, age:10, embedding: [1.0,1.0,1.0,1.0]})\nCREATE (n2:person {id:2, age:20, embedding: [2.0,2.0,2.0,2.0]})\nCREATE (n3:person {id:3, age:30, embedding: [3.0,3.0,3.0,3.0]})\nCREATE (n1)-[r:like]->(n2),\n       (n2)-[r:like]->(n3),\n       (n3)-[r:like]->(n1);\n"})}),"\n",(0,d.jsx)(n.h2,{id:"\u5411\u91cf\u67e5\u8be2",children:"\u5411\u91cf\u67e5\u8be2"}),"\n",(0,d.jsx)(n.h3,{id:"knnsearch",children:"KnnSearch"}),"\n",(0,d.jsx)(n.p,{children:"\u6839\u636e\u5411\u91cf\u641c\u7d22\u51fa\u70b9\uff0c\u7b2c\u56db\u4e2a\u53c2\u6570\u662f\u4e2amap\uff0c\u91cc\u9762\u53ef\u4ee5\u6307\u5b9a\u4e00\u4e9b\u5411\u91cf\u641c\u7d22\u7684\u53c2\u6570\u3002"}),"\n",(0,d.jsx)(n.pre,{children:(0,d.jsx)(n.code,{children:"CALL db.vertexVectorKnnSearch('person','embedding', [1.0,2.0,3.0,4.0], {top_k:2, hnsw_ef_search:10})\nyield node return node\n"})}),"\n",(0,d.jsxs)(n.p,{children:["\u6839\u636e\u5411\u91cf\u641c\u7d22\u51fa\u70b9\uff0c\u8fd4\u56de",(0,d.jsx)(n.code,{children:"age"}),"\u5c0f\u4e8e30\u7684"]}),"\n",(0,d.jsx)(n.pre,{children:(0,d.jsx)(n.code,{children:"CALL db.vertexVectorKnnSearch('person','embedding',[1.0,2.0,3.0,4.0], {top_k:2, hnsw_ef_search:10})\nyield node where node.age < 30 return node\n"})}),"\n",(0,d.jsx)(n.p,{children:"\u6839\u636e\u5411\u91cf\u641c\u7d22\u51fa\u70b9\uff0c\u8fd4\u56deage\u5c0f\u4e8e30\u7684\u70b9\uff0c\u7136\u540e\u518d\u67e5\u8fd9\u4e9b\u70b9\u7684\u4e00\u5ea6\u90bb\u5c45\u662f\u8c01\u3002"}),"\n",(0,d.jsx)(n.pre,{children:(0,d.jsx)(n.code,{children:"CALL db.vertexVectorKnnSearch('person','embedding',[1.0,2.0,3.0,4.0], {top_k:2, hnsw_ef_search:10})\nyield node where node.age < 30 with node as p\nmatch(p)-[r]->(m) return m\n"})}),"\n",(0,d.jsx)(n.h3,{id:"rangesearch",children:"RangeSearch"}),"\n",(0,d.jsx)(n.p,{children:"\u6839\u636e\u5411\u91cf\u641c\u7d22\u51fa\u8ddd\u79bb\u5c0f\u4e8e10\u7684\u3001age\u5c0f\u4e8e30\u7684\u70b9\uff0c\u7136\u540e\u518d\u67e5\u8fd9\u4e9b\u70b9\u7684\u4e00\u5ea6\u90bb\u5c45\u662f\u8c01\u3002"}),"\n",(0,d.jsx)(n.pre,{children:(0,d.jsx)(n.code,{children:"CALL db.vertexVectorRangeSearch('person','embedding',[1.0,2.0,3.0,4.0], {radius:10.0, hnsw_ef_search:10})\nyield node where node.age < 30 with node as p\nmatch(p)-[r]->(m) return m\n"})})]})}function h(e={}){const{wrapper:n}={...(0,s.R)(),...e.components};return n?(0,d.jsx)(n,{...e,children:(0,d.jsx)(l,{...e})}):l(e)}}}]);