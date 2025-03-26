"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[84488],{28453:(e,n,t)=>{t.d(n,{R:()=>r,x:()=>l});var a=t(96540);const i={},o=a.createContext(i);function r(e){const n=a.useContext(o);return a.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function l(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(i):e.components||i:r(e.components),a.createElement(o.Provider,{value:n},e.children)}},83126:(e,n,t)=>{t.r(n),t.d(n,{assets:()=>s,contentTitle:()=>r,default:()=>u,frontMatter:()=>o,metadata:()=>l,toc:()=>d});var a=t(74848),i=t(28453);const o={},r="Local Package Deployment",l={id:"installation&running/local-package-deployment",title:"Local Package Deployment",description:"This document describes TuGraph Local Package Deployment.",source:"@site/versions/version-4.3.1/en-US/source/5.installation&running/4.local-package-deployment.md",sourceDirName:"5.installation&running",slug:"/installation&running/local-package-deployment",permalink:"/tugraph-db/en/4.3.1/installation&running/local-package-deployment",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:4,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Docker Deployment",permalink:"/tugraph-db/en/4.3.1/installation&running/docker-deployment"},next:{title:"Cloud Deployment",permalink:"/tugraph-db/en/4.3.1/installation&running/cloud-deployment"}},s={},d=[{value:"1. Environment preparation",id:"1-environment-preparation",level:2},{value:"2. Download the installation package",id:"2-download-the-installation-package",level:2},{value:"3. Installation method under CentOS",id:"3-installation-method-under-centos",level:2},{value:"4. Installation method under Ubuntu",id:"4-installation-method-under-ubuntu",level:2}];function c(e){const n={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",header:"header",p:"p",pre:"pre",...(0,i.R)(),...e.components};return(0,a.jsxs)(a.Fragment,{children:[(0,a.jsx)(n.header,{children:(0,a.jsx)(n.h1,{id:"local-package-deployment",children:"Local Package Deployment"})}),"\n",(0,a.jsxs)(n.blockquote,{children:["\n",(0,a.jsx)(n.p,{children:"This document describes TuGraph Local Package Deployment."}),"\n"]}),"\n",(0,a.jsx)(n.h2,{id:"1-environment-preparation",children:"1. Environment preparation"}),"\n",(0,a.jsx)(n.p,{children:"TuGraph local package deployment requires a corresponding environment, and quick verification can use streamlined installation packages, which require almost no third-party libraries."}),"\n",(0,a.jsx)(n.p,{children:"If you need to use the complete TuGraph function, please refer to the tugraph-db source code directory ci/images/tugraph-runtime-*-Dockerfile. This script contains the complete environment construction process."}),"\n",(0,a.jsx)(n.h2,{id:"2-download-the-installation-package",children:"2. Download the installation package"}),"\n",(0,a.jsxs)(n.p,{children:['For the latest version of the installation package address, see "TuGraph-Latest-Version" of ',(0,a.jsx)(n.a,{href:"../../1.guide.md",children:"Guide"})]}),"\n",(0,a.jsxs)(n.p,{children:["You can also visit Github to download: ",(0,a.jsx)(n.a,{href:"https://github.com/TuGraph-family/tugraph-db/releases",children:"TuGraph Release"})]}),"\n",(0,a.jsx)(n.h2,{id:"3-installation-method-under-centos",children:"3. Installation method under CentOS"}),"\n",(0,a.jsx)(n.p,{children:"The .rpm installation package for TuGraph installed on CentOS, which contains the TuGraph executable file as well as header files and related library files required for writing embedded programs and stored procedures."}),"\n",(0,a.jsx)(n.p,{children:"Use the downloaded `tugraph_x.y.z.rpm installation package to install it in the terminal. You only need to run the following command:"}),"\n",(0,a.jsx)(n.pre,{children:(0,a.jsx)(n.code,{className:"language-shell",children:"$ rpm -ivh tugraph-x.y.z.rpm\n"})}),"\n",(0,a.jsxs)(n.p,{children:["Users can also specify the installation directory by specifying the ",(0,a.jsx)(n.code,{children:"--prefix"})," option."]}),"\n",(0,a.jsx)(n.h2,{id:"4-installation-method-under-ubuntu",children:"4. Installation method under Ubuntu"}),"\n",(0,a.jsx)(n.p,{children:"A .deb installation package for TuGraph installed on Ubuntu, which contains the TuGraph executable file as well as header files and related library files required for writing embedded programs and stored procedures."}),"\n",(0,a.jsxs)(n.p,{children:["Use the downloaded ",(0,a.jsx)(n.code,{children:"tugraph_x.y.z.deb"})," installation package to install it in the terminal. You only need to run the following command:"]}),"\n",(0,a.jsx)(n.pre,{children:(0,a.jsx)(n.code,{className:"language-shell",children:"$ sudo dpkg -i tugraph-x.y.z.deb\n"})}),"\n",(0,a.jsxs)(n.p,{children:["This command installs TuGraph in the ",(0,a.jsx)(n.code,{children:"/usr/local"})," directory by default. Users can also change the installation directory by specifying the ",(0,a.jsx)(n.code,{children:"--instdir=<directory>"})," option."]})]})}function u(e={}){const{wrapper:n}={...(0,i.R)(),...e.components};return n?(0,a.jsx)(n,{...e,children:(0,a.jsx)(c,{...e})}):c(e)}}}]);