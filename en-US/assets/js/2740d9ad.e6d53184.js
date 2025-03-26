"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[89716],{28453:(e,n,s)=>{s.d(n,{R:()=>o,x:()=>d});var i=s(96540);const l={},r=i.createContext(l);function o(e){const n=i.useContext(r);return i.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function d(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(l):e.components||l:o(e.components),i.createElement(r.Provider,{value:n},e.children)}},96371:(e,n,s)=>{s.r(n),s.d(n,{assets:()=>c,contentTitle:()=>o,default:()=>a,frontMatter:()=>r,metadata:()=>d,toc:()=>h});var i=s(74848),l=s(28453);const r={},o="\u65e5\u5fd7\u4fe1\u606f",d={id:"permission/log",title:"\u65e5\u5fd7\u4fe1\u606f",description:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph \u7684\u65e5\u5fd7\u529f\u80fd\u3002",source:"@site/versions/version-4.5.1/zh-CN/source/10.permission/5.log.md",sourceDirName:"10.permission",slug:"/permission/log",permalink:"/tugraph-db/en-US/zh/4.5.1/permission/log",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:5,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"\u8fd0\u7ef4\u76d1\u63a7",permalink:"/tugraph-db/en-US/zh/4.5.1/permission/monitoring"},next:{title:"\u5355\u5143\u6d4b\u8bd5",permalink:"/tugraph-db/en-US/zh/4.5.1/quality/unit-testing"}},c={},h=[{value:"1.\u7b80\u4ecb",id:"1\u7b80\u4ecb",level:2},{value:"2.\u670d\u52a1\u5668\u65e5\u5fd7",id:"2\u670d\u52a1\u5668\u65e5\u5fd7",level:2},{value:"2.1.\u670d\u52a1\u5668\u65e5\u5fd7\u914d\u7f6e\u9879",id:"21\u670d\u52a1\u5668\u65e5\u5fd7\u914d\u7f6e\u9879",level:3},{value:"2.2.\u670d\u52a1\u5668\u65e5\u5fd7\u8f93\u51fa\u5b8f\u4f7f\u7528\u793a\u4f8b",id:"22\u670d\u52a1\u5668\u65e5\u5fd7\u8f93\u51fa\u5b8f\u4f7f\u7528\u793a\u4f8b",level:3},{value:"2.3.\u5b58\u50a8\u8fc7\u7a0b\u65e5\u5fd7",id:"23\u5b58\u50a8\u8fc7\u7a0b\u65e5\u5fd7",level:3},{value:"2.3.1.cpp\u5b58\u50a8\u8fc7\u7a0b",id:"231cpp\u5b58\u50a8\u8fc7\u7a0b",level:4},{value:"2.3.1.python\u5b58\u50a8\u8fc7\u7a0b",id:"231python\u5b58\u50a8\u8fc7\u7a0b",level:4},{value:"3.\u5ba1\u8ba1\u65e5\u5fd7",id:"3\u5ba1\u8ba1\u65e5\u5fd7",level:2}];function t(e){const n={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",h4:"h4",header:"header",p:"p",pre:"pre",...(0,l.R)(),...e.components};return(0,i.jsxs)(i.Fragment,{children:[(0,i.jsx)(n.header,{children:(0,i.jsx)(n.h1,{id:"\u65e5\u5fd7\u4fe1\u606f",children:"\u65e5\u5fd7\u4fe1\u606f"})}),"\n",(0,i.jsxs)(n.blockquote,{children:["\n",(0,i.jsx)(n.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph \u7684\u65e5\u5fd7\u529f\u80fd\u3002"}),"\n"]}),"\n",(0,i.jsx)(n.h2,{id:"1\u7b80\u4ecb",children:"1.\u7b80\u4ecb"}),"\n",(0,i.jsx)(n.p,{children:"TuGraph \u4fdd\u7559\u4e24\u79cd\u7c7b\u578b\u7684\u65e5\u5fd7\uff1a\u670d\u52a1\u5668\u65e5\u5fd7\u548c\u5ba1\u8ba1\u65e5\u5fd7\u3002\u670d\u52a1\u5668\u65e5\u5fd7\u8bb0\u5f55\u4eba\u4e3a\u53ef\u8bfb\u7684\u670d\u52a1\u5668\u72b6\u6001\u4fe1\u606f\uff0c\u800c\u5ba1\u6838\u65e5\u5fd7\u7ef4\u62a4\u670d\u52a1\u5668\u4e0a\u6267\u884c\u7684\u6bcf\u4e2a\u64cd\u4f5c\u52a0\u5bc6\u540e\u7684\u4fe1\u606f\u3002"}),"\n",(0,i.jsx)(n.h2,{id:"2\u670d\u52a1\u5668\u65e5\u5fd7",children:"2.\u670d\u52a1\u5668\u65e5\u5fd7"}),"\n",(0,i.jsx)(n.h3,{id:"21\u670d\u52a1\u5668\u65e5\u5fd7\u914d\u7f6e\u9879",children:"2.1.\u670d\u52a1\u5668\u65e5\u5fd7\u914d\u7f6e\u9879"}),"\n",(0,i.jsxs)(n.p,{children:["\u670d\u52a1\u5668\u65e5\u5fd7\u7684\u8f93\u51fa\u4f4d\u7f6e\u53ef\u4ee5\u901a\u8fc7",(0,i.jsx)(n.code,{children:"log_dir"}),"\u914d\u7f6e\u6307\u5b9a\u3002\u670d\u52a1\u5668\u65e5\u5fd7\u8be6\u7ec6\u7a0b\u5ea6\u53ef\u901a\u8fc7",(0,i.jsx)(n.code,{children:"verbose"}),"\u914d\u7f6e\u9879\u6307\u5b9a\u3002"]}),"\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"log_dir"}),"\u914d\u7f6e\u9879\u9ed8\u8ba4\u4e3a\u7a7a\u3002\u82e5",(0,i.jsx)(n.code,{children:"log_dir"}),"\u914d\u7f6e\u9879\u4e3a\u7a7a\uff0c\u5219\u6240\u6709\u65e5\u5fd7\u4f1a\u8f93\u51fa\u5230\u63a7\u5236\u53f0(daemon\u6a21\u5f0f\u4e0b\u82e5log_dir\u914d\u7f6e\u9879\u4e3a\u7a7a\u5219\u4e0d\u4f1a\u5411console\u8f93\u51fa\u4efb\u4f55\u65e5\u5fd7)\uff1b\u82e5\u624b\u52a8\u6307\u5b9a",(0,i.jsx)(n.code,{children:"log_dir"}),"\u914d\u7f6e\u9879\uff0c\u5219\u65e5\u5fd7\u6587\u4ef6\u4f1a\u751f\u6210\u5728\u5bf9\u5e94\u7684\u8def\u5f84\u4e0b\u9762\u3002\u5355\u4e2a\u65e5\u5fd7\u6587\u4ef6\u6700\u5927\u5927\u5c0f\u4e3a256MB\u3002"]}),"\n",(0,i.jsxs)(n.p,{children:[(0,i.jsx)(n.code,{children:"verbose"}),"\u914d\u7f6e\u9879\u63a7\u5236\u65e5\u5fd7\u7684\u8be6\u7ec6\u7a0b\u5ea6\uff0c\u4ece\u7c97\u5230\u7ec6\u5206\u4e3a",(0,i.jsx)(n.code,{children:"0, 1, 2"}),"\u4e09\u4e2a\u7b49\u7ea7\uff0c\u9ed8\u8ba4\u7b49\u7ea7\u4e3a",(0,i.jsx)(n.code,{children:"1"}),"\u3002\u7b49\u7ea7\u4e3a",(0,i.jsx)(n.code,{children:"2"}),"\u65f6\uff0c\u65e5\u5fd7\u8bb0\u5f55\u6700\u8be6\u7ec6\uff0c\u670d\u52a1\u5668\u5c06\u6253\u5370",(0,i.jsx)(n.code,{children:"DEBUG"}),"\u53ca\u4ee5\u4e0a\u7b49\u7ea7\u7684\u5168\u90e8\u65e5\u5fd7\u4fe1\u606f\uff1b\u7b49\u7ea7\u4e3a",(0,i.jsx)(n.code,{children:"1"}),"\u65f6\uff0c\u670d\u52a1\u5668\u5c06\u4ec5\u6253\u5370",(0,i.jsx)(n.code,{children:"INFO"}),"\u7b49\u7ea7\u53ca\u4ee5\u4e0a\u7684\u4e3b\u8981\u4e8b\u4ef6\u7684\u65e5\u5fd7\uff1b\u7b49\u7ea7\u4e3a",(0,i.jsx)(n.code,{children:"0"}),"\u65f6\uff0c\u670d\u52a1\u5668\u5c06\u4ec5\u6253\u5370",(0,i.jsx)(n.code,{children:"ERROR"}),"\u7b49\u7ea7\u53ca\u4ee5\u4e0a\u7684\u9519\u8bef\u65e5\u5fd7\u3002"]}),"\n",(0,i.jsx)(n.h3,{id:"22\u670d\u52a1\u5668\u65e5\u5fd7\u8f93\u51fa\u5b8f\u4f7f\u7528\u793a\u4f8b",children:"2.2.\u670d\u52a1\u5668\u65e5\u5fd7\u8f93\u51fa\u5b8f\u4f7f\u7528\u793a\u4f8b"}),"\n",(0,i.jsx)(n.p,{children:"\u5982\u679c\u5f00\u53d1\u8005\u5728\u5f00\u53d1\u8fc7\u7a0b\u4e2d\u5e0c\u671b\u5728\u4ee3\u7801\u4e2d\u6dfb\u52a0\u65e5\u5fd7\uff0c\u53ef\u4ee5\u53c2\u8003\u5982\u4e0b\u793a\u4f8b"}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{children:'#include "tools/lgraph_log.h" //\u6dfb\u52a0\u65e5\u5fd7\u4f9d\u8d56\n\n\nvoid LogExample() {\n    // \u6570\u636e\u5e93\u542f\u52a8\u9636\u6bb5\u5df2\u7ecf\u5bf9\u65e5\u5fd7\u6a21\u5757\u8fdb\u884c\u4e86\u521d\u59cb\u5316\uff0c\u5f00\u53d1\u8005\u53ea\u9700\u76f4\u63a5\u8c03\u7528\u5b8f\u5373\u53ef\n    // \u65e5\u5fd7\u7b49\u7ea7\u5206\u4e3aDEBUG, INFO, WARNING, ERROR, FATAL\u4e94\u4e2a\u7b49\u7ea7\n    LOG_DEBUG() << "This is a debug level log message.";\n    LOG_INFO() << "This is a info level log message.";\n    LOG_WARN() << "This is a warning level log message.";\n    LOG_ERROR() << "This is a error level log message.";\n    LOG_FATAL() << "This is a fatal level log message.";\n}\n'})}),"\n",(0,i.jsx)(n.p,{children:"\u66f4\u591a\u7528\u6cd5\u53ef\u4ee5\u53c2\u8003test/test_lgraph_log.cpp\u4e2d\u7684\u65e5\u5fd7\u5b8f\u7684\u4f7f\u7528\u65b9\u6cd5"}),"\n",(0,i.jsx)(n.h3,{id:"23\u5b58\u50a8\u8fc7\u7a0b\u65e5\u5fd7",children:"2.3.\u5b58\u50a8\u8fc7\u7a0b\u65e5\u5fd7"}),"\n",(0,i.jsxs)(n.p,{children:["\u7528\u6237\u5728\u5b58\u50a8\u8fc7\u7a0b\u7684\u7f16\u5199\u8fc7\u7a0b\u4e2d\u53ef\u4ee5\u4f7f\u7528\u65e5\u5fd7\u529f\u80fd\u5c06\u6240\u9700\u7684\u8c03\u8bd5\u4fe1\u606f\u8f93\u51fa\u5230\u65e5\u5fd7\u4e2d\u8fdb\u884c\u67e5\u770b\uff0c\u8f85\u52a9\u5f00\u53d1\u3002\u8c03\u8bd5\u4fe1\u606f\u4f1a\u8f93\u51fa\u5230\u4e0e\u670d\u52a1\u5668\u65e5\u5fd7\u76f8\u540c\u7684\u65e5\u5fd7\u6587\u4ef6\u4e2d(\u5982\u672a\u6307\u5b9a",(0,i.jsx)(n.code,{children:"log_dir"}),"\u5219\u540c\u6837\u8f93\u51fa\u81f3console)"]}),"\n",(0,i.jsx)(n.h4,{id:"231cpp\u5b58\u50a8\u8fc7\u7a0b",children:"2.3.1.cpp\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,i.jsxs)(n.p,{children:["\u8bf7\u4f7f\u75282.2\u4e2d\u63d0\u4f9b\u7684log\u5b8f\u8f93\u51fa\u8c03\u8bd5\u4fe1\u606f\uff0c\u907f\u514d\u4f7f\u7528cout\u6216\u8005printf\u7b49\u8f93\u51fa\u65b9\u5f0f\u3002\u5177\u4f53\u4f7f\u7528\u65b9\u5f0f\u53ef\u53c2\u8003\u5982\u4e0b\u793a\u4f8b\u4ee3\u7801\uff08\u8be6\u89c1",(0,i.jsx)(n.code,{children:"procedures/demo/log_demo.cpp"}),"\uff09"]}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{children:'#include <stdlib.h>\n#include "lgraph/lgraph.h"\n#include "tools/lgraph_log.h"  // add log dependency\nusing namespace lgraph_api;\n\nvoid LogExample() {\n    LOG_DEBUG() << "This is a debug level log message.";\n    LOG_INFO() << "This is a info level log message.";\n    LOG_WARN() << "This is a warning level log message.";\n    LOG_ERROR() << "This is a error level log message.";\n}\n\nextern "C" bool Process(GraphDB& db, const std::string& request, std::string& response) {\n    response = "TuGraph log demo";\n    LogExample();\n    return true;\n}\n'})}),"\n",(0,i.jsx)(n.p,{children:"\u5c06\u4ee5\u4e0a\u793a\u4f8b\u4ee3\u7801\u4f5c\u4e3a\u5b58\u50a8\u8fc7\u7a0b\u63d2\u5165\u6570\u636e\u5e93\u5e76\u8fd0\u884c\u540e\uff0c\u53ef\u4ee5\u5728\u65e5\u5fd7\u6587\u4ef6\u4e2d\u770b\u5230\u76f8\u5e94\u7684\u65e5\u5fd7\u6761\u76ee\u3002"}),"\n",(0,i.jsx)(n.h4,{id:"231python\u5b58\u50a8\u8fc7\u7a0b",children:"2.3.1.python\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,i.jsx)(n.p,{children:"\u8bf7\u4f7f\u7528python\u81ea\u5e26\u7684print\u8f93\u51fa\u8c03\u8bd5\u4fe1\u606f\uff0c\u8c03\u8bd5\u4fe1\u606f\u4f1a\u5728\u5b58\u50a8\u8fc7\u7a0b\u8fd0\u884c\u7ed3\u675f\u540e\u5408\u5e76\u4e3a\u4e00\u6761WARN\u7b49\u7ea7\u7684\u65e5\u5fd7\u6761\u76ee\u8f93\u51fa\u81f3\u65e5\u5fd7\u6587\u4ef6\u4e2d\u3002"}),"\n",(0,i.jsx)(n.h2,{id:"3\u5ba1\u8ba1\u65e5\u5fd7",children:"3.\u5ba1\u8ba1\u65e5\u5fd7"}),"\n",(0,i.jsx)(n.p,{children:"\u5ba1\u6838\u65e5\u5fd7\u8bb0\u5f55\u6bcf\u4e2a\u8bf7\u6c42\u548c\u54cd\u5e94\uff0c\u4ee5\u53ca\u53d1\u9001\u8bf7\u6c42\u7684\u7528\u6237\u4ee5\u53ca\u6536\u5230\u8bf7\u6c42\u7684\u65f6\u95f4\u3002\u5ba1\u6838\u65e5\u5fd7\u53ea\u80fd\u662f\u6253\u5f00\u6216\u5173\u95ed\u72b6\u6001\u3002\u53ef\u4ee5\u4f7f\u7528 TuGraph \u53ef\u89c6\u5316\u5de5\u5177\u548c REST API \u67e5\u8be2\u7ed3\u679c\u3002"}),"\n",(0,i.jsxs)(n.p,{children:["\u5f00\u542f\u5ba1\u8ba1\u65e5\u5fd7\u9700\u8981\u5728\u914d\u7f6e\u6587\u4ef6\u4e2d\u5c06",(0,i.jsx)(n.code,{children:"enable_audit_log"}),"\u53c2\u6570\u8bbe\u7f6e\u4e3a",(0,i.jsx)(n.code,{children:"true"}),"\u3002\u914d\u7f6e\u6587\u4ef6\u548c\u914d\u7f6e\u53c2\u6570\u8bf4\u660e\u8be6\u89c1\uff1a",(0,i.jsx)(n.a,{href:"../../5.installation&running/7.tugraph-running.md",children:"\u6570\u636e\u5e93\u8fd0\u884c/\u670d\u52a1\u914d\u7f6e"}),"\u3002"]})]})}function a(e={}){const{wrapper:n}={...(0,l.R)(),...e.components};return n?(0,i.jsx)(n,{...e,children:(0,i.jsx)(t,{...e})}):t(e)}}}]);