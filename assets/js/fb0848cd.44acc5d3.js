"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[97254],{28453:(e,r,t)=>{t.d(r,{R:()=>o,x:()=>c});var s=t(96540);const n={},u=s.createContext(n);function o(e){const r=s.useContext(u);return s.useMemo((function(){return"function"==typeof e?e(r):{...r,...e}}),[r,e])}function c(e){let r;return r=e.disableParentContext?"function"==typeof e.components?e.components(n):e.components||n:o(e.components),s.createElement(u.Provider,{value:r},e.children)}},84342:(e,r,t)=>{t.r(r),t.d(r,{assets:()=>i,contentTitle:()=>o,default:()=>l,frontMatter:()=>u,metadata:()=>c,toc:()=>a});var s=t(74848),n=t(28453);const u={},o="Rust \u5b58\u50a8\u8fc7\u7a0b",c={id:"olap&procedure/procedure/Rust-procedure",title:"Rust \u5b58\u50a8\u8fc7\u7a0b",description:"1. \u4ecb\u7ecd",source:"@site/versions/version-4.5.2/zh-CN/source/9.olap&procedure/1.procedure/5.Rust-procedure.md",sourceDirName:"9.olap&procedure/1.procedure",slug:"/olap&procedure/procedure/Rust-procedure",permalink:"/tugraph-db/zh/4.5.2/olap&procedure/procedure/Rust-procedure",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:5,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Traversal API",permalink:"/tugraph-db/zh/4.5.2/olap&procedure/procedure/traversal"},next:{title:"OLAP API",permalink:"/tugraph-db/zh/4.5.2/olap&procedure/olap/tutorial"}},i={},a=[{value:"1. \u4ecb\u7ecd",id:"1-\u4ecb\u7ecd",level:2},{value:"2. \u5982\u4f55\u4f7f\u7528",id:"2-\u5982\u4f55\u4f7f\u7528",level:2},{value:"3.API\u6587\u6863",id:"3api\u6587\u6863",level:2}];function d(e){const r={a:"a",code:"code",h1:"h1",h2:"h2",header:"header",li:"li",p:"p",ul:"ul",...(0,n.R)(),...e.components};return(0,s.jsxs)(s.Fragment,{children:[(0,s.jsx)(r.header,{children:(0,s.jsx)(r.h1,{id:"rust-\u5b58\u50a8\u8fc7\u7a0b",children:"Rust \u5b58\u50a8\u8fc7\u7a0b"})}),"\n",(0,s.jsx)(r.h2,{id:"1-\u4ecb\u7ecd",children:"1. \u4ecb\u7ecd"}),"\n",(0,s.jsx)(r.p,{children:"Rust \u5b58\u50a8\u8fc7\u7a0b\u76ee\u524d\u4ec5\u652f\u6301v1\u7248\u672c\uff0cTuGraph\u80fd\u591f\u652f\u6301\u4e00\u5207\u7f16\u8bd1\u6210\u52a8\u6001\u5e93\u7684\u8bed\u8a00\u4f5c\u4e3a\u63d2\u4ef6\u3002Rust\u8bed\u8a00\u4f5c\u4e3a\u7cfb\u7edf\u7f16\u7a0b\u8bed\u8a00\u7684\u65b0\u8d77\u4e4b\u79c0\uff0c\u5728\u5b89\u5168\u6027\u4e0a\u3001\u53ef\u9760\u6027\u4ee5\u53ca\u4eba\u4f53\u5de5\u7a0b\u5b66\u4e0a\u76f8\u8f83\u4e8eC++\u5177\u6709\u8f83\u5927\u4f18\u52bf\u3002"}),"\n",(0,s.jsxs)(r.p,{children:["\u6211\u4eec\u63d0\u4f9b\u4e86TuGraph\u7684",(0,s.jsx)(r.a,{href:"https://crates.io/crates/tugraph",children:"Rust binding"}),"\u5e93\u6765\u652f\u6301\u5728Rust\u4e2d\u8c03\u7528lgrahp api\uff0c\u540c\u65f6\u63d0\u4f9b",(0,s.jsx)(r.a,{href:"https://crates.io/crates/tugraph-plugin-util",children:"tugraph-plugin-util"})," \u5de5\u5177\u5e93\u6765\u5e2e\u52a9\u5927\u5bb6\u66f4\u52a0\u7b80\u6d01\u5730\u7f16\u5199Rust\u63d2\u4ef6\u4ee3\u7801\u3002"]}),"\n",(0,s.jsx)(r.h2,{id:"2-\u5982\u4f55\u4f7f\u7528",children:"2. \u5982\u4f55\u4f7f\u7528"}),"\n",(0,s.jsx)(r.p,{children:"Rust\u5b58\u50a8\u8fc7\u7a0b\u7684\u4f7f\u7528\u5206\u4e09\u6b65\uff1a"}),"\n",(0,s.jsxs)(r.ul,{children:["\n",(0,s.jsxs)(r.li,{children:["\u7f16\u8bd1\uff0c\u4ecerust\u6e90\u7801\u7f16\u8bd1\u51faso\u5e93\u3002\u6211\u4eec\u51c6\u5907\u4e86\u4e00\u4efd\u4e00\u7ad9\u5f0f\u7684\u63d2\u4ef6\u7f16\u5199\u6559\u7a0b\uff0c\u4eceIDE\u7684\u63d2\u4ef6\u5b89\u88c5\uff0c\u73af\u5883\u914d\u7f6e\uff0c\u5230\u7f16\u8bd1\uff0c\u8be6\u7ec6\u53c2\u8003",(0,s.jsx)(r.code,{children:"rust-tugraph-plugin-tutorial"}),"\u3002"]}),"\n",(0,s.jsxs)(r.li,{children:["\u52a0\u8f7d\uff0c\u5c06so\u5e93\u52a0\u8f7d\u5230\u670d\u52a1\u7aef\uff0c\u53ef\u4ee5\u901a\u8fc7REST\u6216RPC\u63a5\u53e3\uff0c\u8fd9\u4e00\u6b65\u548cC++\u5e93\u7684\u4f7f\u7528\u65b9\u5f0f\u7c7b\u4f3c\uff0c\u53ef\u4ee5\u53c2\u8003",(0,s.jsx)(r.a,{href:"/tugraph-db/zh/4.5.2/olap&procedure/procedure/",children:"\u6587\u6863"}),"\u3002"]}),"\n",(0,s.jsx)(r.li,{children:"\u8fd0\u884c\uff0c\u548cc++ procdure\u4f7f\u7528\u65b9\u5f0f\u76f8\u540c\uff0c\u4e0d\u5728\u8d58\u8ff0\u3002"}),"\n"]}),"\n",(0,s.jsx)(r.h2,{id:"3api\u6587\u6863",children:"3.API\u6587\u6863"}),"\n",(0,s.jsxs)(r.p,{children:["Rust\u793e\u533a\u4e60\u60ef\uff0c\u6240\u6709\u7684\u4ee3\u7801\u548c\u6587\u6863\u90fd\u53ef\u4ee5\u4ece",(0,s.jsx)(r.a,{href:"https://crates.io/crates/tugraph",children:(0,s.jsx)(r.code,{children:"crates.io"})}),"\u4ee5\u53ca",(0,s.jsx)(r.a,{href:"https://docs.rs/tugraph/latest/tugraph",children:(0,s.jsx)(r.code,{children:"docs.rs"})}),"\u627e\u5230\u3002"]})]})}function l(e={}){const{wrapper:r}={...(0,n.R)(),...e.components};return r?(0,s.jsx)(r,{...e,children:(0,s.jsx)(d,{...e})}):d(e)}}}]);