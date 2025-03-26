"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[28359],{28453:(e,t,a)=>{a.d(t,{R:()=>o,x:()=>i});var r=a(96540);const s={},n=r.createContext(s);function o(e){const t=r.useContext(n);return r.useMemo((function(){return"function"==typeof e?e(t):{...t,...e}}),[t,e])}function i(e){let t;return t=e.disableParentContext?"function"==typeof e.components?e.components(s):e.components||s:o(e.components),r.createElement(n.Provider,{value:t},e.children)}},96042:(e,t,a)=>{a.r(t),a.d(t,{assets:()=>c,contentTitle:()=>o,default:()=>l,frontMatter:()=>n,metadata:()=>i,toc:()=>d});var r=a(74848),s=a(28453);const n={},o="Backup and Restore",i={id:"utility-tools/backup-and-restore",title:"Backup and Restore",description:"This document mainly introduces the data backup and restore function of TuGraph.",source:"@site/versions/version-4.5.2/en-US/source/6.utility-tools/3.backup-and-restore.md",sourceDirName:"6.utility-tools",slug:"/utility-tools/backup-and-restore",permalink:"/tugraph-db/en/4.5.2/utility-tools/backup-and-restore",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:3,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Data Export",permalink:"/tugraph-db/en/4.5.2/utility-tools/data-export"},next:{title:"Data Warmup",permalink:"/tugraph-db/en/4.5.2/utility-tools/data-warmup"}},c={},d=[{value:"1.Data Backup",id:"1data-backup",level:2},{value:"2.Data Restore",id:"2data-restore",level:2}];function u(e){const t={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",header:"header",li:"li",p:"p",pre:"pre",ul:"ul",...(0,s.R)(),...e.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(t.header,{children:(0,r.jsx)(t.h1,{id:"backup-and-restore",children:"Backup and Restore"})}),"\n",(0,r.jsxs)(t.blockquote,{children:["\n",(0,r.jsx)(t.p,{children:"This document mainly introduces the data backup and restore function of TuGraph."}),"\n"]}),"\n",(0,r.jsx)(t.h2,{id:"1data-backup",children:"1.Data Backup"}),"\n",(0,r.jsx)(t.p,{children:"TuGraph can use the 'lgraph_backup' tool to backup data.\nThe 'lgraph_backup' tool can backup data from a TuGraph database to another directory. It can be used as follows:"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-bash",children:"$ lgraph_backup -s {source_dir} -d {destination_dir} -c {true/false}\n"})}),"\n",(0,r.jsx)(t.p,{children:"Details:"}),"\n",(0,r.jsxs)(t.ul,{children:["\n",(0,r.jsxs)(t.li,{children:[(0,r.jsx)(t.code,{children:"-s {source_dir}"})," specifies the directory where the database (source database) to be backed up resides."]}),"\n",(0,r.jsxs)(t.li,{children:[(0,r.jsx)(t.code,{children:"-d {destination_dir}"})," specifies the directory where the backup file (destination database) is stored.\nIf the target database is not empty, 'lgraph_backup' prompts you whether to overwrite the database."]}),"\n",(0,r.jsxs)(t.li,{children:[(0,r.jsx)(t.code,{children:"-c {true/false}"})," indicates whether a compaction occurs during backup.\nEvery compaction creates a tighter backup, but every compaction takes longer to create. This option defaults to 'true'."]}),"\n"]}),"\n",(0,r.jsx)(t.h2,{id:"2data-restore",children:"2.Data Restore"}),"\n",(0,r.jsxs)(t.p,{children:["The target database ",(0,r.jsx)(t.code,{children:"{destination_dir}"})," obtained using the ",(0,r.jsx)(t.code,{children:"lgraph_backup"})," tool backs up all subgraphs of the source database ",(0,r.jsx)(t.code,{children:"{source_dir}"}),", but does not include the raft information of the HA cluster, thereby ensuring that the service and cluster can be successfully restarted with the backup database and the data is consistent with the source database. The following command can be used to restart the service with the backup database. When the service starts, the storage process of all subgraphs will be restored to ensure that the backup service is completely consistent with the original service."]}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-bash",children:"$ lgraph_server -c lgraph.json --directory {destination_dir} -d start\n"})}),"\n",(0,r.jsx)(t.p,{children:"Details"}),"\n",(0,r.jsxs)(t.ul,{children:["\n",(0,r.jsxs)(t.li,{children:[(0,r.jsx)(t.code,{children:"-d {destination_dir}"})," Specify the directory where the backup file (target database) is located."]}),"\n"]})]})}function l(e={}){const{wrapper:t}={...(0,s.R)(),...e.components};return t?(0,r.jsx)(t,{...e,children:(0,r.jsx)(u,{...e})}):u(e)}}}]);