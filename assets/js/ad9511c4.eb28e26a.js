"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[44397],{28453:(e,t,a)=>{a.d(t,{R:()=>s,x:()=>h});var n=a(96540);const i={},r=n.createContext(i);function s(e){const t=n.useContext(r);return n.useMemo((function(){return"function"==typeof e?e(t):{...t,...e}}),[t,e])}function h(e){let t;return t=e.disableParentContext?"function"==typeof e.components?e.components(i):e.components||i:s(e.components),n.createElement(r.Provider,{value:t},e.children)}},42483:(e,t,a)=>{a.r(t),a.d(t,{assets:()=>o,contentTitle:()=>s,default:()=>p,frontMatter:()=>r,metadata:()=>h,toc:()=>c});var n=a(74848),i=a(28453);const r={},s="What is Graph",h={id:"introduction/what-is-graph",title:"What is Graph",description:"The graph that we're going to introduce today, it's neither a picture, nor a diagram\uff0cit's a class of mathematics called graph theory. We can see on the screen that there are two graphs on the left and right, which represent things and their relationship. We abstract it out in a form, and we call such a form a graph.",source:"@site/versions/version-3.5.1/en-US/source/2.introduction/1.what-is-graph.md",sourceDirName:"2.introduction",slug:"/introduction/what-is-graph",permalink:"/tugraph-db/en/3.5.1/introduction/what-is-graph",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:1,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Guide",permalink:"/tugraph-db/en/3.5.1/guide"},next:{title:"What is a graph database",permalink:"/tugraph-db/en/3.5.1/introduction/what-is-gdbms"}},o={},c=[];function d(e){const t={h1:"h1",header:"header",img:"img",p:"p",...(0,i.R)(),...e.components};return(0,n.jsxs)(n.Fragment,{children:[(0,n.jsx)(t.header,{children:(0,n.jsx)(t.h1,{id:"what-is-graph",children:"What is Graph"})}),"\n",(0,n.jsx)(t.p,{children:"The graph that we're going to introduce today, it's neither a picture, nor a diagram\uff0cit's a class of mathematics called graph theory. We can see on the screen that there are two graphs on the left and right, which represent things and their relationship. We abstract it out in a form, and we call such a form a graph."}),"\n",(0,n.jsx)(t.p,{children:(0,n.jsx)(t.img,{alt:"alt what is graph",src:a(68309).A+"",width:"552",height:"296"})}),"\n",(0,n.jsx)(t.p,{children:"The basic elements of a graph are a vertex, and an edge, a vertex is a representation of the thing, of the entity, and an edge is a representation of the relationship between them."}),"\n",(0,n.jsx)(t.p,{children:"We see the graph on the left, the dots have companies, they have employees, and they have projects. What are their sides? The relationship between the company and the employees is an employment relationship, and there can be a friend relationship between the employees, and there can be a participation relationship between the project and the employees. That is, we can use graphs to abstract things and their relationships."}),"\n",(0,n.jsx)(t.p,{children:"The diagram on the right is a schematic diagram of a financial transaction. Each of us has a bank account, and when our bank account makes a consumption or makes a transfer, this account is the point on the graph, and this edge can indicate that there is such a transfer relationship between us."}),"\n",(0,n.jsx)(t.p,{children:"We can also see from this example that edges can attach information and points can attach information. The point can attach which bank this account is in and what the account is. The value of the transaction can be attached to the edge, and many times, if your transaction is large, it indicates that special attention is needed. This edge might not necessarily be the amount of a particular transfer, it might be the amount accumulated over a period of time and so on. Graphs can actually be very expressive."}),"\n",(0,n.jsx)(t.p,{children:"On the other hand, for a graph like finance, it can be very large. There could be more than a billion points, a hundred billion or even a trillion edges. As you can imagine, it's actually quite challenging to actually work with these graphs."})]})}function p(e={}){const{wrapper:t}={...(0,i.R)(),...e.components};return t?(0,n.jsx)(t,{...e,children:(0,n.jsx)(d,{...e})}):d(e)}},68309:(e,t,a)=>{a.d(t,{A:()=>n});const n=a.p+"assets/images/what-is-graph-en-48fc5e7076905ff553e38c0c2e79dca5.png"}}]);