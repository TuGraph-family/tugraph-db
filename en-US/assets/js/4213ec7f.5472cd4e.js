"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[36134],{28453:(n,r,e)=>{e.d(r,{R:()=>i,x:()=>c});var t=e(96540);const h={},s=t.createContext(h);function i(n){const r=t.useContext(s);return t.useMemo((function(){return"function"==typeof n?n(r):{...r,...n}}),[r,n])}function c(n){let r;return r=n.disableParentContext?"function"==typeof n.components?n.components(h):n.components||h:i(n.components),t.createElement(s.Provider,{value:r},n.children)}},67061:(n,r,e)=>{e.r(r),e.d(r,{assets:()=>a,contentTitle:()=>i,default:()=>l,frontMatter:()=>s,metadata:()=>c,toc:()=>d});var t=e(74848),h=e(28453);const s={},i="\u6587\u6863\u5730\u56fe",c={id:"guide",title:"\u6587\u6863\u5730\u56fe",description:"\u8fd9\u91cc\u662f\u6587\u6863\u5730\u56fe\uff0c\u5e2e\u52a9\u7528\u6237\u5feb\u901f\u5b66\u4e60\u548c\u4f7f\u7528TuGraph\u793e\u533a\u7248\u3002",source:"@site/versions/version-4.3.1/zh-CN/source/1.guide.md",sourceDirName:".",slug:"/guide",permalink:"/tugraph-db/en-US/zh/4.3.1/guide",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:1,frontMatter:{},sidebar:"tutorialSidebar",next:{title:"\u4ec0\u4e48\u662f\u56fe",permalink:"/tugraph-db/en-US/zh/4.3.1/introduction/what-is-graph"}},a={},d=[{value:"\u5feb\u901f\u4e0a\u624b",id:"\u5feb\u901f\u4e0a\u624b",level:2},{value:"\u5f00\u53d1\u6307\u5357",id:"\u5f00\u53d1\u6307\u5357",level:2},{value:"\u53c2\u4e0e\u793e\u533a\u8d21\u732e",id:"\u53c2\u4e0e\u793e\u533a\u8d21\u732e",level:2},{value:"\u4e3b\u8981\u4ed3\u5e93",id:"\u4e3b\u8981\u4ed3\u5e93",level:2},{value:"\u89c6\u9891\u4e2d\u5fc3",id:"\u89c6\u9891\u4e2d\u5fc3",level:2},{value:"TuGraph\u6700\u65b0\u7248\u672c",id:"tugraph\u6700\u65b0\u7248\u672c",level:2}];function u(n){const r={a:"a",blockquote:"blockquote",h1:"h1",h2:"h2",header:"header",p:"p",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",...(0,h.R)(),...n.components};return(0,t.jsxs)(t.Fragment,{children:[(0,t.jsx)(r.header,{children:(0,t.jsx)(r.h1,{id:"\u6587\u6863\u5730\u56fe",children:"\u6587\u6863\u5730\u56fe"})}),"\n",(0,t.jsxs)(r.blockquote,{children:["\n",(0,t.jsx)(r.p,{children:"\u8fd9\u91cc\u662f\u6587\u6863\u5730\u56fe\uff0c\u5e2e\u52a9\u7528\u6237\u5feb\u901f\u5b66\u4e60\u548c\u4f7f\u7528TuGraph\u793e\u533a\u7248\u3002"}),"\n"]}),"\n",(0,t.jsx)(r.h2,{id:"\u5feb\u901f\u4e0a\u624b",children:"\u5feb\u901f\u4e0a\u624b"}),"\n",(0,t.jsxs)(r.blockquote,{children:["\n",(0,t.jsxs)(r.p,{children:["\u53ef\u4ee5\u5148\u4e86\u89e3",(0,t.jsx)(r.a,{href:"/tugraph-db/en-US/zh/4.3.1/introduction/what-is-graph",children:"\u4ec0\u4e48\u662f\u56fe"}),"\u3001\u56fe",(0,t.jsx)(r.a,{href:"/tugraph-db/en-US/zh/4.3.1/introduction/scenarios",children:"\u53ef\u4ee5\u505a\u4ec0\u4e48"}),"\u3001\u4ee5\u53ca",(0,t.jsx)(r.a,{href:"/tugraph-db/en-US/zh/4.3.1/introduction/what-is-tugraph",children:"\u4ec0\u4e48\u662fTuGraph"}),"\u3002"]}),"\n"]}),"\n",(0,t.jsxs)(r.blockquote,{children:["\n",(0,t.jsxs)(r.p,{children:["\u57fa\u4e8e",(0,t.jsx)(r.a,{href:"/tugraph-db/en-US/zh/4.3.1/installation&running/cloud-deployment",children:"\u963f\u91cc\u4e91\u8ba1\u7b97\u5de2"}),"\u6216",(0,t.jsx)(r.a,{href:"/tugraph-db/en-US/zh/4.3.1/installation&running/docker-deployment",children:"Docker\u65b9\u5f0f"}),"\u5feb\u901f\u90e8\u7f72TuGraph\u3002"]}),"\n"]}),"\n",(0,t.jsxs)(r.blockquote,{children:["\n",(0,t.jsxs)(r.p,{children:["\u901a\u8fc7\u4ea7\u54c1\u5185\u7f6e\u7684\u573a\u666f\u4e0a\u624b\u4f53\u9a8c\uff1a",(0,t.jsx)(r.a,{href:"/tugraph-db/en-US/zh/4.3.1/quick-start/demo/movie",children:"\u7535\u5f71"}),"\u3001",(0,t.jsx)(r.a,{href:"/tugraph-db/en-US/zh/4.3.1/quick-start/demo/wandering-earth",children:"\u6d41\u6d6a\u5730\u7403"}),"\u3001",(0,t.jsx)(r.a,{href:"/tugraph-db/en-US/zh/4.3.1/quick-start/demo/the-three-body",children:"\u4e09\u4f53"}),"\u3001",(0,t.jsx)(r.a,{href:"/tugraph-db/en-US/zh/4.3.1/quick-start/demo/three-kingdoms",children:"\u4e09\u56fd"}),"\u3002"]}),"\n"]}),"\n",(0,t.jsxs)(r.blockquote,{children:["\n",(0,t.jsxs)(r.p,{children:["\u53ef\u89c6\u5316\u64cd\u4f5c\u624b\u518c\u770b\u8fd9\u91cc\uff1a",(0,t.jsx)(r.a,{href:"/tugraph-db/en-US/zh/4.3.1/user-guide/tugraph-browser",children:"\u53ef\u89c6\u5316\u64cd\u4f5c\u624b\u518c"}),"\u3001",(0,t.jsx)(r.a,{href:"/tugraph-db/en-US/zh/4.3.1/user-guide/tugraph-browser-legacy",children:"\u53ef\u89c6\u5316\u64cd\u4f5c\u624b\u518c\uff08\u65e7\u7248\uff09"}),"\u3002"]}),"\n"]}),"\n",(0,t.jsx)(r.h2,{id:"\u5f00\u53d1\u6307\u5357",children:"\u5f00\u53d1\u6307\u5357"}),"\n",(0,t.jsxs)(r.blockquote,{children:["\n",(0,t.jsxs)(r.p,{children:["\u901a\u8fc7\u5ba2\u6237\u7aef\u8bbf\u95eeTuGraph\uff1a",(0,t.jsx)(r.a,{href:"/tugraph-db/en-US/zh/4.3.1/client-tools/bolt-client",children:"Bolt Client"})]}),"\n"]}),"\n",(0,t.jsxs)(r.blockquote,{children:["\n",(0,t.jsxs)(r.p,{children:["\u67e5\u8be2\u8bed\u8a00\u3001\u8bed\u53e5\u521b\u5efa\u56fe\u6a21\u578b\u770b\u8fd9\u91cc\uff1a",(0,t.jsx)(r.a,{href:"/tugraph-db/en-US/zh/4.3.1/query/cypher",children:"Cypher API"}),"."]}),"\n"]}),"\n",(0,t.jsxs)(r.blockquote,{children:["\n",(0,t.jsxs)(r.p,{children:["\u5b58\u50a8\u8fc7\u7a0b\u548c\u7b97\u6cd5\u4ecb\u7ecd\u770b\u8fd9\u91cc\uff1a",(0,t.jsx)(r.a,{href:"/tugraph-db/en-US/zh/4.3.1/olap&procedure/procedure/",children:"Procedure API (POG API)"}),"\u3001",(0,t.jsx)(r.a,{href:"/tugraph-db/en-US/zh/4.3.1/olap&procedure/olap/tutorial",children:"OLAP API"}),"\u3002"]}),"\n"]}),"\n",(0,t.jsxs)(r.blockquote,{children:["\n",(0,t.jsxs)(r.p,{children:["C++/Python\u5f00\u53d1\u5b58\u50a8\u8fc7\u7a0b\u63a5\u53e3\u770b\u8fd9\u91cc\uff1a",(0,t.jsx)(r.a,{target:"_blank","data-noBrokenLinkCheck":!0,href:e(86715).A+"",children:"C++/Python Procedure API"}),"\u3002"]}),"\n"]}),"\n",(0,t.jsx)(r.h2,{id:"\u53c2\u4e0e\u793e\u533a\u8d21\u732e",children:"\u53c2\u4e0e\u793e\u533a\u8d21\u732e"}),"\n",(0,t.jsxs)(r.blockquote,{children:["\n",(0,t.jsxs)(r.p,{children:["\u5728\u5f00\u59cb\u8d21\u732e\u524d\uff0c\u53ef\u4ee5\u5148\u4e86\u89e3",(0,t.jsx)(r.a,{href:"/tugraph-db/en-US/zh/4.3.1/contributor-manual/contributing",children:"\u5982\u4f55\u8d21\u732e"})]}),"\n"]}),"\n",(0,t.jsxs)(r.blockquote,{children:["\n",(0,t.jsxs)(r.p,{children:["\u5982\u679c\u60f3\u4e86\u89e3\u793e\u533a\u89d2\u8272\u7684\u5212\u5206\uff0c\u8bf7\u8bbf\u95ee",(0,t.jsx)(r.a,{href:"/tugraph-db/en-US/zh/4.3.1/contributor-manual/community-roles",children:"\u793e\u533a\u89d2\u8272"})]}),"\n"]}),"\n",(0,t.jsx)(r.h2,{id:"\u4e3b\u8981\u4ed3\u5e93",children:"\u4e3b\u8981\u4ed3\u5e93"}),"\n",(0,t.jsxs)(r.blockquote,{children:["\n",(0,t.jsxs)(r.p,{children:["TuGraph-DB \u4ed3\u5e93: ",(0,t.jsx)(r.a,{href:"https://github.com/TuGraph-family/tugraph-db",children:"https://github.com/TuGraph-family/tugraph-db"})]}),"\n"]}),"\n",(0,t.jsxs)(r.blockquote,{children:["\n",(0,t.jsxs)(r.p,{children:["\u53ef\u89c6\u5316\u754c\u9762: ",(0,t.jsx)(r.a,{href:"https://github.com/TuGraph-family/tugraph-db-browser",children:"https://github.com/TuGraph-family/tugraph-db-browser"})]}),"\n"]}),"\n",(0,t.jsxs)(r.blockquote,{children:["\n",(0,t.jsxs)(r.p,{children:["\u57fa\u4e8etwitter\u6570\u636e\u7684\u7b80\u5355\u6d4b\u8bd5\u65b9\u6cd5: ",(0,t.jsx)(r.a,{href:"https://github.com/TuGraph-family/gdbms-microbenchmark",children:"https://github.com/TuGraph-family/gdbms-microbenchmark"})]}),"\n"]}),"\n",(0,t.jsxs)(r.blockquote,{children:["\n",(0,t.jsxs)(r.p,{children:["\u57fa\u4e8e\u6807\u51c6LDBC-SNB\u7684\u6d4b\u8bd5\u65b9\u6cd5: ",(0,t.jsx)(r.a,{href:"https://github.com/TuGraph-family/tugraph-snb-interactive",children:"https://github.com/TuGraph-family/tugraph-snb-interactive"})]}),"\n"]}),"\n",(0,t.jsxs)(r.blockquote,{children:["\n",(0,t.jsxs)(r.p,{children:["TuGraph-Analytics \u4ed3\u5e93: ",(0,t.jsx)(r.a,{href:"https://github.com/TuGraph-family/tugraph-analytics",children:"https://github.com/TuGraph-family/tugraph-analytics"})]}),"\n"]}),"\n",(0,t.jsx)(r.h2,{id:"\u89c6\u9891\u4e2d\u5fc3",children:"\u89c6\u9891\u4e2d\u5fc3"}),"\n",(0,t.jsxs)(r.blockquote,{children:["\n",(0,t.jsx)(r.p,{children:(0,t.jsx)(r.a,{href:"https://space.bilibili.com/1196053065/channel/seriesdetail?sid=2593741",children:"TuGraph\u5feb\u901f\u4e0a\u624b"})}),"\n"]}),"\n",(0,t.jsxs)(r.blockquote,{children:["\n",(0,t.jsx)(r.p,{children:(0,t.jsx)(r.a,{href:"https://space.bilibili.com/1196053065/channel/seriesdetail?sid=3009777",children:"TuGraph\u6280\u672f\u5206\u4eab\u96c6\u5408"})}),"\n"]}),"\n",(0,t.jsxs)(r.blockquote,{children:["\n",(0,t.jsx)(r.p,{children:(0,t.jsx)(r.a,{href:"https://www.bilibili.com/video/BV15U4y1r7AW/",children:"3\u5206\u949f\u8bfb\u61c2\u56fe\u8ba1\u7b97"})}),"\n"]}),"\n",(0,t.jsx)(r.h2,{id:"tugraph\u6700\u65b0\u7248\u672c",children:"TuGraph\u6700\u65b0\u7248\u672c"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,t.jsxs)(r.table,{children:[(0,t.jsx)(r.thead,{children:(0,t.jsxs)(r.tr,{children:[(0,t.jsx)(r.th,{children:"\u63cf\u8ff0"}),(0,t.jsx)(r.th,{children:"\u6587\u4ef6"}),(0,t.jsx)(r.th,{children:"\u94fe\u63a5"})]})}),(0,t.jsxs)(r.tbody,{children:[(0,t.jsxs)(r.tr,{children:[(0,t.jsx)(r.td,{children:"CentOS7 \u5b89\u88c5\u5305"}),(0,t.jsx)(r.td,{children:"tugraph-4.3.1-1.el7.x86_64.rpm"}),(0,t.jsx)(r.td,{children:(0,t.jsx)(r.a,{href:"https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/tugraph-4.3.1/tugraph-4.3.1-1.el7.x86_64.rpm",children:"\u4e0b\u8f7d"})})]}),(0,t.jsxs)(r.tr,{children:[(0,t.jsx)(r.td,{children:"CentOS8 \u5b89\u88c5\u5305"}),(0,t.jsx)(r.td,{children:"tugraph-4.3.1-1.el8.x86_64.rpm"}),(0,t.jsx)(r.td,{children:(0,t.jsx)(r.a,{href:"https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/tugraph-4.3.1/tugraph-4.3.1-1.el8.x86_64.rpm",children:"\u4e0b\u8f7d"})})]}),(0,t.jsxs)(r.tr,{children:[(0,t.jsx)(r.td,{children:"Ubuntu18.04 \u5b89\u88c5\u5305"}),(0,t.jsx)(r.td,{children:"tugraph-4.3.1-1.x86_64.deb"}),(0,t.jsx)(r.td,{children:(0,t.jsx)(r.a,{href:"https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/tugraph-4.3.1/tugraph-4.3.1-1.x86_64.deb",children:"\u4e0b\u8f7d"})})]}),(0,t.jsxs)(r.tr,{children:[(0,t.jsx)(r.td,{children:"CentOS7 \u9884\u5b89\u88c5\u955c\u50cf"}),(0,t.jsx)(r.td,{children:"tugraph-runtime-centos7-4.3.1.tar"}),(0,t.jsxs)(r.td,{children:[(0,t.jsx)(r.a,{href:"https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/tugraph-4.3.1/tugraph-runtime-centos7-4.3.1.tar",children:"\u4e0b\u8f7d"})," \u3001",(0,t.jsx)(r.a,{href:"https://hub.docker.com/r/tugraph/tugraph-runtime-centos7",children:"\u8bbf\u95ee"})]})]}),(0,t.jsxs)(r.tr,{children:[(0,t.jsx)(r.td,{children:"CentOS8 \u9884\u5b89\u88c5\u955c\u50cf"}),(0,t.jsx)(r.td,{children:"tugraph-runtime-centos8-4.3.1.tar"}),(0,t.jsxs)(r.td,{children:[(0,t.jsx)(r.a,{href:"https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/tugraph-4.3.1/tugraph-runtime-centos8-4.3.1.tar",children:"\u4e0b\u8f7d"})," \u3001",(0,t.jsx)(r.a,{href:"https://hub.docker.com/r/tugraph/tugraph-runtime-centos8",children:"\u8bbf\u95ee"})]})]}),(0,t.jsxs)(r.tr,{children:[(0,t.jsx)(r.td,{children:"Ubuntu18.04 \u9884\u5b89\u88c5\u955c\u50cf"}),(0,t.jsx)(r.td,{children:"tugraph-runtime-ubuntu18.04-4.3.1.tar"}),(0,t.jsxs)(r.td,{children:[(0,t.jsx)(r.a,{href:"https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/tugraph-4.3.1/tugraph-runtime-ubuntu18.04-4.3.1.tar",children:"\u4e0b\u8f7d"})," \u3001",(0,t.jsx)(r.a,{href:"https://hub.docker.com/r/tugraph/tugraph-runtime-ubuntu18.04",children:"\u8bbf\u95ee"})]})]}),(0,t.jsxs)(r.tr,{children:[(0,t.jsx)(r.td,{children:"CentOS7 \u7cbe\u7b80\u5b89\u88c5\u5305"}),(0,t.jsx)(r.td,{children:"tugraph-mini-4.3.1-1.el7.x86_64.rpm"}),(0,t.jsx)(r.td,{children:(0,t.jsx)(r.a,{href:"https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/tugraph-4.3.1/tugraph-mini-4.3.1-1.el7.x86_64.rpm",children:"\u4e0b\u8f7d"})})]}),(0,t.jsxs)(r.tr,{children:[(0,t.jsx)(r.td,{children:"CentOS8 \u7cbe\u7b80\u5b89\u88c5\u5305"}),(0,t.jsx)(r.td,{children:"tugraph-mini-4.3.1-1.el8.x86_64.rpm"}),(0,t.jsx)(r.td,{children:(0,t.jsx)(r.a,{href:"https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/tugraph-4.3.1/tugraph-mini-4.3.1-1.el8.x86_64.rpm",children:"\u4e0b\u8f7d"})})]}),(0,t.jsxs)(r.tr,{children:[(0,t.jsx)(r.td,{children:"Ubuntu18.04 \u7cbe\u7b80\u5b89\u88c5\u5305"}),(0,t.jsx)(r.td,{children:"tugraph-mini-4.3.1-1.x86_64.deb"}),(0,t.jsx)(r.td,{children:(0,t.jsx)(r.a,{href:"https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/tugraph-4.3.1/tugraph-mini-4.3.1-1.x86_64.deb",children:"\u4e0b\u8f7d"})})]}),(0,t.jsxs)(r.tr,{children:[(0,t.jsx)(r.td,{children:"CentOS7 \u7cbe\u7b80\u9884\u5b89\u88c5\u955c\u50cf"}),(0,t.jsx)(r.td,{children:"tugraph-mini-runtime-centos7-4.3.1.tar"}),(0,t.jsxs)(r.td,{children:[(0,t.jsx)(r.a,{href:"https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/tugraph-4.3.1/tugraph-mini-runtime-centos7-4.3.1.tar",children:"\u4e0b\u8f7d"})," \u3001",(0,t.jsx)(r.a,{href:"https://hub.docker.com/r/tugraph/tugraph-mini-runtime-centos7",children:"\u8bbf\u95ee"})]})]}),(0,t.jsxs)(r.tr,{children:[(0,t.jsx)(r.td,{children:"CentOS8 \u7cbe\u7b80\u9884\u5b89\u88c5\u955c\u50cf"}),(0,t.jsx)(r.td,{children:"tugraph-mini-runtime-centos8-4.3.1.tar"}),(0,t.jsxs)(r.td,{children:[(0,t.jsx)(r.a,{href:"https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/tugraph-4.3.1/tugraph-mini-runtime-centos8-4.3.1.tar",children:"\u4e0b\u8f7d"})," \u3001",(0,t.jsx)(r.a,{href:"https://hub.docker.com/r/tugraph/tugraph-mini-runtime-centos8",children:"\u8bbf\u95ee"})]})]}),(0,t.jsxs)(r.tr,{children:[(0,t.jsx)(r.td,{children:"Ubuntu18.04 \u7cbe\u7b80\u9884\u5b89\u88c5\u955c\u50cf"}),(0,t.jsx)(r.td,{children:"tugraph-mini-runtime-ubuntu18.04-4.3.1.tar"}),(0,t.jsxs)(r.td,{children:[(0,t.jsx)(r.a,{href:"https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/tugraph-4.3.1/tugraph-mini-runtime-ubuntu18.04-4.3.1.tar",children:"\u4e0b\u8f7d"})," \u3001",(0,t.jsx)(r.a,{href:"https://hub.docker.com/r/tugraph/tugraph-mini-runtime-ubuntu18.04",children:"\u8bbf\u95ee"})]})]}),(0,t.jsxs)(r.tr,{children:[(0,t.jsx)(r.td,{children:"CentOS7 \u7f16\u8bd1\u955c\u50cf"}),(0,t.jsx)(r.td,{children:"tugraph-compile-centos7-1.3.2.tar"}),(0,t.jsxs)(r.td,{children:[(0,t.jsx)(r.a,{href:"https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/tugraph-docker-compile/tugraph-compile-centos7-1.3.2.tar",children:"\u4e0b\u8f7d"})," \u3001",(0,t.jsx)(r.a,{href:"https://hub.docker.com/r/tugraph/tugraph-compile-centos7",children:"\u8bbf\u95ee"})]})]}),(0,t.jsxs)(r.tr,{children:[(0,t.jsx)(r.td,{children:"CentOS8 \u7f16\u8bd1\u955c\u50cf"}),(0,t.jsx)(r.td,{children:"tugraph-compile-centos8-1.3.2.tar"}),(0,t.jsxs)(r.td,{children:[(0,t.jsx)(r.a,{href:"https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/tugraph-docker-compile/tugraph-compile-centos8-1.3.2.tar",children:"\u4e0b\u8f7d"})," \u3001",(0,t.jsx)(r.a,{href:"https://hub.docker.com/r/tugraph/tugraph-compile-centos8",children:"\u8bbf\u95ee"})]})]}),(0,t.jsxs)(r.tr,{children:[(0,t.jsx)(r.td,{children:"Ubuntu18.04 \u7f16\u8bd1\u955c\u50cf"}),(0,t.jsx)(r.td,{children:"tugraph-compile-ubuntu18.04-1.3.2.tar"}),(0,t.jsxs)(r.td,{children:[(0,t.jsx)(r.a,{href:"https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/tugraph-docker-compile/tugraph-compile-ubuntu18.04-1.3.2.tar",children:"\u4e0b\u8f7d"})," \u3001",(0,t.jsx)(r.a,{href:"https://hub.docker.com/r/tugraph/tugraph-compile-ubuntu18.04",children:"\u8bbf\u95ee"})]})]})]})]}),"\n",(0,t.jsxs)(r.p,{children:["\u7248\u672c\u66f4\u65b0\u65e5\u5fd7\u89c1\uff1a",(0,t.jsx)(r.a,{href:"https://github.com/TuGraph-family/tugraph-db/blob/master/release/CHANGELOG_CN.md",children:"\u94fe\u63a5"}),"\u3002"]}),"\n",(0,t.jsxs)(r.p,{children:["\u5982\u679c\u60a8\u4e0d\u6e05\u695a\u4f7f\u7528\u5b89\u88c5\u5305\u548c\u955c\u50cf\uff0c\u8bf7\u53c2\u8003 ",(0,t.jsx)(r.a,{href:"/tugraph-db/en-US/zh/4.3.1/best-practices/selection",children:"\u73af\u5883\u548c\u7248\u672c\u9009\u62e9"}),"\u3002"]})]})}function l(n={}){const{wrapper:r}={...(0,h.R)(),...n.components};return r?(0,t.jsx)(r,{...n,children:(0,t.jsx)(u,{...n})}):u(n)}},86715:(n,r,e)=>{e.d(r,{A:()=>t});const t=e.p+"assets/files/index-07adf872280840868ada97d8aeecb891.rst"}}]);