"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[26623],{11207:(e,t,a)=>{a.r(t),a.d(t,{assets:()=>d,contentTitle:()=>i,default:()=>p,frontMatter:()=>o,metadata:()=>s,toc:()=>u});var r=a(74848),n=a(28453);const o={},i="Data Warmup",s={id:"utility-tools/data-warmup",title:"Data Warmup",description:"This document mainly introduces the data Warmup function of TuGraph.",source:"@site/versions/version-4.2.0/en-US/source/6.utility-tools/4.data-warmup.md",sourceDirName:"6.utility-tools",slug:"/utility-tools/data-warmup",permalink:"/tugraph-db/en/4.2.0/utility-tools/data-warmup",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:4,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Backup and Restore",permalink:"/tugraph-db/en/4.2.0/utility-tools/backup-and-restore"},next:{title:"Cluster management",permalink:"/tugraph-db/en/4.2.0/utility-tools/ha-cluster-management"}},d={},u=[{value:"1.Introduction",id:"1introduction",level:2},{value:"2.lgraph_warmup",id:"2lgraph_warmup",level:2}];function l(e){const t={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",header:"header",p:"p",pre:"pre",...(0,n.R)(),...e.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(t.header,{children:(0,r.jsx)(t.h1,{id:"data-warmup",children:"Data Warmup"})}),"\n",(0,r.jsxs)(t.blockquote,{children:["\n",(0,r.jsx)(t.p,{children:"This document mainly introduces the data Warmup function of TuGraph."}),"\n"]}),"\n",(0,r.jsx)(t.h2,{id:"1introduction",children:"1.Introduction"}),"\n",(0,r.jsx)(t.p,{children:"TuGraph is a disk-based database where data loaded into memory only when accessed. Therefore, for a period of time after the server just turned on, the system performance may be degraded due to frequent IO operations. At this point, we can improve this problem by data warm-up."}),"\n",(0,r.jsx)(t.h2,{id:"2lgraph_warmup",children:"2.lgraph_warmup"}),"\n",(0,r.jsx)(t.p,{children:"Data warmup can be done using the tool lgraph_warmup. An example of its use is as follows:"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-bash",children:"$ lgraph_warmup -d {directory} -g {graph_list}\n"})}),"\n",(0,r.jsx)(t.p,{children:"Details:"}),"\n",(0,r.jsx)(t.p,{children:"The - '-d {db_dir}' option specifies the data directory for the TuGraph server"}),"\n",(0,r.jsx)(t.p,{children:"The - '-g {graph_list}' option specifies the names of graphs to be warmed-up, separated by commas"}),"\n",(0,r.jsx)(t.p,{children:"The warm-up process takes different times depending on the data size and the type of disk being used. Preheating a large database on a mechanical disk may take a long time. Please wait patiently."})]})}function p(e={}){const{wrapper:t}={...(0,n.R)(),...e.components};return t?(0,r.jsx)(t,{...e,children:(0,r.jsx)(l,{...e})}):l(e)}},28453:(e,t,a)=>{a.d(t,{R:()=>i,x:()=>s});var r=a(96540);const n={},o=r.createContext(n);function i(e){const t=r.useContext(o);return r.useMemo((function(){return"function"==typeof e?e(t):{...t,...e}}),[t,e])}function s(e){let t;return t=e.disableParentContext?"function"==typeof e.components?e.components(n):e.components||n:i(e.components),r.createElement(o.Provider,{value:t},e.children)}}}]);