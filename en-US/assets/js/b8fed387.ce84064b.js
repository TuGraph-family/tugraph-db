"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[31462],{798:(e,n,t)=>{t.r(n),t.d(n,{assets:()=>l,contentTitle:()=>a,default:()=>u,frontMatter:()=>o,metadata:()=>i,toc:()=>c});var r=t(74848),s=t(28453);const o={},a="Bolt client",i={id:"client-tools/bolt-client",title:"Bolt client",description:"TuGraph implements Neo4j's bolt protocol, you can use Neo4j's client to access TuGraph.",source:"@site/versions/version-4.5.2/en-US/source/7.client-tools/5.bolt-client.md",sourceDirName:"7.client-tools",slug:"/client-tools/bolt-client",permalink:"/tugraph-db/en-US/en/4.5.2/client-tools/bolt-client",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:5,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"TuGraph-OGM",permalink:"/tugraph-db/en-US/en/4.5.2/client-tools/tugraph-ogm"},next:{title:"TuGraph console client",permalink:"/tugraph-db/en-US/en/4.5.2/client-tools/bolt-console-client"}},l={},c=[{value:"Enable bolt port",id:"enable-bolt-port",level:2},{value:"Use case example",id:"use-case-example",level:2},{value:"Limitations on use",id:"limitations-on-use",level:2},{value:"Example of Client Usage",id:"example-of-client-usage",level:2}];function d(e){const n={a:"a",code:"code",h1:"h1",h2:"h2",header:"header",p:"p",pre:"pre",...(0,s.R)(),...e.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(n.header,{children:(0,r.jsx)(n.h1,{id:"bolt-client",children:"Bolt client"})}),"\n",(0,r.jsx)(n.p,{children:"TuGraph implements Neo4j's bolt protocol, you can use Neo4j's client to access TuGraph."}),"\n",(0,r.jsx)(n.h2,{id:"enable-bolt-port",children:"Enable bolt port"}),"\n",(0,r.jsxs)(n.p,{children:["The Bolt port is enabled by default on port 7687. If you need to change the port, you can do so in the configuration file. For services deployed using the Docker method, the configuration file is located within the container at ",(0,r.jsx)(n.code,{children:"/usr/local/etc/lgraph.json"}),"; for services deployed using the RPM package, the configuration file is on the server at ",(0,r.jsx)(n.code,{children:"/usr/local/etc/lgraph.json"}),". To apply the port change, you will need to restart the service. Specific instructions for restarting the service can be found in ",(0,r.jsx)(n.a,{href:"../../5.installation&running/7.tugraph-running.md",children:"Tugraph Running"}),'. Additionally, for a detailed explanation of the configuration options available in the configuration file, see the "Configuration parameters" section in ',(0,r.jsx)(n.a,{href:"../../5.installation&running/7.tugraph-running.md",children:"Tugraph Running"}),"."]}),"\n",(0,r.jsx)(n.h2,{id:"use-case-example",children:"Use case example"}),"\n",(0,r.jsx)(n.p,{children:"Add Maven dependency"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-xml",children:"<dependency>\n    <groupId>org.neo4j.driver</groupId>\n    <artifactId>neo4j-java-driver</artifactId>\n    <version>4.4.2</version>\n</dependency>\n"})}),"\n",(0,r.jsx)(n.p,{children:"Instantiate the client as follows:"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-java",children:'    Driver driver = GraphDatabase.driver("bolt://ip:port", AuthTokens.basic("admin", "73@TuGraph"));\n'})}),"\n",(0,r.jsx)(n.p,{children:"Common statements:"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-java",children:'        //Create a session through the driver object, setting the session to connect to a specific database, to execute Cypher statements.\n        Session session = driver.session(SessionConfig.forDatabase("default"));\n        //Clear the graph project, please do not attempt this lightly as it will delete both the schema and data of the selected graph project.\n        session.run("CALL db.dropDB()");\n        //Create vertex\n        session.run("CALL db.createVertexLabel(\'person\', \'id\' , \'id\' ,\'INT32\', false, \'name\' ,\'STRING\', false)");\n        //Create edge\n        session.run("CALL db.createEdgeLabel(\'is_friend\',\'[[\\"person\\",\\"person\\"]]\')");\n        //Create index\n        session.run("CALL db.addIndex(\\"person\\", \\"name\\", false)");\n        //Insert vertex data\n        session.run("create (n1:person {name:\'jack\',id:1}), (n2:person {name:\'lucy\',id:2})");\n        //Insert edge data\n        session.run("match (n1:person {id:1}), (n2:person {id:2}) create (n1)-[r:is_friend]->(n2)");\n        //Query vertices and edges\n        Result res = session.run("match (n)-[r]->(m) return n,r,m");\n        //Parameterized Query\n        String cypherQuery = "MATCH (n1:person {id:$id})-[r]-(n2:person {name:$name}) RETURN n1, r, n2";\n        Result result1 = session.run(cypherQuery, parameters("id", 1, "name", "lucy"));\n        while (result1.hasNext()) {\n        Record record = result1.next();\n        System.out.println("n1: " + record.get("n1").asMap());\n        System.out.println("r: " + record.get("r").asMap());\n        System.out.println("n2: " + record.get("n2").asMap());\n                }        \n        //Delete vertex data\n        session.run("match (n1:person {id:1}) delete n1");\n        //Delete edge data\n        session.run("match (n1:person {id:1})-[r]-(n2:person{id:2}) delete r");\n        //Delete edge\n        session.run("CALL db.deleteLabel(\'edge\', \'is_friend\')");\n        //Delete vertex\n        session.run("CALL db.deleteLabel(\'vertex\', \'person\')");\n'})}),"\n",(0,r.jsxs)(n.p,{children:["Details on the use of Cypher and stored procedures can be found in ",(0,r.jsx)(n.a,{href:"../../8.query/1.cypher.md",children:"Cypher"})]}),"\n",(0,r.jsx)(n.h2,{id:"limitations-on-use",children:"Limitations on use"}),"\n",(0,r.jsx)(n.p,{children:"Currently, our system does not fully support all advanced features of the Neo4j Bolt protocol. In particular, immediate transaction processing and flexible weak schema, two unique capabilities of the Neo4j client, are not yet supported. Therefore, when using the client, please make sure to avoid operations that involve these features to ensure smooth running of the application and consistency of data."}),"\n",(0,r.jsx)(n.h2,{id:"example-of-client-usage",children:"Example of Client Usage"}),"\n",(0,r.jsxs)(n.p,{children:["In the code directory under demo/Bolt, there are examples in Golang, JavaScript, Java, JavaScript, Python, and Rust. To learn more, see ",(0,r.jsx)(n.a,{href:"https://github.com/TuGraph-family/tugraph-db/tree/master/demo",children:"Example of Client Usage"})]})]})}function u(e={}){const{wrapper:n}={...(0,s.R)(),...e.components};return n?(0,r.jsx)(n,{...e,children:(0,r.jsx)(d,{...e})}):d(e)}},28453:(e,n,t)=>{t.d(n,{R:()=>a,x:()=>i});var r=t(96540);const s={},o=r.createContext(s);function a(e){const n=r.useContext(o);return r.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function i(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(s):e.components||s:a(e.components),r.createElement(o.Provider,{value:n},e.children)}}}]);