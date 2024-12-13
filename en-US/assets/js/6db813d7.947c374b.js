"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[56460],{98931:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>i,contentTitle:()=>d,default:()=>a,frontMatter:()=>c,metadata:()=>h,toc:()=>t});var l=r(74848),s=r(28453);const c={},d="\u547d\u4ee4\u884c\u5de5\u5177",h={id:"developer-manual/client-tools/tugraph-cli",title:"\u547d\u4ee4\u884c\u5de5\u5177",description:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd lgraph_cypher \u6587\u6863\u7684\u4f7f\u7528",source:"@site/versions/version-4.1.0/zh-CN/source/5.developer-manual/4.client-tools/5.tugraph-cli.md",sourceDirName:"5.developer-manual/4.client-tools",slug:"/developer-manual/client-tools/tugraph-cli",permalink:"/tugraph-db/en-US/zh/4.1.0/developer-manual/client-tools/tugraph-cli",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:5,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"TuGraph-OGM",permalink:"/tugraph-db/en-US/zh/4.1.0/developer-manual/client-tools/tugraph-ogm"},next:{title:"Bolt\u5ba2\u6237\u7aef",permalink:"/tugraph-db/en-US/zh/4.1.0/developer-manual/client-tools/bolt-client"}},i={},t=[{value:"1.\u5355\u547d\u4ee4\u6a21\u5f0f",id:"1\u5355\u547d\u4ee4\u6a21\u5f0f",level:2},{value:"1.1.\u547d\u4ee4\u884c\u53c2\u6570:",id:"11\u547d\u4ee4\u884c\u53c2\u6570",level:3},{value:"1.2.\u547d\u4ee4\u793a\u4f8b:",id:"12\u547d\u4ee4\u793a\u4f8b",level:3},{value:"2.\u4ea4\u4e92\u6a21\u5f0f",id:"2\u4ea4\u4e92\u6a21\u5f0f",level:2},{value:"2.1.\u8fdb\u5165 lgraph_cypher \u4ea4\u4e92\u6a21\u5f0f:",id:"21\u8fdb\u5165-lgraph_cypher-\u4ea4\u4e92\u6a21\u5f0f",level:3},{value:"2.2.command\u79cd\u7c7b\u4e0e\u8bf4\u660e:",id:"22command\u79cd\u7c7b\u4e0e\u8bf4\u660e",level:3},{value:"2.3.cypher \u67e5\u8be2\u547d\u4ee4:",id:"23cypher-\u67e5\u8be2\u547d\u4ee4",level:3},{value:"2.4.\u8f85\u52a9\u529f\u80fd:",id:"24\u8f85\u52a9\u529f\u80fd",level:3}];function o(e){const n={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",li:"li",p:"p",pre:"pre",strong:"strong",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",ul:"ul",...(0,s.R)(),...e.components};return(0,l.jsxs)(l.Fragment,{children:[(0,l.jsx)(n.header,{children:(0,l.jsx)(n.h1,{id:"\u547d\u4ee4\u884c\u5de5\u5177",children:"\u547d\u4ee4\u884c\u5de5\u5177"})}),"\n",(0,l.jsxs)(n.blockquote,{children:["\n",(0,l.jsx)(n.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd lgraph_cypher \u6587\u6863\u7684\u4f7f\u7528"}),"\n"]}),"\n",(0,l.jsxs)(n.p,{children:["TuGraph \u53d1\u5e03\u7248\u672c\u9644\u5e26\u540d\u4e3a",(0,l.jsx)(n.code,{children:"lgraph_cypher"}),"\u7684\u67e5\u8be2\u5ba2\u6237\u7aef\uff0c\u53ef\u7528\u4e8e\u5411 TuGraph \u670d\u52a1\u5668\u63d0\u4ea4 OpenCypher \u8bf7\u6c42\u3002",(0,l.jsx)(n.code,{children:"lgraph_cypher"}),"\u5ba2\u6237\u7aef\u6709\u4e24\u79cd\u6267\u884c\u6a21\u5f0f\uff1a\u5355\u547d\u4ee4\u6a21\u5f0f\u548c\u4ea4\u4e92\u5f0f\u6a21\u5f0f\u3002"]}),"\n",(0,l.jsx)(n.h2,{id:"1\u5355\u547d\u4ee4\u6a21\u5f0f",children:"1.\u5355\u547d\u4ee4\u6a21\u5f0f"}),"\n",(0,l.jsxs)(n.p,{children:["\u5728\u5355\u547d\u4ee4\u6a21\u5f0f\u4e0b\uff0c",(0,l.jsx)(n.code,{children:"lgraph_cypher"}),"\u53ef\u7528\u4e8e\u63d0\u4ea4\u5355\u4e2a Cypher \u67e5\u8be2\u5e76\u5c06\u7ed3\u679c\u76f4\u63a5\u6253\u5370\u5230\u7ec8\u7aef\uff0c\u6253\u5370\u7ed3\u679c\u4e5f\u53ef\u4ee5\u5bb9\u6613\u5730\u91cd\u5b9a\u5411\u5199\u5165\u6307\u5b9a\u6587\u4ef6\u3002\u5f53\u7528\u6237\u9700\u8981\u4ece\u670d\u52a1\u5668\u83b7\u53d6\u5927\u91cf\u7ed3\u679c\u5e76\u5c06\u5176\u4fdd\u5b58\u5728\u6587\u4ef6\u4e2d\u65f6\uff0c\u8fd9\u975e\u5e38\u4fbf\u5229\u3002\n\u5728\u6b64\u6a21\u5f0f\u4e0b\uff0c",(0,l.jsx)(n.code,{children:"lgraph_cypher"}),"\u5de5\u5177\u5177\u6709\u4ee5\u4e0b\u9009\u9879\uff1a"]}),"\n",(0,l.jsx)(n.h3,{id:"11\u547d\u4ee4\u884c\u53c2\u6570",children:"1.1.\u547d\u4ee4\u884c\u53c2\u6570:"}),"\n",(0,l.jsxs)(n.p,{children:["| \u53c2\u6570     | \u7c7b\u578b   | \u8bf4\u660e                                                                     |\n| -------- | ------ | ------------------------------------------------------------------------ | ------ | ----------------------------------------------------------------------------------------------------------------------------------------- |\n| --help   | \\     | \u5217\u51fa\u6240\u6709\u53c2\u6570\u53ca\u8bf4\u660e\u3002                                                     |\n| -example | \\     | \u5217\u51fa\u547d\u4ee4\u5b9e\u4f8b\u3002                                                           |\n| -c       | string | \u6570\u636e\u5e93\u7684\u914d\u7f6e\u6587\u4ef6\uff0c\u7528\u4e8e\u83b7\u53d6 ip \u4e0e port \u4fe1\u606f\u3002                             |\n| -h       | string | \u6570\u636e\u5e93\u670d\u52a1\u5668 ip \u5730\u5740\uff0c\u5982\u6709\u914d\u7f6e\u6587\u4ef6\u5219\u53ef\u820d\u53bb\u6b64\u53c2\u6570\u3002\u9ed8\u8ba4\u503c\u4e3a",(0,l.jsx)(n.code,{children:"127.0.0.1"})," \u3002 |\n| -p       | string | \u6570\u636e\u5e93\u670d\u52a1\u5668\u7aef\u53e3\uff0c\u5982\u6709\u914d\u7f6e\u6587\u4ef6\u5219\u53ef\u820d\u53bb\u6b64\u53c2\u6570\u3002\u9ed8\u8ba4\u503c\u4e3a",(0,l.jsx)(n.code,{children:"7071"})," \u3002          |\n| -u       | string | \u6570\u636e\u5e93\u767b\u5f55\u7528\u6237\u540d\u3002                                                       |\n| -P       | string | \u6570\u636e\u5e93\u767b\u5f55\u5bc6\u7801\u3002                                                         |\n| -f       | string | \u5305\u542b\u5355\u6761 Cypher \u67e5\u8be2\u5355\u6587\u672c\u6587\u4ef6\u7684\u8def\u5f84\u3002                                   |\n| -s       | string | \u5355\u884c cypher \u67e5\u8be2\u547d\u4ee4\u3002\u4ee5",(0,l.jsx)(n.code,{children:'"'}),"\u5f00\u5934\u7ed3\u5c3e\u3002                                    |\n| -t       | int    | \u8fdb\u884c cypher \u67e5\u8be2\u65f6\u670d\u52a1\u5668\u7684\u8d85\u65f6\u9608\u503c\u3002\u9ed8\u8ba4\u503c\u4e3a",(0,l.jsx)(n.code,{children:"150"}),"\u79d2\u3002 -format            | string | \u67e5\u8be2\u7ed3\u679c\u663e\u793a\u6a21\u5f0f\u3002\u652f\u6301",(0,l.jsx)(n.code,{children:"plain"}),"\u4e0e",(0,l.jsx)(n.code,{children:"table"}),"\u4e24\u79cd\u683c\u5f0f\u3002",(0,l.jsx)(n.code,{children:"plain"}),"\u683c\u5f0f\u4f1a\u5c06\u67e5\u8be2\u7ed3\u679c\u5355\u5217\u6253\u5370\u3002",(0,l.jsx)(n.code,{children:"table"}),"\u683c\u5f0f\u4f1a\u5c06\u67e5\u8be2\u7ed3\u679c\u4ee5\u8868\u683c\u65b9\u5f0f\u663e\u793a\u3002\u9ed8\u8ba4\u503c\u4e3a",(0,l.jsx)(n.code,{children:"table"}),"\u3002 |"]}),"\n",(0,l.jsx)(n.h3,{id:"12\u547d\u4ee4\u793a\u4f8b",children:"1.2.\u547d\u4ee4\u793a\u4f8b:"}),"\n",(0,l.jsx)(n.p,{children:(0,l.jsx)(n.strong,{children:"cypher \u547d\u4ee4\u6587\u4ef6\u67e5\u8be2\uff1a"})}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-powershell",children:"$ ./lgraph_cypher.py -c /home/usr/lgraph_standalone.json -u user -P password -f /home/usr/cypher.json\n"})}),"\n",(0,l.jsx)(n.p,{children:(0,l.jsx)(n.strong,{children:"cypher \u547d\u4ee4\u5355\u53e5\u67e5\u8be2\uff1a"})}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-powershell",children:'$ ./lgraph_cypher.py -c /home/usr/lgraph_standalone.json -u user -P password -s "MATCH (n) RETURN n"\n'})}),"\n",(0,l.jsx)(n.h2,{id:"2\u4ea4\u4e92\u6a21\u5f0f",children:"2.\u4ea4\u4e92\u6a21\u5f0f"}),"\n",(0,l.jsxs)(n.p,{children:[(0,l.jsx)(n.code,{children:"lgraph_cypher"}),"\u4e5f\u53ef\u4ee5\u5728\u4ea4\u4e92\u6a21\u5f0f\u4e0b\u8fd0\u884c\u3002\u5728\u4ea4\u4e92\u5f0f\u6a21\u5f0f\u4e0b\uff0c\u5ba2\u6237\u7aef\u4e0e\u670d\u52a1\u5668\u4fdd\u6301\u8fde\u63a5\uff0c\u5e76\u5728\u8bfb\u53d6-\u8bc4\u4f30-\u6253\u5370-\u5faa\u73af\u4e2d\u4e0e\u7528\u6237\u8fdb\u884c\u4ea4\u4e92\u3002"]}),"\n",(0,l.jsx)(n.h3,{id:"21\u8fdb\u5165-lgraph_cypher-\u4ea4\u4e92\u6a21\u5f0f",children:"2.1.\u8fdb\u5165 lgraph_cypher \u4ea4\u4e92\u6a21\u5f0f:"}),"\n",(0,l.jsxs)(n.p,{children:["\u5982\u4e0d\u52a0",(0,l.jsx)(n.code,{children:"-f"}),"\u6216",(0,l.jsx)(n.code,{children:"-s"}),"\u547d\u4ee4\u884c\u9009\u9879\uff0c\u8fd0\u884c",(0,l.jsx)(n.code,{children:"lgraph_cypher"}),"\u65f6\u5c06\u4f1a\u8fdb\u5165\u4ea4\u4e92\u6a21\u5f0f\u3002\u4f7f\u7528\u65b9\u5f0f\u5982\u4e0b\uff1a"]}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{children:"$ ./lgraph_cypher.py -c /home/usr/lgraph_standalone.json -u admin -P 73@TuGraph\n"})}),"\n",(0,l.jsx)(n.p,{children:"\u5982\u6210\u529f\u8fdb\u5165\u5219\u4f1a\u663e\u793a\u76f8\u5e94\u767b\u5f55\u6210\u529f\u4fe1\u606f\uff1a"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{children:'**********************************************************************\n*                  TuGraph Graph Database X.Y.Z                      *\n*                                                                    *\n*        Copyright(C) 2023 Ant Group. All rights reserved.           *\n*                                                                    *\n**********************************************************************\nlogin success\n----------------------------------\nHost: 127.0.0.1\nPort: 7071\nUsername: admin\n----------------------------------\ntype ":help" to see all commands.\n>\n'})}),"\n",(0,l.jsxs)(n.p,{children:["\u73b0\u5728\u6211\u4eec\u4e5f\u63d0\u4f9b\u4e00\u4e2a\u4ea4\u4e92\u5f0f shell \uff0c\u7528\u4e8e\u7528\u6237\u8f93\u5165 Cypher \u67e5\u8be2\u8bed\u53e5\u6216\u4f7f\u7528",(0,l.jsx)(n.code,{children:":help"}),"\u547d\u4ee4\u6765\u68c0\u67e5\u53ef\u7528\u547d\u4ee4\u3002"]}),"\n",(0,l.jsx)(n.h3,{id:"22command\u79cd\u7c7b\u4e0e\u8bf4\u660e",children:"2.2.command\u79cd\u7c7b\u4e0e\u8bf4\u660e:"}),"\n",(0,l.jsxs)(n.p,{children:["\u9664 Cypher \u67e5\u8be2\u5916\uff0c",(0,l.jsx)(n.code,{children:"lgraph_cypher"})," \u7684 shell \u8fd8\u63a5\u53d7\u4ee5\u4e0b\u547d\u4ee4\uff1a"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(n.table,{children:[(0,l.jsx)(n.thead,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.th,{children:"\u547d\u4ee4"}),(0,l.jsx)(n.th,{children:"\u5bf9\u5e94\u53c2\u6570"}),(0,l.jsx)(n.th,{children:"\u8bf4\u660e"})]})}),(0,l.jsxs)(n.tbody,{children:[(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:":help"}),(0,l.jsx)(n.td,{children:"\\"}),(0,l.jsx)(n.td,{children:"\u663e\u793a\u670d\u52a1\u5668\u4fe1\u606f\u4e0e\u6240\u6709 command \u5bf9\u5e94\u8bf4\u660e\u3002"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:":db_info"}),(0,l.jsx)(n.td,{children:"\\"}),(0,l.jsx)(n.td,{children:"\u5f53\u524d\u670d\u52a1\u5668\u72b6\u6001\u67e5\u8be2\u3002\u5bf9\u5e94 REST API \u7684/db/info\u3002"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:":clear"}),(0,l.jsx)(n.td,{children:"\\"}),(0,l.jsx)(n.td,{children:"\u6e05\u7a7a\u5c4f\u5e55\u3002"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:":use"}),(0,l.jsx)(n.td,{children:"{\u56fe\u7684\u540d\u79f0}"}),(0,l.jsxs)(n.td,{children:["\u4f7f\u7528\u8be5\u540d\u79f0\u6307\u5b9a\u7684\u56fe\uff0c\u9ed8\u8ba4\u503c\u4e3a",(0,l.jsx)(n.code,{children:"default"})," \u3002"]})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:":source"}),(0,l.jsx)(n.td,{children:(0,l.jsx)(n.code,{children:"-t {\u67e5\u8be2timeout\u503c} -f {\u67e5\u8be2\u6587\u4ef6}"})}),(0,l.jsxs)(n.td,{children:["\u53ef\u4ea4\u4e92\u6a21\u5f0f\u4e0b\u7684 cypher \u547d\u4ee4\u6587\u4ef6\u67e5\u8be2\u3002\u8d85\u65f6\u9608\u503c\u9ed8\u8ba4\u503c\u4e3a",(0,l.jsx)(n.code,{children:"150"}),"\u79d2\u3002\u67e5\u8be2\u6587\u4ef6\u683c\u5f0f\u53c2\u8003\u65e0\u4ea4\u4e92\u5f0f\u67e5\u8be2\u53c2\u6570\u3002"]})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:":exit"}),(0,l.jsx)(n.td,{children:"\\"}),(0,l.jsx)(n.td,{children:"\u9000\u51fa\u4ea4\u4e92\u6a21\u5f0f\u5e76\u8fd4\u56de\u539f\u547d\u4ee4\u884c\u3002"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:":format"}),(0,l.jsxs)(n.td,{children:[(0,l.jsx)(n.code,{children:"plain"})," or ",(0,l.jsx)(n.code,{children:"table"})]}),(0,l.jsxs)(n.td,{children:["\u66f4\u6539 cypher \u67e5\u8be2\u7ed3\u679c\u7684\u663e\u793a\u6a21\u5f0f\u3002\u652f\u6301",(0,l.jsx)(n.code,{children:"plain"}),"\u4e0e",(0,l.jsx)(n.code,{children:"table"}),"\u6a21\u5f0f\u3002"]})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:":save all/command/result"}),(0,l.jsxs)(n.td,{children:[(0,l.jsx)(n.code,{children:"-f {\u6587\u4ef6\u8def\u5f84}"})," ",(0,l.jsx)(n.code,{children:"{cypher\u8bed\u53e5}"})]}),(0,l.jsxs)(n.td,{children:["\u5b58\u50a8 cypher \u547d\u4ee4\uff08command\uff09\u6216\u67e5\u8be2\u7ed3\u679c\uff08result\uff09\u6216\u4ee5\u4e0a\u4e8c\u8005\uff08all\uff09\u3002\u9ed8\u8ba4\u5b58\u50a8\u4f4d\u7f6e\u4e3a",(0,l.jsx)(n.code,{children:"/saved_cypher.txt"})]})]})]})]}),"\n",(0,l.jsx)(n.p,{children:(0,l.jsx)(n.strong,{children:"\u6ce8\u610f:"})}),"\n",(0,l.jsxs)(n.ul,{children:["\n",(0,l.jsxs)(n.li,{children:["\u6bcf\u6761\u547d\u4ee4\u90fd\u5e94\u8be5\u4ee5\u5192\u53f7\u5f00\u59cb ",(0,l.jsx)(n.code,{children:":"}),"."]}),"\n"]}),"\n",(0,l.jsx)(n.p,{children:(0,l.jsx)(n.strong,{children:":save \u547d\u4ee4\u4f8b\u5b50:"})}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{children:":save all -f /home/usr/saved.txt match (n) where return n, n.name limit 1000\n"})}),"\n",(0,l.jsx)(n.h3,{id:"23cypher-\u67e5\u8be2\u547d\u4ee4",children:"2.3.cypher \u67e5\u8be2\u547d\u4ee4:"}),"\n",(0,l.jsxs)(n.p,{children:['\u5728\u4ea4\u4e92\u6a21\u5f0f\u4e0b\uff0c\u7528\u6237\u4e5f\u53ef\u76f4\u63a5\u8f93\u5165\u5355\u53e5 cypher \u547d\u4ee4\u8fdb\u884c\u67e5\u8be2\uff0c\u4ee5"',(0,l.jsx)(n.code,{children:";"}),'"\u7ed3\u675f\u3002\u8f93\u5165\u547d\u4ee4\u4e0d\u533a\u5206\u5927\u5c0f\u5199\u3002\u4f8b\u5b50\u5982\u4e0b\uff1a']}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{children:"login success\n>MATCH (n) RETURN n, n.name;\n+---+---+-------------+\n|   | n |n.name       |\n+---+---+-------------+\n| 0 | 0 |david        |\n| 1 | 1 |Ann          |\n| 2 | 2 |first movie  |\n| 3 | 3 |Andres       |\n+---+---+-------------+\ntime spent: 0.000520706176758\nsize of query: 4\n>\n"})}),"\n",(0,l.jsxs)(n.p,{children:[(0,l.jsx)(n.code,{children:"lgraph_cypher"}),"\u8f93\u5165\u547d\u4ee4\u65f6\u652f\u6301\u591a\u884c\u8f93\u5165\uff0c\u7528\u6237\u53ef\u4f7f\u7528",(0,l.jsx)(n.code,{children:"ENTER"}),"\u952e\u5c06\u957f\u67e5\u8be2\u8bed\u53e5\u5206\u591a\u884c\u8f93\u5165\u3002\u591a\u884c\u8f93\u5165\u60c5\u51b5\u4e0b\u547d\u4ee4\u884c\u5f00\u5934\u4f1a\u4ece",(0,l.jsx)(n.code,{children:">"}),"\u53d8\u4e3a",(0,l.jsx)(n.code,{children:"=>"}),"\uff0c\u7136\u540e\u7528\u6237\u53ef\u4ee5\u7ee7\u7eed\u8f93\u5165\u67e5\u8be2\u7684\u5176\u4f59\u90e8\u5206\u3002"]}),"\n",(0,l.jsx)(n.p,{children:"\u4f8b\u5b50\u5982\u4e0b\uff1a"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{children:"login success\n>MATCH (n)\n=>WHERE n.uid='M11'\n=>RETURN n, n.name;\n"})}),"\n",(0,l.jsx)(n.h3,{id:"24\u8f85\u52a9\u529f\u80fd",children:"2.4.\u8f85\u52a9\u529f\u80fd:"}),"\n",(0,l.jsxs)(n.p,{children:[(0,l.jsx)(n.strong,{children:"\u5386\u53f2\u67e5\u8be2\uff1a"})," \u5728\u4ea4\u4e92\u6a21\u5f0f\u4e0b\u6309\u4e0a\u4e0b\u65b9\u5411\u952e\u53ef\u67e5\u8be2\u8f93\u5165\u5386\u53f2\u3002"]}),"\n",(0,l.jsxs)(n.p,{children:[(0,l.jsx)(n.strong,{children:"\u81ea\u52a8\u8865\u5168\uff1a"})," lgraph_cypher \u4f1a\u6839\u636e\u8f93\u5165\u5386\u53f2\u8fdb\u884c\u81ea\u52a8\u8865\u5168\u3002\u5728\u8865\u5168\u63d0\u793a\u51fa\u73b0\u7684\u60c5\u51b5\u4e0b\uff0c\u6309\u4e0b\u53f3\u65b9\u5411\u952e\u5c31\u4f1a\u81ea\u52a8\u8865\u5168\u547d\u4ee4\u3002"]})]})}function a(e={}){const{wrapper:n}={...(0,s.R)(),...e.components};return n?(0,l.jsx)(n,{...e,children:(0,l.jsx)(o,{...e})}):o(e)}},28453:(e,n,r)=>{r.d(n,{R:()=>d,x:()=>h});var l=r(96540);const s={},c=l.createContext(s);function d(e){const n=l.useContext(c);return l.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function h(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(s):e.components||s:d(e.components),l.createElement(c.Provider,{value:n},e.children)}}}]);