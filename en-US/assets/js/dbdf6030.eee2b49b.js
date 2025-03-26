"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[71916],{21771:(e,t,r)=>{r.r(t),r.d(t,{assets:()=>l,contentTitle:()=>i,default:()=>c,frontMatter:()=>a,metadata:()=>o,toc:()=>h});var n=r(74848),s=r(28453);const a={},i="High Availability mode",o={id:"installation&running/high-availability-mode",title:"High Availability mode",description:"This document describes the principles, preparations, and server operations of the high availability mode",source:"@site/versions/version-4.3.0/en-US/source/5.installation&running/8.high-availability-mode.md",sourceDirName:"5.installation&running",slug:"/installation&running/high-availability-mode",permalink:"/tugraph-db/en-US/en/4.3.0/installation&running/high-availability-mode",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:8,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Tugraph Running",permalink:"/tugraph-db/en-US/en/4.3.0/installation&running/tugraph-running"},next:{title:"Data Importing",permalink:"/tugraph-db/en-US/en/4.3.0/utility-tools/data-import"}},l={},h=[{value:"1.Theory",id:"1theory",level:2},{value:"2.Preparation",id:"2preparation",level:2},{value:"3.Start the initial backup group",id:"3start-the-initial-backup-group",level:2},{value:"3.1.The initial data is consistent",id:"31the-initial-data-is-consistent",level:3},{value:"3.2.Inconsistent initial data",id:"32inconsistent-initial-data",level:3},{value:"4.Start witness node",id:"4start-witness-node",level:2},{value:"4.1. Witness nodes are not allowed to become leader",id:"41-witness-nodes-are-not-allowed-to-become-leader",level:3},{value:"4.1. Allow witness nodes to become leaders",id:"41-allow-witness-nodes-to-become-leaders",level:3},{value:"5.Scale out other servers",id:"5scale-out-other-servers",level:2},{value:"6.Stopping the Server",id:"6stopping-the-server",level:2},{value:"7.Restarting the Server",id:"7restarting-the-server",level:2},{value:"8.docker deploys a highly available cluster",id:"8docker-deploys-a-highly-available-cluster",level:2},{value:"8.1.Install mirror",id:"81install-mirror",level:3},{value:"8.2.Create container",id:"82create-container",level:3},{value:"8.3.Start service",id:"83start-service",level:3},{value:"9.Server Status",id:"9server-status",level:2},{value:"10.Data synchronization in high availability mode",id:"10data-synchronization-in-high-availability-mode",level:2}];function d(e){const t={a:"a",blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",li:"li",ol:"ol",p:"p",pre:"pre",strong:"strong",ul:"ul",...(0,s.R)(),...e.components};return(0,n.jsxs)(n.Fragment,{children:[(0,n.jsx)(t.header,{children:(0,n.jsx)(t.h1,{id:"high-availability-mode",children:"High Availability mode"})}),"\n",(0,n.jsxs)(t.blockquote,{children:["\n",(0,n.jsx)(t.p,{children:"This document describes the principles, preparations, and server operations of the high availability mode"}),"\n"]}),"\n",(0,n.jsx)(t.h2,{id:"1theory",children:"1.Theory"}),"\n",(0,n.jsx)(t.p,{children:"TuGraph provides high availability (HA) mode through multi-machine hot backup. In high availability mode, write operations to the database will be synchronized to all servers (non-witness), so that even if some servers are down, the availability of the service will not be affected."}),"\n",(0,n.jsxs)(t.p,{children:["When the high-availability mode is started, multiple TuGraph servers form a backup group, which is a high-availability cluster. Each backup group consists of three or more TuGraph servers, one of which serves as the ",(0,n.jsx)(t.code,{children:"leader"})," and the other replication group servers as ",(0,n.jsx)(t.code,{children:"followers"}),". Write requests are served by a ",(0,n.jsx)(t.code,{children:"leader"}),", which replicates and synchronizes each request to a ",(0,n.jsx)(t.code,{children:"follower"})," and can only respond to the client after the request has been synchronized to the server. This way, if any server fails, the other servers will still have all the data written so far. If the ",(0,n.jsx)(t.code,{children:"leader"})," server fails, other servers will automatically select a new ",(0,n.jsx)(t.code,{children:"leader"}),"."]}),"\n",(0,n.jsxs)(t.p,{children:["TuGraph's high-availability mode provides two types of nodes: ",(0,n.jsx)(t.code,{children:"replica"})," nodes and ",(0,n.jsx)(t.code,{children:"witness"})," nodes. Among them, the ",(0,n.jsx)(t.code,{children:"replica"})," node is an ordinary node, has logs and data, and can provide services to the outside world. The ",(0,n.jsx)(t.code,{children:"witness"})," node is a node that only receives heartbeats and logs but does not save data. According to deployment requirements, ",(0,n.jsx)(t.code,{children:"leader"})," nodes and ",(0,n.jsx)(t.code,{children:"follower"})," nodes can be flexibly deployed as ",(0,n.jsx)(t.code,{children:"replica"})," nodes or ",(0,n.jsx)(t.code,{children:"witness"})," nodes. Based on this, there are two deployment methods for TuGraph high-availability mode: one is the ordinary deployment mode, and the other is the simple deployment mode with witness."]}),"\n",(0,n.jsxs)(t.p,{children:["For normal deployment mode, ",(0,n.jsx)(t.code,{children:"leader"})," and all ",(0,n.jsx)(t.code,{children:"followers"})," are nodes of type ",(0,n.jsx)(t.code,{children:"replica"}),". Write requests are served by a ",(0,n.jsx)(t.code,{children:"leader"}),", which copies each request to a ",(0,n.jsx)(t.code,{children:"follower"})," and cannot respond to the client until the request has been synchronized to more than half of the servers. This way, if less than half of the servers fail, the other servers will still have all the data written so far. If the ",(0,n.jsx)(t.code,{children:"leader"})," server fails, other servers will automatically elect a new ",(0,n.jsx)(t.code,{children:"leader"})," to ensure data consistency and service availability."]}),"\n",(0,n.jsxs)(t.p,{children:["However, when the user server resources are insufficient or a network partition occurs, a normal HA cluster cannot be established. At this time, since the ",(0,n.jsx)(t.code,{children:"witness"})," node has no data and takes up little resources, the ",(0,n.jsx)(t.code,{children:"witness"})," node and the ",(0,n.jsx)(t.code,{children:"replica"})," node can be deployed on one machine. For example, when there are only 2 machines, you can deploy the ",(0,n.jsx)(t.code,{children:"replica"})," node on one machine, and the ",(0,n.jsx)(t.code,{children:"replica"})," node and ",(0,n.jsx)(t.code,{children:"witness"})," node on another machine, which not only saves resources, but also does not require log application To the state machine, there is no need to generate and install snapshots, so the response to requests is very fast, and it can help quickly elect a new leader when the cluster crashes or the network is partitioned. This is the simple deployment mode of the TuGraph HA cluster. Although ",(0,n.jsx)(t.code,{children:"witness"})," nodes have many benefits, since there is no data, the cluster actually adds a node that cannot become ",(0,n.jsx)(t.code,{children:"leader"}),", so the availability will be slightly reduced. To improve the availability of the cluster, you can allow the ",(0,n.jsx)(t.code,{children:"witness"})," node to be the leader temporarily by specifying the ",(0,n.jsx)(t.code,{children:"ha_enable_witness_to_leader"})," parameter as ",(0,n.jsx)(t.code,{children:"true"}),". After the ",(0,n.jsx)(t.code,{children:"witness"})," node synchronizes the new log to other nodes, it will actively switch the leader role to the node with the latest log."]}),"\n",(0,n.jsx)(t.p,{children:"This feature is supported in version 3.6 and above."}),"\n",(0,n.jsx)(t.h2,{id:"2preparation",children:"2.Preparation"}),"\n",(0,n.jsx)(t.p,{children:"To enable high availability mode, users need to:"}),"\n",(0,n.jsxs)(t.ul,{children:["\n",(0,n.jsx)(t.li,{children:"Three or more instances of TuGraph servers."}),"\n",(0,n.jsx)(t.li,{children:"To enable high availability mode when starting lgraph_server, the 'enable_ha' option can be set to 'true' using a configuration file or the command line."}),"\n",(0,n.jsx)(t.li,{children:"Set the correct rpc_port through the configuration file or command line"}),"\n"]}),"\n",(0,n.jsx)(t.h2,{id:"3start-the-initial-backup-group",children:"3.Start the initial backup group"}),"\n",(0,n.jsxs)(t.p,{children:["After installing TuGraph, you can use the ",(0,n.jsx)(t.code,{children:"lgraph_server"})," command to start a high-availability cluster on different machines. This section mainly explains how to start a high-availability cluster. For cluster status management after startup, see ",(0,n.jsx)(t.a,{href:"/tugraph-db/en-US/en/4.3.0/utility-tools/ha-cluster-management",children:"lgraph_peer tool"})]}),"\n",(0,n.jsx)(t.h3,{id:"31the-initial-data-is-consistent",children:"3.1.The initial data is consistent"}),"\n",(0,n.jsxs)(t.p,{children:["When the data in all servers is the same or there is no data at startup, the user can\nspecify ",(0,n.jsx)(t.code,{children:"--ha_conf host1:port1,host2:port2"})," to start the server.\nIn this way, all prepared TuGraph instances can be added to the initial backup group at one time,\nAll servers in the backup group elect ",(0,n.jsx)(t.code,{children:"leader"})," according to the RAFT protocol, and other\nservers join the backup group with the role of ",(0,n.jsx)(t.code,{children:"follower"}),"."]}),"\n",(0,n.jsx)(t.p,{children:"An example command to start an initial backup group is as follows:"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-bash",children:"$ ./lgraph_server -c lgraph.json --rpc_port 9090 --enable_ha true --ha_conf 172.22.224.15:9090,172.22.224.16:9090,172.22.224.17:9090\n"})}),"\n",(0,n.jsx)(t.p,{children:"After the first server is started, it will elect itself as the 'leader' and organize a backup group with only itself."}),"\n",(0,n.jsx)(t.h3,{id:"32inconsistent-initial-data",children:"3.2.Inconsistent initial data"}),"\n",(0,n.jsxs)(t.p,{children:["If there is already data in the first server (imported by the ",(0,n.jsx)(t.code,{children:"lgraph_import"})," tool or transferred from a server in non-high availability mode),\nAnd it has not been used in high-availability mode before, the user should use the boostrap method to start. Start the server with data in bootstrap\nmode with the ",(0,n.jsx)(t.code,{children:"ha_bootstrap_role"})," parameter as 1, and specify the machine as the ",(0,n.jsx)(t.code,{children:"leader"})," through the ",(0,n.jsx)(t.code,{children:"ha_conf"}),"\nparameter. In bootstrap mode, the server will copy its own data to the new server before adding the newly\njoined server to the backup group, so that the data in each server is consistent."]}),"\n",(0,n.jsx)(t.p,{children:"An example command to start a data server is as follows:"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-bash",children:"$ ./lgraph_server -c lgraph.json --rpc_port 9090 --enable_ha true --ha_conf 172.22.224.15:9090,172.22.224.16:9090,172.22.224.17:9090 --ha_bootstrap_role 1\n"})}),"\n",(0,n.jsxs)(t.p,{children:["Other servers without data need to specify the ",(0,n.jsx)(t.code,{children:"ha_bootstrap_role"})," parameter as 2, and specify the ",(0,n.jsx)(t.code,{children:"leader"})," through the ",(0,n.jsx)(t.code,{children:"ha_conf"})," parameter. The command example is as follows"]}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-bash",children:"**$ ./lgraph_server -c lgraph.json --rpc_port 9090 --enable_ha true --ha_conf 172.22.224.15:9090,172.22.224.16:9090,172.22.224.17:9090 --ha_bootstrap_role 2\n"})}),"\n",(0,n.jsx)(t.p,{children:(0,n.jsx)(t.strong,{children:"You need to pay attention to two points when using bootstrap to start an HA cluster:"})}),"\n",(0,n.jsxs)(t.ol,{children:["\n",(0,n.jsxs)(t.li,{children:["You need to wait for the ",(0,n.jsx)(t.code,{children:"leader"})," node to generate a snapshot and start successfully before joining the ",(0,n.jsx)(t.code,{children:"follower"})," node, otherwise the ",(0,n.jsx)(t.code,{children:"follower"})," node may fail to join. When starting the ",(0,n.jsx)(t.code,{children:"follower"})," node, you can configure the ",(0,n.jsx)(t.code,{children:"ha_node_join_group_s"})," parameter to be slightly larger to allow multiple waits and timeout retries when joining the HA cluster."]}),"\n",(0,n.jsx)(t.li,{children:"The HA cluster can only use the bootstrap mode when it is started for the first time. It can only be started in the normal mode (see Section 3.1) when it is started later. In particular, multiple nodes of the same cluster cannot be started in the bootstrap mode, otherwise it may cause Data inconsistency"}),"\n"]}),"\n",(0,n.jsx)(t.h2,{id:"4start-witness-node",children:"4.Start witness node"}),"\n",(0,n.jsx)(t.h3,{id:"41-witness-nodes-are-not-allowed-to-become-leader",children:"4.1. Witness nodes are not allowed to become leader"}),"\n",(0,n.jsxs)(t.p,{children:["The startup method of ",(0,n.jsx)(t.code,{children:"witness"})," node is the same as that of ordinary nodes. You only need to set the ",(0,n.jsx)(t.code,{children:"ha_is_witness"})," parameter to ",(0,n.jsx)(t.code,{children:"true"}),". Note that the number of witness nodes should be less than half of the total number of cluster nodes."]}),"\n",(0,n.jsx)(t.p,{children:"An example command to start the witness node server is as follows:"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-bash",children:"$ ./lgraph_server -c lgraph.json --rpc_port 9090 --enable_ha true --ha_conf 172.22.224.15:9090,172.22.224.16:9090,172.22.224.17:9090 --ha_is_witness 1\n"})}),"\n",(0,n.jsxs)(t.p,{children:["Note: By default, the ",(0,n.jsx)(t.code,{children:"witness"})," node is not allowed to become the ",(0,n.jsx)(t.code,{children:"leader"})," node, which can improve the performance of the cluster, but will reduce the availability of the cluster when the ",(0,n.jsx)(t.code,{children:"leader"})," node crashes."]}),"\n",(0,n.jsx)(t.h3,{id:"41-allow-witness-nodes-to-become-leaders",children:"4.1. Allow witness nodes to become leaders"}),"\n",(0,n.jsxs)(t.p,{children:["You can specify the ",(0,n.jsx)(t.code,{children:"ha_enable_witness_to_leader"})," parameter as ",(0,n.jsx)(t.code,{children:"true"}),", so that the ",(0,n.jsx)(t.code,{children:"witness"})," node can temporarily become the ",(0,n.jsx)(t.code,{children:"leader"})," node, and then actively switch to the master after the new log synchronization is completed."]}),"\n",(0,n.jsxs)(t.p,{children:["An example of the command to start the ",(0,n.jsx)(t.code,{children:"witness"})," node server that is allowed to become the ",(0,n.jsx)(t.code,{children:"leader"})," node is as follows:"]}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-bash",children:"$ ./lgraph_server -c lgraph.json --rpc_port 9090 --enable_ha true --ha_conf 172.22.224.15:9090,172.22.224.16:9090,172.22.224.17:9090 --ha_is_witness 1 --ha_enable_witness_to_leader 1\n"})}),"\n",(0,n.jsxs)(t.p,{children:["Note: Although allowing ",(0,n.jsx)(t.code,{children:"witness"})," nodes to become ",(0,n.jsx)(t.code,{children:"leader"})," nodes can improve the availability of the cluster, it may affect data consistency in extreme cases. Therefore, it should generally be ensured that the number of ",(0,n.jsx)(t.code,{children:"witness"})," nodes + 1 is less than half of the total number of cluster nodes."]}),"\n",(0,n.jsx)(t.h2,{id:"5scale-out-other-servers",children:"5.Scale out other servers"}),"\n",(0,n.jsxs)(t.p,{children:["After starting the initial backup group, if you want to scale out the backup group, add new servers to the backup group,\nThe ",(0,n.jsx)(t.code,{children:"--ha_conf HOST:PORT"})," option should be used, where ",(0,n.jsx)(t.code,{children:"HOST"})," can be the IP address of any server already in this backup group,\nAnd ",(0,n.jsx)(t.code,{children:"PORT"})," is its RPC port. E.g:"]}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-bash",children:"./lgraph_server -c lgraph.json --rpc_port 9090 --enable_ha true --ha_conf 172.22.224.15:9090\n"})}),"\n",(0,n.jsxs)(t.p,{children:["This command will start a TuGraph server in high availability mode and try to add it to the backup group containing the server ",(0,n.jsx)(t.code,{children:"172.22.224.15:9090"}),".\nNote that joining a backup group requires a server to synchronize its data with the backup group's ",(0,n.jsx)(t.code,{children:"leader"})," server, and this process may take a considerable amount of time, depending on the size of the data."]}),"\n",(0,n.jsx)(t.h2,{id:"6stopping-the-server",children:"6.Stopping the Server"}),"\n",(0,n.jsx)(t.p,{children:"When a server goes offline via 'CTRL-C', it will notify the current 'leader' server to remove the server from the backup group. If the leader server goes offline, it will pass the leader identity permission to another server before going offline."}),"\n",(0,n.jsx)(t.p,{children:"If a server is terminated or disconnected from other servers in the backup group, the server is considered a failed node and the leader server will remove it from the backup group after a specified time limit."}),"\n",(0,n.jsx)(t.p,{children:"If any server leaves the backup group and wishes to rejoin, it must start with the '--ha_conf {HOST:PORT}' option, where 'HOST' is the IP address of a server in the current backup group."}),"\n",(0,n.jsx)(t.h2,{id:"7restarting-the-server",children:"7.Restarting the Server"}),"\n",(0,n.jsxs)(t.p,{children:["Restarting the entire backup group is not recommended as it disrupts service. All servers can be shut down if desired. But on reboot,\nIt must be ensured that at least N/2+1 servers in the backup group at shutdown can start normally, otherwise the startup will fail. and,\nRegardless of whether ",(0,n.jsx)(t.code,{children:"enable_bootstrap"})," is specified as true when initially starting the replication group, restarting the server only needs to pass\nSpecify the ",(0,n.jsx)(t.code,{children:"--ha_conf host1:port1,host2:port2"})," parameter to restart all servers at once. The command example is as follows:"]}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-bash",children:"$ ./lgraph_server -c lgraph.json --rpc_port 9090 --enable_ha true --ha_conf 172.22.224.15:9090,172.22.224.16:9090,172.22.224.17:9090\n"})}),"\n",(0,n.jsx)(t.h2,{id:"8docker-deploys-a-highly-available-cluster",children:"8.docker deploys a highly available cluster"}),"\n",(0,n.jsx)(t.p,{children:"In real business scenarios, it is likely to encounter the need to deploy high-availability clusters on multiple operating systems or architectures.\nDifferentiated environments may cause some dependencies to be missing when compiling TuGraph. therefore,\nCompiling software in docker and deploying high-availability clusters is very valuable. Take the centos7 version of docker as an example,\nThe steps to deploy a highly available cluster are as follows."}),"\n",(0,n.jsx)(t.h3,{id:"81install-mirror",children:"8.1.Install mirror"}),"\n",(0,n.jsx)(t.p,{children:"Use the following command to download TuGraph's compiled docker image environment"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-shell",children:"docker pull tugraph/tugraph-compile-centos7\n"})}),"\n",(0,n.jsx)(t.p,{children:"Then pull the TuGraph source code and compile and install"}),"\n",(0,n.jsx)(t.h3,{id:"82create-container",children:"8.2.Create container"}),"\n",(0,n.jsxs)(t.p,{children:["Use the following command to create a container, use ",(0,n.jsx)(t.code,{children:"--net=host"})," to make the container run in host mode, in this mode\nDocker and the host machine share the network namespace, that is, they share the same IP."]}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-shell",children:"docker run --net=host -itd -p -v {src_dir}:{dst_dir} --name tugraph_ha tugraph/tugraph-compile-centos7 /bin/bash\n"})}),"\n",(0,n.jsx)(t.h3,{id:"83start-service",children:"8.3.Start service"}),"\n",(0,n.jsx)(t.p,{children:"Use the following command to start the service on each server, because docker and the host share IP, so you can directly specify to start the service on the host IP"}),"\n",(0,n.jsx)(t.pre,{children:(0,n.jsx)(t.code,{className:"language-shell",children:"$ lgraph_server -c lgraph.json --host 172.22.224.15 --rpc_port 9090 --enable_ha true --ha_conf 172.22.224.15:9090,172.22.224.16:9090,172.22.224.17:9090\n"})}),"\n",(0,n.jsx)(t.h2,{id:"9server-status",children:"9.Server Status"}),"\n",(0,n.jsx)(t.p,{children:"The current status of the backup group can be obtained from the TuGraph visualization tool, REST API, and Cypher query."}),"\n",(0,n.jsx)(t.p,{children:"In the TuGraph visualization tool, you can find the list of servers and their roles in the backup group in the DBInfo section."}),"\n",(0,n.jsxs)(t.p,{children:["With the REST API, you can use ",(0,n.jsx)(t.code,{children:"GET /info/peers"})," to request information."]}),"\n",(0,n.jsxs)(t.p,{children:["In Cypher, the ",(0,n.jsx)(t.code,{children:"CALL dbms.listServers()"})," statement is used to query the status information of the current backup group."]}),"\n",(0,n.jsx)(t.h2,{id:"10data-synchronization-in-high-availability-mode",children:"10.Data synchronization in high availability mode"}),"\n",(0,n.jsx)(t.p,{children:"In high availability mode, different servers in the same backup group may not always be in the same state. For performance reasons, if a request has been synchronized to more than half of the servers, the leader server will consider the request to be in the committed state. Although the rest of the servers will eventually receive the new request, the inconsistent state of the servers will persist for some time. A client may also send a request to a server that has just restarted, thus having an older state and waiting to join a backup group."}),"\n",(0,n.jsx)(t.p,{children:"To ensure that the client sees consistently continuous data, and in particular to get rid of the 'reverse time travel' problem, where the client reads a state older than it has seen before, each TuGraph server keeps a monotonically increasing data version number. The mapping of the data version number to the database state in the backup group is globally consistent, meaning that if two servers have the same data version number, they must have the same data. When responding to a request, the server includes its data version number in the response. Thus, the client can tell which version it has seen. The client can choose to send this data version number along with the request. Upon receiving a request with a data version number, the server compares the data version number to its current version and rejects the request if its own version is lower than the requested version. This mechanism ensures that the client never reads a state that is older than before."})]})}function c(e={}){const{wrapper:t}={...(0,s.R)(),...e.components};return t?(0,n.jsx)(t,{...e,children:(0,n.jsx)(d,{...e})}):d(e)}},28453:(e,t,r)=>{r.d(t,{R:()=>i,x:()=>o});var n=r(96540);const s={},a=n.createContext(s);function i(e){const t=n.useContext(a);return n.useMemo((function(){return"function"==typeof e?e(t):{...t,...e}}),[t,e])}function o(e){let t;return t=e.disableParentContext?"function"==typeof e.components?e.components(s):e.components||s:i(e.components),n.createElement(a.Provider,{value:t},e.children)}}}]);