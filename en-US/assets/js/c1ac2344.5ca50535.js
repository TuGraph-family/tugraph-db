"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[67338],{28453:(e,n,r)=>{r.d(n,{R:()=>c,x:()=>h});var i=r(96540);const l={},a=i.createContext(l);function c(e){const n=i.useContext(a);return i.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function h(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(l):e.components||l:c(e.components),i.createElement(a.Provider,{value:n},e.children)}},29555:(e,n,r)=>{r.r(n),r.d(n,{assets:()=>s,contentTitle:()=>c,default:()=>d,frontMatter:()=>a,metadata:()=>h,toc:()=>t});var i=r(74848),l=r(28453);const a={},c="Docker\u90e8\u7f72",h={id:"installation&running/docker-deployment",title:"Docker\u90e8\u7f72",description:"\u672c\u6587\u6863\u4ecb\u7ecdTuGraph Compile\u53caTuGraph Runtime\u7684Docker\u955c\u50cf\u7684\u521b\u5efa\u3001\u4e0b\u8f7d\u3002",source:"@site/versions/version-4.5.1/zh-CN/source/5.installation&running/3.docker-deployment.md",sourceDirName:"5.installation&running",slug:"/installation&running/docker-deployment",permalink:"/tugraph-db/en-US/zh/4.5.1/installation&running/docker-deployment",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:3,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"\u73af\u5883\u5206\u7c7b",permalink:"/tugraph-db/en-US/zh/4.5.1/installation&running/environment-mode"},next:{title:"\u672c\u5730\u5305\u90e8\u7f72",permalink:"/tugraph-db/en-US/zh/4.5.1/installation&running/local-package-deployment"}},s={},t=[{value:"1.\u7b80\u4ecb",id:"1\u7b80\u4ecb",level:2},{value:"2.\u73b0\u6709Docker Image",id:"2\u73b0\u6709docker-image",level:2},{value:"2.1.\u955c\u50cf\u4e0b\u8f7d\u65b9\u5f0f",id:"21\u955c\u50cf\u4e0b\u8f7d\u65b9\u5f0f",level:3},{value:"2.2.\u547d\u540d\u89c4\u8303",id:"22\u547d\u540d\u89c4\u8303",level:3},{value:"2.2.1.TuGraph Compile Image",id:"221tugraph-compile-image",level:4},{value:"2.2.2.TuGraph Runtime Image",id:"222tugraph-runtime-image",level:4},{value:"2.2.3.TuGraph Mini Runtime Image",id:"223tugraph-mini-runtime-image",level:4},{value:"2.3.\u5e38\u89c1Docker\u64cd\u4f5c",id:"23\u5e38\u89c1docker\u64cd\u4f5c",level:3},{value:"2.4. M1\u82af\u7247\u652f\u6301",id:"24-m1\u82af\u7247\u652f\u6301",level:3},{value:"2.5. \u8fd0\u884c\u670d\u52a1",id:"25-\u8fd0\u884c\u670d\u52a1",level:3},{value:"3. \u4f7f\u7528\u548c\u5f00\u53d1TuGraph-DB Docker\u955c\u50cf\u65f6\u7684\u6700\u4f73\u5b9e\u8df5",id:"3-\u4f7f\u7528\u548c\u5f00\u53d1tugraph-db-docker\u955c\u50cf\u65f6\u7684\u6700\u4f73\u5b9e\u8df5",level:2}];function o(e){const n={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",h4:"h4",header:"header",li:"li",ol:"ol",p:"p",pre:"pre",ul:"ul",...(0,l.R)(),...e.components};return(0,i.jsxs)(i.Fragment,{children:[(0,i.jsx)(n.header,{children:(0,i.jsx)(n.h1,{id:"docker\u90e8\u7f72",children:"Docker\u90e8\u7f72"})}),"\n",(0,i.jsxs)(n.blockquote,{children:["\n",(0,i.jsx)(n.p,{children:"\u672c\u6587\u6863\u4ecb\u7ecdTuGraph Compile\u53caTuGraph Runtime\u7684Docker\u955c\u50cf\u7684\u521b\u5efa\u3001\u4e0b\u8f7d\u3002"}),"\n"]}),"\n",(0,i.jsx)(n.h2,{id:"1\u7b80\u4ecb",children:"1.\u7b80\u4ecb"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"TuGraph Compile Image\uff1a\u63d0\u4f9b\u7f16\u8bd1\u73af\u5883\uff0c\u53ef\u4ee5\u7528\u4e8eTuGraph\u7684\u7f16\u8bd1\uff0c\u6d4b\u8bd5\uff1b"}),"\n",(0,i.jsx)(n.li,{children:"TuGraph Runtime Image\uff1a\u63d0\u4f9b\u4e8c\u8fdb\u5236\u53ef\u8fd0\u884c\u73af\u5883\uff0c\u9644\u5e26TuGraph\u5e93\u548c\u53ef\u6267\u884c\u6587\u4ef6\uff1b"}),"\n",(0,i.jsx)(n.li,{children:"TuGraph Mini Runtime Image: \u63d0\u4f9b\u4e8c\u8fdb\u5236\u53ef\u8fd0\u884c\u73af\u5883\uff0c\u4e0d\u5305\u542bTuGraph\u4e2dJava\u3001Python\u76f8\u5173\u7684\u529f\u80fd\uff0c\u65e0C++ plugin\u7f16\u8bd1\u8fd0\u884c\uff0c\u4ec5so\u4e0a\u4f20\u3002"}),"\n"]}),"\n",(0,i.jsx)(n.h2,{id:"2\u73b0\u6709docker-image",children:"2.\u73b0\u6709Docker Image"}),"\n",(0,i.jsx)(n.h3,{id:"21\u955c\u50cf\u4e0b\u8f7d\u65b9\u5f0f",children:"2.1.\u955c\u50cf\u4e0b\u8f7d\u65b9\u5f0f"}),"\n",(0,i.jsxs)(n.p,{children:["\u955c\u50cf\u6258\u7ba1\u5728",(0,i.jsx)(n.a,{href:"https://hub.docker.com/u/tugraph",children:"DockerHub"}),"\uff0c\u53ef\u76f4\u63a5\u4e0b\u8f7d\u4f7f\u7528\u3002"]}),"\n",(0,i.jsxs)(n.p,{children:["\u6700\u65b0\u7248\u672c\u7684Docker\u5730\u5740\u53c2\u89c1 ",(0,i.jsx)(n.a,{href:"../../1.guide.md",children:"\u6587\u6863\u5730\u56fe"}),'\u7684"TuGraph\u6700\u65b0\u7248\u672c"\u7ae0\u8282\u3002']}),"\n",(0,i.jsx)(n.h3,{id:"22\u547d\u540d\u89c4\u8303",children:"2.2.\u547d\u540d\u89c4\u8303"}),"\n",(0,i.jsx)(n.h4,{id:"221tugraph-compile-image",children:"2.2.1.TuGraph Compile Image"}),"\n",(0,i.jsx)(n.p,{children:"\u63d0\u4f9b\u7f16\u8bd1\u73af\u5883\uff0c\u53ef\u4ee5\u7528\u4e8eTuGraph\u7684\u7f16\u8bd1\u3002"}),"\n",(0,i.jsx)(n.p,{children:(0,i.jsx)(n.code,{children:"tugraph/tugraph-compile-[os name & version]:[tugraph compile version]"})}),"\n",(0,i.jsxs)(n.p,{children:["\u4f8b\u5982\uff1a ",(0,i.jsx)(n.code,{children:"tugraph/tugraph-compile-centos7:1.2.0"})]}),"\n",(0,i.jsx)(n.h4,{id:"222tugraph-runtime-image",children:"2.2.2.TuGraph Runtime Image"}),"\n",(0,i.jsx)(n.p,{children:"\u63d0\u4f9b\u4e8c\u8fdb\u5236\u53ef\u8fd0\u884c\u73af\u5883\uff0c\u9644\u5e26TuGraph\u5e93\u548c\u53ef\u6267\u884c\u6587\u4ef6\u3002"}),"\n",(0,i.jsx)(n.p,{children:(0,i.jsx)(n.code,{children:"tugraph/tugraph-runtime-[os name & version]:[tugraph-runtime version]"})}),"\n",(0,i.jsxs)(n.p,{children:["\u4f8b\u5982\uff1a",(0,i.jsx)(n.code,{children:"tugraph/tugraph-runtime-centos7:3.4.0"})]}),"\n",(0,i.jsx)(n.h4,{id:"223tugraph-mini-runtime-image",children:"2.2.3.TuGraph Mini Runtime Image"}),"\n",(0,i.jsx)(n.p,{children:"\u63d0\u4f9b\u4e8c\u8fdb\u5236\u53ef\u8fd0\u884c\u73af\u5883\uff0c\u4e0d\u5305\u542bTuGraph\u79cdJava\u3001Python\u76f8\u5173\u7684\u529f\u80fd\uff0c\u65e0C++ plugin\u7f16\u8bd1\u8fd0\u884c\uff0c\u4ec5so\u4e0a\u4f20\u3002"}),"\n",(0,i.jsx)(n.p,{children:(0,i.jsx)(n.code,{children:"tugraph/tugraph-mini-runtime-[os name & version]:[tugraph-runtime version]"})}),"\n",(0,i.jsxs)(n.p,{children:["\u4f8b\u5982\uff1a ",(0,i.jsx)(n.code,{children:"tugraph/tugraph-mini-runtime-centos7:3.4.0"})]}),"\n",(0,i.jsx)(n.h3,{id:"23\u5e38\u89c1docker\u64cd\u4f5c",children:"2.3.\u5e38\u89c1Docker\u64cd\u4f5c"}),"\n",(0,i.jsxs)(n.p,{children:["Docker\u7531Dockerfile\u751f\u6210\uff0c\u6ce8\u610f\u521b\u5efa\u955c\u50cf\u9700\u8981\u4e0b\u8f7d\u4f9d\u8d56\uff0c\u56e0\u6b64\u7f51\u7edc\u95ee\u9898\u53ef\u80fd\u4f1a\u5bfc\u81f4\u521b\u5efa\u8f83\u6162\u6216\u8005\u521b\u5efa\u5931\u8d25\u3002\u6ce8\u610f\u4e0d\u8981\u8986\u76d6\u955c\u50cf\uff0c\u9664\u975etag\u4e3a ",(0,i.jsx)(n.code,{children:"latest"}),"\u3002"]}),"\n",(0,i.jsx)(n.p,{children:"\u521b\u5efaCompile\u955c\u50cf"}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-bash",children:"docker build -f tugraph-compile-centos7-Dockerfile -t tugraph/tugraph-compile-centos7:1.2.0 .\n"})}),"\n",(0,i.jsx)(n.p,{children:"\u521b\u5efaRuntime / Mini Runtine\u955c\u50cf"}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-bash",children:'docker build --build-arg FILEPATH="${rpm_path_in_oss}" --build-arg FILENAME="${rpm_name}" -f tugraph-compile-centos7-Dockerfile -t tugraph/tugraph-runtime-centos7:1.2.0 .\n'})}),"\n",(0,i.jsx)(n.p,{children:"\u4fee\u6539\u955c\u50cf\u540d\u79f0"}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-bash",children:"docker tag ${image_name}:${image_tag} tugraph/tugraph-runtime-centos7:3.3.0\n"})}),"\n",(0,i.jsx)(n.p,{children:"\u4e0a\u4f20\u955c\u50cf"}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-bash",children:"docker push tugraph/tugraph-compile-centos7:1.2.0 .\n"})}),"\n",(0,i.jsx)(n.p,{children:"\u83b7\u53d6\u955c\u50cf"}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-bash",children:"docker pull tugraph/tugraph-compile-centos7:1.2.0\n"})}),"\n",(0,i.jsx)(n.p,{children:"\u5bfc\u51fa\u955c\u50cf"}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-bash",children:"docker save ${image_name}:${image_tag} | gzip > lgraph_latest.tar.gz\n"})}),"\n",(0,i.jsx)(n.p,{children:"\u5bfc\u5165\u955c\u50cf"}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-bash",children:"docker load --input lgraph_latest.tar.gz\n"})}),"\n",(0,i.jsxs)(n.p,{children:["\u5176\u4ed6Docker\u64cd\u4f5c\u8bf7\u53c2\u8003",(0,i.jsx)(n.a,{href:"https://docs.docker.com/engine/reference/commandline/cli",children:"docker\u5b98\u65b9\u6587\u6863"})]}),"\n",(0,i.jsx)(n.h3,{id:"24-m1\u82af\u7247\u652f\u6301",children:"2.4. M1\u82af\u7247\u652f\u6301"}),"\n",(0,i.jsx)(n.p,{children:"\u5728 M1 \u82af\u7247\u7684\u673a\u5668\u4e0a\u8fd0\u884c amd64 \u5bb9\u5668\u53ef\u80fd\u9020\u6210\u672a\u77e5\u9519\u8bef\u3002TuGraph\u63d0\u4f9b arm64 \u7684\u955c\u50cf\u4f9b M1 \u673a\u5668\u4f7f\u7528\u3002\n\u5305\u542bcompile\u548cruntime\u4e24\u79cd\u955c\u50cf\u3002"}),"\n",(0,i.jsxs)(n.p,{children:["\u5728",(0,i.jsx)(n.code,{children:"tugraph-runtime-centos7:3.6.0"}),"\u4e0e",(0,i.jsx)(n.code,{children:"tugraph-compile-centos7:1.2.7"}),"\u53ca\u4e4b\u540e\uff0c",(0,i.jsx)(n.code,{children:"tugraph-runtime-centos7"}),"\u4e0e",(0,i.jsx)(n.code,{children:"tugraph-compile-centos7"}),"\u63d0\u4f9blinux/amd64\u548clinux/arm64/v8\u4e24\u79cd\u67b6\u6784\u7684\u955c\u50cf\uff0c\u53ef\u4ee5\u5728 M1 \u673a\u5668\u4e0a\u901a\u8fc7docker pull\u83b7\u53d6arm64\u67b6\u6784\u955c\u50cf\u3002"]}),"\n",(0,i.jsx)(n.h3,{id:"25-\u8fd0\u884c\u670d\u52a1",children:"2.5. \u8fd0\u884c\u670d\u52a1"}),"\n",(0,i.jsxs)(n.ol,{children:["\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsx)(n.p,{children:"\u62c9\u53d6\u955c\u50cf"}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-shell",children:"docker pull tugraph/tugraph-runtime-centos7:${VERSION}\n"})}),"\n"]}),"\n",(0,i.jsxs)(n.li,{children:["\n",(0,i.jsx)(n.p,{children:"\u542f\u52a8docker"}),"\n",(0,i.jsx)(n.pre,{children:(0,i.jsx)(n.code,{className:"language-shell",children:" docker run -d -p 7070:7070  -p 7687:7687 -p 9090:9090 -v /root/tugraph/data:/var/lib/lgraph/data  -v /root/tugraph/log:/var/log/lgraph_log \\\n --name tugraph_demo ${REPOSITORY}:${VERSION}\n\n# ${REPOSITORY}\u662f\u955c\u50cf\u5730\u5740\uff0c${VERSION}\u662f\u7248\u672c\u53f7\u3002\n# 7070\u662f\u9ed8\u8ba4\u7684http\u7aef\u53e3\uff0c\u8bbf\u95eetugraph-db-browser\u4f7f\u7528\u3002   \n# 7687\u662fbolt\u7aef\u53e3\uff0cbolt client\u8bbf\u95ee\u4f7f\u7528\u3002\n# 9090\u662f\u9ed8\u8ba4\u7684rpc\u7aef\u53e3\uff0crpc client\u8bbf\u95ee\u4f7f\u7528\u3002\n# /var/lib/lgraph/data\u662f\u5bb9\u5668\u5185\u7684\u9ed8\u8ba4\u6570\u636e\u76ee\u5f55\uff0c/var/log/lgraph_log\u662f\u5bb9\u5668\u5185\u7684\u9ed8\u8ba4\u65e5\u5fd7\u76ee\u5f55\n# \u547d\u4ee4\u5c06\u6570\u636e\u76ee\u5f55\u548c\u65e5\u5fd7\u76ee\u5f55\u6302\u8f7d\u5230\u4e86\u5bbf\u4e3b\u673a\u7684/root/tugraph/\u4e0a\u8fdb\u884c\u6301\u4e45\u5316\uff0c\u60a8\u53ef\u4ee5\u6839\u636e\u5b9e\u9645\u60c5\u51b5\u4fee\u6539\u3002\n"})}),"\n"]}),"\n"]}),"\n",(0,i.jsx)(n.h2,{id:"3-\u4f7f\u7528\u548c\u5f00\u53d1tugraph-db-docker\u955c\u50cf\u65f6\u7684\u6700\u4f73\u5b9e\u8df5",children:"3. \u4f7f\u7528\u548c\u5f00\u53d1TuGraph-DB Docker\u955c\u50cf\u65f6\u7684\u6700\u4f73\u5b9e\u8df5"}),"\n",(0,i.jsx)(n.p,{children:"\u5728\u60a8\u521a\u5f00\u59cb\u4e3aTuGraph\u505a\u51fa\u8d21\u732e\u65f6\uff0c\u8bf7\u4ed4\u7ec6\u9605\u8bfb\u4ee5\u4e0b\u8981\u70b9\uff0c\u5e76\u9075\u5faa\u5b83\u4eec\u3002"}),"\n",(0,i.jsxs)(n.ul,{children:["\n",(0,i.jsx)(n.li,{children:"\u4e3a\u4e86\u907f\u514d\u8fc7\u591a\u7684Docker Layer\uff0c\u8bf7\u5c3d\u91cf\u50cf\u73b0\u6709\u7684Dockerfile\u4e00\u6837\u5355\u884c\u7684\u5199ENV\u548cRUN\u5c06\u60a8\u7684\u4f9d\u8d56\u9879\u6dfb\u52a0\u5230Docker\u4e2d\u3002"}),"\n",(0,i.jsx)(n.li,{children:"\u5bf9\u4e8e\u60a8\u9700\u8981\u6784\u5efa\u4f9d\u8d56\u9879\u7684\u8f6f\u4ef6\u5305/\u8d44\u6e90\uff0c\u8bf7\u4f7f\u7528\u539f\u59cb\u7684\u8f6f\u4ef6\u5305/\u8d44\u6e90\uff0c\u800c\u4e0d\u662f\u5728\u6ca1\u6709VCS\u8ddf\u8e2a\u7684\u60c5\u51b5\u4e0b\u4fee\u6539\u8fd9\u4e9b\u8d44\u6e90\u3002\u4e4b\u540e\u8054\u7cfbTuGraph\u56e2\u961f\u5c06\u5176\u4e0a\u4f20\u5230OSS\u4ee5\u52a0\u901f\u6784\u5efa\u8fc7\u7a0b\uff0c\u5982\u540c\u60a8\u5728Dockerfiles\u4e2d\u770b\u5230\u7684URL\u4e00\u6837\u3002"}),"\n",(0,i.jsx)(n.li,{children:"\u4e3a\u4e86\u4f7f\u5f00\u53d1\u66f4\u52a0\u9ad8\u6548\uff0c\u6700\u597d\u4eceBase TuGraph\u7f16\u8bd1\u955c\u50cf\u6dfb\u52a0\u4f9d\u8d56\u9879\u5f00\u59cb\uff0c\u5b8c\u6210\u5f00\u53d1\u786e\u8ba4\u4f9d\u8d56\u6ca1\u95ee\u9898\u540e\u5728Dockerfile\u4e2d\u91cd\u65b0\u590d\u5236\u8be5\u8fc7\u7a0b\u3002"}),"\n",(0,i.jsx)(n.li,{children:"CI\u4f7f\u7528\u4e86Docker\u955c\u50cf\u3002\u5982\u679c\u60a8\u7684CI\u5931\u8d25\uff0c\u8bf7\u68c0\u67e5\u4f9d\u8d56\u9879\u95ee\u9898\u3002"}),"\n"]})]})}function d(e={}){const{wrapper:n}={...(0,l.R)(),...e.components};return n?(0,i.jsx)(n,{...e,children:(0,i.jsx)(o,{...e})}):o(e)}}}]);