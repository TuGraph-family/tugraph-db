"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[23908],{28453:(e,s,n)=>{n.d(s,{R:()=>o,x:()=>a});var r=n(96540);const i={},l=r.createContext(i);function o(e){const s=r.useContext(l);return r.useMemo((function(){return"function"==typeof e?e(s):{...s,...e}}),[s,e])}function a(e){let s;return s=e.disableParentContext?"function"==typeof e.components?e.components(i):e.components||i:o(e.components),r.createElement(l.Provider,{value:s},e.children)}},64417:(e,s,n)=>{n.r(s),n.d(s,{assets:()=>c,contentTitle:()=>o,default:()=>h,frontMatter:()=>l,metadata:()=>a,toc:()=>d});var r=n(74848),i=n(28453);const l={},o="Privilege",a={id:"permission/privilege",title:"Privilege",description:"1.Introduce",source:"@site/versions/version-4.3.0/en-US/source/10.permission/1.privilege.md",sourceDirName:"10.permission",slug:"/permission/privilege",permalink:"/tugraph-db/en-US/en/4.3.0/permission/privilege",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:1,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Heterogeneous Graph",permalink:"/tugraph-db/en-US/en/4.3.0/olap&procedure/learn/heterogeneous_graph"},next:{title:"Token Usage Guide",permalink:"/tugraph-db/en-US/en/4.3.0/permission/token"}},c={},d=[{value:"1.Introduce",id:"1introduce",level:2},{value:"2.Level of permissions",id:"2level-of-permissions",level:2},{value:"3.Permission keyword",id:"3permission-keyword",level:2},{value:"4.Common permission operations",id:"4common-permission-operations",level:2},{value:"4.1.User action",id:"41user-action",level:3},{value:"4.2.Role actions",id:"42role-actions",level:3},{value:"4.3.Assign roles to users",id:"43assign-roles-to-users",level:3},{value:"4.4.Role empowerment",id:"44role-empowerment",level:3}];function t(e){const s={blockquote:"blockquote",code:"code",h1:"h1",h2:"h2",h3:"h3",header:"header",li:"li",p:"p",pre:"pre",ul:"ul",...(0,i.R)(),...e.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(s.header,{children:(0,r.jsx)(s.h1,{id:"privilege",children:"Privilege"})}),"\n",(0,r.jsx)(s.h2,{id:"1introduce",children:"1.Introduce"}),"\n",(0,r.jsxs)(s.blockquote,{children:["\n",(0,r.jsx)(s.p,{children:"The permissions of TuGraph are managed based on role-based access control. The permissions that define access control are assigned to roles, and the roles are then assigned to users."}),"\n"]}),"\n",(0,r.jsx)(s.h2,{id:"2level-of-permissions",children:"2.Level of permissions"}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"Global layer: indicates global permissions, which have permissions for management and graph operations."}),"\n",(0,r.jsx)(s.li,{children:"Graph layer: control permissions on each graph;"}),"\n",(0,r.jsx)(s.li,{children:"Property level (Commercial version only) : control permissions on a property"}),"\n"]}),"\n",(0,r.jsx)(s.h2,{id:"3permission-keyword",children:"3.Permission keyword"}),"\n",(0,r.jsx)(s.p,{children:"At present, the control of permissions is relatively simple"}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"The Global layer currently has admin permission, and the admin user is preset."}),"\n"]}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"The Graph layer has four operation permissions: none, read, write, and full"}),"\n"]}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"none: No permission, no operation permission for the graph"}),"\n",(0,r.jsx)(s.li,{children:"read: Read-only permission, only has read permission for the graph"}),"\n",(0,r.jsx)(s.li,{children:"write: Read and write permission, not only has read permission for the graph, but also has write permission"}),"\n",(0,r.jsx)(s.li,{children:"full: All permissions, not only have read and write permissions for graphs, but also have permissions to delete graphs, modify graphs, and modify schemas"}),"\n"]}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"The Property layer (Commercial version only)  has the following permissions: none, read, and write"}),"\n"]}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"none: No permission, no operation permission for the property"}),"\n",(0,r.jsx)(s.li,{children:"read: Read-only permission, only has read permission for the property"}),"\n",(0,r.jsx)(s.li,{children:"write: Read and write permission, not only has read permission for the property, but also has write permission"}),"\n"]}),"\n",(0,r.jsx)(s.h2,{id:"4common-permission-operations",children:"4.Common permission operations"}),"\n",(0,r.jsx)(s.h3,{id:"41user-action",children:"4.1.User action"}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"Creating a user"}),"\n"]}),"\n",(0,r.jsx)(s.pre,{children:(0,r.jsx)(s.code,{className:"language-cypher",children:"CALL dbms.security.createUser(user_name::STRING,password::STRING)\n"})}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"Deleting a user"}),"\n"]}),"\n",(0,r.jsx)(s.pre,{children:(0,r.jsx)(s.code,{className:"language-cypher",children:"CALL dbms.security.deleteUser(user_name::STRING)\n"})}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"Change the password of the current user"}),"\n"]}),"\n",(0,r.jsx)(s.pre,{children:(0,r.jsx)(s.code,{className:"language-cypher",children:"CALL dbms.security.changePassword(current_password::STRING,new_password::STRING)\n"})}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"Changes the password of a specified user"}),"\n"]}),"\n",(0,r.jsx)(s.pre,{children:(0,r.jsx)(s.code,{className:"language-cypher",children:"\nCALL dbms.security.changeUserPassword(user_name::STRING,new_password::STRING)\n"})}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"Disable or enable a user"}),"\n"]}),"\n",(0,r.jsx)(s.pre,{children:(0,r.jsx)(s.code,{className:"language-cypher",children:"CALL dbms.security.disableUser(user::STRING,disable::BOOLEAN)\n"})}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"List all users"}),"\n"]}),"\n",(0,r.jsx)(s.pre,{children:(0,r.jsx)(s.code,{className:"language-cypher",children:"CALL dbms.security.listUsers()\n"})}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"Lists the current user information"}),"\n"]}),"\n",(0,r.jsx)(s.pre,{children:(0,r.jsx)(s.code,{className:"language-cypher",children:"CALL dbms.security.showCurrentUser()\n"})}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"Obtain user details"}),"\n"]}),"\n",(0,r.jsx)(s.pre,{children:(0,r.jsx)(s.code,{className:"language-cypher",children:"CALL dbms.security.getUserInfo(user::STRING)\n"})}),"\n",(0,r.jsx)(s.h3,{id:"42role-actions",children:"4.2.Role actions"}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"Create a role"}),"\n"]}),"\n",(0,r.jsx)(s.pre,{children:(0,r.jsx)(s.code,{className:"language-cypher",children:"CALL dbms.security.createRole(role_name::STRING,desc::STRING)\n"})}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"Delete a role"}),"\n"]}),"\n",(0,r.jsx)(s.pre,{children:(0,r.jsx)(s.code,{className:"language-cypher",children:"CALL dbms.security.deleteRole(role_name::STRING\n"})}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"List all characters"}),"\n"]}),"\n",(0,r.jsx)(s.pre,{children:(0,r.jsx)(s.code,{className:"language-cypher",children:"CALL dbms.security.listRoles()\n"})}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"Disable or enable the role"}),"\n"]}),"\n",(0,r.jsx)(s.pre,{children:(0,r.jsx)(s.code,{className:"language-cypher",children:"CALL dbms.security.disableRole(role::STRING,disable::BOOLEAN)\n"})}),"\n",(0,r.jsx)(s.h3,{id:"43assign-roles-to-users",children:"4.3.Assign roles to users"}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"Adds the association between the user and the role"}),"\n"]}),"\n",(0,r.jsx)(s.pre,{children:(0,r.jsx)(s.code,{className:"language-cypher",children:"CALL dbms.security.addUserRoles(user::STRING,roles::LIST)\n"})}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"Deletes the association between the user and the role"}),"\n"]}),"\n",(0,r.jsx)(s.p,{children:"CALL dbms.security.deleteUserRoles(user::STRING,roles::LIST)"}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"Clears the relationship between user roles and rebuilds them"}),"\n"]}),"\n",(0,r.jsx)(s.pre,{children:(0,r.jsx)(s.code,{className:"language-cypher",children:"CALL dbms.security.rebuildUserRoles(user::STRING,roles::LIST)\n"})}),"\n",(0,r.jsx)(s.h3,{id:"44role-empowerment",children:"4.4.Role empowerment"}),"\n",(0,r.jsxs)(s.ul,{children:["\n",(0,r.jsx)(s.li,{children:"Modifies the access permission of a role to a specified graph"}),"\n"]}),"\n",(0,r.jsx)(s.pre,{children:(0,r.jsx)(s.code,{className:"language-cypher",children:"CALL dbms.security.modRoleAccessLevel(role::STRING,access_level::MAP)\n"})}),"\n",(0,r.jsx)(s.p,{children:"Example"}),"\n",(0,r.jsx)(s.pre,{children:(0,r.jsx)(s.code,{className:"language-cypher",children:'CALL dbms.security.modRoleAccessLevel("test_role", {test_graph1:"FULL", test_graph2:"NONE"})\n'})})]})}function h(e={}){const{wrapper:s}={...(0,i.R)(),...e.components};return s?(0,r.jsx)(s,{...e,children:(0,r.jsx)(t,{...e})}):t(e)}}}]);