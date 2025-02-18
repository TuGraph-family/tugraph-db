"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[43137],{10502:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>t,contentTitle:()=>d,default:()=>o,frontMatter:()=>c,metadata:()=>l,toc:()=>h});var s=r(74848),i=r(28453);const c={},d="\u573a\u666f\uff1a\u4e09\u56fd",l={id:"quick-start/demo/three-kingdoms",title:"\u573a\u666f\uff1a\u4e09\u56fd",description:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd \u4e09\u56fd demo\u7684\u4f7f\u7528\u65b9\u6cd5\u3002",source:"@site/versions/version-4.3.2/zh-CN/source/3.quick-start/2.demo/4.three-kingdoms.md",sourceDirName:"3.quick-start/2.demo",slug:"/quick-start/demo/three-kingdoms",permalink:"/tugraph-db/zh/4.3.2/quick-start/demo/three-kingdoms",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:4,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"\u573a\u666f\uff1a\u4e09\u4f53",permalink:"/tugraph-db/zh/4.3.2/quick-start/demo/the-three-body"},next:{title:"Round The World Demo",permalink:"/tugraph-db/zh/4.3.2/quick-start/demo/round-the-world"}},t={},h=[{value:"1. \u7b80\u4ecb",id:"1-\u7b80\u4ecb",level:2},{value:"2. \u6570\u636e\u5efa\u6a21",id:"2-\u6570\u636e\u5efa\u6a21",level:2},{value:"3. \u6570\u636e\u5bfc\u5165",id:"3-\u6570\u636e\u5bfc\u5165",level:2},{value:"4. Cypher\u5206\u6790",id:"4-cypher\u5206\u6790",level:2},{value:"4.1. \u8bf8\u845b\u4eae\u4e3a\u4ec0\u4e48\u9009\u62e9\u5218\u5907",id:"41-\u8bf8\u845b\u4eae\u4e3a\u4ec0\u4e48\u9009\u62e9\u5218\u5907",level:3},{value:"4.2. \u66f9\u64cd\u4e3a\u4ec0\u4e48\u6210\u5c31\u6bd4\u5218\u5907\u9ad8",id:"42-\u66f9\u64cd\u4e3a\u4ec0\u4e48\u6210\u5c31\u6bd4\u5218\u5907\u9ad8",level:3},{value:"4.3. \u4e09\u56fd\u4e2d\u6700\u5f3a\u5927\u7684\u9b4f\u56fd\u4e3a\u4f55\u6700\u5148\u706d\u4ea1",id:"43-\u4e09\u56fd\u4e2d\u6700\u5f3a\u5927\u7684\u9b4f\u56fd\u4e3a\u4f55\u6700\u5148\u706d\u4ea1",level:3},{value:"4.4. \u4e09\u56fd\u5404\u81ea\u7684\u5b9e\u529b\u7a76\u7adf\u5982\u4f55",id:"44-\u4e09\u56fd\u5404\u81ea\u7684\u5b9e\u529b\u7a76\u7adf\u5982\u4f55",level:3},{value:"4.5. \u66f9\u64cd\u7684\u519b\u4e8b\u80fd\u529b\u5982\u4f55\u8bc4\u4ef7",id:"45-\u66f9\u64cd\u7684\u519b\u4e8b\u80fd\u529b\u5982\u4f55\u8bc4\u4ef7",level:3},{value:"5. \u5907\u6ce8",id:"5-\u5907\u6ce8",level:2}];function a(e){const n={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",img:"img",li:"li",p:"p",pre:"pre",ul:"ul",...(0,i.R)(),...e.components};return(0,s.jsxs)(s.Fragment,{children:[(0,s.jsx)(n.header,{children:(0,s.jsx)(n.h1,{id:"\u573a\u666f\u4e09\u56fd",children:"\u573a\u666f\uff1a\u4e09\u56fd"})}),"\n",(0,s.jsxs)(n.blockquote,{children:["\n",(0,s.jsx)(n.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd \u4e09\u56fd demo\u7684\u4f7f\u7528\u65b9\u6cd5\u3002"}),"\n"]}),"\n",(0,s.jsx)(n.h2,{id:"1-\u7b80\u4ecb",children:"1. \u7b80\u4ecb"}),"\n",(0,s.jsx)(n.p,{children:"\u7531\u4e8e\u53f2\u6599\u7684\u7f3a\u5931\uff0c\u4e00\u4e9b\u5373\u4f7f\u4e3a\u4eba\u4eec\u719f\u77e5\u7684\u5386\u53f2\u4e8b\u4ef6\u4e5f\u5f80\u5f80\u5b58\u5728\u7740\u5f88\u591a\u672a\u89e3\u4e4b\u8c1c\u3002\u4ee5\u4e09\u56fd\u5386\u53f2\u4e3a\u4f8b\uff0c\u8bf8\u845b\u4eae\u4e3a\u4ec0\u4e48\u51fa\u5c71\u5e2e\u52a9\u5f53\u65f6\u52bf\u529b\u5f31\u5c0f\u7684\u5218\u5907\uff0c\u540c\u4e3a\u5929\u4e0b\u82f1\u96c4\u7684\u66f9\u64cd\u548c\u5218\u5907\u4e3a\u4ec0\u4e48\u6210\u5c31\u5dee\u8ddd\u5de8\u5927\u7b49\u7b49\u3002\u4ee5\u5f80\u5b66\u8005\u5f80\u5f80\u91c7\u7528\u4e8c\u7ef4\u5173\u7cfb\u5206\u6790\u5386\u53f2\uff0c\u8fd9\u6837\u5f97\u51fa\u7684\u7ed3\u8bba\u5f80\u5f80\u6bd4\u8f83\u7247\u9762\u3002\u4f7f\u7528TuGraph\u5c06\u4e09\u56fd\u7684\u5386\u53f2\u4eba\u7269\u548c\u4e8b\u4ef6\u5bfc\u5165\u56fe\u6a21\u578b\u4e2d\uff0c\u4f7f\u7528\u56fe\u8ba1\u7b97\u65b9\u5f0f\u8fdb\u884c\u5206\u6790\uff0c\u80fd\u591f\u5e2e\u52a9\u6211\u4eec\u4ece\u6709\u9650\u7684\u4fe1\u606f\u4e2d\u83b7\u5f97\u66f4\u6709\u4ef7\u503c\u7684\u77e5\u8bc6\uff0c\u662f\u4e00\u79cd\u975e\u5e38\u6709\u610f\u4e49\u7684\u8de8\u5b66\u79d1\u5c1d\u8bd5\u3002"}),"\n",(0,s.jsx)(n.h2,{id:"2-\u6570\u636e\u5efa\u6a21",children:"2. \u6570\u636e\u5efa\u6a21"}),"\n",(0,s.jsxs)(n.p,{children:["\u6211\u4eec\u8bbe\u8ba1\u4e865\u7c7b\u9876\u70b9\u548c5\u7c7b\u8fb9\uff0c\u70b9\u5305\u62ec\u201c\u4e3b\u516c\u201d\uff0c\u201c\u5dde\u201d\uff0c\u201c\u6587\u81e3\u201d\uff0c\u201c\u6b66\u5c06\u201d\uff0c\u201c\u6218\u5f79\u201d\uff0c\u8fb9\u5305\u62ec\u201c\u7236\u4eb2\u201d\uff0c\u201c\u5144\u957f\u201d\uff0c\u201c\u96b6\u5c5e\u201d\uff0c\u201c\u7c4d\u8d2f\u201d\uff0c\u201c\u53c2\u6218\u201d\u3002\u5176\u5177\u4f53\u5efa\u6a21\u4fe1\u606f\u5982\u4e0b\u6240\u793a\uff1a\n",(0,s.jsx)(n.img,{alt:"image.png",src:r(32450).A+"",width:"1124",height:"1160"})]}),"\n",(0,s.jsx)(n.h2,{id:"3-\u6570\u636e\u5bfc\u5165",children:"3. \u6570\u636e\u5bfc\u5165"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:["\u624b\u52a8\u5bfc\u5165\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:["\u5411TuGraph\u5bfc\u5165\u6570\u636e\uff0c\u65e2\u53ef\u4ee5\u4f7f\u7528TuGraph\u7684",(0,s.jsx)(n.code,{children:"lgraph_import"}),"\u5de5\u5177\u79bb\u7ebf\u5bfc\u5165\u3002lgraph_import\u5bfc\u5165\u547d\u4ee4\u5982\u4e0b\u6240\u793a"]}),"\n"]}),"\n"]}),"\n"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-shell",children:"lgraph_import -c import.json --overwrite true --continue_on_error true\n"})}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:["\u81ea\u52a8\u521b\u5efa\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:["\u70b9\u51fb",(0,s.jsx)(n.code,{children:"\u65b0\u5efa\u56fe\u9879\u76ee"}),"\uff0c\u9009\u62e9\u4e09\u56fd\u6570\u636e\uff0c\u586b\u5199\u56fe\u9879\u76ee\u914d\u7f6e\uff0c\u7cfb\u7edf\u4f1a\u81ea\u52a8\u5b8c\u6210\u4e09\u56fd\u573a\u666f\u56fe\u9879\u76ee\u521b\u5efa\u3002"]}),"\n",(0,s.jsx)(n.li,{children:"\u6216\u8005\u76f4\u63a5\u4f7f\u7528\u4ea7\u54c1\u81ea\u5e26\u7684\u4e09\u56fd\u56fe\u9879\u76ee\uff08\u5e26\u6709\u5b98\u65b9\u56fe\u6807\uff09\u3002"}),"\n"]}),"\n"]}),"\n"]}),"\n",(0,s.jsx)(n.h2,{id:"4-cypher\u5206\u6790",children:"4. Cypher\u5206\u6790"}),"\n",(0,s.jsx)(n.h3,{id:"41-\u8bf8\u845b\u4eae\u4e3a\u4ec0\u4e48\u9009\u62e9\u5218\u5907",children:"4.1. \u8bf8\u845b\u4eae\u4e3a\u4ec0\u4e48\u9009\u62e9\u5218\u5907"}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7\u5982\u4e0bcypher\u547d\u4ee4\u53ef\u4ee5\u67e5\u770b\u8bf8\u845b\u4eae\u548c\u66f9\u64cd\u3001\u5218\u5907\u4e4b\u95f4\u7684\u5173\u7cfb"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:"MATCH p = (cc:\u4e3b\u516c {name: '\u66f9\u64cd'})-[*1..3]-(zgl:\u6587\u81e3 {name: '\u8bf8\u845b\u4eae'}) RETURN p\n"})}),"\n",(0,s.jsxs)(n.p,{children:["\u5f97\u5230\u7684\u67e5\u8be2\u7ed3\u679c\u5982\u4e0b\u56fe\u6240\u793a\n",(0,s.jsx)(n.img,{alt:"image.png",src:r(52357).A+"",width:"1527",height:"999"}),"\n\u4ece\u56fe\u4e2d\u53ef\u4ee5\u5f88\u76f4\u89c2\u7684\u770b\u51fa\uff0c\u66f9\u64cd\u548c\u8bf8\u845b\u4eae\u4e4b\u95f4\u7684\u6700\u77ed\u8def\u5f84\u4e4b\u4e00\u5305\u542b\u5f90\u5dde\u4e4b\u6218\uff0c\u66f9\u64cd\u66fe\u7ecf\u56e0\u7236\u4eb2\u88ab\u6740\u5bf9\u5f90\u5dde\u8fdb\u884c\u8fc7\u5c60\u57ce\uff0c\u800c\u8bf8\u845b\u4eae\u662f\u5f90\u5dde\u7405\u740a\u90e1\u4eba\uff0c\u4efb\u4f55\u4eba\u90fd\u65ad\u7136\u4e0d\u4f1a\u9009\u62e9\u4e00\u4e2a\u5c60\u6740\u8fc7\u81ea\u5df1\u5bb6\u4e61\u7684\u519b\u9600\u4f5c\u4e3a\u4e3b\u516c\u3002\u800c\u76f8\u53cd\uff0c\u5218\u5907\u66fe\u7ecf\u5728\u5f90\u5dde\u4e4b\u6218\u4e2d\u963b\u6b62\u8fc7\u66f9\u64cd\u7684\u66b4\u884c\uff0c\u8fd9\u5e94\u5f53\u662f\u8bf8\u845b\u4eae\u5bf9\u5218\u5907\u597d\u611f\u7684\u539f\u56e0\u4e4b\u4e00\u3002"]}),"\n",(0,s.jsx)(n.h3,{id:"42-\u66f9\u64cd\u4e3a\u4ec0\u4e48\u6210\u5c31\u6bd4\u5218\u5907\u9ad8",children:"4.2. \u66f9\u64cd\u4e3a\u4ec0\u4e48\u6210\u5c31\u6bd4\u5218\u5907\u9ad8"}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7\u5982\u4e0bcypher\u547d\u4ee4\u53ef\u4ee5\u67e5\u770b\u5bb6\u65cf\u5bf9\u66f9\u64cd\u521b\u4e1a\u7684\u52a9\u529b"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:'MATCH (cc:\u4e3b\u516c{name:"\u66f9\u64cd"})<-[r:\u96b6\u5c5e]-(wj:\u6b66\u5c06) WHERE wj.name REGEXP "\u66f9.*" OR wj.name REGEXP "\u590f\u4faf.*" return cc,wj,r\n'})}),"\n",(0,s.jsxs)(n.p,{children:["\u5f97\u5230\u7684\u67e5\u8be2\u7ed3\u679c\u5982\u4e0b\u56fe\u6240\u793a\n",(0,s.jsx)(n.img,{alt:"image.png",src:r(81998).A+"",width:"1527",height:"999"}),"\n\u5e73\u5b9a\u5929\u4e0b\u6700\u91cd\u8981\u7684\u5c31\u662f\u519b\u4e8b\u4eba\u624d\uff0c\u66f9\u64cd\u5176\u7236\u672c\u59d3\u590f\u4faf\uff0c\u8fc7\u7ee7\u4e8e\u66f9\u6c0f\uff0c\u66f9\u6c0f\u548c\u590f\u4faf\u6c0f\u5728\u8c2f\u53bf\u90fd\u5c5e\u4e8e\u5730\u65b9\u5927\u65cf\uff0c\u5728\u66f9\u64cd\u521b\u4e1a\u521d\u671f\u63d0\u4f9b\u4e86\u590f\u4faf\u60c7\uff0c\u590f\u4faf\u6e0a\uff0c\u66f9\u4ec1\uff0c\u66f9\u6d2a\u5728\u5185\u5927\u91cf\u7684\u519b\u4e8b\u4eba\u624d\u3002\u800c\u5218\u5907\u5176\u7236\u65e9\u4e27\uff0c\u6ca1\u6709\u5bb6\u65cf\u52a9\u529b\uff0c\u5e74\u8fc750\u624d\u51d1\u9f50\u4e86\u81ea\u5df1\u7684\u4e94\u864e\u4e0a\u5c06\uff0c\u800c\u8fd9\u65f6\u5df2\u7ecf\u8fc7\u4e86\u5929\u4e0b\u5927\u4e71\u4e89\u593a\u5730\u76d8\u7684\u6700\u4f73\u65f6\u673a\uff0c\u66f9\u64cd\u5df2\u7ecf\u5929\u4e0b\u4e5d\u5dde\u5c45\u5176\u516d\u4e86\u3002"]}),"\n",(0,s.jsx)(n.h3,{id:"43-\u4e09\u56fd\u4e2d\u6700\u5f3a\u5927\u7684\u9b4f\u56fd\u4e3a\u4f55\u6700\u5148\u706d\u4ea1",children:"4.3. \u4e09\u56fd\u4e2d\u6700\u5f3a\u5927\u7684\u9b4f\u56fd\u4e3a\u4f55\u6700\u5148\u706d\u4ea1"}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7\u5982\u4e0bcypher\u547d\u4ee4\u53ef\u4ee5\u67e5\u770b\u66f9\u64cd\u96c6\u56e2\u7684\u91cd\u8981\u6587\u5b98\u7ec4\u6210"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:'MATCH (cc:\u4e3b\u516c)<-[r:\u96b6\u5c5e]-(wc) WHERE cc.name REGEXP "\u66f9.*" AND (label(wc) = "\u6587\u81e3" OR label(wc) = "\u4e3b\u516c") return cc,wc,r\n'})}),"\n",(0,s.jsxs)(n.p,{children:["\u5f97\u5230\u7684\u67e5\u8be2\u7ed3\u679c\u5982\u4e0b\u56fe\u6240\u793a\n",(0,s.jsx)(n.img,{alt:"image.png",src:r(27575).A+"",width:"1527",height:"999"}),"\n\u66f9\u9b4f\u4e8b\u5b9e\u4e0a\u4e8e249\u5e74\u706d\u4ea1\u4e8e\u9ad8\u5e73\u9675\u4e4b\u53d8\uff0c\u7acb\u56fd29\u5e74\uff0c\u5c11\u4e8e\u8700\u6c49\uff0843\u5e74\uff09\u548c\u4e1c\u5434\uff0851\u5e74\uff09\u3002\u4e09\u56fd\u4e2d\u5b9e\u529b\u6700\u5f3a\u5927\u7684\u9b4f\u56fd\u6700\u5148\u706d\u4ea1\u7684\u539f\u56e0\u5c31\u5728\u4e8e\u66f9\u9b4f\u7684\u6587\u5b98\u5236\u5ea6\uff08\u4e5d\u54c1\u4e2d\u6b63\u5236\uff09\u4f7f\u5f97\u6743\u529b\u5f88\u5bb9\u6613\u96c6\u4e2d\u5728\u4e16\u5bb6\u5927\u65cf\u624b\u4e2d\u3002\u4ece\u56fe\u4e2d\u53ef\u4ee5\u770b\u51fa\uff0c\u66f9\u64cd\u66f9\u4e15\u7236\u5b50\u4e24\u4ee3\u7684\u91cd\u8981\u6587\u81e3\u51e0\u4e4e\u90fd\u662f\u4e16\u5bb6\u5927\u65cf\uff0c\u988d\u5ddd\u8340\u6c0f\uff0c\u988d\u5ddd\u949f\u6c0f\uff0c\u988d\u5ddd\u9648\u6c0f\uff0c\u6b66\u5a01\u8d3e\u6c0f\u7b49\uff0c\u751a\u81f3\u8fd8\u51fa\u73b0\u4e86\u5730\u533a\u5316\u8d8b\u52bf\uff0c\u96c6\u4e2d\u4e8e\u988d\u5ddd\uff0c\u6700\u7ec8\u653f\u6743\u4e5f\u4e3a\u548c\u988d\u5ddd\u8340\u6c0f\u5173\u7cfb\u5bc6\u5207\u7684\u6cb3\u5185\u53f8\u9a6c\u6c0f\u6240\u7be1\u593a\u3002"]}),"\n",(0,s.jsx)(n.h3,{id:"44-\u4e09\u56fd\u5404\u81ea\u7684\u5b9e\u529b\u7a76\u7adf\u5982\u4f55",children:"4.4. \u4e09\u56fd\u5404\u81ea\u7684\u5b9e\u529b\u7a76\u7adf\u5982\u4f55"}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7\u5982\u4e0bcypher\u547d\u4ee4\u53ef\u4ee5\u67e5\u770b\u4e09\u56fd\u5404\u96c6\u56e2\u7684\u4eba\u53e3\u5b9e\u529b"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:'MATCH (p) WHERE (label(p)="\u4e3b\u516c" OR label(p)="\u6587\u81e3" OR label(p)="\u6b66\u5c06") AND p.hometown IN ["\u5e7d\u5dde","\u5180\u5dde","\u9752\u5dde","\u5e76\u5dde","\u51c9\u5dde","\u53f8\u5dde","\u8c6b\u5dde","\u5156\u5dde","\u5f90\u5dde"] WITH COUNT(p) AS w\nMATCH (p) WHERE (label(p)="\u4e3b\u516c" OR label(p)="\u6587\u81e3" OR label(p)="\u6b66\u5c06") AND p.hometown IN ["\u76ca\u5dde"] WITH COUNT(p) AS s,w\nMATCH (p) WHERE (label(p)="\u4e3b\u516c" OR label(p)="\u6587\u81e3" OR label(p)="\u6b66\u5c06") AND p.hometown IN ["\u626c\u5dde","\u8346\u5dde","\u4ea4\u5dde"]\nRETURN w as \u9b4f\u4eba\u53e3,s as \u8700\u4eba\u53e3,count(p) as \u5434\u4eba\u53e3\n'})}),"\n",(0,s.jsxs)(n.p,{children:["\u5f97\u5230\u7684\u67e5\u8be2\u7ed3\u679c\u5982\u4e0b\u8868\u6240\u793a\n",(0,s.jsx)(n.img,{alt:"image.png",src:r(78584).A+"",width:"1527",height:"999"}),"\n\u53e4\u4ee3\u793e\u4f1a\u8861\u91cf\u4e00\u4e2a\u56fd\u5bb6\u5b9e\u529b\u7684\u91cd\u8981\u6307\u6807\u662f\u4eba\u53e3\u6570\u91cf\uff0c\u7531\u4e8e\u4eba\u53e3\u6570\u636e\u7f3a\u5931\uff0c\u6211\u4eec\u4f7f\u7528\u4e09\u56fd\u6240\u6709\u4e3b\u516c\u548c\u6587\u81e3\u6b66\u5c06\u7684\u7c4d\u8d2f\u6570\u636e\u4f30\u8ba1\u6bcf\u4e2a\u5dde\u7684\u4eba\u53e3\u6570\u91cf\u3002\u53d1\u73b0\u4e09\u56fd\u4e3b\u8981\u4eba\u7269\u4e2d\uff0c\u6309\u7c4d\u8d2f\u670960\u4e2a\u5c5e\u4e8e\u9b4f\u56fd\uff0c\u670923\u4e2a\u5c5e\u4e8e\u5434\u56fd\uff0c\u4ec5\u67092\u4e2a\u5c5e\u4e8e\u8700\u56fd\uff0c\u8bc1\u660e\u9b4f\u56fd\u786e\u5b9e\u662f\u4e09\u56fd\u4e2d\u6700\u5f3a\u5927\u7684\u56fd\u5bb6\u3002"]}),"\n",(0,s.jsx)(n.h3,{id:"45-\u66f9\u64cd\u7684\u519b\u4e8b\u80fd\u529b\u5982\u4f55\u8bc4\u4ef7",children:"4.5. \u66f9\u64cd\u7684\u519b\u4e8b\u80fd\u529b\u5982\u4f55\u8bc4\u4ef7"}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7\u5982\u4e0bcypher\u547d\u4ee4\u53ef\u4ee5\u67e5\u770b\u66f9\u64cd\u53c2\u4e0e\u7684\u4e3b\u8981\u6218\u5f79"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:'MATCH (cc:\u4e3b\u516c{name:"\u66f9\u64cd"})-[e]-(zy:\u6218\u5f79) RETURN cc,zy,e\n'})}),"\n",(0,s.jsxs)(n.p,{children:["\u5f97\u5230\u7684\u67e5\u8be2\u7ed3\u679c\u5982\u4e0b\u56fe\u6240\u793a\n",(0,s.jsx)(n.img,{alt:"image.png",src:r(99713).A+"",width:"1527",height:"999"}),"\n\u4ece\u56fe\u4e2d\u53ef\u4ee5\u770b\u51fa\uff0c\u66f9\u64cd\u5728\u4e09\u56fd\u4e3b\u8981\u768415\u573a\u6218\u5f79\u4e2d\u53c2\u52a0\u4e868\u573a\uff0c\u51fa\u573a\u7387\u6bd4\u8f83\u9ad8\u3002\u4f46\u662f\u66f9\u64cd\u53ea\u83b7\u80dc\u4e86\u5f90\u5dde\u4e4b\u6218\u3001\u5156\u5dde\u4e4b\u6218\u3001\u5b98\u6e21\u4e4b\u6218\u548c\u8944\u6a0a\u4e4b\u6218\uff0c\u6c49\u4e2d\u4e4b\u6218\u3001\u5b9b\u57ce\u4e4b\u6218\u3001\u7fa4\u96c4\u8ba8\u8463\u548c\u8d64\u58c1\u4e4b\u6218\u90fd\u5931\u8d25\u4e86\uff0c\u7efc\u5408\u80dc\u738750%\uff0c\u8bc1\u660e\u66f9\u64cd\u5e76\u4e0d\u7b97\u4e00\u4e2a\u975e\u5e38\u4f18\u79c0\u7684\u519b\u4e8b\u5bb6\u3002"]}),"\n",(0,s.jsx)(n.h2,{id:"5-\u5907\u6ce8",children:"5. \u5907\u6ce8"}),"\n",(0,s.jsx)(n.p,{children:"\u66f4\u591a\u7684\u5206\u6790\u6709\u5f85\u5927\u5bb6\u79ef\u6781\u8865\u5145\u548c\u5c1d\u8bd5\uff01"})]})}function o(e={}){const{wrapper:n}={...(0,i.R)(),...e.components};return n?(0,s.jsx)(n,{...e,children:(0,s.jsx)(a,{...e})}):a(e)}},27575:(e,n,r)=>{r.d(n,{A:()=>s});const s=r.p+"assets/images/three-kingdoms-cypher3-5d7837e0c354f6d6f0b7a7343a05b12d.png"},28453:(e,n,r)=>{r.d(n,{R:()=>d,x:()=>l});var s=r(96540);const i={},c=s.createContext(i);function d(e){const n=s.useContext(c);return s.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function l(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(i):e.components||i:d(e.components),s.createElement(c.Provider,{value:n},e.children)}},32450:(e,n,r)=>{r.d(n,{A:()=>s});const s=r.p+"assets/images/three-kingdoms-schema-86cedb325b48b2d0b9d4ccbd4ffd4ffd.png"},52357:(e,n,r)=>{r.d(n,{A:()=>s});const s=r.p+"assets/images/three-kingdoms-cypher1-959ef771556fbe22ad88314b24b7e6e7.png"},78584:(e,n,r)=>{r.d(n,{A:()=>s});const s=r.p+"assets/images/three-kingdoms-cypher4-b4ee597e34a8b026d14f09b83862f8bb.png"},81998:(e,n,r)=>{r.d(n,{A:()=>s});const s=r.p+"assets/images/three-kingdoms-cypher2-d345fe10f1310a6c86fdf81afaf7a860.png"},99713:(e,n,r)=>{r.d(n,{A:()=>s});const s=r.p+"assets/images/three-kingdoms-cypher5-531165798a411a41937056e72136cfc9.png"}}]);