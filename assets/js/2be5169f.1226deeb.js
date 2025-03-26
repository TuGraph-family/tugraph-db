"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[90025],{28453:(e,n,r)=>{r.d(n,{R:()=>a,x:()=>o});var t=r(96540);const i={},l=t.createContext(i);function a(e){const n=t.useContext(l);return t.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function o(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(i):e.components||i:a(e.components),t.createElement(l.Provider,{value:n},e.children)}},57002:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>s,contentTitle:()=>a,default:()=>d,frontMatter:()=>l,metadata:()=>o,toc:()=>u});var t=r(74848),i=r(28453);const l={},a="\u73af\u5883\u5206\u7c7b",o={id:"developer-manual/installation/environment-mode",title:"\u73af\u5883\u5206\u7c7b",description:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph \u6d89\u53ca\u7684\u4e09\u79cd\u73af\u5883\u3002",source:"@site/versions/version-4.1.0/zh-CN/source/5.developer-manual/1.installation/2.environment-mode.md",sourceDirName:"5.developer-manual/1.installation",slug:"/developer-manual/installation/environment-mode",permalink:"/tugraph-db/zh/4.1.0/developer-manual/installation/environment-mode",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:2,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"\u73af\u5883\u51c6\u5907",permalink:"/tugraph-db/zh/4.1.0/developer-manual/installation/environment"},next:{title:"Docker\u90e8\u7f72",permalink:"/tugraph-db/zh/4.1.0/developer-manual/installation/docker-deployment"}},s={},u=[{value:"1.\u5206\u7c7b",id:"1\u5206\u7c7b",level:2},{value:"2.\u4f9d\u8d56\u7cfb\u7edf\u5e93",id:"2\u4f9d\u8d56\u7cfb\u7edf\u5e93",level:2}];function c(e){const n={blockquote:"blockquote",h1:"h1",h2:"h2",header:"header",li:"li",p:"p",ul:"ul",...(0,i.R)(),...e.components};return(0,t.jsxs)(t.Fragment,{children:[(0,t.jsx)(n.header,{children:(0,t.jsx)(n.h1,{id:"\u73af\u5883\u5206\u7c7b",children:"\u73af\u5883\u5206\u7c7b"})}),"\n",(0,t.jsxs)(n.blockquote,{children:["\n",(0,t.jsx)(n.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph \u6d89\u53ca\u7684\u4e09\u79cd\u73af\u5883\u3002"}),"\n"]}),"\n",(0,t.jsx)(n.h2,{id:"1\u5206\u7c7b",children:"1.\u5206\u7c7b"}),"\n",(0,t.jsx)(n.p,{children:"\u6839\u636e\u73af\u5883\u6240\u627f\u8f7d\u529f\u80fd\u7684\u4e0d\u540c\uff0c\u533a\u5206\u4e3a\u7f16\u8bd1\u73af\u5883\uff0c\u8fd0\u884c\u73af\u5883\uff0c\u4ee5\u53ca\u7cbe\u7b80\u8fd0\u884c\u73af\u5883\u3002"}),"\n",(0,t.jsxs)(n.ul,{children:["\n",(0,t.jsx)(n.li,{children:"\u7f16\u8bd1\u73af\u5883\uff0c\u5177\u5907TuGraph\u7f16\u8bd1\u7684\u6240\u6709\u4f9d\u8d56\u5e93\uff0c\u5305\u542b\u8fd0\u884c\u73af\u5883\u7684\u6240\u6709\u4f9d\u8d56\uff0c\u5e76\u4e14\u80fd\u591f\u7f16\u8bd1TuGraph\u6e90\u7801\uff0c\u4f46\u4e0d\u5305\u542b\u9884\u7f16\u8bd1\u597d\u7684TuGraph\u53ef\u6267\u884c\u6587\u4ef6\u548c\u5e93\u6587\u4ef6\uff0c\u4f9b\u5f00\u53d1\u8005\u7f16\u8bd1\u6e90\u7801\u4f7f\u7528\u3002"}),"\n",(0,t.jsx)(n.li,{children:"\u8fd0\u884c\u73af\u5883\uff0c\u5177\u5907GCC/Java/Python\u73af\u5883\uff0c\u80fd\u591f\u8fd0\u884cTuGraph\u7684\u6240\u6709\u529f\u80fd\uff0c\u5e76\u4e14\u80fd\u627f\u8f7d\u5168\u6587\u7d22\u5f15\uff0cjava client\uff0cc++\u6e90\u7801\u4e0a\u4f20\u4e3aplugin\uff0c\u4ee5\u53capython plugin\u7684\u5b8c\u6574\u529f\u80fd\uff0c\u5185\u7f6eTuGraph\u9884\u7f16\u8bd1\u597d\u7684\u53ef\u6267\u884c\u6587\u4ef6\u548c\u5e93\u6587\u4ef6\uff0c\u4f9b\u5ba2\u6237\u76f4\u63a5\u5b89\u88c5\u4f7f\u7528\uff0c\u65e0\u9700\u7f16\u8bd1\u6e90\u7801\u3002"}),"\n",(0,t.jsx)(n.li,{children:"\u7cbe\u7b80\u8fd0\u884c\u73af\u5883\uff0c\u7ea6\u7b49\u4e8e\u88f8\u7cfb\u7edf\u52a0\u9884\u7f16\u8bd1TuGraph\uff0c\u4ec5\u80fd\u8fd0\u884cTuGraph\u7684\u57fa\u672c\u529f\u80fd\uff0c\u65e0C++ plugin\u7f16\u8bd1\u8fd0\u884c\uff0c\u4ec5so\u4e0a\u4f20\uff0c\u65e0\u5168\u6587\u7d22\u5f15\uff0c\u65e0python plugin\uff0c\u4f9b\u5feb\u901f\u642d\u5efa\u8bd5\u7528\u3002"}),"\n"]}),"\n",(0,t.jsx)(n.p,{children:"TuGraph\u7f16\u8bd1\u540e\uff0c\u4f1a\u628a\u6240\u6709\u7684\u4f9d\u8d56\u5e93\u4ee5.a\u7684\u5f62\u5f0f\u6253\u5305\u5728\u4e00\u8d77\uff0c\u56e0\u6b64\u539f\u5219\u4e0a\u8fd0\u884c\u4e0d\u9700\u8981\u7684\u5176\u4ed6\u7684\u4f9d\u8d56\u5e93\u3002\u4f46TuGraph\u652f\u6301\u5b58\u50a8\u8fc7\u7a0b\uff0c\u5373\u5728\u670d\u52a1\u7aef\u7f16\u8bd1C++\u4ee3\u7801\uff0c\u56e0\u6b64\u5728\u73af\u5883\u4e2d\u4f9d\u7136\u9700\u8981\u6d89\u53ca\u7684\u7f16\u8bd1\u5668\u3002"}),"\n",(0,t.jsx)(n.h2,{id:"2\u4f9d\u8d56\u7cfb\u7edf\u5e93",children:"2.\u4f9d\u8d56\u7cfb\u7edf\u5e93"}),"\n",(0,t.jsx)(n.p,{children:"\u9488\u5bf9\u4e09\u79cd\u73af\u5883\uff0c\u9664\u53bbTuGraph\u7684\u8fd0\u884c\u5305\uff0c\u6240\u9700\u8981\u7684\u7cfb\u7edf\u5e93\u5982\u4e0b\uff1a"}),"\n",(0,t.jsxs)(n.ul,{children:["\n",(0,t.jsx)(n.li,{children:"\u7f16\u8bd1\u73af\u5883\uff0c\u5305\u62ecgcc\u3001python\u3001java\u7b49\u7f16\u8bd1\u5668\uff0c\u4e5f\u5305\u542bantlr4\u3001pybind11\u7b49\uff0c\u5177\u4f53\u53c2\u89c1tugraph-db\u6e90\u7801\u76ee\u5f55 ci/images/tugraph-compile-*-Dockerfile\u3002"}),"\n",(0,t.jsx)(n.li,{children:"\u8fd0\u884c\u73af\u5883\uff0c\u4e3b\u8981\u7531\u5b58\u50a8\u8fc7\u7a0b\u5f15\u5165\uff0c\u5305\u62ecgcc\u3001boost\u3001cmake\u7b49\uff0c\u5177\u4f53\u53c2\u89c1tugraph-db\u6e90\u7801\u76ee\u5f55 ci/images/tugraph-runtime-*-Dockerfile\u3002"}),"\n",(0,t.jsx)(n.li,{children:"\u7cbe\u7b80\u8fd0\u884c\u73af\u5883\uff0c\u65e0\uff0c\u53ef\u4ee5\u53c2\u89c1tugraph-db\u6e90\u7801\u76ee\u5f55 ci/images/ tugraph-mini-runtime-*-Dockerfile\u3002"}),"\n"]})]})}function d(e={}){const{wrapper:n}={...(0,i.R)(),...e.components};return n?(0,t.jsx)(n,{...e,children:(0,t.jsx)(c,{...e})}):c(e)}}}]);