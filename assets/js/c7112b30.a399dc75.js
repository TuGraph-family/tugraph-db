"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[70134],{86360:(e,r,n)=>{n.r(r),n.d(r,{assets:()=>o,contentTitle:()=>c,default:()=>d,frontMatter:()=>a,metadata:()=>l,toc:()=>i});var t=n(74848),s=n(28453);const a={},c="ORDER BY",l={id:"developer-manual/interface/query/gql/clauses/orderby",title:"ORDER BY",description:"ORDER BY\u662fRETURN\u7684\u5b50\u53e5\uff0c\u5bf9\u8f93\u51fa\u7684\u7ed3\u679c\u8fdb\u884c\u6392\u5e8f\u3002",source:"@site/versions/version-4.0.1/zh-CN/source/5.developer-manual/6.interface/1.query/2.gql/2.clauses/6.orderby.md",sourceDirName:"5.developer-manual/6.interface/1.query/2.gql/2.clauses",slug:"/developer-manual/interface/query/gql/clauses/orderby",permalink:"/tugraph-db/zh/4.0.1/developer-manual/interface/query/gql/clauses/orderby",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:6,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"WHERE",permalink:"/tugraph-db/zh/4.0.1/developer-manual/interface/query/gql/clauses/where"},next:{title:"SKIP",permalink:"/tugraph-db/zh/4.0.1/developer-manual/interface/query/gql/clauses/skip"}},o={},i=[{value:"\u57fa\u672c\u7528\u6cd5",id:"\u57fa\u672c\u7528\u6cd5",level:2},{value:"\u5bf9\u7ed3\u679c\u6392\u5e8f",id:"\u5bf9\u7ed3\u679c\u6392\u5e8f",level:3}];function u(e){const r={code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",p:"p",pre:"pre",...(0,s.R)(),...e.components};return(0,t.jsxs)(t.Fragment,{children:[(0,t.jsx)(r.header,{children:(0,t.jsx)(r.h1,{id:"order-by",children:"ORDER BY"})}),"\n",(0,t.jsxs)(r.p,{children:[(0,t.jsx)(r.code,{children:"ORDER BY"}),"\u662f",(0,t.jsx)(r.code,{children:"RETURN"}),"\u7684\u5b50\u53e5\uff0c\u5bf9\u8f93\u51fa\u7684\u7ed3\u679c\u8fdb\u884c\u6392\u5e8f\u3002"]}),"\n",(0,t.jsx)(r.h2,{id:"\u57fa\u672c\u7528\u6cd5",children:"\u57fa\u672c\u7528\u6cd5"}),"\n",(0,t.jsx)(r.h3,{id:"\u5bf9\u7ed3\u679c\u6392\u5e8f",children:"\u5bf9\u7ed3\u679c\u6392\u5e8f"}),"\n",(0,t.jsx)(r.pre,{children:(0,t.jsx)(r.code,{children:"MATCH (n:Person WHERE n.birthyear < 1970)\nRETURN n.birthyear AS q\nORDER BY q ASC\nLIMIT 5\n"})}),"\n",(0,t.jsx)(r.p,{children:"\u8fd4\u56de\u7ed3\u679c"}),"\n",(0,t.jsx)(r.pre,{children:(0,t.jsx)(r.code,{className:"language-JSON",children:'[{"q":1873},{"q":1908},{"q":1910},{"q":1930},{"q":1932}]\n'})})]})}function d(e={}){const{wrapper:r}={...(0,s.R)(),...e.components};return r?(0,t.jsx)(r,{...e,children:(0,t.jsx)(u,{...e})}):u(e)}},28453:(e,r,n)=>{n.d(r,{R:()=>c,x:()=>l});var t=n(96540);const s={},a=t.createContext(s);function c(e){const r=t.useContext(a);return t.useMemo((function(){return"function"==typeof e?e(r):{...r,...e}}),[r,e])}function l(e){let r;return r=e.disableParentContext?"function"==typeof e.components?e.components(s):e.components||s:c(e.components),t.createElement(a.Provider,{value:r},e.children)}}}]);