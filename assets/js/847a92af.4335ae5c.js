"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[37251],{28453:(e,n,o)=>{o.d(n,{R:()=>s,x:()=>c});var r=o(96540);const i={},l=r.createContext(i);function s(e){const n=r.useContext(l);return r.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function c(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(i):e.components||i:s(e.components),r.createElement(l.Provider,{value:n},e.children)}},80214:(e,n,o)=>{o.r(n),o.d(n,{assets:()=>t,contentTitle:()=>s,default:()=>u,frontMatter:()=>l,metadata:()=>c,toc:()=>d});var r=o(74848),i=o(28453);const l={},s="Compile",c={id:"developer-manual/running/compile",title:"Compile",description:"This document mainly describes how to compile TuGraph from source code.",source:"@site/versions/version-3.5.1/en-US/source/5.developer-manual/2.running/1.compile.md",sourceDirName:"5.developer-manual/2.running",slug:"/developer-manual/running/compile",permalink:"/tugraph-db/en/3.5.1/developer-manual/running/compile",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:1,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Cloud Deployment",permalink:"/tugraph-db/en/3.5.1/developer-manual/installation/cloud-deployment"},next:{title:"Tugraph Running",permalink:"/tugraph-db/en/3.5.1/developer-manual/running/tugraph-running"}},t={},d=[{value:"1.Prerequisites",id:"1prerequisites",level:2},{value:"2.compile",id:"2compile",level:2}];function a(e){const n={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",header:"header",li:"li",ol:"ol",p:"p",...(0,i.R)(),...e.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(n.header,{children:(0,r.jsx)(n.h1,{id:"compile",children:"Compile"})}),"\n",(0,r.jsxs)(n.blockquote,{children:["\n",(0,r.jsx)(n.p,{children:"This document mainly describes how to compile TuGraph from source code."}),"\n"]}),"\n",(0,r.jsx)(n.h2,{id:"1prerequisites",children:"1.Prerequisites"}),"\n",(0,r.jsxs)(n.p,{children:["It is recommended to build TuGraph on a Linux system. Meanwhile, Docker is a good choice. If you want to set up a new environment, please refer to ",(0,r.jsx)(n.a,{href:"/tugraph-db/en/3.5.1/developer-manual/installation/docker-deployment",children:"Dockerfile"}),"\u3002"]}),"\n",(0,r.jsx)(n.h2,{id:"2compile",children:"2.compile"}),"\n",(0,r.jsx)(n.p,{children:"The following are the steps for compiling TuGraph:"}),"\n",(0,r.jsxs)(n.ol,{children:["\n",(0,r.jsxs)(n.li,{children:[(0,r.jsx)(n.code,{children:"deps/build_deps.sh"})," or ",(0,r.jsx)(n.code,{children:"SKIP_WEB=1 deps/build_deps.sh"})," to skip building web interface"]}),"\n",(0,r.jsxs)(n.li,{children:[(0,r.jsx)(n.code,{children:"cmake .. -DOURSYSTEM=centos"})," or ",(0,r.jsx)(n.code,{children:"cmake .. -DOURSYSTEM=ubuntu"})]}),"\n",(0,r.jsxs)(n.li,{children:["If support shell lgraph_cypher,\n|||use ",(0,r.jsx)(n.code,{children:"-DENABLE_PREDOWNLOAD_DEPENDS_PACKAGE=1"})]}),"\n",(0,r.jsx)(n.li,{children:(0,r.jsx)(n.code,{children:"make"})}),"\n",(0,r.jsxs)(n.li,{children:[(0,r.jsx)(n.code,{children:"make package"})," or ",(0,r.jsx)(n.code,{children:"cpack --config CPackConfig.cmake"})]}),"\n"]})]})}function u(e={}){const{wrapper:n}={...(0,i.R)(),...e.components};return n?(0,r.jsx)(n,{...e,children:(0,r.jsx)(a,{...e})}):a(e)}}}]);