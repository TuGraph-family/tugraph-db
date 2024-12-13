"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[115],{14997:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>i,contentTitle:()=>t,default:()=>h,frontMatter:()=>l,metadata:()=>c,toc:()=>a});var s=r(74848),d=r(28453);const l={},t="Procedure API",c={id:"olap&procedure/procedure/procedure",title:"Procedure API",description:"\u6b64\u6587\u6863\u4e3b\u8981\u8bb2\u89e3 TuGraph \u7684\u5b58\u50a8\u8fc7\u7a0b\u4f7f\u7528\u8bf4\u660e",source:"@site/versions/version-4.3.2/zh-CN/source/9.olap&procedure/1.procedure/1.procedure.md",sourceDirName:"9.olap&procedure/1.procedure",slug:"/olap&procedure/procedure/",permalink:"/tugraph-db/en-US/zh/4.3.2/olap&procedure/procedure/",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:1,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"ISO GQL",permalink:"/tugraph-db/en-US/zh/4.3.2/query/gql"},next:{title:"Traversal API",permalink:"/tugraph-db/en-US/zh/4.3.2/olap&procedure/procedure/traversal"}},i={},a=[{value:"1.\u7b80\u4ecb",id:"1\u7b80\u4ecb",level:2},{value:"2.\u5b58\u50a8\u8fc7\u7a0b\u7684\u7248\u672c\u652f\u6301",id:"2\u5b58\u50a8\u8fc7\u7a0b\u7684\u7248\u672c\u652f\u6301",level:2},{value:"3.\u5b58\u50a8\u8fc7\u7a0b\u8bed\u8a00\u652f\u6301",id:"3\u5b58\u50a8\u8fc7\u7a0b\u8bed\u8a00\u652f\u6301",level:2},{value:"4.Procedure v1\u63a5\u53e3",id:"4procedure-v1\u63a5\u53e3",level:2},{value:"4.1.\u7f16\u5199\u5b58\u50a8\u8fc7\u7a0b",id:"41\u7f16\u5199\u5b58\u50a8\u8fc7\u7a0b",level:2},{value:"4.1.1.\u7f16\u5199C++\u5b58\u50a8\u8fc7\u7a0b",id:"411\u7f16\u5199c\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"4.1.2.\u7f16\u5199Python\u5b58\u50a8\u8fc7\u7a0b",id:"412\u7f16\u5199python\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"4.2.\u5982\u4f55\u4f7f\u7528\u5b58\u50a8\u8fc7\u7a0b",id:"42\u5982\u4f55\u4f7f\u7528\u5b58\u50a8\u8fc7\u7a0b",level:2},{value:"4.2.1.\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b",id:"421\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"4.2.2.\u5217\u51fa\u5df2\u52a0\u8f7d\u7684\u5b58\u50a8\u8fc7\u7a0b",id:"422\u5217\u51fa\u5df2\u52a0\u8f7d\u7684\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"4.2.3.\u83b7\u53d6\u5b58\u50a8\u8fc7\u7a0b\u8be6\u60c5",id:"423\u83b7\u53d6\u5b58\u50a8\u8fc7\u7a0b\u8be6\u60c5",level:3},{value:"4.2.4.\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",id:"424\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"4.2.5.\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b",id:"425\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"4.2.6.\u66f4\u65b0\u5b58\u50a8\u8fc7\u7a0b",id:"426\u66f4\u65b0\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"5.Procedure v2\u63a5\u53e3",id:"5procedure-v2\u63a5\u53e3",level:2},{value:"5.1.\u7f16\u5199\u5b58\u50a8\u8fc7\u7a0b",id:"51\u7f16\u5199\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"5.2.\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b",id:"52\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"5.2.1.\u5217\u51fa\u5df2\u52a0\u8f7d\u7684\u5b58\u50a8\u8fc7\u7a0b",id:"521\u5217\u51fa\u5df2\u52a0\u8f7d\u7684\u5b58\u50a8\u8fc7\u7a0b",level:4},{value:"5.2.2.\u83b7\u53d6\u5b58\u50a8\u8fc7\u7a0b\u8be6\u60c5",id:"522\u83b7\u53d6\u5b58\u50a8\u8fc7\u7a0b\u8be6\u60c5",level:4},{value:"5.2.3.\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",id:"523\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",level:4},{value:"5.2.4.\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b",id:"524\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b",level:4},{value:"5.2.5.\u66f4\u65b0\u5b58\u50a8\u8fc7\u7a0b",id:"525\u66f4\u65b0\u5b58\u50a8\u8fc7\u7a0b",level:4}];function o(e){const n={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",h4:"h4",header:"header",li:"li",ol:"ol",p:"p",pre:"pre",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",ul:"ul",...(0,d.R)(),...e.components};return(0,s.jsxs)(s.Fragment,{children:[(0,s.jsx)(n.header,{children:(0,s.jsx)(n.h1,{id:"procedure-api",children:"Procedure API"})}),"\n",(0,s.jsxs)(n.blockquote,{children:["\n",(0,s.jsx)(n.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u8bb2\u89e3 TuGraph \u7684\u5b58\u50a8\u8fc7\u7a0b\u4f7f\u7528\u8bf4\u660e"}),"\n"]}),"\n",(0,s.jsx)(n.h2,{id:"1\u7b80\u4ecb",children:"1.\u7b80\u4ecb"}),"\n",(0,s.jsx)(n.p,{children:"\u5f53\u7528\u6237\u9700\u8981\u8868\u8fbe\u7684\u67e5\u8be2/\u66f4\u65b0\u903b\u8f91\u8f83\u4e3a\u590d\u6742\uff08\u4f8b\u5982 Cypher \u65e0\u6cd5\u63cf\u8ff0\uff0c\u6216\u662f\u5bf9\u6027\u80fd\u8981\u6c42\u8f83\u9ad8\uff09\u65f6\uff0c\u76f8\u6bd4\u8c03\u7528\u591a\u4e2a\u8bf7\u6c42\u5e76\u5728\u5ba2\u6237\u7aef\u5b8c\u6210\u6574\u4e2a\u5904\u7406\u6d41\u7a0b\u7684\u65b9\u5f0f\uff0cTuGraph \u63d0\u4f9b\u7684\u5b58\u50a8\u8fc7\u7a0b\u662f\u66f4\u7b80\u6d01\u548c\u9ad8\u6548\u7684\u9009\u62e9\u3002"}),"\n",(0,s.jsx)(n.p,{children:"\u4e0e\u4f20\u7edf\u6570\u636e\u5e93\u7c7b\u4f3c\uff0cTuGraph \u7684\u5b58\u50a8\u8fc7\u7a0b\u8fd0\u884c\u5728\u670d\u52a1\u5668\u7aef\uff0c\u7528\u6237\u901a\u8fc7\u5c06\u5904\u7406\u903b\u8f91\uff08\u5373\u591a\u4e2a\u64cd\u4f5c\uff09\u5c01\u88c5\u5230\u4e00\u4e2a\u8fc7\u7a0b\u5355\u6b21\u8c03\u7528\uff0c\u5e76\u4e14\u53ef\u4ee5\u5728\u5b9e\u73b0\u65f6\u901a\u8fc7\u5e76\u884c\u5904\u7406\u7684\u65b9\u5f0f\uff08\u4f8b\u5982\u4f7f\u7528\u76f8\u5173\u7684 C++ OLAP \u63a5\u53e3\u4ee5\u53ca\u57fa\u4e8e\u5176\u5b9e\u73b0\u7684\u5185\u7f6e\u7b97\u6cd5\uff09\u8fdb\u4e00\u6b65\u63d0\u5347\u6027\u80fd\u3002"}),"\n",(0,s.jsxs)(n.p,{children:["\u5b58\u50a8\u8fc7\u7a0b\u4e2d\u6709\u4e00\u7c7b\u7279\u6b8a\u7684API\u6765\u8fdb\u884c\u6570\u636e\u7684\u5e76\u884c\u64cd\u4f5c\uff0c\u6211\u4eec\u53eb Traversal API\uff0c\u89c1",(0,s.jsx)(n.a,{href:"/tugraph-db/en-US/zh/4.3.2/olap&procedure/procedure/traversal",children:"\u6587\u6863"}),"\u3002"]}),"\n",(0,s.jsx)(n.h2,{id:"2\u5b58\u50a8\u8fc7\u7a0b\u7684\u7248\u672c\u652f\u6301",children:"2.\u5b58\u50a8\u8fc7\u7a0b\u7684\u7248\u672c\u652f\u6301"}),"\n",(0,s.jsx)(n.p,{children:"\u76ee\u524dTuGraph\u652f\u6301\u4e24\u4e2a\u7248\u672c\u7684\u5b58\u50a8\u8fc7\u7a0b\uff0c\u9002\u7528\u4e8e\u4e0d\u540c\u7684\u573a\u666f\uff0cv3.5\u7248\u672c\u53ea\u652f\u6301v1\uff0c\u53ef\u901a\u8fc7REST\u6216RPC\u63a5\u53e3\u76f4\u63a5\u8c03\u7528\uff1b\u4ecev3.5\u7248\u672c\u5f00\u59cb\u652f\u6301v2\uff0c\u80fd\u591f\u5728\u56fe\u67e5\u8be2\u8bed\u8a00\uff08\u6bd4\u5982Cypher\uff09\u4e2d\u5d4c\u5165\u8c03\u7528\uff0c\u6211\u4eec\u79f0\u4e4b\u4e3aPOG\uff08Procedure On Graph query language\uff0cAPOC\uff09\u3002"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{}),(0,s.jsx)(n.th,{children:"Procedure v1"}),(0,s.jsx)(n.th,{children:"Procedure v2"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"\u9002\u7528\u573a\u666f"}),(0,s.jsx)(n.td,{children:"\u6781\u81f4\u6027\u80fd\uff0c\u6216\u8005\u590d\u6742\u7684\u591a\u4e8b\u52a1\u7ba1\u7406\u60c5\u5f62"}),(0,s.jsx)(n.td,{children:"\u4e00\u822c\u60c5\u51b5\uff0c\u4e0eCypher\u9ad8\u5ea6\u8054\u52a8"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"\u4e8b\u52a1"}),(0,s.jsx)(n.td,{children:"\u51fd\u6570\u5185\u90e8\u521b\u5efa\uff0c\u53ef\u81ea\u7531\u63a7\u5236\u591a\u4e8b\u52a1"}),(0,s.jsx)(n.td,{children:"\u5916\u90e8\u4f20\u5165\u51fd\u6570\uff0c\u5355\u4e00\u4e8b\u52a1"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"\u7b7e\u540d\uff08\u53c2\u6570\u5b9a\u4e49\uff09"}),(0,s.jsx)(n.td,{children:"\u65e0"}),(0,s.jsx)(n.td,{children:"\u6709"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"\u8f93\u5165\u8f93\u51fa\u53c2\u6570\u7c7b\u578b"}),(0,s.jsx)(n.td,{children:"\u4e0d\u9700\u8981\u6307\u5b9a"}),(0,s.jsx)(n.td,{children:"\u9700\u8981\u6307\u5b9a\u53c2\u6570\u7c7b\u578b"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"Cypher Standalone Call"}),(0,s.jsx)(n.td,{children:"\u652f\u6301"}),(0,s.jsx)(n.td,{children:"\u652f\u6301"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"Cypher Embeded Call"}),(0,s.jsx)(n.td,{children:"\u4e0d\u652f\u6301"}),(0,s.jsx)(n.td,{children:"\u652f\u6301"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"\u8bed\u8a00"}),(0,s.jsx)(n.td,{children:"C++/Python/Rust"}),(0,s.jsx)(n.td,{children:"C++"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"\u8c03\u7528\u6a21\u5f0f"}),(0,s.jsx)(n.td,{children:"\u76f4\u63a5\u4f20\u5b57\u7b26\u4e32\uff0c\u4e00\u822c\u4e3aJSON"}),(0,s.jsx)(n.td,{children:"\u901a\u8fc7Cypher\u8bed\u53e5\u4e2d\u7684\u53d8\u91cf"})]})]})]}),"\n",(0,s.jsx)(n.p,{children:"\u5728TuGraph\u4e2d\uff0c\u5b58\u50a8\u8fc7\u7a0bv1\u548cv2\u5355\u72ec\u7ba1\u7406\uff0c\u652f\u6301\u589e\u5220\u67e5\uff0c\u4f46\u4ecd\u4e0d\u5efa\u8bae\u91cd\u540d\u3002"}),"\n",(0,s.jsx)(n.h2,{id:"3\u5b58\u50a8\u8fc7\u7a0b\u8bed\u8a00\u652f\u6301",children:"3.\u5b58\u50a8\u8fc7\u7a0b\u8bed\u8a00\u652f\u6301"}),"\n",(0,s.jsx)(n.p,{children:"\u5728 TuGraph \u4e2d\uff0c\u7528\u6237\u53ef\u4ee5\u52a8\u6001\u7684\u52a0\u8f7d\uff0c\u66f4\u65b0\u548c\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b\u3002TuGraph \u652f\u6301 C++ \u8bed\u8a00\u3001 Python \u8bed\u8a00\u548c Rust \u8bed\u8a00\u7f16\u5199\u5b58\u50a8\u8fc7\u7a0b\u3002\u5728\u6027\u80fd\u4e0a C++ \u8bed\u8a00\u652f\u6301\u7684\u6700\u5b8c\u6574\uff0c\u6027\u80fd\u6700\u4f18\u3002"}),"\n",(0,s.jsx)(n.p,{children:"\u6ce8\u610f\u5b58\u50a8\u8fc7\u7a0b\u662f\u5728\u670d\u52a1\u7aef\u7f16\u8bd1\u6267\u884c\u7684\u903b\u8f91\uff0c\u548c\u5ba2\u6237\u7aef\u7684\u8bed\u8a00\u652f\u6301\u65e0\u5173\u3002"}),"\n",(0,s.jsx)(n.h2,{id:"4procedure-v1\u63a5\u53e3",children:"4.Procedure v1\u63a5\u53e3"}),"\n",(0,s.jsx)(n.h2,{id:"41\u7f16\u5199\u5b58\u50a8\u8fc7\u7a0b",children:"4.1.\u7f16\u5199\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,s.jsx)(n.h3,{id:"411\u7f16\u5199c\u5b58\u50a8\u8fc7\u7a0b",children:"4.1.1.\u7f16\u5199C++\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,s.jsx)(n.p,{children:"\u7528\u6237\u53ef\u4ee5\u901a\u8fc7\u4f7f\u7528 Procedure API \u6216\u8005 Traversal API \u6765\u7f16\u5199 C \u5b58\u50a8\u8fc7\u7a0b\u3002\u4e00\u4e2a\u7b80\u5355\u7684 C \u5b58\u50a8\u8fc7\u7a0b\u4e3e\u4f8b\u5982\u4e0b\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:'#include <iostream>\n#include "lgraph.h"\nusing namespace lgraph_api;\n\nextern "C" LGAPI bool Process(GraphDB& db, const std::string& request, std::string& response) {\n\tauto txn = db.CreateReadTxn();\n\tsize_t n = 0;\n\tfor (auto vit = txn.GetVertexIterator(); vit.IsValid(); vit.Next()) {\n        if (vit.GetLabel() == "student") {\n            auto age = vit.GetField("age");\n            if (!age.is_null() && age.integer() == 10) n++; ## \u7edf\u8ba1\u6240\u6709\u5e74\u9f84\u4e3a10\u7684\u5b66\u751f\u6570\u91cf\n        }\n\t}\n    output = std::to_string(n);\n    return true;\n}\n'})}),"\n",(0,s.jsxs)(n.p,{children:["\u4ece\u4ee3\u7801\u4e2d\u6211\u4eec\u53ef\u4ee5\u770b\u5230\uff0c\u5b58\u50a8\u8fc7\u7a0b\u7684\u5165\u53e3\u51fd\u6570\u662f",(0,s.jsx)(n.code,{children:"Process"}),"\u51fd\u6570\uff0c\u5b83\u7684\u53c2\u6570\u6709\u4e09\u4e2a\uff0c\u5206\u522b\u4e3a\uff1a"]}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"db"}),": \u6570\u636e\u5e93\u5b9e\u4f8b"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"request"}),": \u8f93\u5165\u8bf7\u6c42\u6570\u636e\uff0c\u53ef\u4ee5\u662f\u4e8c\u8fdb\u5236\u5b57\u8282\u6570\u7ec4\uff0c\u6216\u8005 JSON \u4e32\u7b49\u5176\u5b83\u4efb\u610f\u683c\u5f0f\u3002"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"response"}),": \u8f93\u51fa\u6570\u636e\uff0c\u53ef\u4ee5\u662f\u5b57\u7b26\u4e32\uff0c\u4e5f\u53ef\u4ee5\u76f4\u63a5\u8fd4\u56de\u4e8c\u8fdb\u5236\u6570\u636e\u3002"]}),"\n"]}),"\n",(0,s.jsxs)(n.p,{children:[(0,s.jsx)(n.code,{children:"Process"}),"\u51fd\u6570\u7684\u8fd4\u56de\u503c\u662f\u4e00\u4e2a\u5e03\u5c14\u503c\u3002\u5f53\u5b83\u8fd4\u56de",(0,s.jsx)(n.code,{children:"true"}),"\u7684\u65f6\u5019\uff0c\u8868\u793a\u8be5\u8bf7\u6c42\u987a\u5229\u5b8c\u6210\uff0c\u53cd\u4e4b\u8868\u793a\u8fd9\u4e2a\u5b58\u50a8\u8fc7\u7a0b\u5728\u6267\u884c\u8fc7\u7a0b\u4e2d\u53d1\u73b0\u4e86\u9519\u8bef\uff0c\u6b64\u65f6\u7528\u6237\u53ef\u4ee5\u901a\u8fc7",(0,s.jsx)(n.code,{children:"response"}),"\u6765\u8fd4\u56de\u9519\u8bef\u4fe1\u606f\u4ee5\u65b9\u4fbf\u8c03\u8bd5\u3002"]}),"\n",(0,s.jsxs)(n.p,{children:["C++\u5b58\u50a8\u8fc7\u7a0b\u7f16\u5199\u5b8c\u6bd5\u540e\u9700\u8981\u7f16\u8bd1\u6210\u52a8\u6001\u94fe\u63a5\u5e93\u3002TuGraph \u63d0\u4f9b\u4e86",(0,s.jsx)(n.code,{children:"compile.sh"}),"\u811a\u672c\u6765\u5e2e\u52a9\u7528\u6237\u81ea\u52a8\u7f16\u8bd1\u5b58\u50a8\u8fc7\u7a0b\u3002",(0,s.jsx)(n.code,{children:"compile.sh"}),"\u811a\u672c\u53ea\u6709\u4e00\u4e2a\u53c2\u6570\uff0c\u662f\u8be5\u5b58\u50a8\u8fc7\u7a0b\u7684\u540d\u79f0\uff0c\u5728\u4e0a\u9762\u7684\u4f8b\u5b50\u4e2d\u5c31\u662f",(0,s.jsx)(n.code,{children:"age_10"}),"\u3002\u7f16\u8bd1\u8c03\u7528\u547d\u4ee4\u884c\u5982\u4e0b\uff1a"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-bash",children:"g++ -fno-gnu-unique -fPIC -g --std=c++14 -I/usr/local/include/lgraph -rdynamic -O3 -fopenmp -o age_10.so age_10.cpp /usr/local/lib64/liblgraph.so -shared\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u5982\u679c\u7f16\u8bd1\u987a\u5229\uff0c\u4f1a\u751f\u6210 age_10.so\uff0c\u7136\u540e\u7528\u6237\u5c31\u53ef\u4ee5\u5c06\u5b83\u52a0\u8f7d\u5230\u670d\u52a1\u5668\u4e2d\u4e86\u3002"}),"\n",(0,s.jsx)(n.h3,{id:"412\u7f16\u5199python\u5b58\u50a8\u8fc7\u7a0b",children:"4.1.2.\u7f16\u5199Python\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,s.jsx)(n.p,{children:"\u4e0e C++\u7c7b\u4f3c\uff0cPython \u5b58\u50a8\u8fc7\u7a0b\u4e5f\u53ef\u4ee5\u8c03\u7528 core API\uff0c\u4e00\u4e2a\u7b80\u5355\u7684\u4f8b\u5b50\u5982\u4e0b\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:"def Process(db, input):\n    txn = db.CreateReadTxn()\n    it = txn.GetVertexIterator()\n    n = 0\n    while it.IsValid():\n        if it.GetLabel() == 'student' and it['age'] and it['age'] == 10:\n            n = n + 1\n        it.Next()\n    return (True, str(nv))\n"})}),"\n",(0,s.jsxs)(n.p,{children:["Python \u5b58\u50a8\u8fc7\u7a0b\u8fd4\u56de\u7684\u662f\u4e00\u4e2a tuple\uff0c\u5176\u4e2d\u7b2c\u4e00\u4e2a\u5143\u7d20\u662f\u4e00\u4e2a\u5e03\u5c14\u503c\uff0c\u8868\u793a\u8be5\u5b58\u50a8\u8fc7\u7a0b\u662f\u5426\u6210\u529f\u6267\u884c\uff1b\u7b2c\u4e8c\u4e2a\u5143\u7d20\u662f\u4e00\u4e2a",(0,s.jsx)(n.code,{children:"str"}),"\uff0c\u91cc\u9762\u662f\u9700\u8981\u8fd4\u56de\u7684\u7ed3\u679c\u3002"]}),"\n",(0,s.jsx)(n.p,{children:"Python \u5b58\u50a8\u8fc7\u7a0b\u4e0d\u9700\u8981\u7f16\u8bd1\uff0c\u53ef\u4ee5\u76f4\u63a5\u52a0\u8f7d\u3002"}),"\n",(0,s.jsx)(n.h2,{id:"42\u5982\u4f55\u4f7f\u7528\u5b58\u50a8\u8fc7\u7a0b",children:"4.2.\u5982\u4f55\u4f7f\u7528\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,s.jsx)(n.h3,{id:"421\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b",children:"4.2.1.\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,s.jsxs)(n.p,{children:["\u7528\u6237\u53ef\u4ee5\u901a\u8fc7 REST API \u548c RPC \u6765\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b\u3002\u4ee5 REST API \u4e3a\u4f8b\uff0c\u52a0\u8f7d",(0,s.jsx)(n.code,{children:"age_10.so"}),"\u7684 C++\u4ee3\u7801\u5982\u4e0b\uff1a"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:"import requests\nimport json\nimport base64\n\ndata = {'name':'age_10'}\nf = open('./age_10.so','rb')\ncontent = f.read()\ndata['code_base64'] = base64.b64encode(content).decode()\ndata['description'] = 'Custom Page Rank Procedure'\ndata['read_only'] = true\ndata['code_type'] = 'so'\njs = json.dumps(data)\nr = requests.post(url='http://127.0.0.1:7071/db/school/cpp_plugin', data=js,\n            headers={'Content-Type':'application/json'})\nprint(r.status_code)    ## \u6b63\u5e38\u65f6\u8fd4\u56de200\n"})}),"\n",(0,s.jsxs)(n.p,{children:["\u9700\u8981\u6ce8\u610f\u7684\u662f\uff0c\u8fd9\u65f6\u7684",(0,s.jsx)(n.code,{children:"data['code']"}),"\u662f\u4e00\u4e2a\u7ecf\u8fc7 base64 \u5904\u7406\u7684\u5b57\u7b26\u4e32\uff0c",(0,s.jsx)(n.code,{children:"age_10.so"}),"\u4e2d\u7684\u4e8c\u8fdb\u5236\u4ee3\u7801\u662f\u65e0\u6cd5\u901a\u8fc7 JSON \u76f4\u63a5\u4f20\u8f93\u7684\u3002\u6b64\u5916\uff0c\u5b58\u50a8\u8fc7\u7a0b\u7684\u52a0\u8f7d\u548c\u5220\u9664\u90fd\u53ea\u80fd\u7531\u5177\u6709\u7ba1\u7406\u5458\u6743\u9650\u7684\u7528\u6237\u6765\u64cd\u4f5c\u3002"]}),"\n",(0,s.jsx)(n.p,{children:"\u5b58\u50a8\u8fc7\u7a0b\u52a0\u8f7d\u4e4b\u540e\u4f1a\u88ab\u4fdd\u5b58\u5728\u6570\u636e\u5e93\u4e2d\uff0c\u5728\u670d\u52a1\u5668\u91cd\u542f\u540e\u4e5f\u4f1a\u88ab\u81ea\u52a8\u52a0\u8f7d\u3002\u6b64\u5916\uff0c\u5982\u679c\u9700\u8981\u5bf9\u5b58\u50a8\u8fc7\u7a0b\u8fdb\u884c\u66f4\u65b0\uff0c\u8c03\u7528\u7684 REST API \u4e5f\u662f\u540c\u6837\u7684\u3002\u5efa\u8bae\u7528\u6237\u5728\u66f4\u65b0\u5b58\u50a8\u8fc7\u7a0b\u65f6\u66f4\u65b0\u76f8\u5e94\u63cf\u8ff0\uff0c\u4ee5\u4fbf\u533a\u5206\u4e0d\u540c\u7248\u672c\u7684\u5b58\u50a8\u8fc7\u7a0b\u3002"}),"\n",(0,s.jsx)(n.h3,{id:"422\u5217\u51fa\u5df2\u52a0\u8f7d\u7684\u5b58\u50a8\u8fc7\u7a0b",children:"4.2.2.\u5217\u51fa\u5df2\u52a0\u8f7d\u7684\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,s.jsx)(n.p,{children:"\u5728\u670d\u52a1\u5668\u8fd0\u884c\u8fc7\u7a0b\u4e2d\uff0c\u7528\u6237\u53ef\u4ee5\u968f\u65f6\u83b7\u53d6\u5b58\u50a8\u8fc7\u7a0b\u5217\u8868\u3002\u5176\u8c03\u7528\u5982\u4e0b\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'>>> r = requests.get(\'http://127.0.0.1:7071/db/school/cpp_plugin\')\n>>> r.status_code\n200\n>>> r.text\n\'{"plugins":[{"description":"Custom Page Rank Procedure", "name":"age_10", "read_only":true}]}\'\n'})}),"\n",(0,s.jsx)(n.h3,{id:"423\u83b7\u53d6\u5b58\u50a8\u8fc7\u7a0b\u8be6\u60c5",children:"4.2.3.\u83b7\u53d6\u5b58\u50a8\u8fc7\u7a0b\u8be6\u60c5"}),"\n",(0,s.jsx)(n.p,{children:"\u5728\u670d\u52a1\u5668\u8fd0\u884c\u8fc7\u7a0b\u4e2d\uff0c\u7528\u6237\u53ef\u4ee5\u968f\u65f6\u83b7\u53d6\u5355\u4e2a\u5b58\u50a8\u8fc7\u7a0b\u7684\u8be6\u60c5\uff0c\u5305\u62ec\u4ee3\u7801\u3002\u5176\u8c03\u7528\u5982\u4e0b\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'>>> r = requests.get(\'http://127.0.0.1:7071/db/school/cpp_plugin/age_10\')\n>>> r.status_code\n200\n>>> r.text\n\'{"description":"Custom Page Rank Procedure", "name":"age_10", "read_only":true, "code_base64":<CODE>, "code_type":"so"}\'\n'})}),"\n",(0,s.jsx)(n.h3,{id:"424\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",children:"4.2.4.\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,s.jsx)(n.p,{children:"\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b\u7684\u4ee3\u7801\u793a\u4f8b\u5982\u4e0b\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:">>> r = requests.post(url='http://127.0.0.1:7071/db/school/cpp_plugin/age_10', data='',\n                headers={'Content-Type':'application/json'})\n>>> r.status_code\n200\n>>> r.text\n9\n"})}),"\n",(0,s.jsx)(n.h3,{id:"425\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b",children:"4.2.5.\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,s.jsx)(n.p,{children:"\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b\u53ea\u9700\u8981\u5982\u4e0b\u8c03\u7528\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:">>> r = requests.delete(url='http://127.0.0.1:7071/db/school/cpp_plugin/age_10')\n>>> r.status_code\n200\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u4e0e\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b\u7c7b\u4f3c\uff0c\u53ea\u6709\u7ba1\u7406\u5458\u7528\u6237\u624d\u80fd\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b\u3002"}),"\n",(0,s.jsx)(n.h3,{id:"426\u66f4\u65b0\u5b58\u50a8\u8fc7\u7a0b",children:"4.2.6.\u66f4\u65b0\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,s.jsx)(n.p,{children:"\u66f4\u65b0\u5b58\u50a8\u8fc7\u7a0b\u9700\u8981\u6267\u884c\u5982\u4e0b\u4e24\u4e2a\u6b65\u9aa4\uff1a"}),"\n",(0,s.jsxs)(n.ol,{children:["\n",(0,s.jsx)(n.li,{children:"\u5220\u9664\u5df2\u5b58\u5728\u7684\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,s.jsx)(n.li,{children:"\u5b89\u88c5\u65b0\u7684\u5b58\u50a8\u8fc7\u7a0b"}),"\n"]}),"\n",(0,s.jsx)(n.p,{children:"TuGraph \u8f83\u4e3a\u8c28\u614e\u5730\u7ba1\u7406\u5b58\u50a8\u8fc7\u7a0b\u64cd\u4f5c\u7684\u5e76\u53d1\u6027\uff0c\u66f4\u65b0\u5b58\u50a8\u8fc7\u7a0b\u4e0d\u4f1a\u5f71\u54cd\u73b0\u6709\u5b58\u50a8\u8fc7\u7a0b\u7684\u8fd0\u884c\u3002"}),"\n",(0,s.jsx)(n.h2,{id:"5procedure-v2\u63a5\u53e3",children:"5.Procedure v2\u63a5\u53e3"}),"\n",(0,s.jsx)(n.p,{children:"\u4e0b\u9762\u7684\u8bf4\u660e\u4ee5 REST API \u4e3a\u4f8b\uff0c\u4ecb\u7ecd\u5b58\u50a8\u8fc7\u7a0bv2\u7684\u8c03\u7528\u3002"}),"\n",(0,s.jsx)(n.h3,{id:"51\u7f16\u5199\u5b58\u50a8\u8fc7\u7a0b",children:"5.1.\u7f16\u5199\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,s.jsx)(n.p,{children:"\u7528\u6237\u53ef\u4ee5\u901a\u8fc7\u4f7f\u7528 lgraph API \u6765\u7f16\u5199 C++ \u5b58\u50a8\u8fc7\u7a0b\u3002\u4e00\u4e2a\u7b80\u5355\u7684 C++ \u5b58\u50a8\u8fc7\u7a0b\u4e3e\u4f8b\u5982\u4e0b\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-c++",children:'// peek_some_node_salt.cpp\n#include <cstdlib>\n#include "lgraph/lgraph.h"\n#include "lgraph/lgraph_types.h"\n#include "lgraph/lgraph_result.h"\n\n#include "tools/json.hpp"\n\nusing json = nlohmann::json;\nusing namespace lgraph_api;\n\nextern "C" LGAPI bool GetSignature(SigSpec &sig_spec) {\n    sig_spec.input_list = {\n        {.name = "limit", .index = 0, .type = LGraphType::INTEGER},\n    };\n    sig_spec.result_list = {\n        {.name = "node", .index = 0, .type = LGraphType::NODE},\n        {.name = "salt", .index = 1, .type = LGraphType::FLOAT}\n    };\n    return true;\n}\n\nextern "C" LGAPI bool ProcessInTxn(Transaction &txn,\n                                   const std::string &request,\n                                   Result &response) {\n    int64_t limit;\n    try {\n        json input = json::parse(request);\n        limit = input["limit"].get<int64_t>();\n    } catch (std::exception &e) {\n        response.ResetHeader({\n            {"errMsg", LGraphType::STRING}\n        });\n        response.MutableRecord()->Insert(\n            "errMsg",\n            FieldData::String(std::string("error parsing json: ") + e.what()));\n        return false;\n    }\n\n    response.ResetHeader({\n        {"node", LGraphType::NODE},\n        {"salt", LGraphType::FLOAT}\n    });\n    for (size_t i = 0; i < limit; i++) {\n        auto r = response.MutableRecord();\n        auto vit = txn.GetVertexIterator(i);\n        r->Insert("node", vit);\n        r->Insert("salt", FieldData::Float(20.23*float(i)));\n    }\n    return true;\n}\n'})}),"\n",(0,s.jsx)(n.p,{children:"\u4ece\u4ee3\u7801\u4e2d\u6211\u4eec\u53ef\u4ee5\u770b\u5230\uff1a"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:["\n",(0,s.jsxs)(n.p,{children:["\u5b58\u50a8\u8fc7\u7a0b\u5b9a\u4e49\u4e86\u4e00\u4e2a\u83b7\u53d6\u7b7e\u540d\u7684\u65b9\u6cd5",(0,s.jsx)(n.code,{children:"GetSignature"}),"\u3002\u8be5\u65b9\u6cd5\u8fd4\u56de\u4e86\u5b58\u50a8\u8fc7\u7a0b\u7684\u7b7e\u540d\uff0c\u5176\u4e2d\u5305\u542b\u8f93\u5165\u53c2\u6570\u540d\u79f0\u53ca\u5176\u7c7b\u578b\uff0c\u8fd4\u56de\u53c2\u6570\u53ca\u5176\u7c7b\u578b\u3002\u8fd9\u4f7f\u5f97Cypher\u67e5\u8be2\u8bed\u53e5\u5728\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b\u80fd\u591f\u5229\u7528\u7b7e\u540d\u4fe1\u606f\u6821\u9a8c\u8f93\u5165\u6570\u636e\u4ee5\u53ca\u8fd4\u56de\u6570\u636e\u662f\u5426\u5408\u7406\u3002"]}),"\n"]}),"\n",(0,s.jsxs)(n.li,{children:["\n",(0,s.jsxs)(n.p,{children:["\u5165\u53e3\u51fd\u6570\u662f",(0,s.jsx)(n.code,{children:"ProcessInTxn"}),"\u51fd\u6570\uff0c\u5b83\u7684\u53c2\u6570\u6709\u4e09\u4e2a\uff0c\u5206\u522b\u4e3a\uff1a"]}),"\n"]}),"\n",(0,s.jsxs)(n.li,{children:["\n",(0,s.jsxs)(n.p,{children:[(0,s.jsx)(n.code,{children:"txn"}),": \u5b58\u50a8\u8fc7\u7a0b\u6240\u5904\u7684\u4e8b\u52a1\uff0c\u901a\u5e38\u6765\u8bf4\u5373\u8c03\u7528\u8be5\u5b58\u50a8\u8fc7\u7a0b\u7684Cypher\u8bed\u53e5\u6240\u5904\u4e8b\u52a1\u3002"]}),"\n"]}),"\n",(0,s.jsxs)(n.li,{children:["\n",(0,s.jsxs)(n.p,{children:[(0,s.jsx)(n.code,{children:"request"}),": \u8f93\u5165\u6570\u636e\uff0c\u5176\u5185\u5bb9\u4e3a",(0,s.jsx)(n.code,{children:"GetSignature"}),"\u4e2d\u5b9a\u4e49\u7684\u8f93\u5165\u53c2\u6570\u7c7b\u578b\u53ca\u5176Cypher\u67e5\u8be2\u8bed\u53e5\u4e2d\u4f20\u5165\u7684\u503c\u7ecf\u8fc7json\u5e8f\u5217\u5316\u540e\u7684\u5b57\u7b26\u4e32\u3002e.g. ",(0,s.jsx)(n.code,{children:"{num_iteration: 10}"})]}),"\n"]}),"\n",(0,s.jsxs)(n.li,{children:["\n",(0,s.jsxs)(n.p,{children:[(0,s.jsx)(n.code,{children:"response"}),": \u8f93\u51fa\u6570\u636e\uff0c\u4e3a\u4fdd\u8bc1\u5728Cypher\u8bed\u8a00\u4e2d\u80fd\u591f\u517c\u5bb9\uff0c\u7528\u6237\u53ef\u4ee5\u901a\u8fc7\u5f80",(0,s.jsx)(n.code,{children:"lgraph_api::Result"})," \u5199\u5165\u5b58\u50a8\u8fc7\u7a0b\u5904\u7406\u540e\u7684\u6570\u636e\uff0c\u6700\u540e\u7528",(0,s.jsx)(n.code,{children:"lgraph_api::Result::Dump"}),"\u6765\u5e8f\u5217\u5316\u6210json\u683c\u5f0f\u7684\u6570\u636e\u3002"]}),"\n"]}),"\n"]}),"\n",(0,s.jsxs)(n.p,{children:[(0,s.jsx)(n.code,{children:"ProcessInTxn"}),"\u51fd\u6570\u7684\u8fd4\u56de\u503c\u662f\u4e00\u4e2a\u5e03\u5c14\u503c\u3002\u5f53\u5b83\u8fd4\u56de",(0,s.jsx)(n.code,{children:"true"}),"\u7684\u65f6\u5019\uff0c\u8868\u793a\u8be5\u8bf7\u6c42\u987a\u5229\u5b8c\u6210\uff0c\u53cd\u4e4b\u8868\u793a\u8fd9\u4e2a\u5b58\u50a8\u8fc7\u7a0b\u5728\u6267\u884c\u8fc7\u7a0b\u4e2d\u53d1\u73b0\u4e86\u9519\u8bef\u3002"]}),"\n",(0,s.jsxs)(n.p,{children:["C++\u5b58\u50a8\u8fc7\u7a0b\u7f16\u5199\u5b8c\u6bd5\u540e\u9700\u8981\u7f16\u8bd1\u6210\u52a8\u6001\u94fe\u63a5\u5e93\u3002TuGraph \u63d0\u4f9b\u4e86",(0,s.jsx)(n.code,{children:"compile.sh"}),"\u811a\u672c\u6765\u5e2e\u52a9\u7528\u6237\u81ea\u52a8\u7f16\u8bd1\u5b58\u50a8\u8fc7\u7a0b\u3002",(0,s.jsx)(n.code,{children:"compile.sh"}),"\u811a\u672c\u53ea\u6709\u4e00\u4e2a\u53c2\u6570\uff0c\u662f\u8be5\u5b58\u50a8\u8fc7\u7a0b\u7684\u540d\u79f0\uff0c\u5728\u4e0a\u9762\u7684\u4f8b\u5b50\u4e2d\u5c31\u662f",(0,s.jsx)(n.code,{children:"custom_pagerank"}),"\u3002\u7f16\u8bd1\u8c03\u7528\u547d\u4ee4\u884c\u5982\u4e0b\uff1a"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-bash",children:"g++ -fno-gnu-unique -fPIC -g --std=c++14 -I/usr/local/include/lgraph -rdynamic -O3 -fopenmp -o custom_pagerank.so custom_pagerank.cpp /usr/local/lib64/liblgraph.so -shared\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u5982\u679c\u7f16\u8bd1\u987a\u5229\uff0c\u4f1a\u751f\u6210 custom_pagerank.so\uff0c\u7136\u540e\u7528\u6237\u5c31\u53ef\u4ee5\u5c06\u5b83\u52a0\u8f7d\u5230\u670d\u52a1\u5668\u4e2d\u4e86\u3002"}),"\n",(0,s.jsx)(n.h3,{id:"52\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b",children:"5.2.\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,s.jsxs)(n.p,{children:["\u7528\u6237\u53ef\u4ee5\u901a\u8fc7 REST API \u548c RPC \u6765\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b\u3002\u4ee5 REST API \u4e3a\u4f8b\uff0c\u52a0\u8f7d",(0,s.jsx)(n.code,{children:"custom_pagerank.so"}),"\u7684 C++\u4ee3\u7801\u5982\u4e0b\uff1a"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:"import requests\nimport json\nimport base64\n\ndata = {'name':'custom_pagerank'}\nf = open('./custom_pagerank.so','rb')\ncontent = f.read()\ndata['code_base64'] = base64.b64encode(content).decode()\ndata['description'] = 'Custom Page Rank Procedure'\ndata['read_only'] = true\ndata['code_type'] = 'so'\njs = json.dumps(data)\nr = requests.post(url='http://127.0.0.1:7071/db/school/cpp_plugin', data=js,\n            headers={'Content-Type':'application/json'})\nprint(r.status_code)    ## \u6b63\u5e38\u65f6\u8fd4\u56de200\n"})}),"\n",(0,s.jsxs)(n.p,{children:["\u9700\u8981\u6ce8\u610f\u7684\u662f\uff0c\u8fd9\u65f6\u7684",(0,s.jsx)(n.code,{children:"data['code']"}),"\u662f\u4e00\u4e2a\u7ecf\u8fc7 base64 \u5904\u7406\u7684\u5b57\u7b26\u4e32\uff0c",(0,s.jsx)(n.code,{children:"custom_pagerank.so"}),"\u4e2d\u7684\u4e8c\u8fdb\u5236\u4ee3\u7801\u662f\u65e0\u6cd5\u901a\u8fc7 JSON \u76f4\u63a5\u4f20\u8f93\u7684\u3002\u6b64\u5916\uff0c\u5b58\u50a8\u8fc7\u7a0b\u7684\u52a0\u8f7d\u548c\u5220\u9664\u90fd\u53ea\u80fd\u7531\u5177\u6709\u7ba1\u7406\u5458\u6743\u9650\u7684\u7528\u6237\u6765\u64cd\u4f5c\u3002"]}),"\n",(0,s.jsx)(n.p,{children:"\u5b58\u50a8\u8fc7\u7a0b\u52a0\u8f7d\u4e4b\u540e\u4f1a\u88ab\u4fdd\u5b58\u5728\u6570\u636e\u5e93\u4e2d\uff0c\u5728\u670d\u52a1\u5668\u91cd\u542f\u540e\u4e5f\u4f1a\u88ab\u81ea\u52a8\u52a0\u8f7d\u3002\u6b64\u5916\uff0c\u5982\u679c\u9700\u8981\u5bf9\u5b58\u50a8\u8fc7\u7a0b\u8fdb\u884c\u66f4\u65b0\uff0c\u8c03\u7528\u7684 REST API \u4e5f\u662f\u540c\u6837\u7684\u3002\u5efa\u8bae\u7528\u6237\u5728\u66f4\u65b0\u5b58\u50a8\u8fc7\u7a0b\u65f6\u66f4\u65b0\u76f8\u5e94\u63cf\u8ff0\uff0c\u4ee5\u4fbf\u533a\u5206\u4e0d\u540c\u7248\u672c\u7684\u5b58\u50a8\u8fc7\u7a0b\u3002"}),"\n",(0,s.jsx)(n.h4,{id:"521\u5217\u51fa\u5df2\u52a0\u8f7d\u7684\u5b58\u50a8\u8fc7\u7a0b",children:"5.2.1.\u5217\u51fa\u5df2\u52a0\u8f7d\u7684\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,s.jsx)(n.p,{children:"\u5728\u670d\u52a1\u5668\u8fd0\u884c\u8fc7\u7a0b\u4e2d\uff0c\u7528\u6237\u53ef\u4ee5\u968f\u65f6\u83b7\u53d6\u5b58\u50a8\u8fc7\u7a0b\u5217\u8868\u3002\u5176\u8c03\u7528\u5982\u4e0b\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'>>> r = requests.get(\'http://127.0.0.1:7071/db/school/cpp_plugin\')\n>>> r.status_code\n200\n>>> r.text\n\'{"plugins":[{"description":"Custom Page Rank Procedure", "name":"custom_pagerank", "read_only":true}]}\'\n'})}),"\n",(0,s.jsx)(n.h4,{id:"522\u83b7\u53d6\u5b58\u50a8\u8fc7\u7a0b\u8be6\u60c5",children:"5.2.2.\u83b7\u53d6\u5b58\u50a8\u8fc7\u7a0b\u8be6\u60c5"}),"\n",(0,s.jsx)(n.p,{children:"\u5728\u670d\u52a1\u5668\u8fd0\u884c\u8fc7\u7a0b\u4e2d\uff0c\u7528\u6237\u53ef\u4ee5\u968f\u65f6\u83b7\u53d6\u5355\u4e2a\u5b58\u50a8\u8fc7\u7a0b\u7684\u8be6\u60c5\uff0c\u5305\u62ec\u4ee3\u7801\u3002\u5176\u8c03\u7528\u5982\u4e0b\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'>>> r = requests.get(\'http://127.0.0.1:7071/db/school/cpp_plugin/custom_pagerank\')\n>>> r.status_code\n200\n>>> r.text\n\'{"description":"Custom Page Rank Procedure", "name":"custom_pagerank", "read_only":true, "code_base64":<CODE>, "code_type":"so"}\'\n'})}),"\n",(0,s.jsx)(n.h4,{id:"523\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",children:"5.2.3.\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,s.jsx)(n.p,{children:"\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b\u7684\u4ee3\u7801\u793a\u4f8b\u5982\u4e0b\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-Cypher",children:"CALL plugin.cpp.custom_pagerank(10)\nYIELD node, pr WITH node, pr\nMATCH(node)-[r]->(n) RETURN node, r, n, pr\n"})}),"\n",(0,s.jsx)(n.h4,{id:"524\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b",children:"5.2.4.\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,s.jsx)(n.p,{children:"\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b\u53ea\u9700\u8981\u5982\u4e0b\u8c03\u7528\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:">>> r = requests.delete(url='http://127.0.0.1:7071/db/school/cpp_plugin/custom_pagerank')\n>>> r.status_code\n200\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u4e0e\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b\u7c7b\u4f3c\uff0c\u53ea\u6709\u7ba1\u7406\u5458\u7528\u6237\u624d\u80fd\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b\u3002"}),"\n",(0,s.jsx)(n.h4,{id:"525\u66f4\u65b0\u5b58\u50a8\u8fc7\u7a0b",children:"5.2.5.\u66f4\u65b0\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,s.jsx)(n.p,{children:"\u66f4\u65b0\u5b58\u50a8\u8fc7\u7a0b\u9700\u8981\u6267\u884c\u5982\u4e0b\u4e24\u4e2a\u6b65\u9aa4\uff1a"}),"\n",(0,s.jsxs)(n.ol,{children:["\n",(0,s.jsx)(n.li,{children:"\u5220\u9664\u5df2\u5b58\u5728\u7684\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,s.jsx)(n.li,{children:"\u5b89\u88c5\u65b0\u7684\u5b58\u50a8\u8fc7\u7a0b"}),"\n"]}),"\n",(0,s.jsx)(n.p,{children:"TuGraph \u8f83\u4e3a\u8c28\u614e\u5730\u7ba1\u7406\u5b58\u50a8\u8fc7\u7a0b\u64cd\u4f5c\u7684\u5e76\u53d1\u6027\uff0c\u66f4\u65b0\u5b58\u50a8\u8fc7\u7a0b\u4e0d\u4f1a\u5f71\u54cd\u73b0\u6709\u5b58\u50a8\u8fc7\u7a0b\u7684\u8fd0\u884c\u3002"})]})}function h(e={}){const{wrapper:n}={...(0,d.R)(),...e.components};return n?(0,s.jsx)(n,{...e,children:(0,s.jsx)(o,{...e})}):o(e)}},28453:(e,n,r)=>{r.d(n,{R:()=>t,x:()=>c});var s=r(96540);const d={},l=s.createContext(d);function t(e){const n=s.useContext(l);return s.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function c(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(d):e.components||d:t(e.components),s.createElement(l.Provider,{value:n},e.children)}}}]);