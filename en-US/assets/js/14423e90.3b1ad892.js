"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[86617],{28453:(e,n,r)=>{r.d(n,{R:()=>d,x:()=>l});var t=r(96540);const s={},i=t.createContext(s);function d(e){const n=t.useContext(i);return t.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function l(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(s):e.components||s:d(e.components),t.createElement(i.Provider,{value:n},e.children)}},53719:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>a,contentTitle:()=>d,default:()=>o,frontMatter:()=>i,metadata:()=>l,toc:()=>c});var t=r(74848),s=r(28453);const i={},d="Tugraph CLI",l={id:"utility-tools/tugraph-cli",title:"Tugraph CLI",description:"This document mainly introduces the CLI tool lgraph_cypher of TuGraph.",source:"@site/versions/version-4.3.2/en-US/source/6.utility-tools/6.tugraph-cli.md",sourceDirName:"6.utility-tools",slug:"/utility-tools/tugraph-cli",permalink:"/tugraph-db/en-US/en/4.3.2/utility-tools/tugraph-cli",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:6,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Cluster management",permalink:"/tugraph-db/en-US/en/4.3.2/utility-tools/ha-cluster-management"},next:{title:"TuGraph DataX",permalink:"/tugraph-db/en-US/en/4.3.2/utility-tools/tugraph-datax"}},a={},c=[{value:"1.Instructions",id:"1instructions",level:2},{value:"Single command mode",id:"single-command-mode",level:2},{value:"Command line Parameters:",id:"command-line-parameters",level:3},{value:"Examples:",id:"examples",level:3},{value:"Interactive mode",id:"interactive-mode",level:2},{value:"Enter lgraph_cypher interaction mode:",id:"enter-lgraph_cypher-interaction-mode",level:3},{value:"Command Description",id:"command-description",level:3},{value:"cypher query command:",id:"cypher-query-command",level:3},{value:"Auxiliary Features:",id:"auxiliary-features",level:3}];function h(e){const n={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",li:"li",p:"p",pre:"pre",strong:"strong",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",ul:"ul",...(0,s.R)(),...e.components};return(0,t.jsxs)(t.Fragment,{children:[(0,t.jsx)(n.header,{children:(0,t.jsx)(n.h1,{id:"tugraph-cli",children:"Tugraph CLI"})}),"\n",(0,t.jsxs)(n.blockquote,{children:["\n",(0,t.jsx)(n.p,{children:"This document mainly introduces the CLI tool lgraph_cypher of TuGraph."}),"\n"]}),"\n",(0,t.jsx)(n.h2,{id:"1instructions",children:"1.Instructions"}),"\n",(0,t.jsx)(n.p,{children:"The TuGraph release comes with a query client named 'lgraph_cypher' that can be used to submit OpenCypher requests to the TuGraph server. The 'lgraph_cypher' client has two execution modes: single command mode and interactive mode."}),"\n",(0,t.jsx)(n.h2,{id:"single-command-mode",children:"Single command mode"}),"\n",(0,t.jsx)(n.p,{children:"In single-command mode, 'lgraph_cypher' can be used to submit a single Cypher query and print the result directly to the terminal. The printed result can also be easily redirected to a specified file. This is handy when users need to get a lot of results from the server and save them in files."}),"\n",(0,t.jsx)(n.p,{children:"In this mode, the 'lgraph_cypher' tool has the following options:"}),"\n",(0,t.jsx)(n.h3,{id:"command-line-parameters",children:"Command line Parameters:"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,t.jsxs)(n.table,{children:[(0,t.jsx)(n.thead,{children:(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.th,{children:"Parameter"}),(0,t.jsx)(n.th,{children:"Type"}),(0,t.jsx)(n.th,{children:"Instructions"})]})}),(0,t.jsxs)(n.tbody,{children:[(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"--help"}),(0,t.jsx)(n.td,{children:"\\"}),(0,t.jsx)(n.td,{children:"List all parameters and descriptions."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"-example"}),(0,t.jsx)(n.td,{children:"\\"}),(0,t.jsx)(n.td,{children:"List the command instances."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"-c"}),(0,t.jsx)(n.td,{children:"string"}),(0,t.jsx)(n.td,{children:"A database configuration file used to obtain ip and port information."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"-h"}),(0,t.jsx)(n.td,{children:"string"}),(0,t.jsx)(n.td,{children:"Database server IP address. Omit this parameter if you have a configuration file. The default value is' 127.0.0.1 '"})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"-p"}),(0,t.jsx)(n.td,{children:"string"}),(0,t.jsx)(n.td,{children:"Database server port. Omit this parameter if you have a configuration file. The default value is 7071"})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"-u"}),(0,t.jsx)(n.td,{children:"string"}),(0,t.jsx)(n.td,{children:"User name for logging in to the database."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"-P"}),(0,t.jsx)(n.td,{children:"string"}),(0,t.jsx)(n.td,{children:"Password for logging in to the database."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"-f"}),(0,t.jsx)(n.td,{children:"string"}),(0,t.jsx)(n.td,{children:"Contains the path to a single Cypher query single text file."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"-s"}),(0,t.jsx)(n.td,{children:"string"}),(0,t.jsxs)(n.td,{children:["Single-line cypher query command. Start and end with ",(0,t.jsx)(n.code,{children:'"'}),"."]})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"-t"}),(0,t.jsx)(n.td,{children:"int"}),(0,t.jsx)(n.td,{children:"Specifies the server timeout threshold for cypher queries. The default value is 150 seconds."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"-format"}),(0,t.jsx)(n.td,{children:"string"}),(0,t.jsx)(n.td,{children:"Query result display mode. Supports two formats: 'plain' and 'table'. The 'plain' format prints the query results in a single column. The 'table' format displays the query results in a tabular format. The default value is' table '"})]})]})]}),"\n",(0,t.jsx)(n.h3,{id:"examples",children:"Examples:"}),"\n",(0,t.jsx)(n.p,{children:(0,t.jsx)(n.strong,{children:"cypher command file query:"})}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{className:"language-powershell",children:"\n$ ./lgraph_cypher.py -c /home/usr/lgraph_standalone.json -u user -P password -f /home/usr/cypher.json\n\n"})}),"\n",(0,t.jsx)(n.p,{children:(0,t.jsx)(n.strong,{children:"cypher command single-sentence query:"})}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{className:"language-powershell",children:'\n$ ./lgraph_cypher.py -c /home/usr/lgraph_standalone.json -u user -P password -s "MATCH (n) RETURN n"\n\n'})}),"\n",(0,t.jsx)(n.h2,{id:"interactive-mode",children:"Interactive mode"}),"\n",(0,t.jsx)(n.p,{children:"'lgraph_cypher' can also be run in interactive mode. In interactive mode, the client stays connected to the server and interacts with the user in read-evaluate-print-loop."}),"\n",(0,t.jsx)(n.h3,{id:"enter-lgraph_cypher-interaction-mode",children:"Enter lgraph_cypher interaction mode:"}),"\n",(0,t.jsx)(n.p,{children:"If no '-f' or '-s' command line option added, 'lgraph_cypher' will enter interactive mode when running. how to use it:"}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{children:"\n$ ./lgraph_cypher.py -c /home/usr/lgraph_standalone.json -u admin -P 73@TuGraph\n\n"})}),"\n",(0,t.jsx)(n.p,{children:"If the login is successful, the corresponding login success message will be displayed:"}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{children:'**********************************************************************\n*                  TuGraph Graph Database X.Y.Z                      *\n*                                                                    *\n*        Copyright(C) 2018 Ant Group. All rights reserved.           *\n*                                                                    *\n**********************************************************************\nlogin success\n----------------------------------\nHost: 127.0.0.1\nPort: 7071\nUsername: admin\n----------------------------------\ntype ":help" to see all commands.\n>\n'})}),"\n",(0,t.jsx)(n.p,{children:"Now we also provide an interactive shell for users to enter Cypher queries or use the ':help' command to check for available commands."}),"\n",(0,t.jsx)(n.h3,{id:"command-description",children:"Command Description"}),"\n",(0,t.jsx)(n.p,{children:"In addition to the Cypher query, the shell of 'lgraph_cypher' accepts the following commands:"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,t.jsxs)(n.table,{children:[(0,t.jsx)(n.thead,{children:(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.th,{children:"Command"}),(0,t.jsx)(n.th,{children:"Parameters"}),(0,t.jsx)(n.th,{children:"instructions"})]})}),(0,t.jsxs)(n.tbody,{children:[(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:":help"}),(0,t.jsx)(n.td,{children:"\\"}),(0,t.jsx)(n.td,{children:"Displays the server information and the corresponding description of all commands."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:":db_info"}),(0,t.jsx)(n.td,{children:"\\"}),(0,t.jsx)(n.td,{children:"Query the current server status. /db/info for the corresponding REST API."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:":clear"}),(0,t.jsx)(n.td,{children:"\\"}),(0,t.jsx)(n.td,{children:"Clear the screen."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:":use"}),(0,t.jsx)(n.td,{children:"{Graph Name}"}),(0,t.jsxs)(n.td,{children:["The graph specified with this name defaults to ",(0,t.jsx)(n.code,{children:"default"})]})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:":source"}),(0,t.jsx)(n.td,{children:(0,t.jsx)(n.code,{children:"-t {Query the timeout value} -f {The query file}"})}),(0,t.jsx)(n.td,{children:"cypher command file query in interactive mode. The default timeout threshold is 150 seconds. Query file format reference No interactive query parameters."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:":exit"}),(0,t.jsx)(n.td,{children:"\\"}),(0,t.jsx)(n.td,{children:"Exit interactive mode and return to the original command line."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:":format"}),(0,t.jsxs)(n.td,{children:[(0,t.jsx)(n.code,{children:"plain"})," or ",(0,t.jsx)(n.code,{children:"table"})]}),(0,t.jsx)(n.td,{children:"Change the display mode of cypher query results. Support 'plain' and 'table' modes."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:":save all/command/result"}),(0,t.jsxs)(n.td,{children:[(0,t.jsx)(n.code,{children:"-f {The file path}"})," ",(0,t.jsx)(n.code,{children:"{Cypher}"})]}),(0,t.jsx)(n.td,{children:"The cypher command (command), query result (result) or both (all) are stored. The default location is' /saved_cypher.txt '"})]})]})]}),"\n",(0,t.jsx)(n.p,{children:(0,t.jsx)(n.strong,{children:"Note:"})}),"\n",(0,t.jsxs)(n.ul,{children:["\n",(0,t.jsxs)(n.li,{children:["Each command should start with a colon ",(0,t.jsx)(n.code,{children:":"}),"."]}),"\n"]}),"\n",(0,t.jsx)(n.p,{children:(0,t.jsx)(n.strong,{children:":save command example :"})}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{children:":save all -f /home/usr/saved.txt match (n) where return n, n.name limit 1000\n\n"})}),"\n",(0,t.jsx)(n.h3,{id:"cypher-query-command",children:"cypher query command:"}),"\n",(0,t.jsx)(n.p,{children:"In interactive mode, users can also directly input a single sentence cypher command for query, with \"'; ` \"end. Enter commands that are case insensitive. Here's an example:"}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{children:"login success\n>MATCH (n) RETURN n, n.name;\n+---+---+-------------+\n|   | n |n.name       |\n+---+---+-------------+\n| 0 | 0 |david        |\n| 1 | 1 |Ann          |\n| 2 | 2 |first movie  |\n| 3 | 3 |Andres       |\n+---+---+-------------+\ntime spent: 0.000520706176758\nsize of query: 4\n>\n"})}),"\n",(0,t.jsxs)(n.p,{children:[(0,t.jsx)(n.code,{children:"lgraph_cypher"})," supports multi-line input when typing commands. Users can use the ",(0,t.jsx)(n.code,{children:"ENTER"})," key to type long query statements into multiple lines. In the case of multi-line input, the beginning of the command line will change from ",(0,t.jsx)(n.code,{children:">"})," to ",(0,t.jsx)(n.code,{children:"=>"}),", and the user can continue to type the rest of the query."]}),"\n",(0,t.jsx)(n.p,{children:"Example:"}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{children:"login success\n>MATCH (n)\n=>WHERE n.uid='M11'\n=>RETURN n, n.name;\n"})}),"\n",(0,t.jsx)(n.h3,{id:"auxiliary-features",children:"Auxiliary Features:"}),"\n",(0,t.jsxs)(n.p,{children:[(0,t.jsx)(n.strong,{children:"Input History:"})," Press the up and down arrow keys in interactive mode to display the input history."]}),"\n",(0,t.jsxs)(n.p,{children:[(0,t.jsx)(n.strong,{children:"Auto Completion:"})," lgraph_cypher will automatically complete based on the input history. In the event of a completion prompt, pressing the right arrow key will automatically complete the command."]})]})}function o(e={}){const{wrapper:n}={...(0,s.R)(),...e.components};return n?(0,t.jsx)(n,{...e,children:(0,t.jsx)(h,{...e})}):h(e)}}}]);