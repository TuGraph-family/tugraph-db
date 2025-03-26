"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[50499],{28453:(e,n,r)=>{r.d(n,{R:()=>d,x:()=>i});var s=r(96540);const l={},c=s.createContext(l);function d(e){const n=s.useContext(c);return s.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function i(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(l):e.components||l:d(e.components),s.createElement(c.Provider,{value:n},e.children)}},98671:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>o,contentTitle:()=>d,default:()=>h,frontMatter:()=>c,metadata:()=>i,toc:()=>t});var s=r(74848),l=r(28453);const c={},d="\u96c6\u7fa4\u7ba1\u7406",i={id:"utility-tools/ha-cluster-management",title:"\u96c6\u7fa4\u7ba1\u7406",description:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph HA \u96c6\u7fa4\u7684\u7ba1\u7406\u5de5\u5177\uff0c\u4e3b\u8981\u5305\u62ec\u5220\u9664\u8282\u70b9\u3001leader\u8f6c\u79fb\u548c\u751f\u6210snapshot\u529f\u80fd",source:"@site/versions/version-4.3.0/zh-CN/source/6.utility-tools/5.ha-cluster-management.md",sourceDirName:"6.utility-tools",slug:"/utility-tools/ha-cluster-management",permalink:"/tugraph-db/zh/4.3.0/utility-tools/ha-cluster-management",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:5,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"\u6570\u636e\u9884\u70ed",permalink:"/tugraph-db/zh/4.3.0/utility-tools/data-warmup"},next:{title:"\u547d\u4ee4\u884c\u5de5\u5177",permalink:"/tugraph-db/zh/4.3.0/utility-tools/tugraph-cli"}},o={},t=[{value:"1. \u7b80\u4ecb",id:"1-\u7b80\u4ecb",level:2},{value:"2. \u5220\u9664\u8282\u70b9",id:"2-\u5220\u9664\u8282\u70b9",level:2},{value:"3. leader \u8f6c\u79fb",id:"3-leader-\u8f6c\u79fb",level:2},{value:"4. \u751f\u6210snapshot",id:"4-\u751f\u6210snapshot",level:2}];function a(e){const n={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",header:"header",li:"li",p:"p",pre:"pre",ul:"ul",...(0,l.R)(),...e.components};return(0,s.jsxs)(s.Fragment,{children:[(0,s.jsx)(n.header,{children:(0,s.jsx)(n.h1,{id:"\u96c6\u7fa4\u7ba1\u7406",children:"\u96c6\u7fa4\u7ba1\u7406"})}),"\n",(0,s.jsxs)(n.blockquote,{children:["\n",(0,s.jsx)(n.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd TuGraph HA \u96c6\u7fa4\u7684\u7ba1\u7406\u5de5\u5177\uff0c\u4e3b\u8981\u5305\u62ec\u5220\u9664\u8282\u70b9\u3001leader\u8f6c\u79fb\u548c\u751f\u6210snapshot\u529f\u80fd"}),"\n"]}),"\n",(0,s.jsx)(n.h2,{id:"1-\u7b80\u4ecb",children:"1. \u7b80\u4ecb"}),"\n",(0,s.jsxs)(n.p,{children:["HA\u96c6\u7fa4\u542f\u52a8\u4e4b\u540e\uff0c\u53ef\u4ee5\u4f7f\u7528",(0,s.jsx)(n.code,{children:"lgraph_peer"}),"\u5de5\u5177\u8fdb\u884c\u96c6\u7fa4\u7ba1\u7406\uff0c\u53ef\u4ee5\u6267\u884c\u5220\u9664\u8282\u70b9\uff0c\u8f6c\u79fbleader\u548c\u751f\u6210snapshot\u7b49\u529f\u80fd\u3002"]}),"\n",(0,s.jsx)(n.h2,{id:"2-\u5220\u9664\u8282\u70b9",children:"2. \u5220\u9664\u8282\u70b9"}),"\n",(0,s.jsxs)(n.p,{children:["\u5bf9\u4e8eTuGraph HA\u96c6\u7fa4\u4e2d\u957f\u671f\u79bb\u7ebf\u6216\u8005\u4ea7\u751f\u7f51\u7edc\u5206\u533a\u7684\u8282\u70b9\uff0c\u53ef\u4ee5\u4f7f\u7528",(0,s.jsx)(n.code,{children:"lgraph_peer"}),"\u7684",(0,s.jsx)(n.code,{children:"remove_peer"}),"\u547d\u4ee4\u5220\u9664\u8282\u70b9\u3002\u547d\u4ee4\u793a\u4f8b\u5982\u4e0b\u6240\u793a\uff1a"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-shell",children:"$ lgraph_peer --command remove_peer --peer {peer_id} --conf {group_conf}\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u5176\u4e2d\uff1a"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"--command remove_peer"})," \u6307\u5b9a\u8981\u6267\u884c\u7684\u64cd\u4f5c\u4e3aremove_peer\uff0c\u5373\u5220\u9664\u8282\u70b9\u3002"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"--peer {peer_id}"})," \u6307\u5b9a\u8981\u5220\u9664\u8282\u70b9\u7684rpc\u7f51\u7edc\u5730\u5740\uff0c\u5982 ",(0,s.jsx)(n.code,{children:"127.0.0.1:9092"}),"\u3002"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"--conf {group_conf}"})," \u6307\u5b9aHA\u96c6\u7fa4\u7684\u6210\u5458\u914d\u7f6e\uff08\u53ef\u8fde\u901a\u4e3b\u8282\u70b9\u5373\u53ef\uff09\uff0c\u5982 ",(0,s.jsx)(n.code,{children:"127.0.0.1:9092,127.0.0.1:9093,127.0.0.1:9094"})," \u3002"]}),"\n"]}),"\n",(0,s.jsx)(n.h2,{id:"3-leader-\u8f6c\u79fb",children:"3. leader \u8f6c\u79fb"}),"\n",(0,s.jsxs)(n.p,{children:["\u5f53\u9700\u8981\u5bf9\u4e3b\u8282\u70b9\u6267\u884c\u505c\u673a\u6216\u91cd\u542f\u64cd\u4f5c\u65f6\uff0c\u4e3a\u51cf\u5c11\u96c6\u7fa4\u7684\u4e0d\u53ef\u670d\u52a1\u65f6\u95f4\uff0c\u53ef\u4ee5\u4f7f\u7528",(0,s.jsx)(n.code,{children:"lgraph_peer"}),"\u7684",(0,s.jsx)(n.code,{children:"transfer_leader"}),"\u547d\u4ee4\u8f6c\u79fb\u4e3b\u8282\u70b9\u3002\u547d\u4ee4\u793a\u4f8b\u5982\u4e0b\u6240\u793a\uff1a"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-shell",children:"$ lgraph_peer --command transfer_leader --peer {peer_id} --conf {group_conf}\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u5176\u4e2d\uff1a"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"--command transfer_leader"})," \u6307\u5b9a\u8981\u6267\u884c\u7684\u64cd\u4f5c\u4e3atransfer_leader\uff0c\u5373\u8f6c\u79fb\u4e3b\u8282\u70b9\u3002"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"--peer {peer_id}"})," \u6307\u5b9a\u8981\u6210\u4e3a\u4e3b\u8282\u70b9\u7684rpc\u7f51\u7edc\u5730\u5740\uff0c\u5982 ",(0,s.jsx)(n.code,{children:"127.0.0.1:9092"}),"\u3002"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"--conf {group_conf}"})," \u6307\u5b9aHA\u96c6\u7fa4\u7684\u6210\u5458\u914d\u7f6e\uff08\u53ef\u8fde\u901a\u4e3b\u8282\u70b9\u5373\u53ef\uff09\uff0c\u5982 ",(0,s.jsx)(n.code,{children:"127.0.0.1:9092,127.0.0.1:9093,127.0.0.1:9094"})," \u3002"]}),"\n"]}),"\n",(0,s.jsx)(n.h2,{id:"4-\u751f\u6210snapshot",children:"4. \u751f\u6210snapshot"}),"\n",(0,s.jsxs)(n.p,{children:["\u51fa\u4e8e\u8282\u70b9\u542f\u52a8\u65f6\u8bbe\u7f6eha_snapshot_interval_s\u4e3a-1\u4ee5\u9ed8\u8ba4\u4e0d\u6253snapshot\u6216\u5176\u4ed6\u539f\u56e0\uff0c\n\u5f53\u9700\u8981\u8ba9\u67d0\u4e2a\u8282\u70b9\u624b\u52a8\u751f\u6210snapshot\u65f6\uff0c\u53ef\u4ee5\u4f7f\u7528",(0,s.jsx)(n.code,{children:"lgraph_peer"}),"\u7684",(0,s.jsx)(n.code,{children:"snapshot"}),"\u547d\u4ee4\u3002\u547d\u4ee4\u793a\u4f8b\u5982\u4e0b\u6240\u793a\uff1a"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-shell",children:"$ lgraph_peer --command snapshot --peer {peer_id}\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u5176\u4e2d\uff1a"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"--command snapshot"})," \u6307\u5b9a\u8981\u6267\u884c\u7684\u64cd\u4f5c\u4e3asnapshot\uff0c\u5373\u751f\u6210\u5feb\u7167\u3002"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"--peer {peer_id}"})," \u6307\u5b9a\u8981\u751f\u6210\u5feb\u7167\u7684\u8282\u70b9\u7684rpc\u7f51\u7edc\u5730\u5740\uff0c\u5982 ",(0,s.jsx)(n.code,{children:"127.0.0.1:9092"}),"\u3002"]}),"\n"]})]})}function h(e={}){const{wrapper:n}={...(0,l.R)(),...e.components};return n?(0,s.jsx)(n,{...e,children:(0,s.jsx)(a,{...e})}):a(e)}}}]);