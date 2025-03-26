"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[26142],{28453:(n,e,d)=>{d.d(e,{R:()=>l,x:()=>h});var i=d(96540);const s={},r=i.createContext(s);function l(n){const e=i.useContext(r);return i.useMemo((function(){return"function"==typeof n?n(e):{...e,...n}}),[e,n])}function h(n){let e;return e=n.disableParentContext?"function"==typeof n.components?n.components(s):n.components||s:l(n.components),i.createElement(r.Provider,{value:e},n.children)}},43945:(n,e,d)=>{d.r(e),d.d(e,{assets:()=>t,contentTitle:()=>l,default:()=>j,frontMatter:()=>r,metadata:()=>h,toc:()=>c});var i=d(74848),s=d(28453);const r={},l="TuGraph\u56fe\u6a21\u578b\u8bf4\u660e",h={id:"introduction/schema",title:"TuGraph\u56fe\u6a21\u578b\u8bf4\u660e",description:"1. \u6570\u636e\u6a21\u578b",source:"@site/versions/version-4.3.2/zh-CN/source/2.introduction/4.schema.md",sourceDirName:"2.introduction",slug:"/introduction/schema",permalink:"/tugraph-db/en-US/zh/4.3.2/introduction/schema",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:4,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"\u4ec0\u4e48\u662fTuGraph",permalink:"/tugraph-db/en-US/zh/4.3.2/introduction/what-is-tugraph"},next:{title:"\u6027\u80fd\u4f18\u5148",permalink:"/tugraph-db/en-US/zh/4.3.2/introduction/characteristics/performance-oriented"}},t={},c=[{value:"1. \u6570\u636e\u6a21\u578b",id:"1-\u6570\u636e\u6a21\u578b",level:2},{value:"1.1. \u56fe\u6a21\u578b",id:"11-\u56fe\u6a21\u578b",level:3},{value:"1.2. \u6570\u636e\u7c7b\u578b",id:"12-\u6570\u636e\u7c7b\u578b",level:3},{value:"1.3. \u7d22\u5f15",id:"13-\u7d22\u5f15",level:3},{value:"1.3.1 \u666e\u901a\u7d22\u5f15",id:"131-\u666e\u901a\u7d22\u5f15",level:4},{value:"1.3.1.1 \u70b9\u7d22\u5f15",id:"1311-\u70b9\u7d22\u5f15",level:5},{value:"1.3.1.1.1 unique\u7d22\u5f15",id:"13111-unique\u7d22\u5f15",level:6},{value:"1.3.1.1.2 non_unique\u7d22\u5f15",id:"13112-non_unique\u7d22\u5f15",level:6},{value:"1.3.1.2 \u8fb9\u7d22\u5f15",id:"1312-\u8fb9\u7d22\u5f15",level:5},{value:"1.3.1.2.1 unique\u7d22\u5f15",id:"13121-unique\u7d22\u5f15",level:6},{value:"1.3.1.2.2 pair_unique\u7d22\u5f15",id:"13122-pair_unique\u7d22\u5f15",level:6},{value:"1.3.1.2.3 non_unique\u7d22\u5f15",id:"13123-non_unique\u7d22\u5f15",level:6},{value:"1.3.2 \u7ec4\u5408\u7d22\u5f15",id:"132-\u7ec4\u5408\u7d22\u5f15",level:4},{value:"1.3.2.1 \u552f\u4e00\u7d22\u5f15",id:"1321-\u552f\u4e00\u7d22\u5f15",level:5},{value:"1.3.2.2 \u975e\u552f\u4e00\u7d22\u5f15",id:"1322-\u975e\u552f\u4e00\u7d22\u5f15",level:5},{value:"2. \u56fe\u9879\u76ee\u3001\u70b9\u3001\u8fb9\u3001\u5c5e\u6027\u547d\u540d\u89c4\u5219\u548c\u5efa\u8bae",id:"2-\u56fe\u9879\u76ee\u70b9\u8fb9\u5c5e\u6027\u547d\u540d\u89c4\u5219\u548c\u5efa\u8bae",level:2},{value:"2.1 \u547d\u540d\u89c4\u5219",id:"21-\u547d\u540d\u89c4\u5219",level:3},{value:"2.2 \u4f7f\u7528\u9650\u5236",id:"22-\u4f7f\u7528\u9650\u5236",level:3},{value:"2.3 \u547d\u540d\u5efa\u8bae",id:"23-\u547d\u540d\u5efa\u8bae",level:3}];function x(n){const e={code:"code",div:"div",h1:"h1",h2:"h2",h3:"h3",h4:"h4",h5:"h5",h6:"h6",header:"header",li:"li",ol:"ol",p:"p",strong:"strong",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",ul:"ul",...(0,s.R)(),...n.components};return(0,i.jsxs)(i.Fragment,{children:[(0,i.jsx)(e.header,{children:(0,i.jsx)(e.h1,{id:"tugraph\u56fe\u6a21\u578b\u8bf4\u660e",children:"TuGraph\u56fe\u6a21\u578b\u8bf4\u660e"})}),"\n",(0,i.jsx)(e.h2,{id:"1-\u6570\u636e\u6a21\u578b",children:"1. \u6570\u636e\u6a21\u578b"}),"\n",(0,i.jsx)(e.h3,{id:"11-\u56fe\u6a21\u578b",children:"1.1. \u56fe\u6a21\u578b"}),"\n",(0,i.jsx)(e.p,{children:"TuGraph\u662f\u4e00\u4e2a\u5177\u5907\u591a\u56fe\u80fd\u529b\u7684\u5f3a\u7c7b\u578b\u3001\u6709\u5411\u5c5e\u6027\u56fe\u6570\u636e\u5e93\u3002"}),"\n",(0,i.jsxs)(e.ul,{children:["\n",(0,i.jsx)(e.li,{children:"\u56fe\u9879\u76ee\uff1a\u6bcf\u4e2a\u6570\u636e\u5e93\u670d\u52a1\u53ef\u4ee5\u627f\u8f7d\u591a\u4e2a\u56fe\u9879\u76ee\uff08\u591a\u56fe\uff09\uff0c\u6bcf\u4e2a\u56fe\u9879\u76ee\u53ef\u4ee5\u6709\u81ea\u5df1\u7684\u8bbf\u95ee\u63a7\u5236\u914d\u7f6e\uff0c\u6570\u636e\u5e93\u7ba1\u7406\u5458\u53ef\u4ee5\u521b\u5efa\u6216\u5220\u9664\u6307\u5b9a\u56fe\u9879\u76ee\u3002"}),"\n",(0,i.jsxs)(e.li,{children:["\u70b9\uff1a\u6307\u5b9e\u4f53\uff0c\u4e00\u822c\u7528\u4e8e\u8868\u8fbe\u73b0\u5b9e\u4e2d\u7684\u5b9e\u4f53\u5bf9\u8c61\uff0c\u5982\u4e00\u90e8\u7535\u5f71\u3001\u4e00\u4e2a\u6f14\u5458\u3002\n",(0,i.jsxs)(e.ul,{children:["\n",(0,i.jsx)(e.li,{children:"\u4e3b\u952e\uff1a\u7528\u6237\u81ea\u5b9a\u4e49\u7684\u70b9\u6570\u636e\u4e3b\u952e\uff0c\u9ed8\u8ba4\u552f\u4e00\u7d22\u5f15\uff0c\u5728\u5bf9\u5e94\u7684\u70b9\u7c7b\u578b\u4e2d\u552f\u4e00\u3002"}),"\n",(0,i.jsx)(e.li,{children:"VID\uff1a\u70b9\u5728\u5b58\u50a8\u5c42\u81ea\u52a8\u5206\u914d\u56fe\u9879\u76ee\u4e2d\u7684\u552f\u4e00ID\uff0c\u7528\u6237\u4e0d\u53ef\u4fee\u6539\u3002"}),"\n",(0,i.jsx)(e.li,{children:"\u4e0a\u9650\uff1a\u6bcf\u4e2a\u56fe\u9879\u76ee\u5b58\u50a8\u6700\u591a2^(40)\u4e2a\u70b9\u6570\u636e\u3002"}),"\n"]}),"\n"]}),"\n",(0,i.jsxs)(e.li,{children:["\u8fb9\uff1a\u7528\u4e8e\u8868\u8fbe\u70b9\u4e0e\u70b9\u4e4b\u95f4\u7684\u5173\u7cfb\uff0c\u5982\u6f14\u5458\u51fa\u6f14\u7535\u5f71\u3002\n",(0,i.jsxs)(e.ul,{children:["\n",(0,i.jsx)(e.li,{children:"\u6709\u5411\u8fb9\uff1a\u8fb9\u4e3a\u6709\u5411\u8fb9\u3002\u82e5\u8981\u6a21\u62df\u65e0\u5411\u8fb9\uff0c\u7528\u6237\u53ef\u4ee5\u521b\u5efa\u4e24\u4e2a\u65b9\u5411\u76f8\u53cd\u7684\u8fb9\u3002"}),"\n",(0,i.jsx)(e.li,{children:"\u591a\u6761\u8fb9\uff1a\u4e24\u4e2a\u70b9\u6570\u636e\u4e4b\u95f4\u53ef\u4ee5\u6709\u591a\u6761\u8fb9\u6570\u636e\u3002\u5f53\u524dTuGraph\u652f\u6301\u91cd\u590d\u8fb9\uff0c\u5982\u8981\u786e\u4fdd\u8fb9\u8fb9\u552f\u4e00\uff0c\u9700\u8981\u901a\u8fc7\u4e1a\u52a1\u7b56\u7565\u5b9e\u73b0\u3002"}),"\n",(0,i.jsx)(e.li,{children:"\u4e0a\u9650\uff1a\u4e24\u4e2a\u70b9\u6570\u636e\u4e4b\u95f4\u5b58\u50a8\u6700\u591a2^(32)\u6761\u8fb9\u6570\u636e\u3002"}),"\n"]}),"\n"]}),"\n",(0,i.jsx)(e.li,{children:"\u5c5e\u6027\u56fe\uff1a\u70b9\u548c\u8fb9\u53ef\u4ee5\u5177\u6709\u4e0e\u5176\u5173\u8054\u7684\u5c5e\u6027\uff0c\u6bcf\u4e2a\u5c5e\u6027\u53ef\u4ee5\u6709\u4e0d\u540c\u7684\u7c7b\u578b\u3002"}),"\n",(0,i.jsxs)(e.li,{children:["\u5f3a\u7c7b\u578b\uff1a\u6bcf\u4e2a\u70b9\u548c\u8fb9\u6709\u4e14\u4ec5\u6709\u4e00\u4e2a\u6807\u7b7e\uff0c\u521b\u5efa\u6807\u7b7e\u540e\uff0c\u4fee\u6539\u5c5e\u6027\u6570\u91cf\u53ca\u7c7b\u578b\u6709\u4ee3\u4ef7\u3002\n",(0,i.jsxs)(e.ul,{children:["\n",(0,i.jsx)(e.li,{children:"\u6307\u5b9a\u8fb9\u7684\u8d77/\u7ec8\u70b9\u7c7b\u578b\uff1a\u53ef\u9650\u5236\u8fb9\u7684\u8d77\u70b9\u548c\u7ec8\u70b9\u70b9\u7c7b\u578b\uff0c\u652f\u6301\u540c\u7c7b\u578b\u8fb9\u7684\u8d77\u70b9\u548c\u7ec8\u70b9\u7684\u70b9\u7c7b\u578b\u4e0d\u540c\uff0c\u5982\u4e2a\u4eba\u8f6c\u8d26\u7ed9\u516c\u53f8\u3001\u516c\u53f8\u8f6c\u8d26\u7ed9\u516c\u53f8\uff1b\u5f53\u6307\u5b9a\u8fb9\u7684\u8d77/\u7ec8\u70b9\u7c7b\u578b\u540e\uff0c\u53ef\u589e\u52a0\u591a\u7ec4\u8d77/\u7ec8\u70b9\u7c7b\u578b\uff0c\u4e0d\u53ef\u5220\u9664\u5df2\u9650\u5236\u7684\u8d77/\u7ec8\u70b9\u7c7b\u578b\u3002"}),"\n",(0,i.jsx)(e.li,{children:"\u65e0\u9650\u5236\u6a21\u5f0f\uff1a\u652f\u6301\u4e0d\u6307\u5b9a\u8fb9\u7684\u8d77\u70b9\u548c\u7ec8\u70b9\u7684\u70b9\u7c7b\u578b\uff0c\u4efb\u610f\u4e24\u4e2a\u70b9\u7c7b\u578b\u95f4\u5747\u53ef\u521b\u5efa\u8be5\u7c7b\u578b\u7684\u8fb9\u6570\u636e\u3002\u6ce8\uff1a\u5f53\u6307\u5b9a\u8fb9\u7684\u8d77/\u7ec8\u70b9\u7c7b\u578b\u540e\u65e0\u6cd5\u518d\u91c7\u7528\u65e0\u9650\u5236\u6a21\u5f0f\u3002"}),"\n"]}),"\n"]}),"\n"]}),"\n",(0,i.jsx)(e.h3,{id:"12-\u6570\u636e\u7c7b\u578b",children:"1.2. \u6570\u636e\u7c7b\u578b"}),"\n",(0,i.jsx)(e.p,{children:"TuGraph\u652f\u6301\u591a\u79cd\u53ef\u7528\u4e8e\u5c5e\u6027\u7684\u6570\u636e\u7c7b\u578b\u3002\u5177\u4f53\u652f\u6301\u7684\u6570\u636e\u7c7b\u578b\u5982\u4e0b\uff1a"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,i.jsxs)(e.table,{children:[(0,i.jsx)(e.thead,{children:(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.th,{children:(0,i.jsx)(e.strong,{children:"\u6570\u636e\u7c7b\u578b"})}),(0,i.jsx)(e.th,{children:(0,i.jsx)(e.strong,{children:"\u6700\u5c0f\u503c"})}),(0,i.jsx)(e.th,{children:(0,i.jsx)(e.strong,{children:"\u6700\u5927\u503c"})}),(0,i.jsx)(e.th,{children:(0,i.jsx)(e.strong,{children:"\u63cf\u8ff0"})})]})}),(0,i.jsxs)(e.tbody,{children:[(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"BOOL"}),(0,i.jsx)(e.td,{children:"false"}),(0,i.jsx)(e.td,{children:"true"}),(0,i.jsx)(e.td,{children:"\u5e03\u5c14\u503c"})]}),(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"INT8"}),(0,i.jsx)(e.td,{children:"-128"}),(0,i.jsx)(e.td,{children:"127"}),(0,i.jsx)(e.td,{children:"8\u4f4d\u6574\u578b"})]}),(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"INT16"}),(0,i.jsx)(e.td,{children:"-32768"}),(0,i.jsx)(e.td,{children:"32767"}),(0,i.jsx)(e.td,{children:"16\u4f4d\u6574\u578b"})]}),(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"INT32"}),(0,i.jsx)(e.td,{children:"- 2^31"}),(0,i.jsx)(e.td,{children:"2^31 - 1"}),(0,i.jsx)(e.td,{children:"32\u4f4d\u6574\u578b"})]}),(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"INT64"}),(0,i.jsx)(e.td,{children:"- 2^63"}),(0,i.jsx)(e.td,{children:"2^63 - 1"}),(0,i.jsx)(e.td,{children:"64\u4f4d\u6574\u578b"})]}),(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"DATE"}),(0,i.jsx)(e.td,{children:"0000-00-00"}),(0,i.jsx)(e.td,{children:"9999-12-31"}),(0,i.jsx)(e.td,{children:'"YYYY-MM-DD" \u683c\u5f0f\u7684\u65e5\u671f'})]}),(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"DATETIME"}),(0,i.jsx)(e.td,{children:"0000-00-00 00:00:00.000000"}),(0,i.jsx)(e.td,{children:"9999-12-31 23:59:59.999999"}),(0,i.jsxs)(e.td,{children:['"YYYY-MM-DD HH:mm',(0,i.jsx)(e.div,{children:".ffffff"}),'" \u683c\u5f0f\u7684\u65e5\u671f\u65f6\u95f4']})]}),(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"FLOAT"}),(0,i.jsx)(e.td,{}),(0,i.jsx)(e.td,{}),(0,i.jsx)(e.td,{children:"32\u4f4d\u6d6e\u70b9\u6570"})]}),(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"DOUBLE"}),(0,i.jsx)(e.td,{}),(0,i.jsx)(e.td,{}),(0,i.jsx)(e.td,{children:"64\u4f4d\u6d6e\u70b9\u6570"})]}),(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"STRING"}),(0,i.jsx)(e.td,{}),(0,i.jsx)(e.td,{}),(0,i.jsx)(e.td,{children:"\u4e0d\u5b9a\u957f\u5ea6\u7684\u5b57\u7b26\u4e32"})]}),(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"BLOB"}),(0,i.jsx)(e.td,{}),(0,i.jsx)(e.td,{}),(0,i.jsx)(e.td,{children:"\u4e8c\u8fdb\u5236\u6570\u636e\uff08\u5728\u8f93\u5165\u8f93\u51fa\u65f6\u4f7f\u7528Base64\u7f16\u7801\uff09"})]}),(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"POINT"}),(0,i.jsx)(e.td,{}),(0,i.jsx)(e.td,{}),(0,i.jsx)(e.td,{children:"EWKB\u683c\u5f0f\u6570\u636e\uff0c\u8868\u793a\u70b9"})]}),(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"LINESTRING"}),(0,i.jsx)(e.td,{}),(0,i.jsx)(e.td,{}),(0,i.jsx)(e.td,{children:"EWKB\u683c\u5f0f\u6570\u636e\uff0c\u8868\u793a\u7ebf"})]}),(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"POLYGON"}),(0,i.jsx)(e.td,{}),(0,i.jsx)(e.td,{}),(0,i.jsx)(e.td,{children:"EWKB\u683c\u5f0f\u6570\u636e\uff0c\u8868\u793a\u9762(\u591a\u8fb9\u5f62)"})]}),(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"FLOAT_VECTOR"}),(0,i.jsx)(e.td,{}),(0,i.jsx)(e.td,{}),(0,i.jsx)(e.td,{children:"\u5305\u542b32\u4f4d\u6d6e\u70b9\u6570\u7684\u52a8\u6001\u5411\u91cf"})]})]})]}),"\n",(0,i.jsx)(e.h3,{id:"13-\u7d22\u5f15",children:"1.3. \u7d22\u5f15"}),"\n",(0,i.jsx)(e.p,{children:"TuGraph\u652f\u6301\u5bf9\u70b9\u6216\u8fb9\u7684\u5c5e\u6027\u521b\u5efa\u7d22\u5f15\uff0c\u4ee5\u63d0\u5347\u67e5\u8be2\u6548\u7387\u3002\u5176\u7279\u70b9\u5982\u4e0b\uff1a"}),"\n",(0,i.jsxs)(e.ul,{children:["\n",(0,i.jsx)(e.li,{children:"\u7d22\u5f15\u5305\u62ec\u666e\u901a\u7d22\u5f15\u548c\u7ec4\u5408\u7d22\u5f15\uff0c\u666e\u901a\u7d22\u5f15\u57fa\u4e8e\u4e00\u4e2a\u70b9\u6216\u8fb9\u7684\u4e00\u4e2a\u5c5e\u6027\u521b\u5efa\uff0c\u800c\u7ec4\u5408\u7d22\u5f15\u57fa\u4e8e\u4e00\u4e2a\u70b9\u6216\u8fb9\u7684\u591a\u4e2a\u5c5e\u6027\u521b\u5efa\uff08\u4e0d\u8d85\u8fc716\u4e2a\uff09\uff0c\u53ef\u4ee5\u5bf9\u540c\u4e00\u70b9\u6216\u8fb9\u7684\u591a\u4e2a\uff08\u7ec4\uff09\u5c5e\u6027\u521b\u5efa\u7d22\u5f15\u3002"}),"\n",(0,i.jsx)(e.li,{children:"\u5982\u679c\u4e3a\u70b9\u6807\u7b7e\u521b\u5efa\u4e86\u552f\u4e00\u7d22\u5f15\uff0c\u5728\u4fee\u6539\u8be5\u6807\u7b7e\u7684\u70b9\u65f6\uff0c\u4f1a\u5148\u6267\u884c\u6570\u636e\u5b8c\u6574\u6027\u68c0\u67e5\uff0c\u4ee5\u786e\u4fdd\u8be5\u7d22\u5f15\u7684\u552f\u4e00\u6027\u3002"}),"\n",(0,i.jsx)(e.li,{children:"BLOB\u7c7b\u578b\u7684\u5c5e\u6027\u4e0d\u80fd\u5efa\u7acb\u7d22\u5f15\u3002"}),"\n"]}),"\n",(0,i.jsx)(e.p,{children:"TuGraph\u7684\u70b9\u8fb9\u5747\u6709\u591a\u79cd\u7d22\u5f15\u7c7b\u578b\uff0c\u4e0d\u540c\u7684\u7d22\u5f15\u7c7b\u578b\u7684\u529f\u80fd\u548c\u9650\u5236\u4e0d\u540c\uff0c\u5177\u4f53\u5982\u4e0b\uff1a"}),"\n",(0,i.jsx)(e.h4,{id:"131-\u666e\u901a\u7d22\u5f15",children:"1.3.1 \u666e\u901a\u7d22\u5f15"}),"\n",(0,i.jsx)(e.h5,{id:"1311-\u70b9\u7d22\u5f15",children:"1.3.1.1 \u70b9\u7d22\u5f15"}),"\n",(0,i.jsx)(e.h6,{id:"13111-unique\u7d22\u5f15",children:"1.3.1.1.1 unique\u7d22\u5f15"}),"\n",(0,i.jsxs)(e.p,{children:["\u70b9\u7684unique\u7d22\u5f15\u6307\u7684\u662f\u5168\u5c40\u552f\u4e00\u7684\u7d22\u5f15\uff0c\u5373\u82e5\u4e00\u4e2a\u5c5e\u6027\u8bbe\u7f6e\u4e86unique\u7d22\u5f15\uff0c\u5728\u540c\u4e00\u4e2a\u56fe\u4e2d\uff0c\u76f8\u540clabel\u7684\u70b9\u7684\u8be5\u5c5e\u6027\u4e0d\u4f1a\u5b58\u5728\u76f8\u540c\u7684\u503c\uff0c\nunique\u7d22\u5f15key\u7684\u6700\u5927\u957f\u5ea6\u662f480bytes\uff0c",(0,i.jsx)(e.strong,{children:"\u8d85\u8fc7480bytes\u7684\u5c5e\u6027\u4e0d\u80fd\u5efa\u7acbunique\u7d22\u5f15"}),"\u3002\nprimary\u4f5c\u4e3a\u7279\u6b8a\u7684unique\u7d22\u5f15\uff0c\u56e0\u6b64\u6700\u5927key\u7684\u957f\u5ea6\u4e5f\u662f480bytes\u3002"]}),"\n",(0,i.jsx)(e.h6,{id:"13112-non_unique\u7d22\u5f15",children:"1.3.1.1.2 non_unique\u7d22\u5f15"}),"\n",(0,i.jsxs)(e.p,{children:["\u70b9\u7684non_unique\u7d22\u5f15\u6307\u7684\u662f\u975e\u5168\u5c40\u552f\u4e00\u7684\u7d22\u5f15\uff0c\u5373\u82e5\u4e00\u4e2a\u5c5e\u6027\u8bbe\u7f6e\u4e86non_unique\u7d22\u5f15\uff0c\n\u5728\u540c\u4e00\u4e2a\u56fe\u4e2d\uff0c\u76f8\u540clabel\u7684\u70b9\u7684\u8be5\u5c5e\u6027\u53ef\u4ee5\u5b58\u5728\u76f8\u540c\u7684\u503c\u3002\n\u7531\u4e8enon_unique\u7d22\u5f15\u4e00\u4e2akey\u53ef\u80fd\u6620\u5c04\u5230\u591a\u4e2a\u503c\uff0c\u4e3a\u4e86\u52a0\u901f\u67e5\u627e\u548c\u5199\u5165\uff0c\n\u5728\u7528\u6237\u6307\u5b9a\u7684key\u540e\u9762\u52a0\u4e0a\u4e86\u7d22\u5f15key\u76f8\u540c\u7684\u4e00\u7ec4vid\u7684\u6700\u5927\u503c\u3002\n\u6bcf\u4e2avid\u662f5bytes\u957f\u5ea6\uff0c\u56e0\u6b64non_unique\u7d22\u5f15key\u6700\u5927\u957f\u5ea6\u662f475bytes\u3002\n\u4f46\u662f\uff0c\u4e0d\u540c\u4e8eunique\u7d22\u5f15\uff0c\u8d85\u8fc7475bytes\u4e5f\u53ef\u4ee5\u5efa\u7acbnon_unique\u7d22\u5f15\u3002\n\u53ea\u4e0d\u8fc7\u5728\u5bf9\u8fd9\u6837\u7684\u5c5e\u6027\u5efa\u7acb\u7d22\u5f15\u65f6\u4f1a\u53ea\u622a\u53d6",(0,i.jsx)(e.strong,{children:"\u524d475bytes"}),"\u4f5c\u4e3a\u7d22\u5f15key\uff08\u5c5e\u6027\u672c\u8eab\u5b58\u50a8\u7684\u503c\u4e0d\u53d7\u5f71\u54cd\uff09\u3002\n\u5e76\u4e14\uff0c\u5728\u901a\u8fc7\u8fed\u4ee3\u5668\u904d\u5386\u65f6\uff0c\u4e5f\u662f\u5148\u81ea\u52a8\u622a\u53d6\u67e5\u8be2\u503c\u7684\u524d475bytes\u518d\u8fdb\u884c\u904d\u5386\uff0c\n\u6240\u4ee5\u7ed3\u679c\u53ef\u80fd\u548c\u9884\u671f\u4e0d\u4e00\u81f4\uff0c\u9700\u8981\u7528\u6237\u518d\u8fc7\u6ee4\u3002"]}),"\n",(0,i.jsx)(e.h5,{id:"1312-\u8fb9\u7d22\u5f15",children:"1.3.1.2 \u8fb9\u7d22\u5f15"}),"\n",(0,i.jsx)(e.h6,{id:"13121-unique\u7d22\u5f15",children:"1.3.1.2.1 unique\u7d22\u5f15"}),"\n",(0,i.jsxs)(e.p,{children:["\u548c\u70b9\u7c7b\u4f3c\uff0c\u8fb9\u7684unique\u7d22\u5f15\u6307\u7684\u662f\u5168\u5c40\u552f\u4e00\u7684\u7d22\u5f15\uff0c\u5373\u82e5\u4e00\u4e2a\u5c5e\u6027\u8bbe\u7f6e\u4e86unique\u7d22\u5f15\uff0c\u5728\u540c\u4e00\u4e2a\u56fe\u4e2d\uff0c\u76f8\u540clabel\u7684\u8fb9\u7684\u8be5\u5c5e\u6027\u4e0d\u4f1a\u5b58\u5728\u76f8\u540c\u7684\u503c\uff0c\nunique\u7d22\u5f15key\u7684\u6700\u5927\u957f\u5ea6\u662f480bytes\uff0c",(0,i.jsx)(e.strong,{children:"\u8d85\u8fc7480bytes\u7684\u5c5e\u6027\u4e0d\u80fd\u5efa\u7acbunique\u7d22\u5f15"}),"\u3002"]}),"\n",(0,i.jsx)(e.h6,{id:"13122-pair_unique\u7d22\u5f15",children:"1.3.1.2.2 pair_unique\u7d22\u5f15"}),"\n",(0,i.jsxs)(e.p,{children:["pair_unique\u7d22\u5f15\u6307\u7684\u662f\u4e24\u70b9\u95f4\u7684\u552f\u4e00\u7d22\u5f15\uff0c\u5373\u82e5\u4e00\u4e2a\u5c5e\u6027\u8bbe\u7f6e\u4e86unique\u7d22\u5f15\uff0c\u5728\u540c\u4e00\u4e2a\u56fe\u7684\u540c\u4e00\u7ec4\u8d77\u70b9\u548c\u7ec8\u70b9\u4e4b\u95f4\uff0c\n\u76f8\u540clabel\u7684\u8fb9\u7684\u8be5\u5c5e\u6027\u4e0d\u4f1a\u5b58\u5728\u76f8\u540c\u7684\u503c\u3002\u4e3a\u4e86\u4fdd\u8bc1pair_unique\u7d22\u5f15key\u5728\u540c\u4e00\u7ec4\u8d77\u70b9\u548c\u7ec8\u70b9\u4e4b\u95f4\u4e0d\u91cd\u590d\uff0c\n\u7d22\u5f15\u5728\u7528\u6237\u6307\u5b9a\u7684key\u540e\u9762\u52a0\u4e0a\u4e86\u8d77\u70b9\u548c\u7ec8\u70b9\u7684vid\uff0c\u6bcf\u4e2avid\u662f5bytes\u957f\u5ea6\u3002\n\u56e0\u6b64\u6700\u5927key\u7684\u957f\u5ea6\u662f470bytes\uff0c",(0,i.jsx)(e.strong,{children:"\u8d85\u8fc7470bytes\u7684\u5c5e\u6027\u4e0d\u80fd\u5efa\u7acbpair_unique\u7d22\u5f15"}),"\u3002"]}),"\n",(0,i.jsx)(e.h6,{id:"13123-non_unique\u7d22\u5f15",children:"1.3.1.2.3 non_unique\u7d22\u5f15"}),"\n",(0,i.jsxs)(e.p,{children:["\u548c\u70b9\u7c7b\u4f3c\uff0c\u8fb9\u7684non_unique\u7d22\u5f15\u6307\u7684\u662f\u975e\u5168\u5c40\u552f\u4e00\u7684\u7d22\u5f15\uff0c\u5373\u82e5\u4e00\u4e2a\u5c5e\u6027\u8bbe\u7f6e\u4e86non_unique\u7d22\u5f15\uff0c\n\u5728\u540c\u4e00\u4e2a\u56fe\u4e2d\uff0c\u76f8\u540clabel\u7684\u8fb9\u7684\u8be5\u5c5e\u6027\u53ef\u4ee5\u5b58\u5728\u76f8\u540c\u7684\u503c\u3002\n\u7531\u4e8enon_unique\u7d22\u5f15\u4e00\u4e2akey\u53ef\u80fd\u6620\u5c04\u5230\u591a\u4e2a\u503c\uff0c\u4e3a\u4e86\u52a0\u901f\u67e5\u627e\u548c\u5199\u5165\uff0c\n\u5728\u7528\u6237\u6307\u5b9a\u7684key\u540e\u9762\u52a0\u4e0a\u4e86\u7d22\u5f15key\u76f8\u540c\u7684\u4e00\u7ec4eid\u7684\u6700\u5927\u503c\u3002\n\u6bcf\u4e2aeid\u662f24bytes\u957f\u5ea6\uff0c\u56e0\u6b64non_unique\u7d22\u5f15key\u6700\u5927\u957f\u5ea6\u662f456bytes\u3002\n\u4f46\u662f\uff0c\u4e0d\u540c\u4e8eunique\u7d22\u5f15\uff0c\u8d85\u8fc7456bytes\u4e5f\u53ef\u4ee5\u5efa\u7acbnon_unique\u7d22\u5f15\u3002\n\u53ea\u4e0d\u8fc7\u5728\u5bf9\u8fd9\u6837\u7684\u5c5e\u6027\u5efa\u7acb\u7d22\u5f15\u65f6\u4f1a\u53ea\u622a\u53d6",(0,i.jsx)(e.strong,{children:"\u524d456bytes"}),"\u4f5c\u4e3a\u7d22\u5f15key\uff08\u5c5e\u6027\u672c\u8eab\u5b58\u50a8\u7684\u503c\u4e0d\u53d7\u5f71\u54cd\uff09\u3002\n\u5e76\u4e14\uff0c\u5728\u901a\u8fc7\u8fed\u4ee3\u5668\u904d\u5386\u65f6\uff0c\u4e5f\u662f\u5148\u81ea\u52a8\u622a\u53d6\u67e5\u8be2\u503c\u7684\u524d456bytes\u518d\u8fdb\u884c\u904d\u5386\uff0c\n\u6240\u4ee5\u7ed3\u679c\u53ef\u80fd\u548c\u9884\u671f\u4e0d\u4e00\u81f4\uff0c\u9700\u8981\u7528\u6237\u518d\u8fc7\u6ee4\u3002"]}),"\n",(0,i.jsx)(e.h4,{id:"132-\u7ec4\u5408\u7d22\u5f15",children:"1.3.2 \u7ec4\u5408\u7d22\u5f15"}),"\n",(0,i.jsx)(e.p,{children:"\u76ee\u524d\u53ea\u652f\u6301\u5bf9\u70b9\u7684\u591a\u4e2a\u5c5e\u6027\u5efa\u7acb\u7ec4\u5408\u7d22\u5f15\uff0c\u4e0d\u652f\u6301\u5bf9\u8fb9\u7684\u5c5e\u6027\u5efa\u7acb\u7ec4\u5408\u7d22\u5f15\u3002\u7ec4\u5408\u7d22\u5f15\u652f\u6301\u552f\u4e00\u7d22\u5f15\u548c\u975e\u552f\u4e00\u7d22\u5f15\u4e24\u79cd\u7c7b\u578b\uff0c\u5efa\u7acb\u7d22\u5f15\u7684\u8981\u6c42\u5982\u4e0b\uff1a"}),"\n",(0,i.jsxs)(e.ol,{children:["\n",(0,i.jsx)(e.li,{children:"\u5efa\u7acb\u7ec4\u5408\u7d22\u5f15\u7684\u5c5e\u6027\u4e2a\u6570\u57282\u523016\u4e2a\u4e4b\u95f4\uff08\u542b\uff09"}),"\n",(0,i.jsx)(e.li,{children:"\u552f\u4e00\u7ec4\u5408\u7d22\u5f15\u7684\u5c5e\u6027\u957f\u5ea6\u4e4b\u548c\u4e0d\u80fd\u8d85\u8fc7480-2*(\u5c5e\u6027\u4e2a\u6570-1)\u5b57\u8282\uff0c\u975e\u552f\u4e00\u7ec4\u5408\u7d22\u5f15\u7684\u5c5e\u6027\u957f\u5ea6\u4e4b\u548c\u4e0d\u80fd\u8d85\u8fc7475-2*(\u5c5e\u6027\u4e2a\u6570-1)\u5b57\u8282"}),"\n"]}),"\n",(0,i.jsx)(e.h5,{id:"1321-\u552f\u4e00\u7d22\u5f15",children:"1.3.2.1 \u552f\u4e00\u7d22\u5f15"}),"\n",(0,i.jsxs)(e.p,{children:["\u548c\u70b9\u7684\u666e\u901a\u552f\u4e00\u7d22\u5f15\u7c7b\u4f3c\uff0c\u70b9\u7684\u7ec4\u5408\u552f\u4e00\u7d22\u5f15\u6307\u7684\u662f\u5168\u5c40\u552f\u4e00\u7684\u7d22\u5f15\uff0c\u5373\u82e5\u4e00\u7ec4\u5c5e\u6027\u8bbe\u7f6e\u4e86unique\u7d22\u5f15\uff0c\n\u5728\u540c\u4e00\u4e2a\u56fe\u4e2d\uff0c\u76f8\u540clabel\u7684\u70b9\u7684\u8be5\u7ec4\u5c5e\u6027\u4e0d\u4f1a\u5b58\u5728\u76f8\u540c\u7684\u503c\u3002\n\u7531\u4e8e\u5e95\u5c42\u5b58\u50a8\u8bbe\u8ba1\uff0c\u7ec4\u5408\u7d22\u5f15key\u9700\u8981\u4fdd\u5b58\u5c5e\u6027\u7684\u957f\u5ea6\uff0c\u56e0\u6b64\uff0c\n\u7ec4\u5408\u552f\u4e00\u7d22\u5f15key\u7684\u6700\u5927\u957f\u5ea6\u662f480-2*(\u5c5e\u6027\u4e2a\u6570-1) bytes\uff0c",(0,i.jsx)(e.strong,{children:"\u8d85\u8fc7\u7684\u5c5e\u6027\u4e0d\u80fd\u5efa\u7acb\u552f\u4e00\u7d22\u5f15"}),"\u3002"]}),"\n",(0,i.jsx)(e.h5,{id:"1322-\u975e\u552f\u4e00\u7d22\u5f15",children:"1.3.2.2 \u975e\u552f\u4e00\u7d22\u5f15"}),"\n",(0,i.jsxs)(e.p,{children:["\u548c\u70b9\u7684\u666e\u901a\u975e\u552f\u4e00\u7d22\u5f15\u7c7b\u4f3c\uff0c\u70b9\u7684\u975e\u552f\u4e00\u7d22\u5f15\u6307\u7684\u662f\u975e\u5168\u5c40\u552f\u4e00\u7684\u7d22\u5f15\uff0c\u5373\u82e5\u4e00\u7ec4\u5c5e\u6027\u8bbe\u7f6e\u4e86\u975e\u552f\u4e00\u7d22\u5f15\uff0c\n\u5728\u540c\u4e00\u4e2a\u56fe\u4e2d\uff0c\u76f8\u540clabel\u7684\u70b9\u7684\u8be5\u7ec4\u5c5e\u6027\u53ef\u4ee5\u5b58\u5728\u76f8\u540c\u7684\u503c\u3002\n\u7531\u4e8e\u975e\u552f\u4e00\u7d22\u5f15\u4e00\u4e2akey\u53ef\u80fd\u6620\u5c04\u5230\u591a\u4e2a\u503c\uff0c\u4e3a\u4e86\u52a0\u901f\u67e5\u627e\u548c\u5199\u5165\uff0c\n\u5728\u7528\u6237\u6307\u5b9a\u7684key\u540e\u9762\u52a0\u4e0a\u4e86\u7d22\u5f15key\u76f8\u540c\u7684\u4e00\u7ec4vid\u7684\u6700\u5927\u503c\u3002\n\u6bcf\u4e2avid\u662f5bytes\u957f\u5ea6\uff0c\u56e0\u6b64non_unique\u7d22\u5f15key\u6700\u5927\u957f\u5ea6\u662f475-2*(\u5c5e\u6027\u4e2a\u6570-1) bytes\uff0c\n",(0,i.jsx)(e.strong,{children:"\u8d85\u8fc7\u7684\u5c5e\u6027\u4e0d\u80fd\u5efa\u7acb\u975e\u552f\u4e00\u7d22\u5f15"}),"\u3002"]}),"\n",(0,i.jsx)(e.h2,{id:"2-\u56fe\u9879\u76ee\u70b9\u8fb9\u5c5e\u6027\u547d\u540d\u89c4\u5219\u548c\u5efa\u8bae",children:"2. \u56fe\u9879\u76ee\u3001\u70b9\u3001\u8fb9\u3001\u5c5e\u6027\u547d\u540d\u89c4\u5219\u548c\u5efa\u8bae"}),"\n",(0,i.jsx)(e.h3,{id:"21-\u547d\u540d\u89c4\u5219",children:"2.1 \u547d\u540d\u89c4\u5219"}),"\n",(0,i.jsx)(e.p,{children:"\u56fe\u9879\u76ee\u3001\u70b9\u3001\u8fb9\u548c\u5c5e\u6027\u662f\u8bc6\u522b\u7b26\u3002\u8be5\u8282\u63cf\u8ff0\u4e86\u5728TuGraph\u4e2d\u8bc6\u522b\u7b26\u7684\u5141\u8bb8\u7684\u8bed\u6cd5\u3002\n\u4e0b\u9762\u7684\u8868\u63cf\u8ff0\u4e86\u6bcf\u7c7b\u8bc6\u522b\u7b26\u7684\u6700\u5927\u957f\u5ea6\u548c\u5141\u8bb8\u7684\u5b57\u7b26\u3002"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,i.jsxs)(e.table,{children:[(0,i.jsx)(e.thead,{children:(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.th,{children:(0,i.jsx)(e.strong,{children:"\u8bc6\u522b\u7b26"})}),(0,i.jsx)(e.th,{children:(0,i.jsx)(e.strong,{children:"\u957f\u5ea6"})}),(0,i.jsx)(e.th,{children:(0,i.jsx)(e.strong,{children:"\u5141\u8bb8\u7684\u5b57\u7b26"})})]})}),(0,i.jsxs)(e.tbody,{children:[(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"\u7528\u6237\u3001\u89d2\u8272\u3001\u56fe\u9879\u76ee"}),(0,i.jsx)(e.td,{children:"1-64\u5b57\u7b26"}),(0,i.jsx)(e.td,{children:"\u5141\u8bb8\u4e2d\u6587\u3001\u5b57\u6bcd\u3001\u6570\u5b57\u3001\u4e0b\u5212\u7ebf\uff0c\u4e14\u9996\u5b57\u7b26\u4e0d\u4e3a\u6570\u5b57"})]}),(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"\u70b9\u7c7b\u578b\u3001\u8fb9\u7c7b\u578b\u3001\u5c5e\u6027"}),(0,i.jsx)(e.td,{children:"1~256\u5b57\u7b26"}),(0,i.jsx)(e.td,{children:"\u5141\u8bb8\u4e2d\u6587\u3001\u5b57\u6bcd\u3001\u6570\u5b57\u3001\u4e0b\u5212\u7ebf\uff0c\u4e14\u9996\u5b57\u7b26\u4e0d\u4e3a\u6570\u5b57"})]})]})]}),"\n",(0,i.jsx)(e.h3,{id:"22-\u4f7f\u7528\u9650\u5236",children:"2.2 \u4f7f\u7528\u9650\u5236"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,i.jsxs)(e.table,{children:[(0,i.jsx)(e.thead,{children:(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.th,{children:(0,i.jsx)(e.strong,{children:"\u63cf\u8ff0"})}),(0,i.jsx)(e.th,{children:(0,i.jsx)(e.strong,{children:"\u6700\u5927\u4e2a\u6570"})})]})}),(0,i.jsxs)(e.tbody,{children:[(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"\u7528\u6237\u6570\u3001\u89d2\u8272\u6570"}),(0,i.jsx)(e.td,{children:"65536"})]}),(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"\u56fe\u9879\u76ee\u7684\u4e2a\u6570"}),(0,i.jsx)(e.td,{children:"4096"})]}),(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"\u6bcf\u4e2a\u56fe\u9879\u76ee\u7684\u70b9\u548c\u8fb9\u7c7b\u578b\u6570\u91cf\u4e4b\u548c"}),(0,i.jsx)(e.td,{children:"4096"})]}),(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"\u6bcf\u4e2a\u70b9\u6216\u8fb9\u7c7b\u578b\u7684\u5c5e\u6027\u6570\u91cf"}),(0,i.jsx)(e.td,{children:"1024"})]})]})]}),"\n",(0,i.jsx)(e.p,{children:"\u6ce8\uff1a\n1\u3001\u7279\u6b8a\u5b57\u7b26\u548c\u5173\u952e\u5b57\u8bf4\u660e\uff1a\u4f7f\u7528\u7279\u6b8a\u5b57\u7b26\u6216\u975e\u4fdd\u7559\u5173\u952e\u5b57\u65f6\uff0c\u9700\u8981\u4f7f\u7528\u53cd\u5355\u5f15\u53f7/backquote\uff08``\uff09\u8fdb\u884c\u5f15\u7528\uff1b"}),"\n",(0,i.jsxs)(e.p,{children:["\u793a\u4f8b\uff1a ",(0,i.jsx)(e.code,{children:"match (`match`:match) return `match`.id limit 1"})]}),"\n",(0,i.jsx)(e.p,{children:"2\u3001\u5927\u5c0f\u5199\u654f\u611f\u6027\uff1aTuGraph\u5927\u5c0f\u5199\u654f\u611f\uff1b"}),"\n",(0,i.jsx)(e.p,{children:"3\u3001\u56fe\u9879\u76ee\u3001\u70b9/\u8fb9\u3001\u5c5e\u6027\u540d\u79f0\u4e4b\u95f4\u53ef\u4ee5\u91cd\u590d\u4f7f\u7528\uff0c\u540c\u4e00\u70b9\u6216\u8fb9\u4e0b\u7684\u5c5e\u6027\u540d\u79f0\u4e0d\u53ef\u4ee5\u91cd\u590d\uff1b"}),"\n",(0,i.jsx)(e.p,{children:"4\u3001\u5c5e\u6027\u540d\u5b57\u4fdd\u7559\u5173\u952e\u5b57\uff1aSRC_ID / DST_ID / SKIP"}),"\n",(0,i.jsx)(e.h3,{id:"23-\u547d\u540d\u5efa\u8bae",children:"2.3 \u547d\u540d\u5efa\u8bae"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,i.jsxs)(e.table,{children:[(0,i.jsx)(e.thead,{children:(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.th,{children:(0,i.jsx)(e.strong,{children:"\u8bc6\u522b\u7b26"})}),(0,i.jsx)(e.th,{children:(0,i.jsx)(e.strong,{children:"\u63cf\u8ff0"})}),(0,i.jsx)(e.th,{children:(0,i.jsx)(e.strong,{children:"\u5efa\u8bae"})})]})}),(0,i.jsxs)(e.tbody,{children:[(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"\u56fe\u9879\u76ee"}),(0,i.jsx)(e.td,{children:"\u5b57\u6bcd\u6216\u4e2d\u6587\u5f00\u5934"}),(0,i.jsx)(e.td,{children:"\u5982graph123\u3001project123\u7b49"})]}),(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"\u70b9/\u8fb9\u7c7b\u578b"}),(0,i.jsx)(e.td,{children:"\u5b57\u6bcd\u6216\u4e2d\u6587\u5f00\u5934\uff0c\u4f7f\u7528\u4e0b\u5212\u7ebf\u533a\u5206\u5355\u8bcd"}),(0,i.jsx)(e.td,{children:"\u5982person\u3001act_in\u7b49"})]}),(0,i.jsxs)(e.tr,{children:[(0,i.jsx)(e.td,{children:"\u5c5e\u6027"}),(0,i.jsx)(e.td,{children:"\u5b57\u6bcd\u6216\u4e2d\u6587"}),(0,i.jsx)(e.td,{children:"\u5982name\u3001age\u7b49"})]})]})]})]})}function j(n={}){const{wrapper:e}={...(0,s.R)(),...n.components};return e?(0,i.jsx)(e,{...n,children:(0,i.jsx)(x,{...n})}):x(n)}}}]);