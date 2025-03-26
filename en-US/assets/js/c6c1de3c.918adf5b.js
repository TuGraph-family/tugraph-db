"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[36399],{28453:(e,n,i)=>{i.d(n,{R:()=>t,x:()=>l});var a=i(96540);const r={},o=a.createContext(r);function t(e){const n=a.useContext(o);return a.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function l(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(r):e.components||r:t(e.components),a.createElement(o.Provider,{value:n},e.children)}},29815:(e,n,i)=>{i.r(n),i.d(n,{assets:()=>c,contentTitle:()=>t,default:()=>u,frontMatter:()=>o,metadata:()=>l,toc:()=>s});var a=i(74848),r=i(28453);const o={},t="Docker Deployment",l={id:"developer-manual/installation/docker-deployment",title:"Docker Deployment",description:"This document introduces the creation and download of Docker images for TuGraph Compile and TuGraph Runtime.",source:"@site/versions/version-3.6.0/en-US/source/5.developer-manual/1.installation/3.docker-deployment.md",sourceDirName:"5.developer-manual/1.installation",slug:"/developer-manual/installation/docker-deployment",permalink:"/tugraph-db/en-US/en/3.6.0/developer-manual/installation/docker-deployment",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:3,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Environment Mode",permalink:"/tugraph-db/en-US/en/3.6.0/developer-manual/installation/environment-mode"},next:{title:"Local Package Deployment",permalink:"/tugraph-db/en-US/en/3.6.0/developer-manual/installation/local-package-deployment"}},c={},s=[{value:"1.Introduction",id:"1introduction",level:2},{value:"2.Existing Docker Images",id:"2existing-docker-images",level:2},{value:"2.1.Image Download",id:"21image-download",level:3},{value:"2.2.Naming Convention",id:"22naming-convention",level:3},{value:"2.2.1.TuGraph Compile Image",id:"221tugraph-compile-image",level:4},{value:"2.2.2.TuGraph Runtime Image",id:"222tugraph-runtime-image",level:4},{value:"2.2.3.TuGraph Mini Runtime Image",id:"223tugraph-mini-runtime-image",level:4},{value:"2.3.Common Docker Operations",id:"23common-docker-operations",level:2}];function d(e){const n={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",h4:"h4",header:"header",li:"li",p:"p",pre:"pre",ul:"ul",...(0,r.R)(),...e.components};return(0,a.jsxs)(a.Fragment,{children:[(0,a.jsx)(n.header,{children:(0,a.jsx)(n.h1,{id:"docker-deployment",children:"Docker Deployment"})}),"\n",(0,a.jsxs)(n.blockquote,{children:["\n",(0,a.jsx)(n.p,{children:"This document introduces the creation and download of Docker images for TuGraph Compile and TuGraph Runtime."}),"\n"]}),"\n",(0,a.jsx)(n.h2,{id:"1introduction",children:"1.Introduction"}),"\n",(0,a.jsxs)(n.ul,{children:["\n",(0,a.jsx)(n.li,{children:"TuGraph Compile Image: Provides a compilation environment and can be used for TuGraph compilation and testing."}),"\n",(0,a.jsx)(n.li,{children:"TuGraph Runtime Image: Provides a binary executable environment with TuGraph library and executable files."}),"\n",(0,a.jsx)(n.li,{children:"TuGraph Mini Runtime Image: Provides a binary executable environment without Java and Python functions in TuGraph, no C++ plugin compilation and execution, only so upload."}),"\n"]}),"\n",(0,a.jsx)(n.h2,{id:"2existing-docker-images",children:"2.Existing Docker Images"}),"\n",(0,a.jsx)(n.h3,{id:"21image-download",children:"2.1.Image Download"}),"\n",(0,a.jsxs)(n.p,{children:["The images are hosted on ",(0,a.jsx)(n.a,{href:"https://hub.docker.com/u/tugraph",children:"DockerHub"})," and can be downloaded and used directly."]}),"\n",(0,a.jsx)(n.h3,{id:"22naming-convention",children:"2.2.Naming Convention"}),"\n",(0,a.jsx)(n.h4,{id:"221tugraph-compile-image",children:"2.2.1.TuGraph Compile Image"}),"\n",(0,a.jsx)(n.p,{children:"Provides a compilation environment and can be used for TuGraph compilation."}),"\n",(0,a.jsx)(n.p,{children:(0,a.jsx)(n.code,{children:"tugraph/tugraph-compile-[os name & version]:[tugraph compile version]"})}),"\n",(0,a.jsxs)(n.p,{children:["For example: ",(0,a.jsx)(n.code,{children:"tugraph/tugraph-compile-centos7:1.2.0"})]}),"\n",(0,a.jsx)(n.h4,{id:"222tugraph-runtime-image",children:"2.2.2.TuGraph Runtime Image"}),"\n",(0,a.jsx)(n.p,{children:"Provides a binary executable environment with TuGraph library and executable files."}),"\n",(0,a.jsx)(n.p,{children:(0,a.jsx)(n.code,{children:"tugraph/tugraph-runtime-[os name & version]:[tugraph-runtime version]"})}),"\n",(0,a.jsxs)(n.p,{children:["For example:",(0,a.jsx)(n.code,{children:"tugraph/tugraph-runtime-centos7:3.4.0"})]}),"\n",(0,a.jsx)(n.h4,{id:"223tugraph-mini-runtime-image",children:"2.2.3.TuGraph Mini Runtime Image"}),"\n",(0,a.jsx)(n.p,{children:"Provides a binary executable environment without Java and Python functions in TuGraph, no C++ plugin compilation and execution, only so upload."}),"\n",(0,a.jsx)(n.p,{children:(0,a.jsx)(n.code,{children:"tugraph/tugraph-mini-runtime-[os name & version]:[tugraph-runtime version]"})}),"\n",(0,a.jsxs)(n.p,{children:["For example: ",(0,a.jsx)(n.code,{children:"tugraph/tugraph-mini-runtime-centos7:3.4.0"})]}),"\n",(0,a.jsx)(n.h2,{id:"23common-docker-operations",children:"2.3.Common Docker Operations"}),"\n",(0,a.jsxs)(n.p,{children:["Docker is generated from Dockerfile. Note that creating images requires downloading dependencies, so network issues may cause slow creation or creation failure. Do not overwrite images unless the tag is ",(0,a.jsx)(n.code,{children:"latest"}),"."]}),"\n",(0,a.jsx)(n.p,{children:"build Compile image"}),"\n",(0,a.jsx)(n.pre,{children:(0,a.jsx)(n.code,{className:"language-bash",children:"docker build -f tugraph-compile-centos7-Dockerfile -t tugraph/tugraph-compile-centos7:1.2.0 .\n"})}),"\n",(0,a.jsx)(n.p,{children:"build Runtime / Mini Runtime image"}),"\n",(0,a.jsx)(n.pre,{children:(0,a.jsx)(n.code,{className:"language-bash",children:'docker build --build-arg FILEPATH="${rpm_path_in_oss}" --build-arg FILENAME="${rpm_name}" -f tugraph-compile-centos7-Dockerfile -t tugraph/tugraph-runtime-centos7:1.2.0 .\n'})}),"\n",(0,a.jsx)(n.p,{children:"Modify image name"}),"\n",(0,a.jsx)(n.pre,{children:(0,a.jsx)(n.code,{className:"language-bash",children:"docker tag ${image_name}:${image_tag} tugraph/tugraph-runtime-centos7:3.3.0\n"})}),"\n",(0,a.jsx)(n.p,{children:"push image"}),"\n",(0,a.jsx)(n.pre,{children:(0,a.jsx)(n.code,{className:"language-bash",children:"docker push tugraph/tugraph-compile-centos7:1.2.0 .\n"})}),"\n",(0,a.jsx)(n.p,{children:"pull image"}),"\n",(0,a.jsx)(n.pre,{children:(0,a.jsx)(n.code,{className:"language-bash",children:"docker pull tugraph/tugraph-compile-centos7:1.2.0\n"})}),"\n",(0,a.jsx)(n.p,{children:"save image"}),"\n",(0,a.jsx)(n.pre,{children:(0,a.jsx)(n.code,{className:"language-bash",children:"docker save ${image_name}:${image_tag} | gzip > lgraph_latest.tar.gz\n"})}),"\n",(0,a.jsx)(n.p,{children:"load image"}),"\n",(0,a.jsx)(n.pre,{children:(0,a.jsx)(n.code,{className:"language-bash",children:"docker load --input lgraph_latest.tar.gz\n"})}),"\n",(0,a.jsxs)(n.p,{children:["Refer to the ",(0,a.jsx)(n.a,{href:"https://docs.docker.com/engine/reference/commandline/cli",children:"docker official documentation"})," for other Docker operations."]})]})}function u(e={}){const{wrapper:n}={...(0,r.R)(),...e.components};return n?(0,a.jsx)(n,{...e,children:(0,a.jsx)(d,{...e})}):d(e)}}}]);