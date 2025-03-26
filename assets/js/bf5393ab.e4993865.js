"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[29086],{28453:(e,l,n)=>{n.d(l,{R:()=>s,x:()=>c});var r=n(96540);const o={},d=r.createContext(o);function s(e){const l=r.useContext(d);return r.useMemo((function(){return"function"==typeof e?e(l):{...l,...e}}),[l,e])}function c(e){let l;return l=e.disableParentContext?"function"==typeof e.components?e.components(o):e.components||o:s(e.components),r.createElement(d.Provider,{value:l},e.children)}},49317:(e,l,n)=>{n.r(l),n.d(l,{assets:()=>i,contentTitle:()=>s,default:()=>u,frontMatter:()=>d,metadata:()=>c,toc:()=>h});var r=n(74848),o=n(28453);const d={},s="\u65e5\u5fd7\u4fe1\u606f",c={id:"developer-manual/ecosystem-tools/log",title:"\u65e5\u5fd7\u4fe1\u606f",description:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph \u7684\u65e5\u5fd7\u529f\u80fd\u3002",source:"@site/versions/version-3.6.0/zh-CN/source/5.developer-manual/5.ecosystem-tools/4.log.md",sourceDirName:"5.developer-manual/5.ecosystem-tools",slug:"/developer-manual/ecosystem-tools/log",permalink:"/tugraph-db/zh/3.6.0/developer-manual/ecosystem-tools/log",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:4,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"TuGraph-Explorer",permalink:"/tugraph-db/zh/3.6.0/developer-manual/ecosystem-tools/tugraph-explorer"},next:{title:"TuGraph-Restful-Server",permalink:"/tugraph-db/zh/3.6.0/developer-manual/ecosystem-tools/restful"}},i={},h=[{value:"1.\u7b80\u4ecb",id:"1\u7b80\u4ecb",level:2},{value:"2.\u670d\u52a1\u5668\u65e5\u5fd7",id:"2\u670d\u52a1\u5668\u65e5\u5fd7",level:2},{value:"2.1.\u670d\u52a1\u5668\u65e5\u5fd7\u5206\u7c7b",id:"21\u670d\u52a1\u5668\u65e5\u5fd7\u5206\u7c7b",level:3},{value:"2.2.\u670d\u52a1\u5668\u65e5\u5fd7\u914d\u7f6e\u9879",id:"22\u670d\u52a1\u5668\u65e5\u5fd7\u914d\u7f6e\u9879",level:3},{value:"2.3.\u670d\u52a1\u5668\u65e5\u5fd7\u8f93\u51fa\u5b8f\u4f7f\u7528\u793a\u4f8b",id:"23\u670d\u52a1\u5668\u65e5\u5fd7\u8f93\u51fa\u5b8f\u4f7f\u7528\u793a\u4f8b",level:3},{value:"3.\u5ba1\u8ba1\u65e5\u5fd7",id:"3\u5ba1\u8ba1\u65e5\u5fd7",level:2}];function t(e){const l={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",p:"p",pre:"pre",...(0,o.R)(),...e.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(l.header,{children:(0,r.jsx)(l.h1,{id:"\u65e5\u5fd7\u4fe1\u606f",children:"\u65e5\u5fd7\u4fe1\u606f"})}),"\n",(0,r.jsxs)(l.blockquote,{children:["\n",(0,r.jsx)(l.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph \u7684\u65e5\u5fd7\u529f\u80fd\u3002"}),"\n"]}),"\n",(0,r.jsx)(l.h2,{id:"1\u7b80\u4ecb",children:"1.\u7b80\u4ecb"}),"\n",(0,r.jsx)(l.p,{children:"TuGraph \u4fdd\u7559\u4e24\u79cd\u7c7b\u578b\u7684\u65e5\u5fd7\uff1a\u670d\u52a1\u5668\u65e5\u5fd7\u548c\u5ba1\u8ba1\u65e5\u5fd7\u3002\u670d\u52a1\u5668\u65e5\u5fd7\u8bb0\u5f55\u4eba\u4e3a\u53ef\u8bfb\u7684\u670d\u52a1\u5668\u72b6\u6001\u4fe1\u606f\uff0c\u800c\u5ba1\u6838\u65e5\u5fd7\u7ef4\u62a4\u670d\u52a1\u5668\u4e0a\u6267\u884c\u7684\u6bcf\u4e2a\u64cd\u4f5c\u52a0\u5bc6\u540e\u7684\u4fe1\u606f\u3002"}),"\n",(0,r.jsx)(l.h2,{id:"2\u670d\u52a1\u5668\u65e5\u5fd7",children:"2.\u670d\u52a1\u5668\u65e5\u5fd7"}),"\n",(0,r.jsx)(l.h3,{id:"21\u670d\u52a1\u5668\u65e5\u5fd7\u5206\u7c7b",children:"2.1.\u670d\u52a1\u5668\u65e5\u5fd7\u5206\u7c7b"}),"\n",(0,r.jsxs)(l.p,{children:["\u5f53\u524d\u670d\u52a1\u5668\u65e5\u5fd7\u5206\u4e3a\u4e09\u7c7b\uff0c",(0,r.jsx)(l.code,{children:"general log"}),"\uff0c",(0,r.jsx)(l.code,{children:"debug log"}),"\u4e0e",(0,r.jsx)(l.code,{children:"query log"}),"\u3002"]}),"\n",(0,r.jsxs)(l.p,{children:[(0,r.jsx)(l.code,{children:"general log"}),"\u9762\u5411\u666e\u901a\u7528\u6237\uff0c\u5305\u542b\u7528\u6237\u5173\u6ce8\u7684\u6570\u636e\u5e93\u542f\u52a8\u5173\u95ed\u65f6\u7684\u5173\u952e\u4fe1\u606f\uff0c\u5982\u6570\u636e\u5e93\u542f\u52a8\u65f6\u7684\u914d\u7f6e\u4fe1\u606f\uff0c\u670d\u52a1\u76d1\u542c\u7684\u7aef\u53e3\u4ee5\u53ca\u662f\u5426\u6210\u529f\u542f\u52a8\u6216\u5173\u95ed\u7b49\u4fe1\u606f\uff0c\u53ef\u4ee5\u76f4\u89c2\u7684\u770b\u5230\u6570\u636e\u5e93\u662f\u5426\u6b63\u5728\u8fd0\u884c\u3002"]}),"\n",(0,r.jsxs)(l.p,{children:[(0,r.jsx)(l.code,{children:"debug log"}),"\u9762\u5411\u6570\u636e\u5e93\u5f00\u53d1\u8005\uff0c\u5305\u542b\u5f00\u53d1\u8005\u5173\u6ce8\u7684\u6570\u636e\u5e93\u8fd0\u884c\u8fc7\u7a0b\u4e2d\u4ea7\u751f\u7684debug\u76f8\u5173\u4fe1\u606f\uff0c\u5982\u6570\u636e\u5e93\u6536\u5230\u7684\u7f51\u7edc\u8bf7\u6c42\uff0cquery\u7684\u6267\u884c\u8fc7\u7a0b\u7b49\uff0c\u53ef\u4ee5\u534f\u52a9\u5f00\u53d1\u8005\u8fdb\u884c\u5f00\u53d1\uff0c\u5e76\u53ef\u4ee5\u901a\u8fc7",(0,r.jsx)(l.code,{children:"verbose"}),"\u914d\u7f6e\u9879\u63a7\u5236\u65e5\u5fd7\u8be6\u7ec6\u7a0b\u5ea6\u3002"]}),"\n",(0,r.jsxs)(l.p,{children:["\uff08\u5f85\u5b9e\u73b0\uff09",(0,r.jsx)(l.code,{children:"query log"}),"\u9762\u5411\u4e1a\u52a1\u5f00\u53d1\u4eba\u5458\uff0c\u5305\u542b\u4e1a\u52a1\u5f00\u53d1\u8005\u5173\u6ce8\u7684\u6bcf\u4e00\u6b21\u6570\u636e\u5e93\u6267\u884c\u7684query\u8bed\u53e5\u4ee5\u53ca\u6267\u884c\u8be5\u8bed\u53e5\u65f6\u6570\u636e\u5e93\u7684\u6027\u80fd\u76f8\u5173\u4fe1\u606f\uff0c\u5982\u67e5\u8be2\u65f6\u95f4\u7b49\u3002\u53ef\u4ee5\u901a\u8fc7\u8bbe\u5b9a\u65f6\u95f4\u9608\u503c\u7684\u65b9\u5f0f\u8fc7\u6ee4\u6267\u884c\u65f6\u95f4\u4f4e\u4e8e\u6307\u5b9a\u503c\u7684query\uff0c\u65b9\u4fbf\u4e1a\u52a1\u5f00\u53d1\u8005\u8fdb\u884c\u6027\u80fd\u5206\u6790\u3002"]}),"\n",(0,r.jsx)(l.h3,{id:"22\u670d\u52a1\u5668\u65e5\u5fd7\u914d\u7f6e\u9879",children:"2.2.\u670d\u52a1\u5668\u65e5\u5fd7\u914d\u7f6e\u9879"}),"\n",(0,r.jsxs)(l.p,{children:["\u670d\u52a1\u5668\u65e5\u5fd7\u7684\u8f93\u51fa\u4f4d\u7f6e\u53ef\u4ee5\u901a\u8fc7",(0,r.jsx)(l.code,{children:"log_dir"}),"\u914d\u7f6e\u6307\u5b9a\u3002\u670d\u52a1\u5668\u65e5\u5fd7\u4e2d",(0,r.jsx)(l.code,{children:"debug log"}),"\u7684\u8be6\u7ec6\u7a0b\u5ea6\u53ef\u901a\u8fc7",(0,r.jsx)(l.code,{children:"verbose"}),"\u914d\u7f6e\u9879\u6307\u5b9a\u3002"]}),"\n",(0,r.jsxs)(l.p,{children:[(0,r.jsx)(l.code,{children:"log_dir"}),"\u914d\u7f6e\u9879\u9ed8\u8ba4\u4e3a\u7a7a\u3002\u82e5",(0,r.jsx)(l.code,{children:"log_dir"}),"\u4e3a\u7a7a\uff0c\u5219\u6240\u6709\u65e5\u5fd7\u4f1a\u8f93\u51fa\u5230\u63a7\u5236\u53f0\u3002\u82e5\u624b\u52a8\u6307\u5b9a",(0,r.jsx)(l.code,{children:"log_dir"}),"\uff0c\u5219\u4f1a\u5728\u5bf9\u5e94\u8def\u5f84\u4e0b\u751f\u6210\u5982\u4e0b\u65e5\u5fd7\u6587\u4ef6\u5939\u7ed3\u6784\u3002"]}),"\n",(0,r.jsx)(l.pre,{children:(0,r.jsx)(l.code,{children:".\n\u251c\u2500\u2500 debug.log\n\u251c\u2500\u2500 general.log\n\u251c\u2500\u2500 query.log(TODO)\n\u2514\u2500\u2500 history_logs\n    \u251c\u2500\u2500 debug_logs\n    \u251c\u2500\u2500 general_logs\n    \u2514\u2500\u2500 query_logs(TODO)\n"})}),"\n",(0,r.jsxs)(l.p,{children:[(0,r.jsx)(l.code,{children:"general log"}),"\u65e5\u5fd7\u8bb0\u5f55\u5728",(0,r.jsx)(l.code,{children:"genreal.log"}),"\u6587\u4ef6\u4e2d\u3002",(0,r.jsx)(l.code,{children:"debug log"}),"\u65e5\u5fd7\u8bb0\u5f55\u5728",(0,r.jsx)(l.code,{children:"debug.log"}),"\u6587\u4ef6\u4e2d\u3002",(0,r.jsx)(l.code,{children:"query log"}),"\u65e5\u5fd7\u8bb0\u5f55\u5728",(0,r.jsx)(l.code,{children:"query.log"}),"\u6587\u4ef6\u4e2d\u3002\n\u5355\u4e2a\u65e5\u5fd7\u6587\u4ef6\u6700\u5927\u5927\u5c0f\u4e3a64mb\uff0c\u5728\u8fbe\u5230\u89c4\u5b9a\u7684\u6700\u5927\u6587\u4ef6\u5927\u5c0f\u540e\u4f1a\u88ab\u56de\u6536\u5165",(0,r.jsx)(l.code,{children:"history_logs"}),"\u6587\u4ef6\u5939\u4e2d\uff0c",(0,r.jsx)(l.code,{children:"general.log"}),"\u56de\u6536\u8fdb\u5165",(0,r.jsx)(l.code,{children:"general_logs"}),"\u6587\u4ef6\u5939\uff0c",(0,r.jsx)(l.code,{children:"debug.log"}),"\u56de\u6536\u8fdb\u5165",(0,r.jsx)(l.code,{children:"debug_logs"}),"\u6587\u4ef6\u5939\u4e2d\u3002"]}),"\n",(0,r.jsxs)(l.p,{children:[(0,r.jsx)(l.code,{children:"verbose"}),"\u914d\u7f6e\u9879\u63a7\u5236\u4e86",(0,r.jsx)(l.code,{children:"debug log"}),"\u7684\u8be6\u7ec6\u7a0b\u5ea6\uff0c\u4ece\u7c97\u5230\u7ec6\u5206\u4e3a",(0,r.jsx)(l.code,{children:"0, 1, 2"}),"\u4e09\u4e2a\u7b49\u7ea7\u3002\u9ed8\u8ba4\u7b49\u7ea7\u4e3a",(0,r.jsx)(l.code,{children:"2"}),"\uff0c\u6b64\u7b49\u7ea7\u4e0b\uff0c\u65e5\u5fd7\u8bb0\u5f55\u6700\u8be6\u7ec6\uff0c\u670d\u52a1\u5668\u5c06\u6253\u5370",(0,r.jsx)(l.code,{children:"DEBUG"}),"\u53ca\u4ee5\u4e0a\u7b49\u7ea7\u7684\u5168\u90e8\u65e5\u5fd7\u4fe1\u606f\uff0c\u540c\u65f6\u9644\u5e26\u4e0a\u6253\u5370\u8fd9\u6761\u65e5\u5fd7\u4fe1\u606f\u7684",(0,r.jsx)(l.code,{children:"[\u6587\u4ef6\u540d:\u51fd\u6570\u540d:\u884c\u53f7]"}),"\uff0c\u4fbf\u4e8edebug\u3002\u7b49\u7ea7\u4e3a",(0,r.jsx)(l.code,{children:"1"}),"\u65f6\uff0c\u670d\u52a1\u5668\u5c06\u4ec5\u6253\u5370",(0,r.jsx)(l.code,{children:"INFO"}),"\u7b49\u7ea7\u53ca\u4ee5\u4e0a\u7684\u4e3b\u8981\u4e8b\u4ef6\u7684\u65e5\u5fd7\u3002\u7b49\u7ea7\u4e3a",(0,r.jsx)(l.code,{children:"0"}),"\u65f6\uff0c\u670d\u52a1\u5668\u5c06\u4ec5\u6253\u5370",(0,r.jsx)(l.code,{children:"ERROR"}),"\u7b49\u7ea7\u53ca\u4ee5\u4e0a\u7684\u9519\u8bef\u65e5\u5fd7\u3002"]}),"\n",(0,r.jsx)(l.h3,{id:"23\u670d\u52a1\u5668\u65e5\u5fd7\u8f93\u51fa\u5b8f\u4f7f\u7528\u793a\u4f8b",children:"2.3.\u670d\u52a1\u5668\u65e5\u5fd7\u8f93\u51fa\u5b8f\u4f7f\u7528\u793a\u4f8b"}),"\n",(0,r.jsx)(l.p,{children:"\u5982\u679c\u5f00\u53d1\u8005\u5728\u5f00\u53d1\u8fc7\u7a0b\u4e2d\u5e0c\u671b\u5728\u4ee3\u7801\u4e2d\u6dfb\u52a0\u65e5\u5fd7\uff0c\u53ef\u4ee5\u53c2\u8003\u5982\u4e0b\u793a\u4f8b"}),"\n",(0,r.jsx)(l.pre,{children:(0,r.jsx)(l.code,{children:'#include "tools/lgraph_log.h" //\u6dfb\u52a0\u65e5\u5fd7\u4f9d\u8d56\n\n\nvoid LogExample() {\n    // \u6570\u636e\u5e93\u542f\u52a8\u9636\u6bb5\u5df2\u7ecf\u5bf9\u65e5\u5fd7\u6a21\u5757\u8fdb\u884c\u4e86\u521d\u59cb\u5316\uff0c\u5f00\u53d1\u8005\u53ea\u9700\u76f4\u63a5\u8c03\u7528\u5b8f\u5373\u53ef\n    // \u65e5\u5fd7\u7b49\u7ea7\u5206\u4e3aTRACE, DEBUG, INFO, WARNING, ERROR, FATAL\u516d\u4e2a\u7b49\u7ea7\n\n    GENERAL_LOG(INFO) << "This is a info level general log message."; // general log\u7684\u8f93\u51fa\u5b8f\n\n    DEBUG_LOG(ERROR) << "This is a error level debug log message."; // debug log\u7684\u8f93\u51fa\u5b8f\n}\n'})}),"\n",(0,r.jsx)(l.p,{children:"\u66f4\u591a\u7528\u6cd5\u53ef\u4ee5\u53c2\u8003test/test_lgraph_log,cpp\u4e2d\u7684\u65e5\u5fd7\u5b8f\u7684\u4f7f\u7528\u65b9\u6cd5"}),"\n",(0,r.jsx)(l.h2,{id:"3\u5ba1\u8ba1\u65e5\u5fd7",children:"3.\u5ba1\u8ba1\u65e5\u5fd7"}),"\n",(0,r.jsx)(l.p,{children:"\u5ba1\u6838\u65e5\u5fd7\u8bb0\u5f55\u6bcf\u4e2a\u8bf7\u6c42\u548c\u54cd\u5e94\uff0c\u4ee5\u53ca\u53d1\u9001\u8bf7\u6c42\u7684\u7528\u6237\u4ee5\u53ca\u6536\u5230\u8bf7\u6c42\u7684\u65f6\u95f4\u3002\u5ba1\u6838\u65e5\u5fd7\u53ea\u80fd\u662f\u6253\u5f00\u6216\u5173\u95ed\u72b6\u6001\u3002\u53ef\u4ee5\u4f7f\u7528 TuGraph \u53ef\u89c6\u5316\u5de5\u5177\u548c REST API \u67e5\u8be2\u7ed3\u679c\u3002"})]})}function u(e={}){const{wrapper:l}={...(0,o.R)(),...e.components};return l?(0,r.jsx)(l,{...e,children:(0,r.jsx)(t,{...e})}):t(e)}}}]);