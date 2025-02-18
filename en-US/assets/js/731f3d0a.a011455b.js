"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[71224],{28453:(n,e,a)=>{a.d(e,{R:()=>s,x:()=>l});var r=a(96540);const t={},d=r.createContext(t);function s(n){const e=r.useContext(d);return r.useMemo((function(){return"function"==typeof n?n(e):{...e,...n}}),[e,n])}function l(n){let e;return e=n.disableParentContext?"function"==typeof n.components?n.components(t):n.components||t:s(n.components),r.createElement(d.Provider,{value:e},n.children)}},82123:(n,e,a)=>{a.r(e),a.d(e,{assets:()=>o,contentTitle:()=>s,default:()=>p,frontMatter:()=>d,metadata:()=>l,toc:()=>i});var r=a(74848),t=a(28453);const d={},s="Training",l={id:"olap&procedure/learn/training",title:"Training",description:"\u672c\u6587\u6863\u8be6\u7ec6\u4ecb\u7ecd\u4e86\u5982\u4f55\u4f7f\u7528TuGraph\u8fdb\u884c\u56fe\u795e\u7ecf\u7f51\u7edc\uff08GNN\uff09\u7684\u8bad\u7ec3\u3002",source:"@site/versions/version-4.3.0/zh-CN/source/9.olap&procedure/3.learn/3.training.md",sourceDirName:"9.olap&procedure/3.learn",slug:"/olap&procedure/learn/training",permalink:"/tugraph-db/en-US/zh/4.3.0/olap&procedure/learn/training",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:3,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Sampling API",permalink:"/tugraph-db/en-US/zh/4.3.0/olap&procedure/learn/sampling_api"},next:{title:"Heterogeneous Graph",permalink:"/tugraph-db/en-US/zh/4.3.0/olap&procedure/learn/heterogeneous_graph"}},o={},i=[{value:"1. \u8bad\u7ec3",id:"1-\u8bad\u7ec3",level:2},{value:"2. Mini-Batch\u8bad\u7ec3",id:"2-mini-batch\u8bad\u7ec3",level:2},{value:"3. \u5168\u56fe\u8bad\u7ec3",id:"3-\u5168\u56fe\u8bad\u7ec3",level:2}];function h(n){const e={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",header:"header",p:"p",pre:"pre",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",...(0,t.R)(),...n.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(e.header,{children:(0,r.jsx)(e.h1,{id:"training",children:"Training"})}),"\n",(0,r.jsxs)(e.blockquote,{children:["\n",(0,r.jsx)(e.p,{children:"\u672c\u6587\u6863\u8be6\u7ec6\u4ecb\u7ecd\u4e86\u5982\u4f55\u4f7f\u7528TuGraph\u8fdb\u884c\u56fe\u795e\u7ecf\u7f51\u7edc\uff08GNN\uff09\u7684\u8bad\u7ec3\u3002"}),"\n"]}),"\n",(0,r.jsx)(e.h2,{id:"1-\u8bad\u7ec3",children:"1. \u8bad\u7ec3"}),"\n",(0,r.jsx)(e.p,{children:"\u4f7f\u7528TuGraph \u56fe\u5b66\u4e60\u6a21\u5757\u8fdb\u884c\u8bad\u7ec3\u65f6\uff0c\u53ef\u4ee5\u5206\u4e3a\u5168\u56fe\u8bad\u7ec3\u548cmini-batch\u8bad\u7ec3\u3002\n\u5168\u56fe\u8bad\u7ec3\u5373\u628a\u5168\u56fe\u4eceTuGraph db\u52a0\u8f7d\u5230\u5185\u5b58\u4e2d\uff0c\u518d\u8fdb\u884cGNN\u7684\u8bad\u7ec3\u3002\u800cmini-batch\u8bad\u7ec3\u5219\u4f7f\u7528\u4e0a\u9762\u63d0\u5230\u7684TuGraph \u56fe\u5b66\u4e60\u6a21\u5757\u7684\u91c7\u6837\u7b97\u5b50\uff0c\u5c06\u5168\u56fe\u6570\u636e\u8fdb\u884c\u91c7\u6837\u540e\uff0c\u518d\u9001\u5165\u8bad\u7ec3\u6846\u67b6\u4e2d\u8fdb\u884c\u8bad\u7ec3\u3002"}),"\n",(0,r.jsx)(e.h2,{id:"2-mini-batch\u8bad\u7ec3",children:"2. Mini-Batch\u8bad\u7ec3"}),"\n",(0,r.jsx)(e.p,{children:"Mini-Batch\u8bad\u7ec3\u9700\u8981\u4f7f\u7528TuGraph \u56fe\u5b66\u4e60\u6a21\u5757\u7684\u91c7\u6837\u7b97\u5b50\uff0c\u76ee\u524d\u652f\u6301Neighbor Sampling\u3001Edge Sampling\u3001Random Walk Sampling\u548cNegative Sampling\u3002\nTuGraph \u56fe\u5b66\u4e60\u6a21\u5757\u7684\u91c7\u6837\u7b97\u5b50\u8fdb\u884c\u91c7\u6837\u540e\u7684\u7ed3\u679c\u4ee5List\u7684\u5f62\u5f0f\u8fd4\u56de\u3002\n\u4e0b\u9762\u4ee5Neighbor Sampling\u4e3a\u4f8b\uff0c\u4ecb\u7ecd\u5982\u4f55\u5c06\u91c7\u6837\u540e\u7684\u7ed3\u679c\uff0c\u8fdb\u884c\u683c\u5f0f\u8f6c\u6362\uff0c\u9001\u5165\u5230\u8bad\u7ec3\u6846\u67b6\u4e2d\u8fdb\u884c\u8bad\u7ec3\u3002\n\u7528\u6237\u9700\u8981\u63d0\u4f9b\u4e00\u4e2aSample\u7c7b\uff1a"}),"\n",(0,r.jsx)(e.pre,{children:(0,r.jsx)(e.code,{className:"language-python",children:"class TuGraphSample(object):\n    def __init__(self, args=None):\n        super(TuGraphSample, self).__init__()\n        self.args = args\n\n    def sample(self, g, seed_nodes):\n        args = self.args\n        # 1. \u52a0\u8f7d\u56fe\u6570\u636e\n        galaxy = PyGalaxy(args.db_path)\n        galaxy.SetCurrentUser(args.username, args.password)\n        db = galaxy.OpenGraph(args.graph_name, False)\n\n        sample_node = seed_nodes.tolist()\n        length = args.randomwalk_length\n        NodeInfo = []\n        EdgeInfo = []\n\n        # 2. \u91c7\u6837\u65b9\u6cd5\uff0c\u7ed3\u679c\u5b58\u50a8\u5728NodeInfo\u548cEdgeInfo\u4e2d\n        if args.sample_method == 'randomwalk':\n            randomwalk.Process(db, 100, sample_node, length, NodeInfo, EdgeInfo)\n        elif args.sample_method == 'negative':\n            negativesample.Process(db, 100)\n        else:\n            neighborsample(db, 100, sample_node, args.nbor_sample_num, NodeInfo, EdgeInfo)\n        del db\n        del galaxy\n\n        # 3. \u5bf9\u7ed3\u679c\u8fdb\u884c\u683c\u5f0f\u8f6c\u6362\uff0c\u4f7f\u4e4b\u7b26\u5408\u8bad\u7ec3\u683c\u5f0f\n        remap(EdgeInfo[0], EdgeInfo[1], NodeInfo[0])\n        g = dgl.graph((EdgeInfo[0], EdgeInfo[1]))\n        g.ndata['feat'] = torch.tensor(NodeInfo[1])\n        g.ndata['label'] = torch.tensor(NodeInfo[2])\n        return g\n"})}),"\n",(0,r.jsx)(e.p,{children:"\u5982\u4ee3\u7801\u6240\u793a\uff0c\u9996\u5148\u5c06\u56fe\u6570\u636e\u52a0\u8f7d\u5230\u5185\u5b58\u4e2d\u3002\u7136\u540e\u4f7f\u7528\u91c7\u6837\u7b97\u5b50\u5bf9\u56fe\u6570\u636e\u8fdb\u884c\u91c7\u6837\uff0c\u7ed3\u679c\u5b58\u50a8\u5728NodeInfo\u548cEdgeInfo\u4e2d\u3002NodeInfo\u548cEdgeInfo\u662fpython list\u7ed3\u679c\uff0c\u5176\u5b58\u50a8\u7684\u4fe1\u606f\u7ed3\u679c\u5982\u4e0b\uff1a"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(e.table,{children:[(0,r.jsx)(e.thead,{children:(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.th,{children:"\u56fe\u6570\u636e"}),(0,r.jsx)(e.th,{children:"\u5b58\u50a8\u4fe1\u606f\u4f4d\u7f6e"})]})}),(0,r.jsxs)(e.tbody,{children:[(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"\u8fb9\u8d77\u70b9"}),(0,r.jsx)(e.td,{children:"EdgeInfo[0]"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"\u8fb9\u7ec8\u70b9"}),(0,r.jsx)(e.td,{children:"EdgeInfo[1]"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"\u9876\u70b9ID"}),(0,r.jsx)(e.td,{children:"NodeInfo[0]"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"\u9876\u70b9\u7279\u5f81"}),(0,r.jsx)(e.td,{children:"NodeInfo[1]"})]}),(0,r.jsxs)(e.tr,{children:[(0,r.jsx)(e.td,{children:"\u9876\u70b9\u6807\u7b7e"}),(0,r.jsx)(e.td,{children:"NodeInfo[2]"})]})]})]}),"\n",(0,r.jsx)(e.p,{children:"\u6700\u540e\u5bf9\u7ed3\u679c\u8fdb\u884c\u683c\u5f0f\u8f6c\u6362\uff0c\u4f7f\u4e4b\u7b26\u5408\u8bad\u7ec3\u683c\u5f0f\u3002\u8fd9\u91cc\u6211\u4eec\u4f7f\u7528\u7684\u662fDGL\u8bad\u7ec3\u6846\u67b6\uff0c\u56e0\u6b64\u4f7f\u7528\u7ed3\u679c\u6570\u636e\u6784\u9020\u4e86DGL Graph\uff0c\u6700\u7ec8\u5c06DGL Graph\u8fd4\u56de\u3002\n\u6211\u4eec\u63d0\u4f9bTuGraphSample\u7c7b\u4e4b\u540e\uff0c\u5c31\u53ef\u4ee5\u4f7f\u7528\u5b83\u8fdb\u884cMini-Batch\u8bad\u7ec3\u4e86\u3002\n\u4ee4DGL\u7684\u6570\u636e\u52a0\u8f7d\u90e8\u5206\u4f7f\u7528TuGraphSample\u7684\u5b9e\u4f8bsampler\uff1a"}),"\n",(0,r.jsx)(e.pre,{children:(0,r.jsx)(e.code,{className:"language-python",children:"    sampler = TugraphSample(args)\n    fake_g = construct_graph() # just make dgl happy\n    dataloader = dgl.dataloading.DataLoader(fake_g,\n        torch.arange(train_nids),\n        sampler,\n        batch_size=batch_size,\n        device=device,\n        use_ddp=True,\n        num_workers=0,\n        drop_last=False,\n        )\n"})}),"\n",(0,r.jsx)(e.p,{children:"\u4f7f\u7528DGL\u8fdb\u884c\u6a21\u578b\u8bad\u7ec3\uff1a"}),"\n",(0,r.jsx)(e.pre,{children:(0,r.jsx)(e.code,{className:"language-python",children:"def train(dataloader, model):\n    optimizer = torch.optim.Adam(model.parameters(), lr=1e-2, weight_decay=5e-4)\n    model.train()\n    s = time.time()\n    for graph in dataloader:\n        load_time = time.time()\n        graph = dgl.add_self_loop(graph)\n        logits = model(graph, graph.ndata['feat'])\n        loss = loss_fcn(logits, graph.ndata['label'])\n        optimizer.zero_grad()\n        loss.backward()\n        optimizer.step()\n        train_time = time.time()\n        print('load time', load_time - s, 'train_time', train_time - load_time)\n        s = time.time()\n    return float(loss)\n"})}),"\n",(0,r.jsx)(e.h2,{id:"3-\u5168\u56fe\u8bad\u7ec3",children:"3. \u5168\u56fe\u8bad\u7ec3"}),"\n",(0,r.jsx)(e.p,{children:"GNN\uff08\u56fe\u795e\u7ecf\u7f51\u7edc\uff09\u7684\u5168\u56fe\u8bad\u7ec3\u662f\u4e00\u79cd\u6d89\u53ca\u4e00\u6b21\u5904\u7406\u6574\u4e2a\u8bad\u7ec3\u6570\u636e\u96c6\u7684\u8bad\u7ec3\u3002\u5b83\u662f GNN \u6700\u7b80\u5355\u3001\u6700\u76f4\u63a5\u7684\u8bad\u7ec3\u65b9\u6cd5\u4e4b\u4e00\uff0c\u6574\u4e2a\u56fe\u88ab\u89c6\u4e3a\u5355\u4e2a\u5b9e\u4f8b\u3002 \u5728\u5168\u56fe\u8bad\u7ec3\u4e2d\uff0c\u6574\u4e2a\u6570\u636e\u96c6\u88ab\u52a0\u8f7d\u5230\u5185\u5b58\u4e2d\uff0c\u6a21\u578b\u5728\u6574\u4e2a\u56fe\u4e0a\u8fdb\u884c\u8bad\u7ec3\u3002\u8fd9\u79cd\u7c7b\u578b\u7684\u8bad\u7ec3\u5bf9\u4e8e\u4e2d\u5c0f\u578b\u56fe\u7279\u522b\u6709\u7528\uff0c\u5e76\u4e14\u4e3b\u8981\u7528\u4e8e\u4e0d\u968f\u65f6\u95f4\u53d8\u5316\u7684\u9759\u6001\u56fe\u3002\n\u5728\u7b97\u5b50\u8c03\u7528\u65f6\uff0c\u4f7f\u7528\u4ee5\u4e0b\u65b9\u5f0f\uff1a"}),"\n",(0,r.jsx)(e.pre,{children:(0,r.jsx)(e.code,{className:"language-python",children:"getdb.Process(db, olapondb, feature_len, NodeInfo, EdgeInfo)\n"})}),"\n",(0,r.jsx)(e.p,{children:"\u83b7\u53d6\u5168\u56fe\u6570\u636e\uff0c\u7136\u540e\u5c06\u5168\u56fe\u9001\u5165\u8bad\u7ec3\u6846\u67b6\u4e2d\u8fdb\u884c\u8bad\u7ec3\u3002\n\u5b8c\u6574\u4ee3\u7801\uff1a\u8bf7\u53c2\u8003learn/examples/train_full_cora.py\u3002"})]})}function p(n={}){const{wrapper:e}={...(0,t.R)(),...n.components};return e?(0,r.jsx)(e,{...n,children:(0,r.jsx)(h,{...n})}):h(n)}}}]);