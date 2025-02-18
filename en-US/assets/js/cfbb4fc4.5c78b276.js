"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[49571],{28453:(e,r,n)=>{n.d(r,{R:()=>t,x:()=>a});var l=n(96540);const o={},s=l.createContext(o);function t(e){const r=l.useContext(s);return l.useMemo((function(){return"function"==typeof e?e(r):{...r,...e}}),[r,e])}function a(e){let r;return r=e.disableParentContext?"function"==typeof e.components?e.components(o):e.components||o:t(e.components),l.createElement(s.Provider,{value:r},e.children)}},35533:(e,r,n)=>{n.r(r),n.d(r,{assets:()=>p,contentTitle:()=>t,default:()=>i,frontMatter:()=>s,metadata:()=>a,toc:()=>c});var l=n(74848),o=n(28453);const s={},t="TuGraph-Explorer",a={id:"developer-manual/ecosystem-tools/tugraph-explorer",title:"TuGraph-Explorer",description:"TuGraph Explorer \u5f3a\u4f9d\u8d56 TuGraph\uff0c\u56e0\u6b64\uff0c\u5728\u542f\u52a8 Explorer \u4e4b\u524d\uff0c\u6211\u4eec\u5148\u9700\u8981\u5148\u542f\u52a8 TuGraph\u3002",source:"@site/versions/version-3.6.0/zh-CN/source/5.developer-manual/5.ecosystem-tools/3.tugraph-explorer.md",sourceDirName:"5.developer-manual/5.ecosystem-tools",slug:"/developer-manual/ecosystem-tools/tugraph-explorer",permalink:"/tugraph-db/en-US/zh/3.6.0/developer-manual/ecosystem-tools/tugraph-explorer",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:3,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"TuGraph-DataX",permalink:"/tugraph-db/en-US/zh/3.6.0/developer-manual/ecosystem-tools/tugraph-datax"},next:{title:"\u65e5\u5fd7\u4fe1\u606f",permalink:"/tugraph-db/en-US/zh/3.6.0/developer-manual/ecosystem-tools/log"}},p={},c=[{value:"1.TuGraph Explorer \u7b80\u4ecb",id:"1tugraph-explorer-\u7b80\u4ecb",level:2},{value:"2.\u542f\u52a8 TuGraph Explorer",id:"2\u542f\u52a8-tugraph-explorer",level:2},{value:"3.\u8fde\u63a5 TuGraph",id:"3\u8fde\u63a5-tugraph",level:2}];function h(e){const r={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",header:"header",img:"img",li:"li",p:"p",pre:"pre",strong:"strong",ul:"ul",...(0,o.R)(),...e.components};return(0,l.jsxs)(l.Fragment,{children:[(0,l.jsx)(r.header,{children:(0,l.jsx)(r.h1,{id:"tugraph-explorer",children:"TuGraph-Explorer"})}),"\n",(0,l.jsxs)(r.blockquote,{children:["\n",(0,l.jsx)(r.p,{children:"TuGraph Explorer \u5f3a\u4f9d\u8d56 TuGraph\uff0c\u56e0\u6b64\uff0c\u5728\u542f\u52a8 Explorer \u4e4b\u524d\uff0c\u6211\u4eec\u5148\u9700\u8981\u5148\u542f\u52a8 TuGraph\u3002"}),"\n"]}),"\n",(0,l.jsx)(r.h2,{id:"1tugraph-explorer-\u7b80\u4ecb",children:"1.TuGraph Explorer \u7b80\u4ecb"}),"\n",(0,l.jsx)(r.p,{children:"TuGraph Explorer \u662f\u57fa\u4e8e GraphInsight \u6784\u5efa\u7684\u56fe\u53ef\u89c6\u5206\u6790\u5e73\u53f0\uff0c\u63d0\u4f9b\u4e86\u5b8c\u6574\u7684\u56fe\u63a2\u7d22\u5206\u6790\u80fd\u529b\uff0c\u80fd\u591f\u5e2e\u52a9\u7528\u6237\u4ece\u6d77\u91cf\u7684\u56fe\u6570\u636e\u4e2d\u6d1e\u5bdf\u51fa\u6709\u4ef7\u503c\u7684\u4fe1\u606f\u3002TuGraph Explorer \u5f53\u524d\u63d0\u4f9b\u4e86Docker\u90e8\u7f72\u65b9\u5f0f\u3002"}),"\n",(0,l.jsx)(r.h2,{id:"2\u542f\u52a8-tugraph-explorer",children:"2.\u542f\u52a8 TuGraph Explorer"}),"\n",(0,l.jsx)(r.p,{children:"TuGraph \u5b89\u88c5\u6210\u529f\u4ee5\u540e\uff0c\u5c31\u53ef\u4ee5\u5f00\u59cb\u5b89\u88c5 TuGraph Explorer\u3002"}),"\n",(0,l.jsxs)(r.ul,{children:["\n",(0,l.jsx)(r.li,{children:"\u52a0\u8f7d TuGraph Explorer \u955c\u50cf\uff1a"}),"\n"]}),"\n",(0,l.jsx)(r.pre,{children:(0,l.jsx)(r.code,{className:"language-shell",children:"// lgraph_lastest.tar.gz \u662f TuGraph \u955c\u50cf\u6587\u4ef6\u540d\n$ docker import tugraph_explore.tar.gz\n\n// \u52a0\u8f7d\u5b8c\u6bd5\u540e, \u63d0\u793a\u5df2\u52a0\u8f7d\u955c\u50cf\n"})}),"\n",(0,l.jsxs)(r.ul,{children:["\n",(0,l.jsx)(r.li,{children:"\u542f\u52a8 Docker"}),"\n"]}),"\n",(0,l.jsx)(r.pre,{children:(0,l.jsx)(r.code,{className:"language-shell",children:"$ docker run -d -p 7091:7091 -it reg.docker.alibaba-inc.com/tugraph-explore:1.0.1\n$ docker exec -it {container_id} bash\n"})}),"\n",(0,l.jsx)(r.p,{children:"\u53c2\u6570\u8bf4\u660e\uff1a"}),"\n",(0,l.jsxs)(r.ul,{children:["\n",(0,l.jsxs)(r.li,{children:["\n",(0,l.jsx)(r.p,{children:"-p \u7684\u4f5c\u7528\u662f\u7aef\u53e3\u6620\u5c04\uff0c\u793a\u4f8b\u4e2d\u5c06 Docker \u7684 7091 \u7aef\u53e3\u6620\u5c04\u5230\u672c\u5730\u7684 7091 \u7aef\u53e3"}),"\n"]}),"\n",(0,l.jsxs)(r.li,{children:["\n",(0,l.jsx)(r.p,{children:"{container_id} \u662f Docker \u7684 container id\uff0c\u53ef\u4ee5\u901a\u8fc7 docker ps \u83b7\u5f97"}),"\n"]}),"\n",(0,l.jsxs)(r.li,{children:["\n",(0,l.jsx)(r.p,{children:"\u542f\u52a8 TuGraph Explorer"}),"\n"]}),"\n"]}),"\n",(0,l.jsx)(r.pre,{children:(0,l.jsx)(r.code,{className:"language-shell",children:"$ cd /usr/src/tugraphexplore\n$ npm run dev -- -p 7091\n"})}),"\n",(0,l.jsxs)(r.p,{children:["TuGraph Explorer \u670d\u52a1\u542f\u52a8\u8d77\u6765\u4ee5\u540e\uff0c\u901a\u8fc7 ",(0,l.jsx)(r.strong,{children:(0,l.jsx)(r.a,{href:"http://localhost:7091/tugraph/explore.html",children:"http://localhost:7091/tugraph/explore.html"})})," \u5c31\u53ef\u4ee5\u8bbf\u95ee\u4e86\uff0c\u5982\u679c\u4e00\u5207\u6b63\u5e38\uff0c\u5c31\u4f1a\u770b\u5230\u5982\u4e0b\u9875\u9762\u3002",(0,l.jsx)(r.img,{src:"https://tugraph-web-static.oss-cn-beijing.aliyuncs.com/tugraph-expolore/tugraph-explore-index.png",alt:"image.png"})]}),"\n",(0,l.jsx)(r.h2,{id:"3\u8fde\u63a5-tugraph",children:"3.\u8fde\u63a5 TuGraph"}),"\n",(0,l.jsxs)(r.p,{children:["TuGraph Explorer \u542f\u52a8\u8d77\u6765\u4ee5\u540e\uff0c\u7b2c\u4e00\u6b65\u5c31\u662f\u9700\u8981\u8fde\u63a5 TuGraph \u6570\u636e\u5e93\u3002\u70b9\u51fb\u300c\u8fde\u63a5\u300d\u6309\u94ae\uff0c\u5f39\u51fa\u8fde\u63a5\u56fe\u6570\u636e\u5e93\u7684\u9875\u9762\uff0c\u5982\u4e0b\u56fe\u6240\u793a\u3002\n",(0,l.jsx)(r.img,{src:"https://gw.alipayobjects.com/mdn/rms_fa12c2/afts/img/A*JEUKRYMH--4AAAAAAAAAAAAAARQnAQ",alt:"image.png"})]}),"\n",(0,l.jsx)(r.p,{children:"\u8fde\u63a5 TuGraph \u6570\u636e\uff0c\u6211\u4eec\u9700\u8981\u63d0\u4f9b\u4ee5\u4e0b\u4fe1\u606f\uff1a"}),"\n",(0,l.jsxs)(r.ul,{children:["\n",(0,l.jsx)(r.li,{children:"\u56fe\u6570\u636e\u5e93\u7684\u8d26\u53f7"}),"\n",(0,l.jsx)(r.li,{children:"\u56fe\u6570\u636e\u5e93\u7684\u5bc6\u7801"}),"\n",(0,l.jsx)(r.li,{children:"\u56fe\u6570\u636e\u5e93\u7684\u5730\u5740\uff1a\u683c\u5f0f\u4e3a ip:port"}),"\n"]}),"\n",(0,l.jsxs)(r.p,{children:[(0,l.jsx)(r.strong,{children:"\u5730\u5740\u9700\u8981\u586b\u5199\u5bb9\u5668 IP\uff0c\u53ef\u4ee5\u901a\u8fc7\u5982\u4e0b\u6307\u4ee4\u67e5\u770b"}),"\u3002"]}),"\n",(0,l.jsx)(r.pre,{children:(0,l.jsx)(r.code,{children:"$ docker run -d -v /Users/xx/tugraph:/mnt -p 7090:7090 -it reg.docker.alibaba-inc.com/tugraph/tugraph:3.3.0\n$ docker exec -it 8408b49033bc1698(TuGraph \u7684\u5bb9\u5668) bash\n$ cat /etc/hosts\n127.0.0.1\tlocalhost\n::1\tlocalhost ip6-localhost ip6-loopback\nfe00::0\tip6-localnet\nff00::0\tip6-mcastprefix\nff02::1\tip6-allnodes\nff02::2\tip6-allrouters\n172.17.0.4\t8408b543243bc69\n"})}),"\n",(0,l.jsxs)(r.p,{children:["\u5982\u4e0a\u6240\u793a\uff0c\u8fde\u63a5\u56fe\u6570\u636e\u5e93\u7684\u5730\u5740\u5e94\u8be5\u586b\u5199\uff1a",(0,l.jsx)(r.strong,{children:"172.17.0.4:7090"}),"\u3002"]})]})}function i(e={}){const{wrapper:r}={...(0,o.R)(),...e.components};return r?(0,l.jsx)(r,{...e,children:(0,l.jsx)(h,{...e})}):h(e)}}}]);