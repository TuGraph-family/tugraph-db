"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[79750],{28453:(e,t,r)=>{r.d(t,{R:()=>o,x:()=>i});var n=r(96540);const l={},s=n.createContext(l);function o(e){const t=n.useContext(s);return n.useMemo((function(){return"function"==typeof e?e(t):{...t,...e}}),[t,e])}function i(e){let t;return t=e.disableParentContext?"function"==typeof e.components?e.components(l):e.components||l:o(e.components),n.createElement(s.Provider,{value:t},e.children)}},85002:(e,t,r)=>{r.r(t),r.d(t,{assets:()=>c,contentTitle:()=>o,default:()=>h,frontMatter:()=>s,metadata:()=>i,toc:()=>d});var n=r(74848),l=r(28453);const s={},o="Python\u5ba2\u6237\u7aef",i={id:"client-tools/python-client",title:"Python\u5ba2\u6237\u7aef",description:"\u6b64\u6587\u6863\u4e3b\u8981\u662fTuGraph Python SDK\u7684\u4f7f\u7528\u8bf4\u660e, \u6ce8\u610f\u5c06\u6765\u4e0d\u518d\u66f4\u65b0\u7ef4\u62a4\uff0c\u5efa\u8bae\u4f7f\u7528 bolt\u5ba2\u6237\u7aef",source:"@site/versions/version-4.3.1/zh-CN/source/7.client-tools/1.python-client.md",sourceDirName:"7.client-tools",slug:"/client-tools/python-client",permalink:"/tugraph-db/zh/4.3.1/client-tools/python-client",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:1,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"TuGraph-Restful-Server",permalink:"/tugraph-db/zh/4.3.1/utility-tools/restful"},next:{title:"C++\u5ba2\u6237\u7aef",permalink:"/tugraph-db/zh/4.3.1/client-tools/cpp-client"}},c={},d=[{value:"1. \u6982\u8ff0",id:"1-\u6982\u8ff0",level:2},{value:"2. RESTful Client",id:"2-restful-client",level:2},{value:"2.1.\u5b89\u88c5Client",id:"21\u5b89\u88c5client",level:3},{value:"2.2.\u8c03\u7528Cypher",id:"22\u8c03\u7528cypher",level:3},{value:"2.3.\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",id:"23\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"3.RPC Client",id:"3rpc-client",level:2},{value:"3.1.\u5b9e\u4f8b\u5316client\u5bf9\u8c61",id:"31\u5b9e\u4f8b\u5316client\u5bf9\u8c61",level:3},{value:"3.1.1.\u5b9e\u4f8b\u5316\u5355\u8282\u70b9client\u5bf9\u8c61",id:"311\u5b9e\u4f8b\u5316\u5355\u8282\u70b9client\u5bf9\u8c61",level:4},{value:"3.1.2.\u5b9e\u4f8b\u5316HA\u96c6\u7fa4\u76f4\u8fde\u8fde\u63a5client\u5bf9\u8c61",id:"312\u5b9e\u4f8b\u5316ha\u96c6\u7fa4\u76f4\u8fde\u8fde\u63a5client\u5bf9\u8c61",level:4},{value:"3.1.3.\u5b9e\u4f8b\u5316HA\u96c6\u7fa4\u95f4\u63a5\u8fde\u63a5client\u5bf9\u8c61",id:"313\u5b9e\u4f8b\u5316ha\u96c6\u7fa4\u95f4\u63a5\u8fde\u63a5client\u5bf9\u8c61",level:4},{value:"3.2.\u8c03\u7528cypher",id:"32\u8c03\u7528cypher",level:3},{value:"3.3.\u5411leader\u53d1\u9001cypher\u8bf7\u6c42",id:"33\u5411leader\u53d1\u9001cypher\u8bf7\u6c42",level:3},{value:"3.4.\u8c03\u7528GQL",id:"34\u8c03\u7528gql",level:3},{value:"3.5.\u5411leader\u53d1\u9001GQL\u8bf7\u6c42",id:"35\u5411leader\u53d1\u9001gql\u8bf7\u6c42",level:3},{value:"3.6.\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",id:"36\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"3.7.\u5411leader\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",id:"37\u5411leader\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"3.8.\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b",id:"38\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"3.9.\u5217\u4e3e\u5b58\u50a8\u8fc7\u7a0b",id:"39\u5217\u4e3e\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"3.10.\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b",id:"310\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"3.11.\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165schema",id:"311\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165schema",level:3},{value:"3.12.\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e",id:"312\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e",level:3},{value:"3.13.\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165schema",id:"313\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165schema",level:3},{value:"3.14.\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e",id:"314\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e",level:3}];function a(e){const t={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",h4:"h4",header:"header",p:"p",pre:"pre",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",...(0,l.R)(),...e.components};return(0,n.jsxs)(n.Fragment,{children:[(0,n.jsx)(t.header,{children:(0,n.jsx)(t.h1,{id:"python\u5ba2\u6237\u7aef",children:"Python\u5ba2\u6237\u7aef"})}),"\n",(0,n.jsxs)(t.blockquote,{children:["\n",(0,n.jsxs)(t.p,{children:["\u6b64\u6587\u6863\u4e3b\u8981\u662fTuGraph Python SDK\u7684\u4f7f\u7528\u8bf4\u660e, \u6ce8\u610f\u5c06\u6765\u4e0d\u518d\u66f4\u65b0\u7ef4\u62a4\uff0c\u5efa\u8bae\u4f7f\u7528 ",(0,n.jsx)(t.a,{href:"/tugraph-db/zh/4.3.1/client-tools/bolt-client",children:"bolt\u5ba2\u6237\u7aef"})]}),"\n"]}),"\n",(0,n.jsx)(t.h2,{id:"1-\u6982\u8ff0",children:"1. \u6982\u8ff0"}),"\n",(0,n.jsx)(t.p,{children:"Python\u7684TuGraph Client\u6709\u4e24\u79cd\uff0c\u4e00\u79cd\u662fRESTful\u7684Client\uff0c\u4f7f\u7528HTTP\u65b9\u6cd5\u5411server\u53d1\u9001\u8bf7\u6c42\uff0c\u53e6\u4e00\u79cd\u662fRPC\u7684Client\uff0c\u4f7f\u7528RPC\u65b9\u6cd5\u8c03\u7528server\u8fdc\u7a0b\u670d\u52a1\uff0c\u4e24\u8005\u5404\u6709\u4f18\u52a3\u3002\nRESTful client\u7684\u4f7f\u7528\u65b9\u5f0f\u7b80\u5355\uff0c\u5728\u9879\u76ee\u7684src/client/python/TuGraphClient\u76ee\u5f55\u4e0b\u53ef\u4ee5\u627e\u5230Client\u7684\u6e90\u7801\u6587\u4ef6\uff0c\u76f4\u63a5\u5b89\u88c5\u5230\u7528\u6237\u73af\u5883\u4e2d\u5373\u53ef\u4f7f\u7528\uff0c\u4f46\u662f\u652f\u6301\u7684\u529f\u80fd\u8f83\u5c11\uff0c\n\u6027\u80fd\u4e5f\u4e0d\u9ad8\u3002RPC Client\u65e2\u652f\u6301\u5355\u673a\u7684server\uff0c\u4e5f\u652f\u6301\u9ad8\u53ef\u7528\u96c6\u7fa4\u548c\u8d1f\u8f7d\u5747\u8861\uff0c\u63a5\u53e3\u8f83\u591a\uff0c\u529f\u80fd\u5f3a\u5927\u3002\u4f46\u662f\u4f7f\u7528\u65b9\u5f0f\u8f83\u4e3a\u590d\u6742\uff0c\u9700\u8981\u7528\u6237\u81ea\u5df1\u7f16\u8bd1TuGraph\u9879\u76ee\u5f97\u5230liblgraph_client_python.so\uff0c\n\u6216\u8005\u4f7f\u7528runtime\u955c\u50cf\u65f6\u76f4\u63a5\u5728/usr/local/lib64\u76ee\u5f55\u4e0b\u627e\u5230\u8be5\u4f9d\u8d56\u5e93\uff0c\u5c06\u5176\u5f15\u5165python\u9879\u76ee\u5373\u53ef\u6b63\u5e38\u4f7f\u7528\u3002\u63a5\u4e0b\u6765\u5c06\u8be6\u7ec6\u4ecb\u7ecd\u8fd9\u4e24\u79cdClient\u7684\u4f7f\u7528\u65b9\u5f0f\u3002"}),"\n",(0,n.jsx)(t.h2,{id:"2-restful-client",children:"2. RESTful Client"}),"\n",(0,n.jsx)(t.h3,{id:"21\u5b89\u88c5client",children:"2.1.\u5b89\u88c5Client"}),"\n",(0,n.jsx)(t.p,{children:"TuGraph\u7684Python RESTful client\u4f7f\u7528setuptools\u5de5\u5177\u8fdb\u884c\u6253\u5305\u548c\u5206\u53d1\uff0c\u7528\u6237\u53ef\u4ee5\u5c06client\u76f4\u63a5\u5b89\u88c5\u5230\u672c\u5730\u73af\u5883\u4e2d\uff0c\u5728\u4f7f\u7528\u65f6\u5373\u53ef\u76f4\u63a5\u5f15\u5165\u3002"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-shell",children:"cd src/client/python/TuGraphClient\npython3 setup.py build\npython3 setup.py install\n"})}),"\n",(0,n.jsx)(t.p,{children:"\u6ce8\uff1a\u4f7f\u7528setuptools\u5de5\u5177\u5b89\u88c5python client\u65f6\u4f1a\u5b89\u88c5httpx\u7b49\u4f9d\u8d56\uff0c\u9700\u8981\u5728\u901a\u5916\u7f51\u7684\u73af\u5883\u4e0b\u6267\u884c\u3002"}),"\n",(0,n.jsx)(t.h3,{id:"22\u8c03\u7528cypher",children:"2.2.\u8c03\u7528Cypher"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'from TuGraphClient import TuGraphClient, AsyncTuGraphClient\n\nclient = TuGraphClient("127.0.0.1:7071" , "admin", "73@TuGraph")\ncypher = "match (n) return properties(n) limit 1"\nres = client.call_cypher(cypher)\nprint(res)\n\naclient = AsyncTuGraphClient("127.0.0.1:7071" , "admin", "73@TuGraph")\ncypher = "match (n) return properties(n) limit 1"\nres = await aclient.call_cypher(cypher)\nprint(res)\n'})}),"\n",(0,n.jsx)(t.h3,{id:"23\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",children:"2.3.\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'from TuGraphClient import TuGraphClient, AsyncTuGraphClient\n\nclient = TuGraphClient("127.0.0.1:7071" , "admin", "73@TuGraph")\nplugin_type = "cpp"\nplugin_name = "khop"\nplugin_input = "{\\"root\\": 10, \\"hop\\": 3}"\nres = client.call_plugin(plugin_type, plguin_name, plugin_input)\nprint(res)\n\naclient = AsyncTuGraphClient("127.0.0.1:7071" , "admin", "73@TuGraph")\nres = await aclient.call_plugin(plugin_type, plguin_name, plugin_input)\nprint(res)\n'})}),"\n",(0,n.jsx)(t.h2,{id:"3rpc-client",children:"3.RPC Client"}),"\n",(0,n.jsx)(t.p,{children:"Python\u7684TuGraph Rpc Client\u662f\u4f7f\u7528pybind11\u5305\u88c5\u7684CPP Client SDK\uff0c\u4e0b\u8868\u5217\u51fa\u4e86Python\u548cCPP Client SDK\u7684\u5bf9\u5e94\u5173\u7cfb\u3002"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,n.jsxs)(t.table,{children:[(0,n.jsx)(t.thead,{children:(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.th,{children:"Python Client SDK"}),(0,n.jsx)(t.th,{children:"CPP Client SDK"})]})}),(0,n.jsxs)(t.tbody,{children:[(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"client(self: liblgraph_client_python.client, url: str, user: str, password: str)"}),(0,n.jsx)(t.td,{children:"RpcClient(const std::string& url, const std::string& user, const std::string& password)"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"client(self: liblgraph_client_python.client, urls: list, user: str, password: str)"}),(0,n.jsxs)(t.td,{children:["RpcClient(std::vector",(0,n.jsx)(t.a,{href:"std::string",children:"std::string"}),"& urls, std::string user, std::string password)"]})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"callCypher(self: liblgraph_client_python.client, cypher: str, graph: str, json_format: bool, timeout: float, url: str) -> (bool, str)"}),(0,n.jsx)(t.td,{children:"bool CallCypher(std::string& result, const std::string& cypher, const std::string& graph, bool json_format, double timeout, const std::string& url)"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"callCypherToLeader(self: liblgraph_client_python.client, cypher: str, graph: str, json_format: bool, timeout: float) -> (bool, str)"}),(0,n.jsx)(t.td,{children:"bool CallCypherToLeader(std::string& result, const std::string& cypher, const std::string& graph, bool json_format, double timeout)"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"callGql(self: liblgraph_client_python.client, gql: str, graph: str, json_format: bool, timeout: float, url: str) -> (bool, str)"}),(0,n.jsx)(t.td,{children:"bool CallGql(std::string& result, const std::string& gql, const std::string& graph, bool json_format, double timeout, const std::string& url)"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"callGqlToLeader(self: liblgraph_client_python.client, gql: str, graph: str, json_format: bool, timeout: float) -> (bool, str)"}),(0,n.jsx)(t.td,{children:'bool CallGqlToLeader(std::string& result, const std::string& gql, const std::string& graph = "default", bool json_format = true, double timeout = 0)'})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"callProcedure(self: liblgraph_client_python.client, procedure_type: str, procedure_name: str, param: str, procedure_time_out: float, in_process: bool, graph: str, json_format: bool, url: str) -> (bool, str)"}),(0,n.jsx)(t.td,{children:"bool CallProcedure(std::string& result, const std::string& procedure_type, const std::string& procedure_name, const std::string& param, double procedure_time_out, bool in_process, const std::string& graph, bool json_format, const std::string& url)"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"callProcedureToLeader(self: liblgraph_client_python.client, procedure_type: str, procedure_name: str, param: str, procedure_time_out: float, in_process: bool, graph: str, json_format: bool) -> (bool, str)"}),(0,n.jsx)(t.td,{children:"CallProcedureToLeader(std::string& result, const std::string& procedure_type, const std::string& procedure_name, const std::string& param, double procedure_time_out, bool in_process, const std::string& graph, bool json_format)"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"loadProcedure(self: liblgraph_client_python.client, source_file: str, procedure_type: str, procedure_name: str, code_type: str, procedure_description: str, read_only: bool, version: str, graph: str) -> (bool, str)"}),(0,n.jsx)(t.td,{children:"bool LoadProcedure(std::string& result, const std::string& source_file, const std::string& procedure_type, const std::string& procedure_name, const std::string& code_type, const std::string& procedure_description, bool read_only, const std::string& version, const std::string& graph)"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"listProcedures(self: liblgraph_client_python.client, procedure_type: str, version: str, graph: str, url: str) -> (bool, str)"}),(0,n.jsx)(t.td,{children:"bool ListProcedures(std::string& result, const std::string& procedure_type, const std::string& version, const std::string& graph, const std::string& url)"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"deleteProcedure(self: liblgraph_client_python.client, procedure_type: str, procedure_name: str, graph: str) -> (bool, str)"}),(0,n.jsx)(t.td,{children:"bool DeleteProcedure(std::string& result, const std::string& procedure_type, const std::string& procedure_name, const std::string& graph)"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"importSchemaFromContent(self: liblgraph_client_python.client, schema: str, graph: str, json_format: bool, timeout: float) -> (bool, str)"}),(0,n.jsx)(t.td,{children:"bool ImportSchemaFromContent(std::string& result, const std::string& schema, const std::string& graph, bool json_format, double timeout)"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"importDataFromContent(self: liblgraph_client_python.client, desc: str, data: str, delimiter: str, continue_on_error: bool, thread_nums: int, graph: str, json_format: bool, timeout: float) -> (bool, str)"}),(0,n.jsx)(t.td,{children:"ImportDataFromContent(std::string& result, const std::string& desc, const std::string& data, const std::string& delimiter, bool continue_on_error, int thread_nums, const std::string& graph, bool json_format, double timeout)"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"importSchemaFromFile(self: liblgraph_client_python.client, schema_file: str, graph: str, json_format: bool, timeout: float) -> (bool, str)"}),(0,n.jsx)(t.td,{children:"ImportSchemaFromFile(std::string& result, const std::string& schema_file, const std::string& graph, bool json_format, double timeout)"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"importDataFromFile(self: liblgraph_client_python.client, conf_file: str, delimiter: str, continue_on_error: bool, thread_nums: int, skip_packages: int, graph: str, json_format: bool, timeout: float) -> (bool, str)"}),(0,n.jsx)(t.td,{children:"ImportDataFromFile(std::string& result, const std::string& conf_file, const std::string& delimiter, bool continue_on_error, int thread_nums, int skip_packages, const std::string& graph, bool json_format, double timeout)"})]})]})]}),"\n",(0,n.jsx)(t.p,{children:"Python RPC Client\u7684\u4f7f\u7528\u65b9\u5f0f\u8f83\u4e3a\u590d\u6742\uff0c\u7528\u6237\u53ef\u4ee5\u5728\u672c\u5730\u73af\u5883\u4e2d\u7f16\u8bd1TuGraph\u5f97\u5230liblgraph_client_python.so\uff0c\u4e5f\u53ef\u4ee5\u4f7f\u7528\u5b98\u65b9\u63d0\u4f9b\u7684runtime\u955c\u50cf\uff0c\n\u5728\u955c\u50cf\u4e2d\u7684/usr/local/lib64\u76ee\u5f55\u4e0b\u53ef\u4ee5\u76f4\u63a5\u627e\u5230\u8be5\u4f9d\u8d56\u5e93\uff0c\u5f15\u5165\u7528\u6237\u9879\u76ee\u5373\u53ef\u4f7f\u7528\u3002"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:"import liblgraph_client_python\n"})}),"\n",(0,n.jsx)(t.h3,{id:"31\u5b9e\u4f8b\u5316client\u5bf9\u8c61",children:"3.1.\u5b9e\u4f8b\u5316client\u5bf9\u8c61"}),"\n",(0,n.jsx)(t.h4,{id:"311\u5b9e\u4f8b\u5316\u5355\u8282\u70b9client\u5bf9\u8c61",children:"3.1.1.\u5b9e\u4f8b\u5316\u5355\u8282\u70b9client\u5bf9\u8c61"}),"\n",(0,n.jsx)(t.p,{children:"\u5f53\u4ee5\u5355\u8282\u70b9\u6a21\u5f0f\u542f\u52a8server\u65f6\uff0cclient\u6309\u7167\u5982\u4e0b\u683c\u5f0f\u8fdb\u884c\u5b9e\u4f8b\u5316"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'client = liblgraph_client_python.client("127.0.0.1:19099", "admin", "73@TuGraph")\n'})}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{children:"client(self: liblgraph_client_python.client, url: str, user: str, password: str)\n"})}),"\n",(0,n.jsx)(t.h4,{id:"312\u5b9e\u4f8b\u5316ha\u96c6\u7fa4\u76f4\u8fde\u8fde\u63a5client\u5bf9\u8c61",children:"3.1.2.\u5b9e\u4f8b\u5316HA\u96c6\u7fa4\u76f4\u8fde\u8fde\u63a5client\u5bf9\u8c61"}),"\n",(0,n.jsx)(t.p,{children:"\u5f53\u670d\u52a1\u5668\u4e0a\u90e8\u7f72\u7684HA\u96c6\u7fa4\u53ef\u4ee5\u4f7f\u7528ha_conf\u4e2d\u914d\u7f6e\u7684\u7f51\u5740\u76f4\u63a5\u8fde\u63a5\u65f6\uff0cclient\u6309\u7167\u5982\u4e0b\u683c\u5f0f\u8fdb\u884c\u5b9e\u4f8b\u5316\u3002"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'client = liblgraph_client_python.client("127.0.0.1:19099", "admin", "73@TuGraph")\n'})}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{children:"client(self: liblgraph_client_python.client, url: str, user: str, password: str)\n"})}),"\n",(0,n.jsx)(t.p,{children:"\u7528\u6237\u53ea\u9700\u8981\u4f20\u5165HA\u96c6\u7fa4\u4e2d\u7684\u4efb\u610f\u4e00\u4e2a\u8282\u70b9\u7684url\u5373\u53ef\uff0cclient\u4f1a\u6839\u636eserver\u7aef\u8fd4\u56de\u7684\u67e5\u8be2\u4fe1\u606f\u81ea\u52a8\u7ef4\u62a4\u8fde\u63a5\u6c60\uff0c\u5728HA\u96c6\u7fa4\u6a2a\u5411\u6269\u5bb9\u65f6\n\u4e5f\u4e0d\u9700\u8981\u624b\u52a8\u91cd\u542fclient\u3002"}),"\n",(0,n.jsx)(t.h4,{id:"313\u5b9e\u4f8b\u5316ha\u96c6\u7fa4\u95f4\u63a5\u8fde\u63a5client\u5bf9\u8c61",children:"3.1.3.\u5b9e\u4f8b\u5316HA\u96c6\u7fa4\u95f4\u63a5\u8fde\u63a5client\u5bf9\u8c61"}),"\n",(0,n.jsx)(t.p,{children:"\u5f53\u670d\u52a1\u5668\u4e0a\u90e8\u7f72\u7684HA\u96c6\u7fa4\u4e0d\u80fd\u4f7f\u7528ha_conf\u4e2d\u914d\u7f6e\u7684\u7f51\u5740\u76f4\u63a5\u8fde\u63a5\u800c\u5fc5\u987b\u4f7f\u7528\u95f4\u63a5\u7f51\u5740\uff08\u5982\u963f\u91cc\u4e91\u516c\u7f51\u7f51\u5740\uff09\u8fde\u63a5\u65f6\uff0c\nclient\u6309\u7167\u5982\u4e0b\u683c\u5f0f\u8fdb\u884c\u5b9e\u4f8b\u5316"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'client = liblgraph_client_python.client(["189.33.97.23:9091","189.33.97.24:9091", "189.33.97.25:9091"], "admin", "73@TuGraph")\n'})}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{children:"client(self: liblgraph_client_python.client, urls: list, user: str, password: str)\n"})}),"\n",(0,n.jsx)(t.p,{children:"\u56e0\u4e3a\u7528\u6237\u8fde\u63a5\u7684\u7f51\u5740\u548cserver\u542f\u52a8\u65f6\u914d\u7f6e\u7684\u4fe1\u606f\u4e0d\u540c\uff0c\u4e0d\u80fd\u901a\u8fc7\u5411\u96c6\u7fa4\u53d1\u8bf7\u6c42\u7684\u65b9\u5f0f\u81ea\u52a8\u66f4\u65b0client\u8fde\u63a5\u6c60\uff0c\u6240\u4ee5\u9700\u8981\u5728\u542f\u52a8\nclient\u65f6\u624b\u52a8\u4f20\u5165\u6240\u6709\u96c6\u7fa4\u4e2d\u8282\u70b9\u7684\u7f51\u5740\uff0c\u5e76\u5728\u96c6\u7fa4\u8282\u70b9\u53d8\u66f4\u65f6\u624b\u52a8\u91cd\u542fclient\u3002"}),"\n",(0,n.jsx)(t.h3,{id:"32\u8c03\u7528cypher",children:"3.2.\u8c03\u7528cypher"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'ret, res = client.callCypher("CALL db.edgeLabels()", "default", 10)\n'})}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{children:"callCypher(self: liblgraph_client_python.client, cypher: str, graph: str, json_format: bool, timeout: float, url: str) -> (bool, str)\n"})}),"\n",(0,n.jsx)(t.p,{children:"\u672c\u63a5\u53e3\u652f\u6301\u5728\u5355\u673a\u6a21\u5f0f\u548cHA\u6a21\u5f0f\u4e0b\u4f7f\u7528\u3002\u5176\u4e2d\uff0c\u5728HA\u6a21\u5f0f\u4e0b\u7684client\u4e2d\uff0c\u901a\u8fc7\u6307\u5b9aurl\u53c2\u6570\u53ef\u4ee5\u5b9a\u5411\u5411\u67d0\u4e2aserver\u53d1\u9001\u8bfb\u8bf7\u6c42\u3002"}),"\n",(0,n.jsx)(t.h3,{id:"33\u5411leader\u53d1\u9001cypher\u8bf7\u6c42",children:"3.3.\u5411leader\u53d1\u9001cypher\u8bf7\u6c42"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'ret, res = client.callCypherToLeader("CALL db.edgeLabels()", "default", 10)\n'})}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{children:"callCypherToLeader(self: liblgraph_client_python.client, cypher: str, graph: str, json_format: bool, timeout: float) -> (bool, str)\n"})}),"\n",(0,n.jsx)(t.p,{children:"\u672c\u63a5\u53e3\u53ea\u652f\u6301\u5728HA\u6a21\u5f0f\u4e0b\u4f7f\u7528\uff0c\u5728HA\u6a21\u5f0f\u4e0b\u7684client\u4e2d\uff0c\u4e3a\u9632\u6b62\u5411\u672a\u540c\u6b65\u6570\u636e\u7684follower\u53d1\u9001\u8bf7\u6c42\uff0c\n\u7528\u6237\u53ef\u4ee5\u76f4\u63a5\u5411leader\u53d1\u9001\u8bf7\u6c42\uff0cleader\u7531\u96c6\u7fa4\u9009\u51fa\u3002"}),"\n",(0,n.jsx)(t.h3,{id:"34\u8c03\u7528gql",children:"3.4.\u8c03\u7528GQL"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'ret, res = client.callGql("CALL db.edgeLabels()", "default", 10)\n'})}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{children:"callGql(self: liblgraph_client_python.client, gql: str, graph: str, json_format: bool, timeout: float, url: str) -> (bool, str)\n"})}),"\n",(0,n.jsx)(t.p,{children:"\u672c\u63a5\u53e3\u652f\u6301\u5728\u5355\u673a\u6a21\u5f0f\u548cHA\u6a21\u5f0f\u4e0b\u4f7f\u7528\u3002\u5176\u4e2d\uff0c\u5728HA\u6a21\u5f0f\u4e0b\u7684client\u4e2d\uff0c\u901a\u8fc7\u6307\u5b9aurl\u53c2\u6570\u53ef\u4ee5\u5b9a\u5411\u5411\u67d0\u4e2aserver\u53d1\u9001\u8bfb\u8bf7\u6c42\u3002"}),"\n",(0,n.jsx)(t.h3,{id:"35\u5411leader\u53d1\u9001gql\u8bf7\u6c42",children:"3.5.\u5411leader\u53d1\u9001GQL\u8bf7\u6c42"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'ret, res = client.callGqlToLeader("CALL db.edgeLabels()", "default", 10)\n'})}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{children:"callGqlToLeader(self: liblgraph_client_python.client, gql: str, graph: str, json_format: bool, timeout: float) -> (bool, str)\n"})}),"\n",(0,n.jsx)(t.p,{children:"\u672c\u63a5\u53e3\u53ea\u652f\u6301\u5728HA\u6a21\u5f0f\u4e0b\u4f7f\u7528\uff0c\u5728HA\u6a21\u5f0f\u4e0b\u7684client\u4e2d\uff0c\u4e3a\u9632\u6b62\u5411\u672a\u540c\u6b65\u6570\u636e\u7684follower\u53d1\u9001\u8bf7\u6c42\uff0c\n\u7528\u6237\u53ef\u4ee5\u76f4\u63a5\u5411leader\u53d1\u9001\u8bf7\u6c42\uff0cleader\u7531\u96c6\u7fa4\u9009\u51fa\u3002"}),"\n",(0,n.jsx)(t.h3,{id:"36\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",children:"3.6.\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'ret, res = client.callProcedure("CPP", "test_plugin1", "bcefg", 1000, False, "default")\n'})}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{children:"callProcedure(self: liblgraph_client_python.client, procedure_type: str, procedure_name: str, param: str, procedure_time_out: float, in_process: bool, graph: str, json_format: bool, url: str) -> (bool, str)\n"})}),"\n",(0,n.jsx)(t.p,{children:"\u672c\u63a5\u53e3\u652f\u6301\u5728\u5355\u673a\u6a21\u5f0f\u548cHA\u6a21\u5f0f\u4e0b\u4f7f\u7528\uff0c\u9ed8\u8ba4\u4ee5\u5b57\u7b26\u4e32\u683c\u5f0f\u76f4\u63a5\u8fd4\u56de\u5b58\u50a8\u8fc7\u7a0b\u7684\u6267\u884c\u7ed3\u679c\uff0c\u6307\u5b9ajsonFormat\u4e3atrue\u53ef\u4ee5\u8fd4\u56dejson\u683c\u5f0f\u7684\u6267\u884c\u7ed3\u679c\u3002\n\u5176\u4e2d\uff0c\u5728HA\u6a21\u5f0f\u4e0b\u7684client\u4e2d\uff0c\u901a\u8fc7\u6307\u5b9aurl\u53c2\u6570\u53ef\u4ee5\u5b9a\u5411\u5411\u67d0\u4e2aserver\u53d1\u9001\u8bfb\u8bf7\u6c42\u3002"}),"\n",(0,n.jsx)(t.h3,{id:"37\u5411leader\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",children:"3.7.\u5411leader\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'ret, res = client.callProcedureToLeader("CPP", "khop", kHopParamGen(), 1000, false, "default")\n'})}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{children:"callProcedureToLeader(self: liblgraph_client_python.client, procedure_type: str, procedure_name: str, param: str, procedure_time_out: float, in_process: bool, graph: str, json_format: bool) -> (bool, str)\n"})}),"\n",(0,n.jsx)(t.p,{children:"\u672c\u63a5\u53e3\u652f\u6301\u5728HA\u6a21\u5f0f\u4e0b\u4f7f\u7528\uff0c\u9ed8\u8ba4\u4ee5\u5b57\u7b26\u4e32\u683c\u5f0f\u76f4\u63a5\u8fd4\u56de\u5b58\u50a8\u8fc7\u7a0b\u7684\u6267\u884c\u7ed3\u679c\uff0c\u6307\u5b9ajsonFormat\u4e3atrue\u53ef\u4ee5\u8fd4\u56dejson\u683c\u5f0f\u7684\u6267\u884c\u7ed3\u679c\u3002"}),"\n",(0,n.jsx)(t.h3,{id:"38\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b",children:"3.8.\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'ret, res = client.loadProcedure("./test/procedure/khop.so", "CPP", "khop", "SO", "test loadprocedure", true, "v1", "default");\n'})}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{children:"loadProcedure(self: liblgraph_client_python.client, source_file: str, procedure_type: str, procedure_name: str, code_type: str, procedure_description: str, read_only: bool, version: str, graph: str) -> (bool, str)\n"})}),"\n",(0,n.jsx)(t.p,{children:"\u672c\u63a5\u53e3\u652f\u6301\u5728\u5355\u673a\u6a21\u5f0f\u548cHA\u6a21\u5f0f\u4e0b\u4f7f\u7528\u3002\u5176\u4e2d\uff0c\u7531\u4e8e\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b\u662f\u5199\u8bf7\u6c42\uff0cHA\u6a21\u5f0f\u4e0b\u7684client\u53ea\u80fd\u5411leader\u53d1\u9001\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b\u8bf7\u6c42\u3002"}),"\n",(0,n.jsx)(t.h3,{id:"39\u5217\u4e3e\u5b58\u50a8\u8fc7\u7a0b",children:"3.9.\u5217\u4e3e\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'ret, res = client.listProcedures("CPP", "any", "default")\n'})}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{children:"listProcedures(self: liblgraph_client_python.client, procedure_type: str, version: str, graph: str, url: str) -> (bool, str)\n"})}),"\n",(0,n.jsx)(t.p,{children:"\u672c\u63a5\u53e3\u652f\u6301\u5728\u5355\u673a\u6a21\u5f0f\u548cHA\u6a21\u5f0f\u4e0b\u4f7f\u7528\u3002\u5176\u4e2d\uff0c\u5728HA\u6a21\u5f0f\u4e0b\u7684client\u4e2d\uff0c\u901a\u8fc7\u6307\u5b9aurl\u53c2\u6570\u53ef\u4ee5\u5b9a\u5411\u5411\u67d0\u4e2aserver\u53d1\u9001\u8bfb\u8bf7\u6c42\u3002"}),"\n",(0,n.jsx)(t.h3,{id:"310\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b",children:"3.10.\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'ret, res = client.deleteProcedure("CPP", "sortstr", "default")\n'})}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{children:"deleteProcedure(self: liblgraph_client_python.client, procedure_type: str, procedure_name: str, graph: str) -> (bool, str)\n"})}),"\n",(0,n.jsx)(t.p,{children:"\u672c\u63a5\u53e3\u652f\u6301\u5728\u5355\u673a\u6a21\u5f0f\u548cHA\u6a21\u5f0f\u4e0b\u4f7f\u7528\u3002\u5176\u4e2d\uff0c\u7531\u4e8e\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b\u662f\u5199\u8bf7\u6c42\uff0cHA\u6a21\u5f0f\u4e0b\u7684client\u53ea\u80fd\u5411leader\u53d1\u9001\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b\u8bf7\u6c42\u3002"}),"\n",(0,n.jsx)(t.h3,{id:"311\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165schema",children:"3.11.\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165schema"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'ret, res = client.importSchemaFromContent(schema, "default", 1000)\n'})}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{children:"importSchemaFromContent(self: liblgraph_client_python.client, schema: str, graph: str, json_format: bool, timeout: float) -> (bool, str)\n"})}),"\n",(0,n.jsx)(t.p,{children:"\u672c\u63a5\u53e3\u652f\u6301\u5728\u5355\u673a\u6a21\u5f0f\u548cHA\u6a21\u5f0f\u4e0b\u4f7f\u7528\u3002\u5176\u4e2d\uff0c\u7531\u4e8e\u5bfc\u5165schema\u662f\u5199\u8bf7\u6c42\uff0cHA\u6a21\u5f0f\u4e0b\u7684client\u53ea\u80fd\u5411leader\u53d1\u9001\u5bfc\u5165schema\u8bf7\u6c42\u3002"}),"\n",(0,n.jsx)(t.h3,{id:"312\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e",children:"3.12.\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'ret, res = client.importDataFromContent(personDesc, person, ",", true, 16, "default", 1000)\n'})}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{children:"importDataFromContent(self: liblgraph_client_python.client, desc: str, data: str, delimiter: str, continue_on_error: bool, thread_nums: int, graph: str, json_format: bool, timeout: float) -> (bool, str)\n"})}),"\n",(0,n.jsx)(t.p,{children:"\u672c\u63a5\u53e3\u652f\u6301\u5728\u5355\u673a\u6a21\u5f0f\u548cHA\u6a21\u5f0f\u4e0b\u4f7f\u7528\u3002\u5176\u4e2d\uff0c\u7531\u4e8e\u5bfc\u5165\u70b9\u8fb9\u6570\u636e\u662f\u5199\u8bf7\u6c42\uff0cHA\u6a21\u5f0f\u4e0b\u7684client\u53ea\u80fd\u5411leader\u53d1\u9001\u5bfc\u5165\u70b9\u8fb9\u6570\u636e\u8bf7\u6c42\u3002"}),"\n",(0,n.jsx)(t.h3,{id:"313\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165schema",children:"3.13.\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165schema"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'ret, res = client.importSchemaFromFile("./test/data/yago.conf", "default", 1000)\n'})}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{children:"importSchemaFromFile(self: liblgraph_client_python.client, schema_file: str, graph: str, json_format: bool, timeout: float) -> (bool, str)\n"})}),"\n",(0,n.jsx)(t.p,{children:"\u672c\u63a5\u53e3\u652f\u6301\u5728\u5355\u673a\u6a21\u5f0f\u548cHA\u6a21\u5f0f\u4e0b\u4f7f\u7528\u3002\u5176\u4e2d\uff0c\u7531\u4e8e\u5bfc\u5165schema\u662f\u5199\u8bf7\u6c42\uff0cHA\u6a21\u5f0f\u4e0b\u7684client\u53ea\u80fd\u5411leader\u53d1\u9001\u5bfc\u5165schema\u8bf7\u6c42\u3002"}),"\n",(0,n.jsx)(t.h3,{id:"314\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e",children:"3.14.\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'ret, res = client.importDataFromFile("./test/data/yago.conf", ",", true, 16, 0, "default", 1000000000)\n'})}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{children:"importDataFromFile(self: liblgraph_client_python.client, conf_file: str, delimiter: str, continue_on_error: bool, thread_nums: int, skip_packages: int, graph: str, json_format: bool, timeout: float) -> (bool, str)\n"})}),"\n",(0,n.jsx)(t.p,{children:"\u672c\u63a5\u53e3\u652f\u6301\u5728\u5355\u673a\u6a21\u5f0f\u548cHA\u6a21\u5f0f\u4e0b\u4f7f\u7528\u3002\u5176\u4e2d\uff0c\u7531\u4e8e\u5bfc\u5165\u70b9\u8fb9\u6570\u636e\u662f\u5199\u8bf7\u6c42\uff0cHA\u6a21\u5f0f\u4e0b\u7684client\u53ea\u80fd\u5411leader\u53d1\u9001\u5bfc\u5165\u70b9\u8fb9\u6570\u636e\u8bf7\u6c42\u3002"})]})}function h(e={}){const{wrapper:t}={...(0,l.R)(),...e.components};return t?(0,n.jsx)(t,{...e,children:(0,n.jsx)(a,{...e})}):a(e)}}}]);