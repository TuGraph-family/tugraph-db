"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[1842],{28453:(e,n,t)=>{t.d(n,{R:()=>l,x:()=>o});var r=t(96540);const s={},i=r.createContext(s);function l(e){const n=r.useContext(i);return r.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function o(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(s):e.components||s:l(e.components),r.createElement(i.Provider,{value:n},e.children)}},43325:(e,n,t)=>{t.r(n),t.d(n,{assets:()=>a,contentTitle:()=>l,default:()=>u,frontMatter:()=>i,metadata:()=>o,toc:()=>c});var r=t(74848),s=t(28453);const i={},l="\u6570\u636e\u9884\u70ed",o={id:"utility-tools/data-warmup",title:"\u6570\u636e\u9884\u70ed",description:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph \u7684\u6570\u636e\u9884\u70ed\u529f\u80fd\u3002",source:"@site/versions/version-4.5.1/zh-CN/source/6.utility-tools/4.data-warmup.md",sourceDirName:"6.utility-tools",slug:"/utility-tools/data-warmup",permalink:"/tugraph-db/en-US/zh/4.5.1/utility-tools/data-warmup",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:4,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"\u5907\u4efd\u6062\u590d",permalink:"/tugraph-db/en-US/zh/4.5.1/utility-tools/backup-and-restore"},next:{title:"\u96c6\u7fa4\u7ba1\u7406",permalink:"/tugraph-db/en-US/zh/4.5.1/utility-tools/ha-cluster-management"}},a={},c=[{value:"1.\u7b80\u4ecb",id:"1\u7b80\u4ecb",level:2},{value:"1.\u6570\u636e\u9884\u70ed\u547d\u4ee4",id:"1\u6570\u636e\u9884\u70ed\u547d\u4ee4",level:2}];function d(e){const n={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",header:"header",li:"li",p:"p",pre:"pre",ul:"ul",...(0,s.R)(),...e.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(n.header,{children:(0,r.jsx)(n.h1,{id:"\u6570\u636e\u9884\u70ed",children:"\u6570\u636e\u9884\u70ed"})}),"\n",(0,r.jsxs)(n.blockquote,{children:["\n",(0,r.jsx)(n.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph \u7684\u6570\u636e\u9884\u70ed\u529f\u80fd\u3002"}),"\n"]}),"\n",(0,r.jsx)(n.h2,{id:"1\u7b80\u4ecb",children:"1.\u7b80\u4ecb"}),"\n",(0,r.jsx)(n.p,{children:"TuGraph \u662f\u57fa\u4e8e\u78c1\u76d8\u7684\u6570\u636e\u5e93\uff0c\u4ec5\u5f53\u8bbf\u95ee\u6570\u636e\u65f6\uff0c\u6570\u636e\u624d\u4f1a\u52a0\u8f7d\u5230\u5185\u5b58\u4e2d\u3002\u56e0\u6b64\u5728\u670d\u52a1\u5668\u521a\u5f00\u542f\u540e\u7684\u4e00\u6bb5\u65f6\u95f4\u5185\uff0c\u7cfb\u7edf\u6027\u80fd\u53ef\u80fd\u4f1a\u7531\u4e8e\u9891\u7e41\u7684 IO \u64cd\u4f5c\u800c\u53d8\u5dee\u3002\u6b64\u65f6\u6211\u4eec\u53ef\u4ee5\u901a\u8fc7\u4e8b\u5148\u8fdb\u884c\u6570\u636e\u9884\u70ed\u6765\u6539\u5584\u8fd9\u4e00\u95ee\u9898\u3002"}),"\n",(0,r.jsx)(n.h2,{id:"1\u6570\u636e\u9884\u70ed\u547d\u4ee4",children:"1.\u6570\u636e\u9884\u70ed\u547d\u4ee4"}),"\n",(0,r.jsxs)(n.p,{children:["\u6570\u636e\u9884\u70ed\u53ef\u4ee5\u901a\u8fc7\u5de5\u5177 ",(0,r.jsx)(n.code,{children:"lgraph_warmup"})," \u6765\u8fdb\u884c\u3002\u5b83\u7684\u4f7f\u7528\u793a\u4f8b\u5982\u4e0b\uff1a"]}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-bash",children:"$ lgraph_warmup -d {directory} -g {graph_list}\n"})}),"\n",(0,r.jsx)(n.p,{children:"\u5176\u4e2d\uff1a"}),"\n",(0,r.jsxs)(n.ul,{children:["\n",(0,r.jsxs)(n.li,{children:["\n",(0,r.jsxs)(n.p,{children:[(0,r.jsx)(n.code,{children:"-d {db_dir}"})," \u9009\u9879\u6307\u5b9a\u4e86 TuGraph \u670d\u52a1\u5668\u7684\u6570\u636e\u76ee\u5f55"]}),"\n"]}),"\n",(0,r.jsxs)(n.li,{children:["\n",(0,r.jsxs)(n.p,{children:[(0,r.jsx)(n.code,{children:"-g {graph_list}"})," \u9009\u9879\u6307\u5b9a\u9700\u8981\u8fdb\u884c\u6570\u636e\u9884\u70ed\u7684\u56fe\u540d\u79f0\uff0c\u7528\u9017\u53f7\u5206\u9694"]}),"\n"]}),"\n"]}),"\n",(0,r.jsx)(n.p,{children:"\u6839\u636e\u6570\u636e\u5927\u5c0f\u548c\u6240\u4f7f\u7528\u7684\u78c1\u76d8\u7c7b\u578b\u4e0d\u540c\uff0c\u9884\u70ed\u8fc7\u7a0b\u8fd0\u884c\u65f6\u95f4\u4e5f\u4e0d\u540c\u3002\u673a\u68b0\u78c1\u76d8\u4e0a\u9884\u70ed\u4e00\u4e2a\u5927\u6570\u636e\u5e93\u53ef\u80fd\u8017\u65f6\u8f83\u957f\uff0c\u8bf7\u8010\u5fc3\u7b49\u5f85\u3002"})]})}function u(e={}){const{wrapper:n}={...(0,s.R)(),...e.components};return n?(0,r.jsx)(n,{...e,children:(0,r.jsx)(d,{...e})}):d(e)}}}]);