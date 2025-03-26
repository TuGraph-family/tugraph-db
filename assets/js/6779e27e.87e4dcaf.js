"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[643],{27841:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>i,contentTitle:()=>t,default:()=>c,frontMatter:()=>a,metadata:()=>l,toc:()=>o});var s=r(74848),d=r(28453);const a={},t="OLAP API",l={id:"developer-manual/interface/olap/tutorial",title:"OLAP API",description:"\u672c\u6587\u6863\u662f\u4e3aTuGraph\u7684\u7528\u6237\u8bbe\u8ba1\u7684\u5f15\u5bfc\u7a0b\u5e8f\uff0c\u7528\u6237\u5728\u9605\u8bfb\u8be6\u7ec6\u7684\u6587\u6863\u4e4b\u524d\uff0c\u5e94\u8be5\u9996\u5148\u9605\u8bfb\u8be5\u6587\u6863\uff0c\u5bf9TuGraph\u7684\u56fe\u8ba1\u7b97\u8fd0\u884c\u6d41\u7a0b\u6709\u4e00\u4e2a\u5927\u81f4\u7684\u4e86\u89e3\uff0c\u4e4b\u540e\u518d\u9605\u8bfb\u8be6\u7ec6\u6587\u6863\u4f1a\u66f4\u52a0\u65b9\u4fbf\u3002\u5f15\u5bfc\u7a0b\u5e8f\u662f\u57fa\u4e8eTugraph\u7684\u4e00\u4e2a\u7b80\u5355\u7684BFS(\u5bbd\u5ea6\u4f18\u5148\u641c\u7d22)\u7a0b\u5e8f\u5b9e\u4f8b\uff0c\u6211\u4eec\u5c06\u91cd\u70b9\u4ecb\u7ecd\u5176\u4f7f\u7528\u65b9\u5f0f\u3002",source:"@site/versions/version-4.0.1/zh-CN/source/5.developer-manual/6.interface/2.olap/1.tutorial.md",sourceDirName:"5.developer-manual/6.interface/2.olap",slug:"/developer-manual/interface/olap/tutorial",permalink:"/tugraph-db/zh/4.0.1/developer-manual/interface/olap/tutorial",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:1,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"LIMIT",permalink:"/tugraph-db/zh/4.0.1/developer-manual/interface/query/gql/clauses/limit"},next:{title:"OlapBase API",permalink:"/tugraph-db/zh/4.0.1/developer-manual/interface/olap/olap-base-api"}},i={},o=[{value:"1. TuGraph \u56fe\u5206\u6790\u5f15\u64ce\u4ecb\u7ecd",id:"1-tugraph-\u56fe\u5206\u6790\u5f15\u64ce\u4ecb\u7ecd",level:2},{value:"2. Procedure \u7f16\u8bd1\u4e0e\u8fd0\u884c",id:"2-procedure-\u7f16\u8bd1\u4e0e\u8fd0\u884c",level:2},{value:"C++:",id:"c",level:3},{value:"Python:",id:"python",level:3},{value:"3. Embed \u7f16\u8bd1\u4e0e\u8fd0\u884c",id:"3-embed-\u7f16\u8bd1\u4e0e\u8fd0\u884c",level:2},{value:"C++:",id:"c-1",level:3},{value:"Python\uff1a",id:"python-1",level:3},{value:"4. Standalone \u7f16\u8bd1\u4e0e\u8fd0\u884c",id:"4-standalone-\u7f16\u8bd1\u4e0e\u8fd0\u884c",level:2},{value:"C++:",id:"c-2",level:3},{value:"Python:",id:"python-2",level:3}];function p(e){const n={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",li:"li",p:"p",pre:"pre",ul:"ul",...(0,d.R)(),...e.components};return(0,s.jsxs)(s.Fragment,{children:[(0,s.jsx)(n.header,{children:(0,s.jsx)(n.h1,{id:"olap-api",children:"OLAP API"})}),"\n",(0,s.jsxs)(n.blockquote,{children:["\n",(0,s.jsxs)(n.p,{children:["\u672c\u6587\u6863\u662f\u4e3aTuGraph\u7684\u7528\u6237\u8bbe\u8ba1\u7684\u5f15\u5bfc\u7a0b\u5e8f\uff0c\u7528\u6237\u5728\u9605\u8bfb\u8be6\u7ec6\u7684\u6587\u6863\u4e4b\u524d\uff0c\u5e94\u8be5\u9996\u5148\u9605\u8bfb\u8be5\u6587\u6863\uff0c\u5bf9TuGraph\u7684\u56fe\u8ba1\u7b97\u8fd0\u884c\u6d41\u7a0b\u6709\u4e00\u4e2a\u5927\u81f4\u7684\u4e86\u89e3\uff0c\u4e4b\u540e\u518d\u9605\u8bfb\u8be6\u7ec6\u6587\u6863\u4f1a\u66f4\u52a0\u65b9\u4fbf\u3002\u5f15\u5bfc\u7a0b\u5e8f\u662f\u57fa\u4e8eTugraph\u7684\u4e00\u4e2a\u7b80\u5355\u7684",(0,s.jsx)(n.a,{href:"https://en.wikipedia.org/wiki/Breadth-first_search",title:"wikipedia",children:"BFS(\u5bbd\u5ea6\u4f18\u5148\u641c\u7d22)"}),"\u7a0b\u5e8f\u5b9e\u4f8b\uff0c\u6211\u4eec\u5c06\u91cd\u70b9\u4ecb\u7ecd\u5176\u4f7f\u7528\u65b9\u5f0f\u3002"]}),"\n"]}),"\n",(0,s.jsx)(n.h2,{id:"1-tugraph-\u56fe\u5206\u6790\u5f15\u64ce\u4ecb\u7ecd",children:"1. TuGraph \u56fe\u5206\u6790\u5f15\u64ce\u4ecb\u7ecd"}),"\n",(0,s.jsx)(n.p,{children:"TuGraph\u7684\u56fe\u5206\u6790\u5f15\u64ce\uff0c\u9762\u5411\u7684\u573a\u666f\u4e3b\u8981\u662f\u5168\u56fe/\u5168\u91cf\u6570\u636e\u5206\u6790\u7c7b\u7684\u4efb\u52a1\u3002\u501f\u52a9TuGraph\u7684 C++ / Python \u56fe\u5206\u6790\u5f15\u64ce API \uff0c\u7528\u6237\u53ef\u4ee5\u5bf9\u4e0d\u540c\u6570\u636e\u6765\u6e90\u7684\u56fe\u6570\u636e\u5feb\u901f\u5bfc\u51fa\u4e00\u4e2a\u5f85\u5904\u7406\u7684\u590d\u6742\u5b50\u56fe\uff0c\u7136\u540e\u5728\u8be5\u5b50\u56fe\u4e0a\u8fd0\u884c\u8bf8\u5982PageRank\u3001LPA\u3001WCC\u7b49\u8fed\u4ee3\u5f0f\u56fe\u7b97\u6cd5\uff0c\u6700\u540e\u6839\u636e\u8fd0\u884c\u7ed3\u679c\u505a\u51fa\u76f8\u5e94\u7684\u5bf9\u7b56\u3002"}),"\n",(0,s.jsx)(n.p,{children:"\u5728TuGraph\u4e2d\uff0c\u5bfc\u51fa\u548c\u8ba1\u7b97\u8fc7\u7a0b\u5747\u53ef\u4ee5\u901a\u8fc7\u5728\u5185\u5b58\u4e2d\u5e76\u884c\u5904\u7406\u7684\u65b9\u5f0f\u8fdb\u884c\u52a0\u901f\uff0c\u4ece\u800c\u8fbe\u5230\u8fd1\u4e4e\u5b9e\u65f6\u7684\u5904\u7406\u5206\u6790\uff0c\u548c\u4f20\u7edf\u65b9\u6cd5\u76f8\u6bd4\uff0c\u5373\u907f\u514d\u4e86\u6570\u636e\u5bfc\u51fa\u843d\u76d8\u7684\u5f00\u9500\uff0c\u53c8\u80fd\u4f7f\u7528\u7d27\u51d1\u7684\u56fe\u6570\u636e\u7ed3\u6784\u83b7\u5f97\u8ba1\u7b97\u7684\u7406\u60f3\u6027\u80fd\u3002"}),"\n",(0,s.jsx)(n.p,{children:"TuGraph\u56fe\u8ba1\u7b97\u7cfb\u7edf\u793e\u533a\u7248\u5185\u7f6e6\u4e2a\u7b97\u6cd5\uff0c\u5546\u4e1a\u7248\u5185\u7f6e\u4e8625\u79cd\u7b97\u6cd5\uff0c\u7528\u6237\u51e0\u4e4e\u4e0d\u9700\u8981\u81ea\u5df1\u5b9e\u73b0\u5177\u4f53\u7684\u56fe\u8ba1\u7b97\u8fc7\u7a0b\u3002\u5176\u8be6\u7ec6\u4ecb\u7ecd\u53ef\u53c2\u8003algorithms.md\u3002"}),"\n",(0,s.jsx)(n.p,{children:"\u6839\u636e\u6570\u636e\u6765\u6e90\u53ca\u5b9e\u73b0\u4e0d\u540c\uff0c\u53ef\u5206\u4e3aProcedure\u3001Embed\u548cStandalone\u4e09\u79cd\u8fd0\u884c\u65b9\u5f0f\uff0c\u5747\u7ee7\u627f\u4e8eOlapBase API\uff0cOlapBase API\u63a5\u53e3\u6587\u6863\u53ef\u53c2\u8003olapbase-api.md\u3002"}),"\n",(0,s.jsx)(n.p,{children:"\u5176\u4e2dProcedure\u548cEmbed\u7684\u6570\u636e\u6765\u6e90\u662f\u56fe\u6570\u636e\u5e93\u4e2d\u9884\u52a0\u8f7d\u7684db\u6570\u636e\uff0c\u53ef\u4ee5\u5206\u522b\u7f16\u8bd1\u751f\u6210tugraph-web\u52a0\u8f7d\u4f7f\u7528\u7684.so\u6587\u4ef6\u548c\u540e\u53f0\u7ec8\u7aef\u4f7f\u7528\u7684embed\u6587\u4ef6\uff0c\u8f93\u5165\u7684\u56fe\u6570\u636e\u5747\u901a\u8fc7db\u7684\u52a0\u8f7d\u5f62\u5f0f\uff0c\u5176\u63a5\u53e3\u6587\u6863\u53ef\u53c2\u8003olapondb-api.md\u3002\nStandalone\u7528\u4e8e\u7f16\u8bd1\u751f\u6210standalone\u6587\u4ef6\uff0c\u533a\u522b\u4e8e\u524d\u8005\uff0c\u8be5\u6587\u4ef6\u7684\u8f93\u5165\u56fe\u6570\u636e\u901a\u8fc7txt\u3001\u4e8c\u8fdb\u5236\u3001ODPS\u6587\u4ef6\u7684\u5f62\u5f0f\u52a0\u8f7d\uff0c\u5176\u63a5\u53e3\u6587\u6863\u53ef\u53c2\u8003olapondisk-api.md\u3002"}),"\n",(0,s.jsx)(n.h2,{id:"2-procedure-\u7f16\u8bd1\u4e0e\u8fd0\u884c",children:"2. Procedure \u7f16\u8bd1\u4e0e\u8fd0\u884c"}),"\n",(0,s.jsx)(n.p,{children:"\u8be5\u79cd\u65b9\u5f0f\u4e3b\u8981\u7528\u4e8etugraph-web\u754c\u9762\u8fdb\u884c\u53ef\u89c6\u5316\u52a0\u8f7d\u53ca\u8fd0\u884c\u3002\u4f7f\u7528\u65b9\u6cd5\u5982\u4e0b\uff1a"}),"\n",(0,s.jsx)(n.h3,{id:"c",children:"C++:"}),"\n",(0,s.jsxs)(n.p,{children:["\u5728tugraph-db/procedures \u76ee\u5f55\u4e0b\u6267\u884c",(0,s.jsx)(n.code,{children:"bash make_so_cpp.sh bfs"}),"\u5373\u53ef\u5728tugraph-db/procedures\u76ee\u5f55\u4e0b\u5f97\u5230bfs.so\u6587\u4ef6\uff0c\u5c06\u8be5\u6587\u4ef6\u4ee5\u63d2\u4ef6\u5f62\u5f0f\u4e0a\u4f20\u81f3tugraph-web\uff0c\u8f93\u5165\u53c2\u6570\u540e\u5373\u53ef\u6267\u884c\u3002"]}),"\n",(0,s.jsx)(n.h3,{id:"python",children:"Python:"}),"\n",(0,s.jsx)(n.p,{children:"\u5728tugraph-web\u7684\u524d\u7aef\u5c06python\u6587\u4ef6\u4ee5\u63d2\u4ef6\u5f62\u5f0f\u4e0a\u4f20\uff0c\u8f93\u5165\u53c2\u6570\u540e\u5373\u53ef\u6267\u884c\u3002"}),"\n",(0,s.jsxs)(n.p,{children:["\u793a\u4f8b\uff1a\n\u5728tugraph-db/procedures \u7f16\u8bd1.so\u7b97\u6cd5\u6587\u4ef6\n",(0,s.jsx)(n.code,{children:"bash make_so_cpp.sh bfs"})]}),"\n",(0,s.jsx)(n.p,{children:"\u5c06bfs.so\uff08\u6216tugraph-db/procedures/algo_cython/bfs.py\uff09\u6587\u4ef6\u4ee5\u63d2\u4ef6\u5f62\u5f0f\u52a0\u8f7d\u81f3tugraph-web\u540e\uff0c\u8f93\u5165\u5982\u4e0bjson\u53c2\u6570\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-json",children:'{\n    "root_id":"0",\n    "label":"node",\n    "field":"id"\n}\n'})}),"\n",(0,s.jsx)(n.p,{children:"\u5373\u53ef\u5f97\u5230\u8fd4\u56de\u7ed3\u679c\u5982\u4e0b\u3002"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-json",children:'{\n  "core_cost": 0.013641119003295898,\n  "found_vertices": 3829,\n  "num_edges": 88234,\n  "num_vertices": 4039,\n  "output_cost": 8.821487426757813e-06,\n  "prepare_cost": 0.03479194641113281,\n  "total_cost": 0.04844188690185547\n}\n'})}),"\n",(0,s.jsx)(n.p,{children:"\u8f93\u51fa\u5185\u5bb9\u89e3\u91ca\u5982\u4e0b\uff1a"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"core_cost: \u8868\u793a\u7b97\u6cd5\u8fd0\u884c\u6240\u9700\u8981\u7684\u65f6\u95f4\u3002"}),"\n",(0,s.jsx)(n.li,{children:"found_vertices: \u8868\u793a\u67e5\u627e\u5230\u70b9\u7684\u4e2a\u6570\u3002"}),"\n",(0,s.jsx)(n.li,{children:"num_edges: \u8868\u793a\u8be5\u56fe\u6570\u636e\u7684\u8fb9\u6570\u91cf\u3002"}),"\n",(0,s.jsx)(n.li,{children:"num_vertices: \u8868\u793a\u8be5\u56fe\u6570\u636e\u70b9\u7684\u6570\u91cf\u3002"}),"\n",(0,s.jsx)(n.li,{children:"output_cost: \u8868\u793a\u7b97\u6cd5\u7ed3\u679c\u5199\u56dedb\u6240\u9700\u8981\u7684\u65f6\u95f4\u3002"}),"\n",(0,s.jsx)(n.li,{children:"prepare_cost: \u8868\u793a\u9884\u5904\u7406\u9636\u6bb5\u6240\u9700\u8981\u7684\u65f6\u95f4\u3002\u9884\u5904\u7406\u9636\u6bb5\u7684\u5de5\u4f5c\uff1a\u52a0\u8f7d\u53c2\u6570\u3001\u56fe\u6570\u636e\u52a0\u8f7d\u3001\u7d22\u5f15\u521d\u59cb\u5316\u7b49\u3002"}),"\n",(0,s.jsx)(n.li,{children:"total_cost: \u8868\u793a\u6267\u884c\u8be5\u7b97\u6cd5\u6574\u4f53\u8fd0\u884c\u65f6\u95f4\u3002"}),"\n"]}),"\n",(0,s.jsx)(n.p,{children:"make_so.sh\u6587\u4ef6\u4ecb\u7ecd\uff1a\u8be5\u6587\u4ef6\u7528\u4e8e\u5c06TuGraph-OLAP\u6240\u6d89\u53ca\u5230\u7684\u56fe\u7b97\u6cd5\u6587\u4ef6\u7f16\u8bd1\u6210\u4e00\u4e2a\u53ef\u4f9btugraph-web\u4f7f\u7528\u7684.so\u6587\u4ef6\u3002"}),"\n",(0,s.jsx)(n.h2,{id:"3-embed-\u7f16\u8bd1\u4e0e\u8fd0\u884c",children:"3. Embed \u7f16\u8bd1\u4e0e\u8fd0\u884c"}),"\n",(0,s.jsx)(n.p,{children:"\u8be5\u79cd\u65b9\u5f0f\u4e3b\u8981\u7528\u4e8eTuGraph\u5728\u540e\u53f0\u7a0b\u5e8f\u4e2d\u5bf9\u9884\u52a0\u8f7d\u7684db\u56fe\u6570\u636e\u8fdb\u884c\u7b97\u6cd5\u5206\u6790\u3002\u5176\u4f7f\u7528\u65b9\u6cd5\u5982\u4e0b\uff1a\n\u5728tugraph-db/procedures \u76ee\u5f55\u4e0b\u5bf9embed_main.cpp\u6587\u4ef6\u5b8c\u5584\uff0c\u8865\u5145\u6570\u636e\u540d\u79f0\u3001\u8f93\u5165\u53c2\u6570\u3001\u6570\u636e\u8def\u5f84\u7b49\u4fe1\u606f\uff0c\u793a\u4f8b\u5982\u4e0b\uff1a"}),"\n",(0,s.jsx)(n.h3,{id:"c-1",children:"C++:"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-C++",children:'#include <iostream>\n#include "lgraph/lgraph.h"\n#include "lgraph/olap_base.h"\nusing namespace std;\n\nextern "C" bool Process(lgraph_api::GraphDB &db, const std::string &request, std::string &response);\n\nint main(int argc, char **argv) {\n    // db_path\u8868\u793a\u9884\u52a0\u8f7d\u56fe\u6570\u636e\u5b58\u653e\u7684\u8def\u5f84\n    std::string db_path = "../fb_db/";\n    if (argc > 1)\n        db_path = argv[1];\n    lgraph_api::Galaxy g(db_path);\n    g.SetCurrentUser("admin", "73@TuGraph");\n    // \u6307\u5b9a\u56fe\u6570\u636e\u7684\u540d\u79f0\n    lgraph_api::GraphDB db = g.OpenGraph("fb_db");\n    std::string resp;\n    // \u4ee5json\u5f62\u5f0f\u8f93\u5165\u7b97\u6cd5\u53c2\u6570\n    bool r = Process(db, "{\\"root_id\\":\\"0\\", \\"label\\":\\"node\\",\\"field\\":\\"id\\"}", resp);\n    cout << r << endl;\n    cout << resp << endl;\n    return 0;\n}\n'})}),"\n",(0,s.jsxs)(n.p,{children:["\u4fdd\u5b58\u540e\u5728tugraph-db/procedures \u76ee\u5f55\u4e0b\u6267\u884c",(0,s.jsx)(n.code,{children:"bash make_embed.sh bfs"}),"\u5373\u53ef\u5728tugraph-db/procedures/algo_cpp \u76ee\u5f55\u4e0b\u5f97\u5230bfs_procedure\u6587\u4ef6\u3002"]}),"\n",(0,s.jsxs)(n.p,{children:["\u5728tugraph-db/procedures \u6587\u4ef6\u5939\u4e0b\u6267\u884c",(0,s.jsx)(n.code,{children:"./algo_cpp/bfs_procedure"})," \u5373\u53ef\u5f97\u5230\u8fd4\u56de\u7ed3\u679c\uff1a"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-json",children:'{\n  "core_cost":0.025603055953979492,\n  "found_vertices":3829,\n  "num_edges":88234,\n  "num_vertices":4039,\n  "output_cost":9.059906005859375e-06,\n  "prepare_cost":0.056738853454589844,\n  "total_cost":0.0823509693145752\n}\n'})}),"\n",(0,s.jsx)(n.p,{children:"\u53c2\u6570\u89e3\u91ca\u540c\u4e0a\u3002"}),"\n",(0,s.jsx)(n.h3,{id:"python-1",children:"Python\uff1a"}),"\n",(0,s.jsxs)(n.p,{children:["\u5728tugraph-db/procedures\u6587\u4ef6\u5939\u4e0b\u6267\u884c\n",(0,s.jsx)(n.code,{children:"bash make_so_cython.sh bfs"}),"\n\u6216\u5728tugraph-db/procedures/algo_cython\u6587\u4ef6\u5939\u4e0b\u6267\u884c\n",(0,s.jsx)(n.code,{children:"python3 setup.py build_ext -i"}),"\n\u5f97\u5230bfs.so\u540e\uff0c\u5728Python\u4e2dimport bfs\u53ef\u4f7f\u7528\uff0c\u5982tugraph-db/procedures/run_embed.py\u6240\u793a"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'# tugraph-db/procedures/run_embed.py\nfrom lgraph_db_python import *\n\nimport bfs as python_plugin\n\nif __name__ == "__main__":\n    galaxy = PyGalaxy("../build/output/lgraph_db")\n    galaxy.SetCurrentUser("admin", "73@TuGraph")\n    db = galaxy.OpenGraph("default", False)\n    res = python_plugin.Process(db, "{\\"root_id\\":\\"0\\", \\"label\\":\\"node\\",\\"field\\":\\"id\\"}".encode(\'utf-8\'))\n    print(res)\n    del db\n    del galaxy\n'})}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7\u5982\u4e0b\u547d\u4ee4\u6267\u884c"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-bash",children:"python3 run_embed.py\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u8f93\u51fa\u7ed3\u679c\u4e0eC++\u76f8\u540c\u3002"}),"\n",(0,s.jsx)(n.h2,{id:"4-standalone-\u7f16\u8bd1\u4e0e\u8fd0\u884c",children:"4. Standalone \u7f16\u8bd1\u4e0e\u8fd0\u884c"}),"\n",(0,s.jsxs)(n.p,{children:["\u8be5\u6587\u4ef6\u4e3b\u8981\u7528\u4e8e\u5728\u7ec8\u7aef\u5904\u76f4\u63a5\u52a0\u8f7d\u56fe\u6570\u636e\uff0c\u5e76\u8fd0\u884c\u6253\u5370\u8f93\u51fa\u7ed3\u679c\u3002\u4f7f\u7528\u65b9\u6cd5\u5982\u4e0b\uff1a\n\u5728tugraph-db/build\u76ee\u5f55\u4e0b\u6267\u884c",(0,s.jsx)(n.code,{children:"make bfs_standalone"})," (\u9700\u8981\u5728g++\u9ed8\u8ba4include\u8def\u5f84\u4e2d\u5305\u542bboost/sort/sort.hpp)\u5373\u53ef\u5f97\u5230bfs_standalone\u6587\u4ef6,\u8be5\u6587\u4ef6\u751f\u6210\u4e8etugraph-db/build/output/algo\u6587\u4ef6\u5939\u4e0b\u3002\n\u8fd0\u884c\u65b9\u5f0f\uff1a\u5728tugraph-db/build\u76ee\u5f55\u4e0b\u6267\u884c",(0,s.jsx)(n.code,{children:"./output/algo/bfs_standalone -\u2013type [type] \u2013-input_dir [input_dir] --id_mapping [id_mapping] -\u2013vertices [vertices] --root [root] \u2013-output_dir [output_dir]"}),"\u5373\u53ef\u8fd0\u884c\u3002"]}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"[type]"}),"\uff1a\u8868\u793a\u8f93\u5165\u56fe\u6587\u4ef6\u7684\u7c7b\u578b\u6765\u6e90\uff0c\u5305\u542btext\u6587\u672c\u6587\u4ef6\u3001BINARY_FILE\u4e8c\u8fdb\u5236\u6587\u4ef6\u548cODPS\u6e90\u3002"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"[input_dir]"}),"\uff1a\u8868\u793a\u8f93\u5165\u56fe\u6587\u4ef6\u7684\u6587\u4ef6\u5939\u8def\u5f84\uff0c\u6587\u4ef6\u5939\u4e0b\u53ef\u5305\u542b\u4e00\u4e2a\u6216\u591a\u4e2a\u8f93\u5165\u6587\u4ef6\u3002TuGraph\u5728\u8bfb\u53d6\u8f93\u5165\u6587\u4ef6\u65f6\u4f1a\u8bfb\u53d6[input_dir]\u4e0b\u7684\u6240\u6709\u6587\u4ef6\uff0c\u8981\u6c42[input_dir]\u4e0b\u53ea\u80fd\u5305\u542b\u8f93\u5165\u6587\u4ef6\uff0c\u4e0d\u80fd\u5305\u542b\u5176\u5b83\u6587\u4ef6\u3002\u53c2\u6570\u4e0d\u53ef\u7701\u7565\u3002"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"[id_mapping]"}),"\uff1a\u5f53\u8bfb\u5165\u8fb9\u8868\u65f6\uff0c\u662f\u5426\u5bf9\u8f93\u5165\u6570\u636e\u505aid\u6620\u5c04\uff0c\u4f7f\u8fbe\u5230\u7b26\u5408\u7b97\u6cd5\u8fd0\u884c\u7684\u5f62\u5f0f\u30021\u4e3a\u9700\u8981\u505aid\u6620\u5c04\uff0c0\u4e3a\u4e0d\u9700\u8981\u505a\u3002\u8be5\u8fc7\u7a0b\u4f1a\u6d88\u8017\u4e00\u5b9a\u65f6\u95f4\u3002\u53c2\u6570\u53ef\u7701\u7565\uff0c\u9ed8\u8ba4\u503c\u4e3a0\u3002"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"[vertices]"}),"\uff1a\u8868\u793a\u56fe\u7684\u70b9\u4e2a\u6570\uff0c\u4e3a0\u65f6\u8868\u793a\u7528\u6237\u5e0c\u671b\u7cfb\u7edf\u81ea\u52a8\u8bc6\u522b\u70b9\u6570\u91cf\uff1b\u4e3a\u975e\u96f6\u503c\u65f6\u8868\u793a\u7528\u6237\u5e0c\u671b\u81ea\u5b9a\u4e49\u70b9\u4e2a\u6570\uff0c\u8981\u6c42\u7528\u6237\u81ea\u5b9a\u4e49\u70b9\u4e2a\u6570\u9700\u5927\u4e8e\u6700\u5927\u7684\u70b9ID\u3002\u53c2\u6570\u53ef\u7701\u7565\uff0c\u9ed8\u8ba4\u503c\u4e3a0\u3002"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"[root]"}),"\uff1a\u8868\u793a\u8fdb\u884cbfs\u7684\u8d77\u59cb\u70b9id\u3002\u53c2\u6570\u4e0d\u53ef\u7701\u7565\u3002"]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"[output_dir]"}),"\uff1a\u8868\u793a\u8f93\u51fa\u6570\u636e\u4fdd\u5b58\u7684\u6587\u4ef6\u5939\u8def\u5f84\uff0c\u5c06\u8f93\u51fa\u5185\u5bb9\u4fdd\u5b58\u81f3\u8be5\u6587\u4ef6\u4e2d\uff0c\u53c2\u6570\u4e0d\u53ef\u7701\u7565\u3002"]}),"\n"]}),"\n",(0,s.jsx)(n.p,{children:"\u793a\u4f8b\uff1a"}),"\n",(0,s.jsx)(n.h3,{id:"c-2",children:"C++:"}),"\n",(0,s.jsx)(n.p,{children:"\u5728tugraph-db/build\u7f16\u8bd1standalone\u7b97\u6cd5\u7a0b\u5e8f"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-bash",children:"make bfs_standalone\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u5728tugraph-db/build/output\u76ee\u5f55\u4e0b\u8fd0\u884ctext\u6e90\u6587\u4ef6"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-bash",children:"./output/algo/bfs_standalone --type text --input_dir ../test/integration/data/algo/fb_unweighted --root 0\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u5f97\u5230\u8fd0\u884c\u7ed3\u679c\uff1a"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-text",children:"prepare_cost = 0.10(s)\ncore_cost = 0.02(s)\nfound_vertices = 3829\noutput_cost = 0.00(s)\ntotal_cost = 0.11(s)\nDONE.\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u7ed3\u679c\u53c2\u6570\u89e3\u91ca\u540c\u4e0a\u3002"}),"\n",(0,s.jsxs)(n.p,{children:["\u5bf9\u4e8e\u65b0\u7684\u7b97\u6cd5\uff0c\u8fd0\u884c\u65f6\u4e0d\u4e86\u89e3\u8be5\u7b97\u6cd5\u7684\u6240\u9700\u53c2\u6570\u65f6\uff0c\u53ef\u901a\u8fc7",(0,s.jsx)(n.code,{children:"./output/algo/bfs_standalone -h"}),"\u8fdb\u884c\u67e5\u9605\u5bf9\u5e94\u53c2\u6570\u3002"]}),"\n",(0,s.jsx)(n.h3,{id:"python-2",children:"Python:"}),"\n",(0,s.jsxs)(n.p,{children:["Python\u8bed\u8a00\u7684bfs\u62d3\u5c55\u7f16\u8bd1\u8fc7\u7a0b\u4e0eembed\u6a21\u5f0f\u65e0\u533a\u522b\uff0c\u5728\u8fd0\u884c\u65f6\u901a\u8fc7",(0,s.jsx)(n.code,{children:"Standalone"}),"\u63a5\u53e3\u8c03\u7528\uff0c\u793a\u4f8b\u5982\u4e0b\uff1a"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-python",children:'# tugraph-db/procedures/run_standalone.py\nimport bfs as python_plugin\n\nif __name__ == "__main__":\n    python_plugin.Standalone(input_dir=\n                             "../test/integration/data/algo/fb_unweighted",\n                             root=0)\n'})}),"\n",(0,s.jsx)(n.p,{children:"\u901a\u8fc7\u5982\u4e0b\u547d\u4ee4\u6267\u884c"}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-bash",children:"python3 run_standalone.py\n"})}),"\n",(0,s.jsx)(n.p,{children:"\u81f3\u6b64\uff0c\u901a\u8fc7TuGraph\u5bf9\u4e0a\u56fe\u8fdb\u884cbfs\u8fd0\u7b97\u7684\u8fc7\u7a0b\u5df2\u7ecf\u5b8c\u6210\u3002"})]})}function c(e={}){const{wrapper:n}={...(0,d.R)(),...e.components};return n?(0,s.jsx)(n,{...e,children:(0,s.jsx)(p,{...e})}):p(e)}},28453:(e,n,r)=>{r.d(n,{R:()=>t,x:()=>l});var s=r(96540);const d={},a=s.createContext(d);function t(e){const n=s.useContext(a);return s.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function l(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(d):e.components||d:t(e.components),s.createElement(a.Provider,{value:n},e.children)}}}]);