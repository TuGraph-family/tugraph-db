"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[26301],{28453:(e,n,s)=>{s.d(n,{R:()=>o,x:()=>a});var r=s(96540);const i={},t=r.createContext(i);function o(e){const n=r.useContext(t);return r.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function a(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(i):e.components||i:o(e.components),r.createElement(t.Provider,{value:n},e.children)}},89538:(e,n,s)=>{s.r(n),s.d(n,{assets:()=>d,contentTitle:()=>o,default:()=>l,frontMatter:()=>t,metadata:()=>a,toc:()=>c});var r=s(74848),i=s(28453);const t={},o="\u5fd8\u8bb0'admin'\u5bc6\u7801",a={id:"permission/reset_admin_password",title:"\u5fd8\u8bb0'admin'\u5bc6\u7801",description:"TuGraph \u63d0\u4f9b\u4e86\u91cd\u7f6e\u5bc6\u7801\u7684\u529f\u80fd\uff0c\u5f53\u7528\u6237\u5fd8\u8bb0\u7ba1\u7406\u8005\u8d26\u53f7admin\u5bc6\u7801\u65f6\uff0c\u53ef\u4ee5\u901a\u8fc7\u91cd\u7f6e\u5bc6\u7801\u7684\u65b9\u5f0f\u6765\u4fee\u6539\u5bc6\u7801\u3002",source:"@site/versions/version-4.3.2/zh-CN/source/10.permission/3.reset_admin_password.md",sourceDirName:"10.permission",slug:"/permission/reset_admin_password",permalink:"/tugraph-db/en-US/zh/4.3.2/permission/reset_admin_password",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:3,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Token\u4f7f\u7528\u8bf4\u660e",permalink:"/tugraph-db/en-US/zh/4.3.2/permission/token"},next:{title:"\u8fd0\u7ef4\u76d1\u63a7",permalink:"/tugraph-db/en-US/zh/4.3.2/permission/monitoring"}},d={},c=[{value:"1.\u91cd\u7f6e\u5bc6\u7801",id:"1\u91cd\u7f6e\u5bc6\u7801",level:2}];function p(e){const n={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",header:"header",p:"p",pre:"pre",...(0,i.R)(),...e.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(n.header,{children:(0,r.jsx)(n.h1,{id:"\u5fd8\u8bb0admin\u5bc6\u7801",children:"\u5fd8\u8bb0'admin'\u5bc6\u7801"})}),"\n",(0,r.jsxs)(n.blockquote,{children:["\n",(0,r.jsxs)(n.p,{children:["TuGraph \u63d0\u4f9b\u4e86\u91cd\u7f6e\u5bc6\u7801\u7684\u529f\u80fd\uff0c\u5f53\u7528\u6237\u5fd8\u8bb0\u7ba1\u7406\u8005\u8d26\u53f7",(0,r.jsx)(n.code,{children:"admin"}),"\u5bc6\u7801\u65f6\uff0c\u53ef\u4ee5\u901a\u8fc7\u91cd\u7f6e\u5bc6\u7801\u7684\u65b9\u5f0f\u6765\u4fee\u6539\u5bc6\u7801\u3002"]}),"\n"]}),"\n",(0,r.jsx)(n.h2,{id:"1\u91cd\u7f6e\u5bc6\u7801",children:"1.\u91cd\u7f6e\u5bc6\u7801"}),"\n",(0,r.jsx)(n.p,{children:"\u9996\u5148\uff0c\u9700\u8981\u505c\u6b62TuGraph\u670d\u52a1\u7aef\u3002\u518d\u6b21\u542f\u52a8TuGraph\u670d\u52a1\u7aef\u65f6\uff0c\u9700\u8981\u6dfb\u52a0\u5982\u4e0b\u53c2\u6570\uff1a"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-bash",children:"--reset_admin_password 1\n"})}),"\n",(0,r.jsx)(n.p,{children:"\u5982\u4e0b\u6240\u793a\uff1a"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-bash",children:'./lgraph_server -c lgraph_standalone.json --reset_admin_password 1 --log_dir ""\n'})}),"\n",(0,r.jsxs)(n.p,{children:["\u8fd9\u4e00\u64cd\u4f5c\u53ef\u4ee5\u4f7f\u5f97TuGraph\u670d\u52a1\u7aef\u5728\u542f\u52a8\u65f6\uff0c\u91cd\u7f6e\u7ba1\u7406\u8005",(0,r.jsx)(n.code,{children:"admin"}),"\u7684\u5bc6\u7801\u4e3a\u9ed8\u8ba4\u5bc6\u7801\uff1a",(0,r.jsx)(n.code,{children:"73@TuGraph"}),"\u3002\n\u5bc6\u7801\u91cd\u7f6e\u6210\u529f\u4f1a\u7ed9\u51fa\u76f8\u5173\u4fe1\u606f\u201cReset admin password successfully\u201d\u5e76\u5173\u95ed\u5f53\u524d\u670d\u52a1\u7aef\u8fdb\u7a0b\u3002"]}),"\n",(0,r.jsx)(n.p,{children:"\u7528\u6237\u9700\u8981\u4ee5\u6b63\u5e38\u6a21\u5f0f\u91cd\u65b0\u542f\u52a8\u670d\u52a1\u7aef\uff0c\u7136\u540e\u4f7f\u7528\u9ed8\u8ba4\u8d26\u53f7\u5bc6\u7801\u8fdb\u884c\u767b\u5f55\uff0c\u767b\u5f55\u540e\u91cd\u65b0\u8bbe\u7f6e\u5bc6\u7801\u5373\u53ef\u6b63\u5e38\u4f7f\u7528\u3002"})]})}function l(e={}){const{wrapper:n}={...(0,i.R)(),...e.components};return n?(0,r.jsx)(n,{...e,children:(0,r.jsx)(p,{...e})}):p(e)}}}]);