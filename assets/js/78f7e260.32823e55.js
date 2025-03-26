"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[96700],{28453:(n,e,a)=>{a.d(e,{R:()=>s,x:()=>o});var t=a(96540);const r={},i=t.createContext(r);function s(n){const e=t.useContext(i);return t.useMemo((function(){return"function"==typeof n?n(e):{...e,...n}}),[e,n])}function o(n){let e;return e=n.disableParentContext?"function"==typeof n.components?n.components(r):n.components||r:s(n.components),t.createElement(i.Provider,{value:e},n.children)}},78467:(n,e,a)=>{a.r(e),a.d(e,{assets:()=>l,contentTitle:()=>s,default:()=>p,frontMatter:()=>i,metadata:()=>o,toc:()=>d});var t=a(74848),r=a(28453);const i={},s="TuGraph DataX",o={id:"utility-tools/tugraph-datax",title:"TuGraph DataX",description:"This document mainly introduces the installation, compilation and usage examples of TuGraph DataX",source:"@site/versions/version-4.3.1/en-US/source/6.utility-tools/7.tugraph-datax.md",sourceDirName:"6.utility-tools",slug:"/utility-tools/tugraph-datax",permalink:"/tugraph-db/en/4.3.1/utility-tools/tugraph-datax",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:7,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Tugraph CLI",permalink:"/tugraph-db/en/4.3.1/utility-tools/tugraph-cli"},next:{title:"TuGraph Explorer",permalink:"/tugraph-db/en/4.3.1/utility-tools/tugraph-explorer"}},l={},d=[{value:"1.Introduction",id:"1introduction",level:2},{value:"2.Compile and Install",id:"2compile-and-install",level:2},{value:"3.Text data imported into TuGraph with DataX",id:"3text-data-imported-into-tugraph-with-datax",level:2},{value:"MySQL&#39;s data imported into TuGraph with DataX",id:"mysqls-data-imported-into-tugraph-with-datax",level:2}];function c(n){const e={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",header:"header",li:"li",p:"p",pre:"pre",strong:"strong",ul:"ul",...(0,r.R)(),...n.components};return(0,t.jsxs)(t.Fragment,{children:[(0,t.jsx)(e.header,{children:(0,t.jsx)(e.h1,{id:"tugraph-datax",children:"TuGraph DataX"})}),"\n",(0,t.jsxs)(e.blockquote,{children:["\n",(0,t.jsx)(e.p,{children:"This document mainly introduces the installation, compilation and usage examples of TuGraph DataX"}),"\n"]}),"\n",(0,t.jsx)(e.h2,{id:"1introduction",children:"1.Introduction"}),"\n",(0,t.jsxs)(e.p,{children:["On the basis of Ali's open source DataX, TuGraph implements the support of writing plug-ins and jsonline data format, and other data sources can write data into TuGraph through DataX.\nTuGraph DataX introduces ",(0,t.jsx)(e.a,{href:"https://github.com/TuGraph-family/DataX",children:"https://github.com/TuGraph-family/DataX"}),", Supported features include:"]}),"\n",(0,t.jsxs)(e.ul,{children:["\n",(0,t.jsxs)(e.li,{children:["\n",(0,t.jsx)(e.p,{children:"Import TuGraph from various heterogeneous data sources such as MySQL, SQL Server,Oracle, PostgreSQL, HDFS, Hive, HBase, OTS, ODPS, Kafka and so on."}),"\n"]}),"\n",(0,t.jsxs)(e.li,{children:["\n",(0,t.jsx)(e.p,{children:"Import TuGraph to the corresponding target source (to be developed)."}),"\n"]}),"\n"]}),"\n",(0,t.jsxs)(e.p,{children:["Reference for DataX Original Project Introduction ",(0,t.jsx)(e.a,{href:"https://github.com/alibaba/DataX",children:"https://github.com/alibaba/DataX"})]}),"\n",(0,t.jsx)(e.h2,{id:"2compile-and-install",children:"2.Compile and Install"}),"\n",(0,t.jsx)(e.pre,{children:(0,t.jsx)(e.code,{className:"language-bash",children:"git clone https://github.com/TuGraph-family/DataX.git\nmvn -U clean package assembly:assembly -Dmaven.test.skip=true\n"})}),"\n",(0,t.jsx)(e.p,{children:"The compiled DataX file is in the target directory"}),"\n",(0,t.jsx)(e.h2,{id:"3text-data-imported-into-tugraph-with-datax",children:"3.Text data imported into TuGraph with DataX"}),"\n",(0,t.jsxs)(e.p,{children:["Using the data from the lgraph_import section of the TuGraph manual as an example, we have three csv data files, as follows:\n",(0,t.jsx)(e.code,{children:"actors.csv"})]}),"\n",(0,t.jsx)(e.pre,{children:(0,t.jsx)(e.code,{children:"\nnm015950,Stephen Chow\nnm0628806,Man-Tat Ng\nnm0156444,Cecilia Cheung\nnm2514879,Yuqi Zhang\n\n"})}),"\n",(0,t.jsx)(e.p,{children:(0,t.jsx)(e.code,{children:"movies.csv"})}),"\n",(0,t.jsx)(e.pre,{children:(0,t.jsx)(e.code,{children:"\ntt0188766,King of Comedy,1999,7.3\ntt0286112,Shaolin Soccer,2001,7.3\ntt4701660,The Mermaid,2016,6.3\n\n"})}),"\n",(0,t.jsx)(e.p,{children:(0,t.jsx)(e.code,{children:"roles.csv"})}),"\n",(0,t.jsx)(e.pre,{children:(0,t.jsx)(e.code,{children:"\nnm015950,Tianchou Yin,tt0188766\nnm015950,Steel Leg,tt0286112\nnm0628806,,tt0188766\nnm0628806,coach,tt0286112\nnm0156444,PiaoPiao Liu,tt0188766\nnm2514879,Ruolan Li,tt4701660\n\n"})}),"\n",(0,t.jsxs)(e.p,{children:["Then create three DataX job profiles:\n",(0,t.jsx)(e.code,{children:"job_actors.json"})]}),"\n",(0,t.jsx)(e.pre,{children:(0,t.jsx)(e.code,{className:"language-json",children:'{\n  "job": {\n    "setting": {\n      "speed": {\n        "channel": 1\n      }\n    },\n    "content": [\n      {\n        "reader": {\n          "name": "txtfilereader",\n          "parameter": {\n            "path": ["actors.csv"],\n            "encoding": "UTF-8",\n            "column": [\n              {\n                "index": 0,\n                "type": "string"\n              },\n              {\n                "index": 1,\n                "type": "string"\n              }\n            ],\n            "fieldDelimiter": ","\n          }\n        },\n        "writer": {\n          "name": "tugraphwriter",\n          "parameter": {\n            "host": "127.0.0.1",\n            "port": 7071,\n            "username": "admin",\n            "password": "73@TuGraph",\n            "graphName": "default",\n            "schema": [\n              {\n                "label": "actor",\n                "type": "VERTEX",\n                "properties": [\n                  { "name": "aid", "type": "STRING" },\n                  { "name": "name", "type": "STRING" }\n                ],\n                "primary": "aid"\n              }\n            ],\n            "files": [\n              {\n                "label": "actor",\n                "format": "JSON",\n                "columns": ["aid", "name"]\n              }\n            ]\n          }\n        }\n      }\n    ]\n  }\n}\n'})}),"\n",(0,t.jsx)(e.p,{children:(0,t.jsx)(e.code,{children:"job_movies.json"})}),"\n",(0,t.jsx)(e.pre,{children:(0,t.jsx)(e.code,{className:"language-json",children:'{\n  "job": {\n    "setting": {\n      "speed": {\n        "channel": 1\n      }\n    },\n    "content": [\n      {\n        "reader": {\n          "name": "txtfilereader",\n          "parameter": {\n            "path": ["movies.csv"],\n            "encoding": "UTF-8",\n            "column": [\n              {\n                "index": 0,\n                "type": "string"\n              },\n              {\n                "index": 1,\n                "type": "string"\n              },\n              {\n                "index": 2,\n                "type": "string"\n              },\n              {\n                "index": 3,\n                "type": "string"\n              }\n            ],\n            "fieldDelimiter": ","\n          }\n        },\n        "writer": {\n          "name": "tugraphwriter",\n          "parameter": {\n            "host": "127.0.0.1",\n            "port": 7071,\n            "username": "admin",\n            "password": "73@TuGraph",\n            "graphName": "default",\n            "schema": [\n              {\n                "label": "movie",\n                "type": "VERTEX",\n                "properties": [\n                  { "name": "mid", "type": "STRING" },\n                  { "name": "name", "type": "STRING" },\n                  { "name": "year", "type": "STRING" },\n                  { "name": "rate", "type": "FLOAT", "optional": true }\n                ],\n                "primary": "mid"\n              }\n            ],\n            "files": [\n              {\n                "label": "movie",\n                "format": "JSON",\n                "columns": ["mid", "name", "year", "rate"]\n              }\n            ]\n          }\n        }\n      }\n    ]\n  }\n}\n'})}),"\n",(0,t.jsx)(e.p,{children:(0,t.jsx)(e.code,{children:"job_roles.json"})}),"\n",(0,t.jsx)(e.pre,{children:(0,t.jsx)(e.code,{className:"language-json",children:'{\n  "job": {\n    "setting": {\n      "speed": {\n        "channel": 1\n      }\n    },\n    "content": [\n      {\n        "reader": {\n          "name": "txtfilereader",\n          "parameter": {\n            "path": ["roles.csv"],\n            "encoding": "UTF-8",\n            "column": [\n              {\n                "index": 0,\n                "type": "string"\n              },\n              {\n                "index": 1,\n                "type": "string"\n              },\n              {\n                "index": 2,\n                "type": "string"\n              }\n            ],\n            "fieldDelimiter": ","\n          }\n        },\n        "writer": {\n          "name": "tugraphwriter",\n          "parameter": {\n            "host": "127.0.0.1",\n            "port": 7071,\n            "username": "admin",\n            "password": "73@TuGraph",\n            "graphName": "default",\n            "schema": [\n              {\n                "label": "play_in",\n                "type": "EDGE",\n                "properties": [{ "name": "role", "type": "STRING" }]\n              }\n            ],\n            "files": [\n              {\n                "label": "play_in",\n                "format": "JSON",\n                "SRC_ID": "actor",\n                "DST_ID": "movie",\n                "columns": ["SRC_ID", "role", "DST_ID"]\n              }\n            ]\n          }\n        }\n      }\n    ]\n  }\n}\n'})}),"\n",(0,t.jsxs)(e.p,{children:[(0,t.jsx)(e.code,{children:"/lgraph_server -c lgraph_standalone.json -d 'run'"})," 'Start TuGraph and run the following commands in sequence:"]}),"\n",(0,t.jsx)(e.pre,{children:(0,t.jsx)(e.code,{children:"python3 datax/bin/datax.py  job_actors.json\n"})}),"\n",(0,t.jsx)(e.pre,{children:(0,t.jsx)(e.code,{children:"python3 datax/bin/datax.py  job_movies.json\n"})}),"\n",(0,t.jsx)(e.pre,{children:(0,t.jsx)(e.code,{children:"python3 datax/bin/datax.py  job_roles.json\n"})}),"\n",(0,t.jsx)(e.h2,{id:"mysqls-data-imported-into-tugraph-with-datax",children:"MySQL's data imported into TuGraph with DataX"}),"\n",(0,t.jsx)(e.p,{children:"We create the following table of movies under 'test' database"}),"\n",(0,t.jsx)(e.pre,{children:(0,t.jsx)(e.code,{className:"language-sql",children:"CREATE TABLE `movies` (\n  `mid`  varchar(200) NOT NULL,\n  `name` varchar(100) NOT NULL,\n  `year` int(11) NOT NULL,\n  `rate` float(5,2) unsigned NOT NULL,\n  PRIMARY KEY (`mid`)\n);\n"})}),"\n",(0,t.jsx)(e.p,{children:"Insert some data into the table"}),"\n",(0,t.jsx)(e.pre,{children:(0,t.jsx)(e.code,{className:"language-sql",children:"insert into\ntest.movies (mid, name, year, rate)\nvalues\n('tt0188766', 'King of Comedy', 1999, 7.3),\n('tt0286112', 'Shaolin Soccer', 2001, 7.3),\n('tt4701660', 'The Mermaid',   2016,  6.3);\n"})}),"\n",(0,t.jsx)(e.p,{children:"Create a DataX job configuration file"}),"\n",(0,t.jsx)(e.p,{children:(0,t.jsx)(e.code,{children:"job_mysql_to_tugraph.json"})}),"\n",(0,t.jsx)(e.p,{children:(0,t.jsx)(e.strong,{children:"Configuring Field"})}),"\n",(0,t.jsx)(e.pre,{children:(0,t.jsx)(e.code,{className:"language-json",children:'{\n  "job": {\n    "setting": {\n      "speed": {\n        "channel": 1\n      }\n    },\n    "content": [\n      {\n        "reader": {\n          "name": "mysqlreader",\n          "parameter": {\n            "username": "root",\n            "password": "root",\n            "column": ["mid", "name", "year", "rate"],\n            "splitPk": "mid",\n            "connection": [\n              {\n                "table": ["movies"],\n                "jdbcUrl": ["jdbc:mysql://127.0.0.1:3306/test?useSSL=false"]\n              }\n            ]\n          }\n        },\n        "writer": {\n          "name": "tugraphwriter",\n          "parameter": {\n            "host": "127.0.0.1",\n            "port": 7071,\n            "username": "admin",\n            "password": "73@TuGraph",\n            "graphName": "default",\n            "schema": [\n              {\n                "label": "movie",\n                "type": "VERTEX",\n                "properties": [\n                  { "name": "mid", "type": "STRING" },\n                  { "name": "name", "type": "STRING" },\n                  { "name": "year", "type": "STRING" },\n                  { "name": "rate", "type": "FLOAT", "optional": true }\n                ],\n                "primary": "mid"\n              }\n            ],\n            "files": [\n              {\n                "label": "movie",\n                "format": "JSON",\n                "columns": ["mid", "name", "year", "rate"]\n              }\n            ]\n          }\n        }\n      }\n    ]\n  }\n}\n'})}),"\n",(0,t.jsx)(e.p,{children:(0,t.jsx)(e.strong,{children:"Write simple sql"})}),"\n",(0,t.jsx)(e.pre,{children:(0,t.jsx)(e.code,{className:"language-json",children:'{\n  "job": {\n    "setting": {\n      "speed": {\n        "channel": 1\n      }\n    },\n    "content": [\n      {\n        "reader": {\n          "name": "mysqlreader",\n          "parameter": {\n            "username": "root",\n            "password": "root",\n            "connection": [\n              {\n                "querySql": [\n                  "select mid, name, year, rate from test.movies where year > 2000;"\n                ],\n                "jdbcUrl": ["jdbc:mysql://127.0.0.1:3306/test?useSSL=false"]\n              }\n            ]\n          }\n        },\n        "writer": {\n          "name": "tugraphwriter",\n          "parameter": {\n            "host": "127.0.0.1",\n            "port": 7071,\n            "username": "admin",\n            "password": "73@TuGraph",\n            "graphName": "default",\n            "schema": [\n              {\n                "label": "movie",\n                "type": "VERTEX",\n                "properties": [\n                  { "name": "mid", "type": "STRING" },\n                  { "name": "name", "type": "STRING" },\n                  { "name": "year", "type": "STRING" },\n                  { "name": "rate", "type": "FLOAT", "optional": true }\n                ],\n                "primary": "mid"\n              }\n            ],\n            "files": [\n              {\n                "label": "movie",\n                "format": "JSON",\n                "columns": ["mid", "name", "year", "rate"]\n              }\n            ]\n          }\n        }\n      }\n    ]\n  }\n}\n'})}),"\n",(0,t.jsxs)(e.p,{children:[(0,t.jsx)(e.code,{children:"./lgraph_server -c lgraph_standalone.json -d 'run'"})," Start TuGraph and run the following command\uff1a"]}),"\n",(0,t.jsx)(e.pre,{children:(0,t.jsx)(e.code,{className:"language-shell",children:"python3 datax/bin/datax.py  job_mysql_to_tugraph.json\n"})})]})}function p(n={}){const{wrapper:e}={...(0,r.R)(),...n.components};return e?(0,t.jsx)(e,{...n,children:(0,t.jsx)(c,{...n})}):c(n)}}}]);