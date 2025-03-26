"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[71860],{28453:(e,r,n)=>{n.d(r,{R:()=>o,x:()=>l});var t=n(96540);const a={},i=t.createContext(a);function o(e){const r=t.useContext(i);return t.useMemo((function(){return"function"==typeof e?e(r):{...r,...e}}),[r,e])}function l(e){let r;return r=e.disableParentContext?"function"==typeof e.components?e.components(a):e.components||a:o(e.components),t.createElement(i.Provider,{value:r},e.children)}},51803:(e,r,n)=>{n.r(r),n.d(r,{assets:()=>p,contentTitle:()=>o,default:()=>d,frontMatter:()=>i,metadata:()=>l,toc:()=>c});var t=n(74848),a=n(28453);const i={},o="TuGraph Java SDK",l={id:"developer-manual/client-tools/java-client",title:"TuGraph Java SDK",description:"1.Compile java client code",source:"@site/versions/version-3.5.1/en-US/source/5.developer-manual/4.client-tools/3.java-client.md",sourceDirName:"5.developer-manual/4.client-tools",slug:"/developer-manual/client-tools/java-client",permalink:"/tugraph-db/en-US/en/3.5.1/developer-manual/client-tools/java-client",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:3,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"TuGraph C++ SDK",permalink:"/tugraph-db/en-US/en/3.5.1/developer-manual/client-tools/cpp-client"},next:{title:"TuGraph-OGM",permalink:"/tugraph-db/en-US/en/3.5.1/developer-manual/client-tools/tugraph-ogm"}},p={},c=[{value:"1.Compile java client code",id:"1compile-java-client-code",level:2},{value:"2.Demo",id:"2demo",level:2},{value:"2.1.Instantiate the client object",id:"21instantiate-the-client-object",level:3},{value:"2.2.Call cypher",id:"22call-cypher",level:3},{value:"2.3.Call stored procedure",id:"23call-stored-procedure",level:3},{value:"2.4.Load stored procedure",id:"24load-stored-procedure",level:3},{value:"2.5.Import from a byte stream schema",id:"25import-from-a-byte-stream-schema",level:3},{value:"2.6.Import point edge data from byte stream",id:"26import-point-edge-data-from-byte-stream",level:3},{value:"2.7.Import from a file schema",id:"27import-from-a-file-schema",level:3},{value:"2.8.Import edge data from a file",id:"28import-edge-data-from-a-file",level:3}];function s(e){const r={code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",p:"p",pre:"pre",...(0,a.R)(),...e.components};return(0,t.jsxs)(t.Fragment,{children:[(0,t.jsx)(r.header,{children:(0,t.jsx)(r.h1,{id:"tugraph-java-sdk",children:"TuGraph Java SDK"})}),"\n",(0,t.jsx)(r.h2,{id:"1compile-java-client-code",children:"1.Compile java client code"}),"\n",(0,t.jsx)(r.pre,{children:(0,t.jsx)(r.code,{className:"language-shell",children:"cd src/client/java/TuGraphRpcClient\nsh local_build.sh\n"})}),"\n",(0,t.jsx)(r.h2,{id:"2demo",children:"2.Demo"}),"\n",(0,t.jsx)(r.h3,{id:"21instantiate-the-client-object",children:"2.1.Instantiate the client object"}),"\n",(0,t.jsx)(r.p,{children:"Introduce dependencies and instantiate"}),"\n",(0,t.jsx)(r.pre,{children:(0,t.jsx)(r.code,{className:"language-java",children:"import com.alipay.tugraph.TuGraphRpcClient;\nTuGraphRpcClient client = new TuGraphRpcClient(url, user, password);\n"})}),"\n",(0,t.jsx)(r.pre,{children:(0,t.jsx)(r.code,{className:"language-shell",children:"# If tugraph is launched from docker\nurl      : list://ip:9090\nuser     : admin\npassword : 73@TuGraph\n"})}),"\n",(0,t.jsx)(r.pre,{children:(0,t.jsx)(r.code,{children:"public TuGraphRpcClient(String url, String user, String pass)\n@param url: tugraph host looks like list://ip:port\n@param user: login user name\n@param password: login password\n"})}),"\n",(0,t.jsx)(r.h3,{id:"22call-cypher",children:"2.2.Call cypher"}),"\n",(0,t.jsx)(r.pre,{children:(0,t.jsx)(r.code,{className:"language-java",children:'    String res = client.callCypher("CALL db.edgeLabels()", "default", 10);\n    log.info("db.edgeLabels() : " + res);\n'})}),"\n",(0,t.jsx)(r.pre,{children:(0,t.jsx)(r.code,{children:"    @param cypher: inquire statement.\n    @param graph: the graph to query.\n    @param timeout: Maximum execution time, overruns will be interrupted\n    @return: the result of cypher query execution\n    public String callCypher(String cypher, String graph, double timeout)\n"})}),"\n",(0,t.jsx)(r.h3,{id:"23call-stored-procedure",children:"2.3.Call stored procedure"}),"\n",(0,t.jsx)(r.pre,{children:(0,t.jsx)(r.code,{className:"language-java",children:'    String result = client.callPlugin("CPP", "khop", kHopParamGen(), 1000, false, "default", 1000);\n    log.info("testCallPlugin : " + result);\n'})}),"\n",(0,t.jsx)(r.pre,{children:(0,t.jsx)(r.code,{children:"    @param pluginType: the plugin type, currently supported CPP and PY\n    @param pluginName: plugin name\n    @param param: the execution parameters\n    @param pluginTimeOut: Maximum execution time, overruns will be interrupted\n    @param graph: the graph to query.\n    @param timeout: Maximum execution time, overruns will be interrupted\n    @return: the result of plugin execution\n    public String callPlugin(String pluginType, String pluginName, String param, double pluginTimeOut,\n            boolean inProcess, String graph, double timeout)\n"})}),"\n",(0,t.jsx)(r.h3,{id:"24load-stored-procedure",children:"2.4.Load stored procedure"}),"\n",(0,t.jsx)(r.pre,{children:(0,t.jsx)(r.code,{className:"language-java",children:'    String result = client.loadPlugin("./test/plugin/khop.so", "CPP", "khop", "SO", "test loadplugin", true, "default", 1000);\n    log.info("loadPlugin : " + result);\n'})}),"\n",(0,t.jsx)(r.pre,{children:(0,t.jsx)(r.code,{children:"    @param sourceFile: the source_file contain plugin code\n    @param pluginType: the plugin type, currently supported CPP and PY\n    @param pluginName: plugin name\n    @param codeType: code type, currently supported PY, SO, CPP, ZIP\n    @param pluginDescription: plugin description\n    @param readOnly: plugin is read only or not\n    @param graph: the graph to query.\n    @param timeout: Maximum execution time, overruns will be interrupted\n    @return: the result of plugin execution\n    public boolean loadPlugin(String sourceFile, String pluginType, String pluginName, String codeType,\n            String pluginDescription, boolean readOnly, String graph, double timeout) throws IOException\n"})}),"\n",(0,t.jsx)(r.h3,{id:"25import-from-a-byte-stream-schema",children:"2.5.Import from a byte stream schema"}),"\n",(0,t.jsx)(r.pre,{children:(0,t.jsx)(r.code,{className:"language-java",children:'    boolean ret = client.importSchemaFromContent(schema, "default", 1000);\n    log.info("importSchemaFromContent : " + ret);\n'})}),"\n",(0,t.jsx)(r.pre,{children:(0,t.jsx)(r.code,{children:"    @param schema: the schema to be imported\n    @param graph: the graph to query.\n    @param timeout: Maximum execution time, overruns will be interrupted\n    @return: the result of import schema\n    public boolean importSchemaFromContent(String schema, String graph, double timeout) throws UnsupportedEncodingException\n"})}),"\n",(0,t.jsx)(r.h3,{id:"26import-point-edge-data-from-byte-stream",children:"2.6.Import point edge data from byte stream"}),"\n",(0,t.jsx)(r.pre,{children:(0,t.jsx)(r.code,{className:"language-java",children:'    boolean ret = client.importDataFromContent(personDesc, person, ",", true, 16, "default", 1000);\n    log.info("importDataFromContent : " + ret);\n'})}),"\n",(0,t.jsx)(r.pre,{children:(0,t.jsx)(r.code,{children:"    @param desc: data format description\n    @param data: the data to be imported\n    @param delimiter: data separator\n    @param continueOnError: whether to continue when importing data fails\n    @param threadNums: maximum number of threads\n    @param graph: the graph to query.\n    @param timeout: Maximum execution time, overruns will be interrupted\n    @return: the result of import data\n    public boolean importDataFromContent(String desc, String data, String delimiter, boolean continueOnError,\n            int threadNums, String graph, double timeout) throws UnsupportedEncodingException\n"})}),"\n",(0,t.jsx)(r.h3,{id:"27import-from-a-file-schema",children:"2.7.Import from a file schema"}),"\n",(0,t.jsx)(r.pre,{children:(0,t.jsx)(r.code,{className:"language-java",children:'    boolean ret = client.importSchemaFromFile("./test/data/yago.conf", "default", 1000);\n    log.info("importSchemaFromFile : " + ret);\n'})}),"\n",(0,t.jsx)(r.pre,{children:(0,t.jsx)(r.code,{children:"    @param schemaFile: the schema_file contain schema\n    @param graph: the graph to query.\n    @param timeout: Maximum execution time, overruns will be interrupted\n    @return: the result of import schema\n    public boolean importSchemaFromFile(String schemaFile, String graph, double timeout)\n            throws UnsupportedEncodingException, IOException\n"})}),"\n",(0,t.jsx)(r.h3,{id:"28import-edge-data-from-a-file",children:"2.8.Import edge data from a file"}),"\n",(0,t.jsx)(r.pre,{children:(0,t.jsx)(r.code,{className:"language-java",children:'    boolean ret = client.importDataFromFile("./test/data/yago.conf", ",", true, 16, 0, "default", 1000000000);\n    log.info("importDataFromFile : " + ret);\n'})}),"\n",(0,t.jsx)(r.pre,{children:(0,t.jsx)(r.code,{children:"    @param confFile: data file contain format description and data\n    @param delimiter: data separator\n    @param continueOnError: whether to continue when importing data fails\n    @param threadNums: maximum number of threads\n    @param skipPackages: skip packages number\n    @param graph: the graph to query.\n    @param timeout: Maximum execution time, overruns will be interrupted\n    @return: the result of import data\n    public boolean importDataFromFile(String confFile, String delimiter, boolean continueOnError, int threadNums,\n            int skipPackages, String graph, double timeout) throws IOException, UnsupportedEncodingException\n"})})]})}function d(e={}){const{wrapper:r}={...(0,a.R)(),...e.components};return r?(0,t.jsx)(r,{...e,children:(0,t.jsx)(s,{...e})}):s(e)}}}]);