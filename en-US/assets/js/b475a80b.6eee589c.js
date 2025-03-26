"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[21812],{28453:(e,r,n)=>{n.d(r,{R:()=>l,x:()=>o});var a=n(96540);const t={},i=a.createContext(t);function l(e){const r=a.useContext(i);return a.useMemo((function(){return"function"==typeof e?e(r):{...r,...e}}),[r,e])}function o(e){let r;return r=e.disableParentContext?"function"==typeof e.components?e.components(t):e.components||t:l(e.components),a.createElement(i.Provider,{value:r},e.children)}},98160:(e,r,n)=>{n.r(r),n.d(r,{assets:()=>c,contentTitle:()=>l,default:()=>d,frontMatter:()=>i,metadata:()=>o,toc:()=>p});var a=n(74848),t=n(28453);const i={},l="Java\u5ba2\u6237\u7aef",o={id:"developer-manual/client-tools/java-client",title:"Java\u5ba2\u6237\u7aef",description:"\u6b64\u6587\u6863\u4e3b\u8981\u662fTuGraph Java SDK\u7684\u4f7f\u7528\u8bf4\u660e\u3002",source:"@site/versions/version-4.0.1/zh-CN/source/5.developer-manual/4.client-tools/3.java-client.md",sourceDirName:"5.developer-manual/4.client-tools",slug:"/developer-manual/client-tools/java-client",permalink:"/tugraph-db/en-US/zh/4.0.1/developer-manual/client-tools/java-client",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:3,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"C++\u5ba2\u6237\u7aef",permalink:"/tugraph-db/en-US/zh/4.0.1/developer-manual/client-tools/cpp-client"},next:{title:"TuGraph-OGM",permalink:"/tugraph-db/en-US/zh/4.0.1/developer-manual/client-tools/tugraph-ogm"}},c={},p=[{value:"1.\u7f16\u8bd1java client\u4ee3\u7801",id:"1\u7f16\u8bd1java-client\u4ee3\u7801",level:2},{value:"2.\u4f7f\u7528\u793a\u4f8b",id:"2\u4f7f\u7528\u793a\u4f8b",level:2},{value:"2.1.\u5b9e\u4f8b\u5316client\u5bf9\u8c61",id:"21\u5b9e\u4f8b\u5316client\u5bf9\u8c61",level:3},{value:"2.1.1.\u5b9e\u4f8b\u5316\u5355\u8282\u70b9client\u5bf9\u8c61",id:"211\u5b9e\u4f8b\u5316\u5355\u8282\u70b9client\u5bf9\u8c61",level:4},{value:"2.1.2.\u5b9e\u4f8b\u5316HA\u96c6\u7fa4\u76f4\u8fde\u8fde\u63a5client\u5bf9\u8c61",id:"212\u5b9e\u4f8b\u5316ha\u96c6\u7fa4\u76f4\u8fde\u8fde\u63a5client\u5bf9\u8c61",level:4},{value:"2.1.3.\u5b9e\u4f8b\u5316HA\u96c6\u7fa4\u95f4\u63a5\u8fde\u63a5client\u5bf9\u8c61",id:"213\u5b9e\u4f8b\u5316ha\u96c6\u7fa4\u95f4\u63a5\u8fde\u63a5client\u5bf9\u8c61",level:4},{value:"2.2.\u8c03\u7528cypher",id:"22\u8c03\u7528cypher",level:3},{value:"2.3.\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",id:"23\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"2.4.\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b",id:"24\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"2.5.\u5217\u4e3e\u5b58\u50a8\u8fc7\u7a0b",id:"25\u5217\u4e3e\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"2.6.\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b",id:"26\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b",level:3},{value:"2.7.\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165schema",id:"27\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165schema",level:3},{value:"2.8.\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e",id:"28\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e",level:3},{value:"2.9.\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165schema",id:"29\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165schema",level:3},{value:"2.10.\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e",id:"210\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e",level:3}];function s(e){const r={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",h4:"h4",header:"header",p:"p",pre:"pre",...(0,t.R)(),...e.components};return(0,a.jsxs)(a.Fragment,{children:[(0,a.jsx)(r.header,{children:(0,a.jsx)(r.h1,{id:"java\u5ba2\u6237\u7aef",children:"Java\u5ba2\u6237\u7aef"})}),"\n",(0,a.jsxs)(r.blockquote,{children:["\n",(0,a.jsx)(r.p,{children:"\u6b64\u6587\u6863\u4e3b\u8981\u662fTuGraph Java SDK\u7684\u4f7f\u7528\u8bf4\u660e\u3002"}),"\n"]}),"\n",(0,a.jsx)(r.h2,{id:"1\u7f16\u8bd1java-client\u4ee3\u7801",children:"1.\u7f16\u8bd1java client\u4ee3\u7801"}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-shell",children:"cd deps/tugraph-db-client-java\nsh local_build.sh\n"})}),"\n",(0,a.jsx)(r.h2,{id:"2\u4f7f\u7528\u793a\u4f8b",children:"2.\u4f7f\u7528\u793a\u4f8b"}),"\n",(0,a.jsx)(r.h3,{id:"21\u5b9e\u4f8b\u5316client\u5bf9\u8c61",children:"2.1.\u5b9e\u4f8b\u5316client\u5bf9\u8c61"}),"\n",(0,a.jsx)(r.p,{children:"\u5f15\u5165\u4f9d\u8d56"}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-java",children:"import com.alipay.tugraph.TuGraphDbRpcClient;\n"})}),"\n",(0,a.jsx)(r.h4,{id:"211\u5b9e\u4f8b\u5316\u5355\u8282\u70b9client\u5bf9\u8c61",children:"2.1.1.\u5b9e\u4f8b\u5316\u5355\u8282\u70b9client\u5bf9\u8c61"}),"\n",(0,a.jsx)(r.p,{children:"\u5f53\u4ee5\u5355\u8282\u70b9\u6a21\u5f0f\u542f\u52a8server\u65f6\uff0cclient\u6309\u7167\u5982\u4e0b\u683c\u5f0f\u8fdb\u884c\u5b9e\u4f8b\u5316"}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-java",children:'TuGraphDbRpcClient client = new TuGraphDbRpcClient("127.0.0.1:19099", "admin", "73@TuGraph");\n'})}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{children:"public TuGraphDbRpcClient(String url, String user, String pass)\n@param url: tugraph host looks like ip:port\n@param user: login user name\n@param password: login password\n"})}),"\n",(0,a.jsx)(r.h4,{id:"212\u5b9e\u4f8b\u5316ha\u96c6\u7fa4\u76f4\u8fde\u8fde\u63a5client\u5bf9\u8c61",children:"2.1.2.\u5b9e\u4f8b\u5316HA\u96c6\u7fa4\u76f4\u8fde\u8fde\u63a5client\u5bf9\u8c61"}),"\n",(0,a.jsx)(r.p,{children:"\u5f53\u670d\u52a1\u5668\u4e0a\u90e8\u7f72\u7684HA\u96c6\u7fa4\u53ef\u4ee5\u4f7f\u7528ha_conf\u4e2d\u914d\u7f6e\u7684\u7f51\u5740\u76f4\u63a5\u8fde\u63a5\u65f6\uff0cclient\u6309\u7167\u5982\u4e0b\u683c\u5f0f\u8fdb\u884c\u5b9e\u4f8b\u5316\u3002"}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-java",children:'TuGraphDbRpcClient client = new TuGraphDbRpcClient("127.0.0.1:19099", "admin", "73@TuGraph");\n'})}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{children:"public TuGraphDbRpcClient(String url, String user, String pass)\n@param url: tugraph host looks like ip:port\n@param user: login user name \n@param password: login password\n"})}),"\n",(0,a.jsx)(r.p,{children:"\u7528\u6237\u53ea\u9700\u8981\u4f20\u5165HA\u96c6\u7fa4\u4e2d\u7684\u4efb\u610f\u4e00\u4e2a\u8282\u70b9\u7684url\u5373\u53ef\uff0cclient\u4f1a\u6839\u636eserver\u7aef\u8fd4\u56de\u7684\u67e5\u8be2\u4fe1\u606f\u81ea\u52a8\u7ef4\u62a4\u8fde\u63a5\u6c60\uff0c\u5728HA\u96c6\u7fa4\u6a2a\u5411\u6269\u5bb9\u65f6\n\u4e5f\u4e0d\u9700\u8981\u624b\u52a8\u91cd\u542fclient\u3002"}),"\n",(0,a.jsx)(r.h4,{id:"213\u5b9e\u4f8b\u5316ha\u96c6\u7fa4\u95f4\u63a5\u8fde\u63a5client\u5bf9\u8c61",children:"2.1.3.\u5b9e\u4f8b\u5316HA\u96c6\u7fa4\u95f4\u63a5\u8fde\u63a5client\u5bf9\u8c61"}),"\n",(0,a.jsx)(r.p,{children:"\u5f53\u670d\u52a1\u5668\u4e0a\u90e8\u7f72\u7684HA\u96c6\u7fa4\u4e0d\u80fd\u4f7f\u7528ha_conf\u4e2d\u914d\u7f6e\u7684\u7f51\u5740\u76f4\u63a5\u8fde\u63a5\u800c\u5fc5\u987b\u4f7f\u7528\u95f4\u63a5\u7f51\u5740\uff08\u5982\u963f\u91cc\u4e91\u516c\u7f51\u7f51\u5740\uff09\u8fde\u63a5\u65f6\uff0c\nclient\u6309\u7167\u5982\u4e0b\u683c\u5f0f\u8fdb\u884c\u5b9e\u4f8b\u5316"}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-java",children:'List<String> urls = new ArrayList<>();\nurls.add("189.33.97.23:9091");\nurls.add("189.33.97.24:9091");\nurls.add("189.33.97.25:9091");\nTuGraphDbRpcClient client = new TuGraphDbRpcClient(urls, "admin", "73@TuGraph");\n'})}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{children:"public TuGraphDbRpcClient(List<String> urls, String user, String password)\n@param urls: tugraph host list\n@param user: login user name\n@param password: login password\n"})}),"\n",(0,a.jsx)(r.p,{children:"\u56e0\u4e3a\u7528\u6237\u8fde\u63a5\u7684\u7f51\u5740\u548cserver\u542f\u52a8\u65f6\u914d\u7f6e\u7684\u4fe1\u606f\u4e0d\u540c\uff0c\u4e0d\u80fd\u901a\u8fc7\u5411\u96c6\u7fa4\u53d1\u8bf7\u6c42\u7684\u65b9\u5f0f\u81ea\u52a8\u66f4\u65b0client\u8fde\u63a5\u6c60\uff0c\u6240\u4ee5\u9700\u8981\u5728\u542f\u52a8\nclient\u65f6\u624b\u52a8\u4f20\u5165\u6240\u6709\u96c6\u7fa4\u4e2d\u8282\u70b9\u7684\u7f51\u5740\uff0c\u5e76\u5728\u96c6\u7fa4\u8282\u70b9\u53d8\u66f4\u65f6\u624b\u52a8\u91cd\u542fclient\u3002"}),"\n",(0,a.jsx)(r.h3,{id:"22\u8c03\u7528cypher",children:"2.2.\u8c03\u7528cypher"}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-java",children:'    String res = client.callCypher("CALL db.edgeLabels()", "default", 10);\n    log.info("db.edgeLabels() : " + res);\n'})}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{children:"    @param cypher: inquire statement.\n    @param graph: the graph to query.\n    @param timeout: Maximum execution time, overruns will be interrupted\n    @param url: (Optional) Node address of calling cypher\n    @return: the result of cypher query execution\n    public String callCypher(String cypher, String graph, double timeout, String url)\n"})}),"\n",(0,a.jsx)(r.p,{children:"\u5176\u4e2d\uff0c\u5728HA\u6a21\u5f0f\u4e0b\u7684client\u4e2d\uff0c\u901a\u8fc7\u6307\u5b9aurl\u53c2\u6570\u53ef\u4ee5\u5b9a\u5411\u5411\u67d0\u4e2aserver\u53d1\u9001\u8bfb\u8bf7\u6c42\u3002\n\u6ce8\uff1aJAVA\u4e0d\u652f\u6301\u9ed8\u8ba4\u53c2\u6570\uff0c\u56e0\u6b64\uff0cJAVA\u4e2d\u7684\u9ed8\u8ba4\u53c2\u6570\u662f\u4f7f\u7528\u91cd\u8f7d\u51fd\u6570\u5b9e\u73b0\u7684\u3002"}),"\n",(0,a.jsx)(r.h3,{id:"23\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b",children:"2.3.\u8c03\u7528\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-java",children:'    String result = client.callProcedure("CPP", "khop", kHopParamGen(), 1000, false, "default");\n    log.info("testCallProcedure : " + result);\n'})}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{children:"    @param procedureType: the procedure type, currently supported CPP and PY\n    @param procedureName: procedure name\n    @param param: the execution parameters\n    @param procedureTimeOut: Maximum execution time, overruns will be interrupted\n    @param inProcess: Running query or not\n    @param graph: the graph to query\n    @param url: (Optional) Node address of calling procedure\n    @return: the result of procedure execution\n    public String callProcedure(String procedureType, String procedureName, String param, double procedureTimeOut,\n            boolean inProcess, String graph, String url)\n"})}),"\n",(0,a.jsx)(r.p,{children:"\u5176\u4e2d\uff0c\u5728HA\u6a21\u5f0f\u4e0b\u7684client\u4e2d\uff0c\u901a\u8fc7\u6307\u5b9aurl\u53c2\u6570\u53ef\u4ee5\u5b9a\u5411\u5411\u67d0\u4e2aserver\u53d1\u9001\u8bfb\u8bf7\u6c42\u3002"}),"\n",(0,a.jsx)(r.h3,{id:"24\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b",children:"2.4.\u52a0\u8f7d\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-java",children:'    boolean result = client.loadProcedure("./test/procedure/khop.so", "CPP", "khop", "SO", "test loadprocedure", true, "v1", "default");\n    log.info("loadProcedure : " + result);\n'})}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{children:"    @param sourceFile: the source_file contain procedure code\n    @param procedureType: the procedure type, currently supported CPP and PY\n    @param procedureName: procedure name\n    @param codeType: code type, currently supported PY, SO, CPP, ZIP\n    @param procedureDescription: procedure description\n    @param readOnly: procedure is read only or not\n    @param version: The version of procedure\n    @param graph: the graph to query.\n    @return: the result of procedure execution\n    public boolean loadProcedure(String sourceFile, String procedureType, String procedureName, String codeType,\n                              String procedureDescription, boolean readOnly, String version, String graph) throws Exception\n"})}),"\n",(0,a.jsx)(r.h3,{id:"25\u5217\u4e3e\u5b58\u50a8\u8fc7\u7a0b",children:"2.5.\u5217\u4e3e\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-java",children:'    String result = client.listProcedures("CPP", "any", "default");\n    log.info("listProcedures : " + result);\n'})}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{children:"    @param procedureType: the procedure type, currently supported CPP and PY\n    @param version: The version of procedure\n    @param graph: the graph to query.\n    @param url: (Optional) Node address of listing procedure\n    @return: the list of procedure\n    public String listProcedures(String procedureType, String version, String graph, String url) throws Exception\n"})}),"\n",(0,a.jsx)(r.p,{children:"\u5176\u4e2d\uff0c\u5728HA\u6a21\u5f0f\u4e0b\u7684client\u4e2d\uff0c\u901a\u8fc7\u6307\u5b9aurl\u53c2\u6570\u53ef\u4ee5\u5b9a\u5411\u5411\u67d0\u4e2aserver\u53d1\u9001\u8bfb\u8bf7\u6c42\u3002"}),"\n",(0,a.jsx)(r.h3,{id:"26\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b",children:"2.6.\u5220\u9664\u5b58\u50a8\u8fc7\u7a0b"}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-java",children:'    String result = client.deleteProcedure("CPP", "sortstr", "default");\n    log.info("loadProcedure : " + result);\n'})}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{children:"    @param procedureType: the procedure type, currently supported CPP and PY\n    @param procedureName: procedure name\n    @param graph: the graph to query.\n    @return: the result of procedure execution\n    public boolean deleteProcedure(String procedureType, String procedureName, String graph) throws Exception\n"})}),"\n",(0,a.jsx)(r.h3,{id:"27\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165schema",children:"2.7.\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165schema"}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-java",children:'    boolean ret = client.importSchemaFromContent(schema, "default", 1000);\n    log.info("importSchemaFromContent : " + ret);\n'})}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{children:"    @param schema: the schema to be imported\n    @param graph: the graph to query.\n    @param timeout: Maximum execution time, overruns will be interrupted\n    @return: the result of import schema\n    public boolean importSchemaFromContent(String schema, String graph, double timeout) throws UnsupportedEncodingException \n"})}),"\n",(0,a.jsx)(r.h3,{id:"28\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e",children:"2.8.\u4ece\u5b57\u8282\u6d41\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e"}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-java",children:'    boolean ret = client.importDataFromContent(personDesc, person, ",", true, 16, "default", 1000);\n    log.info("importDataFromContent : " + ret);\n'})}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{children:"    @param desc: data format description\n    @param data: the data to be imported\n    @param delimiter: data separator\n    @param continueOnError: whether to continue when importing data fails\n    @param threadNums: maximum number of threads\n    @param graph: the graph to query.\n    @param timeout: Maximum execution time, overruns will be interrupted\n    @return: the result of import data\n    public boolean importDataFromContent(String desc, String data, String delimiter, boolean continueOnError,\n            int threadNums, String graph, double timeout) throws UnsupportedEncodingException\n"})}),"\n",(0,a.jsx)(r.h3,{id:"29\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165schema",children:"2.9.\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165schema"}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-java",children:'    boolean ret = client.importSchemaFromFile("./test/data/yago.conf", "default", 1000);\n    log.info("importSchemaFromFile : " + ret);\n'})}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{children:"    @param schemaFile: the schema_file contain schema\n    @param graph: the graph to query.\n    @param timeout: Maximum execution time, overruns will be interrupted\n    @return: the result of import schema\n    public boolean importSchemaFromFile(String schemaFile, String graph, double timeout) \n            throws UnsupportedEncodingException, IOException\n"})}),"\n",(0,a.jsx)(r.h3,{id:"210\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e",children:"2.10.\u4ece\u6587\u4ef6\u4e2d\u5bfc\u5165\u70b9\u8fb9\u6570\u636e"}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-java",children:'    boolean ret = client.importDataFromFile("./test/data/yago.conf", ",", true, 16, 0, "default", 1000000000);\n    log.info("importDataFromFile : " + ret);\n'})}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{children:"    @param confFile: data file contain format description and data\n    @param delimiter: data separator\n    @param continueOnError: whether to continue when importing data fails\n    @param threadNums: maximum number of threads\n    @param skipPackages: skip packages number\n    @param graph: the graph to query.\n    @param timeout: Maximum execution time, overruns will be interrupted\n    @return: the result of import data\n    public boolean importDataFromFile(String confFile, String delimiter, boolean continueOnError, int threadNums,\n            int skipPackages, String graph, double timeout) throws IOException, UnsupportedEncodingException\n"})})]})}function d(e={}){const{wrapper:r}={...(0,t.R)(),...e.components};return r?(0,a.jsx)(r,{...e,children:(0,a.jsx)(s,{...e})}):s(e)}}}]);