"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[21511],{28453:(e,n,r)=>{r.d(n,{R:()=>d,x:()=>s});var l=r(96540);const i={},a=l.createContext(i);function d(e){const n=l.useContext(a);return l.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function s(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(i):e.components||i:d(e.components),l.createElement(a.Provider,{value:n},e.children)}},99904:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>t,contentTitle:()=>d,default:()=>p,frontMatter:()=>a,metadata:()=>s,toc:()=>o});var l=r(74848),i=r(28453);const a={},d="OlapOnDB API",s={id:"olap&procedure/olap/olap-on-db-api",title:"OlapOnDB API",description:"\u6b64\u6587\u6863\u4e3b\u8981\u8be6\u7ec6\u4ecb\u7ecd\u4e86OlapOnDB API\u7684\u4f7f\u7528\u8bf4\u660e",source:"@site/versions/version-4.3.0/zh-CN/source/9.olap&procedure/2.olap/3.olap-on-db-api.md",sourceDirName:"9.olap&procedure/2.olap",slug:"/olap&procedure/olap/olap-on-db-api",permalink:"/tugraph-db/en-US/zh/4.3.0/olap&procedure/olap/olap-on-db-api",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:3,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"OlapBase API",permalink:"/tugraph-db/en-US/zh/4.3.0/olap&procedure/olap/olap-base-api"},next:{title:"OlapOnDisk API",permalink:"/tugraph-db/en-US/zh/4.3.0/olap&procedure/olap/olap-on-disk-api"}},t={},o=[{value:"1. \u7b80\u4ecb",id:"1-\u7b80\u4ecb",level:2},{value:"2. \u6a21\u578b",id:"2-\u6a21\u578b",level:2},{value:"2.1 \u57fa\u4e8e\u5feb\u7167\u7684\u5b58\u50a8\u7ed3\u6784",id:"21-\u57fa\u4e8e\u5feb\u7167\u7684\u5b58\u50a8\u7ed3\u6784",level:3},{value:"2.2 BSP\u8ba1\u7b97\u6a21\u578b",id:"22-bsp\u8ba1\u7b97\u6a21\u578b",level:3},{value:"3. \u7b97\u6cd5\u4e3e\u4f8b",id:"3-\u7b97\u6cd5\u4e3e\u4f8b",level:2},{value:"3.1 \u4e3b\u51fd\u6570",id:"31-\u4e3b\u51fd\u6570",level:3},{value:"3.2 PageRank\u7b97\u6cd5\u6d41\u7a0b",id:"32-pagerank\u7b97\u6cd5\u6d41\u7a0b",level:3},{value:"4. \u5176\u4ed6\u5e38\u7528\u51fd\u6570\u529f\u80fd\u63cf\u8ff0",id:"4-\u5176\u4ed6\u5e38\u7528\u51fd\u6570\u529f\u80fd\u63cf\u8ff0",level:2},{value:"4.1 \u4e8b\u52a1\u7684\u521b\u5efa",id:"41-\u4e8b\u52a1\u7684\u521b\u5efa",level:3},{value:"4.2 \u5e76\u884c\u5316\u521b\u5efa\u6709\u5411\u56fe",id:"42-\u5e76\u884c\u5316\u521b\u5efa\u6709\u5411\u56fe",level:3},{value:"4.3 \u5e76\u884c\u5316\u521b\u5efa\u65e0\u5411\u56fe",id:"43-\u5e76\u884c\u5316\u521b\u5efa\u65e0\u5411\u56fe",level:3},{value:"4.4 \u83b7\u53d6\u51fa\u5ea6",id:"44-\u83b7\u53d6\u51fa\u5ea6",level:3},{value:"4.5 \u83b7\u53d6\u5165\u5ea6",id:"45-\u83b7\u53d6\u5165\u5ea6",level:3},{value:"4.6 \u83b7\u53d6\u51fa\u8fb9\u96c6\u5408",id:"46-\u83b7\u53d6\u51fa\u8fb9\u96c6\u5408",level:3},{value:"4.7 \u83b7\u53d6\u5165\u8fb9\u96c6\u5408",id:"47-\u83b7\u53d6\u5165\u8fb9\u96c6\u5408",level:3},{value:"4.8 \u83b7\u53d6TuGraph\u4e2d\u8282\u70b9\u5bf9\u5e94OlapOnDB\u7684\u8282\u70b9\u7f16\u53f7",id:"48-\u83b7\u53d6tugraph\u4e2d\u8282\u70b9\u5bf9\u5e94olapondb\u7684\u8282\u70b9\u7f16\u53f7",level:3},{value:"4.9 \u83b7\u53d6OlapOnDB\u4e2d\u8282\u70b9\u5bf9\u5e94TuGraph\u7684\u8282\u70b9\u7f16\u53f7",id:"49-\u83b7\u53d6olapondb\u4e2d\u8282\u70b9\u5bf9\u5e94tugraph\u7684\u8282\u70b9\u7f16\u53f7",level:3},{value:"4.10 \u6d3b\u8dc3\u70b9\u7684\u63cf\u8ff0",id:"410-\u6d3b\u8dc3\u70b9\u7684\u63cf\u8ff0",level:3}];function c(e){const n={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",li:"li",ol:"ol",p:"p",pre:"pre",...(0,i.R)(),...e.components};return(0,l.jsxs)(l.Fragment,{children:[(0,l.jsx)(n.header,{children:(0,l.jsx)(n.h1,{id:"olapondb-api",children:"OlapOnDB API"})}),"\n",(0,l.jsxs)(n.blockquote,{children:["\n",(0,l.jsx)(n.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u8be6\u7ec6\u4ecb\u7ecd\u4e86OlapOnDB API\u7684\u4f7f\u7528\u8bf4\u660e"}),"\n"]}),"\n",(0,l.jsx)(n.h2,{id:"1-\u7b80\u4ecb",children:"1. \u7b80\u4ecb"}),"\n",(0,l.jsx)(n.p,{children:"\u4e00\u822c\u7528\u6237\u9700\u8981\u81ea\u5df1\u5b9e\u73b0\u7684\u53ea\u662f\u5c06\u9700\u8981\u5206\u6790\u7684\u5b50\u56fe\u62bd\u53d6\u51fa\u6765\u7684\u8fc7\u7a0b\u3002\u7528\u6237\u4e5f\u53ef\u4ee5\u901a\u8fc7\u4f7f\u7528TuGraph\u4e2d\u4e30\u5bcc\u7684\u8f85\u52a9\u63a5\u53e3\u5b9e\u73b0\u81ea\u5df1\u7684\u56fe\u5206\u6790\u7b97\u6cd5\u3002"}),"\n",(0,l.jsx)(n.p,{children:"\u8be5\u6587\u6863\u4e3b\u8981\u4ecb\u7ecdProcedure\u53caEmbed\u7684\u63a5\u53e3\u8bbe\u8ba1\uff0c\u5e76\u4ecb\u7ecd\u90e8\u5206\u5e38\u7528\u63a5\u53e3\uff0c\u5177\u4f53\u7684\u63a5\u53e3\u4fe1\u606f\u53c2\u89c1include/lgraph/olap_on_db.h\u6587\u4ef6\u3002"}),"\n",(0,l.jsx)(n.h2,{id:"2-\u6a21\u578b",children:"2. \u6a21\u578b"}),"\n",(0,l.jsx)(n.p,{children:"Procedure\u53caEmbed\u4f7f\u7528\u5230\u7684\u8f85\u52a9\u51fd\u6570\u4e3b\u8981\u5305\u542b\u5728OlapOnDB\u7c7b\uff0c\u8fd8\u6709\u4e00\u4e9b\u4f7f\u7528\u9891\u7387\u8f83\u9ad8\u7684\u51fd\u6570\u90fd\u4f1a\u9010\u4e00\u4ecb\u7ecd"}),"\n",(0,l.jsx)(n.p,{children:"\u5728TuGraph\u4e2dOLAP\u76f8\u5173\u7684\u6709\u4ee5\u4e0b\u5e38\u7528\u7684\u6570\u636e\u7ed3\u6784\uff1a"}),"\n",(0,l.jsxs)(n.ol,{children:["\n",(0,l.jsxs)(n.li,{children:["DB\u56fe\u5206\u6790\u7c7b ",(0,l.jsx)(n.code,{children:"OlapOnDB<EdgeData>"})]}),"\n",(0,l.jsxs)(n.li,{children:["\u70b9\u6570\u7ec4",(0,l.jsx)(n.code,{children:"ParallelVector<VertexData>"})]}),"\n",(0,l.jsxs)(n.li,{children:["\u70b9\u96c6\u5408",(0,l.jsx)(n.code,{children:"ParallelBitset"})]}),"\n",(0,l.jsxs)(n.li,{children:["\u8fb9\u6570\u636e\u7ed3\u6784",(0,l.jsx)(n.code,{children:"AdjUnit/AdjUnit<Empty>"})]}),"\n",(0,l.jsxs)(n.li,{children:["\u8fb9\u96c6\u5408\u6570\u636e\u7ed3\u6784",(0,l.jsx)(n.code,{children:"AdjList<EdgeData>"})]}),"\n"]}),"\n",(0,l.jsx)(n.h3,{id:"21-\u57fa\u4e8e\u5feb\u7167\u7684\u5b58\u50a8\u7ed3\u6784",children:"2.1 \u57fa\u4e8e\u5feb\u7167\u7684\u5b58\u50a8\u7ed3\u6784"}),"\n",(0,l.jsx)(n.p,{children:"TuGraph\u4e2d\u7684OlapOnDB\u7c7b\u80fd\u591f\u63d0\u4f9b\u6570\u636e\u201c\u5feb\u7167\u201d\uff0c\u5373\u5efa\u7acb\u4e00\u4e2a\u5bf9\u6307\u5b9a\u6570\u636e\u96c6\u7684\u5b8c\u5168\u53ef\u7528\u62f7\u8d1d\uff0c\u8be5\u62f7\u8d1d\u5305\u62ec\u76f8\u5e94\u6570\u636e\u5728\u67d0\u4e2a\u65f6\u95f4\u70b9\uff08\u62f7\u8d1d\u5f00\u59cb\u7684\u65f6\u95f4\u70b9\uff09\u7684\u955c\u50cf\u3002\u7531\u4e8eOLAP\u7684\u64cd\u4f5c\u4ec5\u6d89\u53ca\u8bfb\u64cd\u4f5c\u800c\u4e0d\u6d89\u53ca\u5199\u64cd\u4f5c\uff0cOlapOnDB\u4f1a\u4ee5\u4e00\u79cd\u66f4\u7d27\u51d1\u7684\u65b9\u5f0f\u5bf9\u6570\u636e\u8fdb\u884c\u6392\u5e03\uff0c\u5728\u8282\u7701\u7a7a\u95f4\u7684\u540c\u65f6\uff0c\u63d0\u9ad8\u6570\u636e\u8bbf\u95ee\u7684\u5c40\u90e8\u6027\u3002"}),"\n",(0,l.jsx)(n.h3,{id:"22-bsp\u8ba1\u7b97\u6a21\u578b",children:"2.2 BSP\u8ba1\u7b97\u6a21\u578b"}),"\n",(0,l.jsx)(n.p,{children:"TuGraph\u5728\u8ba1\u7b97\u7684\u8fc7\u7a0b\u4e2d\u4f7f\u7528\u4e86BSP\uff08Bulk Synchronous Parallel\uff09\u6a21\u578b\uff0c\u4f7f\u5f97\u8be5\u8fc7\u7a0b\u80fd\u591f\u5e76\u884c\u6267\u884c\uff0c\u6781\u5927\u7684\u63d0\u9ad8\u4e86\u7a0b\u5e8f\u8fd0\u884c\u6548\u7387\u3002"}),"\n",(0,l.jsx)(n.p,{children:"BSP\u8ba1\u7b97\u6a21\u578b\u7684\u6838\u5fc3\u601d\u8def\u4e3a\u8d85\u6b65\uff08Super Step\uff09\u7684\u63d0\u51fa\u548c\u4f7f\u7528\u3002\u5728OlapOnDB\u521b\u5efa\u540e\uff0c\u5728\u8be5\u6570\u636e\u4e0a\u7684\u8ba1\u7b97\u5206\u4e3a\u591a\u4e2a\u8d85\u6b65\uff0c\u6bd4\u5982PageRank\uff0c\u5206\u4e3a\u591a\u8f6e\u8fed\u4ee3\uff0c\u6bcf\u8f6e\u8fed\u4ee3\u5c31\u662f\u4e00\u4e2a\u8d85\u6b65\u3002\u4e0d\u540c\u8d85\u6b65\u4e4b\u95f4\u7528\u5b58\u5728\u663e\u5f0f\u540c\u6b65\uff0c\u4ece\u800c\u4fdd\u8bc1\u6240\u6709\u7ebf\u7a0b\u5728\u5b8c\u6210\u540c\u4e00\u8d85\u6b65\u540e\u540c\u65f6\u8fdb\u5165\u4e0b\u4e00\u4e2a\u8d85\u6b65\u3002\u5728\u4e00\u4e2a\u8d85\u6b65\u5185\u90e8\uff0c\u6240\u6709\u7684\u7ebf\u7a0b\u5f02\u6b65\u6267\u884c\uff0c\u5229\u7528\u5e76\u884c\u63d0\u5347\u8ba1\u7b97\u6548\u7387\u3002"}),"\n",(0,l.jsx)(n.p,{children:"\u5229\u7528BSP\u8ba1\u7b97\u6a21\u578b\u80fd\u591f\u6709\u6548\u907f\u514d\u6b7b\u9501\uff0c\u901a\u8fc7\u969c\u788d\u540c\u6b65\u7684\u65b9\u5f0f\u80fd\u591f\u4ee5\u786c\u4ef6\u65b9\u5f0f\u5b9e\u73b0\u7c97\u7c92\u5ea6\u7684\u5168\u5c40\u540c\u6b65\uff0c\u4f7f\u5f97\u56fe\u8ba1\u7b97\u80fd\u591f\u5e76\u884c\u5316\u6267\u884c\uff0c\u800c\u7a0b\u5e8f\u5458\u65e0\u9700\u5728\u540c\u6b65\u4e92\u65a5\u4e0a\u5927\u8d39\u5468\u7ae0\u3002"}),"\n",(0,l.jsx)(n.h2,{id:"3-\u7b97\u6cd5\u4e3e\u4f8b",children:"3. \u7b97\u6cd5\u4e3e\u4f8b"}),"\n",(0,l.jsxs)(n.p,{children:["\u5728\u8fd9\u91cc\u5bf9PageRank\u7b97\u6cd5\u5206\u5757\u505a\u89e3\u91ca\uff0c\u5927\u4f53\u4e0a\u5206\u4e3a\u4e3b\u51fd\u6570",(0,l.jsx)(n.code,{children:"Process"}),"\u548cPageRank\u7b97\u6cd5\u6d41\u7a0b",(0,l.jsx)(n.code,{children:"PageRank"}),"\u51fd\u6570"]}),"\n",(0,l.jsx)(n.h3,{id:"31-\u4e3b\u51fd\u6570",children:"3.1 \u4e3b\u51fd\u6570"}),"\n",(0,l.jsxs)(n.p,{children:["\u4e3b\u51fd\u6570\u8f93\u5165\u6709\u4e09\u4e2a\u53c2\u6570\uff0c",(0,l.jsx)(n.code,{children:"TuGraph"}),"\u6570\u636e\u5e93\u53c2\u6570",(0,l.jsx)(n.code,{children:"db"}),"\uff0c\u4ece\u7f51\u9875\u7aef\u83b7\u53d6\u7684\u8bf7\u6c42",(0,l.jsx)(n.code,{children:"request"}),"\uff0c\u7ed9\u7f51\u9875\u7aef\u7684\u8fd4\u56de\u503c",(0,l.jsx)(n.code,{children:"response"}),"\uff0c\u6574\u4f53\u6d41\u7a0b\u53ef\u4ee5\u5206\u4e3a\u4e00\u4e0b\u51e0\u6b65\uff1a"]}),"\n",(0,l.jsxs)(n.ol,{children:["\n",(0,l.jsx)(n.li,{children:"\u76f8\u5173\u53c2\u6570\u7684\u83b7\u53d6"}),"\n",(0,l.jsx)(n.li,{children:"\u5feb\u7167\u7c7b\u7684\u521b\u5efa"}),"\n",(0,l.jsx)(n.li,{children:"PageRank\u7b97\u6cd5\u4e3b\u6d41\u7a0b"}),"\n",(0,l.jsx)(n.li,{children:"\u7f51\u9875\u7aef\u8fd4\u56de\u503c\u7684\u83b7\u53d6\u548c\u53d1\u9001"}),"\n"]}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-C++",children:'extern "C" bool Process(GraphDB & db, const std::string & request, std::string & response) {\n    \n    // \u4ece\u7f51\u9875\u7aef\u8bf7\u6c42\u4e2d\u83b7\u53d6\u8fed\u4ee3\u6b21\u6570\uff08num_iterations\uff09\uff0c\n    int num_iterations = 20;\n    try {\n        json input = json::parse(request);\n        num_iterations = input["num_iterations"].get<int>();\n    } catch (std::exception & e) {\n        throw std::runtime_error("json parse error");\n        return false;\n    }\n\n    // \u8bfb\u4e8b\u52a1\u7684\u521b\u5efa\u4ee5\u53ca\u5feb\u7167\u7c7b\u7684\u521b\u5efa\n    auto txn = db.CreateReadTxn();\n    OlapOnDB<Empty> olapondb(\n        db,\n        txn,\n        SNAPSHOT_PARALLEL\n    );\n\t\n    // \u521b\u5efapr\u6570\u7ec4\u7528\u4e8e\u5b58\u50a8\u6bcf\u4e2a\u8282\u70b9\u7684pr\u503c\n    ParallelVector<double> pr = olapondb.AllocVertexArray<double>();\n    // pagerank\u7b97\u6cd5\u4e3b\u6d41\u7a0b\uff0c\u83b7\u53d6\u6bcf\u4e2a\u8282\u70b9\u7684pagerank\u503c\n    PageRankCore(olapondb, num_iterations, pr);\n    \n    auto all_vertices = olapondb.AllocVertexSubset();\n    all_vertices.Fill();\n    /*\n        \u51fd\u6570\u7528\u9014\uff1a\u4ece\u6240\u6709\u8282\u70b9\u4e2d\u83b7\u53d6pagerank\u503c\u6700\u5927\u7684\u8282\u70b9\u7f16\u53f7\n    \n        \u51fd\u6570\u6d41\u7a0b\u63cf\u8ff0\uff1a\u8be5\u51fd\u6570\u5bf9\u70b9\u96c6\u5408all_vertices\u4e2d\u6240\u6709\u4e3a1\u7684\u4f4d\u5bf9\u5e94\u7684\u8282\u70b9vi\uff08\u53c8\u79f0\u4e3a\u6d3b\u8dc3\u70b9\uff09\u6267\u884cFunc A\uff0c\u518d\u5c06Func A\u7684\u8fd4\u56de\u503c\u4f5c\u4e3aFunc B\u7684\u7b2c\u4e8c\u4e2a\u8f93\u5165\u53c2\u6570\uff0c\u5f97\u5230\u5c40\u90e8\u6700\u5927\u503c\uff08\u56e0\u4e3a\u7b2c\u4e00\u4e2a\u8f93\u5165\u53c2\u6570\u4e3a0\uff0c\u56e0\u6b64\u5b9e\u9645\u4e0a\u8fd4\u56de\u503c\u5c31\u662f\u6bcf\u4e2a\u8282\u70b9\u7684pagerank\u503c\uff09\uff0c\u6700\u540e\u518d\u5c06\u6240\u6709\u7ebf\u7a0b\u7684\u8fd4\u56de\u503c\u6c47\u603b\uff0c\u518d\u6b21 \u6267\u884cFunc B\u5f97\u5230\u5168\u5c40\u8fd4\u56de\u503c\uff0c\u5e76\u5b58\u5165max_pr_vi\u53d8\u91cf\u4e2d\n    */\n    size_t max_pr_vi = olapondb.ProcessVertexActive<size_t>(\n        \n        //Func A\n        [&](size_t vi) {\n            return vi;\n        },\n        all_vertices,\n        0,\n        \n        //Func B\n        [&](size_t a, size_t b) {\n            return pr[a] > pr[b] ? a : b;\n        }\n    );\n    \n    // \u7f51\u9875\u7aef\u8fd4\u56de\u503c\u7684\u83b7\u53d6\u548c\u53d1\u9001\n    json output;\n    output["max_pr_vid"] = olapondb.OriginalVid(max_pr_vi);\n    output["max_pr_val"] = pr[max_pr_vi];\n    response = output.dump();\n    return true;\n}\n'})}),"\n",(0,l.jsx)(n.h3,{id:"32-pagerank\u7b97\u6cd5\u6d41\u7a0b",children:"3.2 PageRank\u7b97\u6cd5\u6d41\u7a0b"}),"\n",(0,l.jsxs)(n.p,{children:[(0,l.jsx)(n.code,{children:"pagerank"}),"\u4e3b\u6d41\u7a0b\u6709\u4e24\u4e2a\u8f93\u5165\u53c2\u6570\uff0c\u5feb\u7167\u7c7b\uff08\u5b50\u56fe\uff09\u8fd8\u6709\u8fed\u4ee3\u6b21\u6570\uff0c\u6574\u4f53\u6d41\u7a0b\u53ef\u4ee5\u5206\u4e3a\u4ee5\u4e0b\u51e0\u6b65\uff1a"]}),"\n",(0,l.jsxs)(n.ol,{children:["\n",(0,l.jsx)(n.li,{children:"\u76f8\u5173\u6570\u636e\u7ed3\u6784\u7684\u521d\u59cb\u5316"}),"\n",(0,l.jsx)(n.li,{children:"\u6bcf\u4e2a\u8282\u70b9pagerank\u503c\u7684\u521d\u59cb\u5316"}),"\n",(0,l.jsx)(n.li,{children:"\u6bcf\u4e2a\u8282\u70b9pagerank\u503c\u7684\u8ba1\u7b97\uff0c\u6d3b\u8dc3\u70b9\u4e3a\u6240\u6709\u70b9\uff08\u610f\u5473\u7740\u6240\u6709\u70b9\u90fd\u9700\u8981\u8ba1\u7b97pagerank\u503c\uff09"}),"\n",(0,l.jsxs)(n.li,{children:["\u5f97\u5230\u6bcf\u4e2a\u8282\u70b9\u7ecf\u8fc7",(0,l.jsx)(n.code,{children:"num_iterations"}),"\u6b21\u8fed\u4ee3\u540e\u7684pagerank\u503c"]}),"\n"]}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-C++",children:'void PageRankCore(OlapBase<Empty>& graph, int num_iterations, ParallelVector<double>& curr) {\n    \n    // \u76f8\u5173\u6570\u636e\u7ed3\u6784\u7684\u521d\u59cb\u5316\n    auto all_vertices = olapondb.AllocVertexSubset();\n    all_vertices.Fill();\n    auto curr = olapondb.AllocVertexArray<double>();\n    auto next = olapondb.AllocVertexArray<double>();\n    size_t num_vertices = olapondb.NumVertices();\n    double one_over_n = (double)1 / num_vertices;\n\n    // \u6bcf\u4e2a\u8282\u70b9pagerank\u503c\u7684\u521d\u59cb\u5316\uff0c\u548c\u8be5\u8282\u70b9\u7684\u51fa\u5ea6\u6210\u53cd\u6bd4\n    double delta = graph.ProcessVertexActive<double>(\n        [&](size_t vi) {\n            curr[vi] = one_over_n;\n            if (olapondb.OutDegree(vi) > 0) {\n                curr[vi] /= olapondb.OutDegree(vi);\n            }\n            return one_over_n;\n        },\n        all_vertices);\n\n    // \u603b\u8fed\u4ee3\u8fc7\u7a0b\n    double d = (double)0.85;\n        for (int ii = 0;ii < num_iterations;ii ++) {\n        printf("delta(%d)=%lf\\n", ii, delta);\n        next.Fill((double)0);\n\n        /*\n            \u51fd\u6570\u7528\u9014\uff1a\u8ba1\u7b97\u6240\u6709\u8282\u70b9\u7684pagerank\u503c\n\n            \u51fd\u6570\u6d41\u7a0b\u63cf\u8ff0\uff1a\u8be5\u51fd\u6570\u7528\u4e8e\u8ba1\u7b97\u6240\u6709\u8282\u70b9\u7684pagerank\u503c\uff0c\u5bf9all_vertices\u4e2d\u6240\u6709\u4e3a1\u7684\u4f4d\u5bf9\u5e94\u7684\u8282\u70b9vi\u6267\u884cFunc C\uff0c\u5f97\u5230\u672c\u8f6e\u8fed\u4ee3\u4e2dvi\u7684pagerank\u503c\uff0c\u5e76\u8fd4\u56devi\u8282\u70b9\u7684pagerank\u53d8\u5316\u503c\uff0c\u6700\u7ec8\u7ecf\u8fc7\u51fd\u6570\u5185\u90e8\u5904\u7406\u6c47\u603b\u6240\u6709\u6d3b\u8dc3\u8282\u70b9\u7684\u603b\u53d8\u5316\u503c\u5e76\u8fd4\u56de\uff0c\u8be5\u503c\u88ab\u5b58\u50a8\u5728delta\u53d8\u91cf\u4e2d\n        */\n        delta = graph.ProcessVertexActive<double>(\n            // Func C\n            [&](size_t vi) {\n                double sum = 0;\n\n                // \u4ece\u90bb\u5c45\u4e2d\u83b7\u53d6\u5f53\u524d\u8282\u70b9\u7684pagerank\u503c\n                for (auto & edge : olapondb.InEdges(vi)) {\n                    size_t src = edge.neighbour;\n                    sum += curr[src];\n                }\n                next[vi] = sum;\n\n                // pagerank\u503c\u8ba1\u7b97\u6838\u5fc3\u516c\u5f0f\n                next[vi] = (1 - d) * one_over_n + d * next[vi];\n                if (ii == num_iterations - 1) {\n                    return (double)0;\n                } else {\n    \n                    // \u76f8\u5173\u4e2d\u95f4\u53d8\u91cf\u7edf\u8ba1\n                    if (olapondb.OutDegree(vi) > 0) {\n                        next[vi] /= olapondb.OutDegree(vi);\n                        return fabs(next[vi] - curr[vi]) * olapondb.OutDegree(vi);\n                    } else {\n                        return fabs(next[vi] - curr[vi]);\n                    }\n                }\n            },\n            all_vertices\n        );\n\n        // \u5c06\u672c\u8f6e\u8fed\u4ee3\u5f97\u5230\u7684pagerank\u503c\u8f93\u51fa\u4f5c\u4e3a\u4e0b\u4e00\u8f6e\u8fed\u4ee3\u7684\u8f93\u5165\n        curr.Swap(next);\n    }\n}\n'})}),"\n",(0,l.jsx)(n.h2,{id:"4-\u5176\u4ed6\u5e38\u7528\u51fd\u6570\u529f\u80fd\u63cf\u8ff0",children:"4. \u5176\u4ed6\u5e38\u7528\u51fd\u6570\u529f\u80fd\u63cf\u8ff0"}),"\n",(0,l.jsx)(n.h3,{id:"41-\u4e8b\u52a1\u7684\u521b\u5efa",children:"4.1 \u4e8b\u52a1\u7684\u521b\u5efa"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-C++",children:"//\u8bfb\u4e8b\u52a1\u7684\u521b\u5efa\nauto txn = db.CreateReadTxn();\n\n//\u5199\u4e8b\u52a1\u7684\u521b\u5efa\nauto txn = db.CreateWriteTxn();\n"})}),"\n",(0,l.jsx)(n.h3,{id:"42-\u5e76\u884c\u5316\u521b\u5efa\u6709\u5411\u56fe",children:"4.2 \u5e76\u884c\u5316\u521b\u5efa\u6709\u5411\u56fe"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-C++",children:"OlapOnDB<Empty> olapondb(\n    db,\n    txn,\n    SNAPSHOT_PARALLEL\n)\n"})}),"\n",(0,l.jsx)(n.h3,{id:"43-\u5e76\u884c\u5316\u521b\u5efa\u65e0\u5411\u56fe",children:"4.3 \u5e76\u884c\u5316\u521b\u5efa\u65e0\u5411\u56fe"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-C++",children:"OlapOnDB<Empty> olapondb(\n    db,\n    txn,\n    SNAPSHOT_PARALLEL | SNAPSHOT_UNDIRECTED\n)\n"})}),"\n",(0,l.jsx)(n.h3,{id:"44-\u83b7\u53d6\u51fa\u5ea6",children:"4.4 \u83b7\u53d6\u51fa\u5ea6"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-C++",children:"size_t OutDegree(size_t vid)\n"})}),"\n",(0,l.jsx)(n.h3,{id:"45-\u83b7\u53d6\u5165\u5ea6",children:"4.5 \u83b7\u53d6\u5165\u5ea6"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-C++",children:"size_t InDegree(size_t vid)\n"})}),"\n",(0,l.jsx)(n.h3,{id:"46-\u83b7\u53d6\u51fa\u8fb9\u96c6\u5408",children:"4.6 \u83b7\u53d6\u51fa\u8fb9\u96c6\u5408"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-C++",children:'/*\n    \u51fd\u6570\u540d\u79f0\uff1aAdjList<EdgeData> OutEdges(size_t vid)\n    \u6570\u636e\u7ed3\u6784:\n        AdjList \u53ef\u4ee5\u7406\u89e3\u4e3a\u7c7b\u578b\u4e3aAdjUnit\u7ed3\u6784\u4f53\u7684\u6570\u7ec4\n        AdjUnit \u6709\u4e24\u4e2a\u6210\u5458\u53d8\u91cf\uff1a 1. size_t neighbour 2. edge_data\uff0c\u5176\u4e2dneighbour\u8868\u793a\u8be5\u51fa\u8fb9\u6307\u5411\u7684\u76ee\u6807\u8282\u70b9\u7f16\u53f7\uff0c\u5982\u679c\u4e3a\u6709\u6743\u56fe\uff0c\u5219edge_data\u6570\u636e\u7c7b\u578b\u548c\u8f93\u5165\u6587\u4ef6\u4e2d\u8fb9\u7684\u6743\u91cd\u503c\u76f8\u540c\uff0c\u5426\u5219\u6570\u636e\u7c7b\u578b\u4e3aEmpty\n\n    \u4f7f\u7528\u793a\u4f8b\uff1a\u8f93\u51fa\u8282\u70b9vid\u7684\u6240\u6709\u51fa\u5ea6\u90bb\u5c45\n*/\nfor (auto & edge : olapondb.OutEdges(vid)) {\n    size_t dst = edge.neighbour;\n    printf("src = %lu,dst = %lu\\n",vid,dst);\n}\n'})}),"\n",(0,l.jsx)(n.h3,{id:"47-\u83b7\u53d6\u5165\u8fb9\u96c6\u5408",children:"4.7 \u83b7\u53d6\u5165\u8fb9\u96c6\u5408"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-C++",children:'AdjList<EdgeData> InEdges(size_t vid)\n\n// \u4f7f\u7528\u793a\u4f8b\uff1a\u8f93\u51fa\u8282\u70b9vid\u7684\u6240\u6709\u5165\u5ea6\u90bb\u5c45\nfor (auto & edge : olapondb.InEdges(vid)) {\n    size_t dst = edge.neighbour;\n    printf("src = %lu,dst = %lu\\n",vid,dst);\n}\n'})}),"\n",(0,l.jsx)(n.h3,{id:"48-\u83b7\u53d6tugraph\u4e2d\u8282\u70b9\u5bf9\u5e94olapondb\u7684\u8282\u70b9\u7f16\u53f7",children:"4.8 \u83b7\u53d6TuGraph\u4e2d\u8282\u70b9\u5bf9\u5e94OlapOnDB\u7684\u8282\u70b9\u7f16\u53f7"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-C++",children:"size_t OriginalVid(size_t vid)\n\n// \u5907\u6ce8\uff1a TuGraph\u4e2d\u8f93\u5165\u7684\u8282\u70b9\u7f16\u53f7\u53ef\u4ee5\u662f\u975e\u6570\u5b57\uff0c\u6bd4\u5982\u4eba\u540d\u7b49\uff0c\u5728\u751f\u6210OlapOnDB\u5b50\u56fe\u7684\u65f6\u5019\uff0c\u4f1a\u5c06\u4eba\u540d\u7b49\u8f6c\u5316\u4e3a\u6570\u5b57\u8fdb\u884c\u540e\u7eed\u5904\u7406\uff0c\u56e0\u6b64\u8be5\u65b9\u6cd5\u53ef\u80fd\u4e0d\u9002\u7528\u4e8e\u67d0\u4e9b\u7279\u5b9a\u573a\u666f\n"})}),"\n",(0,l.jsx)(n.h3,{id:"49-\u83b7\u53d6olapondb\u4e2d\u8282\u70b9\u5bf9\u5e94tugraph\u7684\u8282\u70b9\u7f16\u53f7",children:"4.9 \u83b7\u53d6OlapOnDB\u4e2d\u8282\u70b9\u5bf9\u5e94TuGraph\u7684\u8282\u70b9\u7f16\u53f7"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-C++",children:"size_t MappedVid(size_t original_vid)\n"})}),"\n",(0,l.jsx)(n.h3,{id:"410-\u6d3b\u8dc3\u70b9\u7684\u63cf\u8ff0",children:"4.10 \u6d3b\u8dc3\u70b9\u7684\u63cf\u8ff0"}),"\n",(0,l.jsx)(n.p,{children:"\u6d3b\u8dc3\u70b9\u6307\u7684\u662f\u5728\u6279\u5904\u7406\u51fd\u6570\u4e2d\u9700\u8981\u5904\u7406\u7684\u70b9\uff0c\u5728\u672c\u4f8b\u5b50\u4e2d\u53ea\u662f\u8f93\u51fa\u4e86\u6d3b\u8dc3\u70b9\u7684\u7f16\u53f7\uff0c\u5e76\u4e14\u6c47\u603b\u6d3b\u8dc3\u70b9\u7684\u6570\u91cf\uff1a"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{className:"language-C++",children:'ParallelBitset temp = 000111;\t//\u5f53\u524d\u6d3b\u8dc3\u70b9\u4e3a3\uff0c4\uff0c5\u53f7\u70b9\n\nsize_t delta = ForEachActiveVertex<double>(\n    //void c\n    [&](size_t vi) {\n        printf("active_vertexId = %lu\\n",vi);\n        return 1;\n    },\n    all_vertices\n);\n'})}),"\n",(0,l.jsx)(n.p,{children:"\u51fd\u6570\u7684\u8fd0\u884c\u7ed3\u679c\u663e\u800c\u6613\u89c1\uff0c\u56e0\u4e3a\u591a\u7ebf\u7a0b\u7684\u5173\u7cfb\uff0c\u4e00\u4e0b\u8f93\u51fa\u987a\u5e8f\u53ef\u80fd\u6709\u6240\u53d8\u5316\uff1a"}),"\n",(0,l.jsx)(n.pre,{children:(0,l.jsx)(n.code,{children:"active_vertexId = 3\nactive_vertexId = 4\nactive_vertexId = 5\n"})}),"\n",(0,l.jsxs)(n.p,{children:["\u5c40\u90e8\u8fd4\u56de\u503c\u5747\u4e3a1\uff0c\u8be5\u51fd\u6570\u4f1a\u5728\u4fdd\u8bc1\u7ebf\u7a0b\u5b89\u5168\u7684\u60c5\u51b5\u4e0b\u5c06\u6240\u6709\u7684\u5c40\u90e8\u8fd4\u56de\u503c\u7d2f\u52a0\u5f97\u5230\u6700\u7ec8\u7684\u8fd4\u56de\u503c\uff0c\u5e76\u5b58\u50a8\u5728",(0,l.jsx)(n.code,{children:"delta"}),"\u53d8\u91cf\u4e2d\uff0c\u8be5\u503c\u6700\u7ec8\u4e3a3"]})]})}function p(e={}){const{wrapper:n}={...(0,i.R)(),...e.components};return n?(0,l.jsx)(n,{...e,children:(0,l.jsx)(c,{...e})}):c(e)}}}]);