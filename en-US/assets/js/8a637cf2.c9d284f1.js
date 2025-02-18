"use strict";(self.webpackChunkdocusaurus=self.webpackChunkdocusaurus||[]).push([[489],{28453:(e,t,n)=>{n.d(t,{R:()=>a,x:()=>s});var i=n(96540);const o={},r=i.createContext(o);function a(e){const t=i.useContext(r);return i.useMemo((function(){return"function"==typeof e?e(t):{...t,...e}}),[t,e])}function s(e){let t;return t=e.disableParentContext?"function"==typeof e.components?e.components(o):e.components||o:a(e.components),i.createElement(r.Provider,{value:t},e.children)}},88415:(e,t,n)=>{n.r(t),n.d(t,{assets:()=>c,contentTitle:()=>a,default:()=>d,frontMatter:()=>r,metadata:()=>s,toc:()=>h});var i=n(74848),o=n(28453);const r={},a="TuGraph Contribution Guide",s={id:"contributor-manual/contributing",title:"TuGraph Contribution Guide",description:"1. Preface",source:"@site/versions/version-4.1.0/en-US/source/6.contributor-manual/1.contributing.md",sourceDirName:"6.contributor-manual",slug:"/contributor-manual/contributing",permalink:"/tugraph-db/en-US/en/4.1.0/contributor-manual/contributing",draft:!1,unlisted:!1,tags:[],version:"current",sidebarPosition:1,frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Forgot Admin Password",permalink:"/tugraph-db/en-US/en/4.1.0/developer-manual/other/reset_admin_password"},next:{title:"TuGraph community roles",permalink:"/tugraph-db/en-US/en/4.1.0/contributor-manual/community-roles"}},c={},h=[{value:"1. Preface",id:"1-preface",level:2},{value:"2. What to contribute",id:"2-what-to-contribute",level:2},{value:"3. Preparation",id:"3-preparation",level:2},{value:"3.1. Knowing TuGraph",id:"31-knowing-tugraph",level:2},{value:"3.2. Environment",id:"32-environment",level:2},{value:"3.3. License Agreement",id:"33-license-agreement",level:2},{value:"4. Contribute code process",id:"4-contribute-code-process",level:2},{value:"4.1. Submit the issue",id:"41-submit-the-issue",level:3},{value:"4.2. Pull the branch",id:"42-pull-the-branch",level:3},{value:"4.3. Configuring Github Information",id:"43-configuring-github-information",level:3},{value:"4.4. Compile and Run",id:"44-compile-and-run",level:3},{value:"4.5. Change the code and submit it locally",id:"45-change-the-code-and-submit-it-locally",level:3},{value:"4.6. Code Review",id:"46-code-review",level:3},{value:"4.7. Merge code into Master",id:"47-merge-code-into-master",level:3}];function u(e){const t={a:"a",h1:"h1",h2:"h2",h3:"h3",header:"header",li:"li",ol:"ol",p:"p",strong:"strong",ul:"ul",...(0,o.R)(),...e.components};return(0,i.jsxs)(i.Fragment,{children:[(0,i.jsx)(t.header,{children:(0,i.jsx)(t.h1,{id:"tugraph-contribution-guide",children:"TuGraph Contribution Guide"})}),"\n",(0,i.jsx)(t.h2,{id:"1-preface",children:"1. Preface"}),"\n",(0,i.jsx)(t.p,{children:"Thank you for your contribution to TuGraph. We sincerely hope that more students from the community will join in and build a better graph database project together."}),"\n",(0,i.jsx)(t.p,{children:"For outstanding community contributors, we will award the TuGraph Outstanding Community Contributor Certificate."}),"\n",(0,i.jsx)(t.h2,{id:"2-what-to-contribute",children:"2. What to contribute"}),"\n",(0,i.jsx)(t.p,{children:"We welcome any contribution at any time, whether it's a simple typo fix, bug fix, or new feature addition. Feel free to ask questions or initiate PRs. We also value documentation and integration with other open source projects, and welcome contributions in this regard. For any modification, especially the more complex modification, it is recommended to create an issue and fill it out according to the BUG or PR template."}),"\n",(0,i.jsx)(t.h2,{id:"3-preparation",children:"3. Preparation"}),"\n",(0,i.jsx)(t.h2,{id:"31-knowing-tugraph",children:"3.1. Knowing TuGraph"}),"\n",(0,i.jsxs)(t.p,{children:["You can get start with TuGraph follow instrcutions in ",(0,i.jsx)(t.a,{href:"/tugraph-db/en-US/en/4.1.0/guide",children:"Guide Doc"}),"."]}),"\n",(0,i.jsx)(t.h2,{id:"32-environment",children:"3.2. Environment"}),"\n",(0,i.jsxs)(t.p,{children:['For document contributions, you can directly modify and submit a pull request by clicking on "Edit on GitHub" in the upper right corner of the ',(0,i.jsx)(t.a,{href:"https://tugraph-db.readthedocs.io/en/latest",children:"documentation"}),"."]}),"\n",(0,i.jsxs)(t.p,{children:["For code contributions, it is usually necessary to set up the environment for compilation and execution. You can deploy using ",(0,i.jsx)(t.a,{href:"/tugraph-db/en-US/en/4.1.0/developer-manual/installation/docker-deployment",children:"Docker"})," or ",(0,i.jsx)(t.a,{href:"/tugraph-db/en-US/en/4.1.0/developer-manual/installation/local-package-deployment",children:"local package deployment"}),"."]}),"\n",(0,i.jsx)(t.h2,{id:"33-license-agreement",children:"3.3. License Agreement"}),"\n",(0,i.jsxs)(t.p,{children:["Before contributing code, please take some time to understand the process of contributing code to TuGraph, and read the ",(0,i.jsx)(t.a,{href:"/tugraph-db/en-US/en/4.1.0/contributor-manual/individual-cla",children:"Contributor License Agreement"})," or ",(0,i.jsx)(t.a,{href:"/tugraph-db/en-US/en/4.1.0/contributor-manual/corporate-cla",children:"Corporate Contributor License Agreement"}),". Participating in the contribution is deemed to agree to the above agreement."]}),"\n",(0,i.jsx)(t.h2,{id:"4-contribute-code-process",children:"4. Contribute code process"}),"\n",(0,i.jsx)(t.h3,{id:"41-submit-the-issue",children:"4.1. Submit the issue"}),"\n",(0,i.jsx)(t.p,{children:"Whether you are fixing a bug in TuGraph or adding a new feature to TuGraph, before you submit the code, submit an issue on TuGraph's GitHub, describing the problem you want to fix or the function you want to add. There are several advantages to doing this:"}),"\n",(0,i.jsxs)(t.ul,{children:["\n",(0,i.jsx)(t.li,{children:"There will be no duplication of work in conflict with other developers or their plans for the project."}),"\n",(0,i.jsx)(t.li,{children:"TuGraph maintainers will discuss the bugs or new features you mentioned to determine whether the modification is necessary, whether there is room for improvement or a better way."}),"\n",(0,i.jsx)(t.li,{children:"After reaching an agreement, develop and submit the code, reducing the cost of communication between the two parties and reducing the rejection of pull requests."}),"\n"]}),"\n",(0,i.jsx)(t.h3,{id:"42-pull-the-branch",children:"4.2. Pull the branch"}),"\n",(0,i.jsx)(t.p,{children:"All modifications of TuGraph are made on branches. After modification, the pull request is submitted and merged into the Master by the project maintenance personnel after Code Review. Therefore, after the source code steps are described, you need:"}),"\n",(0,i.jsxs)(t.ol,{children:["\n",(0,i.jsxs)(t.li,{children:["Download the code to a local directory. In this step, you can choose git or https. In recent years, github has stricter permission requirements. The git approach, for example, requires the more complex ssh key(",(0,i.jsx)(t.a,{href:"https://docs.github.com/en/authentication/connecting-to-github-with-ssh/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent",children:"https://docs.github.com/en/authentication/connecting-to-github-with-ssh/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent"}),"). https authentication cannot be performed using the user name and password. git clone ",(0,i.jsx)(t.a,{href:"https://github.com/",children:"https://github.com/"})," Your account name /tugraph-db"]}),"\n",(0,i.jsx)(t.li,{children:"Pull branch ready to modify code:\ngit branch add_xxx_feature"}),"\n",(0,i.jsx)(t.li,{children:"After executing the above command, your code repository switches to the appropriate branch. Run the following command to view your current branch: git branch -a"}),"\n",(0,i.jsx)(t.li,{children:"If you want to switch back to Master, run the git checkout -b master command"}),"\n",(0,i.jsx)(t.li,{children:'If you want to switch back to branching, run the following command: git checkout -b "branchName"'}),"\n"]}),"\n",(0,i.jsx)(t.h3,{id:"43-configuring-github-information",children:"4.3. Configuring Github Information"}),"\n",(0,i.jsx)(t.p,{children:"Run git config --list on your machine to view git's global username and mailbox. Check whether the displayed user.name and user.email match your github user name and email address."}),"\n",(0,i.jsx)(t.p,{children:"If a company has its own gitlab in-house or uses other commercial GitLabs, a mismatch may occur. At this point, you need to set up a separate user name and mailbox for the tugraph-db project. For details about how to set the user name and email address, see github's official documentation."}),"\n",(0,i.jsx)(t.h3,{id:"44-compile-and-run",children:"4.4. Compile and Run"}),"\n",(0,i.jsxs)(t.p,{children:["The process of compile and run can reference ",(0,i.jsx)(t.a,{href:"https://github.com/TuGraph-family/tugraph-db/blob/master/ci/github_ci.sh",children:"ci script"})]}),"\n",(0,i.jsx)(t.h3,{id:"45-change-the-code-and-submit-it-locally",children:"4.5. Change the code and submit it locally"}),"\n",(0,i.jsx)(t.p,{children:"Once you've pulled the branch, you're ready to change the code."}),"\n",(0,i.jsx)(t.p,{children:(0,i.jsx)(t.strong,{children:"Code modification Notes"})}),"\n",(0,i.jsx)(t.p,{children:"Code style consistency: TuGraph uses cpplint to keep code in the same format, and IDE can use.clang to configure the style. Be sure to check the code style locally before submitting it, or ACI will report an error."}),"\n",(0,i.jsx)(t.p,{children:"Add unit test code: New changes should pass through existing unit tests. New unit tests should be provided to prove that there are bugs in the previous code and that the new code has resolved these bugs. You can run all tests with the following command:./unit_test\nIt can also be run with the help of an IDE."}),"\n",(0,i.jsx)(t.p,{children:(0,i.jsx)(t.strong,{children:"Other precautions"})}),"\n",(0,i.jsx)(t.p,{children:"Please keep the code you are editing in the original style, especially the space feed, etc. For unnecessary comments, delete them directly. Add comments where logic and functionality are not easily understood. Keep documentation up to date. After modifying the code, run the following command to submit all the changes to the local computer:\ngit commit -am '(feat) Add the xx function 'git commit -am '(fix) fix xx problem' '"}),"\n",(0,i.jsx)(t.p,{children:'Submit the code to the remote repository\nAfter the code is committed locally, you can then synchronize the code with the remote repository. Run the following command to submit local changes to github: git push origin "branchname"'}),"\n",(0,i.jsx)(t.p,{children:"If you were forking earlier, the origin here is pushed to your code repository, not TuGraph's."}),"\n",(0,i.jsx)(t.p,{children:"After the code has been submitted to GitHub, you can send a request to merge your changes into the TuGraph Master. At this point, you need to go to your corresponding repository on GitHub and press the pull request button in the upper right corner. Select the target branch, which is usually the master, and the system will notify the TuGraph staff, who will Review your code, and when it meets the requirements, it will join the main branch and become part of TuGraph."}),"\n",(0,i.jsx)(t.p,{children:"Please note that the CI will be automatically checked, as well as all Commits signing the cla, with green signs on the commits."}),"\n",(0,i.jsx)(t.h3,{id:"46-code-review",children:"4.6. Code Review"}),"\n",(0,i.jsx)(t.p,{children:"After you submit your code, your code will be assigned to a maintainer for Review. Please wait patiently. If no one has responded to your submission after two working days, you can leave a message under PR and @ the corresponding person."}),"\n",(0,i.jsx)(t.p,{children:"Comments on code Review will be directly noted to the corresponding PR or Issue. If you find the suggestions reasonable, please update them to your code."}),"\n",(0,i.jsx)(t.h3,{id:"47-merge-code-into-master",children:"4.7. Merge code into Master"}),"\n",(0,i.jsx)(t.p,{children:"After the code Review is approved, the TuGraph maintainer will put it into the Master. During this process, the maintainer may specify a new Reviewer and put forward new comments that need to be revised. Normally this step is omitted, and after the code is merged, you will receive an indication that the merge was successful."})]})}function d(e={}){const{wrapper:t}={...(0,o.R)(),...e.components};return t?(0,i.jsx)(t,{...e,children:(0,i.jsx)(u,{...e})}):u(e)}}}]);