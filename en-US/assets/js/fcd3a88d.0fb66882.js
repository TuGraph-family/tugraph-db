"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[67366],{26099:(e,t,n)=>{n.r(t),n.d(t,{assets:()=>d,contentTitle:()=>s,default:()=>c,frontMatter:()=>r,metadata:()=>a,toc:()=>l});var i=n(74848),o=n(28453);const r={},s="Token Usage Guide",a={id:"permission/token",title:"Token Usage Guide",description:"1. Introduction to Token",source:"@site/versions/version-4.2.0/en-US/source/10.permission/2.token.md",sourceDirName:"10.permission",slug:"/permission/token",permalink:"/tugraph-db/en-US/en/4.2.0/permission/token",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:2,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Privilege",permalink:"/tugraph-db/en-US/en/4.2.0/permission/privilege"},next:{title:"Forgot Admin Password",permalink:"/tugraph-db/en-US/en/4.2.0/permission/reset_admin_password"}},d={},l=[{value:"1. Introduction to Token",id:"1-introduction-to-token",level:2},{value:"2. Token Expiration",id:"2-token-expiration",level:2},{value:"2.1. Browser Token Interaction Logic",id:"21-browser-token-interaction-logic",level:3},{value:"2.3. Token Expiration Refresh Mechanism",id:"23-token-expiration-refresh-mechanism",level:3},{value:"2.4. Token Expiration Modification",id:"24-token-expiration-modification",level:3},{value:"3. Introduction to Token-Related Requests Sent by Clients",id:"3-introduction-to-token-related-requests-sent-by-clients",level:2},{value:"3.1. REST",id:"31-rest",level:3},{value:"3.2. RPC",id:"32-rpc",level:3},{value:"4. Token upper limit",id:"4-token-upper-limit",level:2}];function h(e){const t={a:"a",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",li:"li",ol:"ol",p:"p",ul:"ul",...(0,o.R)(),...e.components};return(0,i.jsxs)(i.Fragment,{children:[(0,i.jsx)(t.header,{children:(0,i.jsx)(t.h1,{id:"token-usage-guide",children:"Token Usage Guide"})}),"\n",(0,i.jsx)(t.h2,{id:"1-introduction-to-token",children:"1. Introduction to Token"}),"\n",(0,i.jsx)(t.p,{children:"JWT (JSON Web Token) is an open standard used for authentication and authorization. It is based on the JSON (JavaScript Object Notation) format and is designed for securely transmitting claim information between network applications."}),"\n",(0,i.jsx)(t.p,{children:"JWT consists of three parts: the header, payload, and signature. The header contains the type of JWT and the signature algorithm used, the payload contains the information to be transmitted, and the signature is used to verify the integrity and authenticity of the JWT."}),"\n",(0,i.jsx)(t.p,{children:"In TuGraph, JWT is used to implement a stateless authentication and authorization mechanism. After a user logs in successfully, the server generates a JWT and returns it to the client. The client passes this JWT as an identity credential in subsequent requests to the server. Upon receiving the JWT, the server verifies the signature and parses the information in the payload to determine the user's identity and permissions, and decides whether to allow the execution of the request."}),"\n",(0,i.jsx)(t.h2,{id:"2-token-expiration",children:"2. Token Expiration"}),"\n",(0,i.jsx)(t.h3,{id:"21-browser-token-interaction-logic",children:"2.1. Browser Token Interaction Logic"}),"\n",(0,i.jsxs)(t.ol,{children:["\n",(0,i.jsx)(t.li,{children:"The user opens the browser, enters the username and password, and clicks login."}),"\n",(0,i.jsx)(t.li,{children:"The frontend calls the login interface and inputs the account and password to the backend."}),"\n",(0,i.jsx)(t.li,{children:"After receiving the account and password, the backend verifies them and returns a Token."}),"\n",(0,i.jsx)(t.li,{children:"The frontend stores the Token in the browser cache, and all subsequent requests must carry this Token."}),"\n",(0,i.jsx)(t.li,{children:"If the frontend actively clicks logout or closes the page, it should actively call the logout interface and pass the Token to the backend."}),"\n",(0,i.jsx)(t.li,{children:'After receiving the Token, the backend invalidates it, returns a status code 200, and "logout successful". After receiving the message, the frontend clears the Token in the browser memory, and returns to the login page.'}),"\n",(0,i.jsx)(t.li,{children:"The Token expires (initially set to 24 hours), and the user needs to log in again."}),"\n"]}),"\n",(0,i.jsx)(t.h3,{id:"23-token-expiration-refresh-mechanism",children:"2.3. Token Expiration Refresh Mechanism"}),"\n",(0,i.jsx)(t.p,{children:"Token expiration has a refresh mechanism that is turned off by default. If turned on, the security of the Token will be higher, and there will be two timestamps."}),"\n",(0,i.jsxs)(t.p,{children:["The first timestamp ",(0,i.jsx)(t.code,{children:"refresh_time"})," is used to determine whether the Token has expired (default 24 hours): after expiration, the refresh interface can be called to obtain a new Token, which can be set to a shorter time, such as 1 hour."]}),"\n",(0,i.jsxs)(t.p,{children:["The second timestamp ",(0,i.jsx)(t.code,{children:"expire_time"})," is the forced expiration timestamp (default 24 hours): after expiration, the user must log in again."]}),"\n",(0,i.jsx)(t.h3,{id:"24-token-expiration-modification",children:"2.4. Token Expiration Modification"}),"\n",(0,i.jsx)(t.p,{children:"To facilitate developers to develop on their own, TuGraph provides two ways to modify the expiration time, both of which require admin privileges."}),"\n",(0,i.jsxs)(t.ul,{children:["\n",(0,i.jsxs)(t.li,{children:["Set through the interface call. The interfaces involved in modifying the expiration time are ",(0,i.jsx)(t.code,{children:"update_token_time"})," and ",(0,i.jsx)(t.code,{children:"get_token_time"})," for querying the expiration time. For details, please refer to the ",(0,i.jsx)(t.a,{href:"/tugraph-db/en-US/en/4.2.0/client-tools/restful-api-legacy",children:"REST interface document"}),"."]}),"\n",(0,i.jsxs)(t.li,{children:["Set through startup parameters. When the server-side is started, adding the parameter ",(0,i.jsx)(t.code,{children:"-unlimited_token 1"})," can set the Token to be unlimited. Please refer to the ",(0,i.jsx)(t.a,{href:"/tugraph-db/en-US/en/4.2.0/installation&running/tugraph-running",children:"service running document"})," for details."]}),"\n"]}),"\n",(0,i.jsx)(t.h2,{id:"3-introduction-to-token-related-requests-sent-by-clients",children:"3. Introduction to Token-Related Requests Sent by Clients"}),"\n",(0,i.jsx)(t.p,{children:"The client handles two types of protocol-related requests: REST and RPC."}),"\n",(0,i.jsx)(t.h3,{id:"31-rest",children:"3.1. REST"}),"\n",(0,i.jsx)(t.p,{children:"If the client uses the REST protocol (including the browser), because it is a short connection, Token needs to be carried in every request, and a new Token needs to be obtained after it expires."}),"\n",(0,i.jsx)(t.p,{children:"For self-developed clients, if the REST protocol is used, Token logic needs to be considered."}),"\n",(0,i.jsx)(t.h3,{id:"32-rpc",children:"3.2. RPC"}),"\n",(0,i.jsx)(t.p,{children:"If the client uses the RPC protocol (including the official C++/Java/Python), it uses a long connection to maintain, and no Token operation is involved after login."}),"\n",(0,i.jsx)(t.h2,{id:"4-token-upper-limit",children:"4. Token upper limit"}),"\n",(0,i.jsx)(t.p,{children:"The upper limit of Token refers to the maximum number of Tokens that a user can own at the same time. To prevent unlimited logins, the upper limit of Token is 10000 by default. Since the token generation logic is strongly related to time, a token will be generated for storage every time you log in. The validity period of the token is 24 hours by default. Therefore, it is recommended to log out the unused token in time after login."})]})}function c(e={}){const{wrapper:t}={...(0,o.R)(),...e.components};return t?(0,i.jsx)(t,{...e,children:(0,i.jsx)(h,{...e})}):h(e)}},28453:(e,t,n)=>{n.d(t,{R:()=>s,x:()=>a});var i=n(96540);const o={},r=i.createContext(o);function s(e){const t=i.useContext(r);return i.useMemo((function(){return"function"==typeof e?e(t):{...t,...e}}),[t,e])}function a(e){let t;return t=e.disableParentContext?"function"==typeof e.components?e.components(o):e.components||o:s(e.components),i.createElement(r.Provider,{value:t},e.children)}}}]);