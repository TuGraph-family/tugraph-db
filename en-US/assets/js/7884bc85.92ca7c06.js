"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[72394],{28453:(n,d,e)=>{e.d(d,{R:()=>t,x:()=>l});var r=e(96540);const s={},i=r.createContext(s);function t(n){const d=r.useContext(i);return r.useMemo((function(){return"function"==typeof n?n(d):{...d,...n}}),[d,n])}function l(n){let d;return d=n.disableParentContext?"function"==typeof n.components?n.components(s):n.components||s:t(n.components),r.createElement(i.Provider,{value:d},n.children)}},64361:(n,d,e)=>{e.r(d),e.d(d,{assets:()=>h,contentTitle:()=>t,default:()=>j,frontMatter:()=>i,metadata:()=>l,toc:()=>c});var r=e(74848),s=e(28453);const i={},t="TuGraph\u56fe\u6a21\u578b\u8bf4\u660e",l={id:"introduction/schema",title:"TuGraph\u56fe\u6a21\u578b\u8bf4\u660e",description:"1. \u6570\u636e\u6a21\u578b",source:"@site/versions/version-4.2.0/zh-CN/source/2.introduction/4.schema.md",sourceDirName:"2.introduction",slug:"/introduction/schema",permalink:"/tugraph-db/en-US/zh/4.2.0/introduction/schema",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:4,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"\u4ec0\u4e48\u662fTuGraph",permalink:"/tugraph-db/en-US/zh/4.2.0/introduction/what-is-tugraph"},next:{title:"\u6027\u80fd\u4f18\u5148",permalink:"/tugraph-db/en-US/zh/4.2.0/introduction/characteristics/performance-oriented"}},h={},c=[{value:"1. \u6570\u636e\u6a21\u578b",id:"1-\u6570\u636e\u6a21\u578b",level:2},{value:"1.1. \u56fe\u6a21\u578b",id:"11-\u56fe\u6a21\u578b",level:3},{value:"1.2. \u6570\u636e\u7c7b\u578b",id:"12-\u6570\u636e\u7c7b\u578b",level:3},{value:"1.3. \u7d22\u5f15",id:"13-\u7d22\u5f15",level:3},{value:"2. \u56fe\u9879\u76ee\u3001\u70b9\u3001\u8fb9\u3001\u5c5e\u6027\u547d\u540d\u89c4\u5219\u548c\u5efa\u8bae",id:"2-\u56fe\u9879\u76ee\u70b9\u8fb9\u5c5e\u6027\u547d\u540d\u89c4\u5219\u548c\u5efa\u8bae",level:2},{value:"2.1 \u547d\u540d\u89c4\u5219",id:"21-\u547d\u540d\u89c4\u5219",level:3},{value:"2.2 \u4f7f\u7528\u9650\u5236",id:"22-\u4f7f\u7528\u9650\u5236",level:3},{value:"2.3 \u547d\u540d\u5efa\u8bae",id:"23-\u547d\u540d\u5efa\u8bae",level:3}];function x(n){const d={code:"code",div:"div",h1:"h1",h2:"h2",h3:"h3",header:"header",li:"li",p:"p",strong:"strong",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",ul:"ul",...(0,s.R)(),...n.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(d.header,{children:(0,r.jsx)(d.h1,{id:"tugraph\u56fe\u6a21\u578b\u8bf4\u660e",children:"TuGraph\u56fe\u6a21\u578b\u8bf4\u660e"})}),"\n",(0,r.jsx)(d.h2,{id:"1-\u6570\u636e\u6a21\u578b",children:"1. \u6570\u636e\u6a21\u578b"}),"\n",(0,r.jsx)(d.h3,{id:"11-\u56fe\u6a21\u578b",children:"1.1. \u56fe\u6a21\u578b"}),"\n",(0,r.jsx)(d.p,{children:"TuGraph\u662f\u4e00\u4e2a\u5177\u5907\u591a\u56fe\u80fd\u529b\u7684\u5f3a\u7c7b\u578b\u3001\u6709\u5411\u5c5e\u6027\u56fe\u6570\u636e\u5e93\u3002"}),"\n",(0,r.jsxs)(d.ul,{children:["\n",(0,r.jsx)(d.li,{children:"\u56fe\u9879\u76ee\uff1a\u6bcf\u4e2a\u6570\u636e\u5e93\u670d\u52a1\u53ef\u4ee5\u627f\u8f7d\u591a\u4e2a\u56fe\u9879\u76ee\uff08\u591a\u56fe\uff09\uff0c\u6bcf\u4e2a\u56fe\u9879\u76ee\u53ef\u4ee5\u6709\u81ea\u5df1\u7684\u8bbf\u95ee\u63a7\u5236\u914d\u7f6e\uff0c\u6570\u636e\u5e93\u7ba1\u7406\u5458\u53ef\u4ee5\u521b\u5efa\u6216\u5220\u9664\u6307\u5b9a\u56fe\u9879\u76ee\u3002"}),"\n",(0,r.jsxs)(d.li,{children:["\u70b9\uff1a\u6307\u5b9e\u4f53\uff0c\u4e00\u822c\u7528\u4e8e\u8868\u8fbe\u73b0\u5b9e\u4e2d\u7684\u5b9e\u4f53\u5bf9\u8c61\uff0c\u5982\u4e00\u90e8\u7535\u5f71\u3001\u4e00\u4e2a\u6f14\u5458\u3002\n",(0,r.jsxs)(d.ul,{children:["\n",(0,r.jsx)(d.li,{children:"\u4e3b\u952e\uff1a\u7528\u6237\u81ea\u5b9a\u4e49\u7684\u70b9\u6570\u636e\u4e3b\u952e\uff0c\u9ed8\u8ba4\u552f\u4e00\u7d22\u5f15\uff0c\u5728\u5bf9\u5e94\u7684\u70b9\u7c7b\u578b\u4e2d\u552f\u4e00\u3002"}),"\n",(0,r.jsx)(d.li,{children:"VID\uff1a\u70b9\u5728\u5b58\u50a8\u5c42\u81ea\u52a8\u5206\u914d\u56fe\u9879\u76ee\u4e2d\u7684\u552f\u4e00ID\uff0c\u7528\u6237\u4e0d\u53ef\u4fee\u6539\u3002"}),"\n",(0,r.jsx)(d.li,{children:"\u4e0a\u9650\uff1a\u6bcf\u4e2a\u56fe\u9879\u76ee\u5b58\u50a8\u6700\u591a2^(40)\u4e2a\u70b9\u6570\u636e\u3002"}),"\n"]}),"\n"]}),"\n",(0,r.jsxs)(d.li,{children:["\u8fb9\uff1a\u7528\u4e8e\u8868\u8fbe\u70b9\u4e0e\u70b9\u4e4b\u95f4\u7684\u5173\u7cfb\uff0c\u5982\u6f14\u5458\u51fa\u6f14\u7535\u5f71\u3002\n",(0,r.jsxs)(d.ul,{children:["\n",(0,r.jsx)(d.li,{children:"\u6709\u5411\u8fb9\uff1a\u8fb9\u4e3a\u6709\u5411\u8fb9\u3002\u82e5\u8981\u6a21\u62df\u65e0\u5411\u8fb9\uff0c\u7528\u6237\u53ef\u4ee5\u521b\u5efa\u4e24\u4e2a\u65b9\u5411\u76f8\u53cd\u7684\u8fb9\u3002"}),"\n",(0,r.jsx)(d.li,{children:"\u591a\u6761\u8fb9\uff1a\u4e24\u4e2a\u70b9\u6570\u636e\u4e4b\u95f4\u53ef\u4ee5\u6709\u591a\u6761\u8fb9\u6570\u636e\u3002\u5f53\u524dTuGraph\u652f\u6301\u91cd\u590d\u8fb9\uff0c\u5982\u8981\u786e\u4fdd\u8fb9\u8fb9\u552f\u4e00\uff0c\u9700\u8981\u901a\u8fc7\u4e1a\u52a1\u7b56\u7565\u5b9e\u73b0\u3002"}),"\n",(0,r.jsx)(d.li,{children:"\u4e0a\u9650\uff1a\u4e24\u4e2a\u70b9\u6570\u636e\u4e4b\u95f4\u5b58\u50a8\u6700\u591a2^(32)\u6761\u8fb9\u6570\u636e\u3002"}),"\n"]}),"\n"]}),"\n",(0,r.jsx)(d.li,{children:"\u5c5e\u6027\u56fe\uff1a\u70b9\u548c\u8fb9\u53ef\u4ee5\u5177\u6709\u4e0e\u5176\u5173\u8054\u7684\u5c5e\u6027\uff0c\u6bcf\u4e2a\u5c5e\u6027\u53ef\u4ee5\u6709\u4e0d\u540c\u7684\u7c7b\u578b\u3002"}),"\n",(0,r.jsxs)(d.li,{children:["\u5f3a\u7c7b\u578b\uff1a\u6bcf\u4e2a\u70b9\u548c\u8fb9\u6709\u4e14\u4ec5\u6709\u4e00\u4e2a\u6807\u7b7e\uff0c\u521b\u5efa\u6807\u7b7e\u540e\uff0c\u4fee\u6539\u5c5e\u6027\u6570\u91cf\u53ca\u7c7b\u578b\u6709\u4ee3\u4ef7\u3002\n",(0,r.jsxs)(d.ul,{children:["\n",(0,r.jsx)(d.li,{children:"\u6307\u5b9a\u8fb9\u7684\u8d77/\u7ec8\u70b9\u7c7b\u578b\uff1a\u53ef\u9650\u5236\u8fb9\u7684\u8d77\u70b9\u548c\u7ec8\u70b9\u70b9\u7c7b\u578b\uff0c\u652f\u6301\u540c\u7c7b\u578b\u8fb9\u7684\u8d77\u70b9\u548c\u7ec8\u70b9\u7684\u70b9\u7c7b\u578b\u4e0d\u540c\uff0c\u5982\u4e2a\u4eba\u8f6c\u8d26\u7ed9\u516c\u53f8\u3001\u516c\u53f8\u8f6c\u8d26\u7ed9\u516c\u53f8\uff1b\u5f53\u6307\u5b9a\u8fb9\u7684\u8d77/\u7ec8\u70b9\u7c7b\u578b\u540e\uff0c\u53ef\u589e\u52a0\u591a\u7ec4\u8d77/\u7ec8\u70b9\u7c7b\u578b\uff0c\u4e0d\u53ef\u5220\u9664\u5df2\u9650\u5236\u7684\u8d77/\u7ec8\u70b9\u7c7b\u578b\u3002"}),"\n",(0,r.jsx)(d.li,{children:"\u65e0\u9650\u5236\u6a21\u5f0f\uff1a\u652f\u6301\u4e0d\u6307\u5b9a\u8fb9\u7684\u8d77\u70b9\u548c\u7ec8\u70b9\u7684\u70b9\u7c7b\u578b\uff0c\u4efb\u610f\u4e24\u4e2a\u70b9\u7c7b\u578b\u95f4\u5747\u53ef\u521b\u5efa\u8be5\u7c7b\u578b\u7684\u8fb9\u6570\u636e\u3002\u6ce8\uff1a\u5f53\u6307\u5b9a\u8fb9\u7684\u8d77/\u7ec8\u70b9\u7c7b\u578b\u540e\u65e0\u6cd5\u518d\u91c7\u7528\u65e0\u9650\u5236\u6a21\u5f0f\u3002"}),"\n"]}),"\n"]}),"\n"]}),"\n",(0,r.jsx)(d.h3,{id:"12-\u6570\u636e\u7c7b\u578b",children:"1.2. \u6570\u636e\u7c7b\u578b"}),"\n",(0,r.jsx)(d.p,{children:"TuGraph\u652f\u6301\u591a\u79cd\u53ef\u7528\u4e8e\u5c5e\u6027\u7684\u6570\u636e\u7c7b\u578b\u3002\u5177\u4f53\u652f\u6301\u7684\u6570\u636e\u7c7b\u578b\u5982\u4e0b\uff1a"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(d.table,{children:[(0,r.jsx)(d.thead,{children:(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.th,{children:(0,r.jsx)(d.strong,{children:"\u6570\u636e\u7c7b\u578b"})}),(0,r.jsx)(d.th,{children:(0,r.jsx)(d.strong,{children:"\u6700\u5c0f\u503c"})}),(0,r.jsx)(d.th,{children:(0,r.jsx)(d.strong,{children:"\u6700\u5927\u503c"})}),(0,r.jsx)(d.th,{children:(0,r.jsx)(d.strong,{children:"\u63cf\u8ff0"})})]})}),(0,r.jsxs)(d.tbody,{children:[(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.td,{children:"BOOL"}),(0,r.jsx)(d.td,{children:"false"}),(0,r.jsx)(d.td,{children:"true"}),(0,r.jsx)(d.td,{children:"\u5e03\u5c14\u503c"})]}),(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.td,{children:"INT8"}),(0,r.jsx)(d.td,{children:"-128"}),(0,r.jsx)(d.td,{children:"127"}),(0,r.jsx)(d.td,{children:"8\u4f4d\u6574\u578b"})]}),(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.td,{children:"INT16"}),(0,r.jsx)(d.td,{children:"-32768"}),(0,r.jsx)(d.td,{children:"32767"}),(0,r.jsx)(d.td,{children:"16\u4f4d\u6574\u578b"})]}),(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.td,{children:"INT32"}),(0,r.jsx)(d.td,{children:"- 2^31"}),(0,r.jsx)(d.td,{children:"2^31 - 1"}),(0,r.jsx)(d.td,{children:"32\u4f4d\u6574\u578b"})]}),(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.td,{children:"INT64"}),(0,r.jsx)(d.td,{children:"- 2^63"}),(0,r.jsx)(d.td,{children:"2^63 - 1"}),(0,r.jsx)(d.td,{children:"64\u4f4d\u6574\u578b"})]}),(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.td,{children:"DATE"}),(0,r.jsx)(d.td,{children:"0000-00-00"}),(0,r.jsx)(d.td,{children:"9999-12-31"}),(0,r.jsx)(d.td,{children:'"YYYY-MM-DD" \u683c\u5f0f\u7684\u65e5\u671f'})]}),(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.td,{children:"DATETIME"}),(0,r.jsx)(d.td,{children:"0000-00-00 00:00:00.000000"}),(0,r.jsx)(d.td,{children:"9999-12-31 23:59:59.999999"}),(0,r.jsxs)(d.td,{children:['"YYYY-MM-DD HH:mm',(0,r.jsx)(d.div,{children:".ffffff"}),'" \u683c\u5f0f\u7684\u65e5\u671f\u65f6\u95f4']})]}),(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.td,{children:"FLOAT"}),(0,r.jsx)(d.td,{}),(0,r.jsx)(d.td,{}),(0,r.jsx)(d.td,{children:"32\u4f4d\u6d6e\u70b9\u6570"})]}),(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.td,{children:"DOUBLE"}),(0,r.jsx)(d.td,{}),(0,r.jsx)(d.td,{}),(0,r.jsx)(d.td,{children:"64\u4f4d\u6d6e\u70b9\u6570"})]}),(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.td,{children:"STRING"}),(0,r.jsx)(d.td,{}),(0,r.jsx)(d.td,{}),(0,r.jsx)(d.td,{children:"\u4e0d\u5b9a\u957f\u5ea6\u7684\u5b57\u7b26\u4e32"})]}),(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.td,{children:"BLOB"}),(0,r.jsx)(d.td,{}),(0,r.jsx)(d.td,{}),(0,r.jsx)(d.td,{children:"\u4e8c\u8fdb\u5236\u6570\u636e\uff08\u5728\u8f93\u5165\u8f93\u51fa\u65f6\u4f7f\u7528Base64\u7f16\u7801\uff09"})]})]})]}),"\n",(0,r.jsx)(d.h3,{id:"13-\u7d22\u5f15",children:"1.3. \u7d22\u5f15"}),"\n",(0,r.jsx)(d.p,{children:"TuGraph\u652f\u6301\u5bf9\u70b9\u6216\u8fb9\u7684\u5c5e\u6027\u521b\u5efa\u7d22\u5f15\uff0c\u4ee5\u63d0\u5347\u67e5\u8be2\u6548\u7387\u3002"}),"\n",(0,r.jsxs)(d.ul,{children:["\n",(0,r.jsx)(d.li,{children:"\u7d22\u5f15\u53ef\u4ee5\u662f\u552f\u4e00\u6216\u975e\u552f\u4e00\u7d22\u5f15\u3002"}),"\n",(0,r.jsx)(d.li,{children:"\u5982\u679c\u4e3a\u70b9\u6807\u7b7e\u521b\u5efa\u4e86\u552f\u4e00\u7d22\u5f15\uff0c\u5728\u4fee\u6539\u8be5\u6807\u7b7e\u7684\u70b9\u65f6\uff0c\u4f1a\u5148\u6267\u884c\u6570\u636e\u5b8c\u6574\u6027\u68c0\u67e5\uff0c\u4ee5\u786e\u4fdd\u8be5\u7d22\u5f15\u7684\u552f\u4e00\u6027\u3002"}),"\n",(0,r.jsx)(d.li,{children:"\u6bcf\u4e2a\u7d22\u5f15\u90fd\u57fa\u4e8e\u4e00\u4e2a\u70b9\u6216\u8fb9\u7684\u4e00\u4e2a\u5c5e\u6027\u521b\u5efa\uff0c\u53ef\u4ee5\u5bf9\u540c\u4e00\u70b9\u6216\u8fb9\u7684\u591a\u4e2a\u5c5e\u6027\u521b\u5efa\u7d22\u5f15\u3002"}),"\n",(0,r.jsx)(d.li,{children:"BLOB\u7c7b\u578b\u7684\u5c5e\u6027\u4e0d\u80fd\u5efa\u7acb\u7d22\u5f15\u3002"}),"\n"]}),"\n",(0,r.jsx)(d.h2,{id:"2-\u56fe\u9879\u76ee\u70b9\u8fb9\u5c5e\u6027\u547d\u540d\u89c4\u5219\u548c\u5efa\u8bae",children:"2. \u56fe\u9879\u76ee\u3001\u70b9\u3001\u8fb9\u3001\u5c5e\u6027\u547d\u540d\u89c4\u5219\u548c\u5efa\u8bae"}),"\n",(0,r.jsx)(d.h3,{id:"21-\u547d\u540d\u89c4\u5219",children:"2.1 \u547d\u540d\u89c4\u5219"}),"\n",(0,r.jsx)(d.p,{children:"\u56fe\u9879\u76ee\u3001\u70b9\u3001\u8fb9\u548c\u5c5e\u6027\u662f\u8bc6\u522b\u7b26\u3002\u8be5\u8282\u63cf\u8ff0\u4e86\u5728TuGraph\u4e2d\u8bc6\u522b\u7b26\u7684\u5141\u8bb8\u7684\u8bed\u6cd5\u3002\n\u4e0b\u9762\u7684\u8868\u63cf\u8ff0\u4e86\u6bcf\u7c7b\u8bc6\u522b\u7b26\u7684\u6700\u5927\u957f\u5ea6\u548c\u5141\u8bb8\u7684\u5b57\u7b26\u3002"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(d.table,{children:[(0,r.jsx)(d.thead,{children:(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.th,{children:(0,r.jsx)(d.strong,{children:"\u8bc6\u522b\u7b26"})}),(0,r.jsx)(d.th,{children:(0,r.jsx)(d.strong,{children:"\u957f\u5ea6"})}),(0,r.jsx)(d.th,{children:(0,r.jsx)(d.strong,{children:"\u5141\u8bb8\u7684\u5b57\u7b26"})})]})}),(0,r.jsxs)(d.tbody,{children:[(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.td,{children:"\u7528\u6237\u3001\u89d2\u8272\u3001\u56fe\u9879\u76ee"}),(0,r.jsx)(d.td,{children:"1-64\u5b57\u7b26"}),(0,r.jsx)(d.td,{children:"\u5141\u8bb8\u4e2d\u6587\u3001\u5b57\u6bcd\u3001\u6570\u5b57\u3001\u4e0b\u5212\u7ebf\uff0c\u4e14\u9996\u5b57\u7b26\u4e0d\u4e3a\u6570\u5b57"})]}),(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.td,{children:"\u70b9\u7c7b\u578b\u3001\u8fb9\u7c7b\u578b\u3001\u5c5e\u6027"}),(0,r.jsx)(d.td,{children:"1~256\u5b57\u7b26"}),(0,r.jsx)(d.td,{children:"\u5141\u8bb8\u4e2d\u6587\u3001\u5b57\u6bcd\u3001\u6570\u5b57\u3001\u4e0b\u5212\u7ebf\uff0c\u4e14\u9996\u5b57\u7b26\u4e0d\u4e3a\u6570\u5b57"})]})]})]}),"\n",(0,r.jsx)(d.h3,{id:"22-\u4f7f\u7528\u9650\u5236",children:"2.2 \u4f7f\u7528\u9650\u5236"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(d.table,{children:[(0,r.jsx)(d.thead,{children:(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.th,{children:(0,r.jsx)(d.strong,{children:"\u63cf\u8ff0"})}),(0,r.jsx)(d.th,{children:(0,r.jsx)(d.strong,{children:"\u6700\u5927\u4e2a\u6570"})})]})}),(0,r.jsxs)(d.tbody,{children:[(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.td,{children:"\u7528\u6237\u6570\u3001\u89d2\u8272\u6570"}),(0,r.jsx)(d.td,{children:"65536"})]}),(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.td,{children:"\u56fe\u9879\u76ee\u7684\u4e2a\u6570"}),(0,r.jsx)(d.td,{children:"4096"})]}),(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.td,{children:"\u6bcf\u4e2a\u56fe\u9879\u76ee\u7684\u70b9\u548c\u8fb9\u7c7b\u578b\u6570\u91cf\u4e4b\u548c"}),(0,r.jsx)(d.td,{children:"4096"})]}),(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.td,{children:"\u6bcf\u4e2a\u70b9\u6216\u8fb9\u7c7b\u578b\u7684\u5c5e\u6027\u6570\u91cf"}),(0,r.jsx)(d.td,{children:"1024"})]})]})]}),"\n",(0,r.jsx)(d.p,{children:"\u6ce8\uff1a\n1\u3001\u7279\u6b8a\u5b57\u7b26\u548c\u5173\u952e\u5b57\u8bf4\u660e\uff1a\u4f7f\u7528\u7279\u6b8a\u5b57\u7b26\u6216\u975e\u4fdd\u7559\u5173\u952e\u5b57\u65f6\uff0c\u9700\u8981\u4f7f\u7528\u53cd\u5355\u5f15\u53f7/backquote\uff08``\uff09\u8fdb\u884c\u5f15\u7528\uff1b"}),"\n",(0,r.jsxs)(d.p,{children:["\u793a\u4f8b\uff1a ",(0,r.jsx)(d.code,{children:"match (`match`:match) return `match`.id limit 1"})]}),"\n",(0,r.jsx)(d.p,{children:"2\u3001\u5927\u5c0f\u5199\u654f\u611f\u6027\uff1aTuGraph\u5927\u5c0f\u5199\u654f\u611f\uff1b"}),"\n",(0,r.jsx)(d.p,{children:"3\u3001\u56fe\u9879\u76ee\u3001\u70b9/\u8fb9\u3001\u5c5e\u6027\u540d\u79f0\u4e4b\u95f4\u53ef\u4ee5\u91cd\u590d\u4f7f\u7528\uff0c\u540c\u4e00\u70b9\u6216\u8fb9\u4e0b\u7684\u5c5e\u6027\u540d\u79f0\u4e0d\u53ef\u4ee5\u91cd\u590d\uff1b"}),"\n",(0,r.jsx)(d.p,{children:"4\u3001\u5c5e\u6027\u540d\u5b57\u4fdd\u7559\u5173\u952e\u5b57\uff1aSRC_ID / DST_ID / SKIP"}),"\n",(0,r.jsx)(d.h3,{id:"23-\u547d\u540d\u5efa\u8bae",children:"2.3 \u547d\u540d\u5efa\u8bae"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(d.table,{children:[(0,r.jsx)(d.thead,{children:(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.th,{children:(0,r.jsx)(d.strong,{children:"\u8bc6\u522b\u7b26"})}),(0,r.jsx)(d.th,{children:(0,r.jsx)(d.strong,{children:"\u63cf\u8ff0"})}),(0,r.jsx)(d.th,{children:(0,r.jsx)(d.strong,{children:"\u5efa\u8bae"})})]})}),(0,r.jsxs)(d.tbody,{children:[(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.td,{children:"\u56fe\u9879\u76ee"}),(0,r.jsx)(d.td,{children:"\u5b57\u6bcd\u6216\u4e2d\u6587\u5f00\u5934"}),(0,r.jsx)(d.td,{children:"\u5982graph123\u3001project123\u7b49"})]}),(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.td,{children:"\u70b9/\u8fb9\u7c7b\u578b"}),(0,r.jsx)(d.td,{children:"\u5b57\u6bcd\u6216\u4e2d\u6587\u5f00\u5934\uff0c\u4f7f\u7528\u4e0b\u5212\u7ebf\u533a\u5206\u5355\u8bcd"}),(0,r.jsx)(d.td,{children:"\u5982person\u3001act_in\u7b49"})]}),(0,r.jsxs)(d.tr,{children:[(0,r.jsx)(d.td,{children:"\u5c5e\u6027"}),(0,r.jsx)(d.td,{children:"\u5b57\u6bcd\u6216\u4e2d\u6587"}),(0,r.jsx)(d.td,{children:"\u5982name\u3001age\u7b49"})]})]})]})]})}function j(n={}){const{wrapper:d}={...(0,s.R)(),...n.components};return d?(0,r.jsx)(d,{...n,children:(0,r.jsx)(x,{...n})}):x(n)}}}]);