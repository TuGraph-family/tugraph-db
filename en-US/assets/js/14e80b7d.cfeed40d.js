"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[22220],{28453:(e,n,i)=>{i.d(n,{R:()=>t,x:()=>s});var r=i(96540);const a={},o=r.createContext(a);function t(e){const n=r.useContext(o);return r.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function s(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(a):e.components||a:t(e.components),r.createElement(o.Provider,{value:n},e.children)}},44186:(e,n,i)=>{i.r(n),i.d(n,{assets:()=>c,contentTitle:()=>t,default:()=>h,frontMatter:()=>o,metadata:()=>s,toc:()=>l});var r=i(74848),a=i(28453);const o={},t="Docker Deployment",s={id:"installation&running/docker-deployment",title:"Docker Deployment",description:"This document introduces the creation and download of Docker images for TuGraph Compile and TuGraph Runtime.",source:"@site/versions/version-4.3.2/en-US/source/5.installation&running/3.docker-deployment.md",sourceDirName:"5.installation&running",slug:"/installation&running/docker-deployment",permalink:"/tugraph-db/en-US/en/4.3.2/installation&running/docker-deployment",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:3,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Environment Mode",permalink:"/tugraph-db/en-US/en/4.3.2/installation&running/environment-mode"},next:{title:"Local Package Deployment",permalink:"/tugraph-db/en-US/en/4.3.2/installation&running/local-package-deployment"}},c={},l=[{value:"1.Introduction",id:"1introduction",level:2},{value:"2.Existing Docker Images",id:"2existing-docker-images",level:2},{value:"2.1.Image Download",id:"21image-download",level:3},{value:"2.2.Naming Convention",id:"22naming-convention",level:3},{value:"2.2.1.TuGraph Compile Image",id:"221tugraph-compile-image",level:4},{value:"2.2.2.TuGraph Runtime Image",id:"222tugraph-runtime-image",level:4},{value:"2.2.3.TuGraph Mini Runtime Image",id:"223tugraph-mini-runtime-image",level:4},{value:"2.3.Common Docker Operations",id:"23common-docker-operations",level:3},{value:"2.4. M1 Supports",id:"24-m1-supports",level:3},{value:"2.5. Running service",id:"25-running-service",level:3},{value:"3. Best Practice in Using and Developing Docker Images for TuGraph-DB",id:"3-best-practice-in-using-and-developing-docker-images-for-tugraph-db",level:2}];function d(e){const n={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",h4:"h4",header:"header",li:"li",ol:"ol",p:"p",pre:"pre",ul:"ul",...(0,a.R)(),...e.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(n.header,{children:(0,r.jsx)(n.h1,{id:"docker-deployment",children:"Docker Deployment"})}),"\n",(0,r.jsxs)(n.blockquote,{children:["\n",(0,r.jsx)(n.p,{children:"This document introduces the creation and download of Docker images for TuGraph Compile and TuGraph Runtime."}),"\n"]}),"\n",(0,r.jsx)(n.h2,{id:"1introduction",children:"1.Introduction"}),"\n",(0,r.jsxs)(n.ul,{children:["\n",(0,r.jsx)(n.li,{children:"TuGraph Compile Image: Provides a compilation environment and can be used for TuGraph compilation and testing."}),"\n",(0,r.jsx)(n.li,{children:"TuGraph Runtime Image: Provides a binary executable environment with TuGraph library and executable files."}),"\n",(0,r.jsx)(n.li,{children:"TuGraph Mini Runtime Image: Provides a binary executable environment without Java and Python functions in TuGraph, no C++ plugin compilation and execution, only so upload."}),"\n"]}),"\n",(0,r.jsx)(n.h2,{id:"2existing-docker-images",children:"2.Existing Docker Images"}),"\n",(0,r.jsx)(n.h3,{id:"21image-download",children:"2.1.Image Download"}),"\n",(0,r.jsxs)(n.p,{children:["The images are hosted on ",(0,r.jsx)(n.a,{href:"https://hub.docker.com/u/tugraph",children:"DockerHub"})," and can be downloaded and used directly."]}),"\n",(0,r.jsxs)(n.p,{children:["For the latest version of Docker address, see the TuGraph-Latest-Version section in ",(0,r.jsx)(n.a,{href:"../../1.guide.md",children:"Guide"}),"."]}),"\n",(0,r.jsx)(n.h3,{id:"22naming-convention",children:"2.2.Naming Convention"}),"\n",(0,r.jsx)(n.h4,{id:"221tugraph-compile-image",children:"2.2.1.TuGraph Compile Image"}),"\n",(0,r.jsx)(n.p,{children:"Provides a compilation environment and can be used for TuGraph compilation."}),"\n",(0,r.jsx)(n.p,{children:(0,r.jsx)(n.code,{children:"tugraph/tugraph-compile-[os name & version]:[tugraph compile version]"})}),"\n",(0,r.jsxs)(n.p,{children:["For example: ",(0,r.jsx)(n.code,{children:"tugraph/tugraph-compile-centos7:1.2.0"})]}),"\n",(0,r.jsx)(n.h4,{id:"222tugraph-runtime-image",children:"2.2.2.TuGraph Runtime Image"}),"\n",(0,r.jsx)(n.p,{children:"Provides a binary executable environment with TuGraph library and executable files."}),"\n",(0,r.jsx)(n.p,{children:(0,r.jsx)(n.code,{children:"tugraph/tugraph-runtime-[os name & version]:[tugraph-runtime version]"})}),"\n",(0,r.jsxs)(n.p,{children:["For example:",(0,r.jsx)(n.code,{children:"tugraph/tugraph-runtime-centos7:3.4.0"})]}),"\n",(0,r.jsx)(n.h4,{id:"223tugraph-mini-runtime-image",children:"2.2.3.TuGraph Mini Runtime Image"}),"\n",(0,r.jsx)(n.p,{children:"Provides a binary executable environment without Java and Python functions in TuGraph, no C++ plugin compilation and execution, only so upload."}),"\n",(0,r.jsx)(n.p,{children:(0,r.jsx)(n.code,{children:"tugraph/tugraph-mini-runtime-[os name & version]:[tugraph-runtime version]"})}),"\n",(0,r.jsxs)(n.p,{children:["For example: ",(0,r.jsx)(n.code,{children:"tugraph/tugraph-mini-runtime-centos7:3.4.0"})]}),"\n",(0,r.jsx)(n.h3,{id:"23common-docker-operations",children:"2.3.Common Docker Operations"}),"\n",(0,r.jsxs)(n.p,{children:["Docker is generated from Dockerfile. Note that creating images requires downloading dependencies, so network issues may cause slow creation or creation failure. Do not overwrite images unless the tag is ",(0,r.jsx)(n.code,{children:"latest"}),"."]}),"\n",(0,r.jsx)(n.p,{children:"build Compile image"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-bash",children:"docker build -f tugraph-compile-centos7-Dockerfile -t tugraph/tugraph-compile-centos7:1.2.0 .\n"})}),"\n",(0,r.jsx)(n.p,{children:"build Runtime / Mini Runtime image"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-bash",children:'docker build --build-arg FILEPATH="${rpm_path_in_oss}" --build-arg FILENAME="${rpm_name}" -f tugraph-compile-centos7-Dockerfile -t tugraph/tugraph-runtime-centos7:1.2.0 .\n'})}),"\n",(0,r.jsx)(n.p,{children:"Modify image name"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-bash",children:"docker tag ${image_name}:${image_tag} tugraph/tugraph-runtime-centos7:3.3.0\n"})}),"\n",(0,r.jsx)(n.p,{children:"push image"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-bash",children:"docker push tugraph/tugraph-compile-centos7:1.2.0 .\n"})}),"\n",(0,r.jsx)(n.p,{children:"pull image"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-bash",children:"docker pull tugraph/tugraph-compile-centos7:1.2.0\n"})}),"\n",(0,r.jsx)(n.p,{children:"save image"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-bash",children:"docker save ${image_name}:${image_tag} | gzip > lgraph_latest.tar.gz\n"})}),"\n",(0,r.jsx)(n.p,{children:"load image"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-bash",children:"docker load --input lgraph_latest.tar.gz\n"})}),"\n",(0,r.jsxs)(n.p,{children:["Refer to the ",(0,r.jsx)(n.a,{href:"https://docs.docker.com/engine/reference/commandline/cli",children:"docker official documentation"})," for other Docker operations."]}),"\n",(0,r.jsx)(n.h3,{id:"24-m1-supports",children:"2.4. M1 Supports"}),"\n",(0,r.jsx)(n.p,{children:"Running amd64 containers on M1-chip machines may cause unknown errors. TuGraph provides arm64 images for M1 machines. Contains compile and runtime images."}),"\n",(0,r.jsxs)(n.p,{children:["In ",(0,r.jsx)(n.code,{children:"tugraph-runtime-centos7:3.6.0"})," and ",(0,r.jsx)(n.code,{children:"tugraph-compile-centos7:1.2.7"})," and later, ",(0,r.jsx)(n.code,{children:"tugraph-runtime-centos7"})," and ",(0,r.jsx)(n.code,{children:"tugraph-compile-centos7"})," provide images of two architectures: linux/amd64 and linux/arm64/v8. You can obtain the arm64 architecture image through docker pull on the M1 machine."]}),"\n",(0,r.jsx)(n.h3,{id:"25-running-service",children:"2.5. Running service"}),"\n",(0,r.jsxs)(n.ol,{children:["\n",(0,r.jsxs)(n.li,{children:["\n",(0,r.jsx)(n.p,{children:"Pull the docker images"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-shell",children:"docker pull tugraph/tugraph-runtime-centos7:${VERSION}\n"})}),"\n"]}),"\n",(0,r.jsxs)(n.li,{children:["\n",(0,r.jsx)(n.p,{children:"Start docker"}),"\n",(0,r.jsx)(n.pre,{children:(0,r.jsx)(n.code,{className:"language-shell",children:" docker run -it -d -p 7001:7001 -p 7070:7070 -p 7687:7687 -p 8000:8000 -p 9090:9090 \\\n -v /root/tugraph/data:/var/lib/lgraph/data  -v /root/tugraph/log:/var/log/lgraph_log \\\n --name tugraph_demo tugraph/tugraph-runtime-centos7:${VERSION} /bin/bash\n\ndocker exec -d tugraph_demo bash /setup.sh\n# 8000 is default http port\uff0cfor tugraph-db-browser accessing.\n# 7070 is default http port\uff0cfor legacy tugraph-web accessing.\n# 7687 is bolt port, for neo4j client accessing.\n# 9090 is default RPC port\uff0cfor RPC client accessing.\n\n# The default data directory is /var/lib/lgraph/data and the default log directory is /var/log/lgraph_log,\n# which is configured in the tugraph configuration file at /usr/local/etc/lgraph.json inside the docker.\n# This command mounts the data directory and log directory to /root/tugraph/ on the host for persistence,\n# which you can modify according to your actual situation.\n"})}),"\n"]}),"\n"]}),"\n",(0,r.jsx)(n.h2,{id:"3-best-practice-in-using-and-developing-docker-images-for-tugraph-db",children:"3. Best Practice in Using and Developing Docker Images for TuGraph-DB"}),"\n",(0,r.jsx)(n.p,{children:"Please carefully read the points below and follow them if you are new to contribute to TuGraph."}),"\n",(0,r.jsxs)(n.ul,{children:["\n",(0,r.jsx)(n.li,{children:"To get rid of too much docker layer, please add your dependency to the Docker with the style writing ENV and RUN in a\nsingle line."}),"\n",(0,r.jsx)(n.li,{children:"For the packages/resources you need to build the dependency, use the original ones instead of tweaking them without\ntracking by VCS. And then contact TuGraph team to upload it to OSS to accelerate the building process, as the urls you\nsee in the Dockerfiles."}),"\n",(0,r.jsx)(n.li,{children:"To make the development more efficient, you would better start from adding dependencies to base TuGraph Compile Images\nand then reproduce the process in the Dockerfile after confirming the enviroment works."}),"\n",(0,r.jsx)(n.li,{children:"CI uses the docker images. Please check the dependency problems if your CI fails."}),"\n"]})]})}function h(e={}){const{wrapper:n}={...(0,a.R)(),...e.components};return n?(0,r.jsx)(n,{...e,children:(0,r.jsx)(d,{...e})}):d(e)}}}]);