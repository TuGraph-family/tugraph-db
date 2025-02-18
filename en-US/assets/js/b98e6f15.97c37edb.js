"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[11507],{28453:(e,r,n)=>{n.d(r,{R:()=>d,x:()=>c});var s=n(96540);const o={},t=s.createContext(o);function d(e){const r=s.useContext(t);return s.useMemo((function(){return"function"==typeof e?e(r):{...r,...e}}),[r,e])}function c(e){let r;return r=e.disableParentContext?"function"==typeof e.components?e.components(o):e.components||o:d(e.components),s.createElement(t.Provider,{value:r},e.children)}},44338:(e,r,n)=>{n.r(r),n.d(r,{assets:()=>l,contentTitle:()=>d,default:()=>u,frontMatter:()=>t,metadata:()=>c,toc:()=>a});var s=n(74848),o=n(28453);const t={},d="\u5907\u4efd\u6062\u590d",c={id:"developer-manual/server-tools/backup-and-restore",title:"\u5907\u4efd\u6062\u590d",description:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph \u7684\u6570\u636e\u5907\u4efd\u548c\u6062\u590d\u529f\u80fd\u3002",source:"@site/versions/version-3.5.1/zh-CN/source/5.developer-manual/3.server-tools/3.backup-and-restore.md",sourceDirName:"5.developer-manual/3.server-tools",slug:"/developer-manual/server-tools/backup-and-restore",permalink:"/tugraph-db/en-US/zh/3.5.1/developer-manual/server-tools/backup-and-restore",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:3,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"\u6570\u636e\u5bfc\u51fa",permalink:"/tugraph-db/en-US/zh/3.5.1/developer-manual/server-tools/data-export"},next:{title:"\u6570\u636e\u9884\u70ed",permalink:"/tugraph-db/en-US/zh/3.5.1/developer-manual/server-tools/data-warmup"}},l={},a=[{value:"1.\u6570\u636e\u5907\u4efd",id:"1\u6570\u636e\u5907\u4efd",level:2},{value:"2.\u6570\u636e\u6062\u590d",id:"2\u6570\u636e\u6062\u590d",level:2}];function i(e){const r={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",header:"header",li:"li",p:"p",pre:"pre",ul:"ul",...(0,o.R)(),...e.components};return(0,s.jsxs)(s.Fragment,{children:[(0,s.jsx)(r.header,{children:(0,s.jsx)(r.h1,{id:"\u5907\u4efd\u6062\u590d",children:"\u5907\u4efd\u6062\u590d"})}),"\n",(0,s.jsxs)(r.blockquote,{children:["\n",(0,s.jsx)(r.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph \u7684\u6570\u636e\u5907\u4efd\u548c\u6062\u590d\u529f\u80fd\u3002"}),"\n"]}),"\n",(0,s.jsx)(r.h2,{id:"1\u6570\u636e\u5907\u4efd",children:"1.\u6570\u636e\u5907\u4efd"}),"\n",(0,s.jsxs)(r.p,{children:["TuGraph \u53ef\u4ee5\u901a\u8fc7 ",(0,s.jsx)(r.code,{children:"lgraph_backup"})," \u5de5\u5177\u6765\u8fdb\u884c\u6570\u636e\u5907\u4efd\u3002\n",(0,s.jsx)(r.code,{children:"lgraph_backup"})," \u5de5\u5177\u53ef\u4ee5\u5c06\u4e00\u4e2a TuGraph \u6570\u636e\u5e93\u4e2d\u7684\u6570\u636e\u5907\u4efd\u5230\u53e6\u4e00\u4e2a\u76ee\u5f55\u4e0b\uff0c\u5b83\u7684\u7528\u6cd5\u5982\u4e0b\uff1a"]}),"\n",(0,s.jsx)(r.pre,{children:(0,s.jsx)(r.code,{className:"language-bash",children:"$ lgraph_backup -s {source_dir} -d {destination_dir} -c {true/false}\n"})}),"\n",(0,s.jsx)(r.p,{children:"\u5176\u4e2d\uff1a"}),"\n",(0,s.jsxs)(r.ul,{children:["\n",(0,s.jsxs)(r.li,{children:[(0,s.jsx)(r.code,{children:"-s {source_dir}"})," \u6307\u5b9a\u9700\u8981\u5907\u4efd\u7684\u6570\u636e\u5e93\uff08\u6e90\u6570\u636e\u5e93\uff09\u6240\u5728\u76ee\u5f55\u3002"]}),"\n",(0,s.jsxs)(r.li,{children:[(0,s.jsx)(r.code,{children:"-d {destination_dir}"})," \u6307\u5b9a\u5907\u4efd\u6587\u4ef6\uff08\u76ee\u6807\u6570\u636e\u5e93\uff09\u6240\u5728\u76ee\u5f55\u3002\n\u5982\u679c\u76ee\u6807\u6570\u636e\u5e93\u4e0d\u4e3a\u7a7a\uff0c",(0,s.jsx)(r.code,{children:"lgraph_backup"})," \u4f1a\u63d0\u793a\u662f\u5426\u8986\u76d6\u8be5\u6570\u636e\u5e93\u3002"]}),"\n",(0,s.jsxs)(r.li,{children:[(0,s.jsx)(r.code,{children:"-c {true/false}"})," \u6307\u660e\u662f\u5426\u5728\u5907\u4efd\u8fc7\u7a0b\u4e2d\u8fdb\u884c compaction\u3002\ncompaction \u80fd\u4f7f\u4ea7\u751f\u7684\u5907\u4efd\u6587\u4ef6\u66f4\u7d27\u51d1\uff0c\u4f46\u5907\u4efd\u65f6\u95f4\u4e5f\u4f1a\u53d8\u957f\u3002\u8be5\u9009\u9879\u9ed8\u8ba4\u4e3a ",(0,s.jsx)(r.code,{children:"true"}),"\u3002"]}),"\n"]}),"\n",(0,s.jsx)(r.h2,{id:"2\u6570\u636e\u6062\u590d",children:"2.\u6570\u636e\u6062\u590d"}),"\n",(0,s.jsxs)(r.p,{children:["\u4f7f\u7528",(0,s.jsx)(r.code,{children:"lgraph_backup"})," \u5de5\u5177\u5f97\u5230\u7684\u76ee\u6807\u6570\u636e\u5e93",(0,s.jsx)(r.code,{children:"{destination_dir}"}),"\u5907\u4efd\u4e86\u6e90\u6570\u636e\u5e93\n",(0,s.jsx)(r.code,{children:"{source_dir}"}),"\u7684\u6240\u6709\u5b50\u56fe\uff0c\u4f46\u4e0d\u5305\u542bHA\u96c6\u7fa4\u7684raft\u4fe1\u606f\uff0c\u4ece\u800c\u4fdd\u8bc1\u670d\u52a1\u548c\u96c6\u7fa4\u80fd\n\u4ee5\u5907\u4efd\u6570\u636e\u5e93\u6210\u529f\u91cd\u542f\u5e76\u4e0e\u6e90\u6570\u636e\u5e93\u7684\u6570\u636e\u4e00\u81f4\u3002\u4f7f\u7528\u5982\u4e0b\u547d\u4ee4\u53ef\u4ee5\u7528\u5907\u4efd\u6570\u636e\u5e93\u91cd\u542f\u670d\u52a1\uff0c\n\u5728\u670d\u52a1\u542f\u52a8\u65f6\u4f1a\u6062\u590d\u6240\u6709\u5b50\u56fe\u7684\u5b58\u50a8\u8fc7\u7a0b\uff0c\u4fdd\u8bc1\u5907\u4efd\u670d\u52a1\u548c\u539f\u670d\u52a1\u5b8c\u5168\u4e00\u81f4\u3002"]}),"\n",(0,s.jsx)(r.pre,{children:(0,s.jsx)(r.code,{className:"language-bash",children:"$ lgraph_server -c lgraph.json --directory {destination_dir} -d start\n"})}),"\n",(0,s.jsx)(r.p,{children:"\u5176\u4e2d\uff1a"}),"\n",(0,s.jsxs)(r.ul,{children:["\n",(0,s.jsxs)(r.li,{children:[(0,s.jsx)(r.code,{children:"-d {destination_dir}"})," \u6307\u5b9a\u5907\u4efd\u6587\u4ef6\uff08\u76ee\u6807\u6570\u636e\u5e93\uff09\u6240\u5728\u76ee\u5f55\u3002"]}),"\n"]})]})}function u(e={}){const{wrapper:r}={...(0,o.R)(),...e.components};return r?(0,s.jsx)(r,{...e,children:(0,s.jsx)(i,{...e})}):i(e)}}}]);