"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[19466],{2183:(e,t,n)=>{n.r(t),n.d(t,{assets:()=>u,contentTitle:()=>o,default:()=>d,frontMatter:()=>c,metadata:()=>s,toc:()=>a});var r=n(74848),i=n(28453);const c={},o="TuGraph\u4ea7\u54c1\u67b6\u6784",s={id:"introduction/architecture",title:"TuGraph\u4ea7\u54c1\u67b6\u6784",description:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph \u7684\u4ea7\u54c1\u67b6\u6784\u3002",source:"@site/versions/version-4.0.1/zh-CN/source/2.introduction/5.architecture.md",sourceDirName:"2.introduction",slug:"/introduction/architecture",permalink:"/tugraph-db/en-US/zh/4.0.1/introduction/architecture",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:5,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"HTAP",permalink:"/tugraph-db/en-US/zh/4.0.1/introduction/characteristics/htap"},next:{title:"\u529f\u80fd\u6982\u89c8",permalink:"/tugraph-db/en-US/zh/4.0.1/introduction/functionality"}},u={},a=[{value:"1.\u7b80\u4ecb",id:"1\u7b80\u4ecb",level:2}];function h(e){const t={blockquote:"blockquote",h1:"h1",h2:"h2",header:"header",img:"img",li:"li",p:"p",ul:"ul",...(0,i.R)(),...e.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(t.header,{children:(0,r.jsx)(t.h1,{id:"tugraph\u4ea7\u54c1\u67b6\u6784",children:"TuGraph\u4ea7\u54c1\u67b6\u6784"})}),"\n",(0,r.jsxs)(t.blockquote,{children:["\n",(0,r.jsx)(t.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph \u7684\u4ea7\u54c1\u67b6\u6784\u3002"}),"\n"]}),"\n",(0,r.jsx)(t.h2,{id:"1\u7b80\u4ecb",children:"1.\u7b80\u4ecb"}),"\n",(0,r.jsx)(t.p,{children:(0,r.jsx)(t.img,{alt:"\u4ea7\u54c1\u67b6\u6784",src:n(11746).A+"",width:"1072",height:"520"})}),"\n",(0,r.jsx)(t.p,{children:"\u4e0a\u56fe\u4ece\u529f\u80fd\u6a21\u5757\u7684\u89d2\u5ea6\uff0c\u4ee5 TuGraph \u4e3a\u4f8b\uff0c\u7ed9\u51fa\u4e86\u4f01\u4e1a\u7ea7\u56fe\u6570\u636e\u5e93\u7684\u6574\u4f53\u67b6\u6784\uff0c\u81ea\u4e0b\u800c\u4e0a\u5305\u62ec\uff1a"}),"\n",(0,r.jsxs)(t.ul,{children:["\n",(0,r.jsx)(t.li,{children:"\u8f6f\u786c\u4ef6\u73af\u5883\u3002\u6d89\u53ca\u56fe\u6570\u636e\u5e93\u7684\u5f00\u53d1\u548c\u4f7f\u7528\u73af\u5883\u3002TuGraph \u4e3b\u8981\u57fa\u4e8e\u5e95\u5c42\u7684 C++\u8bed\u8a00\u5f00\u53d1\uff0c\u80fd\u591f\u517c\u5bb9\u5e02\u9762\u4e0a\u5927\u90e8\u5206\u64cd\u4f5c\u7cfb\u7edf\u548c CPU\u3002"}),"\n",(0,r.jsx)(t.li,{children:"\u5b58\u50a8\u5c42\uff0c\u5305\u62ec KV \u5b58\u50a8\u5c42\u548c\u56fe\u5b58\u50a8\u5c42\u3002\u5b58\u50a8\u5c42\u9700\u8981\u652f\u6301\u8ba1\u7b97\u5c42\u6240\u9700\u7684\u5404\u4e2a\u529f\u80fd\u3002"}),"\n",(0,r.jsx)(t.li,{children:"\u8ba1\u7b97\u5c42\u3002\u8ba1\u7b97\u5c42\u5e94\u5305\u62ec\u56fe\u4e8b\u52a1\u5f15\u64ce\u3001\u56fe\u5206\u6790\u5f15\u64ce\u548c\u56fe\u795e\u7ecf\u7f51\u7edc\u5f15\u64ce\uff0c\u4e5f\u5305\u542b\u4e86\u670d\u52a1\u7aef\u63d0\u4f9b\u7684\u591a\u79cd\u7f16\u7a0b\u63a5\u53e3\uff0c\u5305\u62ec\u63cf\u8ff0\u5f0f\u67e5\u8be2\u8bed\u8a00 Cypher\uff0c\u5b58\u50a8\u8fc7\u7a0b\u7b49\u3002"}),"\n",(0,r.jsx)(t.li,{children:"\u5ba2\u6237\u7aef\u3002\u5ba2\u6237\u7aef SDK \u5e94\u652f\u6301 Java\u3001Python\u3001C++ \u7b49\u591a\u79cd\u8bed\u8a00\uff0c\u4e5f\u652f\u6301\u547d\u4ee4\u884c\u7684\u4ea4\u4e92\u65b9\u5f0f\u3002Browser \u548c Explorer \u901a\u8fc7\u7f51\u9875\u7aef\u4ea4\u4e92\u7684\u65b9\u5f0f\uff0c\u964d\u4f4e\u4e86\u56fe\u6570\u636e\u5e93\u7684\u4f7f\u7528\u95e8\u69db\u3002"}),"\n",(0,r.jsx)(t.li,{children:"\u5728\u751f\u6001\u5de5\u5177\u65b9\u9762\uff0c\u8986\u76d6\u4e86\u4f01\u4e1a\u7ea7\u56fe\u6570\u636e\u5e93\u7684\u5f00\u53d1\u3001\u8fd0\u7ef4\u3001\u7ba1\u7406\u7b49\u94fe\u8def\uff0c\u63d0\u5347\u53ef\u7528\u6027\u3002"}),"\n"]})]})}function d(e={}){const{wrapper:t}={...(0,i.R)(),...e.components};return t?(0,r.jsx)(t,{...e,children:(0,r.jsx)(h,{...e})}):h(e)}},11746:(e,t,n)=>{n.d(t,{A:()=>r});const r=n.p+"assets/images/architecture-719f752da7eb2e67e0d805d644339d87.png"},28453:(e,t,n)=>{n.d(t,{R:()=>o,x:()=>s});var r=n(96540);const i={},c=r.createContext(i);function o(e){const t=r.useContext(c);return r.useMemo((function(){return"function"==typeof e?e(t):{...t,...e}}),[t,e])}function s(e){let t;return t=e.disableParentContext?"function"==typeof e.components?e.components(i):e.components||i:o(e.components),r.createElement(c.Provider,{value:t},e.children)}}}]);