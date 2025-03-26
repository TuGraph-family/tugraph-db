"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[13218],{106:(e,n,i)=>{i.d(n,{A:()=>s});const s=i.p+"assets/images/WKB-fe5482c2e6681a0467a03cbf55761578.png"},24813:(e,n,i)=>{i.d(n,{A:()=>s});const s=i.p+"assets/images/EPSG_4326-ebcf508237a6a659deda1c1c05da731b.png"},24870:(e,n,i)=>{i.d(n,{A:()=>s});const s=i.p+"assets/images/EPSG_7203-813d52c83637ec9bff32110935eb851d.png"},28453:(e,n,i)=>{i.d(n,{R:()=>r,x:()=>l});var s=i(96540);const d={},t=s.createContext(d);function r(e){const n=s.useContext(t);return s.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function l(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(d):e.components||d:r(e.components),s.createElement(t.Provider,{value:n},e.children)}},38482:(e,n,i)=>{i.d(n,{A:()=>s});const s=i.p+"assets/images/createVertexLabel-6b5834819d1f4d20958b9ba6f13ebbde.png"},39726:(e,n,i)=>{i.d(n,{A:()=>s});const s=i.p+"assets/images/createLineTestData-d6b3f97eeae6bc43b1db36b3ac1536d4.png"},43428:(e,n,i)=>{i.d(n,{A:()=>s});const s=i.p+"assets/images/createFoodData-42a1476e438f5b07b017a493510a074f.png"},74273:(e,n,i)=>{i.d(n,{A:()=>s});const s=i.p+"assets/images/createVertexLabel_lineTest-b37d40db2f4c254af64535f30a2842c5.png"},79564:(e,n,i)=>{i.d(n,{A:()=>s});const s=i.p+"assets/images/querryFood-bd53767c9a6b584a1ba73dae4f429efb.png"},85496:(e,n,i)=>{i.r(n),i.d(n,{assets:()=>c,contentTitle:()=>r,default:()=>o,frontMatter:()=>t,metadata:()=>l,toc:()=>a});var s=i(74848),d=i(28453);const t={},r="\u5730\u7406\u7a7a\u95f4\u6570\u636e\u7c7b\u578b\u4f7f\u7528\u793a\u4f8b",l={id:"best-practices/spatial",title:"\u5730\u7406\u7a7a\u95f4\u6570\u636e\u7c7b\u578b\u4f7f\u7528\u793a\u4f8b",description:"1. \u7b80\u4ecb",source:"@site/versions/version-4.3.2/zh-CN/source/13.best-practices/5.spatial.md",sourceDirName:"13.best-practices",slug:"/best-practices/spatial",permalink:"/tugraph-db/zh/4.3.2/best-practices/spatial",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:5,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"\u73af\u5883\u548c\u7248\u672c\u9009\u62e9",permalink:"/tugraph-db/zh/4.3.2/best-practices/selection"},next:{title:"FAQ",permalink:"/tugraph-db/zh/4.3.2/faq"}},c={},a=[{value:"1. \u7b80\u4ecb",id:"1-\u7b80\u4ecb",level:2},{value:"2. \u9884\u5907\u77e5\u8bc6",id:"2-\u9884\u5907\u77e5\u8bc6",level:2},{value:"2.1 WGS84\u5750\u6807\u7cfb EPSG: 4326",id:"21-wgs84\u5750\u6807\u7cfb-epsg-4326",level:3},{value:"2.2 Cartesian(\u7b1b\u5361\u5c14)\u5750\u6807\u7cfb EPSG: 7203",id:"22-cartesian\u7b1b\u5361\u5c14\u5750\u6807\u7cfb-epsg-7203",level:3},{value:"2.3 \u6570\u636e\u5b58\u50a8\u683c\u5f0f",id:"23-\u6570\u636e\u5b58\u50a8\u683c\u5f0f",level:3},{value:"2.4 \u5e38\u7528\u51fd\u6570",id:"24-\u5e38\u7528\u51fd\u6570",level:3},{value:"3. \u6570\u636e\u7c7b\u578b",id:"3-\u6570\u636e\u7c7b\u578b",level:2},{value:"4. \u51fd\u6570\u4ecb\u7ecd",id:"4-\u51fd\u6570\u4ecb\u7ecd",level:2},{value:"5. \u7f8e\u98df\u63a2\u7d22",id:"5-\u7f8e\u98df\u63a2\u7d22",level:2},{value:"5.1 \u57fa\u4e8e\u5730\u7406\u4f4d\u7f6e\u7684\u4e2a\u6027\u5316\u63a8\u8350",id:"51-\u57fa\u4e8e\u5730\u7406\u4f4d\u7f6e\u7684\u4e2a\u6027\u5316\u63a8\u8350",level:3},{value:"5.2 \u6570\u636e\u6a21\u578b\u8bbe\u8ba1",id:"52-\u6570\u636e\u6a21\u578b\u8bbe\u8ba1",level:3},{value:"5.3 \u6784\u5efa\u7f8e\u98df\u63a2\u7d22\u67e5\u8be2",id:"53-\u6784\u5efa\u7f8e\u98df\u63a2\u7d22\u67e5\u8be2",level:3},{value:"6. \u5c55\u671b",id:"6-\u5c55\u671b",level:2}];function h(e){const n={a:"a",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",img:"img",li:"li",p:"p",pre:"pre",strong:"strong",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",ul:"ul",...(0,d.R)(),...e.components};return(0,s.jsxs)(s.Fragment,{children:[(0,s.jsx)(n.header,{children:(0,s.jsx)(n.h1,{id:"\u5730\u7406\u7a7a\u95f4\u6570\u636e\u7c7b\u578b\u4f7f\u7528\u793a\u4f8b",children:"\u5730\u7406\u7a7a\u95f4\u6570\u636e\u7c7b\u578b\u4f7f\u7528\u793a\u4f8b"})}),"\n",(0,s.jsx)(n.h2,{id:"1-\u7b80\u4ecb",children:"1. \u7b80\u4ecb"}),"\n",(0,s.jsx)(n.p,{children:"TuGraph\u56fe\u6570\u636e\u5e93\u7531\u8682\u8681\u96c6\u56e2\u4e0e\u6e05\u534e\u5927\u5b66\u8054\u5408\u7814\u53d1\uff0c\u6784\u5efa\u4e86\u4e00\u5957\u5305\u542b\u56fe\u5b58\u50a8\u3001\u56fe\u8ba1\u7b97\u3001\u56fe\u5b66\u4e60\u3001\u56fe\u7814\u53d1\u5e73\u53f0\u7684\u5b8c\u5584\u7684\u56fe\u6280\u672f\u4f53\u7cfb\uff0c\u62e5\u6709\u4e1a\u754c\u9886\u5148\u89c4\u6a21\u7684\u56fe\u96c6\u7fa4\u3002\u8fd1\u5e74\u6765\uff0c\u5730\u7406\u7a7a\u95f4\u529f\u80fd\u5728\u56fe\u6570\u636e\u5e93\u4e2d\u7684\u5e94\u7528\u4ef7\u503c\u663e\u8457\uff0c\u5b83\u4e0d\u4ec5\u589e\u5f3a\u4e86\u6570\u636e\u7684\u8868\u8fbe\u80fd\u529b\uff0c\u8fd8\u4fc3\u8fdb\u4e86\u8de8\u9886\u57df\u6570\u636e\u7684\u878d\u5408\u5206\u6790\uff0c\u5c24\u5176\u5728\u793e\u4ea4\u7f51\u7edc\u3001\u5730\u56fe\u63a2\u7d22\u3001\u57ce\u5e02\u89c4\u5212\u7b49\u5173\u952e\u9886\u57df\u5c55\u73b0\u4e86\u5f3a\u5927\u7684\u5b9e\u7528\u4ef7\u503c\u3002TuGraph\u4e5f\u6b63\u5728\u9010\u6b65\u652f\u6301\u5730\u7406\u7a7a\u95f4\u529f\u80fd\u3002"}),"\n",(0,s.jsx)(n.h2,{id:"2-\u9884\u5907\u77e5\u8bc6",children:"2. \u9884\u5907\u77e5\u8bc6"}),"\n",(0,s.jsxs)(n.p,{children:["EPSG(",(0,s.jsx)(n.a,{href:"https://epsg.io/",children:"EPSG.io: Coordinate Systems Worldwide"}),") \u662f\u4e00\u4e2a\u6807\u51c6\u5316\u7684\u5730\u7406\u7a7a\u95f4\u53c2\u8003\u7cfb\u7edf\u6807\u8bc6\u7b26\u96c6\u5408\uff0c \u7528\u4e8e\u6807\u8bc6\u4e0d\u540c\u7684\u5730\u7406\u7a7a\u95f4\u53c2\u8003\u7cfb\u7edf\uff0c\u5305\u62ec\u5750\u6807\u7cfb\u7edf\u3001\u5730\u7406\u5750\u6807\u7cfb\u3001\u6295\u5f71\u5750\u6807\u7cfb\u7b49\u3002\u6211\u4eec\u5e38\u7528EPSG\u7f16\u7801\u8868\u793a\u6570\u636e\u7684\u5750\u6807\u7cfb\uff0c\u8fd9\u91cc\u6211\u4eec\u4ecb\u7ecd\u4e24\u79cd\u6700\u5e38\u89c1\u7684\u7a7a\u95f4\u5730\u7406\u5750\u6807\u7cfb\uff0c\u4e5f\u662f\u5927\u90e8\u5206\u6570\u636e\u5e93\u652f\u6301\u7684\u5750\u6807\u7cfb\u7c7b\u578b\u3002"]}),"\n",(0,s.jsx)(n.h3,{id:"21-wgs84\u5750\u6807\u7cfb-epsg-4326",children:"2.1 WGS84\u5750\u6807\u7cfb EPSG: 4326"}),"\n",(0,s.jsxs)(n.p,{children:["\u5168\u7403GPS\u5b9a\u4f4d\u7cfb\u7edf: WGS84\u662f\u5168\u7403\u5b9a\u4f4d\u7cfb\u7edf(GPS)\u7684\u57fa\u7840\uff0c\u5141\u8bb8\u5168\u7403\u7684GPS\u63a5\u6536\u5668\u786e\u5b9a\u7cbe\u786e\u4f4d\u7f6e\u3002\u51e0\u4e4e\u6240\u6709\u73b0\u4ee3GPS\u8bbe\u5907\u90fd\u662f\u57fa\u4e8eWGS84\u5750\u6807\u7cfb\u6765\u63d0\u4f9b\u4f4d\u7f6e\u4fe1\u606f\u3002\u5730\u56fe\u5236\u4f5c\u548c\u5730\u7406\u4fe1\u606f\u7cfb\u7edf(GIS): \u5728\u5730\u56fe\u5236\u4f5c\u548cGIS\u9886\u57df\uff0cWGS84\u88ab\u5e7f\u6cdb\u7528\u4e8e\u5b9a\u4e49\u5730\u7403\u4e0a\u7684\u4f4d\u7f6e\u3002\u8fd9\u5305\u62ec\u5404\u79cd\u7c7b\u578b\u7684\u5730\u56fe\u521b\u5efa\u3001\u7a7a\u95f4\u6570\u636e\u5206\u6790\u548c\u7ba1\u7406\u7b49\u3002",(0,s.jsx)(n.img,{alt:"image.png",src:i(24813).A+"",width:"821",height:"390"})]}),"\n",(0,s.jsx)(n.h3,{id:"22-cartesian\u7b1b\u5361\u5c14\u5750\u6807\u7cfb-epsg-7203",children:"2.2 Cartesian(\u7b1b\u5361\u5c14)\u5750\u6807\u7cfb EPSG: 7203"}),"\n",(0,s.jsxs)(n.p,{children:["\u76f4\u89d2\u5750\u6807\u7cfb\uff0c\u662f\u4e00\u79cd\u6700\u57fa\u672c\u3001\u6700\u5e7f\u6cdb\u5e94\u7528\u7684\u5750\u6807\u7cfb\u7edf\u3002\u5b83\u901a\u8fc7\u4e24\u6761\u6570\u8f74\u5b9a\u4e49\u4e00\u4e2a\u5e73\u9762\uff0c\u4e09\u6761\u6570\u8f74\u5b9a\u4e49\u4e00\u4e2a\u7a7a\u95f4\uff0c\u8fd9\u4e9b\u8f74\u4e92\u76f8\u5782\u76f4\uff0c\u5728\u6570\u5b66\u3001\u7269\u7406\u3001\u5de5\u7a0b\u3001\u5929\u6587\u548c\u8bb8\u591a\u5176\u4ed6\u9886\u57df\u4e2d\u6709\u7740\u5e7f\u6cdb\u7684\u5e94\u7528\u3002",(0,s.jsx)(n.img,{alt:"image.png",src:i(24870).A+"",width:"560",height:"560"})]}),"\n",(0,s.jsx)(n.h3,{id:"23-\u6570\u636e\u5b58\u50a8\u683c\u5f0f",children:"2.3 \u6570\u636e\u5b58\u50a8\u683c\u5f0f"}),"\n",(0,s.jsx)(n.p,{children:"OGC(Open Geospatial Consortium) \u5b9a\u4e49\u4e86\u7a7a\u95f4\u6570\u636e\u7684\u6807\u51c6\u8868\u793a\u683c\u5f0f\uff0c\u5206\u522b\u4e3aWKT\u4e0eWKB\u683c\u5f0f\uff0c\u7528\u4e8e\u5728\u4e0d\u540c\u7cfb\u7edf\u548c\u5e73\u53f0\u4e4b\u95f4\u4ea4\u6362\u548c\u5b58\u50a8\u7a7a\u95f4\u6570\u636e\uff0c\u73b0\u5df2\u88ab\u5e7f\u6cdb\u91c7\u7528\u3002\u5176\u4e2dWKT(well-kown text)\u683c\u5f0f, \u662f\u4e00\u79cd\u6587\u672c\u6807\u8bb0\u8bed\u8a00,\u6613\u4e8e\u4eba\u7c7b\u9605\u8bfb\u548c\u7f16\u5199\uff0c\u800cWKB(Well-Known Binary)\u683c\u5f0f\u91c7\u7528\u4e00\u7cfb\u5217\u5b57\u8282\u6765\u7f16\u7801\u7a7a\u95f4\u6570\u636e\uff0c\u66f4\u9002\u5408\u5728\u8ba1\u7b97\u673a\u4e2d\u5b58\u50a8;"}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.strong,{children:"WKT:"})}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:"POINT(<x> <y>)\nLINESTRING(<x1> <y1>, <x2><y2>, ...)\n"})}),"\n",(0,s.jsx)(n.p,{children:"WKT\u683c\u5f0f\u7684\u6570\u636e\u5982\u4e0a\u4f8b\u6240\u793a\uff0c\u5148\u6307\u5b9a\u7a7a\u95f4\u6570\u636e\u7c7b\u578b\uff0c\u518d\u5728\u62ec\u53f7\u5185\u6307\u5b9a\u5177\u4f53\u7684\u5750\u6807\uff0c\u4e00\u4e2a\u5750\u6807\u5bf9\u8868\u793a\u4e00\u4e2a\u70b9\uff0c\u6bcf\u4e2a\u5750\u6807\u5bf9\u4e4b\u95f4\u7528\u9017\u53f7\u9694\u5f00\u3002\u5bf9\u4e8ePolygon\u7c7b\u578b\u7684\u6570\u636e\uff0c\u7b2c\u4e00\u4e2a\u5750\u6807\u5bf9\u9700\u8981\u4e0e\u6700\u540e\u4e00\u4e2a\u5750\u6807\u5bf9\u76f8\u540c\uff0c\u5f62\u6210\u95ed\u5408\u7684\u9762\u3002"}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.strong,{children:"WKB:"})}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.img,{alt:"image.png",src:i(106).A+"",width:"858",height:"264"})}),"\n",(0,s.jsx)(n.p,{children:"\u9488\u5bf9EWKB\u683c\u5f0f\u7684\u7f16\u7801\uff0c\u8bf4\u660e\u5982\u4e0b:"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"\u7b2c0 - 1\u4f4d: \u7f16\u7801\u65b9\u5f0f;"}),"\n",(0,s.jsxs)(n.li,{children:["\u7b2c2 - 5\u4f4d: \u7a7a\u95f4\u6570\u636e\u7c7b\u578b;\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"0100: point"}),"\n",(0,s.jsx)(n.li,{children:"0200: linestring"}),"\n",(0,s.jsx)(n.li,{children:"0300: polygon"}),"\n"]}),"\n"]}),"\n",(0,s.jsxs)(n.li,{children:["\u7b2c6 - 9\u4f4d: \u6570\u636e\u7ef4\u5ea6;\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"0020: \u4e8c\u7ef4"}),"\n",(0,s.jsx)(n.li,{children:"0030: \u4e09\u7ef4"}),"\n"]}),"\n"]}),"\n",(0,s.jsx)(n.li,{children:"\u7b2c10 - 17\u4f4d: \u5750\u6807\u7cfb\u7684EPSG\u7f16\u7801;"}),"\n",(0,s.jsx)(n.li,{children:"\u7b2c18 - n\u4f4d: double\u7c7b\u578b\u7684\u5750\u6807\u5bf9\u768416\u8fdb\u5236\u8868\u793a\u3002"}),"\n"]}),"\n",(0,s.jsx)(n.h3,{id:"24-\u5e38\u7528\u51fd\u6570",children:"2.4 \u5e38\u7528\u51fd\u6570"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"Name"}),(0,s.jsx)(n.th,{children:"Description"}),(0,s.jsx)(n.th,{children:"Signature"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:(0,s.jsx)(n.code,{children:"dbms.graph.createGraph"})}),(0,s.jsx)(n.td,{children:"\u521b\u5efa\u5b50\u56fe"}),(0,s.jsx)(n.td,{children:(0,s.jsx)(n.code,{children:"dbms.graph.createGraph(graph_name::STRING, description::STRING, max_size_GB::INTEGER) :: (::VOID)"})})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:(0,s.jsx)(n.code,{children:"db.createVertexLabel"})}),(0,s.jsx)(n.td,{children:"\u521b\u5efaVertex Label"}),(0,s.jsx)(n.td,{children:(0,s.jsx)(n.code,{children:"db.createVertexLabel(label_name::STRING,field_specs::LIST) :: (::VOID)"})})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:(0,s.jsx)(n.code,{children:"db.getLabelSchema"})}),(0,s.jsx)(n.td,{children:"\u5217\u51falabel schema"}),(0,s.jsx)(n.td,{children:(0,s.jsx)(n.code,{children:"db.getLabelSchema(label_type::STRING,label_name::STRING) :: (name::STRING,type::STRING,optional::BOOLEAN)"})})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:(0,s.jsx)(n.code,{children:"db.deleteLabel"})}),(0,s.jsx)(n.td,{children:"\u5220\u9664Vertex"}),(0,s.jsx)(n.td,{children:(0,s.jsx)(n.code,{children:"db.deleteLabel(label_type::STRING,label_name::STRING) :: (::VOID)"})})]})]})]}),"\n",(0,s.jsxs)(n.p,{children:["\u66f4\u5b8c\u6574\u8be6\u7ec6\u7684\u51fd\u6570\u4f7f\u7528\u4ee5\u53ca\u63d2\u5165\u6570\u636e\u7684\u8bed\u53e5\uff0c\u53ef\u4ee5\u53c2\u8003 ",(0,s.jsx)(n.a,{href:"/tugraph-db/zh/4.3.2/query/cypher",children:"Cypher API"})]}),"\n",(0,s.jsx)(n.h2,{id:"3-\u6570\u636e\u7c7b\u578b",children:"3. \u6570\u636e\u7c7b\u578b"}),"\n",(0,s.jsx)(n.p,{children:"\u76ee\u524d\u5728TuGraph\u4e2d\uff0c\u6211\u4eec\u5df2\u7ecf\u652f\u6301\u4e86Point, Linestring\u4e0ePolygon\u4e09\u79cd\u7c7b\u578b:"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"Point\uff1a\u70b9    point(2.0, 2.0, 7203)"}),"\n",(0,s.jsx)(n.li,{children:"Linestring\uff1a\u6298\u7ebf LINESTRING(0 2,1 1,2 0)"}),"\n",(0,s.jsx)(n.li,{children:"Polygon\uff1a\u591a\u8fb9\u5f62  POLYGON((0 0,0 7,4 2,2 0,0 0))"}),"\n"]}),"\n",(0,s.jsx)(n.p,{children:"\u5176\u4e2d\u5750\u6807\u70b9\u90fd\u662fdouble\u578b\uff0c\u521b\u5efa\u56fe\u6a21\u578b\u548c\u63d2\u5165\u6570\u636e\u793a\u4f8b\u5982\u4e0b\uff1a"}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.strong,{children:"\u521b\u5efa\u6807\u8bb0\u7f8e\u98df\u4f4d\u7f6e\u7684\u70b9\u6a21\u578b"})}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:"CALL db.createVertexLabel('food', 'id', 'id', int64, false, 'name', string, true,'pointTest',point,true) \n"})}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.img,{alt:"image.png",src:i(38482).A+"",width:"932",height:"322"})}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.strong,{children:"\u63d2\u5165\u6807\u8bb0\u7f8e\u98df\u70b9\u7684\u6570\u636e"})}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:" CREATE (n:food {id:10001, name: 'aco Bell',pointTest:point(3.0,4.0,7203)}) RETURN n\n"})}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.img,{alt:"image.png",src:i(43428).A+"",width:"1112",height:"706"})}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.strong,{children:"\u521b\u5efa\u5177\u6709\u6298\u7ebf\u5c5e\u6027\u7684\u70b9\u6a21\u578b"})}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:"CALL db.createVertexLabel('lineTest', 'id', 'id', int64, false, 'name', string, true,'linestringTest',linestring,true)\n"})}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.img,{alt:"image.png",src:i(74273).A+"",width:"953",height:"432"})}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.strong,{children:"\u63d2\u5165\u5177\u6709\u6298\u7ebf\u5c5e\u6027\u7684\u70b9\u6570\u636e"})}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:"CREATE (n:lineTest {id:102, name: 'Tom',linestringTest:linestringwkt('LINESTRING(0 2,1 1,2 0)', 7203)}) RETURN n\n"})}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.img,{alt:"image.png",src:i(39726).A+"",width:"1777",height:"854"})}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.strong,{children:"\u521b\u5efa\u5177\u6709\u591a\u8fb9\u578b\u5c5e\u6027\u7684\u70b9\u6a21\u578b"})}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:"CALL db.createVertexLabel('polygonTest', 'id', 'id', int64, false, 'name', string, true,'polygonTest',polygon,true)\n"})}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.img,{alt:"image.png",src:i(94037).A+"",width:"922",height:"389"})}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.strong,{children:"\u63d2\u5165\u5177\u6709\u591a\u8fb9\u578b\u5c5e\u6027\u7684\u70b9\u6570\u636e"})}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:"CREATE (n:polygonTest {id:103, name: 'polygonTest',polygonTest:polygonwkt('POLYGON((0 0,0 7,4 2,2 0,0 0))', 7203)}) RETURN n\n"})}),"\n",(0,s.jsx)(n.h2,{id:"4-\u51fd\u6570\u4ecb\u7ecd",children:"4. \u51fd\u6570\u4ecb\u7ecd"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"\u51fd\u6570\u540d"}),(0,s.jsx)(n.th,{children:"\u63cf\u8ff0"}),(0,s.jsx)(n.th,{children:"\u8f93\u5165\u53c2\u6570"}),(0,s.jsx)(n.th,{children:"\u8fd4\u56de\u503c\u7c7b\u578b"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"Distance()"}),(0,s.jsx)(n.td,{children:"\u8ba1\u7b97\u4e24\u4e2a\u7a7a\u95f4\u6570\u636e\u95f4\u7684\u8ddd\u79bb(\u8981\u6c42\u5750\u6807\u7cfb\u76f8\u540c)"}),(0,s.jsx)(n.td,{children:"Spatial data1, Spatial data2"}),(0,s.jsx)(n.td,{children:"double"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"Disjoint()"}),(0,s.jsx)(n.td,{children:"\u5224\u65ad\u4e24\u4e2a\u7a7a\u95f4\u6570\u636e\u662f\u5426\u76f8\u4ea4\uff08\u5f00\u53d1\u4e2d\uff09"}),(0,s.jsx)(n.td,{children:"Spatial data1, Spatial data2"}),(0,s.jsx)(n.td,{children:"bool"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"WithinBBox()"}),(0,s.jsx)(n.td,{children:"\u5224\u65ad\u67d0\u4e2a\u7a7a\u95f4\u6570\u636e\u662f\u5426\u5728\u7ed9\u5b9a\u7684\u957f\u65b9\u5f62\u533a\u57df\u5185\uff08\u5f00\u53d1\u4e2d\uff09"}),(0,s.jsx)(n.td,{children:"Spatial data, Point1"}),(0,s.jsx)(n.td,{children:"bool"})]})]})]}),"\n",(0,s.jsx)(n.h2,{id:"5-\u7f8e\u98df\u63a2\u7d22",children:"5. \u7f8e\u98df\u63a2\u7d22"}),"\n",(0,s.jsx)(n.h3,{id:"51-\u57fa\u4e8e\u5730\u7406\u4f4d\u7f6e\u7684\u4e2a\u6027\u5316\u63a8\u8350",children:"5.1 \u57fa\u4e8e\u5730\u7406\u4f4d\u7f6e\u7684\u4e2a\u6027\u5316\u63a8\u8350"}),"\n",(0,s.jsx)(n.p,{children:"\u5728\u672c\u7ae0\u8282\u4e2d\uff0c\u6211\u4eec\u5c06\u63a2\u7d22\u5982\u4f55\u5229\u7528Tugraph\u56fe\u6570\u636e\u5e93\u7684\u5730\u7406\u7a7a\u95f4\u529f\u80fd\uff0c\u6784\u5efa\u4e00\u4e2a\u751f\u52a8\u6709\u8da3\u7684\u7f8e\u98df\u63a2\u7d22\u5e94\u7528\uff0c\u5c06\u201c\u4eba\u201d\u4e0e\u201c\u7f8e\u98df\u201d\u901a\u8fc7\u5730\u7406\u4f4d\u7f6e\u7d27\u5bc6\u76f8\u8fde\uff0c\u5b9e\u73b0\u4e2a\u6027\u5316\u7f8e\u98df\u63a8\u8350\u3002\u60f3\u8c61\u4e00\u4e0b\uff0c\u65e0\u8bba\u8eab\u5904\u4f55\u65b9\uff0c\u53ea\u9700\u8f7b\u8f7b\u4e00\u70b9\uff0c\u5468\u56f4\u8bf1\u4eba\u7f8e\u98df\u4fbf\u5c3d\u6536\u773c\u5e95\uff0c\u8fd9\u6b63\u662f\u6211\u4eec\u5373\u5c06\u6784\u5efa\u7684\u573a\u666f\u9b45\u529b\u6240\u5728\u3002"}),"\n",(0,s.jsx)(n.h3,{id:"52-\u6570\u636e\u6a21\u578b\u8bbe\u8ba1",children:"5.2 \u6570\u636e\u6a21\u578b\u8bbe\u8ba1"}),"\n",(0,s.jsx)(n.p,{children:"\u6211\u4eec\u9996\u5148\u5b9a\u4e49\u4e24\u79cd\u6838\u5fc3\u8282\u70b9\u7c7b\u578b\uff1a"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"Food\uff08\u7f8e\u98df\uff09\u8282\u70b9\uff1a\u6bcf\u4e00\u5bb6\u9910\u5385\u6216\u5c0f\u5403\u5e97\u90fd\u53ef\u4ee5\u4f5c\u4e3a\u4e00\u4e2aFood\u8282\u70b9\uff0c\u5176\u5c5e\u6027\u5305\u62ec\u4f46\u4e0d\u9650\u4e8e\u540d\u79f0\u3001\u5730\u5740\u3001\u8bc4\u5206\u3001\u7f8e\u98df\u7c7b\u522b\u7b49\u3002\u7279\u522b\u5730\uff0c\u6211\u4eec\u5c06\u5728\u6bcf\u4e2aFood\u8282\u70b9\u4e0a\u9644\u52a0\u5730\u7406\u5750\u6807\u4fe1\u606f\uff0c\u7528\u4ee5\u7cbe\u786e\u8bb0\u5f55\u5176\u5730\u7406\u4f4d\u7f6e\u3002"}),"\n"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:" CALL db.createVertexLabel('food', 'id', 'id', int64, false, 'name', string, true,'pointTest',point,true,'mark',double,true)\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u51c6\u5907\u6570\u636e\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:"CREATE (n:food {id:10001, name: 'Starbucks',pointTest:point(1.0,1.0,7203),mark:4.8}) RETURN n\nCREATE (n:food {id:10002, name: 'KFC',pointTest:point(2.0,1.0,7203),mark:4.5}) RETURN n\nCREATE (n:food {id:10003, name: 'Pizza Hut',pointTest:point(2.0,5.0,7203),mark:4.5}) RETURN n\nCREATE (n:food {id:10004, name: 'Taco Bell',pointTest:point(3.0,4.0,7203),mark:4.7}) RETURN n\nCREATE (n:food {id:10005, name: 'Pizza Fusion',pointTest:point(5.0,3.0,7203),mark:4.9}) RETURN n\nCREATE (n:food {id:10006, name: 'HaiDiLao Hot Pot',pointTest:point(2.0,2.0,7203),mark:4.8}) RETURN n\nCREATE (n:food {id:10007, name: 'Lao Sze Chuan',pointTest:point(4.0,3.0,7203),mark:4.7}) RETURN n\n"})}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"Person\uff08\u4eba\u7269\uff09\u8282\u70b9\uff1a\u4ee3\u8868\u5e94\u7528\u7684\u7528\u6237\uff0c\u5c5e\u6027\u5305\u542b\u7528\u6237\u540d\u3001\u5f53\u524d\u4f4d\u7f6e\u7b49\u3002\u7528\u6237\u7684\u5f53\u524d\u4f4d\u7f6e\u540c\u6837\u901a\u8fc7\u5730\u7406\u5750\u6807\u8868\u793a\uff0c\u4fbf\u4e8e\u540e\u7eed\u7684\u5730\u7406\u7a7a\u95f4\u67e5\u8be2\u3002"}),"\n"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:" CALL db.createVertexLabel('person', 'id', 'id', int64, false, 'name', string, true,'pointTest',point,true)\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u51c6\u5907\u6570\u636e\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:" CREATE (n:person {id:1, name: 'Tom',pointTest:point(3.0,3.0,7203)}) RETURN n\n"})}),"\n",(0,s.jsx)(n.h3,{id:"53-\u6784\u5efa\u7f8e\u98df\u63a2\u7d22\u67e5\u8be2",children:"5.3 \u6784\u5efa\u7f8e\u98df\u63a2\u7d22\u67e5\u8be2"}),"\n",(0,s.jsx)(n.p,{children:"\u80fd\u591f\u6839\u636e\u7528\u6237\u7684\u5f53\u524d\u4f4d\u7f6e\uff0c\u5bfb\u627e\u8ddd\u79bb2.5\u4ee5\u5185\u7684\u7f8e\u98df,\u6839\u636e\u8ddd\u79bb\u8fdb\u884c\u5347\u5e8f\u6392\u5217\u3002\u8fd4\u56de\u8ddd\u79bb\u548c\u8bc4\u5206\u8ba9\u7528\u6237\u5f97\u5012\u66f4\u597d\u7684\u4f53\u9a8c\u3002"}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.strong,{children:"\u67e5\u8be2\u8bed\u53e5"})}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{children:"match (n:person{id:1}),(m:food) with n.pointTest as p1,m.pointTest as p2,m.name as food,m.mark as mark\nCALL spatial.distance(p1,p2) YIELD distance \nWHERE distance<2.5\nRETURN food,distance,mark ORDER by distance\n"})}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.img,{alt:"image.png",src:i(79564).A+"",width:"1786",height:"821"})}),"\n",(0,s.jsx)(n.p,{children:"\u6b64\u67e5\u8be2\u9996\u5148\u5339\u914d\u7279\u5b9a\u7684Person\u8282\u70b9\uff08\u4ee5\u7528\u6237\u540d\u201cTom\u201d\u4e3a\u4f8b\uff09\uff0c\u7136\u540e\u627e\u5230\u6240\u6709Food\u8282\u70b9\uff0c\u5229\u7528\u81ea\u5b9a\u4e49\u7684distance\u51fd\u6570\uff0c\u8ba1\u7b97Person\u8282\u70b9\u5f53\u524d\u4f4d\u7f6e\u4e0e\u6bcf\u4e2aFood\u8282\u70b9\u4e4b\u95f4\u7684\u76f4\u7ebf\u8ddd\u79bb\uff0c\u7b5b\u9009\u51fa\u8ddd\u79bb\u57282.5\u4e4b\u5185\u7684\u7f8e\u98df\u3002\u6700\u540e\uff0c\u6309\u7167\u7f8e\u98df\u7684\u8ddd\u79bb\u5347\u5e8f\u6392\u5217\u7ed3\u679c\uff0c\u9644\u5e26\u8bc4\u5206\u53c2\u8003\uff0c\u4e3a\u7528\u6237\u63d0\u4f9b\u6700\u4f18\u8d28\u7684\u63a8\u8350\u3002"}),"\n",(0,s.jsx)(n.h2,{id:"6-\u5c55\u671b",children:"6. \u5c55\u671b"}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7\u4e0a\u8ff0\u7ae0\u8282\uff0c\u6211\u4eec\u4e0d\u4ec5\u5c55\u793a\u4e86TuGraph\u5728\u5730\u7406\u7a7a\u95f4\u6570\u636e\u5904\u7406\u7684\u80fd\u529b\uff0c\u4e5f\u63cf\u7ed8\u4e86\u4e00\u4e2a\u5bcc\u6709\u5438\u5f15\u529b\u7684\u7f8e\u98df\u63a2\u7d22\u573a\u666f\uff0c\u8bc1\u660e\u4e86\u56fe\u6570\u636e\u5e93\u5728\u7ed3\u5408\u5730\u7406\u4f4d\u7f6e\u4fe1\u606f\u8fdb\u884c\u4e2a\u6027\u5316\u670d\u52a1\u65b9\u9762\u5177\u6709\u5de8\u5927\u6f5c\u529b\u3002\u65e0\u8bba\u662f\u5bfb\u627e\u5468\u672b\u7684\u4f11\u95f2\u53bb\u5904\uff0c\u8fd8\u662f\u63a2\u7d22\u65c5\u884c\u9014\u4e2d\u7684\u7279\u8272\u7f8e\u98df\uff0c\u8fd9\u6837\u7684\u5e94\u7528\u90fd\u5c06\u6781\u5927\u5730\u4e30\u5bcc\u4eba\u4eec\u7684\u751f\u6d3b\u4f53\u9a8c\u3002"}),"\n",(0,s.jsx)(n.p,{children:"TuGraph\u5c06\u6765\u4f1a\u7ee7\u7eed\u5b9e\u73b0Disjoint() \u3001WithinBBox()\uff0c\u4f1a\u4e30\u5bcc\u66f4\u591a\u4f7f\u7528\u573a\u666f\u3002\u5f53\u7136\uff0c\u4e5f\u6b22\u8fce\u5927\u5bb6\u4e00\u8d77\u53c2\u4e0e\uff0c\u5171\u540c\u5f00\u53d1\u5730\u7406\u7a7a\u95f4\u529f\u80fd\u3002"})]})}function o(e={}){const{wrapper:n}={...(0,d.R)(),...e.components};return n?(0,s.jsx)(n,{...e,children:(0,s.jsx)(h,{...e})}):h(e)}},94037:(e,n,i)=>{i.d(n,{A:()=>s});const s=i.p+"assets/images/createVertexLabel_PolygonTest-6c36f0248cb4843d13546b71df5b0eb1.png"}}]);