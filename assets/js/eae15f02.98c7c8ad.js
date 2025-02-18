"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[96432],{28453:(e,n,t)=>{t.d(n,{R:()=>d,x:()=>l});var s=t(96540);const r={},i=s.createContext(r);function d(e){const n=s.useContext(i);return s.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function l(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(r):e.components||r:d(e.components),s.createElement(i.Provider,{value:n},e.children)}},68275:(e,n,t)=>{t.r(n),t.d(n,{assets:()=>h,contentTitle:()=>d,default:()=>o,frontMatter:()=>i,metadata:()=>l,toc:()=>c});var s=t(74848),r=t(28453);const i={},d="TuGraph RESTful API",l={id:"client-tools/restful-api",title:"TuGraph RESTful API",description:"This document describes how to call the Rest API of TuGrpah.",source:"@site/versions/version-4.2.0/en-US/source/7.client-tools/7.restful-api.md",sourceDirName:"7.client-tools",slug:"/client-tools/restful-api",permalink:"/tugraph-db/en/4.2.0/client-tools/restful-api",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:7,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"TuGraph console client",permalink:"/tugraph-db/en/4.2.0/client-tools/bolt-console-client"},next:{title:"RPC API",permalink:"/tugraph-db/en/4.2.0/client-tools/rpc-api"}},h={},c=[{value:"1.Introduction",id:"1introduction",level:2},{value:"2.Request And Response Format",id:"2request-and-response-format",level:2},{value:"2.1.Standard Response Format",id:"21standard-response-format",level:3},{value:"2.2. Request Type",id:"22-request-type",level:3},{value:"2.2.1. User Login",id:"221-user-login",level:4},{value:"2.2.2. User Logout",id:"222-user-logout",level:4},{value:"2.2.3. Refresh Token",id:"223-refresh-token",level:4},{value:"2.2.4. Call Cypher",id:"224-call-cypher",level:4},{value:"2.2.5. Upload File",id:"225-upload-file",level:4},{value:"2.2.6. Check File",id:"226-check-file",level:4},{value:"2.2.7. Delete Cached File",id:"227-delete-cached-file",level:4},{value:"2.2.8. Import Schema",id:"228-import-schema",level:4},{value:"2.2.9. Import Data",id:"229-import-data",level:4},{value:"2.2.10. Import Progress Query",id:"2210-import-progress-query",level:4}];function a(e){const n={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",h4:"h4",header:"header",li:"li",p:"p",pre:"pre",strong:"strong",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",ul:"ul",...(0,r.R)(),...e.components};return(0,s.jsxs)(s.Fragment,{children:[(0,s.jsx)(n.header,{children:(0,s.jsx)(n.h1,{id:"tugraph-restful-api",children:"TuGraph RESTful API"})}),"\n",(0,s.jsxs)(n.blockquote,{children:["\n",(0,s.jsx)(n.p,{children:"This document describes how to call the Rest API of TuGrpah."}),"\n"]}),"\n",(0,s.jsx)(n.h2,{id:"1introduction",children:"1.Introduction"}),"\n",(0,s.jsx)(n.p,{children:"TuGraph provides HTTP RESTful APIs, which allow users to access TuGraph servers through HTTP requests remotely."}),"\n",(0,s.jsx)(n.p,{children:"This document specifiers the TuGraph HTTP RESTful API."}),"\n",(0,s.jsx)(n.h2,{id:"2request-and-response-format",children:"2.Request And Response Format"}),"\n",(0,s.jsx)(n.p,{children:"Tugraph HTTP Server receives requests in json format\uff0cAfter authentication, fields in the request are extracted, the request is processed according to the defined interface logic, and the response in json format is returned."}),"\n",(0,s.jsx)(n.h3,{id:"21standard-response-format",children:"2.1.Standard Response Format"}),"\n",(0,s.jsx)(n.p,{children:"Each response is returned in the standard response format, as follows\uff1a"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"}),(0,s.jsx)(n.th,{children:"necessary"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"errorCode"}),(0,s.jsx)(n.td,{children:"error code"}),(0,s.jsx)(n.td,{children:"int"}),(0,s.jsx)(n.td,{children:"yes"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"success"}),(0,s.jsx)(n.td,{children:"Whether the request was successful"}),(0,s.jsx)(n.td,{children:"int"}),(0,s.jsx)(n.td,{children:"yes"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"errorMessage"}),(0,s.jsx)(n.td,{children:"error message"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"data"}),(0,s.jsx)(n.td,{children:"the response information returned when the request is successful"}),(0,s.jsx)(n.td,{children:"json string"}),(0,s.jsx)(n.td,{children:"yes"})]})]})]}),"\n",(0,s.jsx)(n.h3,{id:"22-request-type",children:"2.2. Request Type"}),"\n",(0,s.jsx)(n.h4,{id:"221-user-login",children:"2.2.1. User Login"}),"\n",(0,s.jsx)(n.p,{children:"The user sends the login request to the server with the user name and password. After successful login, the client receives a signed token (the Json Web Token) and a Boolean variable (default_password) to determine whether it is the default password. The jwt token stored by the client and added to the Authorization domain of the request header in subsequent requests. If the login fails, you will receive the Authentication failed error."}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"URI"}),":     /login"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"REQUEST"}),":"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"}),(0,s.jsx)(n.th,{children:"necessary"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"userName"}),(0,s.jsx)(n.td,{children:"name of the user"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"password"}),(0,s.jsx)(n.td,{children:"password of the user"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]})]})]}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"RESPONSE"}),":\nIf successful, the success field in the returned response message will be set to 00 and the token will be included in data"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"}),(0,s.jsx)(n.th,{children:"necessary"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"authorization"}),(0,s.jsx)(n.td,{children:"token"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"default_password"}),(0,s.jsx)(n.td,{children:"whether the password is the default password"}),(0,s.jsx)(n.td,{children:"bool"}),(0,s.jsx)(n.td,{children:"yes"})]})]})]}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.strong,{children:"Example request."})}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:'    {"userName" : "test", "password" : "123456"}\n'})}),"\n",(0,s.jsx)(n.h4,{id:"222-user-logout",children:"2.2.2. User Logout"}),"\n",(0,s.jsx)(n.p,{children:"When a user logs out, the authenticated token is deleted. and the user needs to log in again to obtain a new token when sending subsequent requests."}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:["\n",(0,s.jsxs)(n.p,{children:[(0,s.jsx)(n.strong,{children:"URI"}),":     /logout"]}),"\n"]}),"\n",(0,s.jsxs)(n.li,{children:["\n",(0,s.jsxs)(n.p,{children:[(0,s.jsx)(n.strong,{children:"METHOD"}),":  POST"]}),"\n"]}),"\n",(0,s.jsxs)(n.li,{children:["\n",(0,s.jsxs)(n.p,{children:[(0,s.jsx)(n.strong,{children:"REQUEST"}),":\nThe http request header carries the token returned by login interface, and the body has no parameters"]}),"\n"]}),"\n",(0,s.jsxs)(n.li,{children:["\n",(0,s.jsxs)(n.p,{children:[(0,s.jsx)(n.strong,{children:"RESPONSE"}),":\nIf successful, the success field in the returned response message will be set to 00, and data is empty"]}),"\n"]}),"\n"]}),"\n",(0,s.jsx)(n.h4,{id:"223-refresh-token",children:"2.2.3. Refresh Token"}),"\n",(0,s.jsx)(n.p,{children:"If the delivered token becomes invalid, you need to invoke this interface for re-authentication. The token is valid within one hour after the first login and needs to be refreshed"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:["\n",(0,s.jsxs)(n.p,{children:[(0,s.jsx)(n.strong,{children:"URI"}),":     /refresh"]}),"\n"]}),"\n",(0,s.jsxs)(n.li,{children:["\n",(0,s.jsxs)(n.p,{children:[(0,s.jsx)(n.strong,{children:"METHOD"}),":  POST"]}),"\n"]}),"\n",(0,s.jsxs)(n.li,{children:["\n",(0,s.jsxs)(n.p,{children:[(0,s.jsx)(n.strong,{children:"REQUEST"}),":\nThe http request header carries the token returned by login interface, and the body has no parameters"]}),"\n"]}),"\n",(0,s.jsxs)(n.li,{children:["\n",(0,s.jsxs)(n.p,{children:[(0,s.jsx)(n.strong,{children:"RESPONSE"}),":\nIf successful, the success field in the returned response message will be set to 00, and the token will be included in data"]}),"\n"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"}),(0,s.jsx)(n.th,{children:"necessary"})]})}),(0,s.jsx)(n.tbody,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"authorization"}),(0,s.jsx)(n.td,{children:"token"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]})})]}),"\n",(0,s.jsx)(n.h4,{id:"224-call-cypher",children:"2.2.4. Call Cypher"}),"\n",(0,s.jsx)(n.p,{children:"User manipulation of data and models in tugraph requires calling the cypher interface and is initiated through the standard cypher query language"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"URI"}),":     /cypher"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"REQUEST"}),":"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"}),(0,s.jsx)(n.th,{children:"necessary"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"graph"}),(0,s.jsx)(n.td,{children:"the name of the subgraph to be queried"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"script"}),(0,s.jsx)(n.td,{children:"query statement"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]})]})]}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"RESPONSE"}),":\nIf successful, the success field in the returned response message will be set to 00, and the query results will be included in data"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"}),(0,s.jsx)(n.th,{children:"necessary"})]})}),(0,s.jsx)(n.tbody,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"result"}),(0,s.jsx)(n.td,{children:"query results"}),(0,s.jsx)(n.td,{children:"json string"}),(0,s.jsx)(n.td,{children:"yes"})]})})]}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.strong,{children:"Example request."})}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:'    {"script" : "Match (n) return n", "graph" : "default"}\n'})}),"\n",(0,s.jsx)(n.h4,{id:"225-upload-file",children:"2.2.5. Upload File"}),"\n",(0,s.jsx)(n.p,{children:"This interface is used to upload local files to the TuGraph machine. You can upload text/binary files, large files, and small files. For large files, the client must split the files, and each file fragment must not be larger than 1MB. Parameters Begin-Pos and Size correspond to the offset and fragment size of this fragment in the complete file. The parameter must be placed in the header of the http request, and the request body contains only the file fragment content. The request header of this interface contains more than token parameters"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"URI"}),":     /upload_file"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"REQUEST"}),":"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"header parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"}),(0,s.jsx)(n.th,{children:"necessary"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"File-Name"}),(0,s.jsx)(n.td,{children:"the name of the file"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"Begin-Pos"}),(0,s.jsx)(n.td,{children:"Offset of the start position of the current file fragment within the file"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"Size"}),(0,s.jsx)(n.td,{children:"the current file fragment size"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]})]})]}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"RESPONSE"}),":\nIf successful, the success field in the returned response message will be set to 00"]}),"\n"]}),"\n",(0,s.jsx)(n.h4,{id:"226-check-file",children:"2.2.6. Check File"}),"\n",(0,s.jsx)(n.p,{children:"this interface is used to check the correctness of uploaded files. If the check succeeds, the system returns a success message when the same file is uploaded again"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"URI"}),":     /check_file"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"REQUEST"}),":"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"}),(0,s.jsx)(n.th,{children:"necessary"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"fileName"}),(0,s.jsx)(n.td,{children:"the name of the file"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"checkSum"}),(0,s.jsx)(n.td,{children:"the checksum of the file"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:'yes when flag set to "1"'})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"fileSize"}),(0,s.jsx)(n.td,{children:"the size of the file"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:'yes when flag set to "2"'})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"flag"}),(0,s.jsx)(n.td,{children:'If flag is "1", check md5. If flag is "2"\uff0ccheck file size'}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]})]})]}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"RESPONSE"}),":\nIf successful, the success field in the returned response message will be set to 00"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"}),(0,s.jsx)(n.th,{children:"necessary"})]})}),(0,s.jsx)(n.tbody,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"pass"}),(0,s.jsx)(n.td,{children:"true on success, false otherwise"}),(0,s.jsx)(n.td,{children:"bool"}),(0,s.jsx)(n.td,{children:"yes"})]})})]}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.strong,{children:"Example request."})}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:'{"fileName" : "test.csv", "checkSum" : "$MD5", "flag" : \u201c1\u201d}\n'})}),"\n",(0,s.jsx)(n.h4,{id:"227-delete-cached-file",children:"2.2.7. Delete Cached File"}),"\n",(0,s.jsx)(n.p,{children:"The admin user can delete all the files uploaded by anyone. Other users can delete their own files. You can delete a file with a specified name, a file uploaded by a specified user, or all files"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"URI"}),":     /clear_cache"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"REQUEST"}),":"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"}),(0,s.jsx)(n.th,{children:"necessary"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"fileName"}),(0,s.jsx)(n.td,{children:"the name of the file"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:'yes when flag set to "0"'})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"userName"}),(0,s.jsx)(n.td,{children:"the name of the user"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:'yes when flag set to "1"'})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"flag"}),(0,s.jsx)(n.td,{children:"When flag is set to 0, the file specified by fileName is deleted; when flag is set to 1, all files uploaded by userName are deleted; when flag is set to 2, all files are deleted"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]})]})]}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"RESPONSE"}),":\nIf successful, the success field in the returned response message will be set to 00"]}),"\n"]}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.strong,{children:"Example request."})}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:'{"fileName" : "test.csv", "userName" : "test", "flag" : \u201c1\u201d}\n'})}),"\n",(0,s.jsx)(n.h4,{id:"228-import-schema",children:"2.2.8. Import Schema"}),"\n",(0,s.jsx)(n.p,{children:"This interface can create a schema model based on the schema information specified by description parameter. For details about the schema format, refer to data-import.md"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"URI"}),":     /import_schema"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"REQUEST"}),":"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"}),(0,s.jsx)(n.th,{children:"necessary"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"graph"}),(0,s.jsx)(n.td,{children:"name of the subgraph"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"description"}),(0,s.jsx)(n.td,{children:"schema infomation"}),(0,s.jsx)(n.td,{children:"json string"}),(0,s.jsx)(n.td,{children:"yes"})]})]})]}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"RESPONSE"}),":\nIf successful, the success field in the returned response message will be set to 00"]}),"\n"]}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.strong,{children:"Example request."})}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:'{\n\t"graph": "test_graph",\n\t"description": {\n\t\t"schema": [{\n\t\t\t"label": "Person",\n\t\t\t"type": "VERTEX",\n\t\t\t"primary": "name",\n\t\t\t"properties": [{\n\t\t\t\t"name": "name",\n\t\t\t\t"type": "STRING"\n\t\t\t}, {\n\t\t\t\t"name": "birthyear",\n\t\t\t\t"type": "INT16",\n\t\t\t\t"optional": true\n\t\t\t}, {\n\t\t\t\t"name": "phone",\n\t\t\t\t"type": "INT16",\n\t\t\t\t"unique": true,\n\t\t\t\t"index": true\n\t\t\t}]\n\t\t}, {\n\t\t\t"label": "City",\n\t\t\t"type": "VERTEX",\n\t\t\t"primary": "name",\n\t\t\t"properties": [{\n\t\t\t\t"name": "name",\n\t\t\t\t"type": "STRING"\n\t\t\t}]\n\t\t}, {\n\t\t\t"label": "Film",\n\t\t\t"type": "VERTEX",\n\t\t\t"primary": "title",\n\t\t\t"properties": [{\n\t\t\t\t"name": "title",\n\t\t\t\t"type": "STRING"\n\t\t\t}]\n\t\t}, {\n\t\t\t"label": "HAS_CHILD",\n\t\t\t"type": "EDGE"\n\t\t}, {\n\t\t\t"label": "MARRIED",\n\t\t\t"type": "EDGE"\n\t\t}, {\n\t\t\t"label": "BORN_IN",\n\t\t\t"type": "EDGE",\n\t\t\t"properties": [{\n\t\t\t\t"name": "weight",\n\t\t\t\t"type": "FLOAT",\n\t\t\t\t"optional": true\n\t\t\t}]\n\t\t}, {\n\t\t\t"label": "DIRECTED",\n\t\t\t"type": "EDGE"\n\t\t}, {\n\t\t\t"label": "WROTE_MUSIC_FOR",\n\t\t\t"type": "EDGE"\n\t\t}, {\n\t\t\t"label": "ACTED_IN",\n\t\t\t"type": "EDGE",\n\t\t\t"properties": [{\n\t\t\t\t"name": "charactername",\n\t\t\t\t"type": "STRING"\n\t\t\t}]\n\t\t}, {\n\t\t\t"label": "PLAY_IN",\n\t\t\t"type": "EDGE",\n\t\t\t"properties": [{\n\t\t\t\t"name": "role",\n\t\t\t\t"type": "STRING",\n\t\t\t\t"optional": true\n\t\t\t}],\n\t\t\t"constraints": [\n\t\t\t\t["Person", "Film"]\n\t\t\t]\n\t\t}]\n\t}\n}\n'})}),"\n",(0,s.jsx)(n.h4,{id:"229-import-data",children:"2.2.9. Import Data"}),"\n",(0,s.jsx)(n.p,{children:"This interface allows users to specify uploaded and verified files as data files and import them to the subgraph specified by the graph parameter. The import process is asynchronous, and the server returns a task id after receiving the import request"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"URI"}),":     /import_data"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"REQUEST"}),":"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"}),(0,s.jsx)(n.th,{children:"necessary"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"graph"}),(0,s.jsx)(n.td,{children:"name of the subgraph"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"schema"}),(0,s.jsx)(n.td,{children:"schema infomation"}),(0,s.jsx)(n.td,{children:"json string"}),(0,s.jsx)(n.td,{children:"yes"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"delimiter"}),(0,s.jsx)(n.td,{children:"column delimiter"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"continueOnError"}),(0,s.jsx)(n.td,{children:"Whether to skip the error and continue when an error occurs"}),(0,s.jsx)(n.td,{children:"boolean"}),(0,s.jsx)(n.td,{children:"no"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"skipPackages"}),(0,s.jsx)(n.td,{children:"number of packets skipped"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"no"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"taskId"}),(0,s.jsx)(n.td,{children:"used to restart the failed task"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"no"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"flag"}),(0,s.jsx)(n.td,{children:"If flag is set to 1, the data file is deleted after the import is successful"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"no"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"other"}),(0,s.jsx)(n.td,{children:"other parameter"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"no"})]})]})]}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"RESPONSE"}),":\nIf successful, the success field in the returned response message will be set to 00"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"}),(0,s.jsx)(n.th,{children:"necessary"})]})}),(0,s.jsx)(n.tbody,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"taskId"}),(0,s.jsx)(n.td,{children:"task id is used to find a import task"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]})})]}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.strong,{children:"Example request."})}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:'{\n   "graph": "default",         //\u5bfc\u5165\u7684\u5b50\u56fe\u540d\u79f0\n   "delimiter": ",",\t\t\t\t\t\t//\u6570\u636e\u5206\u9694\u7b26\n   "continueOnError": true,\t\t//\u9047\u5230\u9519\u8bef\u65f6\u662f\u5426\u8df3\u8fc7\u9519\u8bef\u6570\u636e\u5e76\u7ee7\u7eed\u5bfc\u5165\n   "skipPackages": \u201c0\u201d,\t\t\t\t\t//\u8df3\u8fc7\u7684\u5305\u4e2a\u6570\n   "flag" : "1",\n\t "schema": {\n\t\t"files": [{\n\t\t\t"DST_ID": "Film",\t\t\t\t//\u7ec8\u70b9label\u7c7b\u578b\n\t\t\t"SRC_ID": "Person",\t\t\t//\u8d77\u70b9label\u7c7b\u578b\n\t\t\t"columns": [\t\t\t\t\t\t//\u6570\u636e\u683c\u5f0f\u8bf4\u660e\n\t\t\t\t"SRC_ID",\t\t\t\t\t\t\t//\u8d77\u70b9id\n\t\t\t\t"DST_ID",\t\t\t\t\t\t\t//\u7ec8\u70b9id\n                "SKIP",\t\t\t\t\t\t\t\t//\u8868\u793a\u8df3\u8fc7\u6b64\u5217\u6570\u636e\n\t\t\t\t"charactername"\t\t\t\t//\u5c5e\u6027\u540d\u79f0\n\t\t\t],\n\t\t\t"format": "CSV",\t\t\t\t//\u6570\u636e\u6587\u4ef6\u683c\u5f0f\u7c7b\u578b,\u652f\u6301csv\u548cjson\n\t\t\t"path": "acted_in.csv",\t//\u6570\u636e\u6587\u4ef6\u540d\u79f0\n\t\t\t"header": 0, \t\t\t\t\t\t//\u6570\u636e\u4ece\u7b2c\u51e0\u884c\u5f00\u59cb\n\t\t\t"label": "ACTED_IN"\t\t\t//\u8fb9\u7684\u7c7b\u578b\n\t\t}]\n\t}\n}\n'})}),"\n",(0,s.jsx)(n.h4,{id:"2210-import-progress-query",children:"2.2.10. Import Progress Query"}),"\n",(0,s.jsx)(n.p,{children:"This interface is used to query the execution progress of a import task"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"URI"}),":     /import_progress"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"REQUEST"}),":"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"}),(0,s.jsx)(n.th,{children:"necessary"})]})}),(0,s.jsx)(n.tbody,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"taskId"}),(0,s.jsx)(n.td,{children:"task id returned by import_data interface"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"\u662f"})]})})]}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.strong,{children:"RESPONSE"}),":\nIf successful, the success field in the returned response message will be set to 00"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"body parameter"}),(0,s.jsx)(n.th,{children:"parameter description"}),(0,s.jsx)(n.th,{children:"parameter type"}),(0,s.jsx)(n.th,{children:"necessary"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"success"}),(0,s.jsx)(n.td,{children:"whether import is successful"}),(0,s.jsx)(n.td,{children:"boolean"}),(0,s.jsx)(n.td,{children:"yes"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"reason"}),(0,s.jsx)(n.td,{children:"reason of the import failure"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes if success is false"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"progress"}),(0,s.jsx)(n.td,{children:"import progress at the current time"}),(0,s.jsx)(n.td,{children:"string"}),(0,s.jsx)(n.td,{children:"yes"})]})]})]}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.strong,{children:"Example request."})}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:'{"taskId" : "$taskId"}\n'})})]})}function o(e={}){const{wrapper:n}={...(0,r.R)(),...e.components};return n?(0,s.jsx)(n,{...e,children:(0,s.jsx)(a,{...e})}):a(e)}}}]);