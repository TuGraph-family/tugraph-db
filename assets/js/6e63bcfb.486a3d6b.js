"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[93580],{1970:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>d,contentTitle:()=>o,default:()=>h,frontMatter:()=>i,metadata:()=>s,toc:()=>l});var t=r(74848),a=r(28453);const i={},o="Environment",s={id:"developer-manual/installation/environment",title:"Environment",description:"This document provides an overview of the required hardware and software environment for deploying TuGraph.",source:"@site/versions/version-4.1.0/en-US/source/5.developer-manual/1.installation/1.environment.md",sourceDirName:"5.developer-manual/1.installation",slug:"/developer-manual/installation/environment",permalink:"/tugraph-db/en/4.1.0/developer-manual/installation/environment",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:1,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"TuGraph Browser(old version)",permalink:"/tugraph-db/en/4.1.0/user-guide/tugraph-browser-legacy"},next:{title:"Environment Mode",permalink:"/tugraph-db/en/4.1.0/developer-manual/installation/environment-mode"}},d={},l=[{value:"1.Hardware Environment",id:"1hardware-environment",level:2},{value:"1.1. CPU",id:"11-cpu",level:3},{value:"1.2. Memory",id:"12-memory",level:3},{value:"1.3. Storage",id:"13-storage",level:3},{value:"2. Software environment",id:"2-software-environment",level:2},{value:"2.1. Operating System",id:"21-operating-system",level:3},{value:"2.2. System Libraries",id:"22-system-libraries",level:3},{value:"3.Recommended Configuration",id:"3recommended-configuration",level:2}];function c(e){const n={a:"a",blockquote:"blockquote",h1:"h1",h2:"h2",h3:"h3",header:"header",p:"p",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",...(0,a.R)(),...e.components};return(0,t.jsxs)(t.Fragment,{children:[(0,t.jsx)(n.header,{children:(0,t.jsx)(n.h1,{id:"environment",children:"Environment"})}),"\n",(0,t.jsxs)(n.blockquote,{children:["\n",(0,t.jsx)(n.p,{children:"This document provides an overview of the required hardware and software environment for deploying TuGraph."}),"\n"]}),"\n",(0,t.jsx)(n.h2,{id:"1hardware-environment",children:"1.Hardware Environment"}),"\n",(0,t.jsx)(n.h3,{id:"11-cpu",children:"1.1. CPU"}),"\n",(0,t.jsx)(n.p,{children:"TuGraph supports X86_64 and ARM64 architectures for both physical, virtual, and containerized environments. Tested and certified hardware platforms include Intel, AMD, Kunpeng, Hygon, Feiteng, and others."}),"\n",(0,t.jsx)(n.h3,{id:"12-memory",children:"1.2. Memory"}),"\n",(0,t.jsx)(n.p,{children:"We recommend having a memory capacity that is equal to or greater than the actual data size. For optimal performance, caching all data in memory is ideal. In terms of data locality, graph databases have poorer locality compared to relational databases. Therefore, if the data cannot fit in memory, frequent swapping may occur."}),"\n",(0,t.jsx)(n.h3,{id:"13-storage",children:"1.3. Storage"}),"\n",(0,t.jsx)(n.p,{children:"We strongly recommend using NVMe SSD as external storage. The database performs numerous synchronous write operations, often in a random manner, making the read/write performance of the external storage critical to the overall database performance. Hence, high IOPS and low-latency NVMe SSDs are the optimal choice."}),"\n",(0,t.jsx)(n.p,{children:"If circumstances permit only the use of SATA interface SSDs or cloud-based network disks, although performance may be affected, TuGraph can still run correctly."}),"\n",(0,t.jsx)(n.p,{children:"It is recommended to have external storage size at least four times the actual data size. For example, if the data size is 1TB, preparing a 4TB hard drive would be more reliable."}),"\n",(0,t.jsx)(n.h2,{id:"2-software-environment",children:"2. Software environment"}),"\n",(0,t.jsx)(n.h3,{id:"21-operating-system",children:"2.1. Operating System"}),"\n",(0,t.jsx)(n.p,{children:"TuGraph is compatible with popular operating systems, including Ubuntu, CentOS, SUSE, Galaxy Kylin, China Standard Software, UOS, and others, all of which have been tested and certified."}),"\n",(0,t.jsx)(n.p,{children:"Among them, the most stable system versions are Ubuntu 18.04, CentOS 7, and CentOS 8."}),"\n",(0,t.jsx)(n.h3,{id:"22-system-libraries",children:"2.2. System Libraries"}),"\n",(0,t.jsxs)(n.p,{children:["The requirements for system libraries differ between the compilation environment and the runtime environment. For specific details, please refer to the",(0,t.jsx)(n.a,{href:"/tugraph-db/en/4.1.0/developer-manual/installation/environment-mode",children:"environment mode"}),"\u3002"]}),"\n",(0,t.jsx)(n.h2,{id:"3recommended-configuration",children:"3.Recommended Configuration"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,t.jsxs)(n.table,{children:[(0,t.jsx)(n.thead,{children:(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.th,{children:"Hardware"}),(0,t.jsx)(n.th,{children:"Minimum Configuration"}),(0,t.jsx)(n.th,{children:"Recommended Configuration"})]})}),(0,t.jsxs)(n.tbody,{children:[(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"CPU"}),(0,t.jsx)(n.td,{children:"4 Cores"}),(0,t.jsx)(n.td,{children:"64 Cores"})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"Memory"}),(0,t.jsx)(n.td,{children:"4GB"}),(0,t.jsx)(n.td,{children:"512GB"})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"Storage"}),(0,t.jsx)(n.td,{children:"100GB"}),(0,t.jsx)(n.td,{children:"2TB NVMe SSD"})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"OS"}),(0,t.jsx)(n.td,{children:"Linux 4.9"}),(0,t.jsx)(n.td,{children:"CentOS 7.3"})]})]})]})]})}function h(e={}){const{wrapper:n}={...(0,a.R)(),...e.components};return n?(0,t.jsx)(n,{...e,children:(0,t.jsx)(c,{...e})}):c(e)}},28453:(e,n,r)=>{r.d(n,{R:()=>o,x:()=>s});var t=r(96540);const a={},i=t.createContext(a);function o(e){const n=t.useContext(i);return t.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function s(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(a):e.components||a:o(e.components),t.createElement(i.Provider,{value:n},e.children)}}}]);