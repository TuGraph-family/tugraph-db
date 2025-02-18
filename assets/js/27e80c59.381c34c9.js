"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[89736],{28453:(n,e,r)=>{r.d(e,{R:()=>d,x:()=>i});var l=r(96540);const t={},s=l.createContext(t);function d(n){const e=l.useContext(s);return l.useMemo((function(){return"function"==typeof n?n(e):{...e,...n}}),[e,n])}function i(n){let e;return e=n.disableParentContext?"function"==typeof n.components?n.components(t):n.components||t:d(n.components),l.createElement(s.Provider,{value:e},n.children)}},30091:(n,e,r)=>{r.r(e),r.d(e,{assets:()=>h,contentTitle:()=>d,default:()=>o,frontMatter:()=>s,metadata:()=>i,toc:()=>c});var l=r(74848),t=r(28453);const s={},d="\u5feb\u901f\u4e0a\u624b",i={id:"quick-start/preparation",title:"\u5feb\u901f\u4e0a\u624b",description:"\u6b64\u6587\u6863\u4e3b\u8981\u7528\u4e8e\u65b0\u7528\u6237\u5feb\u901f\u4e0a\u624b\uff0c\u5176\u4e2d\u5305\u542b\u4e86 TuGraph \u7684\u7b80\u4ecb\u3001\u7279\u5f81\u3001\u5b89\u88c5\u548c\u4f7f\u7528\u3002",source:"@site/versions/version-4.3.0/zh-CN/source/3.quick-start/1.preparation.md",sourceDirName:"3.quick-start",slug:"/quick-start/preparation",permalink:"/tugraph-db/zh/4.3.0/quick-start/preparation",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:1,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"\u540d\u8bcd\u89e3\u91ca",permalink:"/tugraph-db/zh/4.3.0/introduction/glossary"},next:{title:"\u5f71\u89c6\u573a\u666fDemo",permalink:"/tugraph-db/zh/4.3.0/quick-start/demo/movie"}},h={},c=[{value:"1.\u7b80\u4ecb",id:"1\u7b80\u4ecb",level:2},{value:"1.1.\u652f\u6301\u7684\u5e73\u53f0",id:"11\u652f\u6301\u7684\u5e73\u53f0",level:3},{value:"1.2.\u786c\u4ef6\u8981\u6c42",id:"12\u786c\u4ef6\u8981\u6c42",level:3},{value:"2.\u5b89\u88c5",id:"2\u5b89\u88c5",level:2},{value:"2.1.\u901a\u8fc7docker\u5feb\u901f\u4f53\u9a8c",id:"21\u901a\u8fc7docker\u5feb\u901f\u4f53\u9a8c",level:3}];function a(n){const e={a:"a",blockquote:"blockquote",code:"code",em:"em",h1:"h1",h2:"h2",h3:"h3",header:"header",li:"li",ol:"ol",p:"p",pre:"pre",strong:"strong",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",ul:"ul",...(0,t.R)(),...n.components};return(0,l.jsxs)(l.Fragment,{children:[(0,l.jsx)(e.header,{children:(0,l.jsx)(e.h1,{id:"\u5feb\u901f\u4e0a\u624b",children:"\u5feb\u901f\u4e0a\u624b"})}),"\n",(0,l.jsxs)(e.blockquote,{children:["\n",(0,l.jsx)(e.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u7528\u4e8e\u65b0\u7528\u6237\u5feb\u901f\u4e0a\u624b\uff0c\u5176\u4e2d\u5305\u542b\u4e86 TuGraph \u7684\u7b80\u4ecb\u3001\u7279\u5f81\u3001\u5b89\u88c5\u548c\u4f7f\u7528\u3002"}),"\n"]}),"\n",(0,l.jsx)(e.h2,{id:"1\u7b80\u4ecb",children:"1.\u7b80\u4ecb"}),"\n",(0,l.jsx)(e.p,{children:"TuGraph \u662f\u8682\u8681\u96c6\u56e2\u81ea\u4e3b\u7814\u53d1\u7684\u5927\u89c4\u6a21\u56fe\u8ba1\u7b97\u7cfb\u7edf\uff0c\u63d0\u4f9b\u56fe\u6570\u636e\u5e93\u5f15\u64ce\u548c\u56fe\u5206\u6790\u5f15\u64ce\u3002\u5176\u4e3b\u8981\u7279\u70b9\u662f\u5927\u6570\u636e\u91cf\u5b58\u50a8\u548c\u8ba1\u7b97\uff0c\u9ad8\u541e\u5410\u7387\uff0c\u4ee5\u53ca\u7075\u6d3b\u7684 API\uff0c\u540c\u65f6\u652f\u6301\u9ad8\u6548\u7684\u5728\u7ebf\u4e8b\u52a1\u5904\u7406\uff08OLTP\uff09\u548c\u5728\u7ebf\u5206\u6790\u5904\u7406\uff08OLAP\uff09\u3002 LightGraph\u3001GeaGraph \u662f TuGraph \u7684\u66fe\u7528\u540d\u3002"}),"\n",(0,l.jsx)(e.p,{children:"\u4e3b\u8981\u529f\u80fd\u7279\u5f81\u5305\u62ec\uff1a"}),"\n",(0,l.jsxs)(e.ul,{children:["\n",(0,l.jsx)(e.li,{children:"\u6807\u7b7e\u5c5e\u6027\u56fe\u6a21\u578b"}),"\n",(0,l.jsx)(e.li,{children:"\u652f\u6301\u591a\u56fe"}),"\n",(0,l.jsx)(e.li,{children:"\u5b8c\u5584\u7684 ACID \u4e8b\u52a1\u5904\u7406"}),"\n",(0,l.jsx)(e.li,{children:"\u5185\u7f6e 34 \u56fe\u5206\u6790\u7b97\u6cd5"}),"\n",(0,l.jsx)(e.li,{children:"\u57fa\u4e8e web \u5ba2\u6237\u7aef\u7684\u56fe\u53ef\u89c6\u5316\u5de5\u5177"}),"\n",(0,l.jsx)(e.li,{children:"\u652f\u6301 RESTful API \u548c RPC"}),"\n",(0,l.jsx)(e.li,{children:"OpenCypher \u56fe\u67e5\u8be2\u8bed\u8a00"}),"\n",(0,l.jsx)(e.li,{children:"\u57fa\u4e8e C++/Python \u7684\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,l.jsx)(e.li,{children:"\u9002\u7528\u4e8e\u9ad8\u6548\u56fe\u7b97\u6cd5\u5f00\u53d1\u7684 Traversal API"}),"\n"]}),"\n",(0,l.jsx)(e.p,{children:"\u6027\u80fd\u53ca\u53ef\u6269\u5c55\u6027\u7279\u5f81\u5305\u62ec\uff1a"}),"\n",(0,l.jsxs)(e.ul,{children:["\n",(0,l.jsx)(e.li,{children:"TB \u7ea7\u5927\u5bb9\u91cf"}),"\n",(0,l.jsx)(e.li,{children:"\u5343\u4e07\u70b9/\u79d2\u7684\u9ad8\u541e\u5410\u7387"}),"\n",(0,l.jsx)(e.li,{children:"\u9ad8\u53ef\u7528\u6027\u652f\u6301"}),"\n",(0,l.jsx)(e.li,{children:"\u9ad8\u6027\u80fd\u6279\u91cf\u5bfc\u5165"}),"\n",(0,l.jsx)(e.li,{children:"\u5728\u7ebf/\u79bb\u7ebf\u5907\u4efd"}),"\n"]}),"\n",(0,l.jsx)(e.h3,{id:"11\u652f\u6301\u7684\u5e73\u53f0",children:"1.1.\u652f\u6301\u7684\u5e73\u53f0"}),"\n",(0,l.jsx)(e.p,{children:"TuGraph \u65e0\u8bba\u662f\u7269\u7406\u3001\u865a\u62df\u8fd8\u662f\u5bb9\u5668\u5316\u73af\u5883\uff0c\u5747\u652f\u6301 X86_64 \u548c ARM64 \u67b6\u6784\u7684\u7684\u5e73\u53f0\u3002"}),"\n",(0,l.jsx)(e.h3,{id:"12\u786c\u4ef6\u8981\u6c42",children:"1.2.\u786c\u4ef6\u8981\u6c42"}),"\n",(0,l.jsx)(e.p,{children:(0,l.jsx)(e.em,{children:"\u76ee\u524d\u6211\u4eec\u5efa\u8bae\u7528\u6237\u4f7f\u7528 NVMe SSD \u914d\u5408\u8f83\u5927\u7684\u5185\u5b58\u914d\u7f6e\u4ee5\u83b7\u53d6\u6700\u4f73\u6027\u80fd\u3002"})}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(e.table,{children:[(0,l.jsx)(e.thead,{children:(0,l.jsxs)(e.tr,{children:[(0,l.jsx)(e.th,{children:"\u786c\u4ef6"}),(0,l.jsx)(e.th,{children:"\u6700\u4f4e\u914d\u7f6e"}),(0,l.jsx)(e.th,{children:"\u5efa\u8bae\u914d\u7f6e"})]})}),(0,l.jsxs)(e.tbody,{children:[(0,l.jsxs)(e.tr,{children:[(0,l.jsx)(e.td,{children:"CPU"}),(0,l.jsx)(e.td,{children:"X86_64"}),(0,l.jsx)(e.td,{children:"Xeon E5 2670 v4"})]}),(0,l.jsxs)(e.tr,{children:[(0,l.jsx)(e.td,{children:"\u5185\u5b58"}),(0,l.jsx)(e.td,{children:"4GB"}),(0,l.jsx)(e.td,{children:"256GB"})]}),(0,l.jsxs)(e.tr,{children:[(0,l.jsx)(e.td,{children:"\u786c\u76d8"}),(0,l.jsx)(e.td,{children:"100GB"}),(0,l.jsx)(e.td,{children:"1TB NVMe SSD"})]}),(0,l.jsxs)(e.tr,{children:[(0,l.jsx)(e.td,{children:"\u64cd\u4f5c\u7cfb\u7edf"}),(0,l.jsx)(e.td,{children:"Linux 2.6"}),(0,l.jsx)(e.td,{children:"Ubuntu 18.04, CentOS 7.3"})]})]})]}),"\n",(0,l.jsx)(e.h2,{id:"2\u5b89\u88c5",children:"2.\u5b89\u88c5"}),"\n",(0,l.jsx)(e.p,{children:"TuGraph \u53ef\u4ee5\u901a\u8fc7 Docker Image \u5feb\u901f\u5b89\u88c5\uff0c\u6216\u8005\u901a\u8fc7 rpm/deb \u5305\u672c\u5730\u5b89\u88c5\u3002\u53e6\u5916TuGraph\u5728\u963f\u91cc\u4e91\u8ba1\u7b97\u5de2\u4e0a\u63d0\u4f9b\u4e86\u793e\u533a\u7248\u670d\u52a1\uff0c\u60a8\u65e0\u9700\u81ea\u884c\u8d2d\u7f6e\u4e91\u4e3b\u673a\uff0c\u5373\u53ef\u5728\u8ba1\u7b97\u5de2\u4e0a\u5feb\u901f\u90e8\u7f72TuGraph\u670d\u52a1\u3001\u5b9e\u73b0\u8fd0\u7ef4\u76d1\u63a7\uff0c\u4ece\u800c\u642d\u5efa\u60a8\u81ea\u5df1\u7684\u56fe\u5e94\u7528\u3002"}),"\n",(0,l.jsxs)(e.blockquote,{children:["\n",(0,l.jsxs)(e.p,{children:["\u5b89\u88c5\u5305/\u955c\u50cf\u4e0b\u8f7d\uff1a\u53c2\u8003",(0,l.jsx)(e.a,{href:"/tugraph-db/zh/4.3.0/guide",children:"\u4e0b\u8f7d\u5730\u5740"}),"\u4e2d\u7684\u201cTuGraph\u6700\u65b0\u7248\u672c\u201d\u7ae0\u8282\u3002"]}),"\n"]}),"\n",(0,l.jsxs)(e.blockquote,{children:["\n",(0,l.jsxs)(e.p,{children:["\u8ba1\u7b97\u5de2\u90e8\u7f72\uff1a\u53ef\u4ee5\u5728\u963f\u91cc\u4e91\u8ba1\u7b97\u5de2\u81ea\u884c\u641c\u7d22\uff0c\u4e5f\u53ef\u4ee5\u901a\u8fc7",(0,l.jsx)(e.a,{href:"/tugraph-db/zh/4.3.0/installation&running/cloud-deployment",children:"\u90e8\u7f72\u94fe\u63a5"}),"\u5feb\u901f\u8bbf\u95ee\u3002"]}),"\n"]}),"\n",(0,l.jsx)(e.h3,{id:"21\u901a\u8fc7docker\u5feb\u901f\u4f53\u9a8c",children:"2.1.\u901a\u8fc7docker\u5feb\u901f\u4f53\u9a8c"}),"\n",(0,l.jsxs)(e.ol,{children:["\n",(0,l.jsxs)(e.li,{children:["\n",(0,l.jsx)(e.p,{children:"\u672c\u5730\u5b89\u88c5 docker \u73af\u5883"}),"\n",(0,l.jsxs)(e.p,{children:["\u53c2\u8003 docker \u5b98\u65b9\u6587\u6863\uff1a",(0,l.jsx)(e.a,{href:"https://docs.docker.com/get-started/",children:"https://docs.docker.com/get-started/"})]}),"\n"]}),"\n",(0,l.jsxs)(e.li,{children:["\n",(0,l.jsx)(e.p,{children:"\u62c9\u53d6\u955c\u50cf"}),"\n",(0,l.jsx)(e.pre,{children:(0,l.jsx)(e.code,{className:"language-shell",children:"docker pull tugraph/tugraph-runtime-centos7\n"})}),"\n"]}),"\n",(0,l.jsxs)(e.li,{children:["\n",(0,l.jsx)(e.p,{children:"\u542f\u52a8docker"}),"\n",(0,l.jsx)(e.pre,{children:(0,l.jsx)(e.code,{className:"language-shell",children:" docker run -it -d -p 7070:7070 -p 7687:7687 -p 8000:8000 -p 9090:9090 \\\n -v /root/tugraph/data:/var/lib/lgraph/data  -v /root/tugraph/log:/var/log/lgraph_log \\\n --name tugraph_demo tugraph/tugraph-runtime-centos7:${VERSION} /bin/bash\n\ndocker exec -d tugraph_demo bash /setup.sh\n# 8000\u662f\u9ed8\u8ba4\u7684http\u7aef\u53e3\uff0c\u8bbf\u95eetugraph-db-browser\u4f7f\u7528\u3002\n# 7070\u662f\u9ed8\u8ba4\u7684http\u7aef\u53e3\uff0c\u8bbf\u95eelegacy tugraph-web\u8bbf\u95ee\u4f7f\u7528\u3002\n# 7687\u662fbolt\u7aef\u53e3\uff0cbolt client\u8bbf\u95ee\u4f7f\u7528\u3002\n# 9090\u662f\u9ed8\u8ba4\u7684rpc\u7aef\u53e3\uff0crpc client\u8bbf\u95ee\u4f7f\u7528\u3002\n\n# \u6839\u636e/usr/local/etc/lgraph.json\u7684tugraph\u542f\u52a8\u7684\u9ed8\u8ba4\u914d\u7f6e\uff0c\n# /var/lib/lgraph/data\u662f\u5bb9\u5668\u5185\u7684\u9ed8\u8ba4\u6570\u636e\u76ee\u5f55\uff0c/var/log/lgraph_log\u662f\u5bb9\u5668\u5185\u7684\u9ed8\u8ba4\u65e5\u5fd7\u76ee\u5f55\n# \u547d\u4ee4\u5c06\u6570\u636e\u76ee\u5f55\u548c\u65e5\u5fd7\u76ee\u5f55\u6302\u8f7d\u5230\u4e86\u5bbf\u4e3b\u673a\u7684/root/tugraph/\u4e0a\u8fdb\u884c\u6301\u4e45\u5316\uff0c\u60a8\u53ef\u4ee5\u6839\u636e\u5b9e\u9645\u60c5\u51b5\u4fee\u6539\u3002\n"})}),"\n"]}),"\n",(0,l.jsxs)(e.li,{children:["\n",(0,l.jsx)(e.p,{children:"\u524d\u7aef\u8bbf\u95ee"}),"\n"]}),"\n"]}),"\n",(0,l.jsxs)(e.p,{children:["\u8bbf\u95eetugraph-db-browser: ",(0,l.jsx)(e.code,{children:"http://x.x.x.x:8000"}),"\uff0c\u9ed8\u8ba4\u7528\u6237\u540d\u4e3a ",(0,l.jsx)(e.code,{children:"admin"}),"\uff0c\u5bc6\u7801\u4e3a ",(0,l.jsx)(e.code,{children:"73@TuGraph"}),"\u3002\n\u9996\u6b21\u767b\u5f55\u4f1a\u9ed8\u8ba4\u8df3\u8f6c\u4fee\u6539\u5bc6\u7801\u9875\u9762\uff0c\u8bf7\u5c3d\u5feb\u4fee\u6539\u9ed8\u8ba4\u5bc6\u7801\u907f\u514d\u5b89\u5168\u98ce\u9669\u3002"]}),"\n",(0,l.jsxs)(e.p,{children:[(0,l.jsx)(e.strong,{children:"\u6ce8\u610f"}),"\uff1a\n\u5982\u679c\u9700\u8981\u4f7f\u7528legacy tugraph-web\uff0c\u8bbf\u95ee",(0,l.jsx)(e.code,{children:"http://x.x.x.x:7070"}),"\uff0c\u9ed8\u8ba4\u7528\u6237\u540d\u4e3a ",(0,l.jsx)(e.code,{children:"admin"}),"\uff0c\u5bc6\u7801\u4e3a ",(0,l.jsx)(e.code,{children:"73@TuGraph"}),"\u3002\n\u4f46\u8bf7\u6ce8\u610f\uff0clegacy web\u662f\u4e0d\u518d\u7ef4\u62a4\u7684\u7248\u672c\uff0c\u5efa\u8bae\u4f7f\u7528tugraph-db-browser\u3002"]})]})}function o(n={}){const{wrapper:e}={...(0,t.R)(),...n.components};return e?(0,l.jsx)(e,{...n,children:(0,l.jsx)(a,{...n})}):a(n)}}}]);