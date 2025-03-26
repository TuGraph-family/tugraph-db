"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[46831],{28453:(e,n,r)=>{r.d(n,{R:()=>c,x:()=>i});var s=r(96540);const l={},d=s.createContext(l);function c(e){const n=s.useContext(d);return s.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function i(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(l):e.components||l:c(e.components),s.createElement(d.Provider,{value:n},e.children)}},71968:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>h,contentTitle:()=>c,default:()=>t,frontMatter:()=>d,metadata:()=>i,toc:()=>o});var s=r(74848),l=r(28453);const d={},c="\u90e8\u7f72\u9ad8\u53ef\u7528\u6a21\u5f0f",i={id:"installation&running/high-availability-mode",title:"\u90e8\u7f72\u9ad8\u53ef\u7528\u6a21\u5f0f",description:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd\u4e86\u9ad8\u53ef\u7528\u6a21\u5f0f\u7684\u539f\u7406\u3001\u51c6\u5907\u5de5\u4f5c\u3001\u4ee5\u53ca\u670d\u52a1\u5668\u7684\u64cd\u4f5c\u8bf4\u660e",source:"@site/versions/version-4.3.2/zh-CN/source/5.installation&running/8.high-availability-mode.md",sourceDirName:"5.installation&running",slug:"/installation&running/high-availability-mode",permalink:"/tugraph-db/zh/4.3.2/installation&running/high-availability-mode",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:8,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"\u6570\u636e\u5e93\u8fd0\u884c",permalink:"/tugraph-db/zh/4.3.2/installation&running/tugraph-running"},next:{title:"\u6570\u636e\u5bfc\u5165",permalink:"/tugraph-db/zh/4.3.2/utility-tools/data-import"}},h={},o=[{value:"1.\u539f\u7406",id:"1\u539f\u7406",level:2},{value:"2.\u51c6\u5907\u5de5\u4f5c",id:"2\u51c6\u5907\u5de5\u4f5c",level:2},{value:"3.\u542f\u52a8\u521d\u59cb\u5907\u4efd\u7ec4",id:"3\u542f\u52a8\u521d\u59cb\u5907\u4efd\u7ec4",level:2},{value:"3.1.\u521d\u59cb\u6570\u636e\u4e00\u81f4",id:"31\u521d\u59cb\u6570\u636e\u4e00\u81f4",level:3},{value:"3.2.\u521d\u59cb\u6570\u636e\u4e0d\u4e00\u81f4",id:"32\u521d\u59cb\u6570\u636e\u4e0d\u4e00\u81f4",level:3},{value:"4.\u542f\u52a8witness\u8282\u70b9",id:"4\u542f\u52a8witness\u8282\u70b9",level:2},{value:"4.1.\u4e0d\u5141\u8bb8witness\u8282\u70b9\u6210\u4e3aleader",id:"41\u4e0d\u5141\u8bb8witness\u8282\u70b9\u6210\u4e3aleader",level:3},{value:"4.2.\u5141\u8bb8witness\u8282\u70b9\u6210\u4e3aleader",id:"42\u5141\u8bb8witness\u8282\u70b9\u6210\u4e3aleader",level:3},{value:"5.\u6a2a\u5411\u6269\u5c55\u5176\u4ed6\u670d\u52a1\u5668",id:"5\u6a2a\u5411\u6269\u5c55\u5176\u4ed6\u670d\u52a1\u5668",level:2},{value:"6.\u505c\u6b62\u670d\u52a1\u5668",id:"6\u505c\u6b62\u670d\u52a1\u5668",level:2},{value:"7.\u91cd\u542f\u670d\u52a1\u5668",id:"7\u91cd\u542f\u670d\u52a1\u5668",level:2},{value:"8.docker\u90e8\u7f72\u9ad8\u53ef\u7528\u96c6\u7fa4",id:"8docker\u90e8\u7f72\u9ad8\u53ef\u7528\u96c6\u7fa4",level:2},{value:"8.1.\u5b89\u88c5\u955c\u50cf",id:"81\u5b89\u88c5\u955c\u50cf",level:3},{value:"8.2.\u521b\u5efa\u5bb9\u5668",id:"82\u521b\u5efa\u5bb9\u5668",level:3},{value:"8.3.\u542f\u52a8\u670d\u52a1",id:"83\u542f\u52a8\u670d\u52a1",level:3},{value:"9.\u67e5\u770b\u670d\u52a1\u5668\u72b6\u6001",id:"9\u67e5\u770b\u670d\u52a1\u5668\u72b6\u6001",level:2},{value:"10.\u9ad8\u53ef\u7528\u6a21\u5f0f\u4e0b\u6570\u636e\u540c\u6b65\u95ee\u9898",id:"10\u9ad8\u53ef\u7528\u6a21\u5f0f\u4e0b\u6570\u636e\u540c\u6b65\u95ee\u9898",level:2}];function a(e){const n={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",li:"li",ol:"ol",p:"p",pre:"pre",strong:"strong",ul:"ul",...(0,l.R)(),...e.components};return(0,s.jsxs)(s.Fragment,{children:[(0,s.jsx)(n.header,{children:(0,s.jsx)(n.h1,{id:"\u90e8\u7f72\u9ad8\u53ef\u7528\u6a21\u5f0f",children:"\u90e8\u7f72\u9ad8\u53ef\u7528\u6a21\u5f0f"})}),"\n",(0,s.jsxs)(n.blockquote,{children:["\n",(0,s.jsx)(n.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd\u4e86\u9ad8\u53ef\u7528\u6a21\u5f0f\u7684\u539f\u7406\u3001\u51c6\u5907\u5de5\u4f5c\u3001\u4ee5\u53ca\u670d\u52a1\u5668\u7684\u64cd\u4f5c\u8bf4\u660e"}),"\n"]}),"\n",(0,s.jsx)(n.h2,{id:"1\u539f\u7406",children:"1.\u539f\u7406"}),"\n",(0,s.jsx)(n.p,{children:"TuGraph \u901a\u8fc7\u591a\u673a\u70ed\u5907\u4efd\u6765\u63d0\u4f9b\u9ad8\u53ef\u7528\uff08HA\uff09\u6a21\u5f0f\u3002\u5728\u9ad8\u53ef\u7528\u6a21\u5f0f\u4e0b\uff0c\u5bf9\u6570\u636e\u5e93\u7684\u5199\u64cd\u4f5c\u4f1a\u88ab\u540c\u6b65\u5230\u6240\u6709\u670d\u52a1\u5668\uff08\u975ewitness\uff09\u4e0a\uff0c\u8fd9\u6837\u5373\u4f7f\u6709\u90e8\u5206\u670d\u52a1\u5668\u5b95\u673a\u4e5f\u4e0d\u4f1a\u5f71\u54cd\u670d\u52a1\u7684\u53ef\u7528\u6027\u3002"}),"\n",(0,s.jsxs)(n.p,{children:["\u9ad8\u53ef\u7528\u6a21\u5f0f\u542f\u52a8\u65f6\uff0c\u591a\u4e2a TuGraph \u670d\u52a1\u5668\u7ec4\u6210\u4e00\u4e2a\u5907\u4efd\u7ec4\uff0c\u5373\u9ad8\u53ef\u7528\u96c6\u7fa4\u3002\u6bcf\u4e2a\u5907\u4efd\u7ec4\u7531\u4e09\u4e2a\u6216\u66f4\u591a TuGraph \u670d\u52a1\u5668\u7ec4\u6210\uff0c\u5176\u4e2d\u67d0\u53f0\u670d\u52a1\u5668\u4f1a\u4f5c\u4e3a",(0,s.jsx)(n.code,{children:"leader"}),"\uff0c\u800c\u5176\u4ed6\u590d\u5236\u7ec4\u670d\u52a1\u5668\u5219\u4f5c\u4e3a",(0,s.jsx)(n.code,{children:"follower"}),"\u3002\u5199\u5165\u8bf7\u6c42\u7531",(0,s.jsx)(n.code,{children:"leader"}),"\n\u63d0\u4f9b\u670d\u52a1\uff0c\u8be5",(0,s.jsx)(n.code,{children:"leader"}),"\u5c06\u6bcf\u4e2a\u8bf7\u6c42\u590d\u5236\u540c\u6b65\u5230",(0,s.jsx)(n.code,{children:"follower"}),"\uff0c\u5e76\u5728\u8bf7\u6c42\u540c\u6b65\u5230\u670d\u52a1\u5668\u540e\u624d\u80fd\u54cd\u5e94\u5ba2\u6237\u7aef\u3002\u8fd9\u6837\uff0c\u5982\u679c\u4efb\u4f55\u670d\u52a1\u5668\u53d1\u751f\u6545\u969c\uff0c\u5176\u4ed6\u670d\u52a1\u5668\u4ecd\u5c06\u5177\u6709\u5230\u76ee\u524d\u4e3a\u6b62\u5df2\u5199\u5165\u7684\u6240\u6709\u6570\u636e\u3002\u5982\u679c",(0,s.jsx)(n.code,{children:"leader"}),"\n\u670d\u52a1\u5668\u53d1\u751f\u6545\u969c\uff0c\u5176\u4ed6\u670d\u52a1\u5668\u5c06\u81ea\u52a8\u9009\u62e9\u51fa\u65b0\u7684",(0,s.jsx)(n.code,{children:"leader"}),"\u3002"]}),"\n",(0,s.jsxs)(n.p,{children:["TuGraph\u7684\u9ad8\u53ef\u7528\u6a21\u5f0f\u63d0\u4f9b\u4e24\u79cd\u7c7b\u578b\u7684\u8282\u70b9\uff1a",(0,s.jsx)(n.code,{children:"replica"}),"\u8282\u70b9\u548c",(0,s.jsx)(n.code,{children:"witness"}),"\u8282\u70b9\u3002\u5176\u4e2d\uff0c",(0,s.jsx)(n.code,{children:"replica"}),"\u8282\u70b9\u662f\u666e\u901a\u8282\u70b9\uff0c\u6709\u65e5\u5fd7\u6709\u6570\u636e\uff0c\u53ef\u5bf9\u5916\u63d0\u4f9b\u670d\u52a1\u3002\n\u800c",(0,s.jsx)(n.code,{children:"witness"}),"\u8282\u70b9\u662f\u4e00\u79cd\u53ea\u63a5\u6536\u5fc3\u8df3\u548c\u65e5\u5fd7\u4f46\u4e0d\u4fdd\u5b58\u6570\u636e\u7684\u8282\u70b9\u3002\u6839\u636e\u90e8\u7f72\u9700\u6c42\uff0c",(0,s.jsx)(n.code,{children:"leader"}),"\u8282\u70b9\u548c",(0,s.jsx)(n.code,{children:"follower"}),"\u8282\u70b9\u53ef\u4ee5\u7075\u6d3b\u7684\u90e8\u7f72\u4e3a",(0,s.jsx)(n.code,{children:"replica"}),"\u8282\u70b9\u6216",(0,s.jsx)(n.code,{children:"witness"}),"\u8282\u70b9\u3002\n\u57fa\u4e8e\u6b64\uff0cTuGraph\u9ad8\u53ef\u7528\u6a21\u5f0f\u7684\u90e8\u7f72\u65b9\u5f0f\u6709\u4e24\u79cd\uff1a\u4e00\u662f\u666e\u901a\u90e8\u7f72\u6a21\u5f0f\uff0c\u4e8c\u662f\u5e26witness\u7684\u7b80\u7ea6\u90e8\u7f72\u6a21\u5f0f\u3002"]}),"\n",(0,s.jsxs)(n.p,{children:["\u5bf9\u4e8e\u666e\u901a\u90e8\u7f72\u6a21\u5f0f\uff0c",(0,s.jsx)(n.code,{children:"leader"}),"\u548c\u6240\u6709",(0,s.jsx)(n.code,{children:"follower"}),"\u5747\u4e3a",(0,s.jsx)(n.code,{children:"replica"}),"\u7c7b\u578b\u7684\u8282\u70b9\u3002\u5199\u5165\u8bf7\u6c42\u7531",(0,s.jsx)(n.code,{children:"leader"}),"\u63d0\u4f9b\u670d\u52a1\uff0c\u8be5",(0,s.jsx)(n.code,{children:"leader"}),"\u5c06\u6bcf\u4e2a\u8bf7\u6c42\u590d\u5236\u540c\u6b65\u5230",(0,s.jsx)(n.code,{children:"follower"}),"\uff0c\n\u5e76\u5728\u8bf7\u6c42\u540c\u6b65\u5230\u8d85\u8fc7\u534a\u6570\u7684\u670d\u52a1\u5668\u540e\u624d\u80fd\u54cd\u5e94\u5ba2\u6237\u7aef\u3002\u8fd9\u6837\uff0c\u5982\u679c\u5c11\u4e8e\u534a\u6570\u7684\u670d\u52a1\u5668\u53d1\u751f\u6545\u969c\uff0c\u5176\u4ed6\u670d\u52a1\u5668\u4ecd\u5c06\u5177\u6709\u5230\u76ee\u524d\u4e3a\u6b62\u5df2\u5199\u5165\u7684\u6240\u6709\u6570\u636e\u3002\u5982\u679c",(0,s.jsx)(n.code,{children:"leader"}),"\n\u670d\u52a1\u5668\u53d1\u751f\u6545\u969c\uff0c\u5176\u4ed6\u670d\u52a1\u5668\u5c06\u81ea\u52a8\u9009\u4e3e\u51fa\u65b0\u7684",(0,s.jsx)(n.code,{children:"leader"}),"\uff0c\u901a\u8fc7\u8fd9\u79cd\u65b9\u5f0f\u4fdd\u8bc1\u6570\u636e\u7684\u4e00\u81f4\u6027\u548c\u670d\u52a1\u7684\u53ef\u7528\u6027\u3002"]}),"\n",(0,s.jsxs)(n.p,{children:["\u7136\u800c\uff0c\u5728\u7528\u6237\u670d\u52a1\u5668\u8d44\u6e90\u4e0d\u591f\u6216\u8005\u53d1\u751f\u7f51\u7edc\u5206\u533a\u65f6\uff0c\u4e0d\u80fd\u5efa\u7acb\u6b63\u5e38\u7684HA\u96c6\u7fa4\u3002\u6b64\u65f6\uff0c\u7531\u4e8e",(0,s.jsx)(n.code,{children:"witness"}),"\u8282\u70b9\u6ca1\u6709\u6570\u636e\uff0c\u5bf9\u8d44\u6e90\u5360\u7528\u5c0f\uff0c\u53ef\u4ee5\u5c06",(0,s.jsx)(n.code,{children:"witness"}),"\u8282\u70b9\u548c",(0,s.jsx)(n.code,{children:"replica"}),"\u8282\u70b9\u90e8\u7f72\u5728\u4e00\u53f0\u673a\u5668\u4e0a\u3002\n\u4f8b\u5982\uff0c\u5f53\u53ea\u67092\u53f0\u673a\u5668\u7684\u60c5\u51b5\u4e0b\uff0c\u53ef\u4ee5\u5728\u4e00\u53f0\u673a\u5668\u4e0a\u90e8\u7f72",(0,s.jsx)(n.code,{children:"replica"}),"\u8282\u70b9\uff0c\u5728\u53e6\u4e00\u53f0\u673a\u5668\u4e0a\u90e8\u7f72",(0,s.jsx)(n.code,{children:"replica"}),"\u8282\u70b9\u548c",(0,s.jsx)(n.code,{children:"witness"}),"\u8282\u70b9\uff0c\u4e0d\u4ec5\u8282\u7701\u8d44\u6e90\uff0c\u800c\u4e14\u4e0d\u9700\u8981\u628a\u65e5\u5fd7\u5e94\u7528\u5230\u72b6\u6001\u673a\u4e0a\uff0c\n\u4e5f\u4e0d\u9700\u8981\u751f\u6210\u548c\u5b89\u88c5\u5feb\u7167\uff0c\u56e0\u6b64\u54cd\u5e94\u8bf7\u6c42\u7684\u901f\u5ea6\u5f88\u5feb\uff0c\u53ef\u4ee5\u5728\u96c6\u7fa4\u5d29\u6e83\u6216\u7f51\u7edc\u5206\u533a\u65f6\u534f\u52a9\u5feb\u901f\u9009\u4e3e\u51fa\u65b0\u7684",(0,s.jsx)(n.code,{children:"leader"}),"\uff0c\u8fd9\u5c31\u662fTuGraph HA\u96c6\u7fa4\u7684\u7b80\u7ea6\u90e8\u7f72\u6a21\u5f0f\u3002\n\u5c3d\u7ba1",(0,s.jsx)(n.code,{children:"witness"}),"\u8282\u70b9\u6709\u8bf8\u591a\u597d\u5904\uff0c\u4f46\u662f\u7531\u4e8e\u6ca1\u6709\u6570\u636e\uff0c\u96c6\u7fa4\u5b9e\u9645\u4e0a\u589e\u52a0\u4e86\u4e00\u4e2a\u4e0d\u80fd\u6210\u4e3a",(0,s.jsx)(n.code,{children:"leader"}),"\u7684\u8282\u70b9\uff0c\u56e0\u6b64\u53ef\u7528\u6027\u4f1a\u7565\u6709\u964d\u4f4e\u3002\u4e3a\u63d0\u9ad8\u96c6\u7fa4\u7684\u53ef\u7528\u6027\uff0c\n\u53ef\u901a\u8fc7\u6307\u5b9a",(0,s.jsx)(n.code,{children:"ha_enable_witness_to_leader"}),"\u53c2\u6570\u4e3a",(0,s.jsx)(n.code,{children:"true"}),"\uff0c\u5141\u8bb8",(0,s.jsx)(n.code,{children:"witness"}),"\u8282\u70b9\u4e34\u65f6\u5f53\u4e3b\u3002",(0,s.jsx)(n.code,{children:"witness"}),"\u8282\u70b9\u5728\u628a\u65b0\u65e5\u5fd7\u540c\u6b65\u5230\u5176\u4ed6\u8282\u70b9\u4e4b\u540e\uff0c\n\u4f1a\u5c06leader\u89d2\u8272\u4e3b\u52a8\u5207\u6362\u5230\u6709\u6700\u65b0\u65e5\u5fd7\u7684\u8282\u70b9\u3002"]}),"\n",(0,s.jsx)(n.p,{children:"v3.6\u53ca\u4ee5\u4e0a\u7248\u672c\u652f\u6301\u6b64\u529f\u80fd\u3002"}),"\n",(0,s.jsx)(n.h2,{id:"2\u51c6\u5907\u5de5\u4f5c",children:"2.\u51c6\u5907\u5de5\u4f5c"}),"\n",(0,s.jsx)(n.p,{children:"\u8981\u542f\u7528\u9ad8\u53ef\u7528\u6a21\u5f0f\uff0c\u7528\u6237\u9700\u8981\uff1a"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:["\n",(0,s.jsx)(n.p,{children:"\u4e09\u53f0\u53ca\u4ee5\u4e0a\u7684 TuGraph \u670d\u52a1\u5668\u5b9e\u4f8b\u3002"}),"\n"]}),"\n",(0,s.jsxs)(n.li,{children:["\n",(0,s.jsxs)(n.p,{children:["\u5728\u542f\u52a8 lgraph_server \u65f6\u6253\u5f00\u9ad8\u53ef\u7528\u6a21\u5f0f\uff0c\u53ef\u4ee5\u4f7f\u7528\u914d\u7f6e\u6587\u4ef6\u6216\u8005\u547d\u4ee4\u884c\u5c06",(0,s.jsx)(n.code,{children:"enable_ha"}),"\u9009\u9879\u8bbe\u7f6e\u4e3a",(0,s.jsx)(n.code,{children:"true"}),"\u3002"]}),"\n"]}),"\n",(0,s.jsxs)(n.li,{children:["\n",(0,s.jsxs)(n.p,{children:["\u8bbe\u7f6e\u6b63\u786e\u7684",(0,s.jsx)(n.code,{children:"rpc_port"}),"\uff0c\u53ef\u901a\u8fc7\u914d\u7f6e\u6587\u4ef6\u6216\u8005\u547d\u4ee4\u884c\u8bbe\u7f6e\u3002"]}),"\n"]}),"\n"]}),"\n",(0,s.jsx)(n.h2,{id:"3\u542f\u52a8\u521d\u59cb\u5907\u4efd\u7ec4",children:"3.\u542f\u52a8\u521d\u59cb\u5907\u4efd\u7ec4"}),"\n",(0,s.jsxs)(n.p,{children:["\u5b89\u88c5\u597dTuGraph\u4e4b\u540e\uff0c\u53ef\u4ee5\u4f7f\u7528",(0,s.jsx)(n.code,{children:"lgraph_server"}),"\u547d\u4ee4\u5728\u4e0d\u540c\u7684\u673a\u5668\u4e0a\u542f\u52a8\u9ad8\u53ef\u7528\u96c6\u7fa4\u3002\u672c\u8282\u4e3b\u8981\u8bb2\u89e3\u9ad8\u53ef\u7528\u96c6\u7fa4\u7684\u542f\u52a8\u65b9\u5f0f\uff0c\u542f\u52a8\u4e4b\u540e\u7684\u96c6\u7fa4\u72b6\u6001\u7ba1\u7406\u53c2\u89c1",(0,s.jsx)(n.a,{href:"/tugraph-db/zh/4.3.2/utility-tools/ha-cluster-management",children:"lgraph_peer\u5de5\u5177"})]}),"\n",(0,s.jsx)(n.h3,{id:"31\u521d\u59cb\u6570\u636e\u4e00\u81f4",children:"3.1.\u521d\u59cb\u6570\u636e\u4e00\u81f4"}),"\n",(0,s.jsxs)(n.p,{children:["\u5f53\u542f\u52a8\u65f6\u6240\u6709\u670d\u52a1\u5668\u4e2d\u7684\u6570\u636e\u76f8\u540c\u6216\u6ca1\u6709\u6570\u636e\u65f6\uff0c\u7528\u6237\u53ef\u4ee5\u901a\u8fc7\u6307\u5b9a",(0,s.jsx)(n.code,{children:"--ha_conf host1:port1,host2:port2"}),"\u542f\u52a8\u670d\u52a1\u5668\u3002\n\u8fd9\u79cd\u65b9\u5f0f\u53ef\u4ee5\u5c06\u51c6\u5907\u597d\u7684\u6240\u6709TuGraph\u5b9e\u4f8b\u4e00\u6b21\u6027\u52a0\u5165\u521d\u59cb\u5907\u4efd\u7ec4\uff0c\u7531\u5907\u4efd\u7ec4\u4e2d\u7684\u6240\u6709\u670d\u52a1\u5668\u6839\u636eraft\u534f\u8bae\u9009\u4e3e\u51fa",(0,s.jsx)(n.code,{children:"leader"}),"\uff0c\u5e76\u5c06\u5176\u4ed6\u670d\u52a1\u5668\u4ee5",(0,s.jsx)(n.code,{children:"follower"}),"\u7684\u89d2\u8272\u52a0\u5165\u5907\u4efd\u7ec4\u3002"]}),"\n",(0,s.jsx)(n.p,{children:"\u542f\u52a8\u521d\u59cb\u5907\u4efd\u7ec4\u7684\u547d\u4ee4\u793a\u4f8b\u5982\u4e0b\u6240\u793a\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-bash",children:"$ ./lgraph_server -c lgraph.json --rpc_port 9090 --enable_ha true --ha_conf 172.22.224.15:9090,172.22.224.16:9090,172.22.224.17:9090\n"})}),"\n",(0,s.jsx)(n.h3,{id:"32\u521d\u59cb\u6570\u636e\u4e0d\u4e00\u81f4",children:"3.2.\u521d\u59cb\u6570\u636e\u4e0d\u4e00\u81f4"}),"\n",(0,s.jsxs)(n.p,{children:["\u5982\u679c\u7b2c\u4e00\u53f0\u670d\u52a1\u5668\u4e2d\u5df2\u6709\u6570\u636e\uff08\u4ee5",(0,s.jsx)(n.code,{children:"lgraph_import"}),"\u5de5\u5177\u5bfc\u5165\u6216\u4ece\u975e\u9ad8\u53ef\u7528\u6a21\u5f0f\u7684\u670d\u52a1\u5668\u4f20\u8f93\u5f97\u5230\uff09\uff0c\n\u5e76\u4e14\u4e4b\u524d\u5e76\u672a\u5728\u9ad8\u53ef\u7528\u6a21\u5f0f\u4e0b\u4f7f\u7528\uff0c\u5219\u7528\u6237\u5e94\u4f7f\u7528boostrap\u65b9\u5f0f\u542f\u52a8\u3002\n\u4ee5",(0,s.jsx)(n.code,{children:"ha_bootstrap_role"}),"\u53c2\u6570\u4e3a1\u5728bootstrap\u6a21\u5f0f\u4e0b\u542f\u52a8\u6709\u6570\u636e\u7684\u670d\u52a1\u5668\uff0c\u5e76\u901a\u8fc7",(0,s.jsx)(n.code,{children:"ha_conf"}),"\u53c2\u6570\u6307\u5b9a\u672c\u673a\u4e3a",(0,s.jsx)(n.code,{children:"leader"}),"\u3002\n\u5728bootstrap\u6a21\u5f0f\u4e0b\uff0c\u670d\u52a1\u5668\u5728\u5c06\u65b0\u52a0\u5165\u7684\u670d\u52a1\u5668\u6dfb\u52a0\u5230\u5907\u4efd\u7ec4\u4e4b\u524d\u4f1a\u5c06\u81ea\u5df1\u7684\n\u6570\u636e\u590d\u5236\u5230\u65b0\u670d\u52a1\u5668\u4e2d\uff0c\u4ee5\u4f7f\u6bcf\u4e2a\u670d\u52a1\u5668\u4e2d\u7684\u6570\u636e\u4fdd\u6301\u4e00\u81f4\u3002"]}),"\n",(0,s.jsx)(n.p,{children:"\u542f\u52a8\u6709\u6570\u636e\u670d\u52a1\u5668\u7684\u547d\u4ee4\u793a\u4f8b\u5982\u4e0b\u6240\u793a\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-bash",children:"$ ./lgraph_server -c lgraph.json --rpc_port 9090 --enable_ha true --ha_conf 172.22.224.15:9090,172.22.224.16:9090,172.22.224.17:9090 --ha_bootstrap_role 1\n"})}),"\n",(0,s.jsxs)(n.p,{children:["\u5176\u4ed6\u65e0\u6570\u636e\u7684\u670d\u52a1\u5668\u9700\u8981\u6307\u5b9a",(0,s.jsx)(n.code,{children:"ha_bootstrap_role"}),"\u53c2\u6570\u4e3a2\uff0c\u5e76\u901a\u8fc7",(0,s.jsx)(n.code,{children:"ha_conf"}),"\u53c2\u6570\u6307\u5b9a",(0,s.jsx)(n.code,{children:"leader"}),"\u5373\u53ef\uff0c\u547d\u4ee4\u793a\u4f8b\u5982\u4e0b\u6240\u793a"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-bash",children:"$ ./lgraph_server -c lgraph.json --rpc_port 9090 --enable_ha true --ha_conf 172.22.224.15:9090,172.22.224.16:9090,172.22.224.17:9090 --ha_bootstrap_role 2\n"})}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.strong,{children:"\u4f7f\u7528bootstrap\u542f\u52a8HA\u96c6\u7fa4\u65f6\u9700\u8981\u6ce8\u610f\u4e24\u70b9\uff1a"})}),"\n",(0,s.jsxs)(n.ol,{children:["\n",(0,s.jsxs)(n.li,{children:["\u9700\u8981\u7b49\u5f85",(0,s.jsx)(n.code,{children:"leader"}),"\u8282\u70b9\u751f\u6210snapshot\u5e76\u4e14\u6210\u529f\u542f\u52a8\u4e4b\u540e\u518d\u52a0\u5165",(0,s.jsx)(n.code,{children:"follower"}),"\u8282\u70b9\uff0c\u5426\u5219",(0,s.jsx)(n.code,{children:"follower"}),"\u8282\u70b9\u53ef\u80fd\u52a0\u5165\u5931\u8d25\u3002\u5728\u542f\u52a8",(0,s.jsx)(n.code,{children:"follower"}),"\u8282\u70b9\u65f6\u53ef\u4ee5\u5c06",(0,s.jsx)(n.code,{children:"ha_node_join_group_s"}),"\u53c2\u6570\u914d\u7f6e\u7684\u7a0d\u5927\uff0c\u4ee5\u5728\u52a0\u5165HA\u96c6\u7fa4\u65f6\u591a\u6b21\u7b49\u5f85\u548c\u8d85\u65f6\u91cd\u8bd5\u3002"]}),"\n",(0,s.jsx)(n.li,{children:"HA\u96c6\u7fa4\u53ea\u6709\u5728\u7b2c\u4e00\u6b21\u542f\u52a8\u65f6\u53ef\u4ee5\u4f7f\u7528bootstrap\u6a21\u5f0f\uff0c\u540e\u7eed\u518d\u542f\u52a8\u65f6\u53ea\u80fd\u4f7f\u7528\u666e\u901a\u6a21\u5f0f(\u89c13.1\u8282)\u542f\u52a8\uff0c\u5c24\u5176\u4e0d\u80fd\u8ba9\u540c\u4e00\u4e2a\u96c6\u7fa4\u7684\u591a\u4e2a\u8282\u70b9\u4ee5bootstrap\u6a21\u5f0f\u542f\u52a8\uff0c\u5426\u5219\u53ef\u80fd\u4ea7\u751f\u6570\u636e\u4e0d\u4e00\u81f4\u7684\u60c5\u51b5"}),"\n"]}),"\n",(0,s.jsx)(n.h2,{id:"4\u542f\u52a8witness\u8282\u70b9",children:"4.\u542f\u52a8witness\u8282\u70b9"}),"\n",(0,s.jsx)(n.h3,{id:"41\u4e0d\u5141\u8bb8witness\u8282\u70b9\u6210\u4e3aleader",children:"4.1.\u4e0d\u5141\u8bb8witness\u8282\u70b9\u6210\u4e3aleader"}),"\n",(0,s.jsxs)(n.p,{children:[(0,s.jsx)(n.code,{children:"witness"}),"\u8282\u70b9\u7684\u542f\u52a8\u65b9\u5f0f\u548c\u666e\u901a\u8282\u70b9\u7684\u542f\u52a8\u65b9\u5f0f\u4e00\u81f4\uff0c\u53ea\u9700\u8981\u8bbe\u7f6e",(0,s.jsx)(n.code,{children:"ha_is_witness"}),"\u53c2\u6570\u4e3a",(0,s.jsx)(n.code,{children:"true"}),"\u5373\u53ef\u3002\u9700\u6ce8\u610f\uff0cwitness\u8282\u70b9\u7684\u6570\u91cf\u5e94\u5c11\u4e8e\u96c6\u7fa4\u8282\u70b9\u603b\u6570\u91cf\u7684\u4e00\u534a\u3002"]}),"\n",(0,s.jsxs)(n.p,{children:["\u542f\u52a8",(0,s.jsx)(n.code,{children:"witness"}),"\u8282\u70b9\u670d\u52a1\u5668\u7684\u547d\u4ee4\u793a\u4f8b\u5982\u4e0b\u6240\u793a\uff1a"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-bash",children:"$ ./lgraph_server -c lgraph.json --rpc_port 9090 --enable_ha true --ha_conf 172.22.224.15:9090,172.22.224.16:9090,172.22.224.17:9090 --ha_is_witness 1\n"})}),"\n",(0,s.jsxs)(n.p,{children:["\u6ce8\uff1a\u9ed8\u8ba4\u4e0d\u5141\u8bb8",(0,s.jsx)(n.code,{children:"witness"}),"\u8282\u70b9\u6210\u4e3a",(0,s.jsx)(n.code,{children:"leader"}),"\u8282\u70b9\uff0c\u8fd9\u53ef\u4ee5\u63d0\u9ad8\u96c6\u7fa4\u7684\u6027\u80fd\uff0c\u4f46\u662f\u5728",(0,s.jsx)(n.code,{children:"leader"}),"\u8282\u70b9\u5d29\u6e83\u65f6\u4f1a\u964d\u4f4e\u96c6\u7fa4\u7684\u53ef\u7528\u6027\u3002"]}),"\n",(0,s.jsx)(n.h3,{id:"42\u5141\u8bb8witness\u8282\u70b9\u6210\u4e3aleader",children:"4.2.\u5141\u8bb8witness\u8282\u70b9\u6210\u4e3aleader"}),"\n",(0,s.jsxs)(n.p,{children:["\u53ef\u4ee5\u901a\u8fc7\u6307\u5b9a",(0,s.jsx)(n.code,{children:"ha_enable_witness_to_leader"}),"\u53c2\u6570\u4e3a",(0,s.jsx)(n.code,{children:"true"}),"\uff0c\u4f7f\u5f97",(0,s.jsx)(n.code,{children:"witness"}),"\u8282\u70b9\u53ef\u4ee5\u4e34\u65f6\u6210\u4e3a",(0,s.jsx)(n.code,{children:"leader"}),"\u8282\u70b9\uff0c\u5728\u5c06\u65b0\u65e5\u5fd7\u540c\u6b65\u5b8c\u6210\u4e4b\u540e\u518d\u4e3b\u52a8\u5207\u4e3b"]}),"\n",(0,s.jsxs)(n.p,{children:["\u542f\u52a8\u5141\u8bb8\u6210\u4e3a",(0,s.jsx)(n.code,{children:"leader"}),"\u8282\u70b9\u7684",(0,s.jsx)(n.code,{children:"witness"}),"\u8282\u70b9\u670d\u52a1\u5668\u7684\u547d\u4ee4\u793a\u4f8b\u5982\u4e0b\u6240\u793a\uff1a"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-bash",children:"$ ./lgraph_server -c lgraph.json --rpc_port 9090 --enable_ha true --ha_conf 172.22.224.15:9090,172.22.224.16:9090,172.22.224.17:9090 --ha_is_witness 1 --ha_enable_witness_to_leader 1\n"})}),"\n",(0,s.jsxs)(n.p,{children:["\u6ce8\uff1a\u5c3d\u7ba1\u5141\u8bb8",(0,s.jsx)(n.code,{children:"witness"}),"\u8282\u70b9\u6210\u4e3a",(0,s.jsx)(n.code,{children:"leader"}),"\u8282\u70b9\u53ef\u4ee5\u63d0\u9ad8\u96c6\u7fa4\u7684\u53ef\u7528\u6027\uff0c\u4f46\u662f\u5728\u6781\u7aef\u60c5\u51b5\u4e0b\u53ef\u80fd\u4f1a\u5f71\u54cd\u6570\u636e\u7684\u4e00\u81f4\u6027\u3002\u56e0\u6b64\u4e00\u822c\u5e94\u4fdd\u8bc1",(0,s.jsx)(n.code,{children:"witness"}),"\u8282\u70b9\u6570\u91cf+1\u5c11\u4e8e\u96c6\u7fa4\u8282\u70b9\u603b\u6570\u91cf\u7684\u4e00\u534a\u3002"]}),"\n",(0,s.jsx)(n.h2,{id:"5\u6a2a\u5411\u6269\u5c55\u5176\u4ed6\u670d\u52a1\u5668",children:"5.\u6a2a\u5411\u6269\u5c55\u5176\u4ed6\u670d\u52a1\u5668"}),"\n",(0,s.jsxs)(n.p,{children:["\u542f\u52a8\u521d\u59cb\u5907\u4efd\u7ec4\u540e\uff0c\u5982\u679c\u60f3\u5bf9\u5907\u4efd\u7ec4\u8fdb\u884c\u6a2a\u5411\u6269\u5c55\uff0c\u8981\u5c06\u65b0\u670d\u52a1\u5668\u6dfb\u52a0\u5230\u5907\u4efd\u7ec4\uff0c\n\u5e94\u4f7f\u7528",(0,s.jsx)(n.code,{children:"--ha_conf HOST\uff1aPORT"}),"\u9009\u9879\uff0c\u5176\u4e2d",(0,s.jsx)(n.code,{children:"HOST"}),"\u53ef\u4ee5\u662f\u8be5\u5907\u4efd\u7ec4\u4e2d\u5df2\u6709\u7684\u4efb\u4f55\u670d\u52a1\u5668\u7684 IP \u5730\u5740\uff0c\n\u800c",(0,s.jsx)(n.code,{children:"PORT"}),"\u662f\u5176 RPC \u7aef\u53e3\u3002\u4f8b\u5982\uff1a"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-bash",children:"./lgraph_server -c lgraph.json --rpc_port 9090 --enable_ha true --ha_conf 172.22.224.15:9090\n"})}),"\n",(0,s.jsxs)(n.p,{children:["\u6b64\u547d\u4ee4\u5c06\u542f\u52a8\u4e00\u53f0\u9ad8\u53ef\u7528\u6a21\u5f0f\u7684 TuGraph \u670d\u52a1\u5668\uff0c\u5e76\u5c1d\u8bd5\u5c06\u5176\u6dfb\u52a0\u5230\u5305\u542b\u670d\u52a1\u5668",(0,s.jsx)(n.code,{children:"172.22.224.15:9090"}),"\u7684\u5907\u4efd\u7ec4\u4e2d\u3002\n\u8bf7\u6ce8\u610f\uff0c\u52a0\u5165\u5907\u4efd\u7ec4\u9700\u8981\u670d\u52a1\u5668\u5c06\u5176\u6570\u636e\u4e0e\u5907\u4efd\u7ec4\u7684",(0,s.jsx)(n.code,{children:"leader"}),"\u670d\u52a1\u5668\u540c\u6b65\uff0c\u6b64\u8fc7\u7a0b\u53ef\u80fd\u9700\u8981\u76f8\u5f53\u957f\u7684\u65f6\u95f4\uff0c\u5177\u4f53\u53d6\u51b3\u4e8e\u6570\u636e\u7684\u5927\u5c0f\u3002"]}),"\n",(0,s.jsx)(n.h2,{id:"6\u505c\u6b62\u670d\u52a1\u5668",children:"6.\u505c\u6b62\u670d\u52a1\u5668"}),"\n",(0,s.jsxs)(n.p,{children:["\u5f53\u670d\u52a1\u5668\u901a\u8fc7",(0,s.jsx)(n.code,{children:"CTRL-C"}),"\u4e0b\u7ebf\u65f6\uff0c\u5b83\u5c06\u901a\u77e5\u5f53\u524d\u7684",(0,s.jsx)(n.code,{children:"leader"}),"\u670d\u52a1\u5668\uff0c\u544a\u77e5\u5176\u4ece\u5907\u4efd\u7ec4\u4e2d\u5220\u9664\u8be5\u4e0b\u7ebf\u7684\u670d\u52a1\u5668\u3002\u5982\u679c",(0,s.jsx)(n.code,{children:"leader"}),"\u670d\u52a1\u5668\u4e0b\u7ebf\uff0c\n\u5b83\u5c06\u5728\u4e0b\u7ebf\u524d\u5c06",(0,s.jsx)(n.code,{children:"leader"}),"\u8eab\u4efd\u6743\u9650\u4f20\u7ed9\u53e6\u4e00\u53f0\u670d\u52a1\u5668\u3002"]}),"\n",(0,s.jsxs)(n.p,{children:["\u5982\u679c\u670d\u52a1\u5668\u88ab\u7ec8\u6b62\u6216\u8005\u4e0e\u5907\u4efd\u7ec4\u4e2d\u7684\u5176\u4ed6\u670d\u52a1\u5668\u5931\u53bb\u8fde\u63a5\uff0c\u5219\u8be5\u670d\u52a1\u5668\u5c06\u88ab\u89c6\u4e3a\u5931\u8d25\u8282\u70b9\uff0c",(0,s.jsx)(n.code,{children:"leader"}),"\u670d\u52a1\u5668\u5c06\u5728\u7279\u5b9a\u65f6\u9650\u540e\u5c06\u5176\u4ece\u5907\u4efd\u7ec4\u4e2d\u5220\u9664\u3002"]}),"\n",(0,s.jsxs)(n.p,{children:["\u5982\u679c\u4efb\u4f55\u670d\u52a1\u5668\u79bb\u5f00\u5907\u4efd\u7ec4\u5e76\u5e0c\u671b\u91cd\u65b0\u52a0\u5165\uff0c\u5219\u5fc5\u987b\u4ece",(0,s.jsx)(n.code,{children:"--ha_conf HOST:PORT"}),"\u9009\u9879\u5f00\u59cb\uff0c\u5176\u4e2d",(0,s.jsx)(n.code,{children:"HOST"}),"\u662f\u5f53\u524d\u5907\u4efd\u7ec4\u4e2d\u7684\u67d0\u53f0\u670d\u52a1\u5668\u7684 IP \u5730\u5740\u3002"]}),"\n",(0,s.jsx)(n.h2,{id:"7\u91cd\u542f\u670d\u52a1\u5668",children:"7.\u91cd\u542f\u670d\u52a1\u5668"}),"\n",(0,s.jsxs)(n.p,{children:["\u4e0d\u5efa\u8bae\u91cd\u65b0\u542f\u52a8\u6574\u4e2a\u5907\u4efd\u7ec4\uff0c\u56e0\u4e3a\u5b83\u4f1a\u4e2d\u65ad\u670d\u52a1\u3002\u5982\u679c\u9700\u8981\uff0c\u53ef\u4ee5\u5173\u95ed\u6240\u6709\u670d\u52a1\u5668\u3002\u4f46\u5728\u91cd\u65b0\u542f\u52a8\u65f6\uff0c\n\u5fc5\u987b\u4fdd\u8bc1\u5173\u95ed\u65f6\u7684\u5907\u4efd\u7ec4\u4e2d\u81f3\u5c11\u6709N/2+1\u7684\u670d\u52a1\u5668\u80fd\u6b63\u5e38\u542f\u52a8\uff0c\u5426\u5219\u542f\u52a8\u5931\u8d25\u3002 \u5e76\u4e14\uff0c\n\u65e0\u8bba\u521d\u59cb\u542f\u52a8\u590d\u5236\u7ec4\u65f6\u662f\u5426\u6307\u5b9a",(0,s.jsx)(n.code,{children:"enable_bootstrap"}),"\u4e3atrue\uff0c\u91cd\u542f\u670d\u52a1\u5668\u65f6\u90fd\u53ea\u9700\u901a\u8fc7\n\u6307\u5b9a",(0,s.jsx)(n.code,{children:"--ha_conf host1:port1,host2:port2"}),"\u53c2\u6570\u4e00\u6b21\u6027\u91cd\u542f\u6240\u6709\u670d\u52a1\u5668\u5373\u53ef\uff0c\u547d\u4ee4\u793a\u4f8b\u5982\u4e0b\u6240\u793a\uff1a"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-bash",children:"$ ./lgraph_server -c lgraph.json --rpc_port 9090 --enable_ha true --ha_conf 172.22.224.15:9090,172.22.224.16:9090,172.22.224.17:9090\n"})}),"\n",(0,s.jsx)(n.h2,{id:"8docker\u90e8\u7f72\u9ad8\u53ef\u7528\u96c6\u7fa4",children:"8.docker\u90e8\u7f72\u9ad8\u53ef\u7528\u96c6\u7fa4"}),"\n",(0,s.jsx)(n.p,{children:"\u5728\u771f\u5b9e\u4e1a\u52a1\u573a\u666f\u4e2d\uff0c\u5f88\u53ef\u80fd\u9047\u5230\u5728\u591a\u79cd\u64cd\u4f5c\u7cfb\u7edf\u6216\u67b6\u6784\u4e0a\u90e8\u7f72\u9ad8\u53ef\u7528\u96c6\u7fa4\u7684\u9700\u6c42\u3002\n\u5dee\u5f02\u5316\u7684\u73af\u5883\u53ef\u80fd\u5bfc\u81f4\u7f16\u8bd1TuGraph\u65f6\u7f3a\u5c11\u67d0\u4e9b\u4f9d\u8d56\u3002\u56e0\u6b64\uff0c\n\u5728docker\u4e2d\u7f16\u8bd1\u8f6f\u4ef6\u5e76\u90e8\u7f72\u9ad8\u53ef\u7528\u96c6\u7fa4\u662f\u975e\u5e38\u6709\u5e94\u7528\u4ef7\u503c\u7684\u3002\u4ee5centos7\u7248\u672c\u7684docker\u4e3a\u4f8b\uff0c\n\u90e8\u7f72\u9ad8\u53ef\u7528\u96c6\u7fa4\u7684\u6b65\u9aa4\u5982\u4e0b\u6240\u793a\u3002"}),"\n",(0,s.jsx)(n.h3,{id:"81\u5b89\u88c5\u955c\u50cf",children:"8.1.\u5b89\u88c5\u955c\u50cf"}),"\n",(0,s.jsx)(n.p,{children:"\u4f7f\u7528\u5982\u4e0b\u547d\u4ee4\u4e0b\u8f7dTuGraph\u7684\u7f16\u8bd1docker\u955c\u50cf\u73af\u5883"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-shell",children:"docker pull tugraph/tugraph-compile-centos7\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u7136\u540e\u62c9\u53d6TuGraph\u6e90\u7801\u5e76\u7f16\u8bd1\u5b89\u88c5"}),"\n",(0,s.jsx)(n.h3,{id:"82\u521b\u5efa\u5bb9\u5668",children:"8.2.\u521b\u5efa\u5bb9\u5668"}),"\n",(0,s.jsxs)(n.p,{children:["\u4f7f\u7528\u5982\u4e0b\u547d\u4ee4\u521b\u5efa\u5bb9\u5668\uff0c\u4f7f\u7528",(0,s.jsx)(n.code,{children:"--net=host"}),"\u4f7f\u5f97\u5bb9\u5668\u8fd0\u884c\u5728host\u6a21\u5f0f\uff0c\u6b64\u6a21\u5f0f\u4e0b\ndocker\u548c\u5bbf\u4e3b\u673a\u548c\u5171\u4eab\u7f51\u7edcnamespace\uff0c\u5373\u5171\u7528\u540c\u4e00\u4e2aIP\u3002"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-shell",children:"docker run --net=host -itd -p -v {src_dir}:{dst_dir} --name tugraph_ha tugraph/tugraph-compile-centos7 /bin/bash\n"})}),"\n",(0,s.jsx)(n.h3,{id:"83\u542f\u52a8\u670d\u52a1",children:"8.3.\u542f\u52a8\u670d\u52a1"}),"\n",(0,s.jsx)(n.p,{children:"\u5728\u6bcf\u53f0\u670d\u52a1\u5668\u4e0a\u4f7f\u7528\u5982\u4e0b\u547d\u4ee4\u542f\u52a8\u670d\u52a1\uff0c\u56e0\u4e3adocker\u548c\u5bbf\u4e3b\u673a\u5171\u4eabIP\uff0c\u6240\u4ee5\u53ef\u4ee5\u76f4\u63a5\u6307\u5b9a\u5728\u5bbf\u4e3b\u673aIP\u4e0a\u542f\u52a8\u670d\u52a1"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-shell",children:"$ lgraph_server -c lgraph.json --host 172.22.224.15 --rpc_port 9090 --enable_ha true --ha_conf 172.22.224.15:9090,172.22.224.16:9090,172.22.224.17:9090\n"})}),"\n",(0,s.jsx)(n.h2,{id:"9\u67e5\u770b\u670d\u52a1\u5668\u72b6\u6001",children:"9.\u67e5\u770b\u670d\u52a1\u5668\u72b6\u6001"}),"\n",(0,s.jsx)(n.p,{children:"\u5907\u4efd\u7ec4\u7684\u5f53\u524d\u72b6\u6001\u53ef\u4ee5\u5728 TuGraph \u53ef\u89c6\u5316\u5de5\u5177\u3001REST API \u4ee5\u53ca Cypher \u67e5\u8be2\u4e2d\u83b7\u53d6\u3002"}),"\n",(0,s.jsx)(n.p,{children:"\u5728 TuGraph \u53ef\u89c6\u5316\u5de5\u5177\u4e2d\uff0c\u53ef\u4ee5\u5728 DBInfo \u90e8\u5206\u4e2d\u627e\u5230\u5907\u4efd\u7ec4\u4e2d\u7684\u670d\u52a1\u5668\u53ca\u5176\u89d2\u8272\u5217\u8868\u3002"}),"\n",(0,s.jsxs)(n.p,{children:["\u4f7f\u7528 REST API \u65f6\uff0c\u53ef\u4ee5\u4f7f\u7528",(0,s.jsx)(n.code,{children:"GET /info/peers"})," \u8bf7\u6c42\u83b7\u53d6\u4fe1\u606f\u3002"]}),"\n",(0,s.jsxs)(n.p,{children:["\u5728 Cypher \u4e2d\uff0c\u4f7f\u7528",(0,s.jsx)(n.code,{children:"CALL dbms.listServers()"}),"\u8bed\u53e5\u6765\u67e5\u8be2\u5f53\u524d\u5907\u4efd\u7ec4\u7684\u72b6\u6001\u4fe1\u606f\u3002"]}),"\n",(0,s.jsx)(n.h2,{id:"10\u9ad8\u53ef\u7528\u6a21\u5f0f\u4e0b\u6570\u636e\u540c\u6b65\u95ee\u9898",children:"10.\u9ad8\u53ef\u7528\u6a21\u5f0f\u4e0b\u6570\u636e\u540c\u6b65\u95ee\u9898"}),"\n",(0,s.jsxs)(n.p,{children:["\u5728\u9ad8\u53ef\u7528\u6a21\u5f0f\u4e0b\uff0c\u540c\u4e00\u5907\u4efd\u7ec4\u4e2d\u7684\u4e0d\u540c\u670d\u52a1\u5668\u53ef\u80fd\u5e76\u4e0d\u603b\u662f\u5904\u4e8e\u76f8\u540c\u7684\u72b6\u6001\u3002\u51fa\u4e8e\u6027\u80fd\u539f\u56e0\uff0c\u5982\u679c\u8bf7\u6c42\u5df2\u540c\u6b65\u5230\u8d85\u8fc7\u4e00\u534a\u7684\u670d\u52a1\u5668\uff0c\u5219",(0,s.jsx)(n.code,{children:"leader"}),"\u670d\u52a1\u5668\u5c06\u8ba4\u4e3a\u8be5\u8bf7\u6c42\u5c5e\u4e8e",(0,s.jsx)(n.code,{children:"committed"}),"\u72b6\u6001\u3002\u5c3d\u7ba1\u5176\u4f59\u670d\u52a1\u5668\u6700\u7ec8\u5c06\u6536\u5230\u65b0\u8bf7\u6c42\uff0c\u4f46\u670d\u52a1\u5668\u7684\u72b6\u6001\u4e0d\u4e00\u81f4\u5c06\u6301\u7eed\u4e00\u6bb5\u65f6\u95f4\u3002\u5ba2\u6237\u7aef\u4e5f\u53ef\u80fd\u5411\u521a\u521a\u91cd\u65b0\u542f\u52a8\u7684\u670d\u52a1\u5668\u53d1\u9001\u8bf7\u6c42\uff0c\u4ece\u800c\u5177\u6709\u8f83\u65e7\u7684\u72b6\u6001\u3002"]}),"\n",(0,s.jsxs)(n.p,{children:["\u4e3a\u4e86\u786e\u4fdd\u5ba2\u6237\u7aef\u770b\u5230\u4e00\u81f4\u8fde\u7eed\u7684\u6570\u636e\uff0c\u7279\u522b\u662f\u4e3a\u4e86\u6446\u8131",(0,s.jsx)(n.code,{children:"\u53cd\u5411\u65f6\u95f4\u65c5\u884c"}),"\u95ee\u9898\uff08\u5176\u4e2d\u5ba2\u6237\u7aef\u8bfb\u53d6\u6bd4\u4ee5\u524d\u770b\u5230\u7684\u72b6\u6001\u66f4\u65e7\u7684\u72b6\u6001\uff09\uff0c\u6bcf\u4e2a TuGraph \u670d\u52a1\u5668\u90fd\u4f1a\u4fdd\u6301\u4e00\u4e2a\u5355\u8c03\u589e\u52a0\u7684\u6570\u636e\u7248\u672c\u53f7\u3002\u5907\u4efd\u7ec4\u4e2d\u6570\u636e\u7248\u672c\u53f7\u5230\u6570\u636e\u5e93\u72b6\u6001\u7684\u6620\u5c04\u5168\u5c40\u4e00\u81f4\uff0c\u8fd9\u610f\u5473\u7740\u5982\u679c\u4e24\u53f0\u670d\u52a1\u5668\u5177\u6709\u76f8\u540c\u7684\u6570\u636e\u7248\u672c\u53f7\uff0c\u5219\u5b83\u4eec\u5fc5\u987b\u5177\u6709\u76f8\u540c\u7684\u6570\u636e\u3002\u54cd\u5e94\u8bf7\u6c42\u65f6\uff0c\u670d\u52a1\u5668\u5728\u54cd\u5e94\u4e2d\u5305\u542b\u4e86\u5176\u6570\u636e\u7248\u672c\u53f7\u3002\u56e0\u6b64\uff0c\u5ba2\u6237\u7aef\u53ef\u4ee5\u77e5\u9053\u5b83\u770b\u5230\u4e86\u54ea\u4e2a\u7248\u672c\u3002\u5ba2\u6237\u7aef\u6536\u5230\u65e7\u7248\u672c\u7684\u6570\u636e\u4e4b\u540e\u53ef\u4ee5\u91cd\u65b0\u5411Leader\u53d1\u9001\u8bf7\u6c42\uff0c\u4ece\u800c\u83b7\u53d6\u5230\u6700\u65b0\u7684\u6570\u636e\u3002"]})]})}function t(e={}){const{wrapper:n}={...(0,l.R)(),...e.components};return n?(0,s.jsx)(n,{...e,children:(0,s.jsx)(a,{...e})}):a(e)}}}]);