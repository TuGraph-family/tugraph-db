"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[1953],{12538:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>d,contentTitle:()=>c,default:()=>u,frontMatter:()=>i,metadata:()=>o,toc:()=>l});var t=r(74848),s=r(28453);const i={},c="\u5907\u4efd\u6062\u590d",o={id:"utility-tools/backup-and-restore",title:"\u5907\u4efd\u6062\u590d",description:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph \u7684\u6570\u636e\u5907\u4efd\u548c\u6062\u590d\u529f\u80fd\u3002",source:"@site/versions/version-4.2.0/zh-CN/source/6.utility-tools/3.backup-and-restore.md",sourceDirName:"6.utility-tools",slug:"/utility-tools/backup-and-restore",permalink:"/tugraph-db/zh/4.2.0/utility-tools/backup-and-restore",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:3,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"\u6570\u636e\u5bfc\u51fa",permalink:"/tugraph-db/zh/4.2.0/utility-tools/data-export"},next:{title:"\u6570\u636e\u9884\u70ed",permalink:"/tugraph-db/zh/4.2.0/utility-tools/data-warmup"}},d={},l=[{value:"1.\u6570\u636e\u5907\u4efd",id:"1\u6570\u636e\u5907\u4efd",level:2},{value:"2.\u6570\u636e\u6062\u590d",id:"2\u6570\u636e\u6062\u590d",level:2}];function a(e){const n={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",header:"header",li:"li",p:"p",pre:"pre",ul:"ul",...(0,s.R)(),...e.components};return(0,t.jsxs)(t.Fragment,{children:[(0,t.jsx)(n.header,{children:(0,t.jsx)(n.h1,{id:"\u5907\u4efd\u6062\u590d",children:"\u5907\u4efd\u6062\u590d"})}),"\n",(0,t.jsxs)(n.blockquote,{children:["\n",(0,t.jsx)(n.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph \u7684\u6570\u636e\u5907\u4efd\u548c\u6062\u590d\u529f\u80fd\u3002"}),"\n"]}),"\n",(0,t.jsx)(n.h2,{id:"1\u6570\u636e\u5907\u4efd",children:"1.\u6570\u636e\u5907\u4efd"}),"\n",(0,t.jsxs)(n.p,{children:["TuGraph \u53ef\u4ee5\u901a\u8fc7 ",(0,t.jsx)(n.code,{children:"lgraph_backup"})," \u5de5\u5177\u6765\u8fdb\u884c\u6570\u636e\u5907\u4efd\u3002\n",(0,t.jsx)(n.code,{children:"lgraph_backup"})," \u5de5\u5177\u53ef\u4ee5\u5c06\u4e00\u4e2a TuGraph \u6570\u636e\u5e93\u4e2d\u7684\u6570\u636e\u5907\u4efd\u5230\u53e6\u4e00\u4e2a\u76ee\u5f55\u4e0b\uff0c\u5b83\u7684\u7528\u6cd5\u5982\u4e0b\uff1a"]}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{className:"language-bash",children:"$ lgraph_backup -s {source_dir} -d {destination_dir} -c {true/false}\n"})}),"\n",(0,t.jsx)(n.p,{children:"\u5176\u4e2d\uff1a"}),"\n",(0,t.jsxs)(n.ul,{children:["\n",(0,t.jsxs)(n.li,{children:[(0,t.jsx)(n.code,{children:"-s {source_dir}"})," \u6307\u5b9a\u9700\u8981\u5907\u4efd\u7684\u6570\u636e\u5e93\uff08\u6e90\u6570\u636e\u5e93\uff09\u6240\u5728\u76ee\u5f55\u3002"]}),"\n",(0,t.jsxs)(n.li,{children:[(0,t.jsx)(n.code,{children:"-d {destination_dir}"})," \u6307\u5b9a\u5907\u4efd\u6587\u4ef6\uff08\u76ee\u6807\u6570\u636e\u5e93\uff09\u6240\u5728\u76ee\u5f55\u3002\n\u5982\u679c\u76ee\u6807\u6570\u636e\u5e93\u4e0d\u4e3a\u7a7a\uff0c",(0,t.jsx)(n.code,{children:"lgraph_backup"})," \u4f1a\u63d0\u793a\u662f\u5426\u8986\u76d6\u8be5\u6570\u636e\u5e93\u3002"]}),"\n",(0,t.jsxs)(n.li,{children:[(0,t.jsx)(n.code,{children:"-c {true/false}"})," \u6307\u660e\u662f\u5426\u5728\u5907\u4efd\u8fc7\u7a0b\u4e2d\u8fdb\u884c compaction\u3002\ncompaction \u80fd\u4f7f\u4ea7\u751f\u7684\u5907\u4efd\u6587\u4ef6\u66f4\u7d27\u51d1\uff0c\u4f46\u5907\u4efd\u65f6\u95f4\u4e5f\u4f1a\u53d8\u957f\u3002\u8be5\u9009\u9879\u9ed8\u8ba4\u4e3a ",(0,t.jsx)(n.code,{children:"true"}),"\u3002"]}),"\n"]}),"\n",(0,t.jsx)(n.h2,{id:"2\u6570\u636e\u6062\u590d",children:"2.\u6570\u636e\u6062\u590d"}),"\n",(0,t.jsxs)(n.p,{children:["\u4f7f\u7528",(0,t.jsx)(n.code,{children:"lgraph_backup"})," \u5de5\u5177\u5f97\u5230\u7684\u76ee\u6807\u6570\u636e\u5e93",(0,t.jsx)(n.code,{children:"{destination_dir}"}),"\u5907\u4efd\u4e86\u6e90\u6570\u636e\u5e93\n",(0,t.jsx)(n.code,{children:"{source_dir}"}),"\u7684\u6240\u6709\u5b50\u56fe\uff0c\u4f46\u4e0d\u5305\u542bHA\u96c6\u7fa4\u7684raft\u4fe1\u606f\uff0c\u4ece\u800c\u4fdd\u8bc1\u670d\u52a1\u548c\u96c6\u7fa4\u80fd\n\u4ee5\u5907\u4efd\u6570\u636e\u5e93\u6210\u529f\u91cd\u542f\u5e76\u4e0e\u6e90\u6570\u636e\u5e93\u7684\u6570\u636e\u4e00\u81f4\u3002\u4f7f\u7528\u5982\u4e0b\u547d\u4ee4\u53ef\u4ee5\u7528\u5907\u4efd\u6570\u636e\u5e93\u91cd\u542f\u670d\u52a1\uff0c\n\u5728\u670d\u52a1\u542f\u52a8\u65f6\u4f1a\u6062\u590d\u6240\u6709\u5b50\u56fe\u7684\u5b58\u50a8\u8fc7\u7a0b\uff0c\u4fdd\u8bc1\u5907\u4efd\u670d\u52a1\u548c\u539f\u670d\u52a1\u5b8c\u5168\u4e00\u81f4\u3002"]}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{className:"language-bash",children:"$ lgraph_server -c lgraph.json --directory {destination_dir} -d start\n"})}),"\n",(0,t.jsx)(n.p,{children:"\u5176\u4e2d\uff1a"}),"\n",(0,t.jsxs)(n.ul,{children:["\n",(0,t.jsxs)(n.li,{children:[(0,t.jsx)(n.code,{children:"-d {destination_dir}"})," \u6307\u5b9a\u5907\u4efd\u6587\u4ef6\uff08\u76ee\u6807\u6570\u636e\u5e93\uff09\u6240\u5728\u76ee\u5f55\u3002"]}),"\n"]})]})}function u(e={}){const{wrapper:n}={...(0,s.R)(),...e.components};return n?(0,t.jsx)(n,{...e,children:(0,t.jsx)(a,{...e})}):a(e)}},28453:(e,n,r)=>{r.d(n,{R:()=>c,x:()=>o});var t=r(96540);const s={},i=t.createContext(s);function c(e){const n=t.useContext(i);return t.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function o(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(s):e.components||s:c(e.components),t.createElement(i.Provider,{value:n},e.children)}}}]);