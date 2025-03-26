"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[79148],{28453:(e,n,r)=>{r.d(n,{R:()=>o,x:()=>l});var a=r(96540);const t={},i=a.createContext(t);function o(e){const n=a.useContext(i);return a.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function l(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(t):e.components||t:o(e.components),a.createElement(i.Provider,{value:n},e.children)}},33533:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>s,contentTitle:()=>o,default:()=>p,frontMatter:()=>i,metadata:()=>l,toc:()=>d});var a=r(74848),t=r(28453);const i={},o="Learning Tutorial",l={id:"developer-manual/interface/learn/tutorial",title:"Learning Tutorial",description:"This document is designed as a guide for TuGraph users. Before reading the detailed documentation, users should first read this document to have a general understanding of the graph learning process of TuGraph, which will make it easier to read the detailed documentation later. The guide program is based on a simple program instance of TuGraph, and we will focus on its usage.",source:"@site/versions/version-3.5.1/en-US/source/5.developer-manual/6.interface/5.learn/1.tutorial.md",sourceDirName:"5.developer-manual/6.interface/5.learn",slug:"/developer-manual/interface/learn/tutorial",permalink:"/tugraph-db/en-US/en/3.5.1/developer-manual/interface/learn/tutorial",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:1,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"RPC API",permalink:"/tugraph-db/en-US/en/3.5.1/developer-manual/interface/protocol/rpc-api"},next:{title:"Sampling API",permalink:"/tugraph-db/en-US/en/3.5.1/developer-manual/interface/learn/sampling_api"}},s={},d=[{value:"1. TuGraph Graph Learning Module",id:"1-tugraph-graph-learning-module",level:2},{value:"2. Data Import",id:"2-data-import",level:2},{value:"3. Feature Conversion",id:"3-feature-conversion",level:2},{value:"4. Sampling Operators and Compilation",id:"4-sampling-operators-and-compilation",level:2},{value:"4.1.Sampling Operator Introduction",id:"41sampling-operator-introduction",level:3},{value:"4.2.Compilation",id:"42compilation",level:3}];function h(e){const n={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",img:"img",li:"li",ol:"ol",p:"p",pre:"pre",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",...(0,t.R)(),...e.components};return(0,a.jsxs)(a.Fragment,{children:[(0,a.jsx)(n.header,{children:(0,a.jsx)(n.h1,{id:"learning-tutorial",children:"Learning Tutorial"})}),"\n",(0,a.jsxs)(n.blockquote,{children:["\n",(0,a.jsx)(n.p,{children:"This document is designed as a guide for TuGraph users. Before reading the detailed documentation, users should first read this document to have a general understanding of the graph learning process of TuGraph, which will make it easier to read the detailed documentation later. The guide program is based on a simple program instance of TuGraph, and we will focus on its usage."}),"\n"]}),"\n",(0,a.jsx)(n.h2,{id:"1-tugraph-graph-learning-module",children:"1. TuGraph Graph Learning Module"}),"\n",(0,a.jsx)(n.p,{children:"The TuGraph graph learning module samples graph data in TuGraph, and the sampled nodes and edges are used as features for graph learning and training. The running process is shown in the following figure:"}),"\n",(0,a.jsx)(n.p,{children:(0,a.jsx)(n.img,{alt:"Alt text",src:r(76847).A+"",width:"746",height:"984"})}),"\n",(0,a.jsx)(n.h2,{id:"2-data-import",children:"2. Data Import"}),"\n",(0,a.jsxs)(n.p,{children:["Please refer to ",(0,a.jsx)(n.a,{href:"/tugraph-db/en-US/en/3.5.1/developer-manual/server-tools/data-import",children:"Data Import"})," for data import."]}),"\n",(0,a.jsx)(n.p,{children:"Taking the cora dataset as an example for the import process:"}),"\n",(0,a.jsx)(n.p,{children:"Execute in the build/output directory:"}),"\n",(0,a.jsx)(n.pre,{children:(0,a.jsx)(n.code,{className:"language-bash",children:"./lgraph_import -c ./../../test/integration/data/algo/cora.conf --dir ./cora_db --overwrite true\n"})}),"\n",(0,a.jsx)(n.p,{children:"Where cora.conf is the graph schema file representing the format of the graph data. cora_db is the imported graph data file name representing the storage location of the graph data."}),"\n",(0,a.jsx)(n.h2,{id:"3-feature-conversion",children:"3. Feature Conversion"}),"\n",(0,a.jsx)(n.p,{children:"Since the features in graph learning are generally represented as long float arrays, TuGraph does not support loading float array types, so they can be imported as string types and converted to char* for subsequent storage and access, and the implementation details can refer to the feature_float.cpp file."}),"\n",(0,a.jsx)(n.p,{children:"The specific execution process is as follows:"}),"\n",(0,a.jsx)(n.p,{children:"Compile the imported plugin in the build directory:"}),"\n",(0,a.jsx)(n.p,{children:(0,a.jsx)(n.code,{children:"make feature_float_embed"})}),"\n",(0,a.jsx)(n.p,{children:"Execute in the build/output directory:"}),"\n",(0,a.jsx)(n.p,{children:(0,a.jsx)(n.code,{children:"./algo/feature_float_embed ./cora_db"})}),"\n",(0,a.jsx)(n.p,{children:"Then the conversion can be performed."}),"\n",(0,a.jsx)(n.h2,{id:"4-sampling-operators-and-compilation",children:"4. Sampling Operators and Compilation"}),"\n",(0,a.jsx)(n.p,{children:"TuGraph implements an operator for obtaining the full graph data and four sampling operators at the cython layer, as follows:"}),"\n",(0,a.jsx)(n.h3,{id:"41sampling-operator-introduction",children:"4.1.Sampling Operator Introduction"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,a.jsxs)(n.table,{children:[(0,a.jsx)(n.thead,{children:(0,a.jsxs)(n.tr,{children:[(0,a.jsx)(n.th,{children:"Sampling Operator"}),(0,a.jsx)(n.th,{children:"Sampling Method"})]})}),(0,a.jsxs)(n.tbody,{children:[(0,a.jsxs)(n.tr,{children:[(0,a.jsx)(n.td,{children:"GetDB"}),(0,a.jsx)(n.td,{children:"Get the graph data from the database and convert it into the required data structure"})]}),(0,a.jsxs)(n.tr,{children:[(0,a.jsx)(n.td,{children:"Neighbor Sampling"}),(0,a.jsx)(n.td,{children:"Sample the neighboring nodes of the given node to obtain the sampling subgraph"})]}),(0,a.jsxs)(n.tr,{children:[(0,a.jsx)(n.td,{children:"Edge Sampling"}),(0,a.jsx)(n.td,{children:"Sample the edges in the graph according to the sampling rate to obtain the sampling subgraph"})]}),(0,a.jsxs)(n.tr,{children:[(0,a.jsx)(n.td,{children:"Random Walk Sampling"}),(0,a.jsx)(n.td,{children:"Conduct a random walk based on the given node to obtain the sampling subgraph"})]}),(0,a.jsxs)(n.tr,{children:[(0,a.jsx)(n.td,{children:"Negative Sampling"}),(0,a.jsx)(n.td,{children:"Generate a subgraph of non-existent edges"})]})]})]}),"\n",(0,a.jsx)(n.h3,{id:"42compilation",children:"4.2.Compilation"}),"\n",(0,a.jsx)(n.p,{children:"Execute in the tugraph-db/build folder:"}),"\n",(0,a.jsx)(n.p,{children:(0,a.jsx)(n.code,{children:"make -j2"})}),"\n",(0,a.jsx)(n.p,{children:"Or execute in the tugraph-db/learn/procedures folder:"}),"\n",(0,a.jsx)(n.p,{children:(0,a.jsx)(n.code,{children:"python3 setup.py build_ext -i"})}),"\n",(0,a.jsx)(n.p,{children:"Once the algorithm so is obtained, it can be used by importing it in Python."}),"\n",(0,a.jsxs)(n.ol,{start:"5",children:["\n",(0,a.jsx)(n.li,{children:"Training\nTuGraph calls the cython layer operator at the Python layer to implement graph learning and training."}),"\n"]}),"\n",(0,a.jsx)(n.p,{children:"The usage of the TuGraph graph learning module is as follows:"}),"\n",(0,a.jsx)(n.p,{children:"Execute in the tugraph-db/learn/example folder:"}),"\n",(0,a.jsx)(n.p,{children:(0,a.jsx)(n.code,{children:"python3 train_full_cora.py"})}),"\n",(0,a.jsx)(n.p,{children:"Then training can be performed."}),"\n",(0,a.jsx)(n.p,{children:"If the final printed loss value is less than 0.9, the training is successful."})]})}function p(e={}){const{wrapper:n}={...(0,t.R)(),...e.components};return n?(0,a.jsx)(n,{...e,children:(0,a.jsx)(h,{...e})}):h(e)}},76847:(e,n,r)=>{r.d(n,{A:()=>a});const a=r.p+"assets/images/learn_flow_chart_en-4a77be5d31e739c6855f132fdc7bcd8a.png"}}]);