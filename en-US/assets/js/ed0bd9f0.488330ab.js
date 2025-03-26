"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[16029],{28453:(e,n,t)=>{t.d(n,{R:()=>s,x:()=>l});var r=t(96540);const o={},i=r.createContext(o);function s(e){const n=r.useContext(i);return r.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function l(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(o):e.components||o:s(e.components),r.createElement(i.Provider,{value:n},e.children)}},56147:(e,n,t)=>{t.r(n),t.d(n,{assets:()=>a,contentTitle:()=>s,default:()=>h,frontMatter:()=>i,metadata:()=>l,toc:()=>c});var r=t(74848),o=t(28453);const i={},s="TuGraph console client",l={id:"client-tools/bolt-console-client",title:"TuGraph console client",description:"lgraph_cli is a console client based on the bolt protocol, written in c++, which requires a connection to tugraph's bolt port.",source:"@site/versions/version-4.5.1/en-US/source/7.client-tools/6.bolt-console-client.md",sourceDirName:"7.client-tools",slug:"/client-tools/bolt-console-client",permalink:"/tugraph-db/en-US/en/4.5.1/client-tools/bolt-console-client",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:6,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Bolt client",permalink:"/tugraph-db/en-US/en/4.5.1/client-tools/bolt-client"},next:{title:"TuGraph RESTful API",permalink:"/tugraph-db/en-US/en/4.5.1/client-tools/restful-api"}},a={},c=[{value:"<code>lgraph_cli</code>",id:"lgraph_cli",level:2},{value:"online export",id:"online-export",level:2},{value:"csv",id:"csv",level:3},{value:"json",id:"json",level:3}];function p(e){const n={code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",p:"p",pre:"pre",...(0,o.R)(),...e.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(n.header,{children:(0,r.jsx)(n.h1,{id:"tugraph-console-client",children:"TuGraph console client"})}),"\n",(0,r.jsxs)(n.p,{children:[(0,r.jsx)(n.code,{children:"lgraph_cli"})," is a console client based on the bolt protocol, written in c++, which requires a connection to tugraph's bolt port."]}),"\n",(0,r.jsxs)(n.p,{children:[(0,r.jsx)(n.code,{children:"lgraph_cypher"})," is a console client based on http, written in python, which requires a connection to tugraph's http port."]}),"\n",(0,r.jsxs)(n.p,{children:[(0,r.jsx)(n.code,{children:"lgraph_cypher"})," needs some python libraries to be installed."]}),"\n",(0,r.jsxs)(n.p,{children:[(0,r.jsx)(n.code,{children:"lgraph_cli"})," is a binary executable file that has no dependencies on other dynamic libraries and can be executed by copying it to a linux server."]}),"\n",(0,r.jsx)(n.h2,{id:"lgraph_cli",children:(0,r.jsx)(n.code,{children:"lgraph_cli"})}),"\n",(0,r.jsxs)(n.p,{children:["The statement ends with a semicolon, type ",(0,r.jsx)(n.code,{children:"exit"}),", ",(0,r.jsx)(n.code,{children:"quit"})," or Ctrl-C to exit the client."]}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-powershell",children:"lgraph_cli --ip 127.0.0.1 --port 7687 --graph default --user admin --password 73@TuGraph\n\nWelcome to the TuGraph console client. Commands end with ';'.\nCopyright(C) 2018-2023 Ant Group. All rights reserved.\nType 'exit', 'quit' or Ctrl-C to exit.\n\nTuGraph> match(n) return n limit 1;\n+-------------------------------------------------------------------------------------------------------------------------------------+\n| n                                                                                                                                   |\n+-------------------------------------------------------------------------------------------------------------------------------------+\n| (:person {id:2,born:1961,poster_image:\"https://image.tmdb.org/t/p/w185/mh0lZ1XsT84FayMNiT6Erh91mVu.jpg\",name:\"Laurence Fishburne\"}) |\n+-------------------------------------------------------------------------------------------------------------------------------------+\n\nTuGraph>\n"})}),"\n",(0,r.jsx)(n.p,{children:"The statement can be inputed on more than one line."}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{children:'TuGraph> match(n)\n      -> return n\n      -> limit 1;\n+-------------------------------------------------------------------------------------------------------------------------------------+\n| n                                                                                                                                   |\n+-------------------------------------------------------------------------------------------------------------------------------------+\n| (:person {id:2,born:1961,poster_image:"https://image.tmdb.org/t/p/w185/mh0lZ1XsT84FayMNiT6Erh91mVu.jpg",name:"Laurence Fishburne"}) |\n+-------------------------------------------------------------------------------------------------------------------------------------+\n\nTuGraph>\n'})}),"\n",(0,r.jsx)(n.p,{children:"non-interactive"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-powershell",children:'\necho "match(n) return n limit 1;" | lgraph_cli --ip 127.0.0.1 --port 7687 --graph default --user admin --password 73@TuGraph\n+-------------------------------------------------------------------------------------------------------------------------------------+\n| n                                                                                                                                   |\n+-------------------------------------------------------------------------------------------------------------------------------------+\n| (:person {id:2,born:1961,poster_image:"https://image.tmdb.org/t/p/w185/mh0lZ1XsT84FayMNiT6Erh91mVu.jpg",name:"Laurence Fishburne"}) |\n+-------------------------------------------------------------------------------------------------------------------------------------+\n1 rows\n\n'})}),"\n",(0,r.jsx)(n.p,{children:"read statements from file"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-powershell",children:'\ncat query.txt\nmatch(n) return n limit 1;\nmatch(n) return n limit 1;\n\nlgraph_cli --ip 127.0.0.1 --port 7687 --graph default --user admin --password 73@TuGraph < query.txt\n+-------------------------------------------------------------------------------------------------------------------------------------+\n| n                                                                                                                                   |\n+-------------------------------------------------------------------------------------------------------------------------------------+\n| (:person {id:2,born:1961,poster_image:"https://image.tmdb.org/t/p/w185/mh0lZ1XsT84FayMNiT6Erh91mVu.jpg",name:"Laurence Fishburne"}) |\n+-------------------------------------------------------------------------------------------------------------------------------------+\n| (:person {id:3,born:1967,poster_image:"https://image.tmdb.org/t/p/w185/8iATAc5z5XOKFFARLsvaawa8MTY.jpg",name:"Carrie-Anne Moss"})   |\n+-------------------------------------------------------------------------------------------------------------------------------------+\n2 rows\n\n+-------------------------------------------------------------------------------------------------------------------------------------+\n| n                                                                                                                                   |\n+-------------------------------------------------------------------------------------------------------------------------------------+\n| (:person {id:2,born:1961,poster_image:"https://image.tmdb.org/t/p/w185/mh0lZ1XsT84FayMNiT6Erh91mVu.jpg",name:"Laurence Fishburne"}) |\n+-------------------------------------------------------------------------------------------------------------------------------------+\n| (:person {id:3,born:1967,poster_image:"https://image.tmdb.org/t/p/w185/8iATAc5z5XOKFFARLsvaawa8MTY.jpg",name:"Carrie-Anne Moss"})   |\n+-------------------------------------------------------------------------------------------------------------------------------------+\n2 rows\n'})}),"\n",(0,r.jsx)(n.h2,{id:"online-export",children:"online export"}),"\n",(0,r.jsx)(n.p,{children:"lgraph_cli supports streaming read, so just redirect the output to a file. The output format supports csv and json"}),"\n",(0,r.jsx)(n.h3,{id:"csv",children:"csv"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-powershell",children:'\necho "match(n) return n.id, n.name;" | lgraph_cli --ip 127.0.0.1 --port 7687 --graph default --user admin --password 73@TuGraph --format csv > output.txt\n\n'})}),"\n",(0,r.jsx)(n.h3,{id:"json",children:"json"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-powershell",children:'\necho "match(n) return n.id, n.name;" | lgraph_cli --ip 127.0.0.1 --port 7687 --graph default --user admin --password 73@TuGraph --format json > output.txt\n\n'})})]})}function h(e={}){const{wrapper:n}={...(0,o.R)(),...e.components};return n?(0,r.jsx)(n,{...e,children:(0,r.jsx)(p,{...e})}):p(e)}}}]);