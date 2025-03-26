"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[85812],{15590:(e,t,n)=>{n.r(t),n.d(t,{assets:()=>c,contentTitle:()=>o,default:()=>h,frontMatter:()=>l,metadata:()=>i,toc:()=>d});var r=n(74848),s=n(28453);const l={},o="TuGraph Python SDK",i={id:"client-tools/python-client",title:"TuGraph Python SDK",description:"This document is the usage instruction of TuGraph Python SDK",source:"@site/versions/version-4.3.2/en-US/source/7.client-tools/1.python-client.md",sourceDirName:"7.client-tools",slug:"/client-tools/python-client",permalink:"/tugraph-db/en-US/en/4.3.2/client-tools/python-client",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:1,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"TuGraph Explorer",permalink:"/tugraph-db/en-US/en/4.3.2/utility-tools/tugraph-explorer"},next:{title:"TuGraph C++ SDK",permalink:"/tugraph-db/en-US/en/4.3.2/client-tools/cpp-client"}},c={},d=[{value:"1.Overview",id:"1overview",level:2},{value:"2.RESTful Client",id:"2restful-client",level:2},{value:"2.1. Install Client",id:"21-install-client",level:3},{value:"2.2.Call cypher",id:"22call-cypher",level:3},{value:"2.3.Call stored procedure",id:"23call-stored-procedure",level:3},{value:"3.RPC Client",id:"3rpc-client",level:2},{value:"3.1.Instantiate the client object",id:"31instantiate-the-client-object",level:3},{value:"3.1.1.Instantiate a single node client object",id:"311instantiate-a-single-node-client-object",level:4},{value:"3.1.2.Instantiate the HA cluster to directly connect to the client object",id:"312instantiate-the-ha-cluster-to-directly-connect-to-the-client-object",level:4},{value:"3.1.3.Instantiate the HA cluster to indirectly connect to the client object",id:"313instantiate-the-ha-cluster-to-indirectly-connect-to-the-client-object",level:4},{value:"3.2.Call cypher",id:"32call-cypher",level:3},{value:"3.3.Send cypher request to leader",id:"33send-cypher-request-to-leader",level:3},{value:"3.4.Call GQL",id:"34call-gql",level:3},{value:"3.5.Send GQL request to leader",id:"35send-gql-request-to-leader",level:3},{value:"3.6.Calling stored procedures",id:"36calling-stored-procedures",level:3},{value:"3.7.Call the stored procedure to the leader",id:"37call-the-stored-procedure-to-the-leader",level:3},{value:"3.8.Load stored procedure",id:"38load-stored-procedure",level:3},{value:"3.9.List stored procedures",id:"39list-stored-procedures",level:3},{value:"3.10.Delete stored procedures",id:"310delete-stored-procedures",level:3},{value:"3.11.Import schema from byte stream",id:"311import-schema-from-byte-stream",level:3},{value:"3.12.Import edge data from byte stream",id:"312import-edge-data-from-byte-stream",level:3},{value:"3.13.Import schema from file",id:"313import-schema-from-file",level:3},{value:"3.14.Import point and edge data from file",id:"314import-point-and-edge-data-from-file",level:3}];function a(e){const t={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",h4:"h4",header:"header",p:"p",pre:"pre",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",...(0,s.R)(),...e.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(t.header,{children:(0,r.jsx)(t.h1,{id:"tugraph-python-sdk",children:"TuGraph Python SDK"})}),"\n",(0,r.jsxs)(t.blockquote,{children:["\n",(0,r.jsx)(t.p,{children:"This document is the usage instruction of TuGraph Python SDK"}),"\n"]}),"\n",(0,r.jsx)(t.h2,{id:"1overview",children:"1.Overview"}),"\n",(0,r.jsx)(t.p,{children:"There are two kinds of Python's TuGraph Client. One is the RESTful Client, which uses the HTTP method to send requests to the server. The other is the RPC Client, which uses the RPC method to call the server remote service. Both have their own advantages and disadvantages. The RESTful client is simple to use. You can find the source code file of the Client in the src/client/python/TuGraphClient directory of the project. You can install it directly into the user environment and use it. However, it supports fewer functions and its performance is not high. RPC Client supports both stand-alone servers and high-availability clusters and load balancing. It has many interfaces and powerful functions. However, the usage is more complicated. Users need to compile the TuGraph project themselves to obtain liblgraph_client_python.so, or directly find the dependent library in the /usr/local/lib64 directory when using the runtime image, and introduce it into the python project for normal use. Next, the usage of these two Clients will be introduced in detail."}),"\n",(0,r.jsx)(t.h2,{id:"2restful-client",children:"2.RESTful Client"}),"\n",(0,r.jsx)(t.h3,{id:"21-install-client",children:"2.1. Install Client"}),"\n",(0,r.jsx)(t.p,{children:"TuGraph's Python RESTful client uses the setuptools tool for packaging and distribution. Users can install the client directly into the local environment and introduce it directly when using it."}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-shell",children:"cd src/client/python/TuGraphClient\npython3 setup.py build\npython3 setup.py install\n"})}),"\n",(0,r.jsx)(t.p,{children:"Note: When using the setuptools tool to install the python client, dependencies such as httpx will be installed and need to be executed in an environment with external network access."}),"\n",(0,r.jsx)(t.h3,{id:"22call-cypher",children:"2.2.Call cypher"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-python",children:'from TuGraphClient import TuGraphClient, AsyncTuGraphClient\n\nclient = TuGraphClient("127.0.0.1:7071" , "admin", "73@TuGraph")\ncypher = "match (n) return properties(n) limit 1"\nres = client.call_cypher(cypher)\nprint(res)\n'})}),"\n",(0,r.jsx)(t.h3,{id:"23call-stored-procedure",children:"2.3.Call stored procedure"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-python",children:'from TuGraphClient import TuGraphClient, AsyncTuGraphClient\n\nclient = TuGraphClient("127.0.0.1:7071" , "admin", "73@TuGraph")\nplugin_type = "cpp"\nplugin_name = "khop"\nplugin_input = "{\\"root\\": 10, \\"hop\\": 3}"\nres = client.call_plugin(plugin_type, plguin_name, plugin_input)\nprint(res)\n'})}),"\n",(0,r.jsx)(t.h2,{id:"3rpc-client",children:"3.RPC Client"}),"\n",(0,r.jsx)(t.p,{children:"Python's TuGraph Rpc Client is a CPP Client SDK packaged using pybind11. The following table lists the correspondence between Python and CPP Client SDK."}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(t.table,{children:[(0,r.jsx)(t.thead,{children:(0,r.jsxs)(t.tr,{children:[(0,r.jsx)(t.th,{children:"Python Client SDK"}),(0,r.jsx)(t.th,{children:"CPP Client SDK"})]})}),(0,r.jsxs)(t.tbody,{children:[(0,r.jsxs)(t.tr,{children:[(0,r.jsx)(t.td,{children:"client(self: liblgraph_client_python.client, url: str, user: str, password: str)"}),(0,r.jsx)(t.td,{children:"RpcClient(const std::string& url, const std::string& user, const std::string& password)"})]}),(0,r.jsxs)(t.tr,{children:[(0,r.jsx)(t.td,{children:"client(self: liblgraph_client_python.client, urls: list, user: str, password: str)"}),(0,r.jsxs)(t.td,{children:["RpcClient(std::vector",(0,r.jsx)(t.a,{href:"std::string",children:"std::string"}),"& urls, std::string user, std::string password)"]})]}),(0,r.jsxs)(t.tr,{children:[(0,r.jsx)(t.td,{children:"callCypher(self: liblgraph_client_python.client, cypher: str, graph: str, json_format: bool, timeout: float, url: str) -> (bool, str)"}),(0,r.jsx)(t.td,{children:"bool CallCypher(std::string& result, const std::string& cypher, const std::string& graph, bool json_format, double timeout, const std::string& url)"})]}),(0,r.jsxs)(t.tr,{children:[(0,r.jsx)(t.td,{children:"callCypherToLeader(self: liblgraph_client_python.client, cypher: str, graph: str, json_format: bool, timeout: float) -> (bool, str)"}),(0,r.jsx)(t.td,{children:"bool CallCypherToLeader(std::string& result, const std::string& cypher, const std::string& graph, bool json_format, double timeout)"})]}),(0,r.jsxs)(t.tr,{children:[(0,r.jsx)(t.td,{children:"callGql(self: liblgraph_client_python.client, gql: str, graph: str, json_format: bool, timeout: float, url: str) -> (bool, str)"}),(0,r.jsx)(t.td,{children:"bool CallGql(std::string& result, const std::string& gql, const std::string& graph, bool json_format, double timeout, const std::string& url)"})]}),(0,r.jsxs)(t.tr,{children:[(0,r.jsx)(t.td,{children:"callGqlToLeader(self: liblgraph_client_python.client, gql: str, graph: str, json_format: bool, timeout: float) -> (bool, str)"}),(0,r.jsx)(t.td,{children:'bool CallGqlToLeader(std::string& result, const std::string& gql, const std::string& graph = "default", bool json_format = true, double timeout = 0)'})]}),(0,r.jsxs)(t.tr,{children:[(0,r.jsx)(t.td,{children:"callProcedure(self: liblgraph_client_python.client, procedure_type: str, procedure_name: str, param: str, procedure_time_out: float, in_process: bool, graph: str, json_format: bool, url: str) -> (bool, str)"}),(0,r.jsx)(t.td,{children:"bool CallProcedure(std::string& result, const std::string& procedure_type, const std::string& procedure_name, const std::string& param, double procedure_time_out, bool in_process, const std::string& graph, bool json_format, const std::string& url)"})]}),(0,r.jsxs)(t.tr,{children:[(0,r.jsx)(t.td,{children:"callProcedureToLeader(self: liblgraph_client_python.client, procedure_type: str, procedure_name: str, param: str, procedure_time_out: float, in_process: bool, graph: str, json_format: bool) -> (bool, str)"}),(0,r.jsx)(t.td,{children:"CallProcedureToLeader(std::string& result, const std::string& procedure_type, const std::string& procedure_name, const std::string& param, double procedure_time_out, bool in_process, const std::string& graph, bool json_format)"})]}),(0,r.jsxs)(t.tr,{children:[(0,r.jsx)(t.td,{children:"loadProcedure(self: liblgraph_client_python.client, source_file: str, procedure_type: str, procedure_name: str, code_type: str, procedure_description: str, read_only: bool, version: str, graph: str) -> (bool, str)"}),(0,r.jsx)(t.td,{children:"bool LoadProcedure(std::string& result, const std::string& source_file, const std::string& procedure_type, const std::string& procedure_name, const std::string& code_type, const std::string& procedure_description, bool read_only, const std::string& version, const std::string& graph)"})]}),(0,r.jsxs)(t.tr,{children:[(0,r.jsx)(t.td,{children:"listProcedures(self: liblgraph_client_python.client, procedure_type: str, version: str, graph: str, url: str) -> (bool, str)"}),(0,r.jsx)(t.td,{children:"bool ListProcedures(std::string& result, const std::string& procedure_type, const std::string& version, const std::string& graph, const std::string& url)"})]}),(0,r.jsxs)(t.tr,{children:[(0,r.jsx)(t.td,{children:"deleteProcedure(self: liblgraph_client_python.client, procedure_type: str, procedure_name: str, graph: str) -> (bool, str)"}),(0,r.jsx)(t.td,{children:"bool DeleteProcedure(std::string& result, const std::string& procedure_type, const std::string& procedure_name, const std::string& graph)"})]}),(0,r.jsxs)(t.tr,{children:[(0,r.jsx)(t.td,{children:"importSchemaFromContent(self: liblgraph_client_python.client, schema: str, graph: str, json_format: bool, timeout: float) -> (bool, str)"}),(0,r.jsx)(t.td,{children:"bool ImportSchemaFromContent(std::string& result, const std::string& schema, const std::string& graph, bool json_format, double timeout)"})]}),(0,r.jsxs)(t.tr,{children:[(0,r.jsx)(t.td,{children:"importDataFromContent(self: liblgraph_client_python.client, desc: str, data: str, delimiter: str, continue_on_error: bool, thread_nums: int, graph: str, json_format: bool, timeout: float) -> (bool, str)"}),(0,r.jsx)(t.td,{children:"ImportDataFromContent(std::string& result, const std::string& desc, const std::string& data, const std::string& delimiter, bool continue_on_error, int thread_nums, const std::string& graph, bool json_format, double timeout)"})]}),(0,r.jsxs)(t.tr,{children:[(0,r.jsx)(t.td,{children:"importSchemaFromFile(self: liblgraph_client_python.client, schema_file: str, graph: str, json_format: bool, timeout: float) -> (bool, str)"}),(0,r.jsx)(t.td,{children:"ImportSchemaFromFile(std::string& result, const std::string& schema_file, const std::string& graph, bool json_format, double timeout)"})]}),(0,r.jsxs)(t.tr,{children:[(0,r.jsx)(t.td,{children:"importDataFromFile(self: liblgraph_client_python.client, conf_file: str, delimiter: str, continue_on_error: bool, thread_nums: int, skip_packages: int, graph: str, json_format: bool, timeout: float) -> (bool, str)"}),(0,r.jsx)(t.td,{children:"ImportDataFromFile(std::string& result, const std::string& conf_file, const std::string& delimiter, bool continue_on_error, int thread_nums, int skip_packages, const std::string& graph, bool json_format, double timeout)"})]})]})]}),"\n",(0,r.jsx)(t.p,{children:"The use of Python RPC Client is more complicated. Users can compile TuGraph in the local environment to get liblgraph_client_python.so, or they can use the official runtime image. The dependent library can be found directly in the /usr/local/lib64 directory in the image. It can be used after introducing the user project."}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-python",children:"import liblgraph_client_python\n"})}),"\n",(0,r.jsx)(t.h3,{id:"31instantiate-the-client-object",children:"3.1.Instantiate the client object"}),"\n",(0,r.jsx)(t.h4,{id:"311instantiate-a-single-node-client-object",children:"3.1.1.Instantiate a single node client object"}),"\n",(0,r.jsx)(t.p,{children:"When starting the server in single-node mode, the client is instantiated in the following format"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-python",children:'client = liblgraph_client_python.client("127.0.0.1:19099", "admin", "73@TuGraph")\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:"client(self: liblgraph_client_python.client, url: str, user: str, password: str)\n"})}),"\n",(0,r.jsx)(t.h4,{id:"312instantiate-the-ha-cluster-to-directly-connect-to-the-client-object",children:"3.1.2.Instantiate the HA cluster to directly connect to the client object"}),"\n",(0,r.jsx)(t.p,{children:"When the HA cluster deployed on the server can be directly connected using the URL configured in ha_conf, the client is instantiated according to the following format"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-python",children:'client = liblgraph_client_python.client("127.0.0.1:19099", "admin", "73@TuGraph")\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:"client(self: liblgraph_client_python.client, url: str, user: str, password: str)\n"})}),"\n",(0,r.jsx)(t.p,{children:"The user only needs to pass in the url of any node in the HA cluster, and the client will automatically maintain the connection pool based on the query information returned by the server, and there is no need to manually restart the client when the HA cluster expands horizontally."}),"\n",(0,r.jsx)(t.h4,{id:"313instantiate-the-ha-cluster-to-indirectly-connect-to-the-client-object",children:"3.1.3.Instantiate the HA cluster to indirectly connect to the client object"}),"\n",(0,r.jsx)(t.p,{children:"When the HA cluster deployed on the server cannot use the URL configured in ha_conf to connect directly but must use an indirect URL (such as the Alibaba Cloud public network URL), the client is instantiated according to the following format."}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-python",children:'client = liblgraph_client_python.client(["189.33.97.23:9091","189.33.97.24:9091", "189.33.97.25:9091"], "admin", "73@TuGraph")\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:"client(self: liblgraph_client_python.client, urls: list, user: str, password: str)\n"})}),"\n",(0,r.jsx)(t.p,{children:"Because the URL that the user connects to is different from the information configured when the server starts, the client connection pool cannot be automatically updated by sending a request to the cluster, so it is necessary to manually pass in the URLs of all nodes in the cluster when starting the client, and when the cluster node changes Manually restart the client."}),"\n",(0,r.jsx)(t.h3,{id:"32call-cypher",children:"3.2.Call cypher"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-python",children:'ret, res = client.callCypher("CALL db.edgeLabels()", "default", 10)\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:"callCypher(self: liblgraph_client_python.client, cypher: str, graph: str, json_format: bool, timeout: float, url: str) -> (bool, str)\n"})}),"\n",(0,r.jsx)(t.p,{children:"This interface supports use in stand-alone mode and HA mode. Among them, in the client in HA mode, a read request can be directed to a server by specifying the url parameter."}),"\n",(0,r.jsx)(t.h3,{id:"33send-cypher-request-to-leader",children:"3.3.Send cypher request to leader"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-python",children:'ret, res = client.callCypherToLeader("CALL db.edgeLabels()", "default", 10)\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:"callCypherToLeader(self: liblgraph_client_python.client, cypher: str, graph: str, json_format: bool, timeout: float) -> (bool, str)\n"})}),"\n",(0,r.jsx)(t.p,{children:"This interface only supports use in HA mode. In the client in HA mode, in order to prevent requests from being sent to followers with unsynchronized data,\nUsers can directly send requests to the leader, and the leader is elected by the cluster."}),"\n",(0,r.jsx)(t.h3,{id:"34call-gql",children:"3.4.Call GQL"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-python",children:'ret, res = client.callGql("CALL db.edgeLabels()", "default", 10)\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:"callGql(self: liblgraph_client_python.client, gql: str, graph: str, json_format: bool, timeout: float, url: str) -> (bool, str)\n"})}),"\n",(0,r.jsx)(t.p,{children:"This interface supports use in stand-alone mode and HA mode. Among them, in the client in HA mode, a read request can be directed to a server by specifying the url parameter."}),"\n",(0,r.jsx)(t.h3,{id:"35send-gql-request-to-leader",children:"3.5.Send GQL request to leader"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-python",children:'ret, res = client.callGqlToLeader("CALL db.edgeLabels()", "default", 10)\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:"callGqlToLeader(self: liblgraph_client_python.client, gql: str, graph: str, json_format: bool, timeout: float) -> (bool, str)\n"})}),"\n",(0,r.jsx)(t.p,{children:"This interface only supports use in HA mode. In the client in HA mode, in order to prevent requests from being sent to followers with unsynchronized data,\nUsers can directly send requests to the leader, and the leader is elected by the cluster."}),"\n",(0,r.jsx)(t.h3,{id:"36calling-stored-procedures",children:"3.6.Calling stored procedures"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-python",children:'ret, res = client.callProcedure("CPP", "test_plugin1", "bcefg", 1000, False, "default")\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:"callProcedure(self: liblgraph_client_python.client, procedure_type: str, procedure_name: str, param: str, procedure_time_out: float, in_process: bool, graph: str, json_format: bool, url: str) -> (bool, str)\n"})}),"\n",(0,r.jsx)(t.p,{children:"This interface supports use in stand-alone mode and HA mode. By default, the execution result of the stored procedure is directly returned in string format. Specifying jsonFormat as true can return the execution result in json format.\nAmong them, in the client in HA mode, a read request can be directed to a server by specifying the url parameter."}),"\n",(0,r.jsx)(t.h3,{id:"37call-the-stored-procedure-to-the-leader",children:"3.7.Call the stored procedure to the leader"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-python",children:'ret, res = client.callProcedureToLeader("CPP", "khop", kHopParamGen(), 1000, false, "default")\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:"callProcedureToLeader(self: liblgraph_client_python.client, procedure_type: str, procedure_name: str, param: str, procedure_time_out: float, in_process: bool, graph: str, json_format: bool) -> (bool, str)\n"})}),"\n",(0,r.jsx)(t.p,{children:"This interface supports use in HA mode. By default, the execution result of the stored procedure is directly returned in string format. Specifying jsonFormat as true can return the execution result in json format."}),"\n",(0,r.jsx)(t.h3,{id:"38load-stored-procedure",children:"3.8.Load stored procedure"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-python",children:'ret, res = client.loadProcedure("./test/procedure/khop.so", "CPP", "khop", "SO", "test loadprocedure", true, "v1", "default");\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:"loadProcedure(self: liblgraph_client_python.client, source_file: str, procedure_type: str, procedure_name: str, code_type: str, procedure_description: str, read_only: bool, version: str, graph: str) -> (bool, str)\n"})}),"\n",(0,r.jsx)(t.p,{children:"This interface supports use in stand-alone mode and HA mode. Among them, since loading a stored procedure is a write request, the client in HA mode can only send a request to load a stored procedure to the leader."}),"\n",(0,r.jsx)(t.h3,{id:"39list-stored-procedures",children:"3.9.List stored procedures"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-python",children:'ret, res = client.listProcedures("CPP", "any", "default")\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:"listProcedures(self: liblgraph_client_python.client, procedure_type: str, version: str, graph: str, url: str) -> (bool, str)\n"})}),"\n",(0,r.jsx)(t.p,{children:"This interface supports use in stand-alone mode and HA mode. Among them, in the client in HA mode, a read request can be directed to a server by specifying the url parameter."}),"\n",(0,r.jsx)(t.h3,{id:"310delete-stored-procedures",children:"3.10.Delete stored procedures"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-python",children:'ret, res = client.deleteProcedure("CPP", "sortstr", "default")\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:"deleteProcedure(self: liblgraph_client_python.client, procedure_type: str, procedure_name: str, graph: str) -> (bool, str)\n"})}),"\n",(0,r.jsx)(t.p,{children:"This interface supports use in stand-alone mode and HA mode. Among them, since deleting a stored procedure is a write request, the client in HA mode can only send a delete request to the leader."}),"\n",(0,r.jsx)(t.h3,{id:"311import-schema-from-byte-stream",children:"3.11.Import schema from byte stream"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-python",children:'ret, res = client.importSchemaFromContent(schema, "default", 1000)\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:"importSchemaFromContent(self: liblgraph_client_python.client, schema: str, graph: str, json_format: bool, timeout: float) -> (bool, str)\n"})}),"\n",(0,r.jsx)(t.p,{children:"This interface supports use in stand-alone mode and HA mode. Among them, since importing schema is a write request, the client in HA mode can only send an import schema request to the leader."}),"\n",(0,r.jsx)(t.h3,{id:"312import-edge-data-from-byte-stream",children:"3.12.Import edge data from byte stream"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-python",children:'ret, res = client.importDataFromContent(personDesc, person, ",", true, 16, "default", 1000)\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:"importDataFromContent(self: liblgraph_client_python.client, desc: str, data: str, delimiter: str, continue_on_error: bool, thread_nums: int, graph: str, json_format: bool, timeout: float) -> (bool, str)\n"})}),"\n",(0,r.jsx)(t.p,{children:"This interface supports use in stand-alone mode and HA mode. Among them, since importing point and edge data is a write request, the client in HA mode can only send a request to import point and edge data to the leader."}),"\n",(0,r.jsx)(t.h3,{id:"313import-schema-from-file",children:"3.13.Import schema from file"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-python",children:'ret, res = client.importSchemaFromFile("./test/data/yago.conf", "default", 1000)\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:"importSchemaFromFile(self: liblgraph_client_python.client, schema_file: str, graph: str, json_format: bool, timeout: float) -> (bool, str)\n"})}),"\n",(0,r.jsx)(t.p,{children:"This interface supports use in stand-alone mode and HA mode. Among them, since importing schema is a write request, the client in HA mode can only send an import schema request to the leader."}),"\n",(0,r.jsx)(t.h3,{id:"314import-point-and-edge-data-from-file",children:"3.14.Import point and edge data from file"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-python",children:'ret, res = client.importDataFromFile("./test/data/yago.conf", ",", true, 16, 0, "default", 1000000000)\n'})}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{children:"importDataFromFile(self: liblgraph_client_python.client, conf_file: str, delimiter: str, continue_on_error: bool, thread_nums: int, skip_packages: int, graph: str, json_format: bool, timeout: float) -> (bool, str)\n"})}),"\n",(0,r.jsx)(t.p,{children:"This interface supports use in stand-alone mode and HA mode. Among them, since importing point and edge data is a write request, the client in HA mode can only send a request to import point and edge data to the leader."})]})}function h(e={}){const{wrapper:t}={...(0,s.R)(),...e.components};return t?(0,r.jsx)(t,{...e,children:(0,r.jsx)(a,{...e})}):a(e)}},28453:(e,t,n)=>{n.d(t,{R:()=>o,x:()=>i});var r=n(96540);const s={},l=r.createContext(s);function o(e){const t=r.useContext(l);return r.useMemo((function(){return"function"==typeof e?e(t):{...t,...e}}),[t,e])}function i(e){let t;return t=e.disableParentContext?"function"==typeof e.components?e.components(s):e.components||s:o(e.components),r.createElement(l.Provider,{value:t},e.children)}}}]);