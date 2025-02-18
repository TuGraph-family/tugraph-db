"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[90733],{28453:(n,e,t)=>{t.d(e,{R:()=>s,x:()=>a});var r=t(96540);const d={},i=r.createContext(d);function s(n){const e=r.useContext(i);return r.useMemo((function(){return"function"==typeof n?n(e):{...e,...n}}),[e,n])}function a(n){let e;return e=n.disableParentContext?"function"==typeof n.components?n.components(d):n.components||d:s(n.components),r.createElement(i.Provider,{value:e},n.children)}},45199:(n,e,t)=>{t.r(e),t.d(e,{assets:()=>o,contentTitle:()=>s,default:()=>h,frontMatter:()=>i,metadata:()=>a,toc:()=>l});var r=t(74848),d=t(28453);const i={},s="TuGraph Roadmap",a={id:"contributor-manual/roadmap",title:"TuGraph Roadmap",description:"1. Introduction",source:"@site/versions/version-4.1.0/en-US/source/6.contributor-manual/5.roadmap.md",sourceDirName:"6.contributor-manual",slug:"/contributor-manual/roadmap",permalink:"/tugraph-db/en-US/en/4.1.0/contributor-manual/roadmap",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:5,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Corporate Contributor License Agreement",permalink:"/tugraph-db/en-US/en/4.1.0/contributor-manual/corporate-cla"},next:{title:"Importing Data from Relational Databases to TuGraph",permalink:"/tugraph-db/en-US/en/4.1.0/best-practices/rdbms-to-tugraph"}},o={},l=[{value:"1. Introduction",id:"1-introduction",level:2},{value:"2. Completed Functionalities",id:"2-completed-functionalities",level:2},{value:"3. Roadmap of 2023 S2",id:"3-roadmap-of-2023-s2",level:2},{value:"4. Functional Updates in 2023",id:"4-functional-updates-in-2023",level:2},{value:"5. Community Collaboration Features",id:"5-community-collaboration-features",level:2}];function c(n){const e={h1:"h1",h2:"h2",header:"header",p:"p",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",...(0,d.R)(),...n.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(e.header,{children:(0,r.jsx)(e.h1,{id:"tugraph-roadmap",children:"TuGraph Roadmap"})}),"\n",(0,r.jsx)(e.h2,{id:"1-introduction",children:"1. Introduction"}),"\n",(0,r.jsx)(e.p,{children:"This document outlines the future development plans for TuGraph, including features currently under development, those not in the development pipeline, and completed functionalities not included in the open-source version."}),"\n",(0,r.jsx)(e.p,{children:"TuGraph aims to be an open-source, high-performance graph database. It adopts a centralized storage approach for graph data and, in the short term, does not consider data sharding. Instead, it employs a master-slave replication mode to address high-concurrency read scenarios, while utilizing cloud-based storage solutions to address storage capacity challenges."}),"\n",(0,r.jsx)(e.h2,{id:"2-completed-functionalities",children:"2. Completed Functionalities"}),"\n",(0,r.jsx)(e.p,{children:"TuGraph was open-sourced on September 1, 2022, and has received regular bug fixes and enhancements based on community feedback."}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(e.table,{children:[(0,r.jsx)(e.thead,{children:(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.th,{children:"Version"}),(0,r.jsx)(e.th,{children:"Functionality"}),(0,r.jsx)(e.th,{children:"Date"})]})}),(0,r.jsxs)(e.tbody,{children:[(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"3.3.0"}),(0,r.jsx)(e.td,{children:"Initial open-source release"}),(0,r.jsx)(e.td,{children:"2022.9.1"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"3.3.1"}),(0,r.jsx)(e.td,{children:"Refactored graph analysis engine with multi-mode support"}),(0,r.jsx)(e.td,{children:"2022.10.14"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"3.3.2"}),(0,r.jsx)(e.td,{children:"Added OGM support and improved unit test coverage"}),(0,r.jsx)(e.td,{children:"2022.11.21"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"3.3.3"}),(0,r.jsx)(e.td,{children:"Iterative improvements to link authentication mechanism and addition of English documentation"}),(0,r.jsx)(e.td,{children:"2022.12.23"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"3.3.4"}),(0,r.jsx)(e.td,{children:"Cloud deployment support and streamlined LDBC SNB Audit process"}),(0,r.jsx)(e.td,{children:"2023.1.28"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"3.4.0"}),(0,r.jsx)(e.td,{children:"Added support for OLAP Python API and upgraded offline data import"}),(0,r.jsx)(e.td,{children:"2023.3.11"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"3.5.0"}),(0,r.jsx)(e.td,{children:"Introduced POG (Procedures On Graph query language), frontend upgrades"}),(0,r.jsx)(e.td,{children:"2023.6.5"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"3.5.1"}),(0,r.jsx)(e.td,{children:"Added learning engine, Procedure Rust API, and storage-property separation"}),(0,r.jsx)(e.td,{children:"2023.7.14"})]})]})]}),"\n",(0,r.jsx)(e.p,{children:"In addition, TuGraph has established a comprehensive quality system, including automated unit testing, integration testing, and performance testing."}),"\n",(0,r.jsx)(e.h2,{id:"3-roadmap-of-2023-s2",children:"3. Roadmap of 2023 S2"}),"\n",(0,r.jsx)(e.p,{children:"The planned high availability feature was delayed in the first half of the year, while the graph learning capability was iterated earlier than scheduled. The second half of the year will focus on completing the high availability feature and being the first to support a pre-print version of ISO GQL."}),"\n",(0,r.jsx)(e.p,{children:"| Version | Functionality | Planned Date |\n| 3.6.0 | High availability support | 2023.8 |\n| 4.0.0 | ISO GQL language support  | 2023.10 |"}),"\n",(0,r.jsx)(e.h2,{id:"4-functional-updates-in-2023",children:"4. Functional Updates in 2023"}),"\n",(0,r.jsx)(e.p,{children:"In addition to the aforementioned core functionalities, the following components are planned for development in 2023."}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(e.table,{children:[(0,r.jsx)(e.thead,{children:(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.th,{children:"Version"}),(0,r.jsx)(e.th,{children:"Functionality"}),(0,r.jsx)(e.th,{children:"Planned Date"})]})}),(0,r.jsxs)(e.tbody,{children:[(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"3.5.x"}),(0,r.jsx)(e.td,{children:"Optimization of the logging system"}),(0,r.jsx)(e.td,{children:"2023.7"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"3.5.x"}),(0,r.jsx)(e.td,{children:"Cold backup support"}),(0,r.jsx)(e.td,{children:"2023.9"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"3.5.x"}),(0,r.jsx)(e.td,{children:"Support for edge upsert import"}),(0,r.jsx)(e.td,{children:"2023.9"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"x.x.x"}),(0,r.jsx)(e.td,{children:"Trigger support exploration"}),(0,r.jsx)(e.td,{children:"2023.12"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"x.x.x"}),(0,r.jsx)(e.td,{children:"Exploring new storage engines"}),(0,r.jsx)(e.td,{children:"TBD"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"x.x.x"}),(0,r.jsx)(e.td,{children:"Rapid schema addition and deletion"}),(0,r.jsx)(e.td,{children:"TBD"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"x.x.x"}),(0,r.jsx)(e.td,{children:"TuGraph Browser version iteration"}),(0,r.jsx)(e.td,{children:"TBD"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"x.x.x"}),(0,r.jsx)(e.td,{children:"Iteration of stored procedure Python API"}),(0,r.jsx)(e.td,{children:"TBD"})]})]})]}),"\n",(0,r.jsx)(e.h2,{id:"5-community-collaboration-features",children:"5. Community Collaboration Features"}),"\n",(0,r.jsx)(e.p,{children:"Currently, the development team's resources are limited, and we cannot implement all the desired features for TuGraph. However, during the feature planning process, we have identified a range of ideas worth exploring. The team has conducted some initial exploration, and we welcome community collaboration in developing the following features:"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(e.table,{children:[(0,r.jsx)(e.thead,{children:(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.th,{children:"Version"}),(0,r.jsx)(e.th,{children:"Functionality"}),(0,r.jsx)(e.th,{children:"Planned Date"})]})}),(0,r.jsxs)(e.tbody,{children:[(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"x.x.x"}),(0,r.jsx)(e.td,{children:"Documentation improvements"}),(0,r.jsx)(e.td,{children:"TBD"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"x.x.x"}),(0,r.jsx)(e.td,{children:"Enrichment of APOC tools"}),(0,r.jsx)(e.td,{children:"TBD"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"x.x.x"}),(0,r.jsx)(e.td,{children:"Support for default property values"}),(0,r.jsx)(e.td,{children:"TBD"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"x.x.x"}),(0,r.jsx)(e.td,{children:"Composite index for multiple properties"}),(0,r.jsx)(e.td,{children:"TBD"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"x.x.x"}),(0,r.jsx)(e.td,{children:"Online full import support"}),(0,r.jsx)(e.td,{children:"TBD"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"x.x.x"}),(0,r.jsx)(e.td,{children:"Enhanced graph permissions"}),(0,r.jsx)(e.td,{children:"TBD"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"x.x.x"}),(0,r.jsx)(e.td,{children:"NLP (Natural Language Processing) support"}),(0,r.jsx)(e.td,{children:"TBD"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"x.x.x"}),(0,r.jsx)(e.td,{children:"Java Procedure API"}),(0,r.jsx)(e.td,{children:"TBD"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"x.x.x"}),(0,r.jsx)(e.td,{children:"Rapid database migration"}),(0,r.jsx)(e.td,{children:"TBD"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"x.x.x"}),(0,r.jsx)(e.td,{children:"Spatiotemporal data support"}),(0,r.jsx)(e.td,{children:"TBD"})]})]})]}),"\n",(0,r.jsx)(e.p,{children:'For simpler features, we will label them as "good first issue" on GitHub issues, and we welcome discussions from technology enthusiasts interested in graph databases.'})]})}function h(n={}){const{wrapper:e}={...(0,d.R)(),...n.components};return e?(0,r.jsx)(e,{...n,children:(0,r.jsx)(c,{...n})}):c(n)}}}]);