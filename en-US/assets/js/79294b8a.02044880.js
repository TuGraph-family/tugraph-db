"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[90779],{28453:(e,n,t)=>{t.d(n,{R:()=>i,x:()=>d});var l=t(96540);const r={},s=l.createContext(r);function i(e){const n=l.useContext(s);return l.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function d(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(r):e.components||r:i(e.components),l.createElement(s.Provider,{value:n},e.children)}},52633:(e,n,t)=>{t.r(n),t.d(n,{assets:()=>c,contentTitle:()=>i,default:()=>j,frontMatter:()=>s,metadata:()=>d,toc:()=>h});var l=t(74848),r=t(28453);const s={},i="TuGraph-Restful-Server",d={id:"utility-tools/restful",title:"TuGraph-Restful-Server",description:"TuGraph Restful Server \u5f3a\u4f9d\u8d56 TuGraph\uff0cRestful Server \u4e0e Tugraph \u5171\u5b58",source:"@site/versions/version-4.3.1/zh-CN/source/6.utility-tools/9.restful.md",sourceDirName:"6.utility-tools",slug:"/utility-tools/restful",permalink:"/tugraph-db/en-US/zh/4.3.1/utility-tools/restful",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:9,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"TuGraph-Explorer",permalink:"/tugraph-db/en-US/zh/4.3.1/utility-tools/tugraph-explorer"},next:{title:"Python\u5ba2\u6237\u7aef",permalink:"/tugraph-db/en-US/zh/4.3.1/client-tools/python-client"}},c={},h=[{value:"1.TuGraph-Restful-Server \u7b80\u4ecb",id:"1tugraph-restful-server-\u7b80\u4ecb",level:2},{value:"2.\u542f\u52a8 TuGraph-Restful-Server",id:"2\u542f\u52a8-tugraph-restful-server",level:2},{value:"3.\u8fde\u63a5 TuGraph-Restful-Server",id:"3\u8fde\u63a5-tugraph-restful-server",level:2},{value:"4.\u6570\u636e\u683c\u5f0f",id:"4\u6570\u636e\u683c\u5f0f",level:2},{value:"5.\u8fd4\u56de\u503c",id:"5\u8fd4\u56de\u503c",level:2},{value:"6.URI\u683c\u5f0f",id:"6uri\u683c\u5f0f",level:2},{value:"7.\u63a5\u53e3",id:"7\u63a5\u53e3",level:2},{value:"7.1 \u7528\u6237\u767b\u9646",id:"71-\u7528\u6237\u767b\u9646",level:3},{value:"7.1.1 URL",id:"711-url",level:4},{value:"7.1.2 REQUEST",id:"712-request",level:4},{value:"7.1.3 RESPONSE",id:"713-response",level:4},{value:"7.2 \u5237\u65b0token",id:"72-\u5237\u65b0token",level:3},{value:"7.2.1 URL",id:"721-url",level:4},{value:"7.2.2 REQUEST",id:"722-request",level:4},{value:"7.2.3 RESPONSE",id:"723-response",level:4},{value:"7.3 \u7528\u6237\u767b\u51fa",id:"73-\u7528\u6237\u767b\u51fa",level:3},{value:"7.3.1 URL",id:"731-url",level:4},{value:"7.3.2 REQUEST",id:"732-request",level:4},{value:"7.4 \u6267\u884ccypher\u67e5\u8be2\u8bf7\u6c42",id:"74-\u6267\u884ccypher\u67e5\u8be2\u8bf7\u6c42",level:3},{value:"7.4.1 URL",id:"741-url",level:4},{value:"7.4.2 REQUEST",id:"742-request",level:4},{value:"7.4.3 RESPONSE",id:"743-response",level:4},{value:"7.5 \u4e0a\u4f20\u6587\u4ef6\u8bf7\u6c42",id:"75-\u4e0a\u4f20\u6587\u4ef6\u8bf7\u6c42",level:3},{value:"7.5.1 URL",id:"751-url",level:4},{value:"7.5.2 REQUEST",id:"752-request",level:4},{value:"7.6 \u6587\u4ef6\u8ba4\u8bc1\u8bf7\u6c42",id:"76-\u6587\u4ef6\u8ba4\u8bc1\u8bf7\u6c42",level:3},{value:"7.6.1 URL",id:"761-url",level:4},{value:"7.6.2 REQUEST",id:"762-request",level:4},{value:"7.6.3 RESPONSE",id:"763-response",level:4},{value:"7.7 \u6279\u91cf\u521b\u5efaschema\u8bf7\u6c42",id:"77-\u6279\u91cf\u521b\u5efaschema\u8bf7\u6c42",level:3},{value:"7.7.1 URL",id:"771-url",level:4},{value:"7.7.2 REQUEST",id:"772-request",level:4},{value:"7.8 \u6570\u636e\u5bfc\u5165\u8bf7\u6c42",id:"78-\u6570\u636e\u5bfc\u5165\u8bf7\u6c42",level:3},{value:"7.8.1 URL",id:"781-url",level:4},{value:"7.8.2 REQUEST",id:"782-request",level:4},{value:"7.8.3 RESPONSE",id:"783-response",level:4},{value:"7.9 \u6e05\u7406\u7528\u6237\u4e0a\u4f20\u6587\u4ef6\u8bf7\u6c42",id:"79-\u6e05\u7406\u7528\u6237\u4e0a\u4f20\u6587\u4ef6\u8bf7\u6c42",level:3},{value:"7.9.1 URL",id:"791-url",level:4},{value:"7.9.2 REQUEST",id:"792-request",level:4},{value:"7.10. \u5bfc\u5165\u8fdb\u5ea6\u67e5\u8be2\u8bf7\u6c42",id:"710-\u5bfc\u5165\u8fdb\u5ea6\u67e5\u8be2\u8bf7\u6c42",level:3},{value:"7.10.1 URL",id:"7101-url",level:4},{value:"7.10.2 REQUEST",id:"7102-request",level:4},{value:"7.10.3 RESPONSE",id:"7103-response",level:4}];function x(e){const n={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",h4:"h4",header:"header",li:"li",p:"p",pre:"pre",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",ul:"ul",...(0,r.R)(),...e.components};return(0,l.jsxs)(l.Fragment,{children:[(0,l.jsx)(n.header,{children:(0,l.jsx)(n.h1,{id:"tugraph-restful-server",children:"TuGraph-Restful-Server"})}),"\n",(0,l.jsxs)(n.blockquote,{children:["\n",(0,l.jsx)(n.p,{children:"TuGraph Restful Server \u5f3a\u4f9d\u8d56 TuGraph\uff0cRestful Server \u4e0e Tugraph \u5171\u5b58"}),"\n"]}),"\n",(0,l.jsx)(n.h2,{id:"1tugraph-restful-server-\u7b80\u4ecb",children:"1.TuGraph-Restful-Server \u7b80\u4ecb"}),"\n",(0,l.jsx)(n.p,{children:"TuGraph Restful Server \u4f7f\u7528brpc\u6846\u67b6\u652f\u6301\u7684http\u534f\u8bae\uff0c\u63d0\u4f9brestful\u63a5\u53e3\u67e5\u8be2\u529f\u80fd\uff0c\u5728\u5b9e\u73b0\u4e2d\uff0crestful server \u4e0erpc server \u4f7f\u7528\u540c\u4e00\u4e2a\u7aef\u53e3\u3002\u76ee\u524drestful\u63a5\u53e3\u63d0\u4f9b\u6587\u4ef6\u4e0a\u4f20\uff0c\u6570\u636e\u5bfc\u5165\uff0c\u5bfc\u5165\u8fdb\u5ea6\u67e5\u8be2\uff0ccypher\u67e5\u8be2\uff0c\u6587\u4ef6\u5220\u9664\u7b49\u529f\u80fd"}),"\n",(0,l.jsx)(n.h2,{id:"2\u542f\u52a8-tugraph-restful-server",children:"2.\u542f\u52a8 TuGraph-Restful-Server"}),"\n",(0,l.jsx)(n.p,{children:"\u9700\u8981\u5728\u542f\u52a8Tugraph\u65f6\u8bbe\u7f6eenable_rpc\u53c2\u6570\u4e3atrue\u7684\u65b9\u5f0f\uff0c\u6b63\u5e38\u542f\u52a8TuGraph"}),"\n",(0,l.jsx)(n.h2,{id:"3\u8fde\u63a5-tugraph-restful-server",children:"3.\u8fde\u63a5 TuGraph-Restful-Server"}),"\n",(0,l.jsx)(n.p,{children:"TuGraph\u6b63\u5e38\u542f\u52a8\u540e\uff0cRestful Server \u5c06\u76d1\u542c\u5728rpc_port\u4e0a\uff0c \u901a\u8fc7\u8bbf\u95ee http://${ip}:${rpc_port}/LGraphHttpService/Query/ url\u53ef\u4ee5\u94fe\u63a5\u5230TuGraph"}),"\n",(0,l.jsx)(n.h2,{id:"4\u6570\u636e\u683c\u5f0f",children:"4.\u6570\u636e\u683c\u5f0f"}),"\n",(0,l.jsxs)(n.p,{children:["\u5ba2\u6237\u7aef\u4e0e\u670d\u52a1\u7aef\u6570\u636e\u4ea4\u4e92\u7684\u683c\u5f0f\u662f JSON\u3002\u5728\u53d1\u9001\u8bf7\u6c42\u65f6\uff0c\u8bf7\u5c06\u53d1\u9001\u6570\u636e\u7684\u8bf7\u6c42\u7684\u62a5\u5934\u8bbe\u7f6e\u4e3a ",(0,l.jsx)(n.code,{children:"Accept:application/json, Content-Type:application/json"}),"\u3002\n\u4f8b\u5982\u5728\u521b\u5efa\u4e00\u4e2a\u70b9\u65f6\uff0c\u8bf7\u6c42\u62a5\u5934\u5305\u542b\u4ee5\u4e0b\u5185\u5bb9\uff1a"]}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{children:"    Accept: application/json; charset=UTF-8\n    Content-Type: application/json\n    server_version: 12\n"})}),"\n",(0,l.jsx)(n.h2,{id:"5\u8fd4\u56de\u503c",children:"5.\u8fd4\u56de\u503c"}),"\n",(0,l.jsx)(n.p,{children:"\u901a\u7528\u8fd4\u56de\u683c\u5f0f"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(n.table,{children:[(0,l.jsx)(n.thead,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"body\u53c2\u6570"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u8bf4\u660e"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u7c7b\u578b"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,l.jsxs)(n.tbody,{children:[(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"errorCode"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u72b6\u6001\u7801"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"errorMessage"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u9519\u8bef\u4fe1\u606f"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"data"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u8fd4\u56de\u7684\u6570\u636e"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]})]})]}),"\n",(0,l.jsx)(n.p,{children:"TuGraph \u8fd4\u56de\u7684 HTTP \u72b6\u6001\u7801\u5305\u542b\u4ee5\u4e0b\u56db\u79cd\uff1a"}),"\n",(0,l.jsxs)(n.ul,{children:["\n",(0,l.jsx)(n.li,{children:"200 OK: \u64cd\u4f5c\u6210\u529f"}),"\n",(0,l.jsx)(n.li,{children:"400 Bad Request: \u8f93\u5165\u6709\u8bef\uff0c\u4f8b\u5982 URI \u9519\u8bef\uff0c\u6216\u8005\u8bf7\u6c42\u4e2d\u7684 JSON \u53c2\u6570\u9519\u8bef"}),"\n",(0,l.jsx)(n.li,{children:"401 Unauthorized: \u672a\u901a\u8fc7\u9274\u6743\u8ba4\u8bc1\uff0c\u4f8b\u5982\u7528\u6237\u540d\u5bc6\u7801\u9519\u8bef\uff0ctoken\u8d85\u8fc7\u6709\u6548\u671f\u7b49"}),"\n",(0,l.jsx)(n.li,{children:"500 Internal Server Error: \u670d\u52a1\u5668\u7aef\u9519\u8bef\n\u5f53\u64cd\u4f5c\u6210\u529f\u65f6\uff0c\u8fd4\u56de\u7684 data \u4e2d\u5305\u542b\u64cd\u4f5c\u7684\u8fd4\u56de\u503c\u3002\n\u5f53\u53d1\u751f\u8f93\u5165\u9519\u8bef\u6216\u8005\u670d\u52a1\u5668\u9519\u8bef\u65f6\uff0c\u8fd4\u56de\u7684 errorMessage \u4e2d\u5305\u542b\u9519\u8bef\u63d0\u793a\u3002"}),"\n"]}),"\n",(0,l.jsx)(n.h2,{id:"6uri\u683c\u5f0f",children:"6.URI\u683c\u5f0f"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(n.table,{children:[(0,l.jsx)(n.thead,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.th,{children:"URI"}),(0,l.jsx)(n.th,{children:"\u8bf4\u660e"})]})}),(0,l.jsxs)(n.tbody,{children:[(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:"/cypher"}),(0,l.jsx)(n.td,{children:"\u6267\u884ccypher\u67e5\u8be2\u8bf7\u6c42"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:"/refresh"}),(0,l.jsx)(n.td,{children:"\u5237\u65b0token\u8bf7\u6c42"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:"/login"}),(0,l.jsx)(n.td,{children:"\u7528\u6237\u767b\u9646\u8bf7\u6c42"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:"/logout"}),(0,l.jsx)(n.td,{children:"\u7528\u6237\u767b\u51fa\u8bf7\u6c42"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:"/upload_files"}),(0,l.jsx)(n.td,{children:"\u4e0a\u4f20\u6587\u4ef6\u8bf7\u6c42"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:"/clear_cache"}),(0,l.jsx)(n.td,{children:"\u6e05\u7406\u7528\u6237\u4e0a\u4f20\u6587\u4ef6\u8bf7\u6c42"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:"/check_file"}),(0,l.jsx)(n.td,{children:"\u6587\u4ef6\u8ba4\u8bc1\u8bf7\u6c42"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:"/import_data"}),(0,l.jsx)(n.td,{children:"\u6570\u636e\u5bfc\u5165\u8bf7\u6c42"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:"/import_progress"}),(0,l.jsx)(n.td,{children:"\u5bfc\u5165\u8fdb\u5ea6\u67e5\u8be2\u8bf7\u6c42"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:"/import_schema"}),(0,l.jsx)(n.td,{children:"\u6279\u91cf\u521b\u5efaschema\u8bf7\u6c42"})]})]})]}),"\n",(0,l.jsx)(n.h2,{id:"7\u63a5\u53e3",children:"7.\u63a5\u53e3"}),"\n",(0,l.jsx)(n.h3,{id:"71-\u7528\u6237\u767b\u9646",children:"7.1 \u7528\u6237\u767b\u9646"}),"\n",(0,l.jsx)(n.p,{children:"\u7528\u4e8e\u7528\u6237\u7b2c\u4e00\u6b21\u4e0e\u670d\u52a1\u7aef\u901a\u4fe1\u65f6\u7684\u9274\u6743\u64cd\u4f5c\uff0c\u8bf7\u6c42\u62a5\u6587\u5728 http body \u4e2d\u643a\u5e26\u7528\u6237\u540d\u548c\u5bc6\u7801\uff0c\u54cd\u5e94\u62a5\u6587\u5728 http body \u4e2d\u4f1a\u8fd4\u56de\u4e00\u4e2a\u5e26\u6709\u6709\u6548\u671f\u7684token\uff0c\u540e\u7eed\u8bf7\u6c42\u4e2d\u9700\u8981\u5728http header\u4e2d\u643a\u5e26\u8be5token\u4f5c\u4e3a\u51ed\u8bc1"}),"\n",(0,l.jsx)(n.h4,{id:"711-url",children:"7.1.1 URL"}),"\n",(0,l.jsx)(n.p,{children:"http://${ip}:${rpc_port}/LGraphHttpService/Query/login"}),"\n",(0,l.jsx)(n.h4,{id:"712-request",children:"7.1.2 REQUEST"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(n.table,{children:[(0,l.jsx)(n.thead,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"body\u53c2\u6570"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u8bf4\u660e"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u7c7b\u578b"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,l.jsxs)(n.tbody,{children:[(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"userName"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u7528\u6237\u540d"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"password"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5bc6\u7801"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]})]})]}),"\n",(0,l.jsx)(n.h4,{id:"713-response",children:"7.1.3 RESPONSE"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(n.table,{children:[(0,l.jsx)(n.thead,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"body\u53c2\u6570"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u8bf4\u660e"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u7c7b\u578b"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,l.jsx)(n.tbody,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"authorization"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"token"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]})})]}),"\n",(0,l.jsx)(n.h3,{id:"72-\u5237\u65b0token",children:"7.2 \u5237\u65b0token"}),"\n",(0,l.jsx)(n.p,{children:"token\u5230\u671f\u540e\u5c06\u65e0\u6cd5\u4f7f\u7528\u6b64token\u4e0e\u670d\u52a1\u7aef\u6b63\u5e38\u901a\u4fe1\uff0c\u9700\u8981\u83b7\u53d6\u65b0\u7684token\u4f5c\u4e3a\u540e\u7eed\u8bf7\u6c42\u7684\u51ed\u8bc1\uff0c\u8bf7\u6c42\u62a5\u6587\u5728http header\u4e2d\u643a\u5e26\u65e7\u7684token\uff0c\u54cd\u5e94\u62a5\u6587\u5728http body\u4e2d\u8fd4\u56de\u65b0\u7684token"}),"\n",(0,l.jsx)(n.h4,{id:"721-url",children:"7.2.1 URL"}),"\n",(0,l.jsx)(n.p,{children:"http://${ip}:${rpc_port}/LGraphHttpService/Query/refresh"}),"\n",(0,l.jsx)(n.h4,{id:"722-request",children:"7.2.2 REQUEST"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(n.table,{children:[(0,l.jsx)(n.thead,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"header\u53c2\u6570"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u8bf4\u660e"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u7c7b\u578b"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,l.jsx)(n.tbody,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"Authorization"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u65e7\u7684token"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]})})]}),"\n",(0,l.jsx)(n.h4,{id:"723-response",children:"7.2.3 RESPONSE"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(n.table,{children:[(0,l.jsx)(n.thead,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"body\u53c2\u6570"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u8bf4\u660e"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u7c7b\u578b"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,l.jsx)(n.tbody,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"authorization"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u65b0\u7684token"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]})})]}),"\n",(0,l.jsx)(n.h3,{id:"73-\u7528\u6237\u767b\u51fa",children:"7.3 \u7528\u6237\u767b\u51fa"}),"\n",(0,l.jsx)(n.p,{children:"\u7528\u6237\u4e0d\u518d\u9700\u8981\u4e0e\u670d\u52a1\u7aef\u8fdb\u884c\u901a\u4fe1\u65f6\uff0c\u9700\u8981\u8bf7\u6c42\u767b\u51fa\u63a5\u53e3\uff0c\u91ca\u653e\u81ea\u5df1\u7684token\u3002\u8bf7\u6c42\u62a5\u6587\u5728http header\u4e2d\u643a\u5e26\u65e7\u7684token\uff0c\u5982\u679c\u62ff\u5230\u8fd4\u56deerrorCode\u4e3a200\u7684\u54cd\u5e94\u62a5\u6587\u5373\u4e3a\u6b63\u5e38\u767b\u51fa"}),"\n",(0,l.jsx)(n.h4,{id:"731-url",children:"7.3.1 URL"}),"\n",(0,l.jsx)(n.p,{children:"http://${ip}:${rpc_port}/LGraphHttpService/Query/logout"}),"\n",(0,l.jsx)(n.h4,{id:"732-request",children:"7.3.2 REQUEST"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(n.table,{children:[(0,l.jsx)(n.thead,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"header\u53c2\u6570"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u8bf4\u660e"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u7c7b\u578b"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,l.jsx)(n.tbody,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"Authorization"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u65e7\u7684token"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]})})]}),"\n",(0,l.jsx)(n.h3,{id:"74-\u6267\u884ccypher\u67e5\u8be2\u8bf7\u6c42",children:"7.4 \u6267\u884ccypher\u67e5\u8be2\u8bf7\u6c42"}),"\n",(0,l.jsx)(n.p,{children:"\u7528\u6237\u901a\u8fc7\u6b64\u7c7b\u8bf7\u6c42\u53d1cypher\u7ed9server\u7aef\u6267\u884c\u5e76\u83b7\u53d6\u6267\u884c\u7ed3\u679c\uff0c\u8bf7\u6c42\u62a5\u6587\u5728http body \u4e2d\u5c06\u6267\u884c\u7684cypher\u8bed\u53e5\u548c\u76ee\u6807\u5b50\u56fe\u53d1\u9001\u7ed9server\uff0cserver\u6267\u884c\u5b8c\u6210\u540e\u5728\u54cd\u5e94\u62a5\u6587\u7684http body\u4e2d\u8fd4\u56de\u6267\u884c\u7ed3\u679c"}),"\n",(0,l.jsx)(n.h4,{id:"741-url",children:"7.4.1 URL"}),"\n",(0,l.jsx)(n.p,{children:"http://${ip}:${rpc_port}/LGraphHttpService/Query/cypher"}),"\n",(0,l.jsx)(n.h4,{id:"742-request",children:"7.4.2 REQUEST"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(n.table,{children:[(0,l.jsx)(n.thead,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"body\u53c2\u6570"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u8bf4\u660e"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u7c7b\u578b"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,l.jsxs)(n.tbody,{children:[(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"graph"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u67e5\u8be2\u76ee\u6807\u5b50\u56fe"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"script"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"cypher\u67e5\u8be2\u8bed\u53e5"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]})]})]}),"\n",(0,l.jsx)(n.h4,{id:"743-response",children:"7.4.3 RESPONSE"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(n.table,{children:[(0,l.jsx)(n.thead,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"body\u53c2\u6570"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u8bf4\u660e"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u7c7b\u578b"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,l.jsx)(n.tbody,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"result"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u8fd4\u56de\u7ed3\u679c"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]})})]}),"\n",(0,l.jsx)(n.h3,{id:"75-\u4e0a\u4f20\u6587\u4ef6\u8bf7\u6c42",children:"7.5 \u4e0a\u4f20\u6587\u4ef6\u8bf7\u6c42"}),"\n",(0,l.jsx)(n.p,{children:"\u7528\u6237\u901a\u8fc7\u6b64\u7c7b\u8bf7\u6c42\u5411server\u53d1\u9001\u6587\u4ef6\uff0c\u53ef\u4ee5\u5bf9\u6587\u4ef6\u8fdb\u884c\u5206\u7247\uff0c\u5206\u7247\u5927\u5c0f\u4e0d\u5927\u4e8e1MB\uff0c\u652f\u6301\u591a\u7ebf\u7a0b\u4e71\u5e8f\u53d1\u9001\uff0c\u8bf7\u6c42\u62a5\u6587\u5728http header \u4e2d\u5305\u542b\u6587\u4ef6\u540d\uff0c\u7b2c\u4e00\u5b57\u8282\u5185\u5bb9\u5728\u6587\u4ef6\u4e2d\u7684\u504f\u79fb\u548c\u5206\u7247\u5927\u5c0f\uff0c\u5728body\u4e2d\u5305\u542b\u6587\u4ef6\u5185\u5bb9\uff0cserver\u6536\u5230\u8bf7\u6c42\u540e\u5c06\u9a8c\u8bc1\u5206\u7247\u5927\u5c0f\u662f\u5426\u4e0e\u5206\u7247\u5185\u5bb9\u4e00\u81f4\uff0c\u4e00\u81f4\u65f6\u5c06\u6587\u4ef6\u5206\u6bb5\u5199\u5165\u6587\u4ef6\u3002\u4e0d\u4e00\u81f4\u65f6\u5c06\u8fd4\u56deerrorCode\u4e3a400\u7684\u54cd\u5e94"}),"\n",(0,l.jsx)(n.h4,{id:"751-url",children:"7.5.1 URL"}),"\n",(0,l.jsx)(n.p,{children:"http://${ip}:${rpc_port}/LGraphHttpService/Query/upload_files"}),"\n",(0,l.jsx)(n.h4,{id:"752-request",children:"7.5.2 REQUEST"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(n.table,{children:[(0,l.jsx)(n.thead,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"header\u53c2\u6570"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u8bf4\u660e"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u7c7b\u578b"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,l.jsxs)(n.tbody,{children:[(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"File-Name"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u6587\u4ef6\u540d"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"Begin-Pos"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5f00\u59cb\u4f4d\u7f6e\u5728\u6587\u4ef6\u5185\u7684\u504f\u79fb"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"Size"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u6587\u4ef6\u5206\u7247\u5927\u5c0f"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"body\u53c2\u6570"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u53c2\u6570\u8bf4\u660e"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u53c2\u6570\u7c7b\u578b"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f\u5426\u5fc5\u586b"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"-"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u6587\u4ef6\u5185\u5bb9"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]})]})]}),"\n",(0,l.jsx)(n.h3,{id:"76-\u6587\u4ef6\u8ba4\u8bc1\u8bf7\u6c42",children:"7.6 \u6587\u4ef6\u8ba4\u8bc1\u8bf7\u6c42"}),"\n",(0,l.jsx)(n.p,{children:"\u7528\u6237\u901a\u8fc7\u6b64\u7c7b\u8bf7\u6c42\u9a8c\u8bc1\u53d1\u9001\u5230server\u7aef\u7684\u6587\u4ef6\u662f\u5426\u4e0e\u671f\u671b\u4e00\u81f4\uff0cserver\u7aef\u4f7f\u7528\u4e24\u79cd\u9a8c\u8bc1\u65b9\u5f0f\uff0cmd5\u503c\u548c\u6587\u4ef6\u957f\u5ea6\uff0c\u76ee\u524d\u5df2\u652f\u6301\u6587\u4ef6\u957f\u5ea6\u9a8c\u8bc1\u3002"}),"\n",(0,l.jsx)(n.h4,{id:"761-url",children:"7.6.1 URL"}),"\n",(0,l.jsx)(n.p,{children:"http://${ip}:${rpc_port}/LGraphHttpService/Query/check_file"}),"\n",(0,l.jsx)(n.h4,{id:"762-request",children:"7.6.2 REQUEST"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(n.table,{children:[(0,l.jsx)(n.thead,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"body\u53c2\u6570"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u8bf4\u660e"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u7c7b\u578b"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,l.jsxs)(n.tbody,{children:[(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"fileName"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u6587\u4ef6\u540d"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"checkSum"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u6587\u4ef6\u5bf9\u5e94md5\u503c"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5426"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"fileSize"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u6587\u4ef6\u957f\u5ea6(\u4ee5\u5b57\u8282\u8ba1\u7b97)"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5426"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"flag"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u6807\u8bb0\u4f4d\uff0c\u4e3a1\u65f6\u6821\u9a8cmd5\u503c\uff0c\u4e3a2\u65f6\u6821\u9a8c\u6587\u4ef6\u957f\u5ea6"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]})]})]}),"\n",(0,l.jsx)(n.h4,{id:"763-response",children:"7.6.3 RESPONSE"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(n.table,{children:[(0,l.jsx)(n.thead,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"body\u53c2\u6570"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u8bf4\u660e"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u7c7b\u578b"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,l.jsx)(n.tbody,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"pass"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u68c0\u67e5\u6210\u529f\u4e3atrue\uff0c\u5426\u5219\u4e3afalse"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"bool"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]})})]}),"\n",(0,l.jsx)(n.h3,{id:"77-\u6279\u91cf\u521b\u5efaschema\u8bf7\u6c42",children:"7.7 \u6279\u91cf\u521b\u5efaschema\u8bf7\u6c42"}),"\n",(0,l.jsx)(n.p,{children:"\u7528\u6237\u901a\u8fc7\u6b64\u7c7b\u8bf7\u6c42\u6279\u91cf\u521b\u5efaschema\uff0c\u8bf7\u6c42\u62a5\u6587\u5728http body \u4e2d\u5c06\u521b\u5efaschema\u7684\u76ee\u6807\u5b50\u56fe\u548cschema\u4fe1\u606f\u53d1\u9001\u7ed9server\uff0c\u5982\u679c\u62ff\u5230\u8fd4\u56deerrorCode\u4e3a200\u7684\u54cd\u5e94\u62a5\u6587\u5373\u4e3a\u6b63\u5e38\u521b\u5efa"}),"\n",(0,l.jsx)(n.h4,{id:"771-url",children:"7.7.1 URL"}),"\n",(0,l.jsx)(n.p,{children:"http://${ip}:${rpc_port}/LGraphHttpService/Query/import_schema"}),"\n",(0,l.jsx)(n.h4,{id:"772-request",children:"7.7.2 REQUEST"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(n.table,{children:[(0,l.jsx)(n.thead,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"body\u53c2\u6570"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u8bf4\u660e"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u7c7b\u578b"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,l.jsxs)(n.tbody,{children:[(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"graph"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u521b\u5efa\u76ee\u6807\u5b50\u56fe"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"schema"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"schema\u63cf\u8ff0\u4fe1\u606f"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]})]})]}),"\n",(0,l.jsx)(n.h3,{id:"78-\u6570\u636e\u5bfc\u5165\u8bf7\u6c42",children:"7.8 \u6570\u636e\u5bfc\u5165\u8bf7\u6c42"}),"\n",(0,l.jsx)(n.p,{children:"\u7528\u6237\u901a\u8fc7\u6b64\u7c7b\u8bf7\u6c42\u5bfc\u5165\u5df2\u7ecf\u4e0a\u4f20\u7684\u6570\u636e\u6587\u4ef6\u3002\u5bfc\u5165\u4e0d\u8bba\u6210\u529f\u6216\u5931\u8d25\uff0c\u90fd\u5c06\u5220\u9664\u5df2\u4e0a\u4f20\u6587\u4ef6\u3002\u6570\u636e\u5bfc\u5165\u8bf7\u6c42\u5728server\u4e2d\u5b9e\u73b0\u4e3a\u4e00\u4e2a\u5f02\u6b65\u4efb\u52a1\uff0c\u54cd\u5e94\u8fd4\u56de\u5e76\u4e0d\u610f\u5473\u7740\u5bfc\u5165\u5df2\u5b8c\u6210\uff0c\u8fd4\u56de\u7684\u662f\u4efb\u52a1id\uff0c\u540e\u7eed\u53ef\u4ee5\u901a\u8fc7\u6b64\u4efb\u52a1id\u67e5\u8be2\u5bfc\u5165\u8fdb\u5ea6"}),"\n",(0,l.jsx)(n.h4,{id:"781-url",children:"7.8.1 URL"}),"\n",(0,l.jsx)(n.p,{children:"http://${ip}:${rpc_port}/LGraphHttpService/Query/import_data"}),"\n",(0,l.jsx)(n.h4,{id:"782-request",children:"7.8.2 REQUEST"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(n.table,{children:[(0,l.jsx)(n.thead,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"body\u53c2\u6570"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u8bf4\u660e"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u7c7b\u578b"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,l.jsxs)(n.tbody,{children:[(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"graph"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5bfc\u5165\u76ee\u6807\u5b50\u56fe"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"schema"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5bfc\u5165schema\u63cf\u8ff0"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"json\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"delimiter"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5206\u9694\u7b26"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"continueOnError"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5355\u884c\u6570\u636e\u51fa\u9519\u662f\u5426\u8df3\u8fc7\u9519\u8bef\u5e76\u7ee7\u7eed"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"boolean"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5426"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"skipPackages"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u8df3\u8fc7\u7684\u5305\u4e2a\u6570"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5426"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"taskId"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u4efb\u52a1id"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5426"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"other"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5176\u4ed6\u53c2\u6570"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"json\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5426"})]})]})]}),"\n",(0,l.jsx)(n.h4,{id:"783-response",children:"7.8.3 RESPONSE"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(n.table,{children:[(0,l.jsx)(n.thead,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"body\u53c2\u6570"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u8bf4\u660e"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u7c7b\u578b"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,l.jsx)(n.tbody,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"taskId"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u4efb\u52a1\u7f16\u53f7"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]})})]}),"\n",(0,l.jsx)(n.h3,{id:"79-\u6e05\u7406\u7528\u6237\u4e0a\u4f20\u6587\u4ef6\u8bf7\u6c42",children:"7.9 \u6e05\u7406\u7528\u6237\u4e0a\u4f20\u6587\u4ef6\u8bf7\u6c42"}),"\n",(0,l.jsx)(n.p,{children:"\u7528\u6237\u901a\u8fc7\u6b64\u7c7b\u8bf7\u6c42\u6e05\u7406\u5df2\u7ecf\u4e0a\u4f20\u7684\u6587\u4ef6\uff0c\u5982\u679c\u62ff\u5230\u8fd4\u56deerrorCode\u4e3a200\u7684\u54cd\u5e94\u62a5\u6587\u5373\u4e3a\u6b63\u5e38\u6e05\u7406"}),"\n",(0,l.jsx)(n.h4,{id:"791-url",children:"7.9.1 URL"}),"\n",(0,l.jsx)(n.p,{children:"http://${ip}:${rpc_port}/LGraphHttpService/Query/clear_cache"}),"\n",(0,l.jsx)(n.h4,{id:"792-request",children:"7.9.2 REQUEST"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(n.table,{children:[(0,l.jsx)(n.thead,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"body\u53c2\u6570"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u8bf4\u660e"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u7c7b\u578b"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,l.jsxs)(n.tbody,{children:[(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"fileName"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u6587\u4ef6\u540d\u79f0"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5426"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"userName"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u7528\u6237\u540d\u79f0"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5426"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"flag"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u6807\u8bb0\u4f4d\uff0c flag = 0\u65f6\u5220\u9664fileName\u6307\u5b9a\u6587\u4ef6, flag = 1\u65f6\u5220\u9664userName\u6307\u5b9a\u7528\u6237\u5df2\u7ecf\u4e0a\u4f20\u7684\u6240\u6709\u6587\u4ef6\uff0c flag = 2\u65f6\u5220\u9664\u6240\u6709\u7528\u6237\u4e0a\u4f20\u7684\u6587\u4ef6"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]})]})]}),"\n",(0,l.jsx)(n.h3,{id:"710-\u5bfc\u5165\u8fdb\u5ea6\u67e5\u8be2\u8bf7\u6c42",children:"7.10. \u5bfc\u5165\u8fdb\u5ea6\u67e5\u8be2\u8bf7\u6c42"}),"\n",(0,l.jsx)(n.p,{children:"\u7528\u6237\u901a\u8fc7\u6b64\u7c7b\u8bf7\u6c42\u83b7\u5f97\u5bfc\u5165\u4efb\u52a1\u7684\u72b6\u6001"}),"\n",(0,l.jsx)(n.h4,{id:"7101-url",children:"7.10.1 URL"}),"\n",(0,l.jsx)(n.p,{children:"http://${ip}:${rpc_port}/LGraphHttpService/Query/import_progress"}),"\n",(0,l.jsx)(n.h4,{id:"7102-request",children:"7.10.2 REQUEST"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(n.table,{children:[(0,l.jsx)(n.thead,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"body\u53c2\u6570"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u8bf4\u660e"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u7c7b\u578b"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,l.jsx)(n.tbody,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"taskId"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u4efb\u52a1\u7f16\u53f7"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]})})]}),"\n",(0,l.jsx)(n.h4,{id:"7103-response",children:"7.10.3 RESPONSE"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(n.table,{children:[(0,l.jsx)(n.thead,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"body\u53c2\u6570"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u8bf4\u660e"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u53c2\u6570\u7c7b\u578b"}),(0,l.jsx)(n.th,{style:{textAlign:"center"},children:"\u662f\u5426\u5fc5\u586b"})]})}),(0,l.jsxs)(n.tbody,{children:[(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"state"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u72b6\u6001\u6807\u8bb0\uff0c\u4e3a0\u8868\u793a\u51c6\u5907\u5bfc\u5165\uff0c\u4e3a1\u8868\u793a\u5bfc\u5165\u4e2d\uff0c\u4e3a2\u8868\u793a\u5bfc\u5165\u6210\u529f\uff0c\u4e3a3\u8868\u793a\u5bfc\u5165\u5931\u8d25"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u662f"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"progress"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5bfc\u5165\u8fdb\u5ea6\uff0cstate\u4e3a1\u65f6\u5305\u542b"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5426"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"reason"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5931\u8d25\u539f\u56e0\uff0cstate\u4e3a3\u65f6\u5305\u542b"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5b57\u7b26\u4e32"}),(0,l.jsx)(n.td,{style:{textAlign:"center"},children:"\u5426"})]})]})]})]})}function j(e={}){const{wrapper:n}={...(0,r.R)(),...e.components};return n?(0,l.jsx)(n,{...e,children:(0,l.jsx)(x,{...e})}):x(e)}}}]);