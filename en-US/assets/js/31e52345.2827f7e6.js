"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[9969],{28453:(e,n,a)=>{a.d(n,{R:()=>d,x:()=>t});var l=a(96540);const r={},i=l.createContext(r);function d(e){const n=l.useContext(i);return l.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function t(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(r):e.components||r:d(e.components),l.createElement(i.Provider,{value:n},e.children)}},81747:(e,n,a)=>{a.r(n),a.d(n,{assets:()=>s,contentTitle:()=>d,default:()=>h,frontMatter:()=>i,metadata:()=>t,toc:()=>o});var l=a(74848),r=a(28453);const i={},d="Sampling API",t={id:"developer-manual/interface/learn/sampling_api",title:"Sampling API",description:"\u6b64\u6587\u6863\u4e3b\u8981\u8be6\u7ec6\u4ecb\u7ecd\u4e86Sampling API\u7684\u4f7f\u7528\u8bf4\u660e",source:"@site/versions/version-4.1.0/zh-CN/source/5.developer-manual/6.interface/5.learn/2.sampling_api.md",sourceDirName:"5.developer-manual/6.interface/5.learn",slug:"/developer-manual/interface/learn/sampling_api",permalink:"/tugraph-db/en-US/zh/4.1.0/developer-manual/interface/learn/sampling_api",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:2,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Learn Tutorial",permalink:"/tugraph-db/en-US/zh/4.1.0/developer-manual/interface/learn/tutorial"},next:{title:"Training",permalink:"/tugraph-db/en-US/zh/4.1.0/developer-manual/interface/learn/training"}},s={},o=[{value:"1. \u6982\u8ff0",id:"1-\u6982\u8ff0",level:2},{value:"2. \u56fe\u6570\u636e\u9884\u5904\u7406",id:"2-\u56fe\u6570\u636e\u9884\u5904\u7406",level:2},{value:"3. \u56fe\u91c7\u6837\u7b97\u5b50\u4ecb\u7ecd",id:"3-\u56fe\u91c7\u6837\u7b97\u5b50\u4ecb\u7ecd",level:2},{value:"3.1.RandomWalk\u7b97\u5b50\uff1a",id:"31randomwalk\u7b97\u5b50",level:3},{value:"3.2.NeighborSample\u7b97\u5b50\uff1a",id:"32neighborsample\u7b97\u5b50",level:3},{value:"3.3.Nagetive\u7b97\u5b50\uff1a",id:"33nagetive\u7b97\u5b50",level:3},{value:"3.4.EdgeSampling\u7b97\u5b50\uff1a",id:"34edgesampling\u7b97\u5b50",level:3},{value:"3.5.GetDB\u7b97\u5b50\uff1a",id:"35getdb\u7b97\u5b50",level:3},{value:"4. \u7528\u6237\u81ea\u5b9a\u4e49\u91c7\u6837\u7b97\u6cd5",id:"4-\u7528\u6237\u81ea\u5b9a\u4e49\u91c7\u6837\u7b97\u6cd5",level:2}];function p(e){const n={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",p:"p",pre:"pre",...(0,r.R)(),...e.components};return(0,l.jsxs)(l.Fragment,{children:[(0,l.jsx)(n.header,{children:(0,l.jsx)(n.h1,{id:"sampling-api",children:"Sampling API"})}),"\n",(0,l.jsxs)(n.blockquote,{children:["\n",(0,l.jsx)(n.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u8be6\u7ec6\u4ecb\u7ecd\u4e86Sampling API\u7684\u4f7f\u7528\u8bf4\u660e"}),"\n"]}),"\n",(0,l.jsx)(n.h2,{id:"1-\u6982\u8ff0",children:"1. \u6982\u8ff0"}),"\n",(0,l.jsx)(n.p,{children:"\u672c\u624b\u518c\u4ecb\u7ecd\u4f7f\u7528TuGraph\u7684Sampling API\u63a5\u53e3\u3002"}),"\n",(0,l.jsx)(n.h2,{id:"2-\u56fe\u6570\u636e\u9884\u5904\u7406",children:"2. \u56fe\u6570\u636e\u9884\u5904\u7406"}),"\n",(0,l.jsx)(n.p,{children:"\u5728\u91c7\u6837\u64cd\u4f5c\u4e4b\u524d\uff0c\u6839\u636e\u56fe\u6570\u636e\u8def\u5f84\u52a0\u8f7d\u56fe\u6570\u636e\uff0c\u5e76\u6620\u5c04\u6210olapondb\u56fe\u5206\u6790\u7c7b\uff0c\u4ee3\u7801\u5982\u4e0b\uff1a"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-python",children:"galaxy = PyGalaxy(args.db_path) # \u6839\u636e\u8def\u5f84\u521b\u5efa\u4e00\u4e2agalaxy\u5b9e\u4f8b\ngalaxy.SetCurrentUser(args.username, args.password) # \u8bbe\u7f6e\u5f53\u524d\u7528\u6237\ndb = galaxy.OpenGraph('default', False) # \u6253\u5f00\u56fe\u6570\u636e\u5e93\u6307\u5b9adb\ntxn = db.CreateReadTxn() # \u521b\u5efa\u4e00\u4e2a\u4e8b\u52a1\u5b9e\u4f8b\nolapondb = PyOlapOnDB('Empty', db, txn) # \u6839\u636e\u56fe\u52a0\u8f7d\u65b9\u5f0f\u3001\u56fe\u6570\u636e\u5e93\u5b9e\u4f8b\u3001\u4e8b\u52a1\u5b9e\u4f8b\u5b9e\u4f8b\u5316OlapOnDB\ndel txn\ndel db\ndel galaxy\n"})}),"\n",(0,l.jsx)(n.h2,{id:"3-\u56fe\u91c7\u6837\u7b97\u5b50\u4ecb\u7ecd",children:"3. \u56fe\u91c7\u6837\u7b97\u5b50\u4ecb\u7ecd"}),"\n",(0,l.jsx)(n.p,{children:"\u56fe\u91c7\u6837\u7b97\u5b50\u5728cython\u5c42\u5b9e\u73b0\uff0c\u7528\u4e8e\u5bf9\u8f93\u5165\u7684\u56fe\u8fdb\u884c\u91c7\u6837\u5904\u7406\uff0c\u751f\u6210\u7684NodeInfo\u7528\u4e8e\u4fdd\u5b58feature\u5c5e\u6027\u3001label\u5c5e\u6027\u7b49\u70b9\u4fe1\u606f\uff0cEdgeInfo\u7528\u4e8e\u4fdd\u5b58\u8fb9\u4fe1\u606f\uff0c\u8fd9\u4e9b\u5143\u6570\u636e\u4fe1\u606f\u53ef\u4ee5\u88ab\u7528\u4e8e\u7279\u5f81\u62bd\u53d6\u3001\u7f51\u7edc\u5d4c\u5165\u7b49\u4efb\u52a1\u4e2d\u3002\u76ee\u524dTuGraph\u56fe\u5b66\u4e60\u6a21\u5757\u652f\u6301GetDB\u3001NeighborSampling\u3001EdgeSampling\u3001RandomWalkSampling\u3001NegativeSampling\u4e94\u79cd\u91c7\u6837\u7b97\u5b50\u3002"}),"\n",(0,l.jsx)(n.h3,{id:"31randomwalk\u7b97\u5b50",children:"3.1.RandomWalk\u7b97\u5b50\uff1a"}),"\n",(0,l.jsx)(n.p,{children:"\u5728\u7ed9\u5b9a\u7684\u91c7\u6837\u70b9\u5468\u56f4\u8fdb\u884c\u6307\u5b9a\u6b21\u6570\u7684\u968f\u673a\u6e38\u8d70\uff0c\u5f97\u5230\u91c7\u6837\u5b50\u56fe\u3002"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-python",children:"Process(db_: lgraph_db_python.PyGraphDB, olapondb: lgraph_db_python.PyOlapOnDB, feature_num: size_t, sample_node: list, step: size_t, NodeInfo: list, EdgeInfo: list)\n"})}),"\n",(0,l.jsx)(n.p,{children:"\u53c2\u6570\u5217\u8868\uff1a\ndb_: \u56fe\u6570\u636e\u5e93\u5b9e\u4f8b\u3002\nolapondb: \u56fe\u5206\u6790\u7c7b\u3002\nfeature_num: size_t\u7c7b\u578b\uff0cfeature\u7279\u5f81\u5411\u91cf\u7684\u957f\u5ea6\u3002\nsample_node: list\u7c7b\u578b\uff0c\u91c7\u6837\u70b9\u5217\u8868\u3002\nstep: size_t\u7c7b\u578b\uff0c\u91c7\u6837\u6b65\u6570\u3002\nNodeInfo: list\u7c7b\u578b\uff0c\u70b9\u5c5e\u6027\u5b57\u5178\u7684\u5217\u8868\uff0c\u8868\u793a\u70b9\u7684\u5143\u6570\u636e\u4fe1\u606f\u3002\nEdgeInfo: list\u7c7b\u578b\uff0c\u8fb9\u5c5e\u6027\u5b57\u5178\u7684\u5217\u8868\uff0c\u8868\u793a\u8fb9\u7684\u5143\u6570\u636e\u4fe1\u606f\u3002\n\u8fd4\u56de\u503c: \u65e0\u3002"}),"\n",(0,l.jsx)(n.h3,{id:"32neighborsample\u7b97\u5b50",children:"3.2.NeighborSample\u7b97\u5b50\uff1a"}),"\n",(0,l.jsx)(n.p,{children:"\u5728\u7ed9\u5b9a\u91c7\u6837\u70b9\u7684\u4e00\u5ea6\u90bb\u5c45\u4e2d\u91c7\u6837\u4e00\u5b9a\u6570\u91cf\u7684\u70b9\uff0c\u5f97\u5230\u91c7\u6837\u5b50\u56fe\u3002"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-python",children:"Process(db_: lgraph_db_python.PyGraphDB, olapondb: lgraph_db_python.PyOlapOnDB, feature_num: size_t, sample_node: list, nei_num: size_t, NodeInfo: list, EdgeInfo: list)\n"})}),"\n",(0,l.jsx)(n.p,{children:"\u53c2\u6570\u5217\u8868\uff1a\ndb_: \u56fe\u6570\u636e\u5e93\u5b9e\u4f8b\u3002\nolapondb: \u56fe\u5206\u6790\u7c7b\u3002\nfeature_num: size_t\u7c7b\u578b\uff0cfeature\u7279\u5f81\u5411\u91cf\u7684\u957f\u5ea6\u3002\nsample_node: list\u7c7b\u578b\uff0c\u91c7\u6837\u70b9\u5217\u8868\u3002\nnei_num: size_t\u7c7b\u578b\uff0c\u90bb\u5c45\u91c7\u6837\u70b9\u6570\u3002\nNodeInfo: list\u7c7b\u578b\uff0c\u70b9\u5c5e\u6027\u5b57\u5178\u7684\u5217\u8868\uff0c\u8868\u793a\u70b9\u7684\u5143\u6570\u636e\u4fe1\u606f\u3002\nEdgeInfo: list\u7c7b\u578b\uff0c\u8fb9\u5c5e\u6027\u5b57\u5178\u7684\u5217\u8868\uff0c\u8868\u793a\u8fb9\u7684\u5143\u6570\u636e\u4fe1\u606f\u3002\n\u8fd4\u56de\u503c: \u65e0\u3002"}),"\n",(0,l.jsx)(n.h3,{id:"33nagetive\u7b97\u5b50",children:"3.3.Nagetive\u7b97\u5b50\uff1a"}),"\n",(0,l.jsx)(n.p,{children:"\u91c7\u7528\u8d1f\u91c7\u6837\u7b97\u6cd5\uff0c\u751f\u6210\u4e0d\u5b58\u5728\u8fb9\u7684\u5b50\u56fe\u3002"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-python",children:"Process(db_: lgraph_db_python.PyGraphDB, olapondb: lgraph_db_python.PyOlapOnDB, feature_num: size_t, num_samples: size_t, NodeInfo: list, EdgeInfo: list)\n"})}),"\n",(0,l.jsx)(n.p,{children:"\u53c2\u6570\u5217\u8868\uff1a\ndb_: \u56fe\u6570\u636e\u5e93\u5b9e\u4f8b\u3002\nolapondb: \u56fe\u5206\u6790\u7c7b\u3002\nfeature_num: size_t\u7c7b\u578b\uff0cfeature\u7279\u5f81\u5411\u91cf\u7684\u957f\u5ea6\u3002\nnum_samples: size_t\u7c7b\u578b\uff0c\u751f\u6210\u4e0d\u5b58\u5728\u8fb9\u7684\u6570\u91cf\u3002\nNodeInfo: list\u7c7b\u578b\uff0c\u70b9\u5c5e\u6027\u5b57\u5178\u7684\u5217\u8868\uff0c\u8868\u793a\u70b9\u7684\u5143\u6570\u636e\u4fe1\u606f\u3002\nEdgeInfo: list\u7c7b\u578b\uff0c\u8fb9\u5c5e\u6027\u5b57\u5178\u7684\u5217\u8868\uff0c\u8868\u793a\u8fb9\u7684\u5143\u6570\u636e\u4fe1\u606f\u3002\n\u8fd4\u56de\u503c\uff1a \u65e0\u3002"}),"\n",(0,l.jsx)(n.h3,{id:"34edgesampling\u7b97\u5b50",children:"3.4.EdgeSampling\u7b97\u5b50\uff1a"}),"\n",(0,l.jsx)(n.p,{children:"\u6839\u636e\u8fb9\u91c7\u6837\u7387\uff0c\u751f\u6210\u91c7\u6837\u8fb9\u7684\u5b50\u56fe\u3002"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-python",children:"Process(db_: lgraph_db_python.PyGraphDB, olapondb:lgraph_db_python.PyOlapOnDB, feature_num: size_t, sample_rate: double, NodeInfo: list, EdgeInfo: list,EdgeInfo: list)\n"})}),"\n",(0,l.jsx)(n.p,{children:"\u53c2\u6570\u5217\u8868\uff1a\ndb_: \u56fe\u6570\u636e\u5e93\u5b9e\u4f8b\u3002\nolapondb: \u56fe\u5206\u6790\u7c7b\u3002\nfeature_num: size_t\u7c7b\u578b\uff0cfeature\u7279\u5f81\u5411\u91cf\u7684\u957f\u5ea6\u3002\nsample_rate: double\u7c7b\u578b\uff0c\u91c7\u6837\u7387\u3002\nNodeInfo: list\u7c7b\u578b\uff0c\u4e00\u4e2a\u70b9\u5c5e\u6027\u5b57\u5178\u7684\u5217\u8868\uff0c\u8868\u793a\u70b9\u7684\u5143\u6570\u636e\u4fe1\u606f\u3002\nEdgeInfo: list\u7c7b\u578b\uff0c\u4e00\u4e2a\u8fb9\u5c5e\u6027\u5b57\u5178\u7684\u5217\u8868\uff0c\u8868\u793a\u8fb9\u7684\u5143\u6570\u636e\u4fe1\u606f\u3002\n\u8fd4\u56de\u503c: \u65e0\u3002"}),"\n",(0,l.jsx)(n.h3,{id:"35getdb\u7b97\u5b50",children:"3.5.GetDB\u7b97\u5b50\uff1a"}),"\n",(0,l.jsx)(n.p,{children:"\u4ece\u6570\u636e\u5e93\u4e2d\u83b7\u53d6\u56fe\u6570\u636e\u5e76\u8f6c\u6362\u6210\u6240\u9700\u6570\u636e\u7ed3\u6784\u3002"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-python",children:"Process(db_: lgraph_db_python.PyGraphDB, olapondb:lgraph_db_python.PyOlapOnDB, feature_num: size_t, NodeInfo: list, EdgeInfo: list)\n"})}),"\n",(0,l.jsx)(n.p,{children:"\u53c2\u6570\u5217\u8868\uff1a\ndb_: \u56fe\u6570\u636e\u5e93\u5b9e\u4f8b\u3002\nolapondb: \u56fe\u5206\u6790\u7c7b\u3002\nfeature_num: size_t\u7c7b\u578b\uff0cfeature\u7279\u5f81\u5411\u91cf\u7684\u957f\u5ea6\u3002\nNodeInfo: list\u7c7b\u578b\uff0c\u4e00\u4e2a\u70b9\u5c5e\u6027\u5b57\u5178\u7684\u5217\u8868\uff0c\u8868\u793a\u70b9\u7684\u5143\u6570\u636e\u4fe1\u606f\u3002\nEdgeInfo: list\u7c7b\u578b\uff0c\u4e00\u4e2a\u8fb9\u5c5e\u6027\u5b57\u5178\u7684\u5217\u8868\uff0c\u8868\u793a\u8fb9\u7684\u5143\u6570\u636e\u4fe1\u606f\u3002\n\u8fd4\u56de\u503c: \u65e0\u3002"}),"\n",(0,l.jsx)(n.h2,{id:"4-\u7528\u6237\u81ea\u5b9a\u4e49\u91c7\u6837\u7b97\u6cd5",children:"4. \u7528\u6237\u81ea\u5b9a\u4e49\u91c7\u6837\u7b97\u6cd5"}),"\n",(0,l.jsxs)(n.p,{children:["\u7528\u6237\u4e5f\u53ef\u4ee5\u901a\u8fc7TuGraph Olap\u63a5\u53e3\u5b9e\u73b0\u81ea\u5b9a\u4e49\u91c7\u6837\u7b97\u6cd5\uff0c\u63a5\u53e3\u6587\u6863\u53c2\u89c1",(0,l.jsx)(n.a,{href:"/tugraph-db/en-US/zh/4.1.0/developer-manual/interface/olap/python-api",children:"\u6b64\u5904"}),"\uff0c\u8be5\u6587\u6863\u4e3b\u8981\u4ecb\u7ecd\u56fe\u91c7\u6837\u7b97\u6cd5\u4f7f\u7528\u7684\u76f8\u5173\u51fd\u6570\u7684\u63a5\u53e3\u8bbe\u8ba1\u3002"]})]})}function h(e={}){const{wrapper:n}={...(0,r.R)(),...e.components};return n?(0,l.jsx)(n,{...e,children:(0,l.jsx)(p,{...e})}):p(e)}}}]);