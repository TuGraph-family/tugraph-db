"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[28866],{28453:(e,n,r)=>{r.d(n,{R:()=>t,x:()=>l});var s=r(96540);const a={},i=s.createContext(a);function t(e){const n=s.useContext(i);return s.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function l(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(a):e.components||a:t(e.components),s.createElement(i.Provider,{value:n},e.children)}},70089:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>c,contentTitle:()=>t,default:()=>u,frontMatter:()=>i,metadata:()=>l,toc:()=>o});var s=r(74848),a=r(28453);const i={},t="SKIP",l={id:"developer-manual/interface/query/gql/clauses/skip",title:"SKIP",description:"SKIP\u6307\u5b9a\u7ed3\u679c\u504f\u79fb\u884c\u6570\u3002",source:"@site/versions/version-4.0.1/zh-CN/source/5.developer-manual/6.interface/1.query/2.gql/2.clauses/7.skip.md",sourceDirName:"5.developer-manual/6.interface/1.query/2.gql/2.clauses",slug:"/developer-manual/interface/query/gql/clauses/skip",permalink:"/tugraph-db/en-US/zh/4.0.1/developer-manual/interface/query/gql/clauses/skip",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:7,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"ORDER BY",permalink:"/tugraph-db/en-US/zh/4.0.1/developer-manual/interface/query/gql/clauses/orderby"},next:{title:"LIMIT",permalink:"/tugraph-db/en-US/zh/4.0.1/developer-manual/interface/query/gql/clauses/limit"}},c={},o=[{value:"\u57fa\u672c\u7528\u6cd5",id:"\u57fa\u672c\u7528\u6cd5",level:2},{value:"\u672a\u4f7f\u7528SKIP",id:"\u672a\u4f7f\u7528skip",level:3},{value:"\u4f7f\u7528SKIP",id:"\u4f7f\u7528skip",level:3}];function d(e){const n={code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",p:"p",pre:"pre",...(0,a.R)(),...e.components};return(0,s.jsxs)(s.Fragment,{children:[(0,s.jsx)(n.header,{children:(0,s.jsx)(n.h1,{id:"skip",children:"SKIP"})}),"\n",(0,s.jsxs)(n.p,{children:[(0,s.jsx)(n.code,{children:"SKIP"}),"\u6307\u5b9a\u7ed3\u679c\u504f\u79fb\u884c\u6570\u3002"]}),"\n",(0,s.jsx)(n.h2,{id:"\u57fa\u672c\u7528\u6cd5",children:"\u57fa\u672c\u7528\u6cd5"}),"\n",(0,s.jsx)(n.h3,{id:"\u672a\u4f7f\u7528skip",children:"\u672a\u4f7f\u7528SKIP"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:"MATCH (n:Person)\nRETURN n.name LIMIT 3\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u8fd4\u56de\u7ed3\u679c"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-JSON",children:'[{"n.name":"Christopher Nolan"},{"n.name":"Corin Redgrave"},{"n.name":"Dennis Quaid"}]\n'})}),"\n",(0,s.jsx)(n.h3,{id:"\u4f7f\u7528skip",children:"\u4f7f\u7528SKIP"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:"MATCH (n:Person)\nRETURN n.name SKIP 1 LIMIT 2\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u8fd4\u56de\u7ed3\u679c"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-JSON",children:'[{"n.name":"Corin Redgrave"},{"n.name":"Dennis Quaid"}]\n'})})]})}function u(e={}){const{wrapper:n}={...(0,a.R)(),...e.components};return n?(0,s.jsx)(n,{...e,children:(0,s.jsx)(d,{...e})}):d(e)}}}]);