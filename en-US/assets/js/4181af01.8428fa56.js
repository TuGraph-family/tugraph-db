"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[98527],{28453:(e,n,t)=>{t.d(n,{R:()=>r,x:()=>o});var i=t(96540);const a={},s=i.createContext(a);function r(e){const n=i.useContext(s);return i.useMemo((function(){return"function"==typeof e?e(n):{...n,...e}}),[n,e])}function o(e){let n;return n=e.disableParentContext?"function"==typeof e.components?e.components(a):e.components||a:r(e.components),i.createElement(s.Provider,{value:n},e.children)}},99339:(e,n,t)=>{t.r(n),t.d(n,{assets:()=>d,contentTitle:()=>r,default:()=>h,frontMatter:()=>s,metadata:()=>o,toc:()=>c});var i=t(74848),a=t(28453);const s={},r="Scenarios",o={id:"introduction/scenarios",title:"Scenarios",description:"This document mainly introduces the application scenarios of graph databases.",source:"@site/versions/version-4.5.2/en-US/source/2.introduction/8.scenarios.md",sourceDirName:"2.introduction",slug:"/introduction/scenarios",permalink:"/tugraph-db/en-US/en/4.5.2/introduction/scenarios",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:8,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Functionality",permalink:"/tugraph-db/en-US/en/4.5.2/introduction/functionality"},next:{title:"Glossary",permalink:"/tugraph-db/en-US/en/4.5.2/introduction/glossary"}},d={},c=[{value:"1.Financial Industry",id:"1financial-industry",level:2},{value:"2.Industrial Industry",id:"2industrial-industry",level:2},{value:"3.Smart City",id:"3smart-city",level:2},{value:"4.Social Governance",id:"4social-governance",level:2},{value:"5.Internet",id:"5internet",level:2}];function l(e){const n={blockquote:"blockquote",h1:"h1",h2:"h2",header:"header",p:"p",strong:"strong",table:"table",tbody:"tbody",td:"td",th:"th",thead:"thead",tr:"tr",...(0,a.R)(),...e.components};return(0,i.jsxs)(i.Fragment,{children:[(0,i.jsx)(n.header,{children:(0,i.jsx)(n.h1,{id:"scenarios",children:"Scenarios"})}),"\n",(0,i.jsxs)(n.blockquote,{children:["\n",(0,i.jsx)(n.p,{children:"This document mainly introduces the application scenarios of graph databases."}),"\n"]}),"\n",(0,i.jsx)(n.h2,{id:"1financial-industry",children:"1.Financial Industry"}),"\n",(0,i.jsx)(n.p,{children:"The entities in the financial industry mainly involve people, companies, accounts, products, etc., and their relationships include transaction relationships, logging relationships, equity relationships, employment relationships, etc. These entities form a financial graph data network. By applying graph databases, we can discover a lot of useful information from the financial graph data network, which helps us make more accurate financial decisions."}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,i.jsxs)(n.table,{children:[(0,i.jsx)(n.thead,{children:(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Scenario"})}),(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Description"})})]})}),(0,i.jsxs)(n.tbody,{children:[(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Loan review"}),(0,i.jsx)(n.td,{children:"By analyzing the applicant's relationship and transaction history, etc., it is auxiliary to judge the applicant's repayment ability and willingness. It can be applied to retail loan review, small and micro loan review, supply chain finance, etc. This method can complement the traditional audit mechanism based on the applicant's own information, which is particularly useful for small and micro loan audits with low individual information coverage."})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Post-loan"}),(0,i.jsx)(n.td,{children:"By analyzing the borrower's transaction history, it assists in analyzing whether the borrower has default risk. Compared with the traditional method of issuing alerts only when loan funds flow to specific users, this method can assign a risk value to each account and issue warnings when the loan flows to high-risk accounts."})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Missing recovery"}),(0,i.jsx)(n.td,{children:"By analyzing the social and shopping data of missing borrowers, other contact methods can be found. This method can significantly improve the recovery rate of missing borrowers."})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Guarantor loop detection"}),(0,i.jsx)(n.td,{children:"Special guarantee structures such as cyclic, chain, family-style, and cross can be found in the guarantee relationship graph to expose potential risks. Compared with the JOIN method of a relational database, the solution based on graph algorithms is more efficient, can detect any length of guarantee loop, and can achieve more complex limited condition detection."})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Credit card fraud detection"}),(0,i.jsx)(n.td,{children:"A relationship network of addresses, contact information, etc. in credit card application information is constructed, and a community discovery algorithm is run in the application relationship network to detect suspected fraudulent gangs, thereby rejecting suspected fraudulent applications and reducing economic losses."})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Anti-money laundering"}),(0,i.jsx)(n.td,{children:"Suspected money laundering behaviors and links can be found through the transaction network and medium network. Money laundering is a complex process involving multiple parties. By conducting graph analysis in transaction and social networks, we can more accurately detect money laundering behaviors."})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Car insurance anti-fraud"}),(0,i.jsx)(n.td,{children:"For insurance fraud involving repair shops, by analyzing the relationship between the insured person, the case location, and the repair shop, fraudulent claims can be more accurately identified, reducing economic losses."})]})]})]}),"\n",(0,i.jsx)(n.h2,{id:"2industrial-industry",children:"2.Industrial Industry"}),"\n",(0,i.jsx)(n.p,{children:"A large amount of heterogeneous data will be generated in the production and manufacturing process, and how to effectively organize and manage these data is one of the most important issues in industrial big data. These data include design documents, equipment data, simulation plans and results, experimental results, experience documents, etc. The relationships between these data are complex, and traditional data management systems can only accumulate data, and they are often unable to search for related materials. Using graph models, these different types of data are organized into a network, which makes it easy to browse and search for data."}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,i.jsxs)(n.table,{children:[(0,i.jsx)(n.thead,{children:(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Scenario"})}),(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Description"})})]})}),(0,i.jsxs)(n.tbody,{children:[(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Supply chain management"}),(0,i.jsx)(n.td,{children:"Supply chain data mainly concerns the correspondence between products, components, and parts, the correspondence between parts and suppliers, and sensitive components in parts. Compared with the traditional BOM system, the solution using graph databases can more conveniently maintain complex networks of multiple component levels and multiple supplier levels, thereby providing basic support for a through supply chain."})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Document management"}),(0,i.jsx)(n.td,{children:"Using graph databases can organize different types of documents together organically according to different relationships. For example, design documents for components, component-part relationships, component test documents, relevant experience documents, etc. can be organized together so that all relevant information for the component can be obtained easily when needed."})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"R&D process management"}),(0,i.jsx)(n.td,{children:"A large amount of simulation, experimentation, and testing needs to be performed in the product development and verification process. Each test process involves many different steps, the connection between the steps, the data versions used in each step, and the algorithm versions all make up a complicated relationship network. Using graph databases can better manage this relationship network, providing a good foundation for data reuse and process improvement in the R&D process."})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Equipment information management"}),(0,i.jsx)(n.td,{children:"Manufacturing requires the management of a large number of devices, which are interrelated (power supply, feeding, spatial relationship) to form a complex network. Traditional databases are difficult to reflect this complex relationship. Using graph databases can easily represent these relationships, thereby better managing equipment information."})]})]})]}),"\n",(0,i.jsx)(n.h2,{id:"3smart-city",children:"3.Smart City"}),"\n",(0,i.jsx)(n.p,{children:"With the development of technology, intelligent management of cities has become a major trend. Intelligent management requires a strong system software to support it, built on a good information management platform. In an intelligent city management system, an intelligent decision-making system needs to make decisions based on a large amount of different information, including various topology information (such as roads, pipelines, etc.), supply and demand information (such as electricity transmission, drinking water supply, sewage discharge, etc.), environmental information (such as temperature, humidity, rainfall, etc.), and so on. To manage these complex and heterogeneous data organically and make decisions based on them, a mature system is needed. Traditional data management systems based on relational data models are not suitable for managing such complex and heterogeneous data. Using a graph model can solve this problem well. If we manage these different data using a graph database, we can achieve many complex intelligent management scenarios."}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,i.jsxs)(n.table,{children:[(0,i.jsx)(n.thead,{children:(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Scenario"})}),(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Description"})})]})}),(0,i.jsxs)(n.tbody,{children:[(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Smart Traffic"}),(0,i.jsx)(n.td,{children:"Based on road topology, road capacity, and current traffic flow, intelligent traffic signal scheduling can be performed to improve traffic efficiency."})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Smart Drainage"}),(0,i.jsx)(n.td,{children:"Based on drainage system information and current rainfall, the drainage system can be scheduled to reduce the occurrence of waterlogging."})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Pipeline Management"}),(0,i.jsx)(n.td,{children:"Organizing the production, installation, topology information, and historical status information of pipelines can help us manage the pipelines throughout their lifecycle, including fault diagnosis, life assessment, etc."})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Crowd Evacuation"}),(0,i.jsx)(n.td,{children:"When a large number of people need to be evacuated, multiple transportation modes such as buses, subways, taxis, shared bicycles, etc., need to be considered, as well as the road carrying capacity. Using a graph database to organically integrate this information can help us make better decisions and provide better support for large public events."})]})]})]}),"\n",(0,i.jsx)(n.h2,{id:"4social-governance",children:"4.Social Governance"}),"\n",(0,i.jsx)(n.p,{children:"Social governance involves public safety, legal affairs, public opinion, network security, and more. Social governance is a comprehensive, multi-system linkage problem. It requires the synthesis of a large amount of data and global considerations to make better decisions. In this multidimensional and complex data problem, the graph data model can provide better adaptability and provide a solid foundation for intelligent social governance decision-making platforms."}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,i.jsxs)(n.table,{children:[(0,i.jsx)(n.thead,{children:(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Scenario"})}),(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Description"})})]})}),(0,i.jsxs)(n.tbody,{children:[(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Criminal Gang"}),(0,i.jsx)(n.td,{children:"Conspiratorial gangs will inevitably make contact through some means, including face-to-face meetings (appearing in the same place), phone calls, or internet contacts, and they may also have economic exchanges. If we store all of this data in a unified graph, we can use graph analysis algorithms to identify closely connected groups and then discover and identify the entire criminal gang."})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Stakeholder Case Investigation"}),(0,i.jsx)(n.td,{children:"Stakeholder cases, especially stakeholder economic cases, often involve a large number of personnel and elements (money, location, events, etc.). How to effectively organize this information to provide evidence for handling cases is a difficulty in stakeholder case investigation. Using graph databases to store and analyze this information can help us quickly locate the data, analyze the personnel structure, and provide better technical support for case investigation."})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Illegal Website Identification"}),(0,i.jsx)(n.td,{children:'Illegal websites, phishing websites, yellow websites, and so on, often use different domain names and IPs to avoid being blocked. The filter method based on a blacklist can only block known illegal websites and cannot effectively identify new domain names and IPs. Based on the IP-domain name network mapping relationship, we can create a graph and then use graph calculations to establish a "trustworthiness" model for domain names and IPs to determine whether a website belongs to an illegal website.'})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Court File Management"}),(0,i.jsx)(n.td,{children:"Case files are often complex, and different cases may be linked through parties, locations, case nature, judges, and so on. These links constitute a complex network. Using a graph database, we can more conveniently manage these complex relationships and improve case handling and query efficiency."})]})]})]}),"\n",(0,i.jsx)(n.h2,{id:"5internet",children:"5.Internet"}),"\n",(0,i.jsx)(n.p,{children:"Social networks and person-product purchase relationships can all form a network. By analyzing this network data, we can provide users with better services, including relevant recommendations, user information collection, important user identification, and spam user identification, and so on."}),"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(0,i.jsxs)(n.table,{children:[(0,i.jsx)(n.thead,{children:(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Scenario"})}),(0,i.jsx)(n.th,{children:(0,i.jsx)(n.strong,{children:"Description"})})]})}),(0,i.jsxs)(n.tbody,{children:[(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"ID-Mapping"}),(0,i.jsx)(n.td,{children:"Graph databases can store all user-related information, including the relationships between users, in one database, and can also use these relationships to identify situations where one person has multiple accounts or multiple people share an account, thus providing decision support for future risk control, recommendations, and other businesses."})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Friend Recommendation"}),(0,i.jsx)(n.td,{children:'Based on the analysis of social networks, we can provide friend recommendations such as "friends of friends" and "common friends".'})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Product Recommendation"}),(0,i.jsx)(n.td,{children:"Based on the user-product relationship graph, we can find users with similar interests and recommend other products that similar users have chosen."})]}),(0,i.jsxs)(n.tr,{children:[(0,i.jsx)(n.td,{children:"Spam User Identification"}),(0,i.jsx)(n.td,{children:"Traditional spam user identification is mainly based on user account information, such as registration information and posting information, but these types of information are relatively easy to forge. On the other hand, network-based information doesn't have this problem: it's difficult to forge. Therefore, spam user identification based on network information can be more accurate and efficient."})]})]})]})]})}function h(e={}){const{wrapper:n}={...(0,a.R)(),...e.components};return n?(0,i.jsx)(n,{...e,children:(0,i.jsx)(l,{...e})}):l(e)}}}]);