"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[26561],{28453:(e,n,r)=>{r.d(n,{R:()=>s,x:()=>l});var a=r(96540);const t={},c=a.createContext(t);function s(e){const n=a.useContext(c);return a.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function l(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(t):e.components||t:s(e.components),a.createElement(c.Provider,{value:n},e.children)}},60272:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>o,contentTitle:()=>s,default:()=>d,frontMatter:()=>c,metadata:()=>l,toc:()=>i});var a=r(74848),t=r(28453);const c={},s="OPTIONAL MATCH",l={id:"developer-manual/interface/query/gql/clauses/optional_match",title:"OPTIONAL MATCH",description:"The OPTIONAL MATCH clause matches a graph pattern and returns null if there is no match.",source:"@site/versions/version-4.0.1/en-US/source/5.developer-manual/6.interface/1.query/2.gql/2.clauses/2.optional_match.md",sourceDirName:"5.developer-manual/6.interface/1.query/2.gql/2.clauses",slug:"/developer-manual/interface/query/gql/clauses/optional_match",permalink:"/tugraph-db/en/4.0.1/developer-manual/interface/query/gql/clauses/optional_match",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:2,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"MATCH",permalink:"/tugraph-db/en/4.0.1/developer-manual/interface/query/gql/clauses/match"},next:{title:"RETURN",permalink:"/tugraph-db/en/4.0.1/developer-manual/interface/query/gql/clauses/return"}},o={},i=[{value:"Basic Usage",id:"basic-usage",level:2},{value:"Match found",id:"match-found",level:3},{value:"Match Not Found",id:"match-not-found",level:3}];function u(e){const n={code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",p:"p",pre:"pre",...(0,t.R)(),...e.components};return(0,a.jsxs)(a.Fragment,{children:[(0,a.jsx)(n.header,{children:(0,a.jsx)(n.h1,{id:"optional-match",children:"OPTIONAL MATCH"})}),"\n",(0,a.jsxs)(n.p,{children:["The ",(0,a.jsx)(n.code,{children:"OPTIONAL MATCH"})," clause matches a graph pattern and returns ",(0,a.jsx)(n.code,{children:"null"})," if there is no match."]}),"\n",(0,a.jsx)(n.h2,{id:"basic-usage",children:"Basic Usage"}),"\n",(0,a.jsx)(n.h3,{id:"match-found",children:"Match found"}),"\n",(0,a.jsx)(n.pre,{children:(0,a.jsx)(n.code,{children:"OPTIONAL MATCH (n:Person{name:'Michael Redgrave'})\nRETURN n.birthyear\n"})}),"\n",(0,a.jsx)(n.p,{children:"return"}),"\n",(0,a.jsx)(n.pre,{children:(0,a.jsx)(n.code,{className:"language-JSON",children:'[{"n.birthyear":1908}]\n'})}),"\n",(0,a.jsx)(n.h3,{id:"match-not-found",children:"Match Not Found"}),"\n",(0,a.jsx)(n.pre,{children:(0,a.jsx)(n.code,{children:"OPTIONAL MATCH (n:Person{name:'Redgrave Michael'})\nRETURN n.birthyear\n"})}),"\n",(0,a.jsx)(n.p,{children:"return"}),"\n",(0,a.jsx)(n.pre,{children:(0,a.jsx)(n.code,{className:"language-JSON",children:'[{"n.birthyear":null}]\n'})})]})}function d(e={}){const{wrapper:n}={...(0,t.R)(),...e.components};return n?(0,a.jsx)(n,{...e,children:(0,a.jsx)(u,{...e})}):u(e)}}}]);