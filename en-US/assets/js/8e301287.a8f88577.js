"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[62497],{28453:(n,e,s)=>{s.d(e,{R:()=>i,x:()=>h});var r=s(96540);const d={},l=r.createContext(d);function i(n){const e=r.useContext(l);return r.useMemo((function(){return"function"==typeof n?n(e):{...e,...n}}),[e,n])}function h(n){let e;return e=n.disableParentContext?"function"==typeof n.components?n.components(d):n.components||d:i(n.components),r.createElement(l.Provider,{value:e},n.children)}},96373:(n,e,s)=>{s.r(e),s.d(e,{assets:()=>t,contentTitle:()=>i,default:()=>x,frontMatter:()=>l,metadata:()=>h,toc:()=>c});var r=s(74848),d=s(28453);const l={},i="RESTful API",h={id:"client-tools/restful-api",title:"RESTful API",description:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGrpah \u7684 Rest API \u7684\u8c03\u7528\u8be6\u60c5\u3002",source:"@site/versions/version-4.5.2/zh-CN/source/7.client-tools/7.restful-api.md",sourceDirName:"7.client-tools",slug:"/client-tools/restful-api",permalink:"/tugraph-db/en-US/zh/4.5.2/client-tools/restful-api",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:7,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"TuGraph console client",permalink:"/tugraph-db/en-US/zh/4.5.2/client-tools/bolt-console-client"},next:{title:"RPC API",permalink:"/tugraph-db/en-US/zh/4.5.2/client-tools/rpc-api"}},t={},c=[{value:"1.\u7b80\u4ecb",id:"1\u7b80\u4ecb",level:2},{value:"2.\u8bf7\u6c42\u4e0e\u54cd\u5e94\u683c\u5f0f",id:"2\u8bf7\u6c42\u4e0e\u54cd\u5e94\u683c\u5f0f",level:2},{value:"2.1.\u6807\u51c6\u54cd\u5e94\u683c\u5f0f",id:"21\u6807\u51c6\u54cd\u5e94\u683c\u5f0f",level:3},{value:"2.2\u8bf7\u6c42\u7c7b\u578b",id:"22\u8bf7\u6c42\u7c7b\u578b",level:3},{value:"2.2.1. \u7528\u6237\u767b\u9646",id:"221-\u7528\u6237\u767b\u9646",level:4},{value:"2.2.2. \u7528\u6237\u767b\u51fa",id:"222-\u7528\u6237\u767b\u51fa",level:4},{value:"2.2.3. \u8eab\u4efd\u5237\u65b0",id:"223-\u8eab\u4efd\u5237\u65b0",level:4},{value:"2.2.4. \u8c03\u7528cypher",id:"224-\u8c03\u7528cypher",level:4}];function j(n){const e={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",h4:"h4",header:"header",li:"li",p:"p",pre:"pre",strong:"strong",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",ul:"ul",...(0,d.R)(),...n.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(e.header,{children:(0,r.jsx)(e.h1,{id:"restful-api",children:"RESTful API"})}),"\n",(0,r.jsxs)(e.blockquote,{children:["\n",(0,r.jsx)(e.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGrpah \u7684 Rest API \u7684\u8c03\u7528\u8be6\u60c5\u3002"}),"\n"]}),"\n",(0,r.jsx)(e.h2,{id:"1\u7b80\u4ecb",children:"1.\u7b80\u4ecb"}),"\n",(0,r.jsx)(e.p,{children:"TuGraph \u63d0\u4f9b\u9075\u4ece REST \u89c4\u8303\u7684 HTTP API\uff0c\u4ee5\u4f9b\u5f00\u53d1\u8005\u901a\u8fc7 HTTP \u8bf7\u6c42\u8fdc\u7a0b\u8c03\u7528 TuGraph \u63d0\u4f9b\u7684\u670d\u52a1\u3002"}),"\n",(0,r.jsx)(e.p,{children:"\u672c\u6587\u6863\u63cf\u8ff0 TuGraph \u7684 HTTP API \u4f7f\u7528\u65b9\u5f0f\u3002"}),"\n",(0,r.jsx)(e.h2,{id:"2\u8bf7\u6c42\u4e0e\u54cd\u5e94\u683c\u5f0f",children:"2.\u8bf7\u6c42\u4e0e\u54cd\u5e94\u683c\u5f0f"}),"\n",(0,r.jsx)(e.p,{children:"TuGraph HTTP Server \u63a5\u6536json\u683c\u5f0f\u7684\u8bf7\u6c42\uff0c\u7ecf\u8fc7\u9274\u6743\u540e\u5f00\u59cb\u63d0\u53d6\u8bf7\u6c42\u4e2d\u7684\u5b57\u6bb5\uff0c\u6839\u636e\u5b9a\u4e49\u597d\u7684\u63a5\u53e3\u903b\u8f91\u5904\u7406\u8bf7\u6c42\uff0c\u5e76\u8fd4\u56dejson\u683c\u5f0f\u7684\u54cd\u5e94\u3002"}),"\n",(0,r.jsx)(e.h3,{id:"21\u6807\u51c6\u54cd\u5e94\u683c\u5f0f",children:"2.1.\u6807\u51c6\u54cd\u5e94\u683c\u5f0f"}),"\n",(0,r.jsx)(e.p,{children:"\u6bcf\u4e00\u4e2a\u54cd\u5e94\u90fd\u4ee5\u6807\u51c6\u54cd\u5e94\u683c\u5f0f\u8fd4\u56de\uff0c\u683c\u5f0f\u5982\u4e0b\uff1a"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(e.table,{children:[(0,r.jsx)(e.thead,{children:(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.th,{children:"body\u53c2\u6570"}),(0,r.jsx)(e.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,r.jsx)(e.th,{children:"\u53c2\u6570\u7c7b\u578b"}),(0,r.jsx)(e.th,{children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,r.jsxs)(e.tbody,{children:[(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"errorCode"}),(0,r.jsx)(e.td,{children:"\u4e1a\u52a1\u7ea7\u9519\u8bef\u7801"}),(0,r.jsx)(e.td,{children:"int\u7c7b\u578b"}),(0,r.jsx)(e.td,{children:"\u662f"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"success"}),(0,r.jsx)(e.td,{children:"\u8bf7\u6c42\u662f\u5426\u6210\u529f"}),(0,r.jsx)(e.td,{children:"int\u7c7b\u578b"}),(0,r.jsx)(e.td,{children:"\u662f"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"errorMessage"}),(0,r.jsx)(e.td,{children:"\u4e1a\u52a1\u7ea7\u9519\u8bef\u4fe1\u606f"}),(0,r.jsx)(e.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,r.jsx)(e.td,{children:"\u662f"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"data"}),(0,r.jsx)(e.td,{children:"\u8bf7\u6c42\u6210\u529f\u65f6\u8fd4\u56de\u7684\u54cd\u5e94\u4fe1\u606f"}),(0,r.jsx)(e.td,{children:"json\u5b57\u7b26\u4e32"}),(0,r.jsx)(e.td,{children:"\u662f"})]})]})]}),"\n",(0,r.jsx)(e.h3,{id:"22\u8bf7\u6c42\u7c7b\u578b",children:"2.2\u8bf7\u6c42\u7c7b\u578b"}),"\n",(0,r.jsx)(e.h4,{id:"221-\u7528\u6237\u767b\u9646",children:"2.2.1. \u7528\u6237\u767b\u9646"}),"\n",(0,r.jsx)(e.p,{children:"\u7528\u6237\u5728\u767b\u9646\u8bf7\u6c42\u4e2d\u643a\u5e26\u7528\u6237\u540d\u548c\u5bc6\u7801\u53d1\u9001\u5230\u670d\u52a1\u7aef\u3002\u767b\u5f55\u6210\u529f\u4f1a\u6536\u5230\u5e26\u6709\u7b7e\u540d\u7684\u4ee4\u724c(Json Web Token)\u548c\u5224\u65ad\u662f\u5426\u4e3a\u9ed8\u8ba4\u5bc6\u7801\u7684\u5e03\u5c14\u578b\u53d8\u91cf\uff0c\u5ba2\u6237\u7aef\u50a8\u5b58\u8be5\u4ee4\u724c\uff0c\u5728\u540e\u7eed\u7684\u8bf7\u6c42\u4e2d\u5c06\u4ee4\u724c\u52a0\u5165\u8bf7\u6c42\u5934\u7684Authorization\u57df\u4e2d\u3002\u5982\u679c\u767b\u5f55\u5931\u8d25\u4f1a\u6536\u5230\u201cAuthentication failed\u201d\u9519\u8bef\u3002"}),"\n",(0,r.jsxs)(e.ul,{children:["\n",(0,r.jsxs)(e.li,{children:[(0,r.jsx)(e.strong,{children:"URI"}),":     /login"]}),"\n",(0,r.jsxs)(e.li,{children:[(0,r.jsx)(e.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,r.jsxs)(e.li,{children:[(0,r.jsx)(e.strong,{children:"REQUEST"}),":"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(e.table,{children:[(0,r.jsx)(e.thead,{children:(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.th,{children:"body\u53c2\u6570"}),(0,r.jsx)(e.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,r.jsx)(e.th,{children:"\u53c2\u6570\u7c7b\u578b"}),(0,r.jsx)(e.th,{children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,r.jsxs)(e.tbody,{children:[(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"user"}),(0,r.jsx)(e.td,{children:"\u7528\u6237\u540d"}),(0,r.jsx)(e.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,r.jsx)(e.td,{children:"\u662f"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"password"}),(0,r.jsx)(e.td,{children:"\u7528\u6237\u5bc6\u7801"}),(0,r.jsx)(e.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,r.jsx)(e.td,{children:"\u662f"})]})]})]}),"\n",(0,r.jsxs)(e.ul,{children:["\n",(0,r.jsxs)(e.li,{children:[(0,r.jsx)(e.strong,{children:"RESPONSE"}),":\n\u5982\u679c\u6210\u529f\uff0c\u8fd4\u56de\u7684\u54cd\u5e94\u4fe1\u606f\u4e2dsuccess\u4e3a00\uff0cdata\u4e2d\u5305\u542b\u4ee4\u724c"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(e.table,{children:[(0,r.jsx)(e.thead,{children:(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.th,{children:"body\u53c2\u6570"}),(0,r.jsx)(e.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,r.jsx)(e.th,{children:"\u53c2\u6570\u7c7b\u578b"})]})}),(0,r.jsxs)(e.tbody,{children:[(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"jwt"}),(0,r.jsx)(e.td,{children:"\u4ee4\u724c"}),(0,r.jsx)(e.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"is_admin"}),(0,r.jsx)(e.td,{children:"\u662f\u5426\u662fadmin\u7528\u6237"}),(0,r.jsx)(e.td,{children:"\u5e03\u5c14\u7c7b\u578b"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"default_password"}),(0,r.jsx)(e.td,{children:"\u9ed8\u8ba4\u5bc6\u7801"}),(0,r.jsx)(e.td,{children:"\u5e03\u5c14\u7c7b\u578b"})]})]})]}),"\n",(0,r.jsx)(e.p,{children:(0,r.jsx)(e.strong,{children:"Example request."})}),"\n",(0,r.jsx)(e.pre,{children:(0,r.jsx)(e.code,{children:'    {"user" : "admin", "password" : "73@TuGraph"}\n'})}),"\n",(0,r.jsx)(e.h4,{id:"222-\u7528\u6237\u767b\u51fa",children:"2.2.2. \u7528\u6237\u767b\u51fa"}),"\n",(0,r.jsx)(e.p,{children:"\u7528\u6237\u767b\u51fa\uff0c\u540c\u65f6\u5220\u9664\u5df2\u7ecf\u8ba4\u8bc1\u7684token\uff0c\u7528\u6237\u540e\u7eed\u53d1\u9001\u8bf7\u6c42\u65f6\uff0c\u9700\u8981\u91cd\u65b0\u767b\u9646\uff0c\u5e76\u83b7\u53d6\u65b0\u7684token\u3002"}),"\n",(0,r.jsxs)(e.ul,{children:["\n",(0,r.jsxs)(e.li,{children:[(0,r.jsx)(e.strong,{children:"URI"}),":     /logout"]}),"\n",(0,r.jsxs)(e.li,{children:[(0,r.jsx)(e.strong,{children:"METHOD"}),":  POST\n",(0,r.jsxs)(e.ul,{children:["\n",(0,r.jsxs)(e.li,{children:[(0,r.jsx)(e.strong,{children:"REQUEST"}),":\nhttp request header\u4e2d\u643a\u5e26\u8c03\u7528login\u63a5\u53e3\u65f6\u8fd4\u56de\u7684token\uff0c\u5177\u4f53\u586b\u5199\u7684\u5b57\u7b26\u4e32\u662f",(0,r.jsx)(e.code,{children:"Bearer ${jwt}"}),"\uff0c",(0,r.jsx)(e.code,{children:"${jwt}"}),"\u5c31\u662flogin\u63a5\u53e3\u8fd4\u56de\u7684jwt\uff0cbody\u4e2d\u6ca1\u6709\u53c2\u6570"]}),"\n"]}),"\n"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(e.table,{children:[(0,r.jsx)(e.thead,{children:(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.th,{children:"header\u53c2\u6570"}),(0,r.jsx)(e.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,r.jsx)(e.th,{children:"\u53c2\u6570\u7c7b\u578b"})]})}),(0,r.jsx)(e.tbody,{children:(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"Authorization"}),(0,r.jsx)(e.td,{children:"Bearer ${jwt}"}),(0,r.jsx)(e.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"})]})})]}),"\n",(0,r.jsxs)(e.ul,{children:["\n",(0,r.jsxs)(e.li,{children:[(0,r.jsx)(e.strong,{children:"RESPONSE"}),":\n\u5982\u679c\u6210\u529f\uff0c\u8fd4\u56de\u7684\u54cd\u5e94\u4fe1\u606f\u4e2dsuccess\u4e3a00\uff0cdata\u4e2d\u5305\u542b"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(e.table,{children:[(0,r.jsx)(e.thead,{children:(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.th,{children:"body\u53c2\u6570"}),(0,r.jsx)(e.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,r.jsx)(e.th,{children:"\u53c2\u6570\u7c7b\u578b"})]})}),(0,r.jsx)(e.tbody,{children:(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"is_admin"}),(0,r.jsx)(e.td,{children:"\u662f\u5426\u662fadmin\u7528\u6237"}),(0,r.jsx)(e.td,{children:"\u5e03\u5c14\u7c7b\u578b"})]})})]}),"\n",(0,r.jsx)(e.h4,{id:"223-\u8eab\u4efd\u5237\u65b0",children:"2.2.3. \u8eab\u4efd\u5237\u65b0"}),"\n",(0,r.jsx)(e.p,{children:"\u5df2\u4e0b\u53d1\u7684token\u5931\u6548\u540e\uff0c\u9700\u8981\u8c03\u7528\u672c\u63a5\u53e3\u91cd\u65b0\u8ba4\u8bc1\u3002\u540e\u7aef\u9a8c\u8bc1token\u5408\u6cd5\u6027\u3002token\u5728\u521d\u6b21\u767b\u5f55\u540e\uff0c1\u5c0f\u65f6\u5185\u6709\u6548\uff0c\u8fc7\u671f\u9700\u8981\u5237\u65b0"}),"\n",(0,r.jsxs)(e.ul,{children:["\n",(0,r.jsxs)(e.li,{children:[(0,r.jsx)(e.strong,{children:"URI"}),":     /refresh"]}),"\n",(0,r.jsxs)(e.li,{children:[(0,r.jsx)(e.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,r.jsxs)(e.li,{children:[(0,r.jsx)(e.strong,{children:"REQUEST"}),":\nhttp request header\u4e2d\u643a\u5e26\u8c03\u7528login\u63a5\u53e3\u8fd4\u56de\u7684token\uff0c\u5177\u4f53\u586b\u5199\u7684\u5b57\u7b26\u4e32\u662f",(0,r.jsx)(e.code,{children:"Bearer ${jwt}"}),"\uff0c",(0,r.jsx)(e.code,{children:"${jwt}"}),"\u5c31\u662flogin\u63a5\u53e3\u8fd4\u56de\u7684jwt\uff0cbody\u4e2d\u6ca1\u6709\u53c2\u6570"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(e.table,{children:[(0,r.jsx)(e.thead,{children:(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.th,{children:"header\u53c2\u6570"}),(0,r.jsx)(e.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,r.jsx)(e.th,{children:"\u53c2\u6570\u7c7b\u578b"})]})}),(0,r.jsx)(e.tbody,{children:(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"Authorization"}),(0,r.jsx)(e.td,{children:"Bearer ${jwt}"}),(0,r.jsx)(e.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"})]})})]}),"\n",(0,r.jsxs)(e.ul,{children:["\n",(0,r.jsxs)(e.li,{children:[(0,r.jsx)(e.strong,{children:"RESPONSE"}),":\n\u5982\u679c\u6210\u529f\uff0c\u8fd4\u56de\u7684\u54cd\u5e94\u4fe1\u606f\u4e2dsuccess\u4e3a00\uff0cdata\u4e2d\u5305\u542b\u4ee4\u724c"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(e.table,{children:[(0,r.jsx)(e.thead,{children:(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.th,{children:"body\u53c2\u6570"}),(0,r.jsx)(e.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,r.jsx)(e.th,{children:"\u53c2\u6570\u7c7b\u578b"})]})}),(0,r.jsxs)(e.tbody,{children:[(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"jwt"}),(0,r.jsx)(e.td,{children:"\u4ee4\u724c"}),(0,r.jsx)(e.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"is_admin"}),(0,r.jsx)(e.td,{children:"\u662f\u5426\u662fadmin\u7528\u6237"}),(0,r.jsx)(e.td,{children:"\u5e03\u5c14\u7c7b\u578b"})]})]})]}),"\n",(0,r.jsx)(e.h4,{id:"224-\u8c03\u7528cypher",children:"2.2.4. \u8c03\u7528cypher"}),"\n",(0,r.jsx)(e.p,{children:"\u7528\u6237\u5bf9TuGraph\u7684\u589e\u5220\u6539\u67e5\u8bf7\u6c42\u9700\u8981\u8c03\u7528cypher\u63a5\u53e3\uff0c\u5e76\u901a\u8fc7\u6807\u51c6\u7684cypher\u67e5\u8be2\u8bed\u8a00\u53d1\u8d77"}),"\n",(0,r.jsxs)(e.ul,{children:["\n",(0,r.jsxs)(e.li,{children:[(0,r.jsx)(e.strong,{children:"URI"}),":     /cypher"]}),"\n",(0,r.jsxs)(e.li,{children:[(0,r.jsx)(e.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,r.jsxs)(e.li,{children:[(0,r.jsx)(e.strong,{children:"REQUEST"}),":"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(e.table,{children:[(0,r.jsx)(e.thead,{children:(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.th,{children:"header\u53c2\u6570"}),(0,r.jsx)(e.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,r.jsx)(e.th,{children:"\u53c2\u6570\u7c7b\u578b"})]})}),(0,r.jsx)(e.tbody,{children:(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"Authorization"}),(0,r.jsx)(e.td,{children:"Bearer ${jwt}"}),(0,r.jsx)(e.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"})]})})]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(e.table,{children:[(0,r.jsx)(e.thead,{children:(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.th,{children:"body\u53c2\u6570"}),(0,r.jsx)(e.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,r.jsx)(e.th,{children:"\u53c2\u6570\u7c7b\u578b"}),(0,r.jsx)(e.th,{children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,r.jsxs)(e.tbody,{children:[(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"graph"}),(0,r.jsx)(e.td,{children:"\u67e5\u8be2\u7684\u5b50\u56fe\u540d\u79f0"}),(0,r.jsx)(e.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,r.jsx)(e.td,{children:"\u662f"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"script"}),(0,r.jsx)(e.td,{children:"cypher\u8bed\u53e5"}),(0,r.jsx)(e.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,r.jsx)(e.td,{children:"\u662f"})]})]})]}),"\n",(0,r.jsxs)(e.ul,{children:["\n",(0,r.jsxs)(e.li,{children:[(0,r.jsx)(e.strong,{children:"RESPONSE"}),":\n\u5982\u679c\u6210\u529f\uff0c\u8fd4\u56de\u7684\u54cd\u5e94\u4fe1\u606f\u4e2dsuccess\u4e3a00\uff0cdata\u4e2d\u5305\u542b\u67e5\u8be2\u7ed3\u679c"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(e.table,{children:[(0,r.jsx)(e.thead,{children:(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.th,{children:"body\u53c2\u6570"}),(0,r.jsx)(e.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,r.jsx)(e.th,{children:"\u53c2\u6570\u7c7b\u578b"}),(0,r.jsx)(e.th,{children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,r.jsx)(e.tbody,{children:(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"result"}),(0,r.jsx)(e.td,{children:"\u67e5\u8be2\u7ed3\u679c"}),(0,r.jsx)(e.td,{children:"json\u5b57\u7b26\u4e32"}),(0,r.jsx)(e.td,{children:"\u662f"})]})})]}),"\n",(0,r.jsx)(e.p,{children:(0,r.jsx)(e.strong,{children:"Example request."})}),"\n",(0,r.jsx)(e.pre,{children:(0,r.jsx)(e.code,{children:'    {"script" : "Match (n) return n", "graph" : "default"}\n'})})]})}function x(n={}){const{wrapper:e}={...(0,d.R)(),...n.components};return e?(0,r.jsx)(e,{...n,children:(0,r.jsx)(j,{...n})}):j(n)}}}]);