"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[92425],{66138:(t,e,n)=>{n.r(e),n.d(e,{assets:()=>c,contentTitle:()=>o,default:()=>d,frontMatter:()=>s,metadata:()=>a,toc:()=>h});var r=n(74848),i=n(28453);const s={},o="\u4ec0\u4e48\u662f\u56fe",a={id:"introduction/what-is-graph",title:"\u4ec0\u4e48\u662f\u56fe",description:"\u672c\u6587\u9762\u5411\u521d\u5b66\u8005\uff0c\u4ecb\u7ecd\u56fe\uff08Graph\uff09\u7684\u57fa\u672c\u6982\u5ff5\u3002",source:"@site/versions/version-3.6.0/zh-CN/source/2.introduction/1.what-is-graph.md",sourceDirName:"2.introduction",slug:"/introduction/what-is-graph",permalink:"/tugraph-db/zh/3.6.0/introduction/what-is-graph",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:1,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"\u6587\u6863\u5730\u56fe",permalink:"/tugraph-db/zh/3.6.0/guide"},next:{title:"\u4ec0\u4e48\u662f\u56fe\u6570\u636e\u5e93",permalink:"/tugraph-db/zh/3.6.0/introduction/what-is-gdbms"}},c={},h=[];function u(t){const e={blockquote:"blockquote",h1:"h1",header:"header",img:"img",p:"p",...(0,i.R)(),...t.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(e.header,{children:(0,r.jsx)(e.h1,{id:"\u4ec0\u4e48\u662f\u56fe",children:"\u4ec0\u4e48\u662f\u56fe"})}),"\n",(0,r.jsxs)(e.blockquote,{children:["\n",(0,r.jsx)(e.p,{children:"\u672c\u6587\u9762\u5411\u521d\u5b66\u8005\uff0c\u4ecb\u7ecd\u56fe\uff08Graph\uff09\u7684\u57fa\u672c\u6982\u5ff5\u3002"}),"\n"]}),"\n",(0,r.jsx)(e.p,{children:"\u6211\u4eec\u4eca\u5929\u4ecb\u7ecd\u7684\u56fe\uff0c\u662f\u56fe\u8bba\u4e2d\u4f7f\u7528\u70b9\u548c\u8fb9\u8868\u793a\u7684\u56fe\uff08Graph\uff09\uff0c\u800c\u975e\u56fe\u50cf\u7684\u56fe\uff08Image\uff09\u3002"}),"\n",(0,r.jsx)(e.p,{children:(0,r.jsx)(e.img,{alt:"alt what is graph",src:n(23223).A+"",width:"326",height:"189"})}),"\n",(0,r.jsx)(e.p,{children:"\u56fe\u7684\u57fa\u672c\u5143\u7d20\u662f\u70b9\u548c\u8fb9\uff0c\u5176\u4e2d\u70b9\u8868\u793a\u4e8b\u7269\u6216\u5b9e\u4f53\uff0c\u8fb9\u8868\u793a\u70b9\u4e4b\u95f4\u7684\u5173\u8054\u5173\u7cfb\u3002"}),"\n",(0,r.jsx)(e.p,{children:"\u5982\u4e0a\u56fe\u5de6\u4fa7\u6240\u793a\uff0c\u70b9\u8868\u793a\u7684\u6709\u516c\u53f8\u3001\u5458\u5de5\u3001\u9879\u76ee\u3002\u8fb9\u8868\u793a\u7684\u662f\u4ed6\u4eec\u4e4b\u95f4\u7684\u5173\u7cfb\uff0c\u5305\u62ec\uff1a\u516c\u53f8\u548c\u5458\u5de5\u4e4b\u95f4\u7684\u96c7\u4f63\u5173\u7cfb\uff0c\u5458\u5de5\u548c\u5458\u5de5\u4e4b\u95f4\u7684\u597d\u53cb\u5173\u7cfb\uff0c\u9879\u76ee\u548c\u5458\u5de5\u4e4b\u95f4\u7684\u53c2\u4e0e\u5173\u7cfb\u3002\u9664\u6b64\u4e4b\u5916\uff0c\u70b9\u548c\u8fb9\u4e0a\u53ef\u4ee5\u9644\u52a0\u5c5e\u6027\uff0c\u6bd4\u5982\u5458\u5de5\u7684\u5de5\u53f7\uff0c\u96c7\u4f63\u7684\u65f6\u95f4\uff0c\u8fd9\u6837\u7684\u56fe\u662f\u5c5e\u6027\u56fe\u3002\u4e5f\u5c31\u662f\u8bf4\uff0c\u6211\u4eec\u53ef\u4ee5\u7528\u56fe\u7684\u65b9\u5f0f\u6765\u62bd\u8c61\u5730\u8868\u793a\u5b9e\u4f53\u53ca\u5176\u5173\u8054\u5173\u7cfb\uff0c\u56fe\u6709\u975e\u5e38\u4e30\u5bcc\u7684\u8868\u8fbe\u80fd\u529b\u3002"}),"\n",(0,r.jsx)(e.p,{children:"\u9664\u4e86\u4e0a\u8ff0\u7684\u5458\u5de5\u56fe\u8c31\uff0c\u56fe\u8fd8\u53ef\u4ee5\u7528\u4e8e\u91d1\u878d\u3001\u5de5\u4e1a\u3001\u533b\u7597\u7b49\u5404\u4e2a\u9886\u57df\u3002\u5728\u5b9e\u9645\u5e94\u7528\u4e2d\uff0c\u56fe\u7684\u89c4\u6a21\u8d8a\u6765\u8d8a\u5927\uff0c\u6bd4\u5982\u91d1\u878d\u4ea4\u6613\u56fe\uff0c\u70b9\u8fb9\u89c4\u6a21\u53ef\u80fd\u5230\u8fbe\u767e\u4ebf\uff0c\u4e0e\u5176\u540c\u65f6\uff0c\u57fa\u4e8e\u56fe\u7684\u67e5\u8be2\u901a\u5e38\u4f1a\u590d\u6742\uff0c\u6700\u5178\u578b\u7684\u662fK\u8df3\u67e5\u8be2\uff0c\u6bcf\u589e\u52a0\u4e00\u8df3\u8bbf\u95ee\u7684\u6570\u636e\u90fd\u5448\u6307\u6570\u589e\u957f\u3002"})]})}function d(t={}){const{wrapper:e}={...(0,i.R)(),...t.components};return e?(0,r.jsx)(e,{...t,children:(0,r.jsx)(u,{...t})}):u(t)}},23223:(t,e,n)=>{n.d(e,{A:()=>r});const r=n.p+"assets/images/what-is-graph-6b293bb23abcbbf3f4e4642faed10f59.png"},28453:(t,e,n)=>{n.d(e,{R:()=>o,x:()=>a});var r=n(96540);const i={},s=r.createContext(i);function o(t){const e=r.useContext(s);return r.useMemo((function(){return"function"==typeof t?t(e):{...e,...t}}),[e,t])}function a(t){let e;return e=t.disableParentContext?"function"==typeof t.components?t.components(i):t.components||i:o(t.components),r.createElement(s.Provider,{value:e},t.children)}}}]);