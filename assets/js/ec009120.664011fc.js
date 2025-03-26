"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[8035],{22638:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>c,contentTitle:()=>s,default:()=>h,frontMatter:()=>i,metadata:()=>d,toc:()=>t});var l=r(74848),a=r(28453);const i={},s="RETURN",d={id:"developer-manual/interface/query/gql/clauses/return",title:"RETURN",description:"RETURN\u5b50\u53e5\u6307\u5b9a\u8fd4\u56de\u7ed3\u679c\uff0c\u5305\u62ec\u8fd4\u56de\u70b9\u3001\u8fb9\u3001\u8def\u5f84\u3001\u5c5e\u6027\u7b49\u3002",source:"@site/versions/version-4.1.0/zh-CN/source/5.developer-manual/6.interface/1.query/2.gql/2.clauses/3.return.md",sourceDirName:"5.developer-manual/6.interface/1.query/2.gql/2.clauses",slug:"/developer-manual/interface/query/gql/clauses/return",permalink:"/tugraph-db/zh/4.1.0/developer-manual/interface/query/gql/clauses/return",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:3,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"OPTIONAL MATCH",permalink:"/tugraph-db/zh/4.1.0/developer-manual/interface/query/gql/clauses/optional_match"},next:{title:"NEXT",permalink:"/tugraph-db/zh/4.1.0/developer-manual/interface/query/gql/clauses/next"}},c={},t=[{value:"\u57fa\u672c\u7528\u6cd5",id:"\u57fa\u672c\u7528\u6cd5",level:2},{value:"\u8fd4\u56de\u70b9",id:"\u8fd4\u56de\u70b9",level:3},{value:"\u8fd4\u56de\u8fb9",id:"\u8fd4\u56de\u8fb9",level:3},{value:"\u8fd4\u56de\u5c5e\u6027",id:"\u8fd4\u56de\u5c5e\u6027",level:3},{value:"\u4e0d\u5e38\u89c1\u5b57\u7b26\u4e32\u4f5c\u4e3a\u53d8\u91cf\u540d",id:"\u4e0d\u5e38\u89c1\u5b57\u7b26\u4e32\u4f5c\u4e3a\u53d8\u91cf\u540d",level:3},{value:"\u5217\u522b\u540d",id:"\u5217\u522b\u540d",level:3},{value:"\u53ef\u9009\u5c5e\u6027",id:"\u53ef\u9009\u5c5e\u6027",level:3},{value:"\u5176\u5b83\u8868\u8fbe\u5f0f",id:"\u5176\u5b83\u8868\u8fbe\u5f0f",level:3},{value:"\u7ed3\u679c\u552f\u4e00\u6027",id:"\u7ed3\u679c\u552f\u4e00\u6027",level:3}];function o(e){const n={code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",p:"p",pre:"pre",...(0,a.R)(),...e.components};return(0,l.jsxs)(l.Fragment,{children:[(0,l.jsx)(n.header,{children:(0,l.jsx)(n.h1,{id:"return",children:"RETURN"})}),"\n",(0,l.jsxs)(n.p,{children:[(0,l.jsx)(n.code,{children:"RETURN"}),"\u5b50\u53e5\u6307\u5b9a\u8fd4\u56de\u7ed3\u679c\uff0c\u5305\u62ec\u8fd4\u56de\u70b9\u3001\u8fb9\u3001\u8def\u5f84\u3001\u5c5e\u6027\u7b49\u3002"]}),"\n",(0,l.jsx)(n.h2,{id:"\u57fa\u672c\u7528\u6cd5",children:"\u57fa\u672c\u7528\u6cd5"}),"\n",(0,l.jsx)(n.h3,{id:"\u8fd4\u56de\u70b9",children:"\u8fd4\u56de\u70b9"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{children:"MATCH (n)\nRETURN n LIMIT 2\n"})}),"\n",(0,l.jsx)(n.p,{children:"\u8fd4\u56de\u7ed3\u679c"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-JSON",children:'[{"n":{"identity":0,"label":"Person","properties":{"birthyear":1910,"name":"Rachel Kempson"}}},{"n":{"identity":1,"label":"Person","properties":{"birthyear":1908,"name":"Michael Redgrave"}}}]\n'})}),"\n",(0,l.jsx)(n.h3,{id:"\u8fd4\u56de\u8fb9",children:"\u8fd4\u56de\u8fb9"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{children:"MATCH (n)-[e]->(m)\nRETURN e LIMIT 2\n"})}),"\n",(0,l.jsx)(n.p,{children:"\u8fd4\u56de\u7ed3\u679c"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-JSON",children:'[{"e":{"dst":2,"forward":false,"identity":0,"label":"HAS_CHILD","label_id":0,"src":0,"temporal_id":0}},{"e":{"dst":3,"forward":false,"identity":0,"label":"HAS_CHILD","label_id":0,"src":0,"temporal_id":0}}]\n'})}),"\n",(0,l.jsx)(n.h3,{id:"\u8fd4\u56de\u5c5e\u6027",children:"\u8fd4\u56de\u5c5e\u6027"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{children:"MATCH (n:Person)\nRETURN n.name LIMIT 2\n"})}),"\n",(0,l.jsx)(n.p,{children:"\u8fd4\u56de\u7ed3\u679c"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-JSON",children:'[{"n.name":"Christopher Nolan"},{"n.name":"Corin Redgrave"}]\n'})}),"\n",(0,l.jsx)(n.h3,{id:"\u4e0d\u5e38\u89c1\u5b57\u7b26\u4e32\u4f5c\u4e3a\u53d8\u91cf\u540d",children:"\u4e0d\u5e38\u89c1\u5b57\u7b26\u4e32\u4f5c\u4e3a\u53d8\u91cf\u540d"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{children:"MATCH (`/uncommon variable`:Person)\nRETURN `/uncommon variable`.name LIMIT 3\n"})}),"\n",(0,l.jsx)(n.p,{children:"\u8fd4\u56de\u7ed3\u679c"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-JSON",children:'[{"`/uncommon variable`.name":"Christopher Nolan"},{"`/uncommon variable`.name":"Corin Redgrave"},{"`/uncommon variable`.name":"Dennis Quaid"}]\n'})}),"\n",(0,l.jsx)(n.h3,{id:"\u5217\u522b\u540d",children:"\u5217\u522b\u540d"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{children:"MATCH (n:Person)\nRETURN n.name AS nname LIMIT 2\n"})}),"\n",(0,l.jsx)(n.p,{children:"\u8fd4\u56de\u7ed3\u679c"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-JSON",children:'[{"nname":"Christopher Nolan"},{"nname":"Corin Redgrave"}]\n'})}),"\n",(0,l.jsx)(n.h3,{id:"\u53ef\u9009\u5c5e\u6027",children:"\u53ef\u9009\u5c5e\u6027"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{children:"MATCH (n:Person)\nRETURN n.age LIMIT 2\n"})}),"\n",(0,l.jsx)(n.p,{children:"\u8fd4\u56de\u7ed3\u679c"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-JSON",children:'[{"n.age":null},{"n.age":null}]\n'})}),"\n",(0,l.jsx)(n.h3,{id:"\u5176\u5b83\u8868\u8fbe\u5f0f",children:"\u5176\u5b83\u8868\u8fbe\u5f0f"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{children:'MATCH (n:Person)\nRETURN n.birthyear > 1970, "I\'m a literal", 1 + 2, abs(-2)\nLIMIT 2\n'})}),"\n",(0,l.jsx)(n.p,{children:"\u8fd4\u56de\u7ed3\u679c"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-JSON",children:'[{"\\"I\'m a literal\\"":"I\'m a literal","1 + 2":3,"abs(-2)":2,"n.birthyear > 1970":false},{"\\"I\'m a literal\\"":"I\'m a literal","1 + 2":3,"abs(-2)":2,"n.birthyear > 1970":false}]\n'})}),"\n",(0,l.jsx)(n.h3,{id:"\u7ed3\u679c\u552f\u4e00\u6027",children:"\u7ed3\u679c\u552f\u4e00\u6027"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{children:"MATCH (n)\nRETURN DISTINCT label(n) AS label\n"})}),"\n",(0,l.jsx)(n.p,{children:"\u8fd4\u56de\u7ed3\u679c"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-JSON",children:'[{"label":"Person"},{"label":"City"},{"label":"Film"}]\n'})})]})}function h(e={}){const{wrapper:n}={...(0,a.R)(),...e.components};return n?(0,l.jsx)(n,{...e,children:(0,l.jsx)(o,{...e})}):o(e)}},28453:(e,n,r)=>{r.d(n,{R:()=>s,x:()=>d});var l=r(96540);const a={},i=l.createContext(a);function s(e){const n=l.useContext(i);return l.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function d(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(a):e.components||a:s(e.components),l.createElement(i.Provider,{value:n},e.children)}}}]);