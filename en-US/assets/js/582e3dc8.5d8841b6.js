"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[32959],{28453:(e,n,t)=>{t.d(n,{R:()=>o,x:()=>a});var r=t(96540);const s={},i=r.createContext(s);function o(e){const n=r.useContext(i);return r.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function a(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(s):e.components||s:o(e.components),r.createElement(i.Provider,{value:n},e.children)}},83809:(e,n,t)=>{t.r(n),t.d(n,{assets:()=>d,contentTitle:()=>o,default:()=>h,frontMatter:()=>i,metadata:()=>a,toc:()=>l});var r=t(74848),s=t(28453);const i={},o="TuGraph-OGM",a={id:"client-tools/tugraph-ogm",title:"TuGraph-OGM",description:"@TODO",source:"@site/versions/version-4.3.1/en-US/source/7.client-tools/4.tugraph-ogm.md",sourceDirName:"7.client-tools",slug:"/client-tools/tugraph-ogm",permalink:"/tugraph-db/en-US/en/4.3.1/client-tools/tugraph-ogm",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:4,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"TuGraph Java SDK",permalink:"/tugraph-db/en-US/en/4.3.1/client-tools/java-client"},next:{title:"Bolt client",permalink:"/tugraph-db/en-US/en/4.3.1/client-tools/bolt-client"}},d={},l=[{value:"1.Introduction",id:"1introduction",level:2},{value:"1.1.Functions of TuGraph-OGM",id:"11functions-of-tugraph-ogm",level:3},{value:"2.Compile TuGraph-OGM",id:"2compile-tugraph-ogm",level:2},{value:"3.Use TuGraph-OGM",id:"3use-tugraph-ogm",level:2},{value:"3.1.\u6784\u5efa\u56fe\u5bf9\u8c61",id:"31\u6784\u5efa\u56fe\u5bf9\u8c61",level:3},{value:"3.2.\u4e0eTuGraph\u5efa\u7acb\u8fde\u63a5",id:"32\u4e0etugraph\u5efa\u7acb\u8fde\u63a5",level:3},{value:"3.3.Perform CRUD operations through OGM",id:"33perform-crud-operations-through-ogm",level:3}];function c(e){const n={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",id:"id",p:"p",pre:"pre",t:"t",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",...(0,s.R)(),...e.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(n.header,{children:(0,r.jsx)(n.h1,{id:"tugraph-ogm",children:"TuGraph-OGM"})}),"\n",(0,r.jsx)(n.p,{children:"@TODO"}),"\n",(0,r.jsx)(n.h2,{id:"1introduction",children:"1.Introduction"}),"\n",(0,r.jsxs)(n.blockquote,{children:["\n",(0,r.jsx)(n.p,{children:"TuGraph-OGM project is open source on other repositories."}),"\n"]}),"\n",(0,r.jsx)(n.p,{children:"TuGraph-OGM (Object Graph Mapping) is a graph object mapping tool designed for TuGraph. It supports the mapping of Java objects (POJO) into TuGraph. The classes in Java are mapped to nodes in the graph, the collections in the classes are mapped to edges, and the properties of the classes are mapped to the attributes of the graph objects. It also provides corresponding functions to manipulate the graph database. Therefore, Java developers can easily use TuGraph database in the familiar ecosystem. TuGraph-OGM is also compatible with Neo4j-OGM. Neo4j ecosystem users can migrate seamlessly to TuGraph database."}),"\n",(0,r.jsx)(n.h3,{id:"11functions-of-tugraph-ogm",children:"1.1.Functions of TuGraph-OGM"}),"\n",(0,r.jsx)(n.p,{children:"TuGraph-OGM provides the following functions for operating TuGraph:"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,r.jsxs)(n.table,{children:[(0,r.jsx)(n.thead,{children:(0,r.jsxs)(n.tr,{children:[(0,r.jsx)(n.th,{children:"Function"}),(0,r.jsx)(n.th,{children:"Usage"})]})}),(0,r.jsxs)(n.tbody,{children:[(0,r.jsxs)(n.tr,{children:[(0,r.jsx)(n.td,{children:"Insert single vertex/edge"}),(0,r.jsx)(n.td,{children:"void session.save(T object)"})]}),(0,r.jsxs)(n.tr,{children:[(0,r.jsx)(n.td,{children:"Batch insert vertexs/edges"}),(0,r.jsx)(n.td,{children:"void session.save(T object)"})]}),(0,r.jsxs)(n.tr,{children:[(0,r.jsx)(n.td,{children:"Delete vertex and its corresponding edges"}),(0,r.jsx)(n.td,{children:"void session.delete(T object)"})]}),(0,r.jsxs)(n.tr,{children:[(0,r.jsx)(n.td,{children:"Delete all vertexs with specific label"}),(0,r.jsx)(n.td,{children:"void session.deleteAll(Class<T> type)"})]}),(0,r.jsxs)(n.tr,{children:[(0,r.jsx)(n.td,{children:"Clear the database"}),(0,r.jsx)(n.td,{children:"void purgeDatabase()"})]}),(0,r.jsxs)(n.tr,{children:[(0,r.jsx)(n.td,{children:"Update vertex"}),(0,r.jsx)(n.td,{children:"void session.save(T newObject)"})]}),(0,r.jsxs)(n.tr,{children:[(0,r.jsx)(n.td,{children:"Query single node by id"}),(0,r.jsxs)(n.td,{children:["T load(Class",(0,r.jsx)(n.t,{children:" type, ID id)"})]})]}),(0,r.jsxs)(n.tr,{children:[(0,r.jsx)(n.td,{children:"Query multiple nodes by ids"}),(0,r.jsxs)(n.td,{children:["Collection<T> loadAll(Class<T> type, Collection",(0,r.jsx)(n.id,{children:" ids)"})]})]}),(0,r.jsxs)(n.tr,{children:[(0,r.jsx)(n.td,{children:"Query all nodes with specific label"}),(0,r.jsx)(n.td,{children:"Collection<T> loadAll(Class<T> type)"})]}),(0,r.jsxs)(n.tr,{children:[(0,r.jsx)(n.td,{children:"Query with conditions"}),(0,r.jsx)(n.td,{children:"Collection<T> loadAll(Class<T> type, Filters filters)"})]}),(0,r.jsxs)(n.tr,{children:[(0,r.jsx)(n.td,{children:"Cypher query (specify the return type)"}),(0,r.jsx)(n.td,{children:"T queryForObject(Class<T> objectType, String cypher, Map<String, ?> parameters)"})]}),(0,r.jsxs)(n.tr,{children:[(0,r.jsx)(n.td,{children:"Cypher query"}),(0,r.jsx)(n.td,{children:"Result query(String cypher, Map<String, ?> parameters)"})]})]})]}),"\n",(0,r.jsx)(n.h2,{id:"2compile-tugraph-ogm",children:"2.Compile TuGraph-OGM"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-shell",children:"cd tugraph-ogm\nmvn clean install -DskipTests -Denforcer.skip=true\n"})}),"\n",(0,r.jsx)(n.h2,{id:"3use-tugraph-ogm",children:"3.Use TuGraph-OGM"}),"\n",(0,r.jsxs)(n.p,{children:["Please refer to the detailed examples in TuGraphOGMDemo under the demo folder.How to import dependencies in ",(0,r.jsx)(n.code,{children:"pom.xml"}),"."]}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{children:"<dependency>\n        <groupId>org.neo4j</groupId>\n        <artifactId>neo4j-ogm-api</artifactId>\n        <version>0.1.0-SNAPSHOT</version>\n    </dependency>\n\n    <dependency>\n        <groupId>org.neo4j</groupId>\n        <artifactId>neo4j-ogm-core</artifactId>\n        <version>0.1.0-SNAPSHOT</version>\n    </dependency>\n\n    <dependency>\n        <groupId>org.neo4j</groupId>\n        <artifactId>tugraph-rpc-driver</artifactId>\n        <version>0.1.0-SNAPSHOT</version>\n    </dependency>\n"})}),"\n",(0,r.jsx)(n.h3,{id:"31\u6784\u5efa\u56fe\u5bf9\u8c61",children:"3.1.\u6784\u5efa\u56fe\u5bf9\u8c61"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-java",children:'@NodeEntity\npublic class Movie {      // create Movie vertex\n    @Id\n    private Long id;      // Movie id\n    private String title; // title attribute\n    private int released; // released attribute\n\n    // create Edge ACTS_IN    (actor)-[:ACTS_IN]->(movie)\n    @Relationship(type = "ACTS_IN", direction = Relationship.Direction.INCOMING)\n    Set<Actor> actors = new HashSet<>();\n\n    public Movie(String title, int year) {\n        this.title = title;\n        this.released = year;\n    }\n\n    public Long getId() {\n        return id;\n    }\n\n    public void setReleased(int released) {\n        this.released = released;\n    }\n}\n\n@NodeEntity\npublic class Actor {      // create Actor vertex\n    @Id\n    private Long id;\n    private String name;\n\n    @Relationship(type = "ACTS_IN", direction = Relationship.Direction.OUTGOING)\n    private Set<Movie> movies = new HashSet<>();\n\n    public Actor(String name) {\n        this.name = name;\n    }\n\n    public void actsIn(Movie movie) {\n        movies.add(movie);\n        movie.getActors().add(this);\n    }\n}\n'})}),"\n",(0,r.jsx)(n.h3,{id:"32\u4e0etugraph\u5efa\u7acb\u8fde\u63a5",children:"3.2.\u4e0eTuGraph\u5efa\u7acb\u8fde\u63a5"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-java",children:'// configuration\nString databaseUri = "list://ip:port";\nString username = "admin";\nString password = "73@TuGraph";\n//Start driver\nDriver driver = new RpcDriver();\nConfiguration.Builder baseConfigurationBuilder = new Configuration.Builder()\n                            .uri(databaseUri)\n                            .verifyConnection(true)\n                            .credentials(username, password);\n                            driver.configure(baseConfigurationBuilder.build());\ndriver.configure(baseConfigurationBuilder.build());\n// Open session\nSessionFactory sessionFactory = new SessionFactory(driver, "entity_path");\nSession session = sessionFactory.openSession();\n'})}),"\n",(0,r.jsx)(n.h3,{id:"33perform-crud-operations-through-ogm",children:"3.3.Perform CRUD operations through OGM"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-java",children:'// new\nMovie jokes = new Movie("Jokes", 1990);  // Create Movie-vertex "jokes"\nsession.save(jokes);                     // Store "jokes" in TuGraph\n\nMovie speed = new Movie("Speed", 2019);\nActor alice = new Actor("Alice Neeves");\nalice.actsIn(speed);                    // Connect the "speed" vertex with the "alice" vertex through the ACTS_IN relationship\nsession.save(speed);                    // Store two vertexs and one edge\n\n// delete\nsession.delete(alice);                  // Delete the "alice" vertex and its connected edge\nMovie m = session.load(Movie.class, jokes.getId());   // Get the "jokes" vertex based on its id\nsession.delete(m);                                    // Delete the "jokes" node\n\n// update\nspeed.setReleased(2018);\nsession.save(speed);                   // Update the "speed" attribute\n\n// get\nCollection<Movie> movies = session.loadAll(Movie.class);  // Get all Movie vertexs\nCollection<Movie> moviesFilter = session.loadAll(Movie.class,\n        new Filter("released", ComparisonOperator.LESS_THAN, 1995));  // Query all movies released before 1995\n\n// Call Cypher\nHashMap<String, Object> parameters = new HashMap<>();\nparameters.put("Speed", 2018);\nMovie cm = session.queryForObject(Movie.class,\n        "MATCH (cm:Movie{Speed: $Speed}) RETURN *", parameters);      // Query the Movie with "Speed" of 2018\n\nsession.query("CALL db.createVertexLabel(\'Director\', \'name\', \'name\'," +\n        "STRING, false, \'age\', INT16, true)", emptyMap());            // Create vertex label "Director"\nsession.query("CALL db.createEdgeLabel(\'DIRECT\', \'[]\')", emptyMap()); // Create edge label "DIRECT"\nResult createResult = session.query(\n        "CREATE (n:Movie{title:\\"The Shawshank Redemption\\", released:1994})" +\n        "<-[r:DIRECT]-" +\n        "(n2:Director{name:\\"Frank Darabont\\", age:63})",\n        emptyMap());\nQueryStatistics statistics = createResult.queryStatistics();          // Get the create result\nSystem.out.println("created " + statistics.getNodesCreated() + " vertices");    // View the number of vertexs created\nSystem.out.println("created " + statistics.getRelationshipsCreated() + " edges");  //View the number of edges created\n\n// Clear the database\nsession.deleteAll(Movie.class);        // Delete all Movie vertexs\nsession.purgeDatabase();               // Delete all data\n'})})]})}function h(e={}){const{wrapper:n}={...(0,s.R)(),...e.components};return n?(0,r.jsx)(n,{...e,children:(0,r.jsx)(c,{...e})}):c(e)}}}]);