"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[32156],{28453:(e,n,s)=>{s.d(n,{R:()=>a,x:()=>l});var i=s(96540);const t={},r=i.createContext(t);function a(e){const n=i.useContext(r);return i.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function l(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(t):e.components||t:a(e.components),i.createElement(r.Provider,{value:n},e.children)}},41001:(e,n,s)=>{s.r(n),s.d(n,{assets:()=>c,contentTitle:()=>a,default:()=>d,frontMatter:()=>r,metadata:()=>l,toc:()=>o});var i=s(74848),t=s(28453);const r={},a="TuGraph Browser(old version)",l={id:"user-guide/tugraph-browser-legacy",title:"TuGraph Browser(old version)",description:"This document focuses on the use of TuGraph Browser",source:"@site/versions/version-4.3.0/en-US/source/4.user-guide/2.tugraph-browser-legacy.md",sourceDirName:"4.user-guide",slug:"/user-guide/tugraph-browser-legacy",permalink:"/tugraph-db/en/4.3.0/user-guide/tugraph-browser-legacy",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:2,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"TuGraph Browser",permalink:"/tugraph-db/en/4.3.0/user-guide/tugraph-browser"},next:{title:"Environment",permalink:"/tugraph-db/en/4.3.0/installation&running/environment"}},c={},o=[{value:"Definition",id:"definition",level:2},{value:"Functionality",id:"functionality",level:2},{value:"How to use it",id:"how-to-use-it",level:2},{value:"1. Connect to the database",id:"1-connect-to-the-database",level:3},{value:"2. Login",id:"2-login",level:3},{value:"3. Workbench",id:"3-workbench",level:3},{value:"3.1 Quick Start",id:"31-quick-start",level:4},{value:"3.2 Create subgraphs and examples",id:"32-create-subgraphs-and-examples",level:4},{value:"3.2.1 Create a subgraph",id:"321-create-a-subgraph",level:5},{value:"3.3 query",id:"33-query",level:4},{value:"3.3.1 User Interface Layout",id:"331-user-interface-layout",level:5},{value:"3.3.2 Result set display area function details",id:"332-result-set-display-area-function-details",level:5},{value:"3.3.3 Schema",id:"333-schema",level:5},{value:"3.3.4 Data import",id:"334-data-import",level:5},{value:"3.3.5 plug-in (Stored Procedure)",id:"335-plug-in-stored-procedure",level:5},{value:"3.3.6 Help",id:"336-help",level:5},{value:"3.4 Console",id:"34-console",level:4},{value:"3.4.1 Basic database information",id:"341-basic-database-information",level:5},{value:"3.4.2 Permission management",id:"342-permission-management",level:5},{value:"3.4.3 Live status",id:"343-live-status",level:5},{value:"3.4.4 Task Management",id:"344-task-management",level:5},{value:"3.4.5 Audit logs",id:"345-audit-logs",level:5}];function h(e){const n={blockquote:"blockquote",h1:"h1",h2:"h2",h3:"h3",h4:"h4",h5:"h5",header:"header",img:"img",li:"li",p:"p",ul:"ul",...(0,t.R)(),...e.components};return(0,i.jsxs)(i.Fragment,{children:[(0,i.jsx)(n.header,{children:(0,i.jsx)(n.h1,{id:"tugraph-browserold-version",children:"TuGraph Browser(old version)"})}),"\n",(0,i.jsxs)(n.blockquote,{children:["\n",(0,i.jsx)(n.p,{children:"This document focuses on the use of TuGraph Browser"}),"\n"]}),"\n",(0,i.jsx)(n.h2,{id:"definition",children:"Definition"}),"\n",(0,i.jsx)(n.p,{children:"TuGraph Browser is a visual development tool provided by TuGraph."}),"\n",(0,i.jsx)(n.h2,{id:"functionality",children:"Functionality"}),"\n",(0,i.jsx)(n.p,{children:"TuGraph Browser provides graph database developers with functions such as visual graph data development, graph data management and maintenance."}),"\n",(0,i.jsx)(n.h2,{id:"how-to-use-it",children:"How to use it"}),"\n",(0,i.jsx)(n.h3,{id:"1-connect-to-the-database",children:"1. Connect to the database"}),"\n",(0,i.jsx)(n.p,{children:"When the user completes the installation of the graph database, you can access it through the Browser, TuGraph Browser tool. The user only needs to type in the browser address bar: IP of the server where TuGraph is located :Port. The default port is 7090."}),"\n",(0,i.jsx)(n.h3,{id:"2-login",children:"2. Login"}),"\n",(0,i.jsx)(n.p,{children:(0,i.jsx)(n.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/2.Operating/1.tugraph-browser-lpgin.png",alt:"alt Login"})}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"When the page is opened successfully, the first thing you see is the login page, and the user needs to fill in the account number and password to log in."}),"\n",(0,i.jsx)(n.li,{children:"Default account: admin"}),"\n",(0,i.jsx)(n.li,{children:"Default password: 73@TuGraph"}),"\n",(0,i.jsx)(n.li,{children:"It is recommended that users change the initialized password in time after logging in"}),"\n"]}),"\n",(0,i.jsx)(n.h3,{id:"3-workbench",children:"3. Workbench"}),"\n",(0,i.jsx)(n.h4,{id:"31-quick-start",children:"3.1 Quick Start"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsx)(n.p,{children:"When you log in for the first time, the system will create an empty graph by default"}),"\n",(0,i.jsx)(n.p,{children:(0,i.jsx)(n.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/2.Operating/2.tugraph-browser-quickstart-01.png",alt:"alt quick start"})}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsx)(n.p,{children:"The user clicks on the help option and selects Get Started quickly"}),"\n",(0,i.jsx)(n.p,{children:(0,i.jsx)(n.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/2.Operating/3.tugraph-browser-quickstart-02.png",alt:"alt help"})}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsx)(n.p,{children:'Then click "One-click Create Model" -- >" One-click Create Data "to complete the construction of the built-in Movie data graph'}),"\n"]}),"\n"]}),"\n",(0,i.jsx)(n.h4,{id:"32-create-subgraphs-and-examples",children:"3.2 Create subgraphs and examples"}),"\n",(0,i.jsx)(n.h5,{id:"321-create-a-subgraph",children:"3.2.1 Create a subgraph"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:["Click on New subgraph\n",(0,i.jsx)(n.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/2.Operating/4.tugraph-browser-create-subgraph-01.png",alt:"alt Create subgraph"})]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:["Fill in the form information\n",(0,i.jsx)(n.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/2.Operating/5.tugraph-browser-create-subgraph-02.png",alt:"alt Fill out the form"})]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsx)(n.p,{children:"Subgraph name"}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsx)(n.p,{children:"Subgraph description"}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsx)(n.p,{children:"Configuration information"}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsx)(n.p,{children:"Click OK to prompt that creation succeeded"}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:["Toggle the subgraph\n",(0,i.jsx)(n.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/2.Operating/6.tugraph-browser-use-graph-01.png",alt:"alt Toggle subgraph"})]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:["Click New Example\n",(0,i.jsx)(n.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/3.3.0-image/create-scene-01.png",alt:"alt Create subgraph"})]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:["Select the example and click Create\n",(0,i.jsx)(n.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/3.3.0-image/select-scene.png",alt:"alt Create subgraph"})]}),"\n"]}),"\n"]}),"\n",(0,i.jsx)(n.h4,{id:"33-query",children:"3.3 query"}),"\n",(0,i.jsx)(n.p,{children:(0,i.jsx)(n.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/2.Operating/7.tugraph-browser-query-01.png",alt:"alt Query"})}),"\n",(0,i.jsx)(n.h5,{id:"331-user-interface-layout",children:"3.3.1 User Interface Layout"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"Cypher input box"}),"\n",(0,i.jsx)(n.li,{children:"Result set display area"}),"\n"]}),"\n",(0,i.jsx)(n.h5,{id:"332-result-set-display-area-function-details",children:"3.3.2 Result set display area function details"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:["Result set tag display and functions\n",(0,i.jsx)(n.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/3.3.0-image/tugraph-browser-result.png",alt:"alt Result set tag"})]}),"\n",(0,i.jsx)(n.li,{children:"All types of statistics for the result set are shown here"}),"\n",(0,i.jsx)(n.li,{children:'Click on the different "label" to make the following changes\n-Change the display color'}),"\n",(0,i.jsx)(n.li,{children:"Modify the node size or edge thickness"}),"\n",(0,i.jsx)(n.li,{children:"Modify default display properties or system properties"}),"\n",(0,i.jsx)(n.li,{children:"Layout modification"}),"\n",(0,i.jsx)(n.li,{children:"Force guide layout"}),"\n",(0,i.jsx)(n.li,{children:"Grid layout"}),"\n",(0,i.jsx)(n.li,{children:"Tree layout"}),"\n",(0,i.jsx)(n.li,{children:"Environment layout"}),"\n",(0,i.jsx)(n.li,{children:"edge polymerization"}),"\n",(0,i.jsx)(n.li,{children:"Edges of the same type and direction can be merged"}),"\n",(0,i.jsx)(n.li,{children:"Create nodes"}),"\n",(0,i.jsx)(n.li,{children:"Click the Create Node button"}),"\n",(0,i.jsx)(n.li,{children:"Select the node type"}),"\n",(0,i.jsx)(n.li,{children:"Add node content"}),"\n",(0,i.jsx)(n.li,{children:"Create relationships"}),"\n",(0,i.jsx)(n.li,{children:"Select start and end points in the canvas"}),"\n",(0,i.jsx)(n.li,{children:"Select the type that can be matched"}),"\n",(0,i.jsx)(n.li,{children:"Enter node information"}),"\n",(0,i.jsx)(n.li,{children:"Stop layout"}),"\n",(0,i.jsx)(n.li,{children:"When too much data is causing the browser page to stall, you can click this stop layout button to improve the smoothness of the experience"}),"\n",(0,i.jsx)(n.li,{children:"Mouse hover"}),"\n",(0,i.jsx)(n.li,{children:"Enable this feature to highlight the one-degree neighbors of the mouse-over node"}),"\n",(0,i.jsx)(n.li,{children:"Result set export"}),"\n",(0,i.jsx)(n.li,{children:"Result sets can be exported as png, json, csv three different file forms"}),"\n",(0,i.jsx)(n.li,{children:"the refresh"}),"\n",(0,i.jsx)(n.li,{children:"Clicking the refresh button will re-execute the initial cypher statement for the current page and refresh the result set\nMaximize -\n-Click Maximize button, the result set display area will be displayed in full screen"}),"\n",(0,i.jsxs)(n.li,{children:["Result set display mode switching\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"Support graph, table and text modes"}),"\n"]}),"\n"]}),"\n"]}),"\n",(0,i.jsx)(n.h5,{id:"333-schema",children:"3.3.3 Schema"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsxs)(n.p,{children:["model\n",(0,i.jsx)(n.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/3.3.0-image/create-schema.png",alt:"Modeling Alt"})]}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsx)(n.p,{children:"schema supports real-time addition, deletion, modification and query"}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsx)(n.p,{children:"schema supports import and export"}),"\n"]}),"\n"]}),"\n",(0,i.jsx)(n.h5,{id:"334-data-import",children:"3.3.4 Data import"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"Local data import"}),"\n"]}),"\n",(0,i.jsx)(n.p,{children:(0,i.jsx)(n.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/2.Operating/14.tugraph-browser-import-data-01.png",alt:"alt Query"})}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"Select the file encoding format"}),"\n",(0,i.jsx)(n.li,{children:"Select a local CSV file"}),"\n",(0,i.jsx)(n.li,{children:"Select the model for the corresponding node or edge"}),"\n",(0,i.jsx)(n.li,{children:"Do the data mapping"}),"\n",(0,i.jsx)(n.li,{children:"Complete the data import"}),"\n",(0,i.jsx)(n.li,{children:"The maximum supported size for a single file is 2GB."}),"\n"]}),"\n",(0,i.jsx)(n.h5,{id:"335-plug-in-stored-procedure",children:"3.3.5 plug-in (Stored Procedure)"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"Use of plug-ins"}),"\n"]}),"\n",(0,i.jsx)(n.p,{children:(0,i.jsx)(n.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/2.Operating/15.tugraph-browser-plugin.png",alt:"alt Query"})}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"Users can upload the plug-in program written locally, execute it in the visual page, and view the execution result"}),"\n",(0,i.jsx)(n.li,{children:"Users can perform, uninstall, execute, and download plug-ins in the visualization page"}),"\n"]}),"\n",(0,i.jsx)(n.h5,{id:"336-help",children:"3.3.6 Help"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:["where it records how TuGraph-browser is used\n",(0,i.jsx)(n.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/2.Operating/16.TuGraph-browser-help.png",alt:"alt Query"})]}),"\n"]}),"\n",(0,i.jsx)(n.h4,{id:"34-console",children:"3.4 Console"}),"\n",(0,i.jsx)(n.h5,{id:"341-basic-database-information",children:"3.4.1 Basic database information"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:["Displays basic database configuration information\n",(0,i.jsx)(n.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/2.Operating/17.tugraph-browser-config.png",alt:"alt Query"})]}),"\n"]}),"\n",(0,i.jsx)(n.h5,{id:"342-permission-management",children:"3.4.2 Permission management"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:["A function module used to create users and roles. Users can manage permissions here\n",(0,i.jsx)(n.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/2.Operating/18.tugraph-browser-auth.png",alt:"alt Query"})]}),"\n"]}),"\n",(0,i.jsx)(n.h5,{id:"343-live-status",children:"3.4.3 Live status"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:["This shows the database real-time status, including: CPU usage, memory usage, disk usage, number of data requests, disk IO\n",(0,i.jsx)(n.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/2.Operating/19.tugraph-browser-status.png",alt:"alt Query"})]}),"\n"]}),"\n",(0,i.jsx)(n.h5,{id:"344-task-management",children:"3.4.4 Task Management"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:["Here the user can see the task in execution and stop the task\n",(0,i.jsx)(n.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/2.Operating/20.tugraph-browser-task.png",alt:"alt Query"})]}),"\n"]}),"\n",(0,i.jsx)(n.h5,{id:"345-audit-logs",children:"3.4.5 Audit logs"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsxs)(n.li,{children:["Database related audit logs recorded here, used to troubleshoot problems encountered in use\n",(0,i.jsx)(n.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/2.Operating/21.tugraph-browser-log.png",alt:"alt Query"})]}),"\n"]})]})}function d(e={}){const{wrapper:n}={...(0,t.R)(),...e.components};return n?(0,i.jsx)(n,{...e,children:(0,i.jsx)(h,{...e})}):h(e)}}}]);