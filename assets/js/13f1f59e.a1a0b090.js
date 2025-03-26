"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[63577],{28453:(e,n,r)=>{r.d(n,{R:()=>a,x:()=>l});var s=r(96540);const i={},t=s.createContext(i);function a(e){const n=s.useContext(t);return s.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function l(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(i):e.components||i:a(e.components),s.createElement(t.Provider,{value:n},e.children)}},49490:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>c,contentTitle:()=>a,default:()=>d,frontMatter:()=>t,metadata:()=>l,toc:()=>o});var s=r(74848),i=r(28453);const t={},a="SKIP",l={id:"developer-manual/interface/query/gql/clauses/skip",title:"SKIP",description:"SKIP specifies the offset of the result rows.",source:"@site/versions/version-4.1.0/en-US/source/5.developer-manual/6.interface/1.query/2.gql/2.clauses/7.skip.md",sourceDirName:"5.developer-manual/6.interface/1.query/2.gql/2.clauses",slug:"/developer-manual/interface/query/gql/clauses/skip",permalink:"/tugraph-db/en/4.1.0/developer-manual/interface/query/gql/clauses/skip",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:7,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"ORDER BY",permalink:"/tugraph-db/en/4.1.0/developer-manual/interface/query/gql/clauses/orderby"},next:{title:"LIMIT",permalink:"/tugraph-db/en/4.1.0/developer-manual/interface/query/gql/clauses/limit"}},c={},o=[{value:"Baisc Usage",id:"baisc-usage",level:2},{value:"Without SKIP",id:"without-skip",level:3},{value:"Using SKIP",id:"using-skip",level:3}];function u(e){const n={code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",p:"p",pre:"pre",...(0,i.R)(),...e.components};return(0,s.jsxs)(s.Fragment,{children:[(0,s.jsx)(n.header,{children:(0,s.jsx)(n.h1,{id:"skip",children:"SKIP"})}),"\n",(0,s.jsxs)(n.p,{children:[(0,s.jsx)(n.code,{children:"SKIP"})," specifies the offset of the result rows."]}),"\n",(0,s.jsx)(n.h2,{id:"baisc-usage",children:"Baisc Usage"}),"\n",(0,s.jsx)(n.h3,{id:"without-skip",children:"Without SKIP"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:"MATCH (n:Person)\nRETURN n.name LIMIT 3\n"})}),"\n",(0,s.jsx)(n.p,{children:"return"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-JSON",children:'[{"n.name":"Christopher Nolan"},{"n.name":"Corin Redgrave"},{"n.name":"Dennis Quaid"}]\n'})}),"\n",(0,s.jsx)(n.h3,{id:"using-skip",children:"Using SKIP"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:"MATCH (n:Person)\nRETURN n.name SKIP 1 LIMIT 2\n"})}),"\n",(0,s.jsx)(n.p,{children:"return"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-JSON",children:'[{"n.name":"Corin Redgrave"},{"n.name":"Dennis Quaid"}]\n'})})]})}function d(e={}){const{wrapper:n}={...(0,i.R)(),...e.components};return n?(0,s.jsx)(n,{...e,children:(0,s.jsx)(u,{...e})}):u(e)}}}]);