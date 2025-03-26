"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[46374],{28453:(n,t,e)=>{e.d(t,{R:()=>l,x:()=>i});var s=e(96540);const r={},d=s.createContext(r);function l(n){const t=s.useContext(d);return s.useMemo((function(){return"function"==typeof n?n(t):{...t,...n}}),[t,n])}function i(n){let t;return t=n.disableParentContext?"function"==typeof n.components?n.components(r):n.components||r:l(n.components),s.createElement(d.Provider,{value:t},n.children)}},72911:(n,t,e)=>{e.r(t),e.d(t,{assets:()=>h,contentTitle:()=>l,default:()=>j,frontMatter:()=>d,metadata:()=>i,toc:()=>c});var s=e(74848),r=e(28453);const d={},l="RESTful API",i={id:"developer-manual/interface/protocol/restful-api",title:"RESTful API",description:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGrpah \u7684 Rest API \u7684\u8c03\u7528\u8be6\u60c5\u3002",source:"@site/versions/version-4.1.0/zh-CN/source/5.developer-manual/6.interface/4.protocol/3.restful-api.md",sourceDirName:"5.developer-manual/6.interface/4.protocol",slug:"/developer-manual/interface/protocol/restful-api",permalink:"/tugraph-db/en-US/zh/4.1.0/developer-manual/interface/protocol/restful-api",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:3,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"RPC API",permalink:"/tugraph-db/en-US/zh/4.1.0/developer-manual/interface/protocol/rpc-api"},next:{title:"Learn Tutorial",permalink:"/tugraph-db/en-US/zh/4.1.0/developer-manual/interface/learn/tutorial"}},h={},c=[{value:"1.\u7b80\u4ecb",id:"1\u7b80\u4ecb",level:2},{value:"2.\u8bf7\u6c42\u4e0e\u54cd\u5e94\u683c\u5f0f",id:"2\u8bf7\u6c42\u4e0e\u54cd\u5e94\u683c\u5f0f",level:2},{value:"2.1.\u6807\u51c6\u54cd\u5e94\u683c\u5f0f",id:"21\u6807\u51c6\u54cd\u5e94\u683c\u5f0f",level:3},{value:"2.2\u8bf7\u6c42\u7c7b\u578b",id:"22\u8bf7\u6c42\u7c7b\u578b",level:3},{value:"2.2.1. \u7528\u6237\u767b\u9646",id:"221-\u7528\u6237\u767b\u9646",level:4},{value:"2.2.2. \u7528\u6237\u767b\u51fa",id:"222-\u7528\u6237\u767b\u51fa",level:4},{value:"2.2.3. \u8eab\u4efd\u5237\u65b0",id:"223-\u8eab\u4efd\u5237\u65b0",level:4},{value:"2.2.4. \u8c03\u7528cypher",id:"224-\u8c03\u7528cypher",level:4},{value:"2.2.5. \u4e0a\u4f20\u6587\u4ef6",id:"225-\u4e0a\u4f20\u6587\u4ef6",level:4},{value:"2.2.6. \u68c0\u67e5\u6587\u4ef6",id:"226-\u68c0\u67e5\u6587\u4ef6",level:4},{value:"2.2.7. \u6e05\u7406\u7f13\u5b58\u6587\u4ef6",id:"227-\u6e05\u7406\u7f13\u5b58\u6587\u4ef6",level:4},{value:"2.2.8. \u5bfc\u5165schema",id:"228-\u5bfc\u5165schema",level:4},{value:"2.2.9. \u5bfc\u5165\u6570\u636e",id:"229-\u5bfc\u5165\u6570\u636e",level:4},{value:"2.2.10. \u5bfc\u5165\u8fdb\u5ea6\u67e5\u8be2",id:"2210-\u5bfc\u5165\u8fdb\u5ea6\u67e5\u8be2",level:4}];function x(n){const t={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",h4:"h4",header:"header",li:"li",p:"p",pre:"pre",strong:"strong",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",ul:"ul",...(0,r.R)(),...n.components};return(0,s.jsxs)(s.Fragment,{children:[(0,s.jsx)(t.header,{children:(0,s.jsx)(t.h1,{id:"restful-api",children:"RESTful API"})}),"\n",(0,s.jsxs)(t.blockquote,{children:["\n",(0,s.jsx)(t.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGrpah \u7684 Rest API \u7684\u8c03\u7528\u8be6\u60c5\u3002"}),"\n"]}),"\n",(0,s.jsx)(t.h2,{id:"1\u7b80\u4ecb",children:"1.\u7b80\u4ecb"}),"\n",(0,s.jsx)(t.p,{children:"TuGraph \u63d0\u4f9b\u9075\u4ece REST \u89c4\u8303\u7684 HTTP API\uff0c\u4ee5\u4f9b\u5f00\u53d1\u8005\u901a\u8fc7 HTTP \u8bf7\u6c42\u8fdc\u7a0b\u8c03\u7528 TuGraph \u63d0\u4f9b\u7684\u670d\u52a1\u3002"}),"\n",(0,s.jsx)(t.p,{children:"\u672c\u6587\u6863\u63cf\u8ff0 TuGraph \u7684 HTTP API \u4f7f\u7528\u65b9\u5f0f\u3002"}),"\n",(0,s.jsx)(t.h2,{id:"2\u8bf7\u6c42\u4e0e\u54cd\u5e94\u683c\u5f0f",children:"2.\u8bf7\u6c42\u4e0e\u54cd\u5e94\u683c\u5f0f"}),"\n",(0,s.jsx)(t.p,{children:"TuGraph HTTP Server \u63a5\u6536json\u683c\u5f0f\u7684\u8bf7\u6c42\uff0c\u7ecf\u8fc7\u9274\u6743\u540e\u5f00\u59cb\u63d0\u53d6\u8bf7\u6c42\u4e2d\u7684\u5b57\u6bb5\uff0c\u6839\u636e\u5b9a\u4e49\u597d\u7684\u63a5\u53e3\u903b\u8f91\u5904\u7406\u8bf7\u6c42\uff0c\u5e76\u8fd4\u56dejson\u683c\u5f0f\u7684\u54cd\u5e94\u3002"}),"\n",(0,s.jsx)(t.h3,{id:"21\u6807\u51c6\u54cd\u5e94\u683c\u5f0f",children:"2.1.\u6807\u51c6\u54cd\u5e94\u683c\u5f0f"}),"\n",(0,s.jsx)(t.p,{children:"\u6bcf\u4e00\u4e2a\u54cd\u5e94\u90fd\u4ee5\u6807\u51c6\u54cd\u5e94\u683c\u5f0f\u8fd4\u56de\uff0c\u683c\u5f0f\u5982\u4e0b\uff1a"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(t.table,{children:[(0,s.jsx)(t.thead,{children:(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.th,{children:"body\u53c2\u6570"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u7c7b\u578b"}),(0,s.jsx)(t.th,{children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,s.jsxs)(t.tbody,{children:[(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"errorCode"}),(0,s.jsx)(t.td,{children:"\u4e1a\u52a1\u7ea7\u9519\u8bef\u7801"}),(0,s.jsx)(t.td,{children:"int\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"\u662f"})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"success"}),(0,s.jsx)(t.td,{children:"\u8bf7\u6c42\u662f\u5426\u6210\u529f"}),(0,s.jsx)(t.td,{children:"int\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"\u662f"})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"errorMessage"}),(0,s.jsx)(t.td,{children:"\u4e1a\u52a1\u7ea7\u9519\u8bef\u4fe1\u606f"}),(0,s.jsx)(t.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"\u662f"})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"data"}),(0,s.jsx)(t.td,{children:"\u8bf7\u6c42\u6210\u529f\u65f6\u8fd4\u56de\u7684\u54cd\u5e94\u4fe1\u606f"}),(0,s.jsx)(t.td,{children:"json\u5b57\u7b26\u4e32"}),(0,s.jsx)(t.td,{children:"\u662f"})]})]})]}),"\n",(0,s.jsx)(t.h3,{id:"22\u8bf7\u6c42\u7c7b\u578b",children:"2.2\u8bf7\u6c42\u7c7b\u578b"}),"\n",(0,s.jsx)(t.h4,{id:"221-\u7528\u6237\u767b\u9646",children:"2.2.1. \u7528\u6237\u767b\u9646"}),"\n",(0,s.jsx)(t.p,{children:"\u7528\u6237\u5728\u767b\u9646\u8bf7\u6c42\u4e2d\u643a\u5e26\u7528\u6237\u540d\u548c\u5bc6\u7801\u53d1\u9001\u5230\u670d\u52a1\u7aef\u3002\u767b\u5f55\u6210\u529f\u4f1a\u6536\u5230\u5e26\u6709\u7b7e\u540d\u7684\u4ee4\u724c(Json Web Token)\u548c\u5224\u65ad\u662f\u5426\u4e3a\u9ed8\u8ba4\u5bc6\u7801\u7684\u5e03\u5c14\u578b\u53d8\u91cf\uff0c\u5ba2\u6237\u7aef\u50a8\u5b58\u8be5\u4ee4\u724c\uff0c\u5728\u540e\u7eed\u7684\u8bf7\u6c42\u4e2d\u5c06\u4ee4\u724c\u52a0\u5165\u8bf7\u6c42\u5934\u7684Authorization\u57df\u4e2d\u3002\u5982\u679c\u767b\u5f55\u5931\u8d25\u4f1a\u6536\u5230\u201cAuthentication failed\u201d\u9519\u8bef\u3002"}),"\n",(0,s.jsxs)(t.ul,{children:["\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"URI"}),":     /login"]}),"\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"REQUEST"}),":"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(t.table,{children:[(0,s.jsx)(t.thead,{children:(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.th,{children:"body\u53c2\u6570"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u7c7b\u578b"}),(0,s.jsx)(t.th,{children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,s.jsxs)(t.tbody,{children:[(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"userName"}),(0,s.jsx)(t.td,{children:"\u7528\u6237\u540d"}),(0,s.jsx)(t.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"\u662f"})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"password"}),(0,s.jsx)(t.td,{children:"\u7528\u6237\u5bc6\u7801"}),(0,s.jsx)(t.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"\u662f"})]})]})]}),"\n",(0,s.jsxs)(t.ul,{children:["\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"RESPONSE"}),":\n\u5982\u679c\u6210\u529f\uff0c\u8fd4\u56de\u7684\u54cd\u5e94\u4fe1\u606f\u4e2dsuccess\u4e3a00\uff0cdata\u4e2d\u5305\u542b\u4ee4\u724c"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(t.table,{children:[(0,s.jsx)(t.thead,{children:(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.th,{children:"body\u53c2\u6570"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u7c7b\u578b"}),(0,s.jsx)(t.th,{children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,s.jsxs)(t.tbody,{children:[(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"authorization"}),(0,s.jsx)(t.td,{children:"\u4ee4\u724c"}),(0,s.jsx)(t.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"\u662f"})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"default_password"}),(0,s.jsx)(t.td,{children:"\u9ed8\u8ba4\u5bc6\u7801"}),(0,s.jsx)(t.td,{children:"\u5e03\u5c14\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"\u662f"})]})]})]}),"\n",(0,s.jsx)(t.p,{children:(0,s.jsx)(t.strong,{children:"Example request."})}),"\n",(0,s.jsx)(t.pre,{children:(0,s.jsx)(t.code,{children:'    {"userName" : "test", "password" : "123456"}\n'})}),"\n",(0,s.jsx)(t.h4,{id:"222-\u7528\u6237\u767b\u51fa",children:"2.2.2. \u7528\u6237\u767b\u51fa"}),"\n",(0,s.jsx)(t.p,{children:"\u7528\u6237\u767b\u51fa\uff0c\u540c\u65f6\u5220\u9664\u5df2\u7ecf\u8ba4\u8bc1\u7684token\uff0c\u7528\u6237\u540e\u7eed\u53d1\u9001\u8bf7\u6c42\u65f6\uff0c\u9700\u8981\u91cd\u65b0\u767b\u9646\uff0c\u5e76\u83b7\u53d6\u65b0\u7684token\u3002"}),"\n",(0,s.jsxs)(t.ul,{children:["\n",(0,s.jsxs)(t.li,{children:["\n",(0,s.jsxs)(t.p,{children:[(0,s.jsx)(t.strong,{children:"URI"}),":     /logout"]}),"\n"]}),"\n",(0,s.jsxs)(t.li,{children:["\n",(0,s.jsxs)(t.p,{children:[(0,s.jsx)(t.strong,{children:"METHOD"}),":  POST"]}),"\n"]}),"\n",(0,s.jsxs)(t.li,{children:["\n",(0,s.jsxs)(t.p,{children:[(0,s.jsx)(t.strong,{children:"REQUEST"}),":\nhttp request header\u4e2d\u643a\u5e26\u8c03\u7528login\u63a5\u53e3\u65f6\u8fd4\u56de\u7684token\uff0cbody\u4e2d\u6ca1\u6709\u53c2\u6570"]}),"\n"]}),"\n",(0,s.jsxs)(t.li,{children:["\n",(0,s.jsxs)(t.p,{children:[(0,s.jsx)(t.strong,{children:"RESPONSE"}),":\n\u5982\u679c\u6210\u529f\uff0c\u8fd4\u56de\u7684\u54cd\u5e94\u4fe1\u606f\u4e2dsuccess\u4e3a00\uff0cdata\u4e3a\u7a7a"]}),"\n"]}),"\n"]}),"\n",(0,s.jsx)(t.h4,{id:"223-\u8eab\u4efd\u5237\u65b0",children:"2.2.3. \u8eab\u4efd\u5237\u65b0"}),"\n",(0,s.jsx)(t.p,{children:"\u5df2\u4e0b\u53d1\u7684token\u5931\u6548\u540e\uff0c\u9700\u8981\u8c03\u7528\u672c\u63a5\u53e3\u91cd\u65b0\u8ba4\u8bc1\u3002\u540e\u7aef\u9a8c\u8bc1token\u5408\u6cd5\u6027\u3002token\u5728\u521d\u6b21\u767b\u5f55\u540e\uff0c1\u5c0f\u65f6\u5185\u6709\u6548\uff0c\u8fc7\u671f\u9700\u8981\u5237\u65b0"}),"\n",(0,s.jsxs)(t.ul,{children:["\n",(0,s.jsxs)(t.li,{children:["\n",(0,s.jsxs)(t.p,{children:[(0,s.jsx)(t.strong,{children:"URI"}),":     /refresh"]}),"\n"]}),"\n",(0,s.jsxs)(t.li,{children:["\n",(0,s.jsxs)(t.p,{children:[(0,s.jsx)(t.strong,{children:"METHOD"}),":  POST"]}),"\n"]}),"\n",(0,s.jsxs)(t.li,{children:["\n",(0,s.jsxs)(t.p,{children:[(0,s.jsx)(t.strong,{children:"REQUEST"}),":\nhttp request header\u4e2d\u643a\u5e26\u8c03\u7528login\u63a5\u53e3\u65f6\u8fd4\u56de\u7684token\uff0cbody\u4e2d\u6ca1\u6709\u53c2\u6570"]}),"\n"]}),"\n",(0,s.jsxs)(t.li,{children:["\n",(0,s.jsxs)(t.p,{children:[(0,s.jsx)(t.strong,{children:"RESPONSE"}),":\n\u5982\u679c\u6210\u529f\uff0c\u8fd4\u56de\u7684\u54cd\u5e94\u4fe1\u606f\u4e2dsuccess\u4e3a00\uff0cdata\u4e2d\u5305\u542b\u4ee4\u724c"]}),"\n"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(t.table,{children:[(0,s.jsx)(t.thead,{children:(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.th,{children:"body\u53c2\u6570"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u7c7b\u578b"}),(0,s.jsx)(t.th,{children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,s.jsx)(t.tbody,{children:(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"authorization"}),(0,s.jsx)(t.td,{children:"\u4ee4\u724c"}),(0,s.jsx)(t.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"\u662f"})]})})]}),"\n",(0,s.jsx)(t.h4,{id:"224-\u8c03\u7528cypher",children:"2.2.4. \u8c03\u7528cypher"}),"\n",(0,s.jsx)(t.p,{children:"\u7528\u6237\u5bf9TuGraph\u7684\u589e\u5220\u6539\u67e5\u8bf7\u6c42\u9700\u8981\u8c03\u7528cypher\u63a5\u53e3\uff0c\u5e76\u901a\u8fc7\u6807\u51c6\u7684cypher\u67e5\u8be2\u8bed\u8a00\u53d1\u8d77"}),"\n",(0,s.jsxs)(t.ul,{children:["\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"URI"}),":     /cypher"]}),"\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"REQUEST"}),":"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(t.table,{children:[(0,s.jsx)(t.thead,{children:(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.th,{children:"body\u53c2\u6570"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u7c7b\u578b"}),(0,s.jsx)(t.th,{children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,s.jsxs)(t.tbody,{children:[(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"graph"}),(0,s.jsx)(t.td,{children:"\u67e5\u8be2\u7684\u5b50\u56fe\u540d\u79f0"}),(0,s.jsx)(t.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"\u662f"})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"script"}),(0,s.jsx)(t.td,{children:"cypher\u8bed\u53e5"}),(0,s.jsx)(t.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"\u662f"})]})]})]}),"\n",(0,s.jsxs)(t.ul,{children:["\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"RESPONSE"}),":\n\u5982\u679c\u6210\u529f\uff0c\u8fd4\u56de\u7684\u54cd\u5e94\u4fe1\u606f\u4e2dsuccess\u4e3a00\uff0cdata\u4e2d\u5305\u542b\u67e5\u8be2\u7ed3\u679c"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(t.table,{children:[(0,s.jsx)(t.thead,{children:(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.th,{children:"body\u53c2\u6570"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u7c7b\u578b"}),(0,s.jsx)(t.th,{children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,s.jsx)(t.tbody,{children:(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"result"}),(0,s.jsx)(t.td,{children:"\u67e5\u8be2\u7ed3\u679c"}),(0,s.jsx)(t.td,{children:"json\u5b57\u7b26\u4e32"}),(0,s.jsx)(t.td,{children:"\u662f"})]})})]}),"\n",(0,s.jsx)(t.p,{children:(0,s.jsx)(t.strong,{children:"Example request."})}),"\n",(0,s.jsx)(t.pre,{children:(0,s.jsx)(t.code,{children:'    {"script" : "Match (n) return n", "graph" : "default"}\n'})}),"\n",(0,s.jsx)(t.h4,{id:"225-\u4e0a\u4f20\u6587\u4ef6",children:"2.2.5. \u4e0a\u4f20\u6587\u4ef6"}),"\n",(0,s.jsx)(t.p,{children:"\u63a5\u53e3\u7528\u4e8e\u5c06\u672c\u5730\u6587\u4ef6\u4e0a\u4f20\u81f3TuGraph\u6240\u5728\u673a\u5668\u3002\u53ef\u4ee5\u4e0a\u4f20\u6587\u672c\u6587\u4ef6\uff0c\u4e8c\u8fdb\u5236\u6587\u4ef6\uff0c\u53ef\u4ee5\u4e0a\u4f20\u5927\u6587\u4ef6\uff0c\u4e5f\u53ef\u4ee5\u4e0a\u4f20\u5c0f\u6587\u4ef6\uff0c\u5bf9\u4e8e\u5927\u6587\u4ef6\uff0c\u5ba2\u6237\u7aef\u5728\u4e0a\u4f20\u65f6\u5e94\u8be5\u5bf9\u6587\u4ef6\u505a\u5207\u5206\uff0c\u6bcf\u4e2a\u6587\u4ef6\u5206\u7247\u4e0d\u5927\u4e8e1MB\uff0c\u53c2\u6570Begin-Pos\u548cSize\u5bf9\u5e94\u672c\u6b21\u5206\u7247\u5728\u5b8c\u6574\u6587\u4ef6\u4e2d\u7684\u504f\u79fb\u91cf\u4e0e\u5206\u7247\u5927\u5c0f\u3002\u53c2\u6570\u9700\u8981\u653e\u5728http\u8bf7\u6c42\u7684\u62a5\u6587\u5934\uff0c\u8bf7\u6c42\u5185\u5bb9\u5bf9\u5e94\u6587\u4ef6\u5206\u7247\u5185\u5bb9\u3002\u672c\u63a5\u53e3\u8bf7\u6c42\u5934\u4e0d\u6b62\u6709token\u53c2\u6570"}),"\n",(0,s.jsxs)(t.ul,{children:["\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"URI"}),":     /upload_file"]}),"\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"REQUEST"}),":"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(t.table,{children:[(0,s.jsx)(t.thead,{children:(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.th,{children:"header\u53c2\u6570"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u7c7b\u578b"}),(0,s.jsx)(t.th,{children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,s.jsxs)(t.tbody,{children:[(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"File-Name"}),(0,s.jsx)(t.td,{children:"\u6587\u4ef6\u540d\u79f0"}),(0,s.jsx)(t.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"\u662f"})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"Begin-Pos"}),(0,s.jsx)(t.td,{children:"\u5f00\u59cb\u4f4d\u7f6e\u5728\u6587\u4ef6\u5185\u7684\u504f\u79fb"}),(0,s.jsx)(t.td,{children:"\u53ef\u4ee5\u8f6c\u6210size_t\u7c7b\u578b\u7684\u5b57\u7b26\u4e32"}),(0,s.jsx)(t.td,{children:"\u662f"})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"Size"}),(0,s.jsx)(t.td,{children:"\u672c\u6b21\u8bf7\u6c42\u6587\u4ef6\u5206\u7247\u5927\u5c0f"}),(0,s.jsx)(t.td,{children:"\u53ef\u4ee5\u8f6c\u6210size_t\u7c7b\u578b\u7684\u5b57\u7b26\u4e32"}),(0,s.jsx)(t.td,{children:"\u662f"})]})]})]}),"\n",(0,s.jsxs)(t.ul,{children:["\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"RESPONSE"}),":\n\u5982\u679c\u6210\u529f\uff0c\u8fd4\u56de\u7684\u54cd\u5e94\u4fe1\u606f\u4e2dsuccess\u4e3a00"]}),"\n"]}),"\n",(0,s.jsx)(t.h4,{id:"226-\u68c0\u67e5\u6587\u4ef6",children:"2.2.6. \u68c0\u67e5\u6587\u4ef6"}),"\n",(0,s.jsx)(t.p,{children:"\u672c\u63a5\u53e3\u7528\u4e8e\u68c0\u67e5\u5df2\u4e0a\u4f20\u6587\u4ef6\u6b63\u786e\u6027\uff0c\u5982\u679c\u6210\u529f\u901a\u8fc7\u68c0\u67e5\uff0c\u518d\u6b21\u4e0a\u4f20\u540c\u4e00\u6587\u4ef6\u65f6\uff0c\u76f4\u63a5\u8fd4\u56de\u6210\u529f"}),"\n",(0,s.jsxs)(t.ul,{children:["\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"URI"}),":     /check_file"]}),"\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"REQUEST"}),":"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(t.table,{children:[(0,s.jsx)(t.thead,{children:(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.th,{children:"body\u53c2\u6570"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u7c7b\u578b"}),(0,s.jsx)(t.th,{children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,s.jsxs)(t.tbody,{children:[(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"fileName"}),(0,s.jsx)(t.td,{children:"\u6587\u4ef6\u540d\u79f0"}),(0,s.jsx)(t.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"\u662f"})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"checkSum"}),(0,s.jsx)(t.td,{children:"\u6587\u4ef6\u5bf9\u5e94md5\u503c"}),(0,s.jsx)(t.td,{children:"\u53ef\u4ee5\u8f6c\u6210int\u7c7b\u578b\u7684\u5b57\u7b26\u4e32"}),(0,s.jsx)(t.td,{children:'flag\u4e3a"1"\u65f6\u5fc5\u586b'})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"fileSize"}),(0,s.jsx)(t.td,{children:"\u6587\u4ef6\u957f\u5ea6(\u4ee5\u5b57\u8282\u8ba1\u7b97)"}),(0,s.jsx)(t.td,{children:"\u53ef\u4ee5\u8f6c\u6210int\u7c7b\u578b\u7684\u5b57\u7b26\u4e32"}),(0,s.jsx)(t.td,{children:'flag\u4e3a"2"\u65f6\u5fc5\u586b'})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"flag"}),(0,s.jsx)(t.td,{children:"\u6807\u8bb0\u4f4d\uff0cflag\u4e3a1\u65f6\u6821\u9a8cmd5\u3002flag\u4e3a2\u65f6\u6821\u9a8c\u6587\u4ef6\u957f\u5ea6"}),(0,s.jsx)(t.td,{children:"\u53ef\u4ee5\u8f6c\u6210int\u7c7b\u578b\u7684\u5b57\u7b26\u4e32"}),(0,s.jsx)(t.td,{children:"\u662f"})]})]})]}),"\n",(0,s.jsxs)(t.ul,{children:["\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"RESPONSE"}),":\n\u5982\u679c\u6210\u529f\uff0c\u8fd4\u56de\u7684\u54cd\u5e94\u4fe1\u606f\u4e2dsuccess\u4e3a00"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(t.table,{children:[(0,s.jsx)(t.thead,{children:(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.th,{children:"body\u53c2\u6570"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u7c7b\u578b"}),(0,s.jsx)(t.th,{children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,s.jsx)(t.tbody,{children:(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"pass"}),(0,s.jsx)(t.td,{children:"\u68c0\u67e5\u6210\u529f\u4e3atrue\uff0c\u5426\u5219\u4e3afalse"}),(0,s.jsx)(t.td,{children:"bool\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"\u662f"})]})})]}),"\n",(0,s.jsx)(t.p,{children:(0,s.jsx)(t.strong,{children:"Example request."})}),"\n",(0,s.jsx)(t.pre,{children:(0,s.jsx)(t.code,{children:'{"fileName" : "test.csv", "checkSum" : "$MD5", "flag" : \u201c1\u201d}\n'})}),"\n",(0,s.jsx)(t.h4,{id:"227-\u6e05\u7406\u7f13\u5b58\u6587\u4ef6",children:"2.2.7. \u6e05\u7406\u7f13\u5b58\u6587\u4ef6"}),"\n",(0,s.jsx)(t.p,{children:"admin\u7528\u6237\u53ef\u4ee5\u5220\u9664\u6240\u6709\u7528\u6237\u4e0a\u4f20\u7684\u6587\u4ef6\uff0c\u5176\u4ed6\u7528\u6237\u53ef\u4ee5\u5220\u9664\u81ea\u5df1\u7684\u4e0a\u4f20\u7684\u6587\u4ef6\uff0c\u53ef\u4ee5\u5220\u9664\u6307\u5b9a\u540d\u79f0\u7684\u6587\u4ef6\uff0c\u6307\u5b9a\u7528\u6237\u4e0a\u4f20\u7684\u6587\u4ef6\uff0c\u4e5f\u53ef\u4ee5\u5220\u9664\u6240\u6709\u6587\u4ef6"}),"\n",(0,s.jsxs)(t.ul,{children:["\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"URI"}),":     /clear_cache"]}),"\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"REQUEST"}),":"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(t.table,{children:[(0,s.jsx)(t.thead,{children:(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.th,{children:"body\u53c2\u6570"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u7c7b\u578b"}),(0,s.jsx)(t.th,{children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,s.jsxs)(t.tbody,{children:[(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"fileName"}),(0,s.jsx)(t.td,{children:"\u6587\u4ef6\u540d\u79f0"}),(0,s.jsx)(t.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:'flag\u4e3a"0"\u65f6\u5fc5\u586b'})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"userName"}),(0,s.jsx)(t.td,{children:"\u7528\u6237\u540d\u79f0"}),(0,s.jsx)(t.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:'flag\u4e3a"1"\u65f6\u5fc5\u586b'})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"flag"}),(0,s.jsx)(t.td,{children:"\u6807\u8bb0\u4f4d\uff0cflag\u4e3a0\u65f6\u5220\u9664fileName\u6307\u5b9a\u6587\u4ef6, flag\u4e3a1\u65f6\u5220\u9664userName\u6307\u5b9a\u7528\u6237\u5df2\u7ecf\u4e0a\u4f20\u7684\u6240\u6709\u6587\u4ef6\uff0cflag\u4e3a2\u65f6\u5220\u9664\u6240\u6709\u7528\u6237\u4e0a\u4f20\u7684\u6587\u4ef6"}),(0,s.jsx)(t.td,{children:"\u53ef\u4ee5\u8f6c\u6210int\u7c7b\u578b\u7684\u5b57\u7b26\u4e32"}),(0,s.jsx)(t.td,{children:"\u662f"})]})]})]}),"\n",(0,s.jsxs)(t.ul,{children:["\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"RESPONSE"}),":\n\u5982\u679c\u6210\u529f\uff0c\u8fd4\u56de\u4e2dsuccess\u4e3a00"]}),"\n"]}),"\n",(0,s.jsx)(t.p,{children:(0,s.jsx)(t.strong,{children:"Example request."})}),"\n",(0,s.jsx)(t.pre,{children:(0,s.jsx)(t.code,{children:'{"fileName" : "test.csv", "userName" : "test", "flag" : \u201c1\u201d}\n'})}),"\n",(0,s.jsx)(t.h4,{id:"228-\u5bfc\u5165schema",children:"2.2.8. \u5bfc\u5165schema"}),"\n",(0,s.jsx)(t.p,{children:"\u672c\u63a5\u53e3\u53ef\u4ee5\u6839\u636e\u7528\u6237\u6307\u5b9a\u7684schema\u4fe1\u606f\u521b\u5efaschema\u6a21\u578b\uff0cschema\u7684\u8be6\u7ec6\u683c\u5f0f\u4fe1\u606f\u8bf7\u53c2\u8003data-import.md"}),"\n",(0,s.jsxs)(t.ul,{children:["\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"URI"}),":     /import_schema"]}),"\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"REQUEST"}),":"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(t.table,{children:[(0,s.jsx)(t.thead,{children:(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.th,{children:"body\u53c2\u6570"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u7c7b\u578b"}),(0,s.jsx)(t.th,{children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,s.jsxs)(t.tbody,{children:[(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"graph"}),(0,s.jsx)(t.td,{children:"\u5b50\u56fe\u540d\u79f0"}),(0,s.jsx)(t.td,{children:"\u5b57\u7b26\u4e32"}),(0,s.jsx)(t.td,{children:"\u662f"})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"description"}),(0,s.jsx)(t.td,{children:"schema\u63cf\u8ff0\u4fe1\u606f"}),(0,s.jsx)(t.td,{children:"json\u5b57\u7b26\u4e32"}),(0,s.jsx)(t.td,{children:"\u662f"})]})]})]}),"\n",(0,s.jsxs)(t.ul,{children:["\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"RESPONSE"}),":\n\u5982\u679c\u6210\u529f\uff0c\u8fd4\u56de\u4e2dsuccess\u4e3a00"]}),"\n"]}),"\n",(0,s.jsx)(t.p,{children:(0,s.jsx)(t.strong,{children:"Example request."})}),"\n",(0,s.jsx)(t.pre,{children:(0,s.jsx)(t.code,{children:'{\n\t"graph": "test_graph",\n\t"description": {\n\t\t"schema": [{\n\t\t\t"label": "Person",\n\t\t\t"type": "VERTEX",\n\t\t\t"primary": "name",\n\t\t\t"properties": [{\n\t\t\t\t"name": "name",\n\t\t\t\t"type": "STRING"\n\t\t\t}, {\n\t\t\t\t"name": "birthyear",\n\t\t\t\t"type": "INT16",\n\t\t\t\t"optional": true\n\t\t\t}, {\n\t\t\t\t"name": "phone",\n\t\t\t\t"type": "INT16",\n\t\t\t\t"unique": true,\n\t\t\t\t"index": true\n\t\t\t}]\n\t\t}, {\n\t\t\t"label": "City",\n\t\t\t"type": "VERTEX",\n\t\t\t"primary": "name",\n\t\t\t"properties": [{\n\t\t\t\t"name": "name",\n\t\t\t\t"type": "STRING"\n\t\t\t}]\n\t\t}, {\n\t\t\t"label": "Film",\n\t\t\t"type": "VERTEX",\n\t\t\t"primary": "title",\n\t\t\t"properties": [{\n\t\t\t\t"name": "title",\n\t\t\t\t"type": "STRING"\n\t\t\t}]\n\t\t}, {\n\t\t\t"label": "HAS_CHILD",\n\t\t\t"type": "EDGE"\n\t\t}, {\n\t\t\t"label": "MARRIED",\n\t\t\t"type": "EDGE"\n\t\t}, {\n\t\t\t"label": "BORN_IN",\n\t\t\t"type": "EDGE",\n\t\t\t"properties": [{\n\t\t\t\t"name": "weight",\n\t\t\t\t"type": "FLOAT",\n\t\t\t\t"optional": true\n\t\t\t}]\n\t\t}, {\n\t\t\t"label": "DIRECTED",\n\t\t\t"type": "EDGE"\n\t\t}, {\n\t\t\t"label": "WROTE_MUSIC_FOR",\n\t\t\t"type": "EDGE"\n\t\t}, {\n\t\t\t"label": "ACTED_IN",\n\t\t\t"type": "EDGE",\n\t\t\t"properties": [{\n\t\t\t\t"name": "charactername",\n\t\t\t\t"type": "STRING"\n\t\t\t}]\n\t\t}, {\n\t\t\t"label": "PLAY_IN",\n\t\t\t"type": "EDGE",\n\t\t\t"properties": [{\n\t\t\t\t"name": "role",\n\t\t\t\t"type": "STRING",\n\t\t\t\t"optional": true\n\t\t\t}],\n\t\t\t"constraints": [\n\t\t\t\t["Person", "Film"]\n\t\t\t]\n\t\t}]\n\t}\n}\n'})}),"\n",(0,s.jsx)(t.h4,{id:"229-\u5bfc\u5165\u6570\u636e",children:"2.2.9. \u5bfc\u5165\u6570\u636e"}),"\n",(0,s.jsx)(t.p,{children:"\u672c\u63a5\u53e3\u5141\u8bb8\u7528\u6237\u6307\u5b9a\u5df2\u7ecf\u901a\u8fc7\u4e0a\u4f20\uff0c\u6821\u9a8c\u7684\u6587\u4ef6\u4f5c\u4e3a\u6570\u636e\u6587\u4ef6\uff0c\u6309\u7167schema\u53c2\u6570\u63cf\u8ff0\u7684\u914d\u7f6e\u4fe1\u606f\uff0c\u5bfc\u5165\u5230graph\u53c2\u6570\u6307\u5b9a\u7684\u5b50\u56fe\u4e2d\u3002\u5bfc\u5165\u8fc7\u7a0b\u662f\u5f02\u6b65\u7684\uff0cserver\u5728\u63a5\u6536\u5230\u5bfc\u5165\u8bf7\u6c42\u540e\uff0c\u8fd4\u56de\u4e00\u4e2a\u4efb\u52a1id"}),"\n",(0,s.jsxs)(t.ul,{children:["\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"URI"}),":     /import_data"]}),"\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"REQUEST"}),":"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(t.table,{children:[(0,s.jsx)(t.thead,{children:(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.th,{children:"body\u53c2\u6570"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u7c7b\u578b"}),(0,s.jsx)(t.th,{children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,s.jsxs)(t.tbody,{children:[(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"graph"}),(0,s.jsx)(t.td,{children:"\u5b50\u56fe\u540d\u79f0"}),(0,s.jsx)(t.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"\u662f"})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"schema"}),(0,s.jsx)(t.td,{children:"\u5bfc\u5165schema\u63cf\u8ff0"}),(0,s.jsx)(t.td,{children:"json\u5b57\u7b26\u4e32"}),(0,s.jsx)(t.td,{children:"\u662f"})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"delimiter"}),(0,s.jsx)(t.td,{children:"\u5206\u9694\u7b26"}),(0,s.jsx)(t.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"\u662f"})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"continueOnError"}),(0,s.jsx)(t.td,{children:"\u5355\u884c\u6570\u636e\u51fa\u9519\u662f\u5426\u8df3\u8fc7\u9519\u8bef\u5e76\u7ee7\u7eed"}),(0,s.jsx)(t.td,{children:"boolean\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"\u5426"})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"skipPackages"}),(0,s.jsx)(t.td,{children:"\u8df3\u8fc7\u7684\u5305\u4e2a\u6570"}),(0,s.jsx)(t.td,{children:"\u53ef\u4ee5\u8f6c\u6210int\u7c7b\u578b\u7684\u5b57\u7b26\u4e32"}),(0,s.jsx)(t.td,{children:"\u5426"})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"taskId"}),(0,s.jsx)(t.td,{children:"\u4efb\u52a1id\uff0c\u7528\u4e8e\u91cd\u542f\u51fa\u9519\u7684\u4efb\u52a1"}),(0,s.jsx)(t.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"\u5426"})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"flag"}),(0,s.jsx)(t.td,{children:"\u6807\u8bb0\u4f4d\uff0cflag\u4e3a1\u65f6\u5bfc\u5165\u6210\u529f\u5c06\u5220\u9664\u6570\u636e\u6587\u4ef6"}),(0,s.jsx)(t.td,{children:"\u53ef\u4ee5\u8f6c\u6210int\u7c7b\u578b\u7684\u5b57\u7b26\u4e32"}),(0,s.jsx)(t.td,{children:"\u5426"})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"other"}),(0,s.jsx)(t.td,{children:"\u5176\u4ed6\u53c2\u6570"}),(0,s.jsx)(t.td,{children:"\u53ef\u4ee5\u8f6c\u6210int\u7c7b\u578b\u7684\u5b57\u7b26\u4e32"}),(0,s.jsx)(t.td,{children:"\u5426"})]})]})]}),"\n",(0,s.jsxs)(t.ul,{children:["\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"RESPONSE"}),":\n\u5982\u679c\u6210\u529f\uff0c\u8fd4\u56de\u7684\u54cd\u5e94\u4fe1\u606f\u4e2dsuccess\u4e3a00"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(t.table,{children:[(0,s.jsx)(t.thead,{children:(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.th,{children:"body\u53c2\u6570"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u7c7b\u578b"}),(0,s.jsx)(t.th,{children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,s.jsx)(t.tbody,{children:(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"taskId"}),(0,s.jsx)(t.td,{children:"\u4efb\u52a1\u7f16\u53f7"}),(0,s.jsx)(t.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"\u662f"})]})})]}),"\n",(0,s.jsx)(t.p,{children:(0,s.jsx)(t.strong,{children:"Example request."})}),"\n",(0,s.jsx)(t.pre,{children:(0,s.jsx)(t.code,{children:'{\n   "graph": "default",         //\u5bfc\u5165\u7684\u5b50\u56fe\u540d\u79f0\n   "delimiter": ",",\t\t\t\t\t\t//\u6570\u636e\u5206\u9694\u7b26\n   "continueOnError": true,\t\t//\u9047\u5230\u9519\u8bef\u65f6\u662f\u5426\u8df3\u8fc7\u9519\u8bef\u6570\u636e\u5e76\u7ee7\u7eed\u5bfc\u5165\n   "skipPackages": \u201c0\u201d,\t\t\t\t\t//\u8df3\u8fc7\u7684\u5305\u4e2a\u6570\n   "flag" : "1",\n\t "schema": {\n\t\t"files": [{\n\t\t\t"DST_ID": "Film",\t\t\t\t//\u7ec8\u70b9label\u7c7b\u578b\n\t\t\t"SRC_ID": "Person",\t\t\t//\u8d77\u70b9label\u7c7b\u578b\n\t\t\t"columns": [\t\t\t\t\t\t//\u6570\u636e\u683c\u5f0f\u8bf4\u660e\n\t\t\t\t"SRC_ID",\t\t\t\t\t\t\t//\u8d77\u70b9id\n\t\t\t\t"DST_ID",\t\t\t\t\t\t\t//\u7ec8\u70b9id\n                "SKIP",\t\t\t\t\t\t\t\t//\u8868\u793a\u8df3\u8fc7\u6b64\u5217\u6570\u636e\n\t\t\t\t"charactername"\t\t\t\t//\u5c5e\u6027\u540d\u79f0\n\t\t\t],\n\t\t\t"format": "CSV",\t\t\t\t//\u6570\u636e\u6587\u4ef6\u683c\u5f0f\u7c7b\u578b,\u652f\u6301csv\u548cjson\n\t\t\t"path": "acted_in.csv",\t//\u6570\u636e\u6587\u4ef6\u540d\u79f0\n\t\t\t"header": 0, \t\t\t\t\t\t//\u6570\u636e\u4ece\u7b2c\u51e0\u884c\u5f00\u59cb\n\t\t\t"label": "ACTED_IN"\t\t\t//\u8fb9\u7684\u7c7b\u578b\n\t\t}]\n\t}\n}\n'})}),"\n",(0,s.jsx)(t.h4,{id:"2210-\u5bfc\u5165\u8fdb\u5ea6\u67e5\u8be2",children:"2.2.10. \u5bfc\u5165\u8fdb\u5ea6\u67e5\u8be2"}),"\n",(0,s.jsx)(t.p,{children:"\u672c\u63a5\u53e3\u7528\u4e8e\u67e5\u8be2\u5bfc\u5165\u4efb\u52a1\u7684\u6267\u884c\u8fdb\u5ea6"}),"\n",(0,s.jsxs)(t.ul,{children:["\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"URI"}),":     /import_progress"]}),"\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"METHOD"}),":  POST"]}),"\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"REQUEST"}),":"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(t.table,{children:[(0,s.jsx)(t.thead,{children:(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.th,{children:"body\u53c2\u6570"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u7c7b\u578b"}),(0,s.jsx)(t.th,{children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,s.jsx)(t.tbody,{children:(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"taskId"}),(0,s.jsx)(t.td,{children:"import_data\u63a5\u53e3\u8fd4\u56de\u7684\u4efb\u52a1id"}),(0,s.jsx)(t.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"\u662f"})]})})]}),"\n",(0,s.jsxs)(t.ul,{children:["\n",(0,s.jsxs)(t.li,{children:[(0,s.jsx)(t.strong,{children:"RESPONSE"}),":\n\u5982\u679c\u6210\u529f\uff0c\u8fd4\u56de\u7684\u54cd\u5e94\u4fe1\u606f\u4e2dsuccess\u4e3a00"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(t.table,{children:[(0,s.jsx)(t.thead,{children:(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.th,{children:"body\u53c2\u6570"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u8bf4\u660e"}),(0,s.jsx)(t.th,{children:"\u53c2\u6570\u7c7b\u578b"}),(0,s.jsx)(t.th,{children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,s.jsxs)(t.tbody,{children:[(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"success"}),(0,s.jsx)(t.td,{children:"\u662f\u5426\u6210\u529f\u5bfc\u5165"}),(0,s.jsx)(t.td,{children:"boolean\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"\u662f"})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"reason"}),(0,s.jsx)(t.td,{children:"\u5bfc\u5165\u5931\u8d25\u539f\u56e0"}),(0,s.jsx)(t.td,{children:"\u5b57\u7b26\u4e32\u7c7b\u578b"}),(0,s.jsx)(t.td,{children:"success\u4e3afalse\u65f6\u5fc5\u586b"})]}),(0,s.jsxs)(t.tr,{children:[(0,s.jsx)(t.td,{children:"progress"}),(0,s.jsx)(t.td,{children:"\u5f53\u524d\u5bfc\u5165\u8fdb\u5ea6"}),(0,s.jsx)(t.td,{children:"\u53ef\u4ee5\u8f6c\u6210double\u7c7b\u578b\u7684\u5b57\u7b26\u4e32"}),(0,s.jsx)(t.td,{children:"\u662f"})]})]})]}),"\n",(0,s.jsx)(t.p,{children:(0,s.jsx)(t.strong,{children:"Example request."})}),"\n",(0,s.jsx)(t.pre,{children:(0,s.jsx)(t.code,{children:'{"taskId" : "$taskId"}\n'})})]})}function j(n={}){const{wrapper:t}={...(0,r.R)(),...n.components};return t?(0,s.jsx)(t,{...n,children:(0,s.jsx)(x,{...n})}):x(n)}}}]);