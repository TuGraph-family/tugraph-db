"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[33384],{28453:(e,n,r)=>{r.d(n,{R:()=>i,x:()=>h});var s=r(96540);const t={},d=s.createContext(t);function i(e){const n=s.useContext(d);return s.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function h(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(t):e.components||t:i(e.components),s.createElement(d.Provider,{value:n},e.children)}},90354:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>l,contentTitle:()=>i,default:()=>o,frontMatter:()=>d,metadata:()=>h,toc:()=>c});var s=r(74848),t=r(28453);const d={},i="TuGraph RESTful API",h={id:"client-tools/restful-api",title:"TuGraph RESTful API",description:"This document describes how to call the Rest API of TuGrpah.",source:"@site/versions/version-4.5.2/en-US/source/7.client-tools/7.restful-api.md",sourceDirName:"7.client-tools",slug:"/client-tools/restful-api",permalink:"/tugraph-db/en-US/en/4.5.2/client-tools/restful-api",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:7,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"TuGraph console client",permalink:"/tugraph-db/en-US/en/4.5.2/client-tools/bolt-console-client"},next:{title:"RPC API",permalink:"/tugraph-db/en-US/en/4.5.2/client-tools/rpc-api"}},l={},c=[{value:"1.Introduction",id:"1introduction",level:2},{value:"2.Request And Response Format",id:"2request-and-response-format",level:2},{value:"2.1.Standard Response Format",id:"21standard-response-format",level:3},{value:"2.2. Request Type",id:"22-request-type",level:3},{value:"2.2.1. User Login",id:"221-user-login",level:4},{value:"2.2.2. User Logout",id:"222-user-logout",level:4},{value:"2.2.3. Refresh Token",id:"223-refresh-token",level:4},{value:"2.2.4. Call Cypher",id:"224-call-cypher",level:4}];function a(e){const n={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",h4:"h4",header:"header",li:"li",p:"p",pre:"pre",strong:"strong",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",ul:"ul",...(0,t.R)(),...e.components};return(0,s.jsxs)(s.Fragment,{children:[(0,s.jsx)(n.header,{children:(0,s.jsx)(n.h1,{id:"tugraph-restful-api",children:"TuGraph RESTful API"})}),"\n",(0,s.jsxs)(n.blockquote,{children:["\n",(0,s.jsx)(n.p,{children:"This document describes how to call the Rest API of TuGrpah."}),"\n"]}),"\n",(0,s.jsx)(n.h2,{id:"1introduction",children:"1.Introduction"}),"\n",(0,s.jsx)(n.p,{children:"TuGraph provides HTTP RESTful APIs, which allow users to access TuGraph servers through HTTP requests remotely."}),"\n",(0,s.jsx)(n.p,{children:"This document specifiers the TuGraph HTTP RESTful API."}),"\n",(0,s.jsx)(n.h2,{id:"2request-and-response-format",children:"2.Request And Response Format"}),"\n",(0,s.jsx)(n.p,{children:"Tugraph HTTP Server receives requests in json format\uff0cAfter authentication, fields in the request are extracted, the request is processed according to the defined interface logic, and the response in json format is returned."}),"\n",(0,s.jsx)(n.h3,{id:"21standard-response-format",children:"2.1.Standard Response Format"}),"\n",(0,s.jsx)(n.p,{children:"Each response is returned in the standard response format, as follows\uff1a"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"}),(0,s.jsx)(n.th,{children:"necessary"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"errorCode"}),(0,s.jsx)(n.td,{children:"error code"}),(0,s.jsx)(n.td,{children:"int"}),(0,s.jsx)(n.td,{children:"yes"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"success"}),(0,s.jsx)(n.td,{children:"Whether the request was successful"}),(0,s.jsx)(n.td,{children:"int"}),(0,s.jsx)(n.td,{children:"yes"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"errorMessage"}),(0,s.jsx)(n.td,{children:"error message"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"data"}),(0,s.jsx)(n.td,{children:"the response information returned when the request is successful"}),(0,s.jsx)(n.td,{children:"json string"}),(0,s.jsx)(n.td,{children:"yes"})]})]})]}),"\n",(0,s.jsx)(n.h3,{id:"22-request-type",children:"2.2. Request Type"}),"\n",(0,s.jsx)(n.h4,{id:"221-user-login",children:"2.2.1. User Login"}),"\n",(0,s.jsx)(n.p,{children:"The user sends the login request to the server with the user name and password. After successful login, the client receives a signed token (the Json Web Token) and a Boolean variable (default_password) to determine whether it is the default password. The jwt token stored by the client and added to the Authorization domain of the request header in subsequent requests. If the login fails, you will receive the Authentication failed error."}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"URI"}),":     /login"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"REQUEST"}),":"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"}),(0,s.jsx)(n.th,{children:"necessary"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"user"}),(0,s.jsx)(n.td,{children:"name of the user"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"password"}),(0,s.jsx)(n.td,{children:"password of the user"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]})]})]}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"RESPONSE"}),":\nIf successful, the success field in the returned response message will be set to 00 and the token will be included in data"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"jwt"}),(0,s.jsx)(n.td,{children:"token"}),(0,s.jsx)(n.td,{children:"string"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"is_admin"}),(0,s.jsx)(n.td,{children:"Whether the user is an admin"}),(0,s.jsx)(n.td,{children:"bool"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"default_password"}),(0,s.jsx)(n.td,{children:"Whether it is the default password"}),(0,s.jsx)(n.td,{children:"bool"})]})]})]}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.strong,{children:"Example request."})}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:'    {"user" : "admin", "password" : "73@TuGraph"}\n'})}),"\n",(0,s.jsx)(n.h4,{id:"222-user-logout",children:"2.2.2. User Logout"}),"\n",(0,s.jsx)(n.p,{children:"When a user logs out, the authenticated token is deleted. and the user needs to log in again to obtain a new token when sending subsequent requests."}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"URI"}),":     /logout"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"REQUEST"}),":\nThe http request header carries the token returned by login interface. The specific string to be filled is ",(0,s.jsx)(n.code,{children:"Bearer ${jwt}"}),", where ",(0,s.jsx)(n.code,{children:"${jwt}"}),' is the "jwt" returned from the login interface\uff0cand the body has no parameters']}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"header parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"})]})}),(0,s.jsx)(n.tbody,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"Authorization"}),(0,s.jsx)(n.td,{children:"Bearer ${jwt}"}),(0,s.jsx)(n.td,{children:"string"})]})})]}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"RESPONSE"}),":\nIf successful, the success field in the returned response message will be set to 00, and data is"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"})]})}),(0,s.jsx)(n.tbody,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"is_admin"}),(0,s.jsx)(n.td,{children:"Whether the user is an admin"}),(0,s.jsx)(n.td,{children:"bool"})]})})]}),"\n",(0,s.jsx)(n.h4,{id:"223-refresh-token",children:"2.2.3. Refresh Token"}),"\n",(0,s.jsx)(n.p,{children:"If the delivered token becomes invalid, you need to invoke this interface for re-authentication. The token is valid within one hour after the first login and needs to be refreshed"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"URI"}),":     /refresh"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"REQUEST"}),":\nThe http request header carries the token returned by login interface. The specific string to be filled is ",(0,s.jsx)(n.code,{children:"Bearer ${jwt}"}),", where ",(0,s.jsx)(n.code,{children:"${jwt}"}),' is the "jwt" returned from the login interface\uff0cand the body has no parameters']}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"header parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"})]})}),(0,s.jsx)(n.tbody,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"Authorization"}),(0,s.jsx)(n.td,{children:"Bearer ${jwt}"}),(0,s.jsx)(n.td,{children:"string"})]})})]}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"RESPONSE"}),":\nIf successful, the success field in the returned response message will be set to 00, and the token will be included in data"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"jwt"}),(0,s.jsx)(n.td,{children:"token"}),(0,s.jsx)(n.td,{children:"string"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"is_admin"}),(0,s.jsx)(n.td,{children:"Whether the user is an admin"}),(0,s.jsx)(n.td,{children:"bool"})]})]})]}),"\n",(0,s.jsx)(n.h4,{id:"224-call-cypher",children:"2.2.4. Call Cypher"}),"\n",(0,s.jsx)(n.p,{children:"User manipulation of data and models in tugraph requires calling the cypher interface and is initiated through the standard cypher query language"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"URI"}),":     /cypher"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"REQUEST"}),":"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"header parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"})]})}),(0,s.jsx)(n.tbody,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"Authorization"}),(0,s.jsx)(n.td,{children:"Bearer ${jwt}"}),(0,s.jsx)(n.td,{children:"string"})]})})]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"}),(0,s.jsx)(n.th,{children:"necessary"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"graph"}),(0,s.jsx)(n.td,{children:"the name of the subgraph to be queried"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"script"}),(0,s.jsx)(n.td,{children:"query statement"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]})]})]}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"RESPONSE"}),":\nIf successful, the success field in the returned response message will be set to 00, and the query results will be included in data"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"}),(0,s.jsx)(n.th,{children:"necessary"})]})}),(0,s.jsx)(n.tbody,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"result"}),(0,s.jsx)(n.td,{children:"query results"}),(0,s.jsx)(n.td,{children:"json string"}),(0,s.jsx)(n.td,{children:"yes"})]})})]}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.strong,{children:"Example request."})}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:'    {"script" : "Match (n) return n", "graph" : "default"}\n'})})]})}function o(e={}){const{wrapper:n}={...(0,t.R)(),...e.components};return n?(0,s.jsx)(n,{...e,children:(0,s.jsx)(a,{...e})}):a(e)}}}]);