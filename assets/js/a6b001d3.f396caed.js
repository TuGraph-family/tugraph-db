"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[53698],{3400:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>c,contentTitle:()=>i,default:()=>p,frontMatter:()=>l,metadata:()=>d,toc:()=>a});var s=r(74848),t=r(28453);const l={},i="\u96c6\u6210\u6d4b\u8bd5",d={id:"quality/integration-testing",title:"\u96c6\u6210\u6d4b\u8bd5",description:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph \u7684\u96c6\u6210\u6d4b\u8bd5\u6846\u67b6\u5982\u4f55\u4f7f\u7528",source:"@site/versions/version-4.2.0/zh-CN/source/11.quality/2.integration-testing.md",sourceDirName:"11.quality",slug:"/quality/integration-testing",permalink:"/tugraph-db/zh/4.2.0/quality/integration-testing",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:2,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"\u5355\u5143\u6d4b\u8bd5",permalink:"/tugraph-db/zh/4.2.0/quality/unit-testing"},next:{title:"\u5982\u4f55\u8d21\u732e",permalink:"/tugraph-db/zh/4.2.0/contributor-manual/contributing"}},c={},a=[{value:"1.TuGraph\u96c6\u6210\u6d4b\u8bd5\u7684\u610f\u4e49",id:"1tugraph\u96c6\u6210\u6d4b\u8bd5\u7684\u610f\u4e49",level:2},{value:"2.TuGraph\u96c6\u6210\u6d4b\u8bd5\u6846\u67b6",id:"2tugraph\u96c6\u6210\u6d4b\u8bd5\u6846\u67b6",level:2},{value:"2.1.\u7ec4\u4ef6\u63cf\u8ff0",id:"21\u7ec4\u4ef6\u63cf\u8ff0",level:3},{value:"2.2.\u7ec4\u4ef6\u7528\u6cd5",id:"22\u7ec4\u4ef6\u7528\u6cd5",level:3},{value:"2.2.1.server",id:"221server",level:4},{value:"2.2.1.1.\u542f\u52a8\u53c2\u6570",id:"2211\u542f\u52a8\u53c2\u6570",level:5},{value:"2.2.1.2.\u542f\u52a8\u547d\u4ee4",id:"2212\u542f\u52a8\u547d\u4ee4",level:5},{value:"2.2.2.client",id:"222client",level:4},{value:"2.2.2.1.\u542f\u52a8\u53c2\u6570",id:"2221\u542f\u52a8\u53c2\u6570",level:5},{value:"2.2.2.2.\u542f\u52a8\u547d\u4ee4",id:"2222\u542f\u52a8\u547d\u4ee4",level:5},{value:"2.2.3.importor",id:"223importor",level:4},{value:"2.2.3.1.\u542f\u52a8\u53c2\u6570",id:"2231\u542f\u52a8\u53c2\u6570",level:5},{value:"2.2.3.2.\u542f\u52a8\u547d\u4ee4",id:"2232\u542f\u52a8\u547d\u4ee4",level:5},{value:"2.2.4.exportor",id:"224exportor",level:4},{value:"2.2.4.1.\u542f\u52a8\u53c2\u6570",id:"2241\u542f\u52a8\u53c2\u6570",level:5},{value:"2.2.4.2.\u542f\u52a8\u547d\u4ee4",id:"2242\u542f\u52a8\u547d\u4ee4",level:5},{value:"2.2.5.backup_binlog",id:"225backup_binlog",level:4},{value:"2.2.5.1.\u542f\u52a8\u53c2\u6570",id:"2251\u542f\u52a8\u53c2\u6570",level:5},{value:"2.2.5.2.\u542f\u52a8\u547d\u4ee4",id:"2252\u542f\u52a8\u547d\u4ee4",level:5},{value:"2.2.6.backup_copy_dir",id:"226backup_copy_dir",level:4},{value:"2.2.6.1.\u542f\u52a8\u53c2\u6570",id:"2261\u542f\u52a8\u53c2\u6570",level:5},{value:"2.2.6.2.\u542f\u52a8\u547d\u4ee4",id:"2262\u542f\u52a8\u547d\u4ee4",level:5},{value:"2.2.7.build_so",id:"227build_so",level:4},{value:"2.2.7.1.\u542f\u52a8\u53c2\u6570",id:"2271\u542f\u52a8\u53c2\u6570",level:5},{value:"2.2.7.2.\u542f\u52a8\u547d\u4ee4",id:"2272\u542f\u52a8\u547d\u4ee4",level:5},{value:"2.2.8.copy_snapshot",id:"228copy_snapshot",level:4},{value:"2.2.8.1.\u542f\u52a8\u53c2\u6570",id:"2281\u542f\u52a8\u53c2\u6570",level:5},{value:"2.2.8.2.\u542f\u52a8\u547d\u4ee4",id:"2282\u542f\u52a8\u547d\u4ee4",level:5},{value:"2.2.9.copy_dir",id:"229copy_dir",level:4},{value:"2.2.9.1.\u542f\u52a8\u53c2\u6570",id:"2291\u542f\u52a8\u53c2\u6570",level:5},{value:"2.2.9.2.\u542f\u52a8\u547d\u4ee4",id:"2292\u542f\u52a8\u547d\u4ee4",level:5},{value:"2.2.10.exec",id:"2210exec",level:4},{value:"2.2.10.1.\u542f\u52a8\u53c2\u6570",id:"22101\u542f\u52a8\u53c2\u6570",level:5},{value:"2.2.10.2.\u542f\u52a8\u547d\u4ee4",id:"22102\u542f\u52a8\u547d\u4ee4",level:5},{value:"2.2.11.algo",id:"2211algo",level:4},{value:"2.2.11.1.\u542f\u52a8\u53c2\u6570",id:"22111\u542f\u52a8\u53c2\u6570",level:5},{value:"2.2.11.2.\u542f\u52a8\u547d\u4ee4",id:"22112\u542f\u52a8\u547d\u4ee4",level:5},{value:"2.2.12.bash",id:"2212bash",level:4},{value:"2.2.12.1.\u542f\u52a8\u53c2\u6570",id:"22121\u542f\u52a8\u53c2\u6570",level:5},{value:"2.2.12.2.\u542f\u52a8\u547d\u4ee4",id:"22122\u542f\u52a8\u547d\u4ee4",level:5},{value:"2.2.13.rest",id:"2213rest",level:4},{value:"2.2.13.1.\u542f\u52a8\u53c2\u6570",id:"22131\u542f\u52a8\u53c2\u6570",level:5},{value:"2.2.13.2.\u542f\u52a8\u547d\u4ee4",id:"22132\u542f\u52a8\u547d\u4ee4",level:5},{value:"2.3.\u6d4b\u8bd5\u6837\u4f8b",id:"23\u6d4b\u8bd5\u6837\u4f8b",level:3},{value:"2.3.1.rest",id:"231rest",level:4},{value:"2.3.2.client",id:"232client",level:4},{value:"2.3.3.exportor/importor",id:"233exportorimportor",level:4},{value:"2.3.4.\u5176\u4ed6\u6d4b\u8bd5",id:"234\u5176\u4ed6\u6d4b\u8bd5",level:4}];function h(e){const n={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",h4:"h4",h5:"h5",header:"header",li:"li",p:"p",pre:"pre",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",ul:"ul",...(0,t.R)(),...e.components};return(0,s.jsxs)(s.Fragment,{children:[(0,s.jsx)(n.header,{children:(0,s.jsx)(n.h1,{id:"\u96c6\u6210\u6d4b\u8bd5",children:"\u96c6\u6210\u6d4b\u8bd5"})}),"\n",(0,s.jsxs)(n.blockquote,{children:["\n",(0,s.jsx)(n.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph \u7684\u96c6\u6210\u6d4b\u8bd5\u6846\u67b6\u5982\u4f55\u4f7f\u7528"}),"\n"]}),"\n",(0,s.jsx)(n.h2,{id:"1tugraph\u96c6\u6210\u6d4b\u8bd5\u7684\u610f\u4e49",children:"1.TuGraph\u96c6\u6210\u6d4b\u8bd5\u7684\u610f\u4e49"}),"\n",(0,s.jsx)(n.p,{children:"\u5728\u5355\u5143\u6d4b\u8bd5\u4e0e\u529f\u80fd\u6d4b\u8bd5\u4e2d\uff0c\u6709\u90e8\u5206\u7528\u4f8b\u76f4\u63a5\u5f00\u542fgalaxy\u6216statemachine\u6765\u8fdb\u884c\u6d4b\u8bd5\uff0c\u8fd9\u5e76\u4e0d\u662f\u4e00\u4e2a\u5b8c\u6574\u7684\u6d41\u7a0b\u3002\u5728\u5b8c\u6574\u7684cs\u67b6\u6784\u4e2d\uff0c\u7528\u6237\u8bf7\u6c42\u662f\u901a\u8fc7\u5ba2\u6237\u7aef\u53d1\u5f80\u670d\u52a1\u7aef\uff0c\u7f51\u7edc\u901a\u4fe1\u662f\u5fc5\u4e0d\u53ef\u5c11\u7684\uff0c\u4e3a\u4e86\u907f\u514d\u5355\u5143\u6d4b\u8bd5\u4e0d\u5b8c\u6574\u5e26\u6765\u7684bug\uff0c\u9488\u5bf9\u8fd9\u79cd\u60c5\u51b5\uff0c\u4f7f\u7528\u96c6\u6210\u6d4b\u8bd5\u6846\u67b6\u8fdb\u884c\u5168\u94fe\u8def\u7684\u5b8c\u6574\u6d4b\u8bd5\u3002"}),"\n",(0,s.jsx)(n.h2,{id:"2tugraph\u96c6\u6210\u6d4b\u8bd5\u6846\u67b6",children:"2.TuGraph\u96c6\u6210\u6d4b\u8bd5\u6846\u67b6"}),"\n",(0,s.jsx)(n.p,{children:"TuGraph\u91c7\u7528pytest\u6846\u67b6\u4f5c\u4e3a\u81ea\u5df1\u7684\u96c6\u6210\u6d4b\u8bd5\u6846\u67b6\uff0cpytest\u6846\u67b6\u4f5c\u4e3a\u76ee\u524d\u4f7f\u7528\u6700\u5e7f\u6cdb\u7684cs\u7aef\u96c6\u6210\u6d4b\u8bd5\u6846\u67b6\uff0c\u4ee5\u5176\u7075\u6d3b\u7b80\u5355\uff0c\u5bb9\u6613\u4e0a\u624b\uff0c\u5e76\u4e14\u652f\u6301\u53c2\u6570\u5316\u7684\u4f7f\u7528\u65b9\u5f0f\u800c\u8457\u79f0\uff0cTuGraph\u57fa\u4e8epytest\u63d0\u4f9b\u7684\u529f\u80fd\uff0c\u62bd\u8c61\u51fa\u4e86\u4e0d\u540c\u7684\u5de5\u5177\uff0c\u901a\u8fc7\u53c2\u6570\u6765\u63a7\u5236\u5404\u4e2a\u5de5\u5177\u7684\u5904\u7406\u903b\u8f91\uff0c\u4ee5\u65b9\u4fbf\u5927\u5bb6\u8fdb\u884c\u9ad8\u6548\u7684\u6d4b\u8bd5\u4ee3\u7801\u5f00\u53d1\u3002"}),"\n",(0,s.jsxs)(n.p,{children:["\u66f4\u591apytest\u4fe1\u606f\u8bf7\u53c2\u8003\u5b98\u7f51: ",(0,s.jsx)(n.a,{href:"https://docs.pytest.org/en/7.2.x/getting-started.html",children:"https://docs.pytest.org/en/7.2.x/getting-started.html"})]}),"\n",(0,s.jsx)(n.h3,{id:"21\u7ec4\u4ef6\u63cf\u8ff0",children:"2.1.\u7ec4\u4ef6\u63cf\u8ff0"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"\u7ec4\u4ef6\u540d\u79f0"}),(0,s.jsx)(n.th,{children:"\u7ec4\u4ef6\u529f\u80fd"}),(0,s.jsx)(n.th,{children:"\u5b9e\u73b0\u65b9\u5f0f"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"server"}),(0,s.jsx)(n.td,{children:"TuGraph\u5355\u673a\u670d\u52a1"}),(0,s.jsx)(n.td,{children:"\u5f00\u542f\u5b50\u8fdb\u7a0b\u5e76\u5728\u5b50\u8fdb\u7a0b\u4e2d\u542f\u52a8\u670d\u52a1"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"client"}),(0,s.jsx)(n.td,{children:"TuGraph Rpc Client"}),(0,s.jsx)(n.td,{children:"\u5f53\u524d\u8fdb\u7a0b\u4e2d\u5f00\u542fTuGraph Python Rpc Client\u53d1\u9001\u8bf7\u6c42"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"importor"}),(0,s.jsx)(n.td,{children:"TuGraph Importor"}),(0,s.jsx)(n.td,{children:"\u5f00\u542f\u5b50\u8fdb\u7a0b\u5e76\u5728\u5b50\u8fdb\u7a0b\u4e2d\u5904\u7406\u5bfc\u5165\u8bf7\u6c42"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"exportor"}),(0,s.jsx)(n.td,{children:"TuGraph Exportor"}),(0,s.jsx)(n.td,{children:"\u5f00\u542f\u5b50\u8fdb\u7a0b\u5e76\u5728\u5b50\u8fdb\u7a0b\u4e2d\u5904\u7406\u5bfc\u51fa\u8bf7\u6c42"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"backup_binlog"}),(0,s.jsx)(n.td,{children:"TuGraph Backup Binlog"}),(0,s.jsx)(n.td,{children:"\u5f00\u542f\u5b50\u8fdb\u7a0b\u5e76\u5728\u5b50\u8fdb\u7a0b\u4e2d\u5904\u7406\u5907\u4efdbinlog\u7684\u8bf7\u6c42"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"backup_copy_dir"}),(0,s.jsx)(n.td,{children:"TuGraph Backup"}),(0,s.jsx)(n.td,{children:"\u5f00\u542f\u5b50\u8fdb\u7a0b\u5e76\u5728\u5b50\u8fdb\u7a0b\u4e2d\u5904\u7406\u5907\u4efd\u5b8c\u6574db\u7684\u8bf7\u6c42"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"build_so"}),(0,s.jsx)(n.td,{children:"\u7f16\u8bd1c++\u52a8\u6001\u8fde\u63a5\u5e93\u7684\u7ec4\u4ef6"}),(0,s.jsx)(n.td,{children:"\u5f00\u542f\u5b50\u8fdb\u7a0b\u5e76\u5728\u5b50\u8fdb\u7a0b\u4e2d\u5904\u7406gcc\u7f16\u8bd1\u903b\u8f91"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"copy_snapshot"}),(0,s.jsx)(n.td,{children:"TuGraph Copy Snapshot"}),(0,s.jsx)(n.td,{children:"\u5f53\u524d\u8fdb\u7a0b\u4e2d\u5904\u7406\u5907\u4efdsnapshot\u7684\u8bf7\u6c42"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"copydir"}),(0,s.jsx)(n.td,{children:"\u6587\u4ef6\u5939\u62f7\u8d1d"}),(0,s.jsx)(n.td,{children:"\u5f53\u524d\u8fdb\u7a0b\u4e2d\u5904\u7406\u6587\u4ef6\u5939\u62f7\u8d1d\u8bf7\u6c42"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"exec"}),(0,s.jsx)(n.td,{children:"\u6267\u884cc++/java\u53ef\u6267\u884c\u6587\u4ef6"}),(0,s.jsx)(n.td,{children:"\u5f00\u542f\u5b50\u8fdb\u7a0b\u5e76\u5728\u5b50\u8fdb\u7a0b\u4e2d\u542f\u52a8C++\u53ef\u6267\u884c\u6587\u4ef6"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"algo"}),(0,s.jsx)(n.td,{children:"\u6267\u884c\u7b97\u6cd5"}),(0,s.jsx)(n.td,{children:"\u5f00\u542f\u5b50\u8fdb\u7a0b\u5e76\u5728\u5b50\u8fdb\u7a0b\u4e2d\u6267\u884c\u7b97\u6cd5"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"bash"}),(0,s.jsx)(n.td,{children:"\u6267\u884cbash\u547d\u4ee4"}),(0,s.jsx)(n.td,{children:"\u5f00\u542f\u5b50\u8fdb\u7a0b\u5e76\u5728\u5b50\u8fdb\u7a0b\u4e2d\u6267\u884cbash\u547d\u4ee4"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"rest"}),(0,s.jsx)(n.td,{children:"TuGraph Python Rest Client"}),(0,s.jsx)(n.td,{children:"\u5f53\u524d\u8fdb\u7a0b\u4e2d\u5f00\u542fTuGraph Python Rest Client\u53d1\u9001\u8bf7\u6c42"})]})]})]}),"\n",(0,s.jsx)(n.h3,{id:"22\u7ec4\u4ef6\u7528\u6cd5",children:"2.2.\u7ec4\u4ef6\u7528\u6cd5"}),"\n",(0,s.jsx)(n.h4,{id:"221server",children:"2.2.1.server"}),"\n",(0,s.jsx)(n.h5,{id:"2211\u542f\u52a8\u53c2\u6570",children:"2.2.1.1.\u542f\u52a8\u53c2\u6570"}),"\n",(0,s.jsx)(n.p,{children:"\u91c7\u7528python\u5b57\u5178\u4f20\u5165"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"cmd\u662f\u542f\u52a8\u547d\u4ee4"}),"\n",(0,s.jsx)(n.li,{children:"cleanup_dir\u662f\u6267\u884c\u5b8c\u6210\u540e\u9700\u8981\u6e05\u7406\u7684\u76ee\u5f55\uff0c\u53ef\u4ee5\u662f\u591a\u4e2a\uff0c\u901a\u8fc7python\u5217\u8868\u4f20\u5165"}),"\n"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --license _FMA_IGNORE_LICENSE_CHECK_SALTED_ --port 7072 --rpc_port 9092",\n             "cleanup_dir":["./testdb"]}\n'})}),"\n",(0,s.jsx)(n.h5,{id:"2212\u542f\u52a8\u547d\u4ee4",children:"2.2.1.2.\u542f\u52a8\u547d\u4ee4"}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7fixtures\u7ec4\u4ef6\u5f15\u5165\u5de5\u5177\uff0c\u5e76\u901a\u8fc7\u542f\u52a8\u53c2\u6570\u6765\u63a7\u5236\u4e0d\u540c\u7684\u5904\u7406\u903b\u8f91\uff0c\u51fd\u6570\u5f00\u59cb\u6267\u884c\u524d\u4f1a\u542f\u52a8server\uff0c\u51fd\u6570\u6267\u884c\u5b8c\u6210\u540e\u4f1a\u505c\u6b62server\uff0c\u5e76\u6e05\u7406cleanup_dir\u6307\u5b9a\u7684\u76ee\u5f55"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'@pytest.mark.parametrize("server", [SERVEROPT], indirect=True)\ndef test_server(self, server):\n    pass\n'})}),"\n",(0,s.jsx)(n.h4,{id:"222client",children:"2.2.2.client"}),"\n",(0,s.jsx)(n.h5,{id:"2221\u542f\u52a8\u53c2\u6570",children:"2.2.2.1.\u542f\u52a8\u53c2\u6570"}),"\n",(0,s.jsx)(n.p,{children:"\u91c7\u7528python\u5b57\u5178\u4f20\u5165"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"host\u662fTuGraph Server\u7684ip\u548c\u7aef\u53e3"}),"\n",(0,s.jsx)(n.li,{children:"user\u662fTuGraph Server\u7684\u7528\u6237\u540d"}),"\n",(0,s.jsx)(n.li,{children:"password\u662fTuGraph Server \u4e2duser\u5bf9\u5e94\u7684\u5bc6\u7801"}),"\n"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'CLIENTOPT = {"host":"127.0.0.1:9092", "user":"admin", "password":"73@TuGraph"}\n'})}),"\n",(0,s.jsx)(n.h5,{id:"2222\u542f\u52a8\u547d\u4ee4",children:"2.2.2.2.\u542f\u52a8\u547d\u4ee4"}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7fixtures\u7ec4\u4ef6\u5f15\u5165\u5de5\u5177\uff0c\u5e76\u901a\u8fc7\u542f\u52a8\u53c2\u6570\u6765\u63a7\u5236\u4e0d\u540c\u7684\u5904\u7406\u903b\u8f91\uff0c\u51fd\u6570\u5f00\u59cb\u6267\u884c\u524d\u4f1a\u542f\u52a8\u5ba2\u6237\u7aef\uff0c\u51fd\u6570\u6267\u884c\u7ed3\u675f\u540e\u4f1a\u7ed3\u675f\u5ba2\u6237\u7aef"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:"@pytest.mark.parametrize(\"server\", [SERVEROPT], indirect=True)\n@pytest.mark.parametrize(\"client\", [CLIENTOPT], indirect=True)\ndef test_client(self, server, client):\n    ret = client.callCypher(\"CALL db.createEdgeLabel('followed', '[]', 'address', string, false, 'date', int32, false)\", \"default\")\n    assert ret[0]\n    ret = client.callCypher(\"CALL db.createEdgeLabel('followed', '[]', 'address', string, false, 'date', int32, false)\", \"default\")\n    assert ret[0] == False\n"})}),"\n",(0,s.jsx)(n.h4,{id:"223importor",children:"2.2.3.importor"}),"\n",(0,s.jsx)(n.h5,{id:"2231\u542f\u52a8\u53c2\u6570",children:"2.2.3.1.\u542f\u52a8\u53c2\u6570"}),"\n",(0,s.jsx)(n.p,{children:"\u91c7\u7528python\u5b57\u5178\u4f20\u5165"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"cmd\u662f\u542f\u52a8\u547d\u4ee4"}),"\n",(0,s.jsx)(n.li,{children:"cleanup_dir\u662f\u6267\u884c\u5b8c\u6210\u540e\u9700\u8981\u6e05\u7406\u7684\u76ee\u5f55\uff0c\u53ef\u4ee5\u662f\u591a\u4e2a\uff0c\u901a\u8fc7python\u5217\u8868\u4f20\u5165"}),"\n"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'IMPORTOPT = {"cmd":"./lgraph_import --config_file ./data/yago/yago.conf --dir ./testdb --user admin --password 73@TuGraph --graph default --overwrite 1",\n             "cleanup_dir":["./testdb", "./.import_tmp"]}\n'})}),"\n",(0,s.jsx)(n.h5,{id:"2232\u542f\u52a8\u547d\u4ee4",children:"2.2.3.2.\u542f\u52a8\u547d\u4ee4"}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7fixtures\u7ec4\u4ef6\u5f15\u5165\u5de5\u5177\uff0c\u5e76\u901a\u8fc7\u542f\u52a8\u53c2\u6570\u6765\u63a7\u5236\u5bfc\u5165\u4e0d\u540c\u7684\u6570\u636e\uff0c\u51fd\u6570\u5f00\u59cb\u6267\u884c\u524d\u4f1a\u5bfc\u5165\u6570\u636e\u5230\u6307\u5b9a\u7684\u76ee\u5f55\uff0c\u51fd\u6570\u6267\u884c\u5b8c\u6210\u540e\u4f1a\u6e05\u7406cleanup_dir\u6307\u5b9a\u7684\u76ee\u5f55"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'@pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)\ndef test_importor(self, importor):\n    pass\n'})}),"\n",(0,s.jsx)(n.h4,{id:"224exportor",children:"2.2.4.exportor"}),"\n",(0,s.jsx)(n.h5,{id:"2241\u542f\u52a8\u53c2\u6570",children:"2.2.4.1.\u542f\u52a8\u53c2\u6570"}),"\n",(0,s.jsx)(n.p,{children:"\u91c7\u7528python\u5b57\u5178\u4f20\u5165"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"cmd\u662f\u542f\u52a8\u547d\u4ee4"}),"\n",(0,s.jsx)(n.li,{children:"cleanup_dir\u662f\u6267\u884c\u5b8c\u6210\u540e\u9700\u8981\u6e05\u7406\u7684\u76ee\u5f55\uff0c\u53ef\u4ee5\u662f\u591a\u4e2a\uff0c\u901a\u8fc7python\u5217\u8868\u4f20\u5165"}),"\n"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'EXPORT_DEF_OPT = {"cmd":"./lgraph_export -d ./testdb -e ./export/default -g default -u admin -p 73@TuGraph",\n                  "cleanup_dir":["./export"]}\n'})}),"\n",(0,s.jsx)(n.h5,{id:"2242\u542f\u52a8\u547d\u4ee4",children:"2.2.4.2.\u542f\u52a8\u547d\u4ee4"}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7fixtures\u7ec4\u4ef6\u5f15\u5165\u5de5\u5177\uff0c\u5e76\u901a\u8fc7\u542f\u52a8\u53c2\u6570\u6765\u63a7\u5236\u5bfc\u51fa\u4e0d\u540c\u7684\u6570\u636e\uff0c\u51fd\u6570\u5f00\u59cb\u6267\u884c\u524d\u4f1a\u5bfc\u51fa\u6570\u636e\u5230\u6307\u5b9a\u7684\u76ee\u5f55\uff0c\u51fd\u6570\u6267\u884c\u5b8c\u6210\u540e\u4f1a\u6e05\u7406cleanup_dir\u6307\u5b9a\u7684\u76ee\u5f55"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'@pytest.mark.parametrize("exportor", [EXPORT_DEF_OPT], indirect=True)\ndef test_exportor(self, exportor):\n    pass\n'})}),"\n",(0,s.jsx)(n.h4,{id:"225backup_binlog",children:"2.2.5.backup_binlog"}),"\n",(0,s.jsx)(n.h5,{id:"2251\u542f\u52a8\u53c2\u6570",children:"2.2.5.1.\u542f\u52a8\u53c2\u6570"}),"\n",(0,s.jsx)(n.p,{children:"\u91c7\u7528python\u5b57\u5178\u4f20\u5165"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"cmd\u662f\u542f\u52a8\u547d\u4ee4"}),"\n",(0,s.jsx)(n.li,{children:"cleanup_dir\u662f\u6267\u884c\u5b8c\u6210\u540e\u9700\u8981\u6e05\u7406\u7684\u76ee\u5f55\uff0c\u53ef\u4ee5\u662f\u591a\u4e2a\uff0c\u901a\u8fc7python\u5217\u8868\u4f20\u5165"}),"\n"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'BINLOGOPT = {"cmd" : "./lgraph_binlog -a restore --host 127.0.0.1 --port 9093 -u admin -p 73@TuGraph -f ./testdb/binlog/*",\n             "cleanup_dir":[]}\n'})}),"\n",(0,s.jsx)(n.h5,{id:"2252\u542f\u52a8\u547d\u4ee4",children:"2.2.5.2.\u542f\u52a8\u547d\u4ee4"}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7fixtures\u7ec4\u4ef6\u5f15\u5165\u5de5\u5177\uff0c\u5e76\u901a\u8fc7\u542f\u52a8\u53c2\u6570\u6765\u63a7\u5236\u5907\u4efd\u4e0d\u540c\u7684binlog\uff0c\u51fd\u6570\u5f00\u59cb\u6267\u884c\u524d\u4f1a\u62f7\u8d1dbinlog\u5230\u6307\u5b9a\u7684\u76ee\u5f55\uff0c\u51fd\u6570\u6267\u884c\u5b8c\u6210\u540e\u4f1a\u6e05\u7406cleanup_dir\u6307\u5b9a\u7684\u76ee\u5f55"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'@pytest.mark.parametrize("backup_binlog", [BINLOGOPT], indirect=True)\ndef test_backup_binlog(self, backup_binlog):\n    pass\n'})}),"\n",(0,s.jsx)(n.h4,{id:"226backup_copy_dir",children:"2.2.6.backup_copy_dir"}),"\n",(0,s.jsx)(n.h5,{id:"2261\u542f\u52a8\u53c2\u6570",children:"2.2.6.1.\u542f\u52a8\u53c2\u6570"}),"\n",(0,s.jsx)(n.p,{children:"\u91c7\u7528python\u5b57\u5178\u4f20\u5165"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"cmd\u662f\u542f\u52a8\u547d\u4ee4"}),"\n",(0,s.jsx)(n.li,{children:"cleanup_dir\u662f\u6267\u884c\u5b8c\u6210\u540e\u9700\u8981\u6e05\u7406\u7684\u76ee\u5f55\uff0c\u53ef\u4ee5\u662f\u591a\u4e2a\uff0c\u901a\u8fc7python\u5217\u8868\u4f20\u5165"}),"\n"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'BACKUPOPT = {"cmd" : "./lgraph_backup --src ./testdb -dst ./testdb1",\n             "cleanup_dir":[]}\n'})}),"\n",(0,s.jsx)(n.h5,{id:"2262\u542f\u52a8\u547d\u4ee4",children:"2.2.6.2.\u542f\u52a8\u547d\u4ee4"}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7fixtures\u7ec4\u4ef6\u5f15\u5165\u5de5\u5177\uff0c\u5e76\u901a\u8fc7\u542f\u52a8\u53c2\u6570\u6765\u63a7\u5236\u5907\u4efd\u4e0d\u540c\u7684db\uff0c\u51fd\u6570\u5f00\u59cb\u6267\u884c\u524d\u4f1a\u62f7\u8d1ddb\u5230\u6307\u5b9a\u7684\u76ee\u5f55\uff0c\u51fd\u6570\u6267\u884c\u5b8c\u6210\u540e\u4f1a\u6e05\u7406cleanup_dir\u6307\u5b9a\u7684\u76ee\u5f55"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'@pytest.mark.parametrize("backup_copy_dir", [BACKUPOPT], indirect=True)\ndef test_backup_copy_dir(self, backup_copy_dir):\n\tpass\n'})}),"\n",(0,s.jsx)(n.h4,{id:"227build_so",children:"2.2.7.build_so"}),"\n",(0,s.jsx)(n.h5,{id:"2271\u542f\u52a8\u53c2\u6570",children:"2.2.7.1.\u542f\u52a8\u53c2\u6570"}),"\n",(0,s.jsx)(n.p,{children:"\u91c7\u7528python\u5b57\u5178\u4f20\u5165"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"cmd\u662f\u542f\u52a8\u547d\u4ee4\uff0c\u91c7\u7528python\u5217\u8868\u4f20\u5165\uff0c\u53ef\u4ee5\u4e00\u6b21\u7f16\u8bd1\u591a\u4e2aso"}),"\n",(0,s.jsx)(n.li,{children:"so_name\u662f\u6267\u884c\u5b8c\u6210\u540e\u9700\u8981\u6e05\u7406\u7684so\uff0c\u53ef\u4ee5\u662f\u591a\u4e2a\uff0c\u901a\u8fc7python\u5217\u8868\u4f20\u5165"}),"\n"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'BUILDOPT = {"cmd":["g++ -fno-gnu-unique -fPIC -g --std=c++17 -I ../../include -I ../../deps/install/include -rdynamic -O3 -fopenmp -DNDEBUG -o ./scan_graph.so ../../test/test_procedures/scan_graph.cpp ./liblgraph.so -shared",\n                       "g++ -fno-gnu-unique -fPIC -g --std=c++17 -I ../../include -I ../../deps/install/include -rdynamic -O3 -fopenmp -DNDEBUG -o ./sortstr.so ../../test/test_procedures/sortstr.cpp ./liblgraph.so -shared"],\n                "so_name":["./scan_graph.so", "./sortstr.so"]}\n'})}),"\n",(0,s.jsx)(n.h5,{id:"2272\u542f\u52a8\u547d\u4ee4",children:"2.2.7.2.\u542f\u52a8\u547d\u4ee4"}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7fixtures\u7ec4\u4ef6\u5f15\u5165\u5de5\u5177\uff0c\u5e76\u901a\u8fc7\u542f\u52a8\u53c2\u6570\u6765\u63a7\u5236\u7f16\u8bd1\u4e0d\u540c\u7684so\uff0c\u51fd\u6570\u5f00\u59cb\u6267\u884c\u524d\u4f1a\u751f\u6210so\u5230\u6307\u5b9a\u7684\u76ee\u5f55\uff0c\u51fd\u6570\u6267\u884c\u5b8c\u6210\u540e\u4f1a\u6e05\u7406so_name\u5217\u8868\u6307\u5b9a\u7684\u52a8\u6001\u5e93"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'@pytest.mark.parametrize("build_so", [BUILDOPT], indirect=True)\ndef test_build_so(self, build_so):\n    pass\n'})}),"\n",(0,s.jsx)(n.h4,{id:"228copy_snapshot",children:"2.2.8.copy_snapshot"}),"\n",(0,s.jsx)(n.h5,{id:"2281\u542f\u52a8\u53c2\u6570",children:"2.2.8.1.\u542f\u52a8\u53c2\u6570"}),"\n",(0,s.jsx)(n.p,{children:"\u91c7\u7528python\u5b57\u5178\u4f20\u5165"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"src\u662f\u539fdb"}),"\n",(0,s.jsx)(n.li,{children:"dst\u662f\u62f7\u8d1d\u540e\u7684snapshot"}),"\n"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'COPYSNAPOPT = {"src" : "./testdb", "dst" : "./testdb1"}\n'})}),"\n",(0,s.jsx)(n.h5,{id:"2282\u542f\u52a8\u547d\u4ee4",children:"2.2.8.2.\u542f\u52a8\u547d\u4ee4"}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7fixtures\u7ec4\u4ef6\u5f15\u5165\u5de5\u5177\uff0c\u5e76\u901a\u8fc7\u542f\u52a8\u53c2\u6570\u6765\u63a7\u5236\u62f7\u8d1d\u4e0d\u540c\u7684snapshot\uff0c\u51fd\u6570\u5f00\u59cb\u6267\u884c\u524d\u4f1a\u62f7\u8d1dsrc\u4e2d\u7684snapshot\u5230dst\u6307\u5b9a\u7684\u76ee\u5f55"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'@pytest.mark.parametrize("copy_snapshot", [COPYSNAPOPT], indirect=True)\ndef test_copy_snapshot(self, copy_snapshot):\n    pass\n'})}),"\n",(0,s.jsx)(n.h4,{id:"229copy_dir",children:"2.2.9.copy_dir"}),"\n",(0,s.jsx)(n.h5,{id:"2291\u542f\u52a8\u53c2\u6570",children:"2.2.9.1.\u542f\u52a8\u53c2\u6570"}),"\n",(0,s.jsx)(n.p,{children:"\u91c7\u7528python\u5b57\u5178\u4f20\u5165"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"src\u662f\u539fdb"}),"\n",(0,s.jsx)(n.li,{children:"dst\u662f\u62f7\u8d1d\u540e\u7684snapshot"}),"\n"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'COPYSNAPOPT = {"src" : "./testdb", "dst" : "./testdb1"}\n'})}),"\n",(0,s.jsx)(n.h5,{id:"2292\u542f\u52a8\u547d\u4ee4",children:"2.2.9.2.\u542f\u52a8\u547d\u4ee4"}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7fixtures\u7ec4\u4ef6\u5f15\u5165\u5de5\u5177\uff0c\u5e76\u901a\u8fc7\u542f\u52a8\u53c2\u6570\u6765\u63a7\u5236\u62f7\u8d1d\u4e0d\u540c\u7684\u76ee\u5f55\uff0c\u51fd\u6570\u5f00\u59cb\u6267\u884c\u524d\u4f1a\u62f7\u8d1dsrc\u5230dst\u6307\u5b9a\u7684\u76ee\u5f55"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'@pytest.mark.parametrize("copy_dir", [COPYDIR], indirect=True)\ndef test_copy_dir(self, copy_dir):\n    pass\n'})}),"\n",(0,s.jsx)(n.h4,{id:"2210exec",children:"2.2.10.exec"}),"\n",(0,s.jsx)(n.h5,{id:"22101\u542f\u52a8\u53c2\u6570",children:"2.2.10.1.\u542f\u52a8\u53c2\u6570"}),"\n",(0,s.jsx)(n.p,{children:"\u91c7\u7528python\u5b57\u5178\u4f20\u5165"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"cmd\u662f\u542f\u52a8\u547d\u4ee4"}),"\n"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'EXECOPT = {\n        "cmd" : "test_rpc_client/cpp/CppClientTest/build/clienttest"\n    }\n'})}),"\n",(0,s.jsx)(n.h5,{id:"22102\u542f\u52a8\u547d\u4ee4",children:"2.2.10.2.\u542f\u52a8\u547d\u4ee4"}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7fixtures\u7ec4\u4ef6\u5f15\u5165\u5de5\u5177\uff0c\u5e76\u901a\u8fc7\u542f\u52a8\u53c2\u6570\u6765\u63a7\u5236\u6267\u884c\u4e0d\u540c\u7684\u903b\u8f91\uff0c\u51fd\u6570\u5f00\u59cb\u6267\u884c\u524d\u4f1a\u5f00\u542f\u5b50\u8fdb\u7a0b\u6267\u884c\u901a\u8fc7cmd\u53c2\u6570\u4f20\u5165\u7684\u547d\u4ee4"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'@pytest.mark.parametrize("exec", [EXECOPT], indirect=True)\ndef test_exec(self, exec):\n        pass\n'})}),"\n",(0,s.jsx)(n.h4,{id:"2211algo",children:"2.2.11.algo"}),"\n",(0,s.jsx)(n.h5,{id:"22111\u542f\u52a8\u53c2\u6570",children:"2.2.11.1.\u542f\u52a8\u53c2\u6570"}),"\n",(0,s.jsx)(n.p,{children:"\u91c7\u7528python\u5b57\u5178\u4f20\u5165"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"cmd\u662f\u542f\u52a8\u547d\u4ee4"}),"\n",(0,s.jsx)(n.li,{children:"result\u662f\u7b97\u6cd5\u9884\u671f\u7684\u6267\u884c\u7ed3\u679c\uff0c\u6267\u884c\u5b8c\u6210\u4f1a\u901a\u8fc7\u5b9e\u9645\u7ed3\u679c\u4e0e\u9884\u671f\u7ed3\u679c\u8fdb\u884c\u6bd4\u8f83\uff0c\u4e0d\u540c\u5219\u6d4b\u8bd5\u5931\u8d25"}),"\n"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'BFSEMBEDOPT = {\n        "cmd" : "algo/bfs_embed ./testdb",\n        "result" : ["found_vertices = 3829"]\n    }\n'})}),"\n",(0,s.jsx)(n.h5,{id:"22112\u542f\u52a8\u547d\u4ee4",children:"2.2.11.2.\u542f\u52a8\u547d\u4ee4"}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7fixtures\u7ec4\u4ef6\u5f15\u5165\u5de5\u5177\uff0c\u5e76\u901a\u8fc7\u542f\u52a8\u53c2\u6570\u6765\u63a7\u5236\u6267\u884c\u4e0d\u540c\u7684\u7b97\u6cd5\u903b\u8f91\uff0c\u51fd\u6570\u5f00\u59cb\u6267\u884c\u524d\u4f1a\u5f00\u542f\u5b50\u8fdb\u7a0b\u6267\u884c\u901a\u8fc7cmd\u53c2\u6570\u4f20\u5165\u7684\u7b97\u6cd5\uff0c\u51fd\u6570\u4e3b\u4f53\u7b49\u5f85\u7b97\u6cd5\u6267\u884c\u5b8c\u6210\u540e\u5bf9\u6bd4\u7ed3\u679c"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'@pytest.mark.parametrize("algo", [BFSEMBEDOPT], indirect=True)\ndef test_exec_bfs_embed(self, algo):\n    pass\n'})}),"\n",(0,s.jsx)(n.h4,{id:"2212bash",children:"2.2.12.bash"}),"\n",(0,s.jsx)(n.h5,{id:"22121\u542f\u52a8\u53c2\u6570",children:"2.2.12.1.\u542f\u52a8\u53c2\u6570"}),"\n",(0,s.jsx)(n.p,{children:"\u91c7\u7528python\u5b57\u5178\u4f20\u5165"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"cmd\u662f\u542f\u52a8\u547d\u4ee4"}),"\n"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'BASHOPT = {\n        "cmd" : "sh ./test_rpc_client/cpp/CppClientTest/compile.sh"\n    }\n'})}),"\n",(0,s.jsx)(n.h5,{id:"22122\u542f\u52a8\u547d\u4ee4",children:"2.2.12.2.\u542f\u52a8\u547d\u4ee4"}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7fixtures\u7ec4\u4ef6\u5f15\u5165\u5de5\u5177\uff0c\u5e76\u901a\u8fc7\u542f\u52a8\u53c2\u6570\u6765\u63a7\u5236\u6267\u884c\u4e0d\u540c\u7684bash\u547d\u4ee4\uff0c\u51fd\u6570\u5f00\u59cb\u6267\u884c\u524d\u4f1a\u5f00\u542f\u5b50\u8fdb\u7a0b\u6267\u884c\u901a\u8fc7cmd\u53c2\u6570\u4f20\u5165\u7684bash\u547d\u4ee4\uff0c\u51fd\u6570\u4e3b\u4f53\u7b49\u5f85\u7b97\u6cd5\u6267\u884c\u5b8c\u6210"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'@pytest.mark.parametrize("bash", [BASHOPT], indirect=True)\ndef test_bash(self, bash):\n    pass\n'})}),"\n",(0,s.jsx)(n.h4,{id:"2213rest",children:"2.2.13.rest"}),"\n",(0,s.jsx)(n.h5,{id:"22131\u542f\u52a8\u53c2\u6570",children:"2.2.13.1.\u542f\u52a8\u53c2\u6570"}),"\n",(0,s.jsx)(n.p,{children:"\u91c7\u7528python\u5b57\u5178\u4f20\u5165"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"port\u662fTuGraph Server\u7684\u7aef\u53e3"}),"\n",(0,s.jsx)(n.li,{children:"user\u662fTuGraph Server\u7684\u7528\u6237\u540d"}),"\n",(0,s.jsx)(n.li,{children:"password\u662fTuGraph Server \u4e2duser\u5bf9\u5e94\u7684\u5bc6\u7801"}),"\n"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'RESTTOPT = {"port":"7073", "user":"admin", "password":"73@TuGraph"}\n'})}),"\n",(0,s.jsx)(n.h5,{id:"22132\u542f\u52a8\u547d\u4ee4",children:"2.2.13.2.\u542f\u52a8\u547d\u4ee4"}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7fixtures\u7ec4\u4ef6\u5f15\u5165\u5de5\u5177\uff0c\u5e76\u901a\u8fc7\u542f\u52a8\u53c2\u6570\u6765\u94fe\u63a5\u4e0d\u540c\u7684TuGraph Rest Server\uff0c\u51fd\u6570\u5f00\u59cb\u6267\u884c\u524d\u4f1a\u542f\u52a8\u5ba2\u6237\u7aef\uff0c\u51fd\u6570\u6267\u884c\u7ed3\u675f\u540e\u4f1a\u7ed3\u675f\u5ba2\u6237\u7aef"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'@pytest.mark.parametrize("rest", [RESTTOPT], indirect=True)\ndef test_get_info(self, server, rest):\n\tpass\n'})}),"\n",(0,s.jsx)(n.h3,{id:"23\u6d4b\u8bd5\u6837\u4f8b",children:"2.3.\u6d4b\u8bd5\u6837\u4f8b"}),"\n",(0,s.jsx)(n.h4,{id:"231rest",children:"2.3.1.rest"}),"\n",(0,s.jsx)(n.p,{children:"\u6837\u4f8b\u4ee3\u7801\u4e2d\u5728test_get_info\u51fd\u6570\u6267\u884c\u4e4b\u524d\u5148\u542f\u52a8server\uff0cserver\u542f\u52a8\u540e\u542f\u52a8\u4e86rest client\uff0c\u8fdb\u5165test_get_info\u51fd\u6570\u540e\u83b7\u53d6server\u7684\u4e00\u4e9b\u4fe1\u606f\uff0c\u5e76\u901a\u8fc7assert\u5224\u65ad\u662f\u5426\u6709\u83b7\u53d6\u5230cpu\u7684\u4fe1\u606f\u3002"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --license _FMA_IGNORE_LICENSE_CHECK_SALTED_ --port 7073 --rpc_port 9093",\n               "cleanup_dir":["./testdb"]}\nRESTTOPT = {"port":"7073", "user":"admin", "password":"73@TuGraph"}\n@pytest.mark.parametrize("server", [SERVEROPT], indirect=True)\n@pytest.mark.parametrize("rest", [RESTTOPT], indirect=True)\ndef test_get_info(self, server, rest):\n    res = rest.get_server_info()\n    log.info("res : %s", res)\n    assert(\'cpu\' in res)\n'})}),"\n",(0,s.jsx)(n.h4,{id:"232client",children:"2.3.2.client"}),"\n",(0,s.jsx)(n.p,{children:"\u6837\u4f8b\u4ee3\u7801\u4e2d\u5728test_flushdb\u51fd\u6570\u6267\u884c\u4e4b\u524d\u5148\u6267\u884c\u4e86\u6570\u636e\u79bb\u7ebf\u5bfc\u5165\u903b\u8f91\uff0c\u5e76\u542f\u52a8server\u540e\uff0c\u901a\u8fc7client\u521b\u5efa\u94fe\u63a5\uff0c\u8fdb\u5165test_flushdb\u51fd\u6570\u540e\uff0c\u901a\u8fc7\u67e5\u8be2\u70b9\u7684\u4e2a\u6570\u5224\u65ad\u5bfc\u5165\u662f\u5426\u6210\u529f\uff0c\u5bfc\u5165\u6210\u529f\u540e\u6267\u884cflushDB\u64cd\u4f5c\uff0c\u518d\u6b21\u901a\u8fc7assert\u5224\u65ad\u662f\u5426\u80fd\u6b63\u5e38\u6e05\u7a7adb"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --license _FMA_IGNORE_LICENSE_CHECK_SALTED_ --port 7072 --rpc_port 9092",\n             "cleanup_dir":["./testdb"]}\n\nCLIENTOPT = {"host":"127.0.0.1:9092", "user":"admin", "password":"73@TuGraph"}\n\nIMPORTOPT = {"cmd":"./lgraph_import --config_file ./data/yago/yago.conf --dir ./testdb --user admin --password 73@TuGraph --graph default --overwrite 1",\n             "cleanup_dir":["./testdb", "./.import_tmp"]}\n\n@pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)\n@pytest.mark.parametrize("server", [SERVEROPT], indirect=True)\n@pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)\ndef test_flushdb(self, importor, server, client):\n    ret = client.callCypher("MATCH (n) RETURN n LIMIT 100", "default")\n    assert ret[0]\n    res = json.loads(ret[1])\n    assert len(res) == 21\n    ret = client.callCypher("CALL db.flushDB()", "default")\n    assert ret[0]\n    res = json.loads(ret[1])\n     assert res == None\n'})}),"\n",(0,s.jsx)(n.h4,{id:"233exportorimportor",children:"2.3.3.exportor/importor"}),"\n",(0,s.jsx)(n.p,{children:"\u6837\u4f8b\u4ee3\u7801\u4e2d\u5728test_export_default\u51fd\u6570\u6267\u884c\u4e4b\u524d\u5148\u6267\u884c\u4e86\u6570\u636e\u79bb\u7ebf\u5bfc\u5165\u903b\u8f91\uff0c\u5bfc\u5165\u6210\u529f\u540e\u5c06\u5f53\u524ddb\u7684\u6570\u636e\u5bfc\u51fa\uff0c\u7136\u540e\u518d\u6b21\u901a\u8fc7\u79bb\u7ebf\u5bfc\u5165\u903b\u8f91\u5c06exportor\u5bfc\u51fa\u7684\u6570\u636e\u5bfc\u5165\u5230\u65b0\u7684\u76ee\u5f55\u4e2d\uff0c\u4ee5\u65b0\u5bfc\u5165\u7684\u6570\u636e\u542f\u52a8db\uff0c\u5e76\u4e14\u521b\u5efa\u94fe\u63a5\u3002\u5728test_export_default\u51fd\u6570\u4e3b\u4f53\u4e2d\u5224\u65ad\u5bfc\u51fa\u540e\u518d\u6b21\u5bfc\u5165\u7684\u6570\u636e\u662f\u5426\u4e0e\u539f\u59cb\u6570\u636e\u4e00\u81f4"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb1 --license _FMA_IGNORE_LICENSE_CHECK_SALTED_ --port 7073 --rpc_port 9093",\n             "cleanup_dir":["./testdb1"]}\n\nCLIENTOPT = {"host":"127.0.0.1:9093", "user":"admin", "password":"73@TuGraph"}\n\nIMPORT_YAGO_OPT = {"cmd":"./lgraph_import --config_file ./data/yago/yago.conf --dir ./testdb --user admin --password 73@TuGraph --graph default --overwrite 1",\n             "cleanup_dir":["./.import_tmp", "./testdb"]}\n\nIMPORT_DEF_OPT = {"cmd":"./lgraph_import -c ./export/default/import.config -d ./testdb1",\n             "cleanup_dir":["./.import_tmp", "./testdb1"]}\n\nEXPORT_DEF_OPT = {"cmd":"./lgraph_export -d ./testdb -e ./export/default -g default -u admin -p 73@TuGraph",\n                  "cleanup_dir":["./export"]}\n\n@pytest.mark.parametrize("importor", [IMPORT_YAGO_OPT], indirect=True)\n@pytest.mark.parametrize("exportor", [EXPORT_DEF_OPT], indirect=True)\n@pytest.mark.parametrize("importor_1", [IMPORT_DEF_OPT], indirect=True)\n@pytest.mark.parametrize("server", [SERVEROPT], indirect=True)\n@pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)\ndef test_export_default(self, importor, exportor, importor_1, server, client):\n    ret = client.callCypher("MATCH (n) RETURN n LIMIT 100", "default")\n    assert ret[0]\n    res = json.loads(ret[1])\n    log.info("res : %s", res)\n    assert len(res) == 21\n'})}),"\n",(0,s.jsx)(n.h4,{id:"234\u5176\u4ed6\u6d4b\u8bd5",children:"2.3.4.\u5176\u4ed6\u6d4b\u8bd5"}),"\n",(0,s.jsxs)(n.p,{children:["\u66f4\u591a\u7528\u4f8b\u8bf7\u53c2\u8003\u96c6\u6210\u6d4b\u8bd5\u4ee3\u7801 ",(0,s.jsx)(n.a,{href:"https://github.com/TuGraph-family/tugraph-db/tree/master/test/integration",children:"https://github.com/TuGraph-family/tugraph-db/tree/master/test/integration"})]})]})}function p(e={}){const{wrapper:n}={...(0,t.R)(),...e.components};return n?(0,s.jsx)(n,{...e,children:(0,s.jsx)(h,{...e})}):h(e)}},28453:(e,n,r)=>{r.d(n,{R:()=>i,x:()=>d});var s=r(96540);const t={},l=s.createContext(t);function i(e){const n=s.useContext(l);return s.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function d(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(t):e.components||t:i(e.components),s.createElement(l.Provider,{value:n},e.children)}}}]);