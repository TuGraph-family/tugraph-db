"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[3967],{22992:(e,n,r)=>{r.d(n,{A:()=>l});const l=r.p+"assets/images/learn_flow_chart_zh-2b6033f80e4ac9e5ed7491e345d34f51.png"},28453:(e,n,r)=>{r.d(n,{R:()=>d,x:()=>i});var l=r(96540);const t={},a=l.createContext(t);function d(e){const n=l.useContext(a);return l.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function i(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(t):e.components||t:d(e.components),l.createElement(a.Provider,{value:n},e.children)}},91109:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>h,contentTitle:()=>d,default:()=>o,frontMatter:()=>a,metadata:()=>i,toc:()=>s});var l=r(74848),t=r(28453);const a={},d="Learn Tutorial",i={id:"olap&procedure/learn/tutorial",title:"Learn Tutorial",description:"\u672c\u6587\u6863\u662f\u4e3a TuGraph \u7684\u7528\u6237\u8bbe\u8ba1\u7684\u5f15\u5bfc\u7a0b\u5e8f\uff0c\u7528\u6237\u5728\u9605\u8bfb\u8be6\u7ec6\u7684\u6587\u6863\u4e4b\u524d\uff0c\u5e94\u8be5\u9996\u5148\u9605\u8bfb\u8be5\u6587\u6863\uff0c\u5bf9 TuGraph \u7684\u56fe\u5b66\u4e60\u8fd0\u884c\u6d41\u7a0b\u6709\u4e00\u4e2a\u5927\u81f4\u7684\u4e86\u89e3\uff0c\u4e4b\u540e\u518d\u9605\u8bfb\u8be6\u7ec6\u6587\u6863\u4f1a\u66f4\u52a0\u65b9\u4fbf\u3002\u5f15\u5bfc\u7a0b\u5e8f\u662f\u57fa\u4e8e Tugraph \u7684\u4e00\u4e2a\u7b80\u5355\u7684\u7a0b\u5e8f\u5b9e\u4f8b\uff0c\u6211\u4eec\u5c06\u91cd\u70b9\u4ecb\u7ecd\u5176\u4f7f\u7528\u65b9\u5f0f\u3002",source:"@site/versions/version-4.3.0/zh-CN/source/9.olap&procedure/3.learn/1.tutorial.md",sourceDirName:"9.olap&procedure/3.learn",slug:"/olap&procedure/learn/tutorial",permalink:"/tugraph-db/zh/4.3.0/olap&procedure/learn/tutorial",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:1,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"\u5185\u7f6e\u7b97\u6cd5",permalink:"/tugraph-db/zh/4.3.0/olap&procedure/olap/algorithms"},next:{title:"Sampling API",permalink:"/tugraph-db/zh/4.3.0/olap&procedure/learn/sampling_api"}},h={},s=[{value:"1.TuGraph \u56fe\u5b66\u4e60\u6a21\u5757\u7b80\u4ecb",id:"1tugraph-\u56fe\u5b66\u4e60\u6a21\u5757\u7b80\u4ecb",level:2},{value:"2. \u8fd0\u884c\u6d41\u7a0b",id:"2-\u8fd0\u884c\u6d41\u7a0b",level:2},{value:"3.TuGraph\u7f16\u8bd1\u53ca\u6570\u636e\u51c6\u5907",id:"3tugraph\u7f16\u8bd1\u53ca\u6570\u636e\u51c6\u5907",level:2},{value:"4. \u6570\u636e\u5bfc\u5165",id:"4-\u6570\u636e\u5bfc\u5165",level:2},{value:"5. feature\u7279\u5f81\u8f6c\u6362",id:"5-feature\u7279\u5f81\u8f6c\u6362",level:2},{value:"6. \u91c7\u6837\u7b97\u5b50\u53ca\u7f16\u8bd1",id:"6-\u91c7\u6837\u7b97\u5b50\u53ca\u7f16\u8bd1",level:2},{value:"6.1.\u91c7\u6837\u7b97\u5b50\u4ecb\u7ecd",id:"61\u91c7\u6837\u7b97\u5b50\u4ecb\u7ecd",level:3},{value:"6.2.\u7f16\u8bd1",id:"62\u7f16\u8bd1",level:3},{value:"7. \u6a21\u578b\u8bad\u7ec3\u53ca\u4fdd\u5b58",id:"7-\u6a21\u578b\u8bad\u7ec3\u53ca\u4fdd\u5b58",level:2},{value:"8. \u6a21\u578b\u52a0\u8f7d",id:"8-\u6a21\u578b\u52a0\u8f7d",level:2}];function c(e){const n={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",img:"img",p:"p",pre:"pre",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",...(0,t.R)(),...e.components};return(0,l.jsxs)(l.Fragment,{children:[(0,l.jsx)(n.header,{children:(0,l.jsx)(n.h1,{id:"learn-tutorial",children:"Learn Tutorial"})}),"\n",(0,l.jsxs)(n.blockquote,{children:["\n",(0,l.jsx)(n.p,{children:"\u672c\u6587\u6863\u662f\u4e3a TuGraph \u7684\u7528\u6237\u8bbe\u8ba1\u7684\u5f15\u5bfc\u7a0b\u5e8f\uff0c\u7528\u6237\u5728\u9605\u8bfb\u8be6\u7ec6\u7684\u6587\u6863\u4e4b\u524d\uff0c\u5e94\u8be5\u9996\u5148\u9605\u8bfb\u8be5\u6587\u6863\uff0c\u5bf9 TuGraph \u7684\u56fe\u5b66\u4e60\u8fd0\u884c\u6d41\u7a0b\u6709\u4e00\u4e2a\u5927\u81f4\u7684\u4e86\u89e3\uff0c\u4e4b\u540e\u518d\u9605\u8bfb\u8be6\u7ec6\u6587\u6863\u4f1a\u66f4\u52a0\u65b9\u4fbf\u3002\u5f15\u5bfc\u7a0b\u5e8f\u662f\u57fa\u4e8e Tugraph \u7684\u4e00\u4e2a\u7b80\u5355\u7684\u7a0b\u5e8f\u5b9e\u4f8b\uff0c\u6211\u4eec\u5c06\u91cd\u70b9\u4ecb\u7ecd\u5176\u4f7f\u7528\u65b9\u5f0f\u3002"}),"\n"]}),"\n",(0,l.jsx)(n.h2,{id:"1tugraph-\u56fe\u5b66\u4e60\u6a21\u5757\u7b80\u4ecb",children:"1.TuGraph \u56fe\u5b66\u4e60\u6a21\u5757\u7b80\u4ecb"}),"\n",(0,l.jsx)(n.p,{children:"\u56fe\u5b66\u4e60\u662f\u4e00\u79cd\u673a\u5668\u5b66\u4e60\u65b9\u6cd5\uff0c\u5176\u6838\u5fc3\u601d\u60f3\u662f\u5229\u7528\u56fe\u7ed3\u6784\u4e2d\u7684\u62d3\u6251\u4fe1\u606f\uff0c\u901a\u8fc7\u9876\u70b9\u4e4b\u95f4\u7684\u8054\u7cfb\u53ca\u89c4\u5f8b\u6765\u8fdb\u884c\u6570\u636e\u5206\u6790\u548c\u5efa\u6a21\u3002\u4e0d\u540c\u4e8e\u4f20\u7edf\u673a\u5668\u5b66\u4e60\u65b9\u6cd5\uff0c\u56fe\u5b66\u4e60\u5229\u7528\u7684\u6570\u636e\u5f62\u5f0f\u4e3a\u56fe\u7ed3\u6784\uff0c\u5176\u4e2d\u9876\u70b9\u8868\u793a\u6570\u636e\u4e2d\u7684\u5b9e\u4f53\uff0c\u800c\u8fb9\u5219\u8868\u793a\u5b9e\u4f53\u4e4b\u95f4\u7684\u5173\u7cfb\u3002\u901a\u8fc7\u5bf9\u8fd9\u4e9b\u9876\u70b9\u548c\u8fb9\u8fdb\u884c\u7279\u5f81\u63d0\u53d6\u548c\u6a21\u5f0f\u6316\u6398\uff0c\u53ef\u4ee5\u63ed\u793a\u51fa\u6570\u636e\u4e2d\u6df1\u5c42\u6b21\u7684\u5173\u8054\u548c\u89c4\u5f8b\uff0c\u4ece\u800c\u7528\u4e8e\u5404\u79cd\u5b9e\u9645\u5e94\u7528\u4e2d\u3002"}),"\n",(0,l.jsx)(n.p,{children:"\u8fd9\u4e2a\u6a21\u5757\u662f\u4e00\u4e2a\u57fa\u4e8e\u56fe\u6570\u636e\u5e93\u7684\u56fe\u5b66\u4e60\u6a21\u5757\uff0c\u4e3b\u8981\u63d0\u4f9b\u4e86\u56db\u79cd\u91c7\u6837\u7b97\u5b50\uff1aNeighbor Sampling\u3001Edge Sampling\u3001Random Walk Sampling \u548c Negative Sampling\u3002\u8fd9\u4e9b\u7b97\u5b50\u53ef\u4ee5\u7528\u4e8e\u5bf9\u56fe\u4e2d\u7684\u9876\u70b9\u548c\u8fb9\u8fdb\u884c\u91c7\u6837\uff0c\u4ece\u800c\u751f\u6210\u8bad\u7ec3\u6570\u636e\u3002\u91c7\u6837\u8fc7\u7a0b\u662f\u5728\u5e76\u884c\u8ba1\u7b97\u73af\u5883\u4e0b\u5b8c\u6210\u7684\uff0c\u5177\u6709\u9ad8\u6548\u6027\u548c\u53ef\u6269\u5c55\u6027\u3002"}),"\n",(0,l.jsx)(n.p,{children:"\u5728\u91c7\u6837\u540e\uff0c\u6211\u4eec\u53ef\u4ee5\u4f7f\u7528\u5f97\u5230\u7684\u8bad\u7ec3\u6570\u636e\u6765\u8bad\u7ec3\u4e00\u4e2a\u6a21\u578b\u3002\u8be5\u6a21\u578b\u53ef\u4ee5\u7528\u4e8e\u5404\u79cd\u56fe\u5b66\u4e60\u4efb\u52a1\uff0c\u6bd4\u5982\u9884\u6d4b\u3001\u5206\u7c7b\u7b49\u3002\u901a\u8fc7\u8bad\u7ec3\uff0c\u6a21\u578b\u53ef\u4ee5\u5b66\u4e60\u5230\u56fe\u4e2d\u7684\u9876\u70b9\u548c\u8fb9\u4e4b\u95f4\u7684\u5173\u7cfb\uff0c\u4ece\u800c\u80fd\u591f\u5bf9\u65b0\u7684\u9876\u70b9\u548c\u8fb9\u8fdb\u884c\u9884\u6d4b\u548c\u5206\u7c7b\u3002\u5728\u5b9e\u9645\u5e94\u7528\u4e2d\uff0c\u8fd9\u4e2a\u6a21\u5757\u53ef\u4ee5\u88ab\u7528\u6765\u5904\u7406\u5404\u79cd\u5927\u89c4\u6a21\u7684\u56fe\u6570\u636e\uff0c\u6bd4\u5982\u793e\u4ea4\u7f51\u7edc\u3001\u63a8\u8350\u7cfb\u7edf\u3001\u751f\u7269\u4fe1\u606f\u5b66\u7b49\u3002"}),"\n",(0,l.jsx)(n.h2,{id:"2-\u8fd0\u884c\u6d41\u7a0b",children:"2. \u8fd0\u884c\u6d41\u7a0b"}),"\n",(0,l.jsxs)(n.p,{children:["TuGraph \u56fe\u5b66\u4e60\u6a21\u5757\u5c06TuGraph\u4e2d\u7684\u56fe\u6570\u636e\u91c7\u6837\uff0c\u91c7\u6837\u540e\u7684\u9876\u70b9\u548c\u8fb9\u4f5c\u4e3a\u56fe\u5b66\u4e60\u7684\u7279\u5f81\uff0c\u8fdb\u884c\u5b66\u4e60\u8bad\u7ec3\u3002\u8fd0\u884c\u6d41\u7a0b\u5982\u4e0b\u56fe\u6240\u793a\uff1a\n",(0,l.jsx)(n.img,{alt:"Alt text",src:r(22992).A+"",width:"808",height:"1046"})]}),"\n",(0,l.jsx)(n.h2,{id:"3tugraph\u7f16\u8bd1\u53ca\u6570\u636e\u51c6\u5907",children:"3.TuGraph\u7f16\u8bd1\u53ca\u6570\u636e\u51c6\u5907"}),"\n",(0,l.jsxs)(n.p,{children:["TuGraph\u7f16\u8bd1\u8bf7\u53c2\u8003\uff1a",(0,l.jsx)(n.a,{href:"/tugraph-db/zh/4.3.0/installation&running/compile",children:"\u7f16\u8bd1"}),"\n\u5728build/output\u76ee\u5f55\u4e0b\u6267\u884c\uff1a"]}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-bash",children:"cp -r ../../test/integration/data/ ./ && cp -r ../../learn/examples/* ./\n"})}),"\n",(0,l.jsx)(n.p,{children:"\u8be5\u6307\u4ee4\u5c06\u6570\u636e\u96c6\u76f8\u5173\u6587\u4ef6\u62f7\u8d1d\u5230build/output\u76ee\u5f55\u4e0b\u3002"}),"\n",(0,l.jsx)(n.h2,{id:"4-\u6570\u636e\u5bfc\u5165",children:"4. \u6570\u636e\u5bfc\u5165"}),"\n",(0,l.jsxs)(n.p,{children:["\u6570\u636e\u5bfc\u5165\u8bf7\u53c2\u8003",(0,l.jsx)(n.a,{href:"/tugraph-db/zh/4.3.0/utility-tools/data-import",children:"\u6570\u636e\u5bfc\u5165"})]}),"\n",(0,l.jsx)(n.p,{children:"\u5bfc\u5165\u8fc7\u7a0b\u4ee5cora\u6570\u636e\u96c6\u4e3a\u4f8b\uff1a"}),"\n",(0,l.jsx)(n.p,{children:"\u5728build/output\u76ee\u5f55\u4e0b\u6267\u884c"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-bash",children:"./lgraph_import -c ./data/algo/cora.conf --dir ./coradb --overwrite 1\n"})}),"\n",(0,l.jsx)(n.p,{children:"\u5176\u4e2dcora.conf\u4e3a\u56feschema\u6587\u4ef6\uff0c\u4ee3\u8868\u56fe\u6570\u636e\u7684\u683c\u5f0f\u3002coradb\u4e3a\u5bfc\u5165\u540e\u7684\u56fe\u6570\u636e\u6587\u4ef6\u540d\u79f0\uff0c\u4ee3\u8868\u56fe\u6570\u636e\u7684\u5b58\u50a8\u4f4d\u7f6e\u3002"}),"\n",(0,l.jsx)(n.h2,{id:"5-feature\u7279\u5f81\u8f6c\u6362",children:"5. feature\u7279\u5f81\u8f6c\u6362"}),"\n",(0,l.jsx)(n.p,{children:"\u7531\u4e8e\u56fe\u5b66\u4e60\u4e2d\u7684feature\u7279\u5f81\u4e00\u822c\u8868\u793a\u4e3a\u8f83\u957f\u7684float\u7c7b\u578b\u6570\u7ec4\uff0cTuGraph\u6682\u4e0d\u652f\u6301float\u6570\u7ec4\u7c7b\u578b\u52a0\u8f7d\uff0c\u56e0\u6b64\u53ef\u5c06\u5176\u6309\u7167string\u7c7b\u578b\u5bfc\u5165\u540e\uff0c\u8f6c\u6362\u6210char*\u65b9\u4fbf\u540e\u7eed\u5b58\u53d6\uff0c\u5177\u4f53\u5b9e\u73b0\u53ef\u53c2\u8003feature_float.cpp\u6587\u4ef6\u3002\n\u5177\u4f53\u6267\u884c\u8fc7\u7a0b\u5982\u4e0b\uff1a"}),"\n",(0,l.jsxs)(n.p,{children:["\u5728build\u76ee\u5f55\u4e0b\u7f16\u8bd1\u5bfc\u5165plugin(\u5982\u679cTuGraph\u5df2\u7f16\u8bd1\u53ef\u8df3\u8fc7)\uff1a\n",(0,l.jsx)(n.code,{children:"make feature_float_embed"})]}),"\n",(0,l.jsxs)(n.p,{children:["\u5728build/output\u76ee\u5f55\u4e0b\u6267\u884c\n",(0,l.jsx)(n.code,{children:"./algo/feature_float_embed ./coradb"}),"\n\u5373\u53ef\u8fdb\u884c\u8f6c\u6362\u3002"]}),"\n",(0,l.jsx)(n.h2,{id:"6-\u91c7\u6837\u7b97\u5b50\u53ca\u7f16\u8bd1",children:"6. \u91c7\u6837\u7b97\u5b50\u53ca\u7f16\u8bd1"}),"\n",(0,l.jsx)(n.p,{children:"TuGraph\u5728cython\u5c42\u5b9e\u73b0\u4e86\u4e00\u79cd\u83b7\u53d6\u5168\u56fe\u6570\u636e\u7684\u7b97\u5b50\u53ca4\u79cd\u91c7\u6837\u7b97\u5b50\uff0c\u5177\u4f53\u5982\u4e0b\uff1a"}),"\n",(0,l.jsx)(n.h3,{id:"61\u91c7\u6837\u7b97\u5b50\u4ecb\u7ecd",children:"6.1.\u91c7\u6837\u7b97\u5b50\u4ecb\u7ecd"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,l.jsxs)(n.table,{children:[(0,l.jsx)(n.thead,{children:(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.th,{children:"\u91c7\u6837\u7b97\u5b50"}),(0,l.jsx)(n.th,{children:"\u91c7\u6837\u65b9\u5f0f"})]})}),(0,l.jsxs)(n.tbody,{children:[(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:"GetDB"}),(0,l.jsx)(n.td,{children:"\u4ece\u6570\u636e\u5e93\u4e2d\u83b7\u53d6\u56fe\u6570\u636e\u5e76\u8f6c\u6362\u6210\u6240\u9700\u6570\u636e\u7ed3\u6784"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:"Neighbor Sampling"}),(0,l.jsx)(n.td,{children:"\u6839\u636e\u7ed9\u5b9a\u7684\u9876\u70b9\u91c7\u6837\u5176\u90bb\u5c45\u9876\u70b9\uff0c\u5f97\u5230\u91c7\u6837\u5b50\u56fe"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:"Edge Sampling"}),(0,l.jsx)(n.td,{children:"\u6839\u636e\u91c7\u6837\u7387\u91c7\u6837\u56fe\u4e2d\u7684\u8fb9\uff0c\u5f97\u5230\u91c7\u6837\u5b50\u56fe"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:"Random Walk Sampling"}),(0,l.jsx)(n.td,{children:"\u6839\u636e\u7ed9\u5b9a\u7684\u9876\u70b9\uff0c\u8fdb\u884c\u968f\u673a\u6e38\u8d70\uff0c\u5f97\u5230\u91c7\u6837\u5b50\u56fe"})]}),(0,l.jsxs)(n.tr,{children:[(0,l.jsx)(n.td,{children:"Negative Sampling"}),(0,l.jsx)(n.td,{children:"\u751f\u6210\u4e0d\u5b58\u5728\u8fb9\u7684\u5b50\u56fe\u3002"})]})]})]}),"\n",(0,l.jsx)(n.h3,{id:"62\u7f16\u8bd1",children:"6.2.\u7f16\u8bd1"}),"\n",(0,l.jsxs)(n.p,{children:["\u5982\u679cTuGraph\u5df2\u7f16\u8bd1\uff0c\u53ef\u8df3\u8fc7\u6b64\u6b65\u9aa4\u3002\n\u5728tugraph-db/build\u6587\u4ef6\u5939\u4e0b\u6267\u884c\n",(0,l.jsx)(n.code,{children:"make -j2"})]}),"\n",(0,l.jsxs)(n.p,{children:["\u6216\u5728tugraph-db/learn/procedures\u6587\u4ef6\u5939\u4e0b\u6267\u884c\n",(0,l.jsx)(n.code,{children:"python3 setup.py build_ext -i"})]}),"\n",(0,l.jsx)(n.p,{children:"\u5f97\u5230\u7b97\u5b50so\u540e\uff0c\u5728Python\u4e2dimport \u5373\u53ef\u4f7f\u7528\u3002"}),"\n",(0,l.jsx)(n.h2,{id:"7-\u6a21\u578b\u8bad\u7ec3\u53ca\u4fdd\u5b58",children:"7. \u6a21\u578b\u8bad\u7ec3\u53ca\u4fdd\u5b58"}),"\n",(0,l.jsxs)(n.p,{children:["TuGraph\u5728python\u5c42\u8c03\u7528cython\u5c42\u7684\u7b97\u5b50\uff0c\u5b9e\u73b0\u56fe\u5b66\u4e60\u6a21\u578b\u7684\u8bad\u7ec3\u3002\n\u4f7f\u7528 TuGraph \u56fe\u5b66\u4e60\u6a21\u5757\u4f7f\u7528\u65b9\u5f0f\u4ecb\u7ecd\u5982\u4e0b:\n\u5728build/output\u6587\u4ef6\u5939\u4e0b\u6267\u884c\n",(0,l.jsx)(n.code,{children:"python3 train_full_cora.py --model_save_path ./cora_model"}),"\n\u5373\u53ef\u8fdb\u884c\u8bad\u7ec3\u3002\n\u6700\u7ec8\u6253\u5370loss\u6570\u503c\u5c0f\u4e8e0.9\uff0c\u5373\u4e3a\u8bad\u7ec3\u6210\u529f\u3002\u81f3\u6b64\uff0c\u56fe\u6a21\u578b\u8bad\u7ec3\u5b8c\u6210\uff0c\u6a21\u578b\u4fdd\u5b58\u5728cora_model\u6587\u4ef6\u3002"]}),"\n",(0,l.jsx)(n.h2,{id:"8-\u6a21\u578b\u52a0\u8f7d",children:"8. \u6a21\u578b\u52a0\u8f7d"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-python",children:"model = build_model()\nmodel.load_state_dict(torch.load(model_save_path))\nmodel.eval()\n"})}),"\n",(0,l.jsx)(n.p,{children:"\u5728\u4f7f\u7528\u4fdd\u5b58\u7684\u6a21\u578b\u4e4b\u524d\uff0c\u9996\u5148\u9700\u8981\u5bf9\u5176\u8fdb\u884c\u52a0\u8f7d\u3002\u4ee3\u7801\u4e2d\uff0c\u4f7f\u7528\u5982\u4e0a\u7684\u4ee3\u7801\u5bf9\u5df2\u8bad\u7ec3\u6a21\u578b\u8fdb\u884c\u52a0\u8f7d\u3002"}),"\n",(0,l.jsx)(n.p,{children:"\u52a0\u8f7d\u4e4b\u540e\uff0c\u6211\u4eec\u53ef\u4ee5\u4f7f\u7528\u6a21\u578b\u5bf9\u65b0\u7684\u9876\u70b9\u548c\u8fb9\u8fdb\u884c\u9884\u6d4b\u548c\u5206\u7c7b\u3002\u5728\u9884\u6d4b\u65f6\uff0c\u6211\u4eec\u53ef\u4ee5\u8f93\u5165\u4e00\u4e2a\u6216\u591a\u4e2a\u9876\u70b9\uff0c\u6a21\u578b\u5c06\u8f93\u51fa\u76f8\u5e94\u7684\u9884\u6d4b\u7ed3\u679c\u3002\u5728\u5206\u7c7b\u65f6\uff0c\u6211\u4eec\u53ef\u4ee5\u5c06\u6574\u4e2a\u56fe\u4f5c\u4e3a\u8f93\u5165\uff0c\u6a21\u578b\u5c06\u5bf9\u56fe\u4e2d\u7684\u9876\u70b9\u548c\u8fb9\u8fdb\u884c\u5206\u7c7b\uff0c\u4ee5\u5b9e\u73b0\u4efb\u52a1\u7684\u76ee\u6807\u3002\n\u4f7f\u7528\u5df2\u8bad\u7ec3\u7684\u6a21\u578b\u53ef\u4ee5\u907f\u514d\u91cd\u65b0\u8bad\u7ec3\u6a21\u578b\u7684\u65f6\u95f4\u548c\u8d44\u6e90\u6d88\u8017\u3002\u6b64\u5916\uff0c\u7531\u4e8e\u6a21\u578b\u5df2\u7ecf\u5b66\u4e60\u5230\u4e86\u56fe\u6570\u636e\u4e2d\u9876\u70b9\u548c\u8fb9\u4e4b\u95f4\u7684\u5173\u7cfb\uff0c\u5b83\u53ef\u4ee5\u5f88\u597d\u5730\u9002\u5e94\u65b0\u7684\u6570\u636e\uff0c\u4ece\u800c\u63d0\u9ad8\u9884\u6d4b\u548c\u5206\u7c7b\u7684\u51c6\u786e\u5ea6\u3002"})]})}function o(e={}){const{wrapper:n}={...(0,t.R)(),...e.components};return n?(0,l.jsx)(n,{...e,children:(0,l.jsx)(c,{...e})}):c(e)}}}]);