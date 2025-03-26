"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[1427],{28453:(e,n,r)=>{r.d(n,{R:()=>l,x:()=>t});var s=r(96540);const a={},i=s.createContext(a);function l(e){const n=s.useContext(i);return s.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function t(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(a):e.components||a:l(e.components),s.createElement(i.Provider,{value:n},e.children)}},83416:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>c,contentTitle:()=>l,default:()=>u,frontMatter:()=>i,metadata:()=>t,toc:()=>o});var s=r(74848),a=r(28453);const i={},l="WHERE",t={id:"developer-manual/interface/query/gql/clauses/where",title:"WHERE",description:"WHERE clause is used to filter records.",source:"@site/versions/version-4.1.0/en-US/source/5.developer-manual/6.interface/1.query/2.gql/2.clauses/5.where.md",sourceDirName:"5.developer-manual/6.interface/1.query/2.gql/2.clauses",slug:"/developer-manual/interface/query/gql/clauses/where",permalink:"/tugraph-db/en-US/en/4.1.0/developer-manual/interface/query/gql/clauses/where",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:5,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"NEXT",permalink:"/tugraph-db/en-US/en/4.1.0/developer-manual/interface/query/gql/clauses/next"},next:{title:"ORDER BY",permalink:"/tugraph-db/en-US/en/4.1.0/developer-manual/interface/query/gql/clauses/orderby"}},c={},o=[{value:"Baisc Usage",id:"baisc-usage",level:2},{value:"Filter vertex",id:"filter-vertex",level:3},{value:"Filter edge",id:"filter-edge",level:3},{value:"Boolean expressions",id:"boolean-expressions",level:3}];function d(e){const n={code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",p:"p",pre:"pre",...(0,a.R)(),...e.components};return(0,s.jsxs)(s.Fragment,{children:[(0,s.jsx)(n.header,{children:(0,s.jsx)(n.h1,{id:"where",children:"WHERE"})}),"\n",(0,s.jsxs)(n.p,{children:[(0,s.jsx)(n.code,{children:"WHERE"})," clause is used to filter records."]}),"\n",(0,s.jsx)(n.h2,{id:"baisc-usage",children:"Baisc Usage"}),"\n",(0,s.jsx)(n.h3,{id:"filter-vertex",children:"Filter vertex"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:"MATCH (n:Person WHERE n.birthyear > 1965)\nRETURN n.name\n"})}),"\n",(0,s.jsx)(n.p,{children:"returns"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-JSON",children:'[{"n.name":"Christopher Nolan"},{"n.name":"Lindsay Lohan"}]\n'})}),"\n",(0,s.jsx)(n.h3,{id:"filter-edge",children:"Filter edge"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:"MATCH (n:Person WHERE n.birthyear > 1965)-[e:ACTED_IN]->(m:Film)\nWHERE e.charactername = 'Halle/Annie'\nRETURN m.title\n"})}),"\n",(0,s.jsx)(n.p,{children:"returns"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-JSON",children:'[{"m.title":"The Parent Trap"}]\n'})}),"\n",(0,s.jsx)(n.h3,{id:"boolean-expressions",children:"Boolean expressions"}),"\n",(0,s.jsxs)(n.p,{children:[(0,s.jsx)(n.code,{children:"AND"}),", ",(0,s.jsx)(n.code,{children:"OR"}),", ",(0,s.jsx)(n.code,{children:"XOR"}),", and ",(0,s.jsx)(n.code,{children:"NOT"})," Boolean expressions can be used in the ",(0,s.jsx)(n.code,{children:"WHERE"})," clause to filter data."]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:"MATCH (n:Person)\nWHERE\n\tn.birthyear > 1930 AND (n.birthyear < 1950 OR n.name = 'Corin Redgrave')\nRETURN n LIMIT 2\n"})}),"\n",(0,s.jsx)(n.p,{children:"returns"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-JSON",children:'[{"n":{"identity":3,"label":"Person","properties":{"birthyear":1939,"name":"Corin Redgrave"}}},{"n":{"identity":11,"label":"Person","properties":{"birthyear":1932,"name":"John Williams"}}}]\n'})})]})}function u(e={}){const{wrapper:n}={...(0,a.R)(),...e.components};return n?(0,s.jsx)(n,{...e,children:(0,s.jsx)(d,{...e})}):d(e)}}}]);