"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[5564],{28453:(e,t,o)=>{o.d(t,{R:()=>a,x:()=>s});var r=o(96540);const i={},n=r.createContext(i);function a(e){const t=r.useContext(n);return r.useMemo((function(){return"function"==typeof e?e(t):{...t,...e}}),[t,e])}function s(e){let t;return t=e.disableParentContext?"function"==typeof e.components?e.components(i):e.components||i:a(e.components),r.createElement(n.Provider,{value:t},e.children)}},66816:(e,t,o)=>{o.r(t),o.d(t,{assets:()=>d,contentTitle:()=>a,default:()=>h,frontMatter:()=>n,metadata:()=>s,toc:()=>c});var r=o(74848),i=o(28453);const n={},a="Data Export",s={id:"utility-tools/data-export",title:"Data Export",description:"This document mainly introduces the data export function of TuGraph.",source:"@site/versions/version-4.2.0/en-US/source/6.utility-tools/2.data-export.md",sourceDirName:"6.utility-tools",slug:"/utility-tools/data-export",permalink:"/tugraph-db/en-US/en/4.2.0/utility-tools/data-export",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:2,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Data Importing",permalink:"/tugraph-db/en-US/en/4.2.0/utility-tools/data-import"},next:{title:"Backup and Restore",permalink:"/tugraph-db/en-US/en/4.2.0/utility-tools/backup-and-restore"}},d={},c=[{value:"1.Introduction",id:"1introduction",level:2},{value:"2.Data Export",id:"2data-export",level:2}];function l(e){const t={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",header:"header",li:"li",p:"p",pre:"pre",ul:"ul",...(0,i.R)(),...e.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(t.header,{children:(0,r.jsx)(t.h1,{id:"data-export",children:"Data Export"})}),"\n",(0,r.jsxs)(t.blockquote,{children:["\n",(0,r.jsx)(t.p,{children:"This document mainly introduces the data export function of TuGraph."}),"\n"]}),"\n",(0,r.jsx)(t.h2,{id:"1introduction",children:"1.Introduction"}),"\n",(0,r.jsxs)(t.p,{children:["TuGraph can use the tool ",(0,r.jsx)(t.code,{children:"lgraph_export"})," to export data from the database that has been imported successfully. The 'lgraph_export' tool can export the data of the specified TuGraph database to the specified directory in the form of 'csv' or 'json' file, and export the configuration file 'import.config'. That required for re-importing the data."]}),"\n",(0,r.jsx)(t.h2,{id:"2data-export",children:"2.Data Export"}),"\n",(0,r.jsx)(t.p,{children:"The following is an example of a command for the tool:"}),"\n",(0,r.jsx)(t.pre,{children:(0,r.jsx)(t.code,{className:"language-bash",children:"$ lgraph_export -d {database_dir} -e {export_destination_dir} -g {graph_to_use} -u {username} -p {password} -f {output_format}\n"})}),"\n",(0,r.jsx)(t.p,{children:"Details:"}),"\n",(0,r.jsxs)(t.ul,{children:["\n",(0,r.jsxs)(t.li,{children:[(0,r.jsx)(t.code,{children:"-d {database_dir}"})," specifies the directory of the database from which the data will be exported. The default value is./testdb '."]}),"\n",(0,r.jsxs)(t.li,{children:[(0,r.jsx)(t.code,{children:"-e {export_destination_dir}"})," specifies the directory where the export file is stored. The default value is./exportdir."]}),"\n",(0,r.jsxs)(t.li,{children:[(0,r.jsx)(t.code,{children:"-g {graph_to_use}"})," specifies the type of graph database. default is' default '."]}),"\n",(0,r.jsxs)(t.li,{children:[(0,r.jsx)(t.code,{children:"-u {username}"})," Specifies the name of the user who performs the export operation."]}),"\n",(0,r.jsxs)(t.li,{children:[(0,r.jsx)(t.code,{children:"-p {password}"})," Specifies the password of the user who performs the export operation."]}),"\n",(0,r.jsxs)(t.li,{children:[(0,r.jsx)(t.code,{children:"-s {field_separator}"})," specifies the separator for the exported file. The default is comma."]}),"\n",(0,r.jsxs)(t.li,{children:[(0,r.jsx)(t.code,{children:"-f {output_format}"})," specifies the format of the exported data. It can be 'json' or 'csv'."]}),"\n",(0,r.jsxs)(t.li,{children:[(0,r.jsx)(t.code,{children:"-h"})," In addition to the specified parameters, you can also use this parameter to view the help of the tool."]}),"\n"]})]})}function h(e={}){const{wrapper:t}={...(0,i.R)(),...e.components};return t?(0,r.jsx)(t,{...e,children:(0,r.jsx)(l,{...e})}):l(e)}}}]);