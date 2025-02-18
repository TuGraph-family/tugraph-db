"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[28761],{28453:(e,n,r)=>{r.d(n,{R:()=>t,x:()=>d});var i=r(96540);const s={},l=i.createContext(s);function t(e){const n=i.useContext(l);return i.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function d(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(s):e.components||s:t(e.components),i.createElement(l.Provider,{value:n},e.children)}},32023:(e,n,r)=>{r.d(n,{A:()=>i});const i=r.p+"assets/images/wandering-earth-3-79833e91472e6b0c46d7d663b964728a.png"},42030:(e,n,r)=>{r.d(n,{A:()=>i});const i=r.p+"assets/images/wandering-earth-2-d6bbcf44b71e4f7e8927ac46e99f8636.png"},57764:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>c,contentTitle:()=>t,default:()=>o,frontMatter:()=>l,metadata:()=>d,toc:()=>h});var i=r(74848),s=r(28453);const l={},t="\u573a\u666f\uff1a\u6d41\u6d6a\u5730\u7403",d={id:"quick-start/demo/wandering-earth",title:"\u573a\u666f\uff1a\u6d41\u6d6a\u5730\u7403",description:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd \u6d41\u6d6a\u5730\u7403 demo\u7684\u4f7f\u7528\u65b9\u6cd5\u3002",source:"@site/versions/version-3.5.1/zh-CN/source/3.quick-start/2.demo/2.wandering-earth.md",sourceDirName:"3.quick-start/2.demo",slug:"/quick-start/demo/wandering-earth",permalink:"/tugraph-db/en-US/zh/3.5.1/quick-start/demo/wandering-earth",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:2,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"\u5f71\u89c6\u573a\u666fDemo",permalink:"/tugraph-db/en-US/zh/3.5.1/quick-start/demo/movie"},next:{title:"\u573a\u666f\uff1a\u4e09\u4f53",permalink:"/tugraph-db/en-US/zh/3.5.1/quick-start/demo/the-three-body"}},c={},h=[{value:"1.Demo\u573a\u666f\u8bbe\u8ba1",id:"1demo\u573a\u666f\u8bbe\u8ba1",level:2},{value:"2.\u76ee\u5f55\u7ed3\u6784",id:"2\u76ee\u5f55\u7ed3\u6784",level:2},{value:"3.\u4f7f\u7528\u8bf4\u660e",id:"3\u4f7f\u7528\u8bf4\u660e",level:2},{value:"4.\u6570\u636e\u5bfc\u5165",id:"4\u6570\u636e\u5bfc\u5165",level:2},{value:"5.Cypher\u67e5\u8be2",id:"5cypher\u67e5\u8be2",level:2},{value:"6.\u4f7f\u7528\u5c55\u793a",id:"6\u4f7f\u7528\u5c55\u793a",level:2},{value:"6.1.\u6570\u636e\u5bfc\u5165\u7684\u5c55\u793a",id:"61\u6570\u636e\u5bfc\u5165\u7684\u5c55\u793a",level:3},{value:"6.2.\u67e5\u8be2\u5c55\u793a",id:"62\u67e5\u8be2\u5c55\u793a",level:3}];function a(e){const n={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",img:"img",li:"li",p:"p",pre:"pre",ul:"ul",...(0,s.R)(),...e.components};return(0,i.jsxs)(i.Fragment,{children:[(0,i.jsx)(n.header,{children:(0,i.jsx)(n.h1,{id:"\u573a\u666f\u6d41\u6d6a\u5730\u7403",children:"\u573a\u666f\uff1a\u6d41\u6d6a\u5730\u7403"})}),"\n",(0,i.jsxs)(n.blockquote,{children:["\n",(0,i.jsx)(n.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd \u6d41\u6d6a\u5730\u7403 demo\u7684\u4f7f\u7528\u65b9\u6cd5\u3002"}),"\n"]}),"\n",(0,i.jsx)(n.h2,{id:"1demo\u573a\u666f\u8bbe\u8ba1",children:"1.Demo\u573a\u666f\u8bbe\u8ba1"}),"\n",(0,i.jsx)(n.p,{children:"Demo\u80cc\u666f\u57fa\u4e8e\u6d41\u6d6a\u5730\u74031\u3001\u6d41\u6d6a\u5730\u74032\u7684\u6545\u4e8b\u80cc\u666f\u8fdb\u884c\u8bbe\u8ba1"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"\u57fa\u4e8e\u5267\u60c5\uff0c\u8bbe\u8ba1\u4e86\u56fe\u7ed3\u6784\uff0c\u5305\u542b\u7ec4\u7ec7\u3001\u89d2\u8272\u3001\u5929\u4f53\u4e0e\u8bbe\u65bd3\u7c7b\u70b9\uff0c\u4e8b\u4ef6\u3001\u5173\u7cfb\u4e24\u7c7b\u8fb9"}),"\n",(0,i.jsx)(n.li,{children:"\u6839\u636e\u5267\u60c5\u51c6\u5907\u4e86\u5bf9\u5e94Schema\u7684\u6570\u636e"}),"\n",(0,i.jsx)(n.li,{children:"\u51c6\u5907\u4e86\u4e00\u4e9bquery\uff0c\u63d0\u51fa\u4e00\u4e9b\u5173\u4e8e\u5267\u60c5\u7684\u95ee\u9898"}),"\n"]}),"\n",(0,i.jsx)(n.h2,{id:"2\u76ee\u5f55\u7ed3\u6784",children:"2.\u76ee\u5f55\u7ed3\u6784"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"rawdata: \u539f\u59cb\u5267\u60c5\u6570\u636e\u548cschema\u5b9a\u4e49\u6587\u4ef6"}),"\n",(0,i.jsx)(n.li,{children:"case.cypher: \u67e5\u8be2\u76f8\u5173\u7684Cypher\u8bed\u53e5"}),"\n",(0,i.jsx)(n.li,{children:"control.sh: \u63a7\u5236TuGraph Server\u542f\u52a8\u505c\u6b62\u7684\u811a\u672c"}),"\n",(0,i.jsx)(n.li,{children:"lgraph_standalone.json: TuGraph Server\u542f\u52a8\u7684\u914d\u7f6e\u6587\u4ef6"}),"\n"]}),"\n",(0,i.jsx)(n.h2,{id:"3\u4f7f\u7528\u8bf4\u660e",children:"3.\u4f7f\u7528\u8bf4\u660e"}),"\n",(0,i.jsx)(n.p,{children:"\u524d\u7f6e\u6761\u4ef6\uff1aTuGraph\u5df2\u5b89\u88c5"}),"\n",(0,i.jsx)(n.h2,{id:"4\u6570\u636e\u5bfc\u5165",children:"4.\u6570\u636e\u5bfc\u5165"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:["\u624b\u52a8\u5bfc\u5165\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"\u6839\u636e\u6570\u636e\u5b58\u653e\u76ee\u5f55\u5bf9\u5e94\u4fee\u6539import.json\u91cc\u9762\u7684DATA_PATH\u3002"}),"\n",(0,i.jsx)(n.li,{children:"\u53c2\u8003control.sh\u4e2d\u7684load\u51fd\u6570\uff0c\u52a0\u8f7d\u6570\u636e\u3002"}),"\n",(0,i.jsx)(n.li,{children:"\u53c2\u8003control.sh\u4e2d\u7684start\u51fd\u6570\uff0c\u542f\u52a8TuGraph\u670d\u52a1\u3002"}),"\n",(0,i.jsx)(n.li,{children:"\u542f\u52a8TuGraph\u670d\u52a1\u540e\uff0c\u8bbf\u95ee${HOST_IP}:8888\uff0c\u6253\u5f00web\u9875\u9762\uff0c\u786e\u8ba4\u6570\u636e\u662f\u5426\u5bfc\u5165\u6210\u529f\u3002"}),"\n"]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\u81ea\u52a8\u521b\u5efa\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:["\u70b9\u51fb",(0,i.jsx)(n.code,{children:"\u65b0\u5efa\u56fe\u9879\u76ee"}),"\uff0c\u9009\u62e9\u6d41\u6d6a\u5730\u7403\u6570\u636e\uff0c\u586b\u5199\u56fe\u9879\u76ee\u914d\u7f6e\uff0c\u7cfb\u7edf\u4f1a\u81ea\u52a8\u5b8c\u6210\u6d41\u6d6a\u5730\u7403\u573a\u666f\u56fe\u9879\u76ee\u521b\u5efa\u3002"]}),"\n",(0,i.jsx)(n.li,{children:"\u6216\u8005\u76f4\u63a5\u4f7f\u7528\u4ea7\u54c1\u81ea\u5e26\u7684\u6d41\u6d6a\u5730\u7403\u56fe\u9879\u76ee\uff08\u5e26\u6709\u5b98\u65b9\u56fe\u6807\uff09\u3002"}),"\n"]}),"\n"]}),"\n"]}),"\n",(0,i.jsx)(n.h2,{id:"5cypher\u67e5\u8be2",children:"5.Cypher\u67e5\u8be2"}),"\n",(0,i.jsx)(n.p,{children:"\u53c2\u8003cypher\u6587\u6863\uff0c\u5728TuGraph\u7684Web\u9875\u9762\u524d\u7aef\u8f93\u5165Cypher\u8fdb\u884c\u67e5\u8be2"}),"\n",(0,i.jsx)(n.h2,{id:"6\u4f7f\u7528\u5c55\u793a",children:"6.\u4f7f\u7528\u5c55\u793a"}),"\n",(0,i.jsx)(n.h3,{id:"61\u6570\u636e\u5bfc\u5165\u7684\u5c55\u793a",children:"6.1.\u6570\u636e\u5bfc\u5165\u7684\u5c55\u793a"}),"\n",(0,i.jsx)(n.p,{children:(0,i.jsx)(n.img,{alt:"\u6570\u636e\u5bfc\u5165\u5c55\u793a",src:r(60805).A+"",width:"1527",height:"1120"})}),"\n",(0,i.jsx)(n.h3,{id:"62\u67e5\u8be2\u5c55\u793a",children:"6.2.\u67e5\u8be2\u5c55\u793a"}),"\n",(0,i.jsx)(n.p,{children:"\u67e5\u8be2\u6728\u661f\u5371\u673a\u7684\u6240\u6709\u4e8b\u4ef6\u7ecf\u8fc7"}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{children:"MATCH (n)-[e:\u4e8b\u4ef6\u5173\u7cfb]-(m) where e.title='\u6728\u661f\u5371\u673a' RETURN n,e\n"})}),"\n",(0,i.jsx)(n.p,{children:(0,i.jsx)(n.img,{alt:"\u6570\u636e\u5bfc\u5165\u5c55\u793a",src:r(42030).A+"",width:"1527",height:"1120"})}),"\n",(0,i.jsx)(n.p,{children:"\u67e5\u8be2\u6240\u4ee5\u5371\u673a\u7684\u76f8\u5173\u4e8b\u4ef6\u7ecf\u8fc7"}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-cypher",children:'MATCH (n)-[e1]-(m)-[e2]-(p)\nwhere e1.title REGEXP ".*\u5371\u673a" and e2.title REGEXP ".*\u5371\u673a" and e1.title <> e2.title\nRETURN n,e1,e2,p\n'})}),"\n",(0,i.jsx)(n.p,{children:(0,i.jsx)(n.img,{alt:"\u6570\u636e\u5bfc\u5165\u5c55\u793a",src:r(32023).A+"",width:"1527",height:"1120"})})]})}function o(e={}){const{wrapper:n}={...(0,s.R)(),...e.components};return n?(0,i.jsx)(n,{...e,children:(0,i.jsx)(a,{...e})}):a(e)}},60805:(e,n,r)=>{r.d(n,{A:()=>i});const i=r.p+"assets/images/wandering-earth-1-0d55cfd964d4531decde10ca519f0435.png"}}]);