"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[83088],{95052:(e,o,n)=>{n.r(o),n.d(o,{assets:()=>d,contentTitle:()=>l,default:()=>u,frontMatter:()=>t,metadata:()=>s,toc:()=>a});var i=n(74848),r=n(28453);const t={},l="Log",s={id:"permission/log",title:"Log",description:"This document mainly introduces the logging function of TuGraph.",source:"@site/versions/version-4.5.1/en-US/source/10.permission/5.log.md",sourceDirName:"10.permission",slug:"/permission/log",permalink:"/tugraph-db/en-US/en/4.5.1/permission/log",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:5,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Monitoring",permalink:"/tugraph-db/en-US/en/4.5.1/permission/monitoring"},next:{title:"Unit Testing",permalink:"/tugraph-db/en-US/en/4.5.1/quality/unit-testing"}},d={},a=[{value:"1.Introduction",id:"1introduction",level:2},{value:"2.Server log",id:"2server-log",level:2},{value:"2.1.Server Log Configuration Items",id:"21server-log-configuration-items",level:3},{value:"2.2.Example of Server Log Output Macro Usage",id:"22example-of-server-log-output-macro-usage",level:3},{value:"2.3.Procedure log",id:"23procedure-log",level:3},{value:"2.3.1.Cpp procedure",id:"231cpp-procedure",level:4},{value:"2.3.1.python procedure",id:"231python-procedure",level:4},{value:"3.Audit log",id:"3audit-log",level:2}];function c(e){const o={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",h4:"h4",header:"header",p:"p",pre:"pre",...(0,r.R)(),...e.components};return(0,i.jsxs)(i.Fragment,{children:[(0,i.jsx)(o.header,{children:(0,i.jsx)(o.h1,{id:"log",children:"Log"})}),"\n",(0,i.jsxs)(o.blockquote,{children:["\n",(0,i.jsx)(o.p,{children:"This document mainly introduces the logging function of TuGraph."}),"\n"]}),"\n",(0,i.jsx)(o.h2,{id:"1introduction",children:"1.Introduction"}),"\n",(0,i.jsx)(o.p,{children:"TuGraph keeps two types of logs: server logs and audit logs. Server logs record human-readable server status information, while audit logs maintain encrypted information for every operation performed on the server."}),"\n",(0,i.jsx)(o.h2,{id:"2server-log",children:"2.Server log"}),"\n",(0,i.jsx)(o.h3,{id:"21server-log-configuration-items",children:"2.1.Server Log Configuration Items"}),"\n",(0,i.jsxs)(o.p,{children:["The output directory of server logs can be specified through the ",(0,i.jsx)(o.code,{children:"log_dir"})," configuration item. The level of log can be specified through the ",(0,i.jsx)(o.code,{children:"verbose"})," configuration item."]}),"\n",(0,i.jsxs)(o.p,{children:["The ",(0,i.jsx)(o.code,{children:"log_dir"})," configuration item is empty by default. If ",(0,i.jsx)(o.code,{children:"log_dir"})," configuration item is empty, then all logs will be write to the console(nothing will be written to the console if the ",(0,i.jsx)(o.code,{children:"log_dir"})," configuration item is empty under daemon mode); if ",(0,i.jsx)(o.code,{children:"log_dir"})," configuration item is configured specifically, log files will bew generated under that path. The maximum size of a single log file is 256MB."]}),"\n",(0,i.jsxs)(o.p,{children:["The ",(0,i.jsx)(o.code,{children:"verbose"})," configuration item controls the level of log, and is divided into three levels of ",(0,i.jsx)(o.code,{children:"0, 1, 2"}),". Ther verbosity of log record grows as the number grows. The default level is ",(0,i.jsx)(o.code,{children:"1"}),". When the level is set to ",(0,i.jsx)(o.code,{children:"2"}),", the server will print logs in ",(0,i.jsx)(o.code,{children:"DEBUG"})," level and above. When the level is set to ",(0,i.jsx)(o.code,{children:"1"}),", the server will print logs in ",(0,i.jsx)(o.code,{children:"INFO"})," level and above. When the level is set to ",(0,i.jsx)(o.code,{children:"0"}),", the server will print log in ",(0,i.jsx)(o.code,{children:"ERROR"})," level and above."]}),"\n",(0,i.jsx)(o.h3,{id:"22example-of-server-log-output-macro-usage",children:"2.2.Example of Server Log Output Macro Usage"}),"\n",(0,i.jsx)(o.p,{children:"If the developer wants to add logs to the code during development, they can refer to the following example:"}),"\n",(0,i.jsx)(o.pre,{children:(0,i.jsx)(o.code,{children:'#include "tools/lgraph_log.h" // add log dependency.\n\nvoid LogExample() {\n    // The log module has been initialized during the database startup, and developers can directly call the macro.\n    // The log level is divided into five levels: DEBUG, INFO, WARNING, ERROR, and FATAL.\n\n    LOG_DEBUG() << "This is a debug level log message.";\n    LOG_INFO() << "This is a info level log message.";\n    LOG_WARN() << "This is a warning level log message.";\n    LOG_ERROR() << "This is a error level log message.";\n    LOG_FATAL() << "This is a fatal level log message.";\n}\n'})}),"\n",(0,i.jsx)(o.p,{children:"You can also refer to the log macro usage in test/test_lgraph_log.cpp."}),"\n",(0,i.jsx)(o.h3,{id:"23procedure-log",children:"2.3.Procedure log"}),"\n",(0,i.jsxs)(o.p,{children:["Developers can use the log function to output debug information that they need to the log for looking up and assisting development during the development of procedures. Debug information will be output to the same log file as the server log (if ",(0,i.jsx)(o.code,{children:"log_dir"})," configuration item is not specified, it will also be output to the console)"]}),"\n",(0,i.jsx)(o.h4,{id:"231cpp-procedure",children:"2.3.1.Cpp procedure"}),"\n",(0,i.jsxs)(o.p,{children:["Please use the log macro provided in 2.2 to output debug information, and avoid using output methods such as cout or printf. For specific usage, please refer to the following sample code (see ",(0,i.jsx)(o.code,{children:"procedures/demo/log_demo.cpp"})," for details)"]}),"\n",(0,i.jsx)(o.pre,{children:(0,i.jsx)(o.code,{children:'#include <stdlib.h>\n#include "lgraph/lgraph.h"\n#include "tools/lgraph_log.h"  // add log dependency\nusing namespace lgraph_api;\n\nvoid LogExample() {\n    LOG_DEBUG() << "This is a debug level log message.";\n    LOG_INFO() << "This is a info level log message.";\n    LOG_WARN() << "This is a warning level log message.";\n    LOG_ERROR() << "This is a error level log message.";\n}\n\nextern "C" bool Process(GraphDB& db, const std::string& request, std::string& response) {\n    response = "TuGraph log demo";\n    LogExample();\n    return true;\n}\n'})}),"\n",(0,i.jsx)(o.p,{children:"After inserting the above sample code into the database as a procedure and running it, you can see the corresponding log entries in the log file."}),"\n",(0,i.jsx)(o.h4,{id:"231python-procedure",children:"2.3.1.python procedure"}),"\n",(0,i.jsx)(o.p,{children:"Please use Python's built-in print to output debug information. The debug information will be merged into a WARN-level log entry and output to the log file after the procedure is executed."}),"\n",(0,i.jsx)(o.h2,{id:"3audit-log",children:"3.Audit log"}),"\n",(0,i.jsx)(o.p,{children:"Audit logs record each request and response, as well as the user who sent the request and when the request received. Audit logging can only be turned on or off. The results can be queried using the TuGraph visualization tool and the REST API."}),"\n",(0,i.jsxs)(o.p,{children:["To enable the Audit Log, you need to set the ",(0,i.jsx)(o.code,{children:"enable_audit_log"})," parameter to ",(0,i.jsx)(o.code,{children:"true"})," in the configuration file. For the configuration file and parameter descriptions, see:",(0,i.jsx)(o.a,{href:"../../5.installation&running/7.tugraph-running.md",children:"Tugraph Running/Service configuration"})]})]})}function u(e={}){const{wrapper:o}={...(0,r.R)(),...e.components};return o?(0,i.jsx)(o,{...e,children:(0,i.jsx)(c,{...e})}):c(e)}},28453:(e,o,n)=>{n.d(o,{R:()=>l,x:()=>s});var i=n(96540);const r={},t=i.createContext(r);function l(e){const o=i.useContext(t);return i.useMemo((function(){return"function"==typeof e?e(o):{...o,...e}}),[o,e])}function s(e){let o;return o=e.disableParentContext?"function"==typeof e.components?e.components(r):e.components||r:l(e.components),i.createElement(t.Provider,{value:o},e.children)}}}]);