"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[97445],{22330:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>o,contentTitle:()=>l,default:()=>h,frontMatter:()=>t,metadata:()=>d,toc:()=>i});var s=r(74848),a=r(28453);const t={},l="\u4f7f\u7528 TuGraph \u56fe\u5b66\u4e60\u6a21\u5757\u8fdb\u884c\u70b9\u5206\u7c7b",d={id:"best-practices/learn_practices",title:"\u4f7f\u7528 TuGraph \u56fe\u5b66\u4e60\u6a21\u5757\u8fdb\u884c\u70b9\u5206\u7c7b",description:"1.\u7b80\u4ecb",source:"@site/versions/version-4.1.0/zh-CN/source/7.best-practices/2.learn_practices.md",sourceDirName:"7.best-practices",slug:"/best-practices/learn_practices",permalink:"/tugraph-db/en-US/zh/4.1.0/best-practices/learn_practices",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:2,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"\u4ece\u5173\u7cfb\u578b\u6570\u636e\u5e93\u5bfc\u5165TuGraph",permalink:"/tugraph-db/en-US/zh/4.1.0/best-practices/rdbms-to-tugraph"},next:{title:"\u6570\u636e\u8fc1\u79fb",permalink:"/tugraph-db/en-US/zh/4.1.0/best-practices/data_migration"}},o={},i=[{value:"1.\u7b80\u4ecb",id:"1\u7b80\u4ecb",level:2},{value:"2. \u524d\u7f6e\u6761\u4ef6",id:"2-\u524d\u7f6e\u6761\u4ef6",level:2},{value:"3. Cora \u6570\u636e\u96c6\u5bfc\u5165TuGraph\u6570\u636e\u5e93",id:"3-cora-\u6570\u636e\u96c6\u5bfc\u5165tugraph\u6570\u636e\u5e93",level:2},{value:"3.1. Cora \u6570\u636e\u96c6\u4ecb\u7ecd",id:"31-cora-\u6570\u636e\u96c6\u4ecb\u7ecd",level:3},{value:"3.2. \u6570\u636e\u5bfc\u5165",id:"32-\u6570\u636e\u5bfc\u5165",level:3},{value:"4. feature\u7279\u5f81\u8f6c\u6362",id:"4-feature\u7279\u5f81\u8f6c\u6362",level:2},{value:"5. \u7f16\u8bd1\u91c7\u6837\u7b97\u5b50",id:"5-\u7f16\u8bd1\u91c7\u6837\u7b97\u5b50",level:2},{value:"6. \u6a21\u578b\u8bad\u7ec3\u53ca\u4fdd\u5b58",id:"6-\u6a21\u578b\u8bad\u7ec3\u53ca\u4fdd\u5b58",level:2},{value:"6.1.\u6570\u636e\u52a0\u8f7d",id:"61\u6570\u636e\u52a0\u8f7d",level:3},{value:"6.2.\u6784\u5efa\u91c7\u6837\u5668",id:"62\u6784\u5efa\u91c7\u6837\u5668",level:3},{value:"6.3.\u5bf9\u7ed3\u679c\u8fdb\u884c\u683c\u5f0f\u8f6c\u6362",id:"63\u5bf9\u7ed3\u679c\u8fdb\u884c\u683c\u5f0f\u8f6c\u6362",level:3},{value:"6.4.\u6784\u5efaGCN\u6a21\u578b",id:"64\u6784\u5efagcn\u6a21\u578b",level:3},{value:"6.5.\u8bad\u7ec3GCN\u6a21\u578b",id:"65\u8bad\u7ec3gcn\u6a21\u578b",level:3}];function c(e){const n={a:"a",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",p:"p",pre:"pre",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",...(0,a.R)(),...e.components};return(0,s.jsxs)(s.Fragment,{children:[(0,s.jsx)(n.header,{children:(0,s.jsx)(n.h1,{id:"\u4f7f\u7528-tugraph-\u56fe\u5b66\u4e60\u6a21\u5757\u8fdb\u884c\u70b9\u5206\u7c7b",children:"\u4f7f\u7528 TuGraph \u56fe\u5b66\u4e60\u6a21\u5757\u8fdb\u884c\u70b9\u5206\u7c7b"})}),"\n",(0,s.jsx)(n.h2,{id:"1\u7b80\u4ecb",children:"1.\u7b80\u4ecb"}),"\n",(0,s.jsx)(n.p,{children:"GNN \u662f\u8bb8\u591a\u56fe\u4e0a\u673a\u5668\u5b66\u4e60\u4efb\u52a1\u7684\u5f3a\u5927\u5de5\u5177\u3002\u5728\u672c\u4ecb\u7ecd\u6027\u6559\u7a0b\u4e2d\uff0c\u60a8\u5c06\u5b66\u4e60\u4f7f\u7528 GNN \u8fdb\u884c\u70b9\u5206\u7c7b\u7684\u57fa\u672c\u5de5\u4f5c\u6d41\u7a0b\uff0c\u5373\u9884\u6d4b\u56fe\u4e2d\u70b9\u7684\u7c7b\u522b\u3002"}),"\n",(0,s.jsx)(n.p,{children:"\u6b64\u6587\u6863\u5c06\u5c55\u793a\u5982\u4f55\u6784\u5efa\u4e00\u4e2a GNN \u7528\u4e8e\u5728 Cora \u6570\u636e\u96c6\u4e0a\u4ec5\u4f7f\u7528\u5c11\u91cf\u6807\u7b7e\u8fdb\u884c\u534a\u76d1\u7763\u70b9\u5206\u7c7b\uff0c\u8fd9\u662f\u4e00\u4e2a\u4ee5\u8bba\u6587\u4e3a\u70b9\u3001\u5f15\u6587\u4e3a\u8fb9\u7684\u5f15\u6587\u7f51\u7edc\u3002\u4efb\u52a1\u662f\u9884\u6d4b\u7ed9\u5b9a\u8bba\u6587\u7684\u7c7b\u522b\u3002"}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7\u5b8c\u6210\u672c\u6559\u7a0b\uff0c\u60a8\u53ef\u4ee5"}),"\n",(0,s.jsx)(n.p,{children:"\u4f7f\u7528 TuGraph \u52a0\u8f7d cora \u6570\u636e\u96c6\u3002"}),"\n",(0,s.jsx)(n.p,{children:"\u4f7f\u7528 TuGraph \u63d0\u4f9b\u7684\u91c7\u6837\u7b97\u5b50\u91c7\u6837\uff0c\u5e76\u6784\u5efa GNN \u6a21\u578b\u3002"}),"\n",(0,s.jsx)(n.p,{children:"\u5728 CPU \u6216 GPU \u4e0a\u8bad\u7ec3\u7528\u4e8e\u70b9\u5206\u7c7b\u7684 GNN \u6a21\u578b\u3002"}),"\n",(0,s.jsx)(n.p,{children:"\u6b64\u6587\u6863\u9700\u8981\u5bf9\u56fe\u795e\u7ecf\u7f51\u7edc\u3001DGL\u7b49\u4f7f\u7528\u6709\u4e00\u5b9a\u7ecf\u9a8c\u3002"}),"\n",(0,s.jsx)(n.h2,{id:"2-\u524d\u7f6e\u6761\u4ef6",children:"2. \u524d\u7f6e\u6761\u4ef6"}),"\n",(0,s.jsx)(n.p,{children:"TuGraph\u56fe\u5b66\u4e60\u6a21\u5757\u9700\u8981TuGraph-db 3.5.1\u53ca\u4ee5\u4e0a\u7248\u672c\u3002"}),"\n",(0,s.jsx)(n.p,{children:"TuGraph\u90e8\u7f72\u63a8\u8350\u91c7\u7528Docker\u955c\u50cftugraph-compile 1.2.4\u53ca\u4ee5\u4e0a\u7248\u672c:"}),"\n",(0,s.jsx)(n.p,{children:"tugraph / tugraph-compile-ubuntu18.04:latest"}),"\n",(0,s.jsx)(n.p,{children:"tugraph / tugraph-compile-centos7:latest"}),"\n",(0,s.jsx)(n.p,{children:"tugraph / tugraph-compile-centos8:latest"}),"\n",(0,s.jsxs)(n.p,{children:["\u4ee5\u4e0a\u955c\u50cf\u5747\u53ef\u5728DockerHub\u4e0a\u83b7\u53d6\u3002\n\u5177\u4f53\u64cd\u4f5c\u8bf7\u53c2\u8003",(0,s.jsx)(n.a,{href:"/tugraph-db/en-US/zh/4.1.0/quick-start/preparation",children:"\u5feb\u901f\u4e0a\u624b"}),"\u3002"]}),"\n",(0,s.jsx)(n.h2,{id:"3-cora-\u6570\u636e\u96c6\u5bfc\u5165tugraph\u6570\u636e\u5e93",children:"3. Cora \u6570\u636e\u96c6\u5bfc\u5165TuGraph\u6570\u636e\u5e93"}),"\n",(0,s.jsx)(n.h3,{id:"31-cora-\u6570\u636e\u96c6\u4ecb\u7ecd",children:"3.1. Cora \u6570\u636e\u96c6\u4ecb\u7ecd"}),"\n",(0,s.jsx)(n.p,{children:"Cora \u6570\u636e\u96c6\u7531 2708 \u7bc7\u8bba\u6587\u7ec4\u6210\uff0c\u5206\u4e3a 7 \u4e2a\u7c7b\u522b\u3002\u6bcf\u7bc7\u8bba\u6587\u7531\u4e00\u4e2a 1433 \u7ef4\u7684\u8bcd\u888b\u8868\u793a\uff0c\u8868\u793a\u8bba\u6587\u4e2d\u7684\u5355\u8bcd\u662f\u5426\u51fa\u73b0\u3002\u8fd9\u4e9b\u8bcd\u888b\u7279\u5f81\u5df2\u7ecf\u9884\u5904\u7406\uff0c\u4ee5\u4ece 0 \u5230 1 \u7684\u8303\u56f4\u5f52\u4e00\u5316\u3002\u8fb9\u8868\u793a\u8bba\u6587\u4e4b\u95f4\u7684\u5f15\u7528\u5173\u7cfb\u3002"}),"\n",(0,s.jsx)(n.p,{children:"TuGraph\u4e2d\u5df2\u7ecf\u63d0\u4f9b\u4e86Cora\u6570\u636e\u96c6\u7684\u5bfc\u5165\u5de5\u5177\uff0c\u7528\u6237\u53ef\u4ee5\u76f4\u63a5\u4f7f\u7528\u3002"}),"\n",(0,s.jsx)(n.h3,{id:"32-\u6570\u636e\u5bfc\u5165",children:"3.2. \u6570\u636e\u5bfc\u5165"}),"\n",(0,s.jsx)(n.p,{children:"Cora\u6570\u636e\u96c6\u5728test/integration/data/algo\u76ee\u5f55\u4e0b\uff0c\u5305\u542b\u70b9\u96c6cora_vertices\u548c\u8fb9\u96c6cora_edge\u3002"}),"\n",(0,s.jsxs)(n.p,{children:["\u9996\u5148\u9700\u8981\u5c06Cora\u6570\u636e\u96c6\u5bfc\u5165\u5230TuGraph\u6570\u636e\u5e93\u4e2d\uff0c\u8be6\u7ec6\u64cd\u4f5c\u53ef\u53c2\u8003",(0,s.jsx)(n.a,{href:"/tugraph-db/en-US/zh/4.1.0/developer-manual/server-tools/data-import",children:"\u6570\u636e\u5bfc\u5165"}),"\u3002"]}),"\n",(0,s.jsx)(n.p,{children:"\u5728build/output\u76ee\u5f55\u4e0b\u6267\u884c\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-bash",children:"cp -r ../../test/integration/data/ ./ && cp -r ../../learn/examples/* ./\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u8be5\u6307\u4ee4\u5c06\u6570\u636e\u96c6\u76f8\u5173\u6587\u4ef6\u62f7\u8d1d\u5230build/output\u76ee\u5f55\u4e0b\u3002"}),"\n",(0,s.jsx)(n.p,{children:"\u7136\u540e\u8fdb\u884c\u6570\u636e\u5bfc\u5165\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-bash",children:"./lgraph_import -c ./data/algo/cora.conf --dir ./coradb --overwrite 1\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u5176\u4e2dcora.conf\u4e3a\u56feschema\u6587\u4ef6\uff0c\u4ee3\u8868\u56fe\u6570\u636e\u7684\u683c\u5f0f,\u53ef\u53c2\u8003test/integration/data/algo/cora.conf\u3002coradb\u4e3a\u5bfc\u5165\u540e\u7684\u56fe\u6570\u636e\u6587\u4ef6\u540d\u79f0\uff0c\u4ee3\u8868\u56fe\u6570\u636e\u7684\u5b58\u50a8\u4f4d\u7f6e\u3002"}),"\n",(0,s.jsx)(n.h2,{id:"4-feature\u7279\u5f81\u8f6c\u6362",children:"4. feature\u7279\u5f81\u8f6c\u6362"}),"\n",(0,s.jsx)(n.p,{children:"\u7531\u4e8eCora\u6570\u636e\u96c6\u4e2d\u7684feature\u7279\u5f81\u4e3a\u957f\u5ea6\u4e3a1433\u7684float\u7c7b\u578b\u6570\u7ec4\uff0cTuGraph\u6682\u4e0d\u652f\u6301float\u6570\u7ec4\u7c7b\u578b\u52a0\u8f7d\uff0c\u56e0\u6b64\u53ef\u5c06\u5176\u6309\u7167string\u7c7b\u578b\u5bfc\u5165\u540e\uff0c\u8f6c\u6362\u6210char*\u65b9\u4fbf\u540e\u7eed\u5b58\u53d6\uff0c\u5177\u4f53\u5b9e\u73b0\u53ef\u53c2\u8003feature_float.cpp\u6587\u4ef6\u3002\n\u5177\u4f53\u6267\u884c\u8fc7\u7a0b\u5982\u4e0b\uff1a"}),"\n",(0,s.jsxs)(n.p,{children:["\u5728build\u76ee\u5f55\u4e0b\u7f16\u8bd1\u5bfc\u5165plugin(\u5982\u679cTuGraph\u5df2\u7f16\u8bd1\u53ef\u8df3\u8fc7)\uff1a\n",(0,s.jsx)(n.code,{children:"make feature_float_embed"})]}),"\n",(0,s.jsxs)(n.p,{children:["\u5728build/output\u76ee\u5f55\u4e0b\u6267\u884c\n",(0,s.jsx)(n.code,{children:"./algo/feature_float_embed ./coradb"}),"\n\u5373\u53ef\u8fdb\u884c\u8f6c\u6362\u3002"]}),"\n",(0,s.jsx)(n.h2,{id:"5-\u7f16\u8bd1\u91c7\u6837\u7b97\u5b50",children:"5. \u7f16\u8bd1\u91c7\u6837\u7b97\u5b50"}),"\n",(0,s.jsxs)(n.p,{children:["\u91c7\u6837\u7b97\u5b50\u7528\u4e8e\u4ece\u6570\u636e\u5e93\u4e2d\u83b7\u53d6\u56fe\u6570\u636e\u5e76\u8f6c\u6362\u6210\u6240\u9700\u6570\u636e\u7ed3\u6784\uff0c\u5177\u4f53\u6267\u884c\u8fc7\u7a0b\u5982\u4e0b(\u5982\u679cTuGraph\u5df2\u7f16\u8bd1\uff0c\u53ef\u8df3\u8fc7\u6b64\u6b65\u9aa4)\uff1a\n\u5728tugraph-db/build\u6587\u4ef6\u5939\u4e0b\u6267\u884c\n",(0,s.jsx)(n.code,{children:"make -j2"})]}),"\n",(0,s.jsxs)(n.p,{children:["\u6216\u5728tugraph-db/learn/procedures\u6587\u4ef6\u5939\u4e0b\u6267\u884c\n",(0,s.jsx)(n.code,{children:"python3 setup.py build_ext -i"})]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'from lgraph_db_python import *  # \u5bfc\u5165tugraph-db\u7684python\u63a5\u53e3\u6a21\u5757\nimport importlib  # \u5bfc\u5165importlib\u6a21\u5757\ngetdb = importlib.import_module("getdb")  #\u83b7\u53d6getdb\u7b97\u5b50\ngetdb.Process(db, olapondb, feature_len, NodeInfo, EdgeInfo) #\u8c03\u7528getdb\u7b97\u5b50\n'})}),"\n",(0,s.jsx)(n.p,{children:"\u5982\u4ee3\u7801\u6240\u793a\uff0c\u5f97\u5230\u7b97\u5b50so\u6587\u4ef6\u540e\uff0cimport \u5bfc\u5165\u4f7f\u7528\u3002"}),"\n",(0,s.jsx)(n.h2,{id:"6-\u6a21\u578b\u8bad\u7ec3\u53ca\u4fdd\u5b58",children:"6. \u6a21\u578b\u8bad\u7ec3\u53ca\u4fdd\u5b58"}),"\n",(0,s.jsxs)(n.p,{children:["TuGraph\u5728python\u5c42\u8c03\u7528cython\u5c42\u7684\u7b97\u5b50\uff0c\u5b9e\u73b0\u56fe\u5b66\u4e60\u6a21\u578b\u7684\u8bad\u7ec3\u3002\n\u4f7f\u7528 TuGraph \u56fe\u5b66\u4e60\u6a21\u5757\u4f7f\u7528\u65b9\u5f0f\u4ecb\u7ecd\u5982\u4e0b:\n\u5728build/output\u6587\u4ef6\u5939\u4e0b\u6267\u884c\n",(0,s.jsx)(n.code,{children:"python3 train_full_cora.py --model_save_path ./cora_model"}),"\n\u5373\u53ef\u8fdb\u884c\u8bad\u7ec3\u3002\n\u6700\u7ec8\u6253\u5370loss\u6570\u503c\u5c0f\u4e8e0.9\uff0c\u5373\u4e3a\u8bad\u7ec3\u6210\u529f\u3002\u81f3\u6b64\uff0c\u56fe\u6a21\u578b\u8bad\u7ec3\u5b8c\u6210\uff0c\u6a21\u578b\u4fdd\u5b58\u5728cora_model\u6587\u4ef6\u3002"]}),"\n",(0,s.jsx)(n.p,{children:"\u8bad\u7ec3\u8be6\u7ec6\u8fc7\u7a0b\u5982\u4e0b\uff1a"}),"\n",(0,s.jsx)(n.h3,{id:"61\u6570\u636e\u52a0\u8f7d",children:"6.1.\u6570\u636e\u52a0\u8f7d"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:"galaxy = PyGalaxy(args.db_path)\ngalaxy.SetCurrentUser(args.username, args.password)\ndb = galaxy.OpenGraph(args.graph_name, False)\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u5982\u4ee3\u7801\u6240\u793a\uff0c\u6839\u636e\u56fe\u6570\u636e\u8def\u5f84\u3001\u7528\u6237\u540d\u3001\u5bc6\u7801\u548c\u5b50\u56fe\u540d\u79f0\u5c06\u6570\u636e\u52a0\u8f7d\u5230\u5185\u5b58\u4e2d\u3002TuGraph\u53ef\u4ee5\u8f7d\u5165\u591a\u4e2a\u5b50\u56fe\u7528\u4e8e\u56fe\u8bad\u7ec3\uff0c\u5728\u6b64\u5904\u6211\u4eec\u53ea\u8f7d\u5165\u4e00\u4e2a\u5b50\u56fe\u3002"}),"\n",(0,s.jsx)(n.h3,{id:"62\u6784\u5efa\u91c7\u6837\u5668",children:"6.2.\u6784\u5efa\u91c7\u6837\u5668"}),"\n",(0,s.jsx)(n.p,{children:"\u8bad\u7ec3\u8fc7\u7a0b\u4e2d\uff0c\u9996\u5148\u4f7f\u7528GetDB\u7b97\u5b50\u4ece\u6570\u636e\u5e93\u4e2d\u83b7\u53d6\u56fe\u6570\u636e\u5e76\u8f6c\u6362\u6210\u6240\u9700\u6570\u636e\u7ed3\u6784\uff0c\u5177\u4f53\u4ee3\u7801\u5982\u4e0b\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:"    GetDB.Process(db_: lgraph_db_python.PyGraphDB, olapondb: lgraph_db_python.PyOlapOnDB, feature_num: size_t, NodeInfo: list, EdgeInfo: list)\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u5982\u4ee3\u7801\u6240\u793a\uff0c\u7ed3\u679c\u5b58\u50a8\u5728NodeInfo\u548cEdgeInfo\u4e2d\u3002NodeInfo\u548cEdgeInfo\u662fpython list\u7ed3\u679c\uff0c\u5176\u5b58\u50a8\u7684\u4fe1\u606f\u7ed3\u679c\u5982\u4e0b\uff1a"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"\u56fe\u6570\u636e"}),(0,s.jsx)(n.th,{children:"\u5b58\u50a8\u4fe1\u606f\u4f4d\u7f6e"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"\u8fb9\u8d77\u70b9"}),(0,s.jsx)(n.td,{children:"EdgeInfo[0]"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"\u8fb9\u7ec8\u70b9"}),(0,s.jsx)(n.td,{children:"EdgeInfo[1]"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"\u9876\u70b9ID"}),(0,s.jsx)(n.td,{children:"NodeInfo[0]"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"\u9876\u70b9\u7279\u5f81"}),(0,s.jsx)(n.td,{children:"NodeInfo[1]"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"\u9876\u70b9\u6807\u7b7e"}),(0,s.jsx)(n.td,{children:"NodeInfo[2]"})]})]})]}),"\n",(0,s.jsx)(n.p,{children:"\u7136\u540e\u6784\u5efa\u91c7\u6837\u5668"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:"    batch_size = 5\n    count = 2708\n    sampler = TugraphSample(args)\n    dataloader = dgl.dataloading.DataLoader(fake_g,\n        torch.arange(count),\n        sampler,\n        batch_size=batch_size,\n        num_workers=0,\n        )\n"})}),"\n",(0,s.jsx)(n.h3,{id:"63\u5bf9\u7ed3\u679c\u8fdb\u884c\u683c\u5f0f\u8f6c\u6362",children:"6.3.\u5bf9\u7ed3\u679c\u8fdb\u884c\u683c\u5f0f\u8f6c\u6362"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:"    src = EdgeInfo[0].astype('int64')\n    dst = EdgeInfo[1].astype('int64')\n    nodes_idx = NodeInfo[0].astype('int64')\n    remap(src, dst, nodes_idx)\n    features = NodeInfo[1].astype('float32')\n    labels = NodeInfo[2].astype('int64')\n    g = dgl.graph((src, dst))\n    g.ndata['feat'] = torch.tensor(features)\n    g.ndata['label'] = torch.tensor(labels)\n    return g\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u5bf9\u7ed3\u679c\u8fdb\u884c\u683c\u5f0f\u8f6c\u6362\uff0c\u4f7f\u4e4b\u7b26\u5408\u8bad\u7ec3\u683c\u5f0f"}),"\n",(0,s.jsx)(n.h3,{id:"64\u6784\u5efagcn\u6a21\u578b",children:"6.4.\u6784\u5efaGCN\u6a21\u578b"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:"class GCN(nn.Module):\n    def __init__(self, in_size, hid_size, out_size):\n        super().__init__()\n        self.layers = nn.ModuleList()\n        # two-layer GCN\n        self.layers.append(dgl.nn.GraphConv(in_size, hid_size, activation=F.relu))\n        self.layers.append(dgl.nn.GraphConv(hid_size, out_size))\n        self.dropout = nn.Dropout(0.5)\n\n    def forward(self, g, features):\n        h = features\n        for i, layer in enumerate(self.layers):\n            if i != 0:\n                h = self.dropout(h)\n            h = layer(g, h)\n        return h\n\ndef build_model():\n    in_size = feature_len  #feature_len\u4e3afeature\u7684\u957f\u5ea6\uff0c\u5728\u6b64\u5904\u4e3a1433\n    out_size = classes  #classes\u4e3a\u7c7b\u522b\u6570\uff0c\u5728\u6b64\u5904\u4e3a7\n    model = GCN(in_size, 16, out_size)  #16\u4e3a\u9690\u85cf\u5c42\u5927\u5c0f\n    return model\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u672c\u6559\u7a0b\u5c06\u6784\u5efa\u4e00\u4e2a\u4e24\u5c42\u56fe\u5377\u79ef\u7f51\u7edc\uff08GCN\uff09\u3002\u6bcf\u5c42\u901a\u8fc7\u805a\u5408\u90bb\u5c45\u4fe1\u606f\u6765\u8ba1\u7b97\u65b0\u7684\u70b9\u8868\u793a\u3002"}),"\n",(0,s.jsx)(n.h3,{id:"65\u8bad\u7ec3gcn\u6a21\u578b",children:"6.5.\u8bad\u7ec3GCN\u6a21\u578b"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:"loss_fcn = nn.CrossEntropyLoss()\ndef train(graph, model, model_save_path):\n    optimizer = torch.optim.Adam(model.parameters(), lr=1e-2, weight_decay=5e-4)\n    model.train()\n    s = time.time()\n    load_time = time.time()\n    graph = dgl.add_self_loop(graph)\n    logits = model(graph, graph.ndata['feat'])\n    loss = loss_fcn(logits, graph.ndata['label'])\n    optimizer.zero_grad()\n    loss.backward()\n    optimizer.step()\n    train_time = time.time()\n    current_loss = float(loss)\n    if model_save_path != \"\":   #\u5982\u679c\u9700\u8981\u4fdd\u5b58\u6a21\u578b\uff0c\u5219\u7ed9\u51fa\u6a21\u578b\u4fdd\u5b58\u8def\u5f84\n        if 'min_loss' not in train.__dict__:\n            train.min_loss = current_loss\n        elif current_loss < train.min_loss:\n            train.min_loss = current_loss\n            model_save_path = 'best_model.pth'\n        torch.save(model.state_dict(), model_save_path)\n    return current_loss\n\nfor epoch in range(50):\n    model.train()\n    total_loss = 0\n    loss = train(g, model)\n    if epoch % 5 == 0:\n        print('In epoch', epoch, ', loss', loss)\n    sys.stdout.flush()\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u5982\u4ee3\u7801\u6240\u793a\uff0c\u6839\u636e\u5b9a\u4e49\u597d\u7684\u91c7\u6837\u5668\u3001\u4f18\u5316\u5668\u548c\u6a21\u578b\u8fdb\u884c\u8fed\u4ee3\u8bad\u7ec350\u6b21\uff0c\u8bad\u7ec3\u540e\u7684\u6a21\u578b\u4fdd\u5b58\u81f3model_save_path\u8def\u5f84\u4e2d\u3002"}),"\n",(0,s.jsx)(n.p,{children:"\u8f93\u51fa\u7ed3\u679c\u5982\u4e0b\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-bash",children:"In epoch 0 , loss 1.9586775302886963\nIn epoch 5 , loss 1.543689250946045\nIn epoch 10 , loss 1.160698413848877\nIn epoch 15 , loss 0.8862786889076233\nIn epoch 20 , loss 0.6973256468772888\nIn epoch 25 , loss 0.5770673751831055\nIn epoch 30 , loss 0.5271289348602295\nIn epoch 35 , loss 0.45514997839927673\nIn epoch 40 , loss 0.43748989701271057\nIn epoch 45 , loss 0.3906335234642029\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u540c\u65f6\uff0c\u56fe\u5b66\u4e60\u6a21\u5757\u53ef\u91c7\u7528GPU\u8fdb\u884c\u52a0\u901f\uff0c\u7528\u6237\u5982\u679c\u9700\u8981\u518dGPU\u4e0a\u8fd0\u884c\uff0c\u9700\u8981\u7528\u6237\u81ea\u884c\u5b89\u88c5\u76f8\u5e94\u7684GPU\u9a71\u52a8\u548c\u73af\u5883\u3002\u5177\u4f53\u53ef\u53c2\u8003learn/README.md\u3002"}),"\n",(0,s.jsx)(n.p,{children:"\u5b8c\u6574\u4ee3\u7801\u53ef\u53c2\u8003learn/examples/train_full_cora.py"})]})}function h(e={}){const{wrapper:n}={...(0,a.R)(),...e.components};return n?(0,s.jsx)(n,{...e,children:(0,s.jsx)(c,{...e})}):c(e)}},28453:(e,n,r)=>{r.d(n,{R:()=>l,x:()=>d});var s=r(96540);const a={},t=s.createContext(a);function l(e){const n=s.useContext(t);return s.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function d(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(a):e.components||a:l(e.components),s.createElement(t.Provider,{value:n},e.children)}}}]);