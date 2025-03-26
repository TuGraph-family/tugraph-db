"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[37936],{28453:(e,r,t)=>{t.d(r,{R:()=>s,x:()=>a});var n=t(96540);const o={},i=n.createContext(o);function s(e){const r=n.useContext(i);return n.useMemo((function(){return"function"==typeof e?e(r):{...r,...e}}),[r,e])}function a(e){let r;return r=e.disableParentContext?"function"==typeof e.components?e.components(o):e.components||o:s(e.components),n.createElement(i.Provider,{value:r},e.children)}},89570:(e,r,t)=>{t.r(r),t.d(r,{assets:()=>u,contentTitle:()=>s,default:()=>l,frontMatter:()=>i,metadata:()=>a,toc:()=>d});var n=t(74848),o=t(28453);const i={},s="Rust Stored Procedures",a={id:"developer-manual/interface/procedure/Rust-procedure",title:"Rust Stored Procedures",description:"1. Introduction",source:"@site/versions/version-3.6.0/en-US/source/5.developer-manual/6.interface/3.procedure/5.Rust-procedure.md",sourceDirName:"5.developer-manual/6.interface/3.procedure",slug:"/developer-manual/interface/procedure/Rust-procedure",permalink:"/tugraph-db/en-US/en/3.6.0/developer-manual/interface/procedure/Rust-procedure",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:5,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Traversal API",permalink:"/tugraph-db/en-US/en/3.6.0/developer-manual/interface/procedure/traversal"},next:{title:"TuGraph RESTful API",permalink:"/tugraph-db/en-US/en/3.6.0/developer-manual/interface/protocol/restful-api"}},u={},d=[{value:"1. Introduction",id:"1-introduction",level:2},{value:"2. How to Use",id:"2-how-to-use",level:2},{value:"3. API Documentation",id:"3-api-documentation",level:2}];function c(e){const r={a:"a",code:"code",h1:"h1",h2:"h2",header:"header",li:"li",p:"p",ul:"ul",...(0,o.R)(),...e.components};return(0,n.jsxs)(n.Fragment,{children:[(0,n.jsx)(r.header,{children:(0,n.jsx)(r.h1,{id:"rust-stored-procedures",children:"Rust Stored Procedures"})}),"\n",(0,n.jsx)(r.h2,{id:"1-introduction",children:"1. Introduction"}),"\n",(0,n.jsx)(r.p,{children:"Rust stored procedures currently only support v1 version. TuGraph supports plugins in any language that can be compiled into a dynamic library. Rust, as a rising star in system programming languages, has significant advantages in terms of safety, reliability, and ergonomics compared to C++."}),"\n",(0,n.jsxs)(r.p,{children:["We provide the Rust binding library for TuGraph to support calling lgrahp API in Rust. We also provide the ",(0,n.jsx)(r.a,{href:"https://crates.io/crates/tugraph-plugin-util",children:"tugraph-plugin-util"})," utility library to help simplify the process of writing Rust plugins."]}),"\n",(0,n.jsx)(r.h2,{id:"2-how-to-use",children:"2. How to Use"}),"\n",(0,n.jsx)(r.p,{children:"Using Rust stored procedures involves three steps:"}),"\n",(0,n.jsxs)(r.ul,{children:["\n",(0,n.jsxs)(r.li,{children:["Compilation: Compile the Rust source code into a dynamic library (so file). We have prepared a comprehensive plugin development tutorial that covers everything from IDE plugin installation and environment setup to compilation. Please refer to the ",(0,n.jsx)(r.code,{children:"rust-tugraph-plugin-tutorial"})," for detailed instructions."]}),"\n",(0,n.jsx)(r.li,{children:"Loading: Load the dynamic library (so file) into the server. This can be done through the REST or RPC interface, similar to the usage of C++ libraries. Please refer to the documentation for more details on the [Procedure v1 API](1.procedure.md#4.Procedure v1 interface)."}),"\n",(0,n.jsx)(r.li,{children:"Execution: Once the library is loaded, use it as you would with a C++ stored procedure. The process is the same and does not need further explanation."}),"\n"]}),"\n",(0,n.jsx)(r.h2,{id:"3-api-documentation",children:"3. API Documentation"}),"\n",(0,n.jsxs)(r.p,{children:["In the Rust community, all code and documentation can be found on ",(0,n.jsx)(r.a,{href:"https://crates.io/crates/tugraph",children:(0,n.jsx)(r.code,{children:"crates.io"})})," and ",(0,n.jsx)(r.a,{href:"https://docs.rs/tugraph/latest/tugraph",children:(0,n.jsx)(r.code,{children:"docs.rs"})}),"."]})]})}function l(e={}){const{wrapper:r}={...(0,o.R)(),...e.components};return r?(0,n.jsx)(r,{...e,children:(0,n.jsx)(c,{...e})}):c(e)}}}]);