"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[23698],{28453:(e,t,r)=>{r.d(t,{R:()=>i,x:()=>d});var n=r(96540);const s={},a=n.createContext(s);function i(e){const t=n.useContext(a);return n.useMemo((function(){return"function"==typeof e?e(t):{...t,...e}}),[t,e])}function d(e){let t;return t=e.disableParentContext?"function"==typeof e.components?e.components(s):e.components||s:i(e.components),n.createElement(a.Provider,{value:t},e.children)}},87575:(e,t,r)=>{r.r(t),r.d(t,{assets:()=>o,contentTitle:()=>i,default:()=>h,frontMatter:()=>a,metadata:()=>d,toc:()=>c});var n=r(74848),s=r(28453);const a={},i="Integration Testing",d={id:"quality/integration-testing",title:"Integration Testing",description:"This document mainly introduces how to use the TuGraph integration testing framework.",source:"@site/versions/version-4.5.1/en-US/source/11.quality/2.integration-testing.md",sourceDirName:"11.quality",slug:"/quality/integration-testing",permalink:"/tugraph-db/en/4.5.1/quality/integration-testing",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:2,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Unit Testing",permalink:"/tugraph-db/en/4.5.1/quality/unit-testing"},next:{title:"TuGraph Contribution Guide",permalink:"/tugraph-db/en/4.5.1/contributor-manual/contributing"}},o={},c=[{value:"1.The Significance of TuGraph Integration Testing",id:"1the-significance-of-tugraph-integration-testing",level:2},{value:"2.TuGraph Integration Testing Framework",id:"2tugraph-integration-testing-framework",level:2},{value:"2.1.Component Description",id:"21component-description",level:3},{value:"2.2.Component Usage",id:"22component-usage",level:3},{value:"2.2.1.server",id:"221server",level:4},{value:"2.2.1.1.Startup Parameters",id:"2211startup-parameters",level:5},{value:"2.2.1.2.Startup Command",id:"2212startup-command",level:5},{value:"2.2.2.client",id:"222client",level:4},{value:"2.2.2.1.Startup Parameters",id:"2221startup-parameters",level:5},{value:"2.2.2.2.Startup Command",id:"2222startup-command",level:5},{value:"2.2.3.importor",id:"223importor",level:4},{value:"2.2.3.1.Startup Parameters",id:"2231startup-parameters",level:5},{value:"2.2.3.2.Startup Command",id:"2232startup-command",level:5},{value:"2.2.4.exportor",id:"224exportor",level:4},{value:"2.2.4.1.Startup Parameters",id:"2241startup-parameters",level:5},{value:"2.2.4.2.Startup Command",id:"2242startup-command",level:5},{value:"2.2.5.backup_binlog",id:"225backup_binlog",level:4},{value:"2.2.5.1.Startup Parameters",id:"2251startup-parameters",level:5},{value:"2.2.5.2.Startup Command",id:"2252startup-command",level:5},{value:"2.2.6.backup_copy_dir",id:"226backup_copy_dir",level:4},{value:"2.2.6.1.Startup Parameters",id:"2261startup-parameters",level:5},{value:"2.2.6.2.Startup Command",id:"2262startup-command",level:5},{value:"2.2.7.build_so",id:"227build_so",level:4},{value:"2.2.7.1.Startup Parameters",id:"2271startup-parameters",level:5},{value:"2.2.7.2.Startup Command",id:"2272startup-command",level:5},{value:"2.2.8.copy_snapshot",id:"228copy_snapshot",level:4},{value:"2.2.8.1.Startup Parameters",id:"2281startup-parameters",level:5},{value:"2.2.8.2.Startup Command",id:"2282startup-command",level:5},{value:"2.2.9.copy_dir",id:"229copy_dir",level:4},{value:"2.2.9.1.Startup Parameters",id:"2291startup-parameters",level:5},{value:"2.2.9.2.Startup Command",id:"2292startup-command",level:5},{value:"2.2.10.exec",id:"2210exec",level:4},{value:"2.2.10.1.Startup Parameters",id:"22101startup-parameters",level:5},{value:"2.2.10.2.Startup Command",id:"22102startup-command",level:5},{value:"2.2.11.algo",id:"2211algo",level:4},{value:"2.2.11.1.Startup Parameters",id:"22111startup-parameters",level:5},{value:"2.2.11.2.Startup Command",id:"22112startup-command",level:5},{value:"2.2.12.bash",id:"2212bash",level:4},{value:"2.2.12.1.Startup Parameters",id:"22121startup-parameters",level:5},{value:"2.2.12.2.Startup Command",id:"22122startup-command",level:5},{value:"2.2.13.rest",id:"2213rest",level:4},{value:"2.2.13.1.Startup Parameters",id:"22131startup-parameters",level:5},{value:"2.2.13.2.Startup Command",id:"22132startup-command",level:5},{value:"2.3.Test Cases",id:"23test-cases",level:3},{value:"2.3.1.rest",id:"231rest",level:4},{value:"2.3.2.client",id:"232client",level:4},{value:"2.3.3.exportor/importor",id:"233exportorimportor",level:4},{value:"2.3.4.Other Tests",id:"234other-tests",level:4}];function l(e){const t={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",h4:"h4",h5:"h5",header:"header",li:"li",p:"p",pre:"pre",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",ul:"ul",...(0,s.R)(),...e.components};return(0,n.jsxs)(n.Fragment,{children:[(0,n.jsx)(t.header,{children:(0,n.jsx)(t.h1,{id:"integration-testing",children:"Integration Testing"})}),"\n",(0,n.jsxs)(t.blockquote,{children:["\n",(0,n.jsx)(t.p,{children:"This document mainly introduces how to use the TuGraph integration testing framework."}),"\n"]}),"\n",(0,n.jsx)(t.h2,{id:"1the-significance-of-tugraph-integration-testing",children:"1.The Significance of TuGraph Integration Testing"}),"\n",(0,n.jsx)(t.p,{children:"In unit tests and function tests, some test cases directly use Galaxy or Statemachine to perform tests, which is not a complete process. In the complete CS architecture, user requests are sent to the server through the client, and network communication is essential. To avoid bugs caused by incomplete unit testing, TuGraph uses an integration testing framework to perform end-to-end testing."}),"\n",(0,n.jsx)(t.h2,{id:"2tugraph-integration-testing-framework",children:"2.TuGraph Integration Testing Framework"}),"\n",(0,n.jsx)(t.p,{children:"TuGraph uses the Pytest framework as its integration testing framework. Pytest is currently the most widely used CS-side integration testing framework. It is known for its flexibility, ease of use, and the ability to support parameterization. Based on the functionality provided by Pytest, TuGraph abstracts different tools and controls the processing logic of each tool through parameters to facilitate efficient testing code development."}),"\n",(0,n.jsxs)(t.p,{children:["For more information on Pytest, please refer to the official website: ",(0,n.jsx)(t.a,{href:"https://docs.pytest.org/en/7.2.x/getting-started.html",children:"https://docs.pytest.org/en/7.2.x/getting-started.html"})]}),"\n",(0,n.jsx)(t.h3,{id:"21component-description",children:"2.1.Component Description"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,n.jsxs)(t.table,{children:[(0,n.jsx)(t.thead,{children:(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.th,{children:"Component Name"}),(0,n.jsx)(t.th,{children:"Component Function"}),(0,n.jsx)(t.th,{children:"Implementation Method"})]})}),(0,n.jsxs)(t.tbody,{children:[(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"server"}),(0,n.jsx)(t.td,{children:"TuGraph standalone service"}),(0,n.jsx)(t.td,{children:"Start a child process and launch the service"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"client"}),(0,n.jsx)(t.td,{children:"TuGraph Rpc Client"}),(0,n.jsx)(t.td,{children:"Open TuGraph Python Rpc Client in the current process and send requests"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"importor"}),(0,n.jsx)(t.td,{children:"TuGraph Importor"}),(0,n.jsx)(t.td,{children:"Start a child process to process import requests"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"exportor"}),(0,n.jsx)(t.td,{children:"TuGraph Exportor"}),(0,n.jsx)(t.td,{children:"Start a child process to process export requests"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"backup_binlog"}),(0,n.jsx)(t.td,{children:"TuGraph Backup Binlog"}),(0,n.jsx)(t.td,{children:"Start a child process to process binlog backup requests"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"backup_copy_dir"}),(0,n.jsx)(t.td,{children:"TuGraph Backup"}),(0,n.jsx)(t.td,{children:"Start a child process to process full db backup requests"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"build_so"}),(0,n.jsx)(t.td,{children:"Component for compiling C++ dynamic libraries"}),(0,n.jsx)(t.td,{children:"Start a child process to handle GCC compilation logic"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"copy_snapshot"}),(0,n.jsx)(t.td,{children:"TuGraph Copy Snapshot"}),(0,n.jsx)(t.td,{children:"Handle backup snapshot requests in the current process"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"copydir"}),(0,n.jsx)(t.td,{children:"Folder copy"}),(0,n.jsx)(t.td,{children:"Handle folder copy requests in the current process"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"exec"}),(0,n.jsx)(t.td,{children:"Execute C++/Java executable files"}),(0,n.jsx)(t.td,{children:"Start a child process to launch the C++ executable file"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"algo"}),(0,n.jsx)(t.td,{children:"Execute algorithm"}),(0,n.jsx)(t.td,{children:"Start a child process to run the algorithm"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"bash"}),(0,n.jsx)(t.td,{children:"Execute bash commands"}),(0,n.jsx)(t.td,{children:"Start a child process to execute bash commands"})]}),(0,n.jsxs)(t.tr,{children:[(0,n.jsx)(t.td,{children:"rest"}),(0,n.jsx)(t.td,{children:"TuGraph Python Rest Client"}),(0,n.jsx)(t.td,{children:"Open TuGraph Python Rest Client in the current process and send requests"})]})]})]}),"\n",(0,n.jsx)(t.h3,{id:"22component-usage",children:"2.2.Component Usage"}),"\n",(0,n.jsx)(t.h4,{id:"221server",children:"2.2.1.server"}),"\n",(0,n.jsx)(t.h5,{id:"2211startup-parameters",children:"2.2.1.1.Startup Parameters"}),"\n",(0,n.jsx)(t.p,{children:"Use a Python dictionary to pass in the parameters:"}),"\n",(0,n.jsxs)(t.ul,{children:["\n",(0,n.jsx)(t.li,{children:"cmd is the startup command"}),"\n",(0,n.jsx)(t.li,{children:"cleanup_dir is the directory that needs to be cleaned up after execution, which can be multiple, passed in as a Python list"}),"\n"]}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --license _FMA_IGNORE_LICENSE_CHECK_SALTED_ --port 7072 --rpc_port 9092",\n             "cleanup_dir":["./testdb"]}\n'})}),"\n",(0,n.jsx)(t.h5,{id:"2212startup-command",children:"2.2.1.2.Startup Command"}),"\n",(0,n.jsx)(t.p,{children:"Import the tool through the fixtures component and control different processing logic through startup parameters. The server will be started before the function starts executing, and the server will be stopped and the directories specified in cleanup_dir will be cleaned up after the function finishes executing."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'@pytest.mark.parametrize("server", [SERVEROPT], indirect=True)\ndef test_server(self, server):\n    pass\n'})}),"\n",(0,n.jsx)(t.h4,{id:"222client",children:"2.2.2.client"}),"\n",(0,n.jsx)(t.h5,{id:"2221startup-parameters",children:"2.2.2.1.Startup Parameters"}),"\n",(0,n.jsx)(t.p,{children:"Use a Python dictionary to pass in the parameters:"}),"\n",(0,n.jsxs)(t.ul,{children:["\n",(0,n.jsx)(t.li,{children:"host is the IP and port of the TuGraph Server"}),"\n",(0,n.jsx)(t.li,{children:"user is the username of the TuGraph Server"}),"\n",(0,n.jsx)(t.li,{children:"password is the password corresponding to the user in the TuGraph Server"}),"\n"]}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'CLIENTOPT = {"host":"127.0.0.1:9092", "user":"admin", "password":"73@TuGraph"}\n'})}),"\n",(0,n.jsx)(t.h5,{id:"2222startup-command",children:"2.2.2.2.Startup Command"}),"\n",(0,n.jsx)(t.p,{children:"Import the tool through the fixtures component and control different processing logic through startup parameters. The client will be started before the function starts executing, and the client will be stopped after the function finishes executing."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:"@pytest.mark.parametrize(\"server\", [SERVEROPT], indirect=True)\n@pytest.mark.parametrize(\"client\", [CLIENTOPT], indirect=True)\ndef test_client(self, server, client):\n    ret = client.callCypher(\"CALL db.createEdgeLabel('followed', '[]', 'address', string, false, 'date', int32, false)\", \"default\")\n    assert ret[0]\n    ret = client.callCypher(\"CALL db.createEdgeLabel('followed', '[]', 'address', string, false, 'date', int32, false)\", \"default\")\n    assert ret[0] == False\n"})}),"\n",(0,n.jsx)(t.h4,{id:"223importor",children:"2.2.3.importor"}),"\n",(0,n.jsx)(t.h5,{id:"2231startup-parameters",children:"2.2.3.1.Startup Parameters"}),"\n",(0,n.jsx)(t.p,{children:"Use a Python dictionary to pass in the parameters:"}),"\n",(0,n.jsxs)(t.ul,{children:["\n",(0,n.jsx)(t.li,{children:"cmd is the startup command"}),"\n",(0,n.jsx)(t.li,{children:"cleanup_dir is the directory that needs to be cleaned up after execution, which can be multiple, passed in as a Python list"}),"\n"]}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'IMPORTOPT = {"cmd":"./lgraph_import --config_file ./data/yago/yago.conf --dir ./testdb --user admin --password 73@TuGraph --graph default --overwrite 1",\n             "cleanup_dir":["./testdb", "./.import_tmp"]}\n'})}),"\n",(0,n.jsx)(t.h5,{id:"2232startup-command",children:"2.2.3.2.Startup Command"}),"\n",(0,n.jsx)(t.p,{children:"Import the tool through the fixtures component and control the import of different data through startup parameters. The data will be imported to the specified directory before the function starts executing, and the directories specified in cleanup_dir will be cleaned up after the function finishes executing."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'@pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)\ndef test_importor(self, importor):\n    pass\n'})}),"\n",(0,n.jsx)(t.h4,{id:"224exportor",children:"2.2.4.exportor"}),"\n",(0,n.jsx)(t.h5,{id:"2241startup-parameters",children:"2.2.4.1.Startup Parameters"}),"\n",(0,n.jsx)(t.p,{children:"Use a Python dictionary to pass in the parameters:"}),"\n",(0,n.jsxs)(t.ul,{children:["\n",(0,n.jsx)(t.li,{children:"cmd is the startup command"}),"\n",(0,n.jsx)(t.li,{children:"cleanup_dir is the directory that needs to be cleaned up after execution, which can be multiple, passed in as a Python list"}),"\n"]}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'EXPORT_DEF_OPT = {"cmd":"./lgraph_export -d ./testdb -e ./export/default -g default -u admin -p 73@TuGraph",\n                  "cleanup_dir":["./export"]}\n'})}),"\n",(0,n.jsx)(t.h5,{id:"2242startup-command",children:"2.2.4.2.Startup Command"}),"\n",(0,n.jsx)(t.p,{children:"Import the tool through the fixtures component and control the export of different data through startup parameters. The data will be exported to the specified directory before the function starts executing, and the directories specified in cleanup_dir will be cleaned up after the function finishes executing."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'@pytest.mark.parametrize("exportor", [EXPORT_DEF_OPT], indirect=True)\ndef test_exportor(self, exportor):\n    pass\n'})}),"\n",(0,n.jsx)(t.h4,{id:"225backup_binlog",children:"2.2.5.backup_binlog"}),"\n",(0,n.jsx)(t.h5,{id:"2251startup-parameters",children:"2.2.5.1.Startup Parameters"}),"\n",(0,n.jsx)(t.p,{children:"\u91c7\u7528python\u5b57\u5178\u4f20\u5165\nUse a Python dictionary to pass in the parameters:"}),"\n",(0,n.jsxs)(t.ul,{children:["\n",(0,n.jsx)(t.li,{children:"cmd is the startup command"}),"\n",(0,n.jsx)(t.li,{children:"cleanup_dir is the directory that needs to be cleaned up after execution, which can be multiple, passed in as a Python list"}),"\n"]}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'BINLOGOPT = {"cmd" : "./lgraph_binlog -a restore --host 127.0.0.1 --port 9093 -u admin -p 73@TuGraph -f ./testdb/binlog/*",\n             "cleanup_dir":[]}\n'})}),"\n",(0,n.jsx)(t.h5,{id:"2252startup-command",children:"2.2.5.2.Startup Command"}),"\n",(0,n.jsx)(t.p,{children:"Import the tool through the fixtures component and control the backup of different binlogs through startup parameters. The binlogs will be copied to the specified directory before the function starts executing, and the directories specified in cleanup_dir will be cleaned up after the function finishes executing."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'@pytest.mark.parametrize("backup_binlog", [BINLOGOPT], indirect=True)\ndef test_backup_binlog(self, backup_binlog):\n    pass\n'})}),"\n",(0,n.jsx)(t.h4,{id:"226backup_copy_dir",children:"2.2.6.backup_copy_dir"}),"\n",(0,n.jsx)(t.h5,{id:"2261startup-parameters",children:"2.2.6.1.Startup Parameters"}),"\n",(0,n.jsx)(t.p,{children:"Use a Python dictionary to pass in the parameters:"}),"\n",(0,n.jsxs)(t.ul,{children:["\n",(0,n.jsx)(t.li,{children:"cmd is the startup command"}),"\n",(0,n.jsx)(t.li,{children:"cleanup_dir is the directory that needs to be cleaned up after execution, which can be multiple, passed in as a Python list"}),"\n"]}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'BACKUPOPT = {"cmd" : "./lgraph_backup --src ./testdb -dst ./testdb1",\n             "cleanup_dir":[]}\n'})}),"\n",(0,n.jsx)(t.h5,{id:"2262startup-command",children:"2.2.6.2.Startup Command"}),"\n",(0,n.jsx)(t.p,{children:"Import the tool through the fixtures component and control the backup of different databases through startup parameters. The database will be copied to the specified directory before the function starts executing, and the directories specified in cleanup_dir will be cleaned up after the function finishes executing."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'@pytest.mark.parametrize("backup_copy_dir", [BACKUPOPT], indirect=True)\ndef test_backup_copy_dir(self, backup_copy_dir):\n\tpass\n'})}),"\n",(0,n.jsx)(t.h4,{id:"227build_so",children:"2.2.7.build_so"}),"\n",(0,n.jsx)(t.h5,{id:"2271startup-parameters",children:"2.2.7.1.Startup Parameters"}),"\n",(0,n.jsx)(t.p,{children:"Use a Python dictionary to pass in the parameters:"}),"\n",(0,n.jsxs)(t.ul,{children:["\n",(0,n.jsx)(t.li,{children:"cmd is the startup command, passed in as a Python list, and multiple so can be compiled at once"}),"\n",(0,n.jsx)(t.li,{children:"so_name is the dynamic library that needs to be cleaned up after execution, which can be multiple, passed in as a Python list"}),"\n"]}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'BUILDOPT = {"cmd":["g++ -fno-gnu-unique -fPIC -g --std=c++17 -I ../../include -I ../../deps/install/include -rdynamic -O3 -fopenmp -DNDEBUG -o ./scan_graph.so ../../test/test_procedures/scan_graph.cpp ./liblgraph.so -shared",\n                       "g++ -fno-gnu-unique -fPIC -g --std=c++17 -I ../../include -I ../../deps/install/include -rdynamic -O3 -fopenmp -DNDEBUG -o ./sortstr.so ../../test/test_procedures/sortstr.cpp ./liblgraph.so -shared"],\n                "so_name":["./scan_graph.so", "./sortstr.so"]}\n'})}),"\n",(0,n.jsx)(t.h5,{id:"2272startup-command",children:"2.2.7.2.Startup Command"}),"\n",(0,n.jsx)(t.p,{children:"Import the tool through the fixtures component and control the compilation of different dynamic libraries through startup parameters. The dynamic libraries will be generated to the specified directory before the function starts executing, and the dynamic libraries specified in the so_name list will be cleaned up after the function finishes executing."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'@pytest.mark.parametrize("build_so", [BUILDOPT], indirect=True)\ndef test_build_so(self, build_so):\n    pass\n'})}),"\n",(0,n.jsx)(t.h4,{id:"228copy_snapshot",children:"2.2.8.copy_snapshot"}),"\n",(0,n.jsx)(t.h5,{id:"2281startup-parameters",children:"2.2.8.1.Startup Parameters"}),"\n",(0,n.jsx)(t.p,{children:"Use a Python dictionary to pass in the parameters:"}),"\n",(0,n.jsxs)(t.ul,{children:["\n",(0,n.jsx)(t.li,{children:"src is the original database"}),"\n",(0,n.jsx)(t.li,{children:"dst is the snapshot copied after the backup"}),"\n"]}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'COPYSNAPOPT = {"src" : "./testdb", "dst" : "./testdb1"}\n'})}),"\n",(0,n.jsx)(t.h5,{id:"2282startup-command",children:"2.2.8.2.Startup Command"}),"\n",(0,n.jsx)(t.p,{children:"Import the tool through the fixtures component and control the copying of different snapshots through startup parameters. The snapshot in src will be copied to the directory specified in dst before the function starts executing."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'@pytest.mark.parametrize("copy_snapshot", [COPYSNAPOPT], indirect=True)\ndef test_copy_snapshot(self, copy_snapshot):\n    pass\n'})}),"\n",(0,n.jsx)(t.h4,{id:"229copy_dir",children:"2.2.9.copy_dir"}),"\n",(0,n.jsx)(t.h5,{id:"2291startup-parameters",children:"2.2.9.1.Startup Parameters"}),"\n",(0,n.jsx)(t.p,{children:"Use a Python dictionary to pass in the parameters:"}),"\n",(0,n.jsxs)(t.ul,{children:["\n",(0,n.jsx)(t.li,{children:"src is the original directory"}),"\n",(0,n.jsx)(t.li,{children:"dst is the directory copied after the backup"}),"\n"]}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'COPYSNAPOPT = {"src" : "./testdb", "dst" : "./testdb1"}\n'})}),"\n",(0,n.jsx)(t.h5,{id:"2292startup-command",children:"2.2.9.2.Startup Command"}),"\n",(0,n.jsx)(t.p,{children:"Import the tool through the fixtures component and control the copying of different directories through startup parameters. The directory in src will be copied to the directory specified in dst before the function starts executing."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'@pytest.mark.parametrize("copy_dir", [COPYDIR], indirect=True)\ndef test_copy_dir(self, copy_dir):\n    pass\n'})}),"\n",(0,n.jsx)(t.h4,{id:"2210exec",children:"2.2.10.exec"}),"\n",(0,n.jsx)(t.h5,{id:"22101startup-parameters",children:"2.2.10.1.Startup Parameters"}),"\n",(0,n.jsx)(t.p,{children:"Use a Python dictionary to pass in the parameters:"}),"\n",(0,n.jsxs)(t.ul,{children:["\n",(0,n.jsx)(t.li,{children:"cmd is the startup command"}),"\n"]}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'EXECOPT = {\n        "cmd" : "test_rpc_client/cpp/CppClientTest/build/clienttest"\n    }\n'})}),"\n",(0,n.jsx)(t.h5,{id:"22102startup-command",children:"2.2.10.2.Startup Command"}),"\n",(0,n.jsx)(t.p,{children:"Import the tool through the fixtures component and control the execution of different logic through startup parameters. A child process will be started to execute the command passed in through the cmd parameter before the function starts executing."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'@pytest.mark.parametrize("exec", [EXECOPT], indirect=True)\ndef test_exec(self, exec):\n        pass\n'})}),"\n",(0,n.jsx)(t.h4,{id:"2211algo",children:"2.2.11.algo"}),"\n",(0,n.jsx)(t.h5,{id:"22111startup-parameters",children:"2.2.11.1.Startup Parameters"}),"\n",(0,n.jsx)(t.p,{children:"Use a Python dictionary to pass in the parameters:"}),"\n",(0,n.jsxs)(t.ul,{children:["\n",(0,n.jsx)(t.li,{children:"cmd is the startup command"}),"\n",(0,n.jsx)(t.li,{children:"result is the expected execution result of the algorithm. After execution is completed, the actual result will be compared with the expected result. If they are different, the test will fail."}),"\n"]}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'BFSEMBEDOPT = {\n        "cmd" : "algo/bfs_embed ./testdb",\n        "result" : ["found_vertices = 3829"]\n    }\n'})}),"\n",(0,n.jsx)(t.h5,{id:"22112startup-command",children:"2.2.11.2.Startup Command"}),"\n",(0,n.jsx)(t.p,{children:"Import the tool through the fixtures component and control the execution of different algorithm logic through startup parameters. A child process will be started to execute the algorithm passed in through the cmd parameter before the function starts executing. The function body will wait for the algorithm to complete and compare the result with the expected result."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'@pytest.mark.parametrize("algo", [BFSEMBEDOPT], indirect=True)\ndef test_exec_bfs_embed(self, algo):\n    pass\n'})}),"\n",(0,n.jsx)(t.h4,{id:"2212bash",children:"2.2.12.bash"}),"\n",(0,n.jsx)(t.h5,{id:"22121startup-parameters",children:"2.2.12.1.Startup Parameters"}),"\n",(0,n.jsx)(t.p,{children:"Use a Python dictionary to pass in the parameters:"}),"\n",(0,n.jsxs)(t.ul,{children:["\n",(0,n.jsx)(t.li,{children:"cmd is the startup command"}),"\n"]}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'BASHOPT = {\n        "cmd" : "sh ./test_rpc_client/cpp/CppClientTest/compile.sh"\n    }\n'})}),"\n",(0,n.jsx)(t.h5,{id:"22122startup-command",children:"2.2.12.2.Startup Command"}),"\n",(0,n.jsx)(t.p,{children:"Import the tool through the fixtures component and control the execution of different bash commands through startup parameters. A child process will be started to execute the bash command passed in through the cmd parameter before the function starts executing. The function body will wait for the command to complete."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'@pytest.mark.parametrize("bash", [BASHOPT], indirect=True)\ndef test_bash(self, bash):\n    pass\n'})}),"\n",(0,n.jsx)(t.h4,{id:"2213rest",children:"2.2.13.rest"}),"\n",(0,n.jsx)(t.h5,{id:"22131startup-parameters",children:"2.2.13.1.Startup Parameters"}),"\n",(0,n.jsx)(t.p,{children:"Use a Python dictionary to pass in the parameters:"}),"\n",(0,n.jsxs)(t.ul,{children:["\n",(0,n.jsx)(t.li,{children:"port is the port of the TuGraph Server"}),"\n",(0,n.jsx)(t.li,{children:"user is the username of the TuGraph Server"}),"\n",(0,n.jsx)(t.li,{children:"password is the password corresponding to user in TuGraph Server"}),"\n"]}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'RESTTOPT = {"port":"7073", "user":"admin", "password":"73@TuGraph"}\n'})}),"\n",(0,n.jsx)(t.h5,{id:"22132startup-command",children:"2.2.13.2.Startup Command"}),"\n",(0,n.jsx)(t.p,{children:"Import the tool through the fixtures component and link to different TuGraph Rest Servers through startup parameters. The client will be started before the function starts executing and stopped after the function finishes executing."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'@pytest.mark.parametrize("rest", [RESTTOPT], indirect=True)\ndef test_get_info(self, server, rest):\n\tpass\n'})}),"\n",(0,n.jsx)(t.h3,{id:"23test-cases",children:"2.3.Test Cases"}),"\n",(0,n.jsx)(t.h4,{id:"231rest",children:"2.3.1.rest"}),"\n",(0,n.jsx)(t.p,{children:"In the sample code, before the test_get_info function is executed, the server is started, and a rest client is started after the server is started. After entering the test_get_info function, some information about the server is obtained, and assert is used to determine whether cpu information is obtained."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --license _FMA_IGNORE_LICENSE_CHECK_SALTED_ --port 7073 --rpc_port 9093",\n               "cleanup_dir":["./testdb"]}\nRESTTOPT = {"port":"7073", "user":"admin", "password":"73@TuGraph"}\n@pytest.mark.parametrize("server", [SERVEROPT], indirect=True)\n@pytest.mark.parametrize("rest", [RESTTOPT], indirect=True)\ndef test_get_info(self, server, rest):\n    res = rest.get_server_info()\n    log.info("res : %s", res)\n    assert(\'cpu\' in res)\n'})}),"\n",(0,n.jsx)(t.h4,{id:"232client",children:"2.3.2.client"}),"\n",(0,n.jsx)(t.p,{children:"In the sample code, before the test_flushdb function is executed, the offline data import logic is executed and the server is started. After creating a connection through the client, the function enters the test_flushdb function. The number of points is queried to determine whether the import is successful. After the import is successful, the flushDB operation is executed. assert is used again to determine whether the db can be emptied normally."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --license _FMA_IGNORE_LICENSE_CHECK_SALTED_ --port 7072 --rpc_port 9092",\n             "cleanup_dir":["./testdb"]}\n\nCLIENTOPT = {"host":"127.0.0.1:9092", "user":"admin", "password":"73@TuGraph"}\n\nIMPORTOPT = {"cmd":"./lgraph_import --config_file ./data/yago/yago.conf --dir ./testdb --user admin --password 73@TuGraph --graph default --overwrite 1",\n             "cleanup_dir":["./testdb", "./.import_tmp"]}\n\n@pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)\n@pytest.mark.parametrize("server", [SERVEROPT], indirect=True)\n@pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)\ndef test_flushdb(self, importor, server, client):\n    ret = client.callCypher("MATCH (n) RETURN n LIMIT 100", "default")\n    assert ret[0]\n    res = json.loads(ret[1])\n    assert len(res) == 21\n    ret = client.callCypher("CALL db.flushDB()", "default")\n    assert ret[0]\n    res = json.loads(ret[1])\n    assert res == None\n'})}),"\n",(0,n.jsx)(t.h4,{id:"233exportorimportor",children:"2.3.3.exportor/importor"}),"\n",(0,n.jsx)(t.p,{children:"In the sample code, before the test_export_default function is executed, the offline data import logic is executed. After the import is successful, the data of the current db is exported. Then, the offline import logic is used again to import the exported data into a new directory. The newly imported data is used to start the db and create a connection. In the body of the test_export_default function, it is determined whether the data after export and import is consistent with the original data."}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-python",children:'SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb1 --license _FMA_IGNORE_LICENSE_CHECK_SALTED_ --port 7073 --rpc_port 9093",\n             "cleanup_dir":["./testdb1"]}\n\nCLIENTOPT = {"host":"127.0.0.1:9093", "user":"admin", "password":"73@TuGraph"}\n\nIMPORT_YAGO_OPT = {"cmd":"./lgraph_import --config_file ./data/yago/yago.conf --dir ./testdb --user admin --password 73@TuGraph --graph default --overwrite 1",\n             "cleanup_dir":["./.import_tmp", "./testdb"]}\n\nIMPORT_DEF_OPT = {"cmd":"./lgraph_import -c ./export/default/import.config -d ./testdb1",\n             "cleanup_dir":["./.import_tmp", "./testdb1"]}\n\nEXPORT_DEF_OPT = {"cmd":"./lgraph_export -d ./testdb -e ./export/default -g default -u admin -p 73@TuGraph",\n                  "cleanup_dir":["./export"]}\n\n@pytest.mark.parametrize("importor", [IMPORT_YAGO_OPT], indirect=True)\n@pytest.mark.parametrize("exportor", [EXPORT_DEF_OPT], indirect=True)\n@pytest.mark.parametrize("importor_1", [IMPORT_DEF_OPT], indirect=True)\n@pytest.mark.parametrize("server", [SERVEROPT], indirect=True)\n@pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)\ndef test_export_default(self, importor, exportor, importor_1, server, client):\n    ret = client.callCypher("MATCH (n) RETURN n LIMIT 100", "default")\n    assert ret[0]\n    res = json.loads(ret[1])\n    log.info("res : %s", res)\n    assert len(res) == 21\n'})}),"\n",(0,n.jsx)(t.h4,{id:"234other-tests",children:"2.3.4.Other Tests"}),"\n",(0,n.jsxs)(t.p,{children:["For more test cases, please refer to the integration test code ",(0,n.jsx)(t.a,{href:"https://github.com/TuGraph-db/tugraph-db/tree/master/test/integration",children:"https://github.com/TuGraph-db/tugraph-db/tree/master/test/integration"})]})]})}function h(e={}){const{wrapper:t}={...(0,s.R)(),...e.components};return t?(0,n.jsx)(t,{...e,children:(0,n.jsx)(l,{...e})}):l(e)}}}]);