"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[33977],{62200:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>a,contentTitle:()=>d,default:()=>h,frontMatter:()=>i,metadata:()=>l,toc:()=>o});var t=r(74848),s=r(28453);const i={},d="DEMO",l={id:"quick-start/demo/movie",title:"DEMO:Movie",description:"This document mainly introduces the usage of the movie demo.",source:"@site/versions/version-4.2.0/en-US/source/3.quick-start/2.demo/1.movie.md",sourceDirName:"3.quick-start/2.demo",slug:"/quick-start/demo/movie",permalink:"/tugraph-db/en/4.2.0/quick-start/demo/movie",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:1,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Quick Start",permalink:"/tugraph-db/en/4.2.0/quick-start/preparation"},next:{title:"DEMO:Wandering Earth",permalink:"/tugraph-db/en/4.2.0/quick-start/demo/wandering-earth"}},a={},o=[{value:"1.Modeling and Data Import",id:"1modeling-and-data-import",level:2},{value:"2.Query Examples",id:"2query-examples",level:2},{value:"2.1.Example One",id:"21example-one",level:3},{value:"2.2.Example Two",id:"22example-two",level:3},{value:"2.3.Example Three",id:"23example-three",level:3},{value:"2.4.Example Four",id:"24example-four",level:3},{value:"2.5.Example Five",id:"25example-five",level:3},{value:"2.6.Example Six",id:"26example-six",level:3},{value:"2.7.Example Seven",id:"27example-seven",level:3}];function c(e){const n={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",img:"img",p:"p",pre:"pre",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",...(0,s.R)(),...e.components};return(0,t.jsxs)(t.Fragment,{children:[(0,t.jsx)(n.header,{children:(0,t.jsx)(n.h1,{id:"demo",children:"DEMO:Movie"})}),"\n",(0,t.jsxs)(n.blockquote,{children:["\n",(0,t.jsx)(n.p,{children:"This document mainly introduces the usage of the movie demo."}),"\n"]}),"\n",(0,t.jsx)(n.h2,{id:"1modeling-and-data-import",children:"1.Modeling and Data Import"}),"\n",(0,t.jsx)(n.p,{children:'After logging in, click "Help", click "Quick Start", click "One-Click Model Creation", click "One-Click Data Creation", and complete the creation of the Movie Scene Graph.'}),"\n",(0,t.jsx)(n.p,{children:"Movie\uff1a"}),"\n",(0,t.jsx)(n.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/1.Guide/2.quick-start.png",alt:"movie_schema",style:{zoom:"25%"}}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,t.jsxs)(n.table,{children:[(0,t.jsx)(n.thead,{children:(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.th,{children:"Label"}),(0,t.jsx)(n.th,{children:"Type"}),(0,t.jsx)(n.th,{children:"Description"})]})}),(0,t.jsxs)(n.tbody,{children:[(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"movie"}),(0,t.jsx)(n.td,{children:"Vertex"}),(0,t.jsx)(n.td,{children:'Represents a specific movie, such as "Forrest Gump".'})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"person"}),(0,t.jsx)(n.td,{children:"Vertex"}),(0,t.jsx)(n.td,{children:"Represents a person, who may be an actor, director, or screenwriter for a movie."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"genre"}),(0,t.jsx)(n.td,{children:"Vertex"}),(0,t.jsx)(n.td,{children:"Represents the genre of a movie, such as drama, horror."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"keyword"}),(0,t.jsx)(n.td,{children:"Vertex"}),(0,t.jsx)(n.td,{children:'Represents some keywords related to the movie, such as "save the world", "virtual reality", "subway".'})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"user"}),(0,t.jsx)(n.td,{children:"Vertex"}),(0,t.jsx)(n.td,{children:"Represents a user who watches movies."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"produce"}),(0,t.jsx)(n.td,{children:"Edge"}),(0,t.jsx)(n.td,{children:"Represents the relationship between the producer of a movie."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"acted_in"}),(0,t.jsx)(n.td,{children:"Edge"}),(0,t.jsx)(n.td,{children:"Represents the relationship between actors and the movies they have appeared in."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"direct"}),(0,t.jsx)(n.td,{children:"Edge"}),(0,t.jsx)(n.td,{children:"Represents the relationship between a movie and its director."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"write"}),(0,t.jsx)(n.td,{children:"Edge"}),(0,t.jsx)(n.td,{children:"Represents the screenwriting relationship of a movie."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"has_genre"}),(0,t.jsx)(n.td,{children:"Edge"}),(0,t.jsx)(n.td,{children:"Represents the genre classification of a movie."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"has_keyword"}),(0,t.jsx)(n.td,{children:"Edge"}),(0,t.jsx)(n.td,{children:"Represents some keywords of a movie, which are more specific labels for classification."})]}),(0,t.jsxs)(n.tr,{children:[(0,t.jsx)(n.td,{children:"rate"}),(0,t.jsx)(n.td,{children:"Edge"}),(0,t.jsx)(n.td,{children:"Represents the rating given by a user to a movie."})]})]})]}),"\n",(0,t.jsx)(n.h2,{id:"2query-examples",children:"2.Query Examples"}),"\n",(0,t.jsx)(n.h3,{id:"21example-one",children:"2.1.Example One"}),"\n",(0,t.jsx)(n.p,{children:"Query all actors of the movie 'Forrest Gump' and return the subgraph composed of the movie and the actors."}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{children:"MATCH (m:movie {title: 'Forrest Gump'})<-[:acted_in]-(a:person) RETURN a, m\n"})}),"\n",(0,t.jsx)(n.h3,{id:"22example-two",children:"2.2.Example Two"}),"\n",(0,t.jsx)(n.p,{children:"Query all actors of the movie 'Forrest Gump' and list the roles they played in the movie."}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{children:"MATCH (m:movie {title: 'Forrest Gump'})<-[r:acted_in]-(a:person) RETURN a.name,r.role\n"})}),"\n",(0,t.jsx)(n.h3,{id:"23example-three",children:"2.3.Example Three"}),"\n",(0,t.jsx)(n.p,{children:"Query all movies rated below 3 by Michael."}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{children:"MATCH (u:user {login: 'Michael'})-[r:rate]->(m:movie) WHERE r.stars < 3 RETURN m.title, r.stars\n"})}),"\n",(0,t.jsx)(n.h3,{id:"24example-four",children:"2.4.Example Four"}),"\n",(0,t.jsx)(n.p,{children:"Query users who have the same dislike of movies as Michael, where the standard for dislike is a rating of less than three."}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{children:"MATCH (u:user {login: 'Michael'})-[r:rate]->(m:movie)<-[s:rate]-(v) WHERE r.stars < 3 AND s.stars < 3 RETURN u, m, v\n"})}),"\n",(0,t.jsx)(n.h3,{id:"25example-five",children:"2.5.Example Five"}),"\n",(0,t.jsx)(n.p,{children:"Recommend movies to Michael by first finding users who dislike the same movies as Michael, and then filtering out the movies that these users like."}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{children:"MATCH (u:user {login: 'Michael'})-[r:rate]->(m:movie)<-[s:rate]-(v)-[r2:rate]->(m2:movie) WHERE r.stars < 3 AND s.stars < 3 AND r2.stars > 3 RETURN u, m, v, m2\n"})}),"\n",(0,t.jsx)(n.h3,{id:"26example-six",children:"2.6.Example Six"}),"\n",(0,t.jsx)(n.p,{children:"Query the movies that Michael's friends like."}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{children:"MATCH (u:user {login: 'Michael'})-[:is_friend]->(v:user)-[r:rate]->(m:movie) WHERE r.stars > 3 RETURN u, v, m\n"})}),"\n",(0,t.jsx)(n.h3,{id:"27example-seven",children:"2.7.Example Seven"}),"\n",(0,t.jsx)(n.p,{children:"By querying the movies that people who gave 'Forrest Gump' a high rating also like, recommend similar movies to users who like 'Forrest Gump'."}),"\n",(0,t.jsx)(n.pre,{children:(0,t.jsx)(n.code,{children:"MATCH (m:movie {title:'Forrest Gump'})<-[r:rate]-(u:user)-[r2:rate]->(m2:movie) WHERE r.stars>3 AND r2.stars>3 RETURN m, u,m2\n"})})]})}function h(e={}){const{wrapper:n}={...(0,s.R)(),...e.components};return n?(0,t.jsx)(n,{...e,children:(0,t.jsx)(c,{...e})}):c(e)}},28453:(e,n,r)=>{r.d(n,{R:()=>d,x:()=>l});var t=r(96540);const s={},i=t.createContext(s);function d(e){const n=t.useContext(i);return t.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function l(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(s):e.components||s:d(e.components),t.createElement(i.Provider,{value:n},e.children)}}}]);