"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[44766],{28453:(e,r,n)=>{n.d(r,{R:()=>o,x:()=>i});var a=n(96540);const t={},s=a.createContext(t);function o(e){const r=a.useContext(s);return a.useMemo((function(){return"function"==typeof e?e(r):{...r,...e}}),[r,e])}function i(e){let r;return r=e.disableParentContext?"function"==typeof e.components?e.components(t):e.components||t:o(e.components),a.createElement(s.Provider,{value:r},e.children)}},96102:(e,r,n)=>{n.r(r),n.d(r,{assets:()=>l,contentTitle:()=>o,default:()=>p,frontMatter:()=>s,metadata:()=>i,toc:()=>h});var a=n(74848),t=n(28453);const s={},o="TuGraph Explorer",i={id:"developer-manual/ecosystem-tools/tugraph-explorer",title:"TuGraph Explorer",description:"TuGraph Explorer is strongly dependent on TuGraph, so before starting Explorer, we need to start TuGraph first.",source:"@site/versions/version-3.5.1/en-US/source/5.developer-manual/5.ecosystem-tools/3.tugraph-explorer.md",sourceDirName:"5.developer-manual/5.ecosystem-tools",slug:"/developer-manual/ecosystem-tools/tugraph-explorer",permalink:"/tugraph-db/en-US/en/3.5.1/developer-manual/ecosystem-tools/tugraph-explorer",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:3,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"TuGraph DataX",permalink:"/tugraph-db/en-US/en/3.5.1/developer-manual/ecosystem-tools/tugraph-datax"},next:{title:"Log",permalink:"/tugraph-db/en-US/en/3.5.1/developer-manual/ecosystem-tools/log"}},l={},h=[{value:"1.Introduction",id:"1introduction",level:2},{value:"2.Install TuGraph",id:"2install-tugraph",level:2},{value:"3.TuGraph operation",id:"3tugraph-operation",level:2},{value:"3.1.Start TuGraph Service",id:"31start-tugraph-service",level:3},{value:"3.2.TuGraph Browser Query",id:"32tugraph-browser-query",level:3},{value:"4.Introduction to TuGraph Explorer",id:"4introduction-to-tugraph-explorer",level:2},{value:"5.Start TuGraph Explorer",id:"5start-tugraph-explorer",level:2},{value:"6.Connect TuGraph",id:"6connect-tugraph",level:2}];function c(e){const r={a:"a",blockquote:"blockquote",br:"br",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",img:"img",li:"li",p:"p",pre:"pre",strong:"strong",ul:"ul",...(0,t.R)(),...e.components};return(0,a.jsxs)(a.Fragment,{children:[(0,a.jsx)(r.header,{children:(0,a.jsx)(r.h1,{id:"tugraph-explorer",children:"TuGraph Explorer"})}),"\n",(0,a.jsxs)(r.p,{children:["TuGraph Explorer is strongly dependent on TuGraph, so before starting Explorer, we need to start TuGraph first.\n",(0,a.jsx)(r.a,{name:"lGD6j"})]}),"\n",(0,a.jsx)(r.h2,{id:"1introduction",children:"1.Introduction"}),"\n",(0,a.jsxs)(r.p,{children:["TuGraph is a graph database independently developed by Ant Group, which provides graph database engine and graph analysis engine. Its main features are large data storage and computation, and it also supports efficient online transaction processing (OLTP) and Online analysis processing (OLAP).\n",(0,a.jsx)(r.a,{name:"BOZFL"})]}),"\n",(0,a.jsx)(r.h2,{id:"2install-tugraph",children:"2.Install TuGraph"}),"\n",(0,a.jsxs)(r.blockquote,{children:["\n",(0,a.jsx)(r.p,{children:"Refer to the official documentation () for more information."}),"\n"]}),"\n",(0,a.jsx)(r.p,{children:"TuGraph needs to be installed via Docker Image, follow these steps to install it locally:"}),"\n",(0,a.jsxs)(r.ul,{children:["\n",(0,a.jsxs)(r.li,{children:["install local Docker environment: reference",(0,a.jsx)(r.a,{href:"https://docs.docker.com/get-started/",children:"official documentation"}),";"]}),"\n"]}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-shell",children:"$ sudo docker --version\n"})}),"\n",(0,a.jsx)(r.p,{children:"If the above command can print the docker version number successfully, it indicates that the docker environment has been installed."}),"\n",(0,a.jsxs)(r.ul,{children:["\n",(0,a.jsxs)(r.li,{children:["To download TuGraph images:",(0,a.jsx)(r.a,{href:"https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/tugraph-3.3.0/TuGraph-Image-3.3.0.tar.gz",children:"Download TuGraph Image"})]}),"\n"]}),"\n",(0,a.jsx)(r.p,{children:"Currently, TuGraph provides an image file based on Ubuntu 16.04 LTS and CenterOS 7.3. The image file is a compressed file named lgraph_x.y.z.ar, where x.y.z is the version number of TuGraph."}),"\n",(0,a.jsxs)(r.ul,{children:["\n",(0,a.jsx)(r.li,{children:"Load the TuGraph image:"}),"\n"]}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-shell",children:"// lgraph_lastest.tar.gz \u662f TuGraph \u955c\u50cf\u6587\u4ef6\u540d\n$ docker import lgraph_lastest.tar.gz\n\n// After the loading is complete, a message is displayed indicating that the image has been loaded\n"})}),"\n",(0,a.jsxs)(r.ul,{children:["\n",(0,a.jsx)(r.li,{children:"Start Docker"}),"\n"]}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-shell",children:"$ docker run -d -v {host_data_dir}:/mnt -p 7090:7090 -it reg.docker.alibaba-inc.com/tugraph/tugraph:x.y.z\n$ docker exec -it {container_id} bash\n\n// host_data_dir = /Users/moyee/tugraph\n// container_id = xxx\n$ docker run -d -v /Users/moyee/tugraph:/mnt -p 7090:7090 -it reg.docker.alibaba-inc.com/tugraph/tugraph:3.1.1\n$ docker exec -it xxx bash\n\n"})}),"\n",(0,a.jsx)(r.p,{children:"Parameter Description"}),"\n",(0,a.jsxs)(r.ul,{children:["\n",(0,a.jsx)(r.li,{children:"-v volume mapping"}),"\n",(0,a.jsx)(r.li,{children:"{host_data_dir} is a directory where the user wants to store data, such as/home/user1/workspace"}),"\n",(0,a.jsx)(r.li,{children:"-p The function of Docker is port mapping. The example maps Docker's port 7090 to the local port 7090"}),"\n",(0,a.jsx)(r.li,{children:"{container_id} is the container id of Docker, which can be obtained through docker ps"}),"\n"]}),"\n",(0,a.jsx)(r.p,{children:(0,a.jsx)(r.a,{name:"LOzYE"})}),"\n",(0,a.jsx)(r.h2,{id:"3tugraph-operation",children:"3.TuGraph operation"}),"\n",(0,a.jsx)(r.p,{children:(0,a.jsx)(r.a,{name:"zLris"})}),"\n",(0,a.jsx)(r.h3,{id:"31start-tugraph-service",children:"3.1.Start TuGraph Service"}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-shell",children:"$ lgraph_server --license /mnt/fma.lic --config ~/demo/movie/lgraph.json\n"})}),"\n",(0,a.jsxs)(r.ul,{children:["\n",(0,a.jsx)(r.li,{children:"fma.lic is the authorization file, should be placed in {host_data_dir} folder, mapped to the /mnt directory of docker"}),"\n",(0,a.jsx)(r.li,{children:"lgraph.json is the configuration file for TuGraph"}),"\n"]}),"\n",(0,a.jsx)(r.p,{children:(0,a.jsx)(r.a,{name:"OUx1A"})}),"\n",(0,a.jsx)(r.h3,{id:"32tugraph-browser-query",children:"3.2.TuGraph Browser Query"}),"\n",(0,a.jsxs)(r.p,{children:["TuGraph Browser Is a visual query tool provided by TuGraph. Users can open the browser, type {IP}:{Port}, enter the default username by 'admin', password by '73@TuGraph' to complete the login. Enter the TuGraph Query page after successful login.",(0,a.jsx)(r.br,{}),(0,a.jsx)(r.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/%E6%96%87%E6%A1%A3/2.Operating/7.tugraph-browser-query-01.png",alt:"image.png"}),"\n",(0,a.jsx)(r.a,{name:"wGOCA"})]}),"\n",(0,a.jsx)(r.h2,{id:"4introduction-to-tugraph-explorer",children:"4.Introduction to TuGraph Explorer"}),"\n",(0,a.jsx)(r.p,{children:"TuGraph Explorer is a GraphInsight based visual graph analysis platform that provides complete graph exploration and analysis capabilities to help users gain valuable insights from massive graph data."}),"\n",(0,a.jsx)(r.p,{children:(0,a.jsx)(r.a,{name:"uw3UH"})}),"\n",(0,a.jsx)(r.h2,{id:"5start-tugraph-explorer",children:"5.Start TuGraph Explorer"}),"\n",(0,a.jsx)(r.p,{children:"Once TuGraph is installed successfully, you can start installing TuGraph Explorer."}),"\n",(0,a.jsxs)(r.ul,{children:["\n",(0,a.jsx)(r.li,{children:"Load TuGraph Explorer image\uff1a"}),"\n"]}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-shell",children:"// lgraph_lastest.tar.gz TuGraph image file name\n$ docker import tugraph_explore.tar.gz\n\n// After the loading is complete, a message is displayed indicating that the image has been loaded\n"})}),"\n",(0,a.jsxs)(r.ul,{children:["\n",(0,a.jsx)(r.li,{children:"Start Docker"}),"\n"]}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-shell",children:"$ docker run -d -p 7091:7091 -it reg.docker.alibaba-inc.com/tugraph-explore:1.0.1\n$ docker exec -it {container_id} bash\n"})}),"\n",(0,a.jsx)(r.p,{children:"Parameter Description:"}),"\n",(0,a.jsxs)(r.ul,{children:["\n",(0,a.jsxs)(r.li,{children:["\n",(0,a.jsx)(r.p,{children:"-p The function of Docker is port mapping. In this example, Docker port 7091 is mapped to local port 7091"}),"\n"]}),"\n",(0,a.jsxs)(r.li,{children:["\n",(0,a.jsx)(r.p,{children:"{container_id} is the id of a Docker container, which can be obtained through docker ps"}),"\n"]}),"\n",(0,a.jsxs)(r.li,{children:["\n",(0,a.jsx)(r.p,{children:"Start TuGraph Explorer"}),"\n"]}),"\n"]}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{className:"language-shell",children:"$ cd /usr/src/tugraphexplore\n$ npm run dev -- -p 7091\n"})}),"\n",(0,a.jsxs)(r.p,{children:["After the TuGraph Explorer service started, it can be accessed through ",(0,a.jsx)(r.code,{children:"**http://localhost:7091/tugraph/explore.html**"})," \uff0cIf everything is normal, you will see the following page.",(0,a.jsx)(r.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/tugraph-expolore/tugraph-explore-index.png",alt:"image.png"})]}),"\n",(0,a.jsx)(r.h2,{id:"6connect-tugraph",children:"6.Connect TuGraph"}),"\n",(0,a.jsxs)(r.p,{children:['Once TuGraph Explorer is up, the first step is to connect to the TuGraph database. Click the "Connect" button to bring up the page for connecting to the Graph database, as shown in the image below.\n',(0,a.jsx)(r.img,{src:"https://gw.alipayobjects.com/mdn/rms_fa12c2/afts/img/A*JEUKRYMH--4AAAAAAAAAAAAAARQnAQ",alt:"image.png"})]}),"\n",(0,a.jsx)(r.p,{children:"To connect to TuGraph data, we need to provide the following information:"}),"\n",(0,a.jsxs)(r.ul,{children:["\n",(0,a.jsx)(r.li,{children:"Graph database account"}),"\n",(0,a.jsx)(r.li,{children:"Graph database password"}),"\n",(0,a.jsx)(r.li,{children:"Address of the graph database: The format is ip:port"}),"\n"]}),"\n",(0,a.jsxs)(r.p,{children:[(0,a.jsx)(r.strong,{children:"The IP address needs to be the container IP address, which can be queried by running the following command"}),"\u3002"]}),"\n",(0,a.jsx)(r.pre,{children:(0,a.jsx)(r.code,{children:"$ docker run -d -v /Users/xx/tugraph:/mnt -p 7090:7090 -it reg.docker.alibaba-inc.com/tugraph/tugraph:3.3.0\n$ docker exec -it 8408b49033bc1698(TuGraph container) bash\n$ cat /etc/hosts\n127.0.0.1\tlocalhost\n::1\tlocalhost ip6-localhost ip6-loopback\nfe00::0\tip6-localnet\nff00::0\tip6-mcastprefix\nff02::1\tip6-allnodes\nff02::2\tip6-allrouters\n172.17.0.4\t8408b543243bc69\n"})}),"\n",(0,a.jsxs)(r.p,{children:["As shown above, the address to connect the graph database should be filled in:",(0,a.jsx)(r.strong,{children:"172.17.0.4:7090"}),"\u3002"]})]})}function p(e={}){const{wrapper:r}={...(0,t.R)(),...e.components};return r?(0,a.jsx)(r,{...e,children:(0,a.jsx)(c,{...e})}):c(e)}}}]);