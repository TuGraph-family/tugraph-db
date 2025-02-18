"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[99885],{28453:(e,n,r)=>{r.d(n,{R:()=>o,x:()=>s});var t=r(96540);const l={},a=t.createContext(l);function o(e){const n=t.useContext(a);return t.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function s(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(l):e.components||l:o(e.components),t.createElement(a.Provider,{value:n},e.children)}},86367:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>i,contentTitle:()=>o,default:()=>u,frontMatter:()=>a,metadata:()=>s,toc:()=>c});var t=r(74848),l=r(28453);const a={},o="\u672c\u5730\u5305\u90e8\u7f72",s={id:"developer-manual/installation/local-package-deployment",title:"\u672c\u5730\u5305\u90e8\u7f72",description:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph \u7684\u672c\u5730\u5305\u90e8\u7f72\u3002",source:"@site/versions/version-3.5.1/zh-CN/source/5.developer-manual/1.installation/4.local-package-deployment.md",sourceDirName:"5.developer-manual/1.installation",slug:"/developer-manual/installation/local-package-deployment",permalink:"/tugraph-db/zh/3.5.1/developer-manual/installation/local-package-deployment",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:4,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Docker\u90e8\u7f72",permalink:"/tugraph-db/zh/3.5.1/developer-manual/installation/docker-deployment"},next:{title:"\u4e91\u90e8\u7f72",permalink:"/tugraph-db/zh/3.5.1/developer-manual/installation/cloud-deployment"}},i={},c=[{value:"1.\u5b89\u88c5\u5305\u4e0b\u8f7d",id:"1\u5b89\u88c5\u5305\u4e0b\u8f7d",level:2},{value:"2.Ubuntu \u4e0b\u7684\u5b89\u88c5\u65b9\u6cd5",id:"2ubuntu-\u4e0b\u7684\u5b89\u88c5\u65b9\u6cd5",level:2},{value:"3.CentOS \u4e0b\u7684\u5b89\u88c5\u65b9\u6cd5",id:"3centos-\u4e0b\u7684\u5b89\u88c5\u65b9\u6cd5",level:2}];function d(e){const n={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",header:"header",p:"p",pre:"pre",...(0,l.R)(),...e.components};return(0,t.jsxs)(t.Fragment,{children:[(0,t.jsx)(n.header,{children:(0,t.jsx)(n.h1,{id:"\u672c\u5730\u5305\u90e8\u7f72",children:"\u672c\u5730\u5305\u90e8\u7f72"})}),"\n",(0,t.jsxs)(n.blockquote,{children:["\n",(0,t.jsx)(n.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph \u7684\u672c\u5730\u5305\u90e8\u7f72\u3002"}),"\n"]}),"\n",(0,t.jsx)(n.h2,{id:"1\u5b89\u88c5\u5305\u4e0b\u8f7d",children:"1.\u5b89\u88c5\u5305\u4e0b\u8f7d"}),"\n",(0,t.jsxs)(n.p,{children:["\u8bf7\u8bbf\u95eeGithub\u8fdb\u884c\u4e0b\u8f7d\uff1a",(0,t.jsx)(n.a,{href:"https://github.com/TuGraph-family/tugraph-db/releases",children:"TuGraph Download"})]}),"\n",(0,t.jsx)(n.h2,{id:"2ubuntu-\u4e0b\u7684\u5b89\u88c5\u65b9\u6cd5",children:"2.Ubuntu \u4e0b\u7684\u5b89\u88c5\u65b9\u6cd5"}),"\n",(0,t.jsx)(n.p,{children:"\u7528\u4e8e\u5728 Ubuntu \u4e0a\u5b89\u88c5\u7684 TuGraph \u7684.deb \u5b89\u88c5\u5305\uff0c\u5176\u4e2d\u5305\u542b\u4e86 TuGraph \u53ef\u6267\u884c\u6587\u4ef6\u4ee5\u53ca\u7f16\u5199\u5d4c\u5165\u5f0f\u7a0b\u5e8f\u548c\u5b58\u50a8\u8fc7\u7a0b\u6240\u9700\u7684\u5934\u6587\u4ef6\u548c\u76f8\u5173\u5e93\u6587\u4ef6\u3002"}),"\n",(0,t.jsxs)(n.p,{children:["\u4f7f\u7528\u5df2\u7ecf\u4e0b\u8f7d\u5b8c\u6210\u7684",(0,t.jsx)(n.code,{children:"tugraph_x.y.z.deb"}),"\u5b89\u88c5\u5305\u5728\u7ec8\u7aef\u4e0b\u5b89\u88c5\uff0c\u53ea\u9700\u8981\u8fd0\u884c\u4ee5\u4e0b\u547d\u4ee4\uff1a"]}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{className:"language-shell",children:"$ sudo dpkg -i tugraph-x.y.z.deb\n"})}),"\n",(0,t.jsxs)(n.p,{children:["\u8be5\u547d\u4ee4\u9ed8\u8ba4\u5c06 TuGraph \u5b89\u88c5\u4e8e",(0,t.jsx)(n.code,{children:"/usr/local"}),"\u76ee\u5f55\u4e0b\u3002\u7528\u6237\u4e5f\u53ef\u4ee5\u901a\u8fc7\u6307\u5b9a ",(0,t.jsx)(n.code,{children:"--instdir=<directory>"})," \u9009\u9879\u66f4\u6539\u5b89\u88c5\u76ee\u5f55\u3002"]}),"\n",(0,t.jsx)(n.h2,{id:"3centos-\u4e0b\u7684\u5b89\u88c5\u65b9\u6cd5",children:"3.CentOS \u4e0b\u7684\u5b89\u88c5\u65b9\u6cd5"}),"\n",(0,t.jsx)(n.p,{children:"\u7528\u4e8e\u5728 CentOS \u4e0a\u5b89\u88c5\u7684 TuGraph \u7684.rpm \u5b89\u88c5\u5305\uff0c\u5176\u4e2d\u5305\u542b\u4e86 TuGraph \u53ef\u6267\u884c\u6587\u4ef6\u4ee5\u53ca\u7f16\u5199\u5d4c\u5165\u5f0f\u7a0b\u5e8f\u548c\u5b58\u50a8\u8fc7\u7a0b\u6240\u9700\u7684\u5934\u6587\u4ef6\u548c\u76f8\u5173\u5e93\u6587\u4ef6\u3002"}),"\n",(0,t.jsx)(n.p,{children:"\u4f7f\u7528\u5df2\u7ecf\u4e0b\u8f7d\u5b8c\u6210\u7684`tugraph_x.y.z.rpm \u5b89\u88c5\u5305\u5728\u7ec8\u7aef\u4e0b\u5b89\u88c5\uff0c\u53ea\u9700\u8981\u8fd0\u884c\u4ee5\u4e0b\u547d\u4ee4\uff1a"}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{className:"language-shell",children:"$ rpm -ivh tugraph-x.y.z.rpm\n"})}),"\n",(0,t.jsxs)(n.p,{children:["\u7528\u6237\u4e5f\u53ef\u4ee5\u901a\u8fc7\u6307\u5b9a",(0,t.jsx)(n.code,{children:"--prefix"}),"\u9009\u9879\u6307\u5b9a\u5b89\u88c5\u76ee\u5f55\u3002"]})]})}function u(e={}){const{wrapper:n}={...(0,l.R)(),...e.components};return n?(0,t.jsx)(n,{...e,children:(0,t.jsx)(d,{...e})}):d(e)}}}]);