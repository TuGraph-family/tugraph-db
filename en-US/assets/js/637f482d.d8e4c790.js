"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[87695],{28453:(e,n,r)=>{r.d(n,{R:()=>a,x:()=>l});var t=r(96540);const i={},s=t.createContext(i);function a(e){const n=t.useContext(s);return t.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function l(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(i):e.components||i:a(e.components),t.createElement(s.Provider,{value:n},e.children)}},73563:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>o,contentTitle:()=>a,default:()=>h,frontMatter:()=>s,metadata:()=>l,toc:()=>d});var t=r(74848),i=r(28453);const s={},a="Quick Start",l={id:"quick-start/preparation",title:"Quick Start",description:"This document is intended for new users to get started quickly and contains an introduction, features, installation, and use of TuGraph.",source:"@site/versions/version-3.5.1/en-US/source/3.quick-start/1.preparation.md",sourceDirName:"3.quick-start",slug:"/quick-start/preparation",permalink:"/tugraph-db/en-US/en/3.5.1/quick-start/preparation",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:1,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Glossary",permalink:"/tugraph-db/en-US/en/3.5.1/introduction/glossary"},next:{title:"DEMO:Movie",permalink:"/tugraph-db/en-US/en/3.5.1/quick-start/demo/movie"}},o={},d=[{value:"1.Introduction",id:"1introduction",level:2},{value:"1.1.Supported Platforms",id:"11supported-platforms",level:3},{value:"1.2.Hardware requirements",id:"12hardware-requirements",level:3},{value:"2.Installation",id:"2installation",level:2},{value:"2.1.Fast experience through Docker",id:"21fast-experience-through-docker",level:3}];function c(e){const n={a:"a",blockquote:"blockquote",code:"code",em:"em",h1:"h1",h2:"h2",h3:"h3",header:"header",li:"li",ol:"ol",p:"p",pre:"pre",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",ul:"ul",...(0,i.R)(),...e.components};return(0,t.jsxs)(t.Fragment,{children:[(0,t.jsx)(n.header,{children:(0,t.jsx)(n.h1,{id:"quick-start",children:"Quick Start"})}),"\n",(0,t.jsxs)(n.blockquote,{children:["\n",(0,t.jsx)(n.p,{children:"This document is intended for new users to get started quickly and contains an introduction, features, installation, and use of TuGraph."}),"\n"]}),"\n",(0,t.jsx)(n.h2,{id:"1introduction",children:"1.Introduction"}),"\n",(0,t.jsx)(n.p,{children:"TuGraph is a large-scale graph computing system independently developed by Ant Group, providing graph database engine and graph analysis engine. Its main features are large data storage and computation, high throughput, and flexible API, while supporting efficient online transaction processing (OLTP) and online analytical processing (OLAP). LightGraph and GeaGraph are former names of TuGraph."}),"\n",(0,t.jsx)(n.p,{children:"The main functional features include:"}),"\n",(0,t.jsxs)(n.ul,{children:["\n",(0,t.jsx)(n.li,{children:"Labeled property Graph Model"}),"\n",(0,t.jsx)(n.li,{children:"Support multiple Graphs"}),"\n",(0,t.jsx)(n.li,{children:"Full ACID transaction processing"}),"\n",(0,t.jsx)(n.li,{children:"Built-in 34 graph analysis algorithm"}),"\n",(0,t.jsx)(n.li,{children:"Graph visualization tool based on Web client"}),"\n",(0,t.jsx)(n.li,{children:"RESTful API and RPC are supported"}),"\n",(0,t.jsx)(n.li,{children:"OpenCypher graph query language"}),"\n",(0,t.jsx)(n.li,{children:"Stored procedure based on C++/Python/Java"}),"\n",(0,t.jsx)(n.li,{children:"The Traversal API for efficient graph algorithm development"}),"\n"]}),"\n",(0,t.jsx)(n.p,{children:"Performance and scalability features include:"}),"\n",(0,t.jsxs)(n.ul,{children:["\n",(0,t.jsx)(n.li,{children:"TB large capacity"}),"\n",(0,t.jsx)(n.li,{children:"High throughput of ten million vertices per second"}),"\n",(0,t.jsx)(n.li,{children:"High Availability Support (Enterprise Edition)"}),"\n",(0,t.jsx)(n.li,{children:"High-performance Batch Import"}),"\n",(0,t.jsx)(n.li,{children:"Online/offline backup"}),"\n"]}),"\n",(0,t.jsx)(n.h3,{id:"11supported-platforms",children:"1.1.Supported Platforms"}),"\n",(0,t.jsx)(n.p,{children:"TuGraph supports both X86_64 and ARM64 architectures in physical, virtual, and containerized environments."}),"\n",(0,t.jsx)(n.h3,{id:"12hardware-requirements",children:"1.2.Hardware requirements"}),"\n",(0,t.jsx)(n.p,{children:(0,t.jsx)(n.em,{children:"You are advised to use NVMe SSDS with large memory configurations for optimal performance\u3002"})}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,t.jsxs)(n.table,{children:[(0,t.jsx)(n.thead,{children:(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.th,{children:"Hardware"}),(0,t.jsx)(n.th,{children:"Minimum Configuration"}),(0,t.jsx)(n.th,{children:"Recommended configuration"})]})}),(0,t.jsxs)(n.tbody,{children:[(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"CPU"}),(0,t.jsx)(n.td,{children:"X86_64"}),(0,t.jsx)(n.td,{children:"Xeon E5 2670 v4"})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"Memory"}),(0,t.jsx)(n.td,{children:"4GB"}),(0,t.jsx)(n.td,{children:"256GB"})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"Disk"}),(0,t.jsx)(n.td,{children:"100GB"}),(0,t.jsx)(n.td,{children:"1TB NVMe SSD"})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"System"}),(0,t.jsx)(n.td,{children:"Linux 2.6"}),(0,t.jsx)(n.td,{children:"Ubuntu 18.04, CentOS 7.3"})]})]})]}),"\n",(0,t.jsx)(n.h2,{id:"2installation",children:"2.Installation"}),"\n",(0,t.jsx)(n.p,{children:"TuGraph can be installed quickly via Docker Image or locally via RPM /deb packages.In addition, TuGraph offers community edition services on Alibaba Cloud Computing Nest, which means you don't need to purchase your own cloud host to quickly deploy TuGraph services and achieve operational monitoring, thus building your own graph application."}),"\n",(0,t.jsxs)(n.blockquote,{children:["\n",(0,t.jsxs)(n.p,{children:["Official Website RPM /deb packages Download:",(0,t.jsx)(n.a,{href:"https://www.tugraph.org/download",children:"TuGraph DownLoad"}),"."]}),"\n"]}),"\n",(0,t.jsxs)(n.blockquote,{children:["\n",(0,t.jsxs)(n.p,{children:["The TuGraph image is hosted on DockerHub",(0,t.jsx)(n.a,{href:"https://hub.docker.com/u/tugraph",children:"DockerHub"}),"."]}),"\n"]}),"\n",(0,t.jsxs)(n.blockquote,{children:["\n",(0,t.jsxs)(n.p,{children:["The Cloud deployment can be searched on Aliyun Computing Nest or accessed quickly through the ",(0,t.jsx)(n.a,{href:"https://computenest.console.aliyun.com/user/cn-hangzhou/serviceInstanceCreate?ServiceId=service-7b50ea3d20e643da95bf&&isTrial=true",children:"deployment link"}),"."]}),"\n"]}),"\n",(0,t.jsx)(n.h3,{id:"21fast-experience-through-docker",children:"2.1.Fast experience through Docker"}),"\n",(0,t.jsxs)(n.ol,{children:["\n",(0,t.jsxs)(n.li,{children:["\n",(0,t.jsx)(n.p,{children:"The Docker environment installed locally"}),"\n",(0,t.jsxs)(n.p,{children:["The docker official documentation\uff1a",(0,t.jsx)(n.a,{href:"https://docs.docker.com/get-started/",children:"https://docs.docker.com/get-started/"})]}),"\n"]}),"\n",(0,t.jsxs)(n.li,{children:["\n",(0,t.jsx)(n.p,{children:"Pull the docker images"}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{className:"language-shell",children:"docker pull tugraph/tugraph-runtime-centos7\n"})}),"\n"]}),"\n",(0,t.jsxs)(n.li,{children:["\n",(0,t.jsx)(n.p,{children:"Start docker"}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{className:"language-shell",children:"docker run -d -p 7070:7070 -p 9090:9090 --name tugraph_demo tugraph/tugraph-runtime-centos7 lgraph_server\n# 7070 is default http port\uff0cfor web visiting\u3002\n# 9090 is default RPC port\uff0cfor RPC client visiting\u3002\n"})}),"\n"]}),"\n",(0,t.jsxs)(n.li,{children:["\n",(0,t.jsx)(n.p,{children:"Open by browser"}),"\n",(0,t.jsx)(n.p,{children:(0,t.jsx)(n.code,{children:"http://x.x.x.x:7070"})}),"\n",(0,t.jsxs)(n.blockquote,{children:["\n",(0,t.jsxs)(n.p,{children:["Default account ",(0,t.jsx)(n.code,{children:"admin"}),",Default password ",(0,t.jsx)(n.code,{children:"73@TuGraph"})]}),"\n"]}),"\n"]}),"\n",(0,t.jsxs)(n.li,{children:["\n",(0,t.jsx)(n.p,{children:"Start"}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{className:"language-shell",children:"docker run -dt -p 7070:7070 --name tugraph_demo tugraph/tugraph-runtime-centos7\ndocker exec -it tugraph_demo bash\n# start the service\nlgraph_server -d start\n"})}),"\n"]}),"\n"]})]})}function h(e={}){const{wrapper:n}={...(0,i.R)(),...e.components};return n?(0,t.jsx)(n,{...e,children:(0,t.jsx)(c,{...e})}):c(e)}}}]);