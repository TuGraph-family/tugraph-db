"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[76306],{28453:(e,n,t)=>{t.d(n,{R:()=>a,x:()=>i});var s=t(96540);const r={},o=s.createContext(r);function a(e){const n=s.useContext(o);return s.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function i(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(r):e.components||r:a(e.components),s.createElement(o.Provider,{value:n},e.children)}},36325:(e,n,t)=>{t.r(n),t.d(n,{assets:()=>d,contentTitle:()=>a,default:()=>h,frontMatter:()=>o,metadata:()=>i,toc:()=>c});var s=t(74848),r=t(28453);const o={},a="Cluster management",i={id:"utility-tools/ha-cluster-management",title:"Cluster management",description:"This document mainly introduces the management tools of TuGraph HA cluster, including the functions of deleting nodes, leader transfer and generating snapshots.",source:"@site/versions/version-4.5.2/en-US/source/6.utility-tools/5.ha-cluster-management.md",sourceDirName:"6.utility-tools",slug:"/utility-tools/ha-cluster-management",permalink:"/tugraph-db/en-US/en/4.5.2/utility-tools/ha-cluster-management",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:5,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Data Warmup",permalink:"/tugraph-db/en-US/en/4.5.2/utility-tools/data-warmup"},next:{title:"Tugraph CLI",permalink:"/tugraph-db/en-US/en/4.5.2/utility-tools/tugraph-cli"}},d={},c=[{value:"1 Introduction",id:"1-introduction",level:2},{value:"2. Delete node",id:"2-delete-node",level:2},{value:"3. leader transfer",id:"3-leader-transfer",level:2},{value:"4. Generate snapshot",id:"4-generate-snapshot",level:2}];function l(e){const n={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",header:"header",li:"li",p:"p",pre:"pre",ul:"ul",...(0,r.R)(),...e.components};return(0,s.jsxs)(s.Fragment,{children:[(0,s.jsx)(n.header,{children:(0,s.jsx)(n.h1,{id:"cluster-management",children:"Cluster management"})}),"\n",(0,s.jsxs)(n.blockquote,{children:["\n",(0,s.jsx)(n.p,{children:"This document mainly introduces the management tools of TuGraph HA cluster, including the functions of deleting nodes, leader transfer and generating snapshots."}),"\n"]}),"\n",(0,s.jsx)(n.h2,{id:"1-introduction",children:"1 Introduction"}),"\n",(0,s.jsxs)(n.p,{children:["After the HA cluster is started, you can use the ",(0,s.jsx)(n.code,{children:"lgraph_peer"})," tool for cluster management, which can perform functions such as deleting nodes, transferring leaders, and generating snapshots."]}),"\n",(0,s.jsx)(n.h2,{id:"2-delete-node",children:"2. Delete node"}),"\n",(0,s.jsxs)(n.p,{children:["For nodes in the TuGraph HA cluster that are offline for a long time or have network partitions, you can use the ",(0,s.jsx)(n.code,{children:"remove_peer"})," command of ",(0,s.jsx)(n.code,{children:"lgraph_peer"})," to delete the node. An example command is as follows:"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-shell",children:"$ lgraph_peer --command remove_peer --peer {peer_id} --conf {group_conf}\n"})}),"\n",(0,s.jsx)(n.p,{children:"in:"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"--command remove_peer"})," specifies that the operation to be performed is remove_peer, that is, delete the node."]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"--peer {peer_id}"})," specifies the rpc network address of the node to be deleted, such as ",(0,s.jsx)(n.code,{children:"127.0.0.1:9092"}),"."]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"--conf {group_conf}"})," specifies the member configuration of the HA cluster (as long as it can be connected to the master node), such as ",(0,s.jsx)(n.code,{children:"127.0.0.1:9092,127.0.0.1:9093,127.0.0.1:9094"}),"."]}),"\n"]}),"\n",(0,s.jsx)(n.h2,{id:"3-leader-transfer",children:"3. leader transfer"}),"\n",(0,s.jsxs)(n.p,{children:["When you need to shut down or restart the master node, in order to reduce the unserviceable time of the cluster, you can use the ",(0,s.jsx)(n.code,{children:"transfer_leader"})," command of ",(0,s.jsx)(n.code,{children:"lgraph_peer"})," to transfer the master node. An example command is as follows:"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-shell",children:"$ lgraph_peer --command transfer_leader --peer {peer_id} --conf {group_conf}\n"})}),"\n",(0,s.jsx)(n.p,{children:"in:"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"--command transfer_leader"})," specifies that the operation to be performed is transfer_leader, that is, transferring the master node."]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"--peer {peer_id}"})," specifies the rpc network address to become the master node, such as ",(0,s.jsx)(n.code,{children:"127.0.0.1:9092"}),"."]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"--conf {group_conf}"})," specifies the member configuration of the HA cluster (as long as it can be connected to the master node), such as ",(0,s.jsx)(n.code,{children:"127.0.0.1:9092,127.0.0.1:9093,127.0.0.1:9094"}),"."]}),"\n"]}),"\n",(0,s.jsx)(n.h2,{id:"4-generate-snapshot",children:"4. Generate snapshot"}),"\n",(0,s.jsxs)(n.p,{children:["For reasons such as setting ha_snapshot_interval_s to -1 when the node starts so that snapshots are not taken by default or other reasons,\nwhen you need to tell a node generate a snapshot, you can use the ",(0,s.jsx)(n.code,{children:"snapshot"})," command of ",(0,s.jsx)(n.code,{children:"lgraph_peer"}),". An example command is as follows:"]}),"\n",(0,s.jsx)(n.pre,{children:(0,s.jsx)(n.code,{className:"language-shell",children:"$ lgraph_peer --command snapshot --peer {peer_id}\n"})}),"\n",(0,s.jsx)(n.p,{children:"in:"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"--command snapshot"})," specifies that the operation to be performed is snapshot, that is, generating a snapshot."]}),"\n",(0,s.jsxs)(n.li,{children:[(0,s.jsx)(n.code,{children:"--peer {peer_id}"})," specifies the rpc network address of the node to generate a snapshot, such as ",(0,s.jsx)(n.code,{children:"127.0.0.1:9092"}),"."]}),"\n"]})]})}function h(e={}){const{wrapper:n}={...(0,r.R)(),...e.components};return n?(0,s.jsx)(n,{...e,children:(0,s.jsx)(l,{...e})}):l(e)}}}]);