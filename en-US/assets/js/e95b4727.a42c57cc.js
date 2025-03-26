"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[34006],{827:(e,n,t)=>{t.d(n,{A:()=>s});const s=t.p+"assets/images/cloud-deployment-3-72b2b61772400506bd9a80eb4d343017.png"},9261:(e,n,t)=>{t.d(n,{A:()=>s});const s=t.p+"assets/images/cloud-deployment-5-f4512e5e080e0b9f6138224cc49d424e.png"},16273:(e,n,t)=>{t.d(n,{A:()=>s});const s=t.p+"assets/images/cloud-deployment-9-8778b1296f1ca19397384c068bab67ea.png"},18509:(e,n,t)=>{t.d(n,{A:()=>s});const s=t.p+"assets/images/cloud-deployment-10-55820cf0e6bb8fac4c2e7d548b97a1fc.png"},27465:(e,n,t)=>{t.d(n,{A:()=>s});const s=t.p+"assets/images/cloud-deployment-1-6de73b2cfb5fe70e92e5e3435ad8574f.png"},28453:(e,n,t)=>{t.d(n,{R:()=>a,x:()=>c});var s=t(96540);const i={},r=s.createContext(i);function a(e){const n=s.useContext(r);return s.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function c(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(i):e.components||i:a(e.components),s.createElement(r.Provider,{value:n},e.children)}},31295:(e,n,t)=>{t.d(n,{A:()=>s});const s=t.p+"assets/images/cloud-deployment-7-a23a6b3eff495502a4c3dcb7e92e19ca.png"},46020:(e,n,t)=>{t.d(n,{A:()=>s});const s=t.p+"assets/images/cloud-deployment-4-0372e82170ac8bffe6d2d02b03b0a9e2.png"},64171:(e,n,t)=>{t.r(n),t.d(n,{assets:()=>l,contentTitle:()=>a,default:()=>h,frontMatter:()=>r,metadata:()=>c,toc:()=>o});var s=t(74848),i=t(28453);const r={},a="Cloud Deployment",c={id:"installation&running/cloud-deployment",title:"Cloud Deployment",description:"This document mainly introduces the cloud deployment of TuGraph, and you can also refer to theAlibaba Cloud ComputeNest deployment document.\u3002",source:"@site/versions/version-4.5.1/en-US/source/5.installation&running/5.cloud-deployment.md",sourceDirName:"5.installation&running",slug:"/installation&running/cloud-deployment",permalink:"/tugraph-db/en-US/en/4.5.1/installation&running/cloud-deployment",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:5,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Local Package Deployment",permalink:"/tugraph-db/en-US/en/4.5.1/installation&running/local-package-deployment"},next:{title:"Compile",permalink:"/tugraph-db/en-US/en/4.5.1/installation&running/compile"}},l={},o=[{value:"1.Introduction",id:"1introduction",level:2},{value:"2.Instance Description",id:"2instance-description",level:2},{value:"3.Deployment Process",id:"3deployment-process",level:2},{value:"3.1.Preparation",id:"31preparation",level:3},{value:"3.2.Deployment Entrance",id:"32deployment-entrance",level:3},{value:"3.3.Apply for Trial Use",id:"33apply-for-trial-use",level:3},{value:"3.4.Create TuGraph Service",id:"34create-tugraph-service",level:3},{value:"3.4.1.Parameter List",id:"341parameter-list",level:4},{value:"3.4.2.Specific Steps",id:"342specific-steps",level:4},{value:"3.5.Start TuGraph Service",id:"35start-tugraph-service",level:3},{value:"4.Common FAQs",id:"4common-faqs",level:2},{value:"No available resources in the selected deployment area",id:"no-available-resources-in-the-selected-deployment-area",level:3},{value:"Unaccessible to the web port",id:"unaccessible-to-the-web-port",level:3},{value:"Username or password is not correct",id:"username-or-password-is-not-correct",level:3}];function d(e){const n={a:"a",blockquote:"blockquote",h1:"h1",h2:"h2",h3:"h3",h4:"h4",header:"header",img:"img",li:"li",p:"p",strong:"strong",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",ul:"ul",...(0,i.R)(),...e.components};return(0,s.jsxs)(s.Fragment,{children:[(0,s.jsx)(n.header,{children:(0,s.jsx)(n.h1,{id:"cloud-deployment",children:"Cloud Deployment"})}),"\n",(0,s.jsxs)(n.blockquote,{children:["\n",(0,s.jsxs)(n.p,{children:["This document mainly introduces the cloud deployment of TuGraph, and you can also refer to the",(0,s.jsx)(n.a,{href:"https://aliyun-computenest.github.io/quickstart-tugraph/",children:"Alibaba Cloud ComputeNest deployment document."}),"\u3002"]}),"\n"]}),"\n",(0,s.jsx)(n.h2,{id:"1introduction",children:"1.Introduction"}),"\n",(0,s.jsx)(n.p,{children:"TuGraph (tugraph.antgroup.com) is a high-performance graph database of Alibaba Group. TuGraph provides community version services on ComputeNest, so you can quickly deploy TuGraph services on ComputeNest and achieve operation and maintenance monitoring, thereby building your own graph application. This document introduces how to open TuGraph community version services on ComputeNest, as well as deployment process and usage instructions."}),"\n",(0,s.jsx)(n.h2,{id:"2instance-description",children:"2.Instance Description"}),"\n",(0,s.jsx)(n.p,{children:"TuGraph is deployed as a community open source version, and the source code can be found in the Github Repo. Currently, the available instance specifications are as follows:"}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"Instance Family"}),(0,s.jsx)(n.th,{children:"vCPU and Memory"}),(0,s.jsx)(n.th,{children:"System Disk"}),(0,s.jsx)(n.th,{children:"Public Bandwidth"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"ecs.r7a.xlarge"}),(0,s.jsx)(n.td,{children:"AMD Memory r7a, 4vCPU 32GiB"}),(0,s.jsx)(n.td,{children:"ESSD Cloud Disk 200GiB PL0"}),(0,s.jsx)(n.td,{children:"Fixed bandwidth of 1Mbps"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"ecs.r6.xlarge"}),(0,s.jsx)(n.td,{children:"Memory r6, 4vCPU 32GiB"}),(0,s.jsx)(n.td,{children:"ESSD Cloud Disk 200GiB PL0"}),(0,s.jsx)(n.td,{children:"Fixed bandwidth of 1Mbps"})]})]})]}),"\n",(0,s.jsxs)(n.p,{children:["Estimated costs can be seen in real time when creating instances (currently free). If you need more specifications or other services (such as cluster high availability requirements, enterprise-level support services, etc.), please contact us at ",(0,s.jsx)(n.a,{href:"mailto:tugraph@service.alipay.com",children:"tugraph@service.alipay.com"}),"."]}),"\n",(0,s.jsx)(n.h2,{id:"3deployment-process",children:"3.Deployment Process"}),"\n",(0,s.jsx)(n.h3,{id:"31preparation",children:"3.1.Preparation"}),"\n",(0,s.jsx)(n.p,{children:"Before starting to use, you need an Alibaba Cloud account to access and create resources such as ECS and VPC."}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"If you use a personal account, you can directly create a service instance."}),"\n",(0,s.jsxs)(n.li,{children:["If you create a service instance using a RAM user and use Alibaba Cloud ComputeNest for the first time:\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"Before creating a service instance, you need to add permissions for the corresponding resources to the account of the RAM user. For detailed operations on adding RAM permissions, please see Grant RAM user permissions. The required permissions are shown in the following table."}),"\n",(0,s.jsx)(n.li,{children:"Authorization to create associated roles is also required. Refer to the following figure and select Agree to authorize and create associated roles."}),"\n"]}),"\n"]}),"\n"]}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"Permission Policy Name"}),(0,s.jsx)(n.th,{children:"Remark"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"AliyunECSFullAccess"}),(0,s.jsx)(n.td,{children:"Permissions for managing cloud server services (ECS)"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"AliyunVPCFullAccess"}),(0,s.jsx)(n.td,{children:"Permissions for managing Virtual Private Cloud (VPC)"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"AliyunROSFullAccess"}),(0,s.jsx)(n.td,{children:"Permissions for managing Resource Orchestration Service (ROS)"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"AliyunComputeNestUserFullAccess"}),(0,s.jsx)(n.td,{children:"Permissions for managing ComputeNest services (ComputeNest) on the user side"})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"AliyunCloudMonitorFullAccess"}),(0,s.jsx)(n.td,{children:"Permissions for managing Alibaba Cloud Monitor (CloudMonitor)"})]})]})]}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.img,{alt:"Cloud Deployment",src:t(27465).A+"",width:"2390",height:"612"})}),"\n",(0,s.jsx)(n.h3,{id:"32deployment-entrance",children:"3.2.Deployment Entrance"}),"\n",(0,s.jsx)(n.p,{children:"You can search in Alibaba Cloud ComputeNest, or quickly access it through the following deployment link."}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.a,{href:"https://computenest.console.aliyun.com/user/cn-hangzhou/serviceInstanceCreate?ServiceId=service-7b50ea3d20e643da95bf&&isTrial=true",children:"Deployment Link"})}),"\n",(0,s.jsx)(n.h3,{id:"33apply-for-trial-use",children:"3.3.Apply for Trial Use"}),"\n",(0,s.jsx)(n.p,{children:"Before formal trial use, you need to apply for trial use, fill in the information as prompted, and create the TuGraph service after passing the review."}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.img,{alt:"Apply for Trial Use",src:t(79282).A+"",width:"2071",height:"949"})}),"\n",(0,s.jsx)(n.h3,{id:"34create-tugraph-service",children:"3.4.Create TuGraph Service"}),"\n",(0,s.jsx)(n.h4,{id:"341parameter-list",children:"3.4.1.Parameter List"}),"\n",(0,s.jsx)(n.p,{children:"During the process of creating a service instance, you need to configure the parameter list of the service instance information. The specific parameters are as follows."}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,s.jsxs)(n.table,{children:[(0,s.jsx)(n.thead,{children:(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.th,{children:"Parameter Group"}),(0,s.jsx)(n.th,{children:"Parameter Item"}),(0,s.jsx)(n.th,{children:"Example"}),(0,s.jsx)(n.th,{children:"Description"})]})}),(0,s.jsxs)(n.tbody,{children:[(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"Service Instance Name"}),(0,s.jsx)(n.td,{children:"N/A"}),(0,s.jsx)(n.td,{children:"test"}),(0,s.jsx)(n.td,{children:"The name of the instance."})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"Region"}),(0,s.jsx)(n.td,{children:"N/A"}),(0,s.jsx)(n.td,{children:"China East 1 (Hangzhou)"}),(0,s.jsx)(n.td,{children:"Select the region of the service instance. It is recommended to select nearby regions to obtain better network latency."})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"Payment Type Configuration"}),(0,s.jsx)(n.td,{children:"Payment Type"}),(0,s.jsx)(n.td,{children:"Pay-As-You-Go"}),(0,s.jsx)(n.td,{children:"For free use, please select Pay-As-You-Go."})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"Availability Zone Configuration"}),(0,s.jsx)(n.td,{children:"Deployment Area"}),(0,s.jsx)(n.td,{children:"Availability ZoneI"}),(0,s.jsx)(n.td,{children:"Different available zones under the region, ensure that the instance is not empty."})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"Select Existing Basic Resource Configuration"}),(0,s.jsx)(n.td,{children:"VPC ID"}),(0,s.jsx)(n.td,{children:"vpc-xxx"}),(0,s.jsx)(n.td,{children:"Select the ID of the Virtual Private Cloud according to the actual situation."})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"Select Existing Basic Resource Configuration"}),(0,s.jsx)(n.td,{children:"Switch ID"}),(0,s.jsx)(n.td,{children:"vsw-xxx"}),(0,s.jsx)(n.td,{children:"Select the Switch ID according to the actual situation. If the Switch ID cannot be found, try switching the region and available zone."})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"ECS Instance Configuration"}),(0,s.jsx)(n.td,{children:"Instance Type"}),(0,s.jsx)(n.td,{children:"ecs.r6.xlarge"}),(0,s.jsx)(n.td,{children:"Currently supports ecs.r6.xlarge and ecs.r7a.xlarge specifications."})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{children:"ECS Instance Configuration"}),(0,s.jsx)(n.td,{children:"Instance Password"}),(0,s.jsx)(n.td,{children:"**"}),(0,s.jsx)(n.td,{children:"Set the instance password. Length of 8-30 characters, must include three items (uppercase letters, lowercase letters, numbers, special characters in ()`~!@#$%^&*_-+={}[]:;'<>,.?/)."})]}),(0,s.jsxs)(n.tr,{children:[(0,s.jsx)(n.td,{}),(0,s.jsx)(n.td,{}),(0,s.jsx)(n.td,{}),(0,s.jsx)(n.td,{})]})]})]}),"\n",(0,s.jsx)(n.h4,{id:"342specific-steps",children:"3.4.2.Specific Steps"}),"\n",(0,s.jsx)(n.p,{children:"The creation of the service is carried out according to the following steps, refer to the figure below:"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:'Create an instance name, such as "test" in the figure below'}),"\n",(0,s.jsx)(n.li,{children:'Select the region, such as "China East 1 (Hangzhou)" in the figure below'}),"\n"]}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.img,{alt:"Create Instance",src:t(827).A+"",width:"2874",height:"1066"})}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"Select the instance type, currently supports ecs.r6.xlarge and ecs.r7a.xlarge specifications. If there is no model available in the list, try selecting other deployment areas."}),"\n",(0,s.jsx)(n.li,{children:"Select the model"}),"\n",(0,s.jsx)(n.li,{children:"Configure the password for the instance"}),"\n",(0,s.jsx)(n.li,{children:'Select the deployment area, such as "Availability Zone I" in the figure below'}),"\n"]}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.img,{alt:"Select Region",src:t(46020).A+"",width:"4102",height:"1242"})}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"Click next to enter the order confirmation page"}),"\n",(0,s.jsx)(n.li,{children:'Check the checkboxes for "Permission Confirmation" and "Service Terms"'}),"\n",(0,s.jsx)(n.li,{children:'Click the green "Start Free Trial" button in the lower left corner to create a service instance'}),"\n"]}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.img,{alt:"Confirmation",src:t(9261).A+"",width:"3414",height:"2180"})}),"\n",(0,s.jsx)(n.h3,{id:"35start-tugraph-service",children:"3.5.Start TuGraph Service"}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"View the service instance: After the service instance is created successfully, it takes about 2 minutes for deployment. After the deployment is complete, you can see the corresponding service instance on the page, as shown in the figure below."}),"\n"]}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.img,{alt:"View Instance",src:t(77494).A+"",width:"4616",height:"1264"})}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"Click the service instance to access TuGraph. After entering the corresponding service instance, you can get 3 ways to\nuse it on the page: web, rpc, ssh. You can also see the password of user admin on the page."}),"\n"]}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.img,{alt:"Access Method",src:t(31295).A+"",width:"1216",height:"612"})}),"\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsxs)(n.li,{children:["Click the link of web to jump to the deployed TuGraph Web. It is recommended that novice users first use the demo to quickly get started with TuGraph.\n",(0,s.jsxs)(n.ul,{children:["\n",(0,s.jsx)(n.li,{children:"First, on the TuGraph Web login page, enter the default username (admin) and the password on the page to log in, as shown in the figure below."}),"\n",(0,s.jsx)(n.li,{children:'After the login is completed, click "New Instance" -> "Create Instance" in sequence, wait for the creation to be completed, and the steps in 3 will change to green in turn, and it will automatically switch to the subgraph MovieDemo1, as shown in the figure below.'}),"\n"]}),"\n"]}),"\n"]}),"\n",(0,s.jsxs)(n.p,{children:[(0,s.jsx)(n.img,{alt:"Login",src:t(84232).A+"",width:"1527",height:"1120"}),"\n",(0,s.jsx)(n.img,{alt:"Create Demo",src:t(16273).A+"",width:"1709",height:"592"})]}),"\n",(0,s.jsx)(n.h2,{id:"4common-faqs",children:"4.Common FAQs"}),"\n",(0,s.jsx)(n.h3,{id:"no-available-resources-in-the-selected-deployment-area",children:"No available resources in the selected deployment area"}),"\n",(0,s.jsx)(n.p,{children:"Sometimes, the selected deployment area (such as Availability Zone G) does not have available resources for the selected package, and an error will be reported as shown in the figure below."}),"\n",(0,s.jsx)(n.p,{children:(0,s.jsx)(n.img,{alt:"Deployment Error",src:t(18509).A+"",width:"596",height:"544"})}),"\n",(0,s.jsxs)(n.p,{children:[(0,s.jsx)(n.strong,{children:"Solution"}),":Try selecting other regions, such as Availability Zone I"]}),"\n",(0,s.jsx)(n.h3,{id:"unaccessible-to-the-web-port",children:"Unaccessible to the web port"}),"\n",(0,s.jsx)(n.p,{children:"It takes some time for the server to get ready. Just wait for a few seconds."}),"\n",(0,s.jsx)(n.h3,{id:"username-or-password-is-not-correct",children:"Username or password is not correct"}),"\n",(0,s.jsx)(n.p,{children:"Please make sure that you are using the password on the page instead of the default one."})]})}function h(e={}){const{wrapper:n}={...(0,i.R)(),...e.components};return n?(0,s.jsx)(n,{...e,children:(0,s.jsx)(d,{...e})}):d(e)}},77494:(e,n,t)=>{t.d(n,{A:()=>s});const s=t.p+"assets/images/cloud-deployment-6-66d573e42075d2367c35e77b3330e2ac.png"},79282:(e,n,t)=>{t.d(n,{A:()=>s});const s=t.p+"assets/images/cloud-deployment-2-2424fad73dbcd8267de47772bd60d29f.png"},84232:(e,n,t)=>{t.d(n,{A:()=>s});const s=t.p+"assets/images/cloud-deployment-8-3141da1eed5c4e147ecdef322cf1c58d.png"}}]);