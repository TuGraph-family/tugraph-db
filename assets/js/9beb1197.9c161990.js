"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[92629],{28453:(e,n,i)=>{i.d(n,{R:()=>t,x:()=>l});var o=i(96540);const r={},s=o.createContext(r);function t(e){const n=o.useContext(s);return o.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function l(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(r):e.components||r:t(e.components),o.createElement(s.Provider,{value:n},e.children)}},54293:(e,n,i)=>{i.r(n),i.d(n,{assets:()=>d,contentTitle:()=>t,default:()=>a,frontMatter:()=>s,metadata:()=>l,toc:()=>c});var o=i(74848),r=i(28453);const s={},t="Token\u4f7f\u7528\u8bf4\u660e",l={id:"permission/token",title:"Token\u4f7f\u7528\u8bf4\u660e",description:"1.Token\u4ecb\u7ecd",source:"@site/versions/version-4.3.2/zh-CN/source/10.permission/2.token.md",sourceDirName:"10.permission",slug:"/permission/token",permalink:"/tugraph-db/zh/4.3.2/permission/token",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:2,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"\u7528\u6237\u6743\u9650",permalink:"/tugraph-db/zh/4.3.2/permission/privilege"},next:{title:"\u5fd8\u8bb0'admin'\u5bc6\u7801",permalink:"/tugraph-db/zh/4.3.2/permission/reset_admin_password"}},d={},c=[{value:"1.Token\u4ecb\u7ecd",id:"1token\u4ecb\u7ecd",level:2},{value:"2. Token\u6709\u6548\u671f",id:"2-token\u6709\u6548\u671f",level:2},{value:"2.1. \u6d4f\u89c8\u5668Token\u4ea4\u4e92\u903b\u8f91",id:"21-\u6d4f\u89c8\u5668token\u4ea4\u4e92\u903b\u8f91",level:3},{value:"2.3. Token\u6709\u6548\u671f\u5237\u65b0\u673a\u5236",id:"23-token\u6709\u6548\u671f\u5237\u65b0\u673a\u5236",level:3},{value:"2.4. Token\u6709\u6548\u671f\u4fee\u6539",id:"24-token\u6709\u6548\u671f\u4fee\u6539",level:3},{value:"3. \u5ba2\u6237\u7aef\u53d1\u9001Token\u76f8\u5173\u8bf7\u6c42\u4ecb\u7ecd",id:"3-\u5ba2\u6237\u7aef\u53d1\u9001token\u76f8\u5173\u8bf7\u6c42\u4ecb\u7ecd",level:2},{value:"3.1. REST",id:"31-rest",level:3},{value:"3.2. RPC",id:"32-rpc",level:3},{value:"4. Token\u4e0a\u9650",id:"4-token\u4e0a\u9650",level:2}];function h(e){const n={a:"a",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",li:"li",ol:"ol",p:"p",ul:"ul",...(0,r.R)(),...e.components};return(0,o.jsxs)(o.Fragment,{children:[(0,o.jsx)(n.header,{children:(0,o.jsx)(n.h1,{id:"token\u4f7f\u7528\u8bf4\u660e",children:"Token\u4f7f\u7528\u8bf4\u660e"})}),"\n",(0,o.jsx)(n.h2,{id:"1token\u4ecb\u7ecd",children:"1.Token\u4ecb\u7ecd"}),"\n",(0,o.jsx)(n.p,{children:"JWT\uff08JSON Web Token\uff09\u662f\u4e00\u79cd\u7528\u4e8e\u8ba4\u8bc1\u548c\u6388\u6743\u7684\u5f00\u653e\u6807\u51c6\u3002\u5b83\u57fa\u4e8eJSON\uff08JavaScript Object Notation\uff09\u683c\u5f0f\uff0c\u5e76\u88ab\u8bbe\u8ba1\u7528\u4e8e\u5728\u7f51\u7edc\u5e94\u7528\u4e4b\u95f4\u5b89\u5168\u5730\u4f20\u8f93\u58f0\u660e\uff08claims\uff09\u4fe1\u606f\u3002"}),"\n",(0,o.jsx)(n.p,{children:"JWT\u7531\u4e09\u90e8\u5206\u7ec4\u6210\uff1a\u5934\u90e8\uff08Header\uff09\u3001\u8d1f\u8f7d\uff08Payload\uff09\u548c\u7b7e\u540d\uff08Signature\uff09\u3002\u5934\u90e8\u5305\u542b\u4e86JWT\u7684\u7c7b\u578b\u4ee5\u53ca\u6240\u4f7f\u7528\u7684\u7b7e\u540d\u7b97\u6cd5\uff0c\u8d1f\u8f7d\u5305\u542b\u4e86\u9700\u8981\u4f20\u8f93\u7684\u4fe1\u606f\uff0c\u800c\u7b7e\u540d\u7528\u4e8e\u9a8c\u8bc1JWT\u7684\u5b8c\u6574\u6027\u548c\u771f\u5b9e\u6027\u3002"}),"\n",(0,o.jsx)(n.p,{children:"\u5728TuGraph\u4e2d\uff0cJWT\u7528\u4e8e\u5b9e\u73b0\u65e0\u72b6\u6001\u7684\u8ba4\u8bc1\u548c\u6388\u6743\u673a\u5236\u3002\u5f53\u7528\u6237\u767b\u5f55\u6210\u529f\u540e\uff0c\u670d\u52a1\u7aef\u4f1a\u751f\u6210\u4e00\u4e2aJWT\u5e76\u5c06\u5176\u8fd4\u56de\u7ed9\u5ba2\u6237\u7aef\u3002\u5ba2\u6237\u7aef\u5728\u540e\u7eed\u7684\u8bf7\u6c42\u4e2d\u5c06\u8fd9\u4e2aJWT\u4f5c\u4e3a\u8eab\u4efd\u51ed\u8bc1\u4f20\u9012\u7ed9\u670d\u52a1\u7aef\u3002\u670d\u52a1\u7aef\u6536\u5230JWT\u540e\uff0c\u901a\u8fc7\u9a8c\u8bc1\u7b7e\u540d\u548c\u89e3\u6790\u8d1f\u8f7d\u4e2d\u7684\u4fe1\u606f\uff0c\u5224\u65ad\u7528\u6237\u7684\u8eab\u4efd\u548c\u6743\u9650\uff0c\u5e76\u51b3\u5b9a\u662f\u5426\u5141\u8bb8\u8be5\u8bf7\u6c42\u7684\u6267\u884c\u3002"}),"\n",(0,o.jsx)(n.h2,{id:"2-token\u6709\u6548\u671f",children:"2. Token\u6709\u6548\u671f"}),"\n",(0,o.jsx)(n.h3,{id:"21-\u6d4f\u89c8\u5668token\u4ea4\u4e92\u903b\u8f91",children:"2.1. \u6d4f\u89c8\u5668Token\u4ea4\u4e92\u903b\u8f91"}),"\n",(0,o.jsxs)(n.ol,{children:["\n",(0,o.jsx)(n.li,{children:"\u7528\u6237\u6253\u5f00\u6d4f\u89c8\u5668\uff0c\u8f93\u5165\u8d26\u53f7\u5bc6\u7801\uff0c\u5e76\u70b9\u51fb\u767b\u5f55\u3002"}),"\n",(0,o.jsx)(n.li,{children:"\u524d\u7aef\u8c03\u7528\u767b\u5f55\u63a5\u53e3\uff0c\u5411\u540e\u7aef\u8f93\u5165\u8d26\u53f7\u548c\u5bc6\u7801\u3002"}),"\n",(0,o.jsx)(n.li,{children:"\u540e\u7aef\u63a5\u6536\u5230\u8d26\u53f7\u5bc6\u7801\u540e\uff0c\u8fdb\u884c\u6821\u9a8c\uff0c\u6821\u9a8c\u6210\u529f\u540e\uff0c\u8fd4\u56deToken\u3002"}),"\n",(0,o.jsx)(n.li,{children:"\u524d\u7aef\u5c06Token\u5b58\u50a8\u5728\u6d4f\u89c8\u5668\u7f13\u5b58\u4e2d\uff0c\u540e\u7eed\u8bf7\u6c42\u90fd\u8981\u643a\u5e26\u6b64Token\u3002"}),"\n",(0,o.jsx)(n.li,{children:"\u5982\u679c\u524d\u7aef\uff0c\u4e3b\u52a8\u70b9\u51fb\u767b\u51fa\u548c\u5173\u95ed\u9875\u9762\uff0c\u524d\u7aef\u5e94\u4e3b\u52a8\u8c03\u7528\u767b\u51fa\u63a5\u53e3\uff0c\u5411\u540e\u7aef\u4f20\u9012Token\u3002"}),"\n",(0,o.jsx)(n.li,{children:"\u540e\u7aef\u63a5\u6536\u540e\uff0c\u5c06Token\u5931\u6548\uff0c\u8fd4\u56de\u72b6\u6001\u7801200\uff0c\u4ee5\u53ca\u201c\u7b49\u51fa\u6210\u529f\u201d\u3002\u524d\u7aef\u63a5\u6536\u5230\u4fe1\u606f\u540e\uff0c\u6e05\u7a7a\u6d4f\u89c8\u5668\u5185\u5b58\u4e2d\u7684Token\uff0c\u5e76\u5c06\u9875\u9762\u9000\u81f3\u767b\u5f55\u9875\u3002"}),"\n",(0,o.jsx)(n.li,{children:"Token\u5931\u6548\uff08\u521d\u59cb\u8bbe\u7f6e\u4e3a24\u5c0f\u65f6\uff09\uff0c\u9700\u8981\u7528\u6237\u91cd\u65b0\u767b\u5f55\u3002"}),"\n"]}),"\n",(0,o.jsx)(n.h3,{id:"23-token\u6709\u6548\u671f\u5237\u65b0\u673a\u5236",children:"2.3. Token\u6709\u6548\u671f\u5237\u65b0\u673a\u5236"}),"\n",(0,o.jsx)(n.p,{children:"Token\u6709\u6548\u671f\u5b58\u5728\u5237\u65b0\u673a\u5236\uff0c\u9ed8\u8ba4\u5173\u95ed\u3002\u5982\u679c\u6253\u5f00\u540e\uff0cToken\u7684\u5b89\u5168\u6027\u4f1a\u66f4\u9ad8\uff0c\u5b9e\u73b0\u4e0a\u5219\u5b58\u5728\u4e24\u4e2a\u65f6\u95f4\u6233\u3002"}),"\n",(0,o.jsxs)(n.p,{children:["\u7b2c\u4e00\u4e2a\u65f6\u95f4\u6233",(0,o.jsx)(n.code,{children:"refresh_time"}),"\u7528\u4e8e\u5224\u5b9aToken\u662f\u5426\u8fc7\u671f\uff08\u9ed8\u8ba424\u5c0f\u65f6\uff09\uff1a\u8fc7\u671f\u540e\u53ef\u4ee5\u8c03\u7528\u5237\u65b0\u63a5\u53e3\u83b7\u53d6\u65b0\u7684Token\uff0c\u53ef\u4ee5\u8bbe\u7f6e\u4e3a\u66f4\u77ed\u7684\u65f6\u95f4\uff0c\u6bd4\u59821\u5c0f\u65f6\u3002"]}),"\n",(0,o.jsxs)(n.p,{children:["\u7b2c\u4e8c\u4e2a\u65f6\u95f4\u6233",(0,o.jsx)(n.code,{children:"expire_time"}),"\u4e3a\u5f3a\u5236\u8fc7\u671f\u65f6\u95f4\u6233\uff08\u9ed8\u8ba424\u5c0f\u65f6\uff09\uff1a\u8fc7\u671f\u540e\u5fc5\u987b\u91cd\u65b0\u767b\u9646\u3002"]}),"\n",(0,o.jsx)(n.h3,{id:"24-token\u6709\u6548\u671f\u4fee\u6539",children:"2.4. Token\u6709\u6548\u671f\u4fee\u6539"}),"\n",(0,o.jsx)(n.p,{children:"\u4e3a\u4e86\u65b9\u4fbf\u5f00\u53d1\u8005\u81ea\u884c\u5f00\u53d1\uff0cTuGraph\u63d0\u4f9b\u4e86\u4e24\u79cd\u65b9\u5f0f\u4fee\u6539\u6709\u6548\u671f\uff0c\u5747\u9700\u8981admin\u6743\u9650\u3002"}),"\n",(0,o.jsxs)(n.ul,{children:["\n",(0,o.jsxs)(n.li,{children:["\n",(0,o.jsxs)(n.p,{children:["\u901a\u8fc7\u63a5\u53e3\u8c03\u7528\u8bbe\u7f6e\u3002\u6d89\u53ca\u6709\u6548\u671f\u4fee\u6539\u7684\u63a5\u53e3",(0,o.jsx)(n.code,{children:"update_token_time"}),"\u548c\u6709\u6548\u671f\u67e5\u8be2\u63a5\u53e3",(0,o.jsx)(n.code,{children:"get_token_time"}),"\u3002\n\u5177\u4f53\u53ef\u67e5\u8be2",(0,o.jsx)(n.a,{href:"/tugraph-db/zh/4.3.2/client-tools/restful-api-legacy",children:"REST\u63a5\u53e3\u6587\u6863"}),"\u3002"]}),"\n"]}),"\n",(0,o.jsxs)(n.li,{children:["\n",(0,o.jsxs)(n.p,{children:["\u901a\u8fc7\u542f\u52a8\u53c2\u6570\u8bbe\u7f6e\u3002server\u7aef\u542f\u52a8\u65f6\uff0c\u6dfb\u52a0\u53c2\u6570",(0,o.jsx)(n.code,{children:"-unlimited_token 1"})," \u53c2\u6570\u53ef\u4ee5\u8bbe\u7f6e\u4e3a\u65e0\u671f\u9650\u3002\u53ef\u53c2\u8003",(0,o.jsx)(n.a,{href:"/tugraph-db/zh/4.3.2/installation&running/tugraph-running",children:"\u670d\u52a1\u8fd0\u884c\u6587\u6863"}),"\u3002"]}),"\n"]}),"\n"]}),"\n",(0,o.jsx)(n.h2,{id:"3-\u5ba2\u6237\u7aef\u53d1\u9001token\u76f8\u5173\u8bf7\u6c42\u4ecb\u7ecd",children:"3. \u5ba2\u6237\u7aef\u53d1\u9001Token\u76f8\u5173\u8bf7\u6c42\u4ecb\u7ecd"}),"\n",(0,o.jsx)(n.p,{children:"\u5ba2\u6237\u7aef\u4f1a\u4e24\u79cd\u534f\u8bae\u5904\u7406\u76f8\u5173\u8bf7\u6c42\uff0c\u4e00\u79cd\u662fREST\uff0c\u4e00\u79cd\u662fRPC\u3002"}),"\n",(0,o.jsx)(n.h3,{id:"31-rest",children:"3.1. REST"}),"\n",(0,o.jsx)(n.p,{children:"\u5982\u679c\u5ba2\u6237\u7aef\u4f7f\u7528REST\u534f\u8bae\uff08\u5305\u62ecBrowser\u6d4f\u89c8\u5668\uff09\uff0c\u7531\u4e8e\u662f\u77ed\u94fe\u63a5\uff0c\u5728\u6bcf\u4e00\u6b21\u8bf7\u6c42\u4e2d\u90fd\u9700\u8981\u643a\u5e26Token\uff0c\u8fc7\u671f\u540e\u9700\u8981\u83b7\u53d6\u65b0\u7684Token\u3002"}),"\n",(0,o.jsx)(n.p,{children:"\u5bf9\u4e8e\u81ea\u884c\u5f00\u53d1\u7684\u5ba2\u6237\u7aef\uff0c\u82e5\u91c7\u7528REST\u534f\u8bae\u5219\u9700\u8981\u8003\u8651Token\u7684\u903b\u8f91\u3002"}),"\n",(0,o.jsx)(n.h3,{id:"32-rpc",children:"3.2. RPC"}),"\n",(0,o.jsx)(n.p,{children:"\u5982\u679c\u5ba2\u6237\u7aef\u4f7f\u7528RPC\u534f\u8bae\uff08\u5305\u62ec\u5b98\u65b9\u7684C++/Java/Python\uff09\uff0c\u4f7f\u7528\u957f\u8fde\u63a5\u4fdd\u6301\uff0c\u767b\u9646\u540e\u4e0d\u6d89\u53caToken\u64cd\u4f5c\u3002"}),"\n",(0,o.jsx)(n.h2,{id:"4-token\u4e0a\u9650",children:"4. Token\u4e0a\u9650"}),"\n",(0,o.jsx)(n.p,{children:"Token\u4e0a\u9650\u662f\u6307\u4e00\u4e2a\u7528\u6237\u6700\u591a\u53ef\u4ee5\u540c\u65f6\u62e5\u6709\u7684Token\u6570\u91cf\u3002\u4e3a\u9632\u6b62\u65e0\u9650\u767b\u9646\uff0cToken\u4e0a\u9650\u9ed8\u8ba4\u4e3a10000\u3002\u7531\u4e8eToken\u751f\u6210\u903b\u8f91\u548c\u65f6\u95f4\u5f3a\u76f8\u5173\uff0c\u6bcf\u767b\u5f55\u4e00\u6b21\u4f1a\u751f\u6210\u4e00\u4e2aToken\u8fdb\u884c\u5b58\u50a8\uff0cToken\u6709\u6548\u671f\u9ed8\u8ba4\u4e3a24\u5c0f\u65f6\uff0c\u56e0\u6b64\u5efa\u8bae\u767b\u5f55\u540e\u4e0d\u4f7f\u7528\u7684Token\u53ca\u65f6\u767b\u51fa\u3002"})]})}function a(e={}){const{wrapper:n}={...(0,r.R)(),...e.components};return n?(0,o.jsx)(n,{...e,children:(0,o.jsx)(h,{...e})}):h(e)}}}]);