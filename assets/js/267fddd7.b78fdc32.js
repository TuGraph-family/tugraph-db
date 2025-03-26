"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[67447],{28453:(e,r,n)=>{n.d(r,{R:()=>t,x:()=>l});var s=n(96540);const o={},d=s.createContext(o);function t(e){const r=s.useContext(d);return s.useMemo((function(){return"function"==typeof e?e(r):{...r,...e}}),[r,e])}function l(e){let r;return r=e.disableParentContext?"function"==typeof e.components?e.components(o):e.components||o:t(e.components),s.createElement(d.Provider,{value:r},e.children)}},68949:(e,r,n)=>{n.r(r),n.d(r,{assets:()=>c,contentTitle:()=>t,default:()=>h,frontMatter:()=>d,metadata:()=>l,toc:()=>i});var s=n(74848),o=n(28453);const d={},t="\u6570\u636e\u5bfc\u51fa",l={id:"developer-manual/server-tools/data-export",title:"\u6570\u636e\u5bfc\u51fa",description:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph \u7684\u6570\u636e\u5bfc\u51fa\u529f\u80fd\u3002",source:"@site/versions/version-4.0.1/zh-CN/source/5.developer-manual/3.server-tools/2.data-export.md",sourceDirName:"5.developer-manual/3.server-tools",slug:"/developer-manual/server-tools/data-export",permalink:"/tugraph-db/zh/4.0.1/developer-manual/server-tools/data-export",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:2,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"\u6570\u636e\u5bfc\u5165",permalink:"/tugraph-db/zh/4.0.1/developer-manual/server-tools/data-import"},next:{title:"\u5907\u4efd\u6062\u590d",permalink:"/tugraph-db/zh/4.0.1/developer-manual/server-tools/backup-and-restore"}},c={},i=[{value:"1.\u7b80\u4ecb",id:"1\u7b80\u4ecb",level:2},{value:"2.\u5bfc\u51fa\u547d\u4ee4",id:"2\u5bfc\u51fa\u547d\u4ee4",level:2}];function a(e){const r={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",header:"header",li:"li",p:"p",pre:"pre",ul:"ul",...(0,o.R)(),...e.components};return(0,s.jsxs)(s.Fragment,{children:[(0,s.jsx)(r.header,{children:(0,s.jsx)(r.h1,{id:"\u6570\u636e\u5bfc\u51fa",children:"\u6570\u636e\u5bfc\u51fa"})}),"\n",(0,s.jsxs)(r.blockquote,{children:["\n",(0,s.jsx)(r.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph \u7684\u6570\u636e\u5bfc\u51fa\u529f\u80fd\u3002"}),"\n"]}),"\n",(0,s.jsx)(r.h2,{id:"1\u7b80\u4ecb",children:"1.\u7b80\u4ecb"}),"\n",(0,s.jsxs)(r.p,{children:["TuGraph \u53ef\u4ee5\u901a\u8fc7 ",(0,s.jsx)(r.code,{children:"lgraph_export"})," \u5de5\u5177\u6765\u5bf9\u5df2\u7ecf\u5b58\u653e\u5728TuGraph\u7684\u56fe\u6570\u636e\u8fdb\u884c\u6570\u636e\u5bfc\u51fa\u3002 ",(0,s.jsx)(r.code,{children:"lgraph_export"})," \u5de5\u5177\u53ef\u4ee5\u5c06\u6307\u5b9a TuGraph \u6570\u636e\u5e93\u7684\u6570\u636e\u4ee5 ",(0,s.jsx)(r.code,{children:"csv"})," \u6216\u8005 ",(0,s.jsx)(r.code,{children:"json"})," \u6587\u4ef6\u5f62\u5f0f\u5bfc\u51fa\u5230\u6307\u5b9a\u76ee\u5f55\uff0c\u540c\u65f6\u5bfc\u51fa\u8fd9\u4e9b\u6570\u636e\u8fdb\u884c\u518d\u5bfc\u5165\u65f6\u9700\u8981\u7684\u914d\u7f6e\u6587\u4ef6 ",(0,s.jsx)(r.code,{children:"import.config"})," \uff0c\u8be6\u7ec6\u63cf\u8ff0\u53ef\u53c2\u89c1",(0,s.jsx)(r.a,{href:"/tugraph-db/zh/4.0.1/developer-manual/server-tools/data-import",children:"\u914d\u7f6e\u6587\u4ef6"}),"\u3002"]}),"\n",(0,s.jsx)(r.h2,{id:"2\u5bfc\u51fa\u547d\u4ee4",children:"2.\u5bfc\u51fa\u547d\u4ee4"}),"\n",(0,s.jsx)(r.p,{children:"\u8be5\u5de5\u5177\u7684\u547d\u4ee4\u793a\u4f8b\u5982\u4e0b\uff1a"}),"\n",(0,s.jsx)(r.pre,{children:(0,s.jsx)(r.code,{className:"language-bash",children:"$ lgraph_export -d {database_dir} -e {export_destination_dir} -g {graph_to_use} -u {username} -p {password} -f {output_format}\n"})}),"\n",(0,s.jsx)(r.p,{children:"\u5176\u4e2d\uff1a"}),"\n",(0,s.jsxs)(r.ul,{children:["\n",(0,s.jsxs)(r.li,{children:[(0,s.jsx)(r.code,{children:"-d {database_dir}"})," \u6307\u5b9a\u9700\u8981\u8fdb\u884c\u6570\u636e\u5bfc\u51fa\u7684\u6570\u636e\u5e93\u6240\u5728\u76ee\u5f55\uff0c\u9ed8\u8ba4\u503c\u4e3a ",(0,s.jsx)(r.code,{children:"./testdb"}),"\u3002"]}),"\n",(0,s.jsxs)(r.li,{children:[(0,s.jsx)(r.code,{children:"-e {export_destination_dir}"})," \u6307\u5b9a\u5bfc\u51fa\u6587\u4ef6\u5b58\u653e\u7684\u76ee\u5f55\uff0c\u9ed8\u8ba4\u503c\u4e3a ",(0,s.jsx)(r.code,{children:"./exportdir"}),"\u3002"]}),"\n",(0,s.jsxs)(r.li,{children:[(0,s.jsx)(r.code,{children:"-g {graph_to_use}"})," \u6307\u5b9a\u56fe\u6570\u636e\u5e93\u7684\u79cd\u7c7b\uff0c\u9ed8\u8ba4\u4e3a ",(0,s.jsx)(r.code,{children:"default"})," \u3002"]}),"\n",(0,s.jsxs)(r.li,{children:[(0,s.jsx)(r.code,{children:"-u {username}"})," \u6307\u5b9a\u8fdb\u884c\u8be5\u5bfc\u51fa\u64cd\u4f5c\u7684\u7528\u6237\u7684\u7528\u6237\u540d\u3002"]}),"\n",(0,s.jsxs)(r.li,{children:[(0,s.jsx)(r.code,{children:"-p {password}"})," \u6307\u5b9a\u8fdb\u884c\u8be5\u5bfc\u51fa\u64cd\u4f5c\u7684\u7528\u6237\u7684\u7528\u6237\u5bc6\u7801\u3002"]}),"\n",(0,s.jsxs)(r.li,{children:[(0,s.jsx)(r.code,{children:"-s {field_separator}"})," \u6307\u5b9a\u5bfc\u51fa\u6587\u4ef6\u7684\u5206\u9694\u7b26\uff0c\u9ed8\u8ba4\u4e3a\u9017\u53f7\u3002"]}),"\n",(0,s.jsxs)(r.li,{children:[(0,s.jsx)(r.code,{children:"-f {output_format}"})," \u6307\u5b9a\u5bfc\u51fa\u6570\u636e\u7684\u683c\u5f0f\uff0c",(0,s.jsx)(r.code,{children:"json"}),"\u6216\u8005",(0,s.jsx)(r.code,{children:"csv"}),"\uff0c\u9ed8\u8ba4\u4e3a",(0,s.jsx)(r.code,{children:"csv"}),"\u3002"]}),"\n",(0,s.jsxs)(r.li,{children:[(0,s.jsx)(r.code,{children:"-h"})," \u9664\u4e0a\u8ff0\u6307\u5b9a\u53c2\u6570\u5916\uff0c\u4e5f\u53ef\u4ee5\u4f7f\u7528\u8be5\u53c2\u6570\u67e5\u770b\u8be5\u5de5\u5177\u7684\u4f7f\u7528\u5e2e\u52a9\u3002"]}),"\n"]})]})}function h(e={}){const{wrapper:r}={...(0,o.R)(),...e.components};return r?(0,s.jsx)(r,{...e,children:(0,s.jsx)(a,{...e})}):a(e)}}}]);