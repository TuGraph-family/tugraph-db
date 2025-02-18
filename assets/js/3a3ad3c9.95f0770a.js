"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[45419],{28453:(e,t,n)=>{n.d(t,{R:()=>i,x:()=>o});var r=n(96540);const s={},a=r.createContext(s);function i(e){const t=r.useContext(a);return r.useMemo((function(){return"function"==typeof e?e(t):{...t,...e}}),[t,e])}function o(e){let t;return t=e.disableParentContext?"function"==typeof e.components?e.components(s):e.components||s:i(e.components),r.createElement(a.Provider,{value:t},e.children)}},40268:(e,t,n)=>{n.r(t),n.d(t,{assets:()=>l,contentTitle:()=>i,default:()=>d,frontMatter:()=>a,metadata:()=>o,toc:()=>c});var r=n(74848),s=n(28453);const a={},i="C++\u5ba2\u6237\u7aef",o={id:"developer-manual/client-tools/cpp-client",title:"C++\u5ba2\u6237\u7aef",description:"\u6b64\u6587\u6863\u4e3b\u8981\u662fTuGraph C++ SDK\u7684\u4f7f\u7528\u8bf4\u660e\u3002",source:"@site/versions/version-3.5.1/zh-CN/source/5.developer-manual/4.client-tools/2.cpp-client.md",sourceDirName:"5.developer-manual/4.client-tools",slug:"/developer-manual/client-tools/cpp-client",permalink:"/tugraph-db/zh/3.5.1/developer-manual/client-tools/cpp-client",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:2,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Python\u5ba2\u6237\u7aef",permalink:"/tugraph-db/zh/3.5.1/developer-manual/client-tools/python-client"},next:{title:"Java\u5ba2\u6237\u7aef",permalink:"/tugraph-db/zh/3.5.1/developer-manual/client-tools/java-client"}},l={},c=[{value:"1.\u6982\u8ff0",id:"1\u6982\u8ff0",level:2},{value:"2.\u4f7f\u7528\u793a\u4f8b",id:"2\u4f7f\u7528\u793a\u4f8b",level:2},{value:"2.1.\u5b9e\u4f8b\u5316client\u5bf9\u8c61",id:"21\u5b9e\u4f8b\u5316client\u5bf9\u8c61",level:3},{value:"2.2.\u8c03\u7528cypher",id:"22\u8c03\u7528cypher",level:3},{value:"2.3.\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",id:"23\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"2.4.\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b",id:"24\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"2.5.\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165schema",id:"25\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165schema",level:3},{value:"2.6.\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e",id:"26\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e",level:3},{value:"2.7.\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165schema",id:"27\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165schema",level:3},{value:"2.8.\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e",id:"28\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e",level:3}];function u(e){const t={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",p:"p",pre:"pre",...(0,s.R)(),...e.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(t.header,{children:(0,r.jsx)(t.h1,{id:"c\u5ba2\u6237\u7aef",children:"C++\u5ba2\u6237\u7aef"})}),"\n",(0,r.jsxs)(t.blockquote,{children:["\n",(0,r.jsx)(t.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u662fTuGraph C++ SDK\u7684\u4f7f\u7528\u8bf4\u660e\u3002"}),"\n"]}),"\n",(0,r.jsx)(t.h2,{id:"1\u6982\u8ff0",children:"1.\u6982\u8ff0"}),"\n",(0,r.jsx)(t.p,{children:"C++ Client \u80fd\u591f\u4f7f\u7528 RPC \u8fde\u63a5lgraph_server\uff0c\u8fdb\u884c\u6570\u636e\u5bfc\u5165\u3001\u6267\u884c\u5b58\u50a8\u8fc7\u7a0b\u3001\u8c03\u7528Cypher\u7b49\u64cd\u4f5c\u3002"}),"\n",(0,r.jsx)(t.h2,{id:"2\u4f7f\u7528\u793a\u4f8b",children:"2.\u4f7f\u7528\u793a\u4f8b"}),"\n",(0,r.jsx)(t.h3,{id:"21\u5b9e\u4f8b\u5316client\u5bf9\u8c61",children:"2.1.\u5b9e\u4f8b\u5316client\u5bf9\u8c61"}),"\n",(0,r.jsx)(t.p,{children:"\u5f15\u5165\u4f9d\u8d56\u5e76\u5b9e\u4f8b\u5316"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-C++",children:'RpcClient client3("0.0.0.0:19099", "admin", "73@TuGraph");\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:"RpcClient(const std::string& url, const std::string& user, const std::string& password);\n@param url: tugraph host looks like ip:port\n@param user: login user name \n@param password: login password\n"})}),"\n",(0,r.jsx)(t.h3,{id:"22\u8c03\u7528cypher",children:"2.2.\u8c03\u7528cypher"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-C++",children:"    std::string str;\n    bool ret = client.CallCypher(str,\n        \"CALL db.createVertexLabel('actor', 'name', 'name', string, false, 'age', int8, true)\");\n\n"})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:'    bool CallCypher(std::string& result, const std::string& cypher,\n        const std::string& graph = "default",\n        bool json_format = true,\n        double timeout = 0);\n    @param result: the result returned by the service.\n    @param cypher: inquire statement.\n    @param graph: the graph to query.\n    @param json_format: The result is returned in JSON format\n    @param timeout: Maximum execution time, overruns will be interrupted\n    @return: whether the command is executed successfully\n'})}),"\n",(0,r.jsx)(t.h3,{id:"23\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",children:"2.3.\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-C++",children:'    std::string str;\n    bool ret = client.CallPlugin(str, "CPP", "test_plugin1", "bcefg");\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:'    bool CallPlugin(std::string& result, const std::string& plugin_type,\n        const std::string& plugin_name, const std::string& param,\n        double plugin_time_out = 0.0, bool in_process = false,\n        const std::string& graph = "default", bool json_format = true,\n        double timeout = 0);\n    @param result: the result returned by the service.\n    @param plugin_type: the plugin type, currently supported CPP and PY\n    @param plugin_name: plugin name\n    @param param: the execution parameters\n    @param plugin_timeout: Maximum execution time, overruns will be interrupted\n    @param graph: the graph to query.\n    @param json_format: The result is returned in JSON format\n    @param timeout: Maximum execution time, overruns will be interrupted\n    @return: whether the command is executed successfully\n'})}),"\n",(0,r.jsx)(t.h3,{id:"24\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b",children:"2.4.\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-C++",children:'    std::string str;\n    bool ret = client.LoadPlugin(str, code_sleep, "PY", "python_plugin1", "PY", "this is a test plugin",\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:'    bool LoadPlugin(std::string& result, const std::string& source_file,\n        const std::string& plugin_type, const std::string& plugin_name,\n        const std::string& code_type, const std::string& plugin_description,\n        bool read_only, const std::string& graph = "default", bool json_format = true,\n        double timeout = 0);\n    @param result: the result returned by the service.\n    @param source_file: the source_file contain plugin code\n    @param plugin_type: the plugin type, currently supported CPP and PY\n    @param plugin_name: plugin name\n    @param code_type: code type, currently supported PY, SO, CPP, ZIP\n    @param plugin_description: plugin description\n    @param read_only: plugin is read only or not\n    @param graph: the graph to query.\n    @param json_format: The result is returned in JSON format\n    @param timeout: Maximum execution time, overruns will be interrupted\n    @return: whether the command is executed successfully\n'})}),"\n",(0,r.jsx)(t.h3,{id:"25\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165schema",children:"2.5.\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165schema"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-C++",children:'    std::string str;\n    bool ret = client.ImportSchemaFromContent(str, sImportContent["schema"]);\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:'    bool ImportSchemaFromContent(std::string& result, const std::string& schema,\n        const std::string& graph = "default", bool json_format = true,\n        double timeout = 0);\n    @param result: the result returned by the service.\n    @param schema: the schema to be imported\n    @param graph: the graph to query.\n    @param json_format: The result is returned in JSON format\n    @param timeout: Maximum execution time, overruns will be interrupted\n    @return: whether the command is executed successfully\n'})}),"\n",(0,r.jsx)(t.h3,{id:"26\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e",children:"2.6.\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-C++",children:'    std::string str;\n    ret = client.ImportDataFromContent(str, sImportContent["person_desc"], sImportContent["person"],",");\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:'    bool ImportDataFromContent(std::string& result, const std::string& desc,\n        const std::string& data, const std::string& delimiter,\n        bool continue_on_error = false, int thread_nums = 8,\n        const std::string& graph = "default", bool json_format = true,\n        double timeout = 0);\n    @param result: the result returned by the service.\n    @param desc: data format description\n    @param data: the data to be imported\n    @param delimiter: data separator\n    @param continueOnError: whether to continue when importing data fails\n    @param threadNums: maximum number of threads\n    @param graph: the graph to query.\n    @param json_format: The result is returned in JSON format\n    @param timeout: Maximum execution time, overruns will be interrupted\n    @return: whether the command is executed successfully\n'})}),"\n",(0,r.jsx)(t.h3,{id:"27\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165schema",children:"2.7.\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165schema"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-C++",children:'    std::string conf_file("./yago.conf");\n    std::string str;\n    ret = client.ImportSchemaFromFile(str, conf_file);\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:'    bool ImportSchemaFromFile(std::string& result, const std::string& schema_file,\n        const std::string& graph = "default", bool json_format = true,\n        double timeout = 0);\n    @param result: the result returned by the service.\n    @param schemaFile: the schema_file contain schema\n    @param graph: the graph to query.\n    @param json_format: The result is returned in JSON format\n    @param timeout: Maximum execution time, overruns will be interrupted\n    @return: whether the command is executed successfully\n'})}),"\n",(0,r.jsx)(t.h3,{id:"28\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e",children:"2.8.\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-C++",children:'    std::string conf_file("./yago.conf");\n    std::string str;\n    ret = client.ImportDataFromFile(str, conf_file, ",");\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:'    bool ImportDataFromFile(std::string& result, const std::string& conf_file,\n        const std::string& delimiter, bool continue_on_error = false,\n        int thread_nums = 8, int skip_packages = 0,\n        const std::string& graph = "default", bool json_format = true,\n        double timeout = 0);\n    @param result: the result returned by the service.\n    @param conf_file: data file contain format description and data\n    @param delimiter: data separator\n    @param continue_on_error: whether to continue when importing data fails\n    @param thread_nums: maximum number of threads\n    @param skip_packages: skip packages number\n    @param graph: the graph to query.\n    @param json_format: The result is returned in JSON format\n    @param timeout: Maximum execution time, overruns will be interrupted\n    @return: whether the command is executed successfully\n'})})]})}function d(e={}){const{wrapper:t}={...(0,s.R)(),...e.components};return t?(0,r.jsx)(t,{...e,children:(0,r.jsx)(u,{...e})}):u(e)}}}]);