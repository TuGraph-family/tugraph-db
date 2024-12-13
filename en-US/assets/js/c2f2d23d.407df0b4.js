"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[63095],{55843:(e,n,t)=>{t.r(n),t.d(n,{assets:()=>d,contentTitle:()=>s,default:()=>p,frontMatter:()=>o,metadata:()=>a,toc:()=>l});var r=t(74848),i=t(28453);const o={},s="Environment and version selection",a={id:"best-practices/selection",title:"Environment and version selection",description:"This document introduces how to select the system environment and deployment method",source:"@site/versions/version-4.3.1/en-US/source/13.best-practices/4.selection.md",sourceDirName:"13.best-practices",slug:"/best-practices/selection",permalink:"/tugraph-db/en-US/en/4.3.1/best-practices/selection",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:4,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Data Migration",permalink:"/tugraph-db/en-US/en/4.3.1/best-practices/data_migration"},next:{title:"FAQ",permalink:"/tugraph-db/en-US/en/4.3.1/faq"}},d={},l=[{value:"1 Introduction",id:"1-introduction",level:2},{value:"2. Environmental capability selection",id:"2-environmental-capability-selection",level:2},{value:"3. Deployment method selection",id:"3-deployment-method-selection",level:2},{value:"4. Next steps",id:"4-next-steps",level:2}];function c(e){const n={a:"a",blockquote:"blockquote",h1:"h1",h2:"h2",header:"header",p:"p",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",...(0,i.R)(),...e.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(n.header,{children:(0,r.jsx)(n.h1,{id:"environment-and-version-selection",children:"Environment and version selection"})}),"\n",(0,r.jsxs)(n.blockquote,{children:["\n",(0,r.jsx)(n.p,{children:"This document introduces how to select the system environment and deployment method"}),"\n"]}),"\n",(0,r.jsx)(n.h2,{id:"1-introduction",children:"1 Introduction"}),"\n",(0,r.jsx)(n.p,{children:"TuGraph provides differentiated system environments and deployment methods for users with different needs to meet the needs of different users such as novices, system developers, production operation and maintenance personnel, and researchers."}),"\n",(0,r.jsx)(n.h2,{id:"2-environmental-capability-selection",children:"2. Environmental capability selection"}),"\n",(0,r.jsx)(n.p,{children:"Users can choose different environments based on actual usage scenarios. The compilation environment has the most complete capabilities and requires more third-party software. Correspondingly, the streamlined operating environment requires almost no installation of any dependent libraries and can run TuGraph's basic functions except stored procedures."}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(n.table,{children:[(0,r.jsx)(n.thead,{children:(0,r.jsxs)(n.tr,{children:[(0,r.jsx)(n.th,{children:"Environment"}),(0,r.jsx)(n.th,{children:"Purpose"}),(0,r.jsx)(n.th,{children:"Remarks"})]})}),(0,r.jsxs)(n.tbody,{children:[(0,r.jsxs)(n.tr,{children:[(0,r.jsx)(n.td,{children:"Compilation environment"}),(0,r.jsx)(n.td,{children:"Compile TuGraph from source"}),(0,r.jsx)(n.td,{children:"For developers"})]}),(0,r.jsxs)(n.tr,{children:[(0,r.jsx)(n.td,{children:"Running environment"}),(0,r.jsx)(n.td,{children:"Run TuGraph installation package"}),(0,r.jsx)(n.td,{children:"Applicable to most users"})]}),(0,r.jsxs)(n.tr,{children:[(0,r.jsx)(n.td,{children:"Simplified operating environment"}),(0,r.jsx)(n.td,{children:"Run simplified TuGraph installation package"}),(0,r.jsx)(n.td,{children:"Less dependence on the operating system"})]})]})]}),"\n",(0,r.jsxs)(n.p,{children:["For a detailed introduction to different environments, see ",(0,r.jsx)(n.a,{href:"/tugraph-db/en-US/en/4.3.1/installation&running/environment-mode",children:"link"}),"."]}),"\n",(0,r.jsx)(n.h2,{id:"3-deployment-method-selection",children:"3. Deployment method selection"}),"\n",(0,r.jsx)(n.p,{children:"TuGraph deployment only requires one server (high availability mode requires multiple servers), and you can choose a suitable deployment method based on actual resource conditions and usage scenarios."}),"\n",(0,r.jsxs)(n.p,{children:["| Deployment method | Description | Remarks |\n|----------|-----------------------|--------------- -------------------------------------------------- --------------------------|\n| Cloud deployment | Alibaba Cloud Computing Nest one-click deployment, free trial | Suitable for novices, process reference ",(0,r.jsx)(n.a,{href:"/tugraph-db/en-US/en/4.3.1/installation&running/cloud-deployment",children:"Link"})," |\n| Docker deployment | Cross-platform deployment through pre-prepared Docker images | Users with hardware requirements, such as performance testing, please refer to [link](../5.developer-manual/1.installation/3.docker-deployment. md) |\n| Local deployment | Tightly coupled deployment in existing systems | Applicable to specified production environment, refer to the process ",(0,r.jsx)(n.a,{href:"/tugraph-db/en-US/en/4.3.1/installation&running/local-package-deployment",children:"Link"})," |"]}),"\n",(0,r.jsx)(n.h2,{id:"4-next-steps",children:"4. Next steps"}),"\n",(0,r.jsxs)(n.p,{children:["After the deployment is completed, you can proceed to ",(0,r.jsx)(n.a,{href:"/tugraph-db/en-US/en/4.3.1/installation&running/tugraph-running",children:"Start Service"})," and [Data Import](../5.developer-manual/3. server-tools/1.data-import.md) and other operations, you can also experience the entire process through ",(0,r.jsx)(n.a,{href:"/tugraph-db/en-US/en/4.3.1/quick-start/demo/movie",children:"Sample Data"}),"."]})]})}function p(e={}){const{wrapper:n}={...(0,i.R)(),...e.components};return n?(0,r.jsx)(n,{...e,children:(0,r.jsx)(c,{...e})}):c(e)}},28453:(e,n,t)=>{t.d(n,{R:()=>s,x:()=>a});var r=t(96540);const i={},o=r.createContext(i);function s(e){const n=r.useContext(o);return r.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function a(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(i):e.components||i:s(e.components),r.createElement(o.Provider,{value:n},e.children)}}}]);