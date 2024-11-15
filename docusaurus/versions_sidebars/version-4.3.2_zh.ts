import type { SidebarsConfig } from "@docusaurus/plugin-content-docs";

/**
 * Creating a sidebar enables you to:
 - create an ordered group of docs
 - render a sidebar for each doc of that group
 - provide next/previous navigation

 The sidebars can be generated from the filesystem, or explicitly defined here.

 Create as many sidebars as you want.
 */
const sidebars: SidebarsConfig = {
  // By default, Docusaurus generates a sidebar from the docs folder structure
  // tutorialSidebar: [{type: 'autogenerated', dirName: '.'}],

  // But you can create a sidebar manually
  tutorialSidebar: [
    "guide",
    {
      type: "category",
      label: "Tugraph 入门",
      items: [
        "introduction/what-is-graph",
        "introduction/what-is-gdbms",
        "introduction/what-is-tugraph",
        "introduction/schema",
        {
          type: "category",
          label: "产品特点",
          items: [
            "introduction/characteristics/performance-oriented",
            "introduction/characteristics/multi-level-Interfaces",
            "introduction/characteristics/htap",
          ],
        },
        "introduction/architecture",
        "introduction/functionality",
        "introduction/scenarios",
        "introduction/glossary",
      ],
    },
    {
      type: "category",
      label: "快速上手",
      items: [
        "quick-start/preparation",
        {
          type: "category",
          label: "demo 示例",
          items: [
            "quick-start/demo/movie",
            "quick-start/demo/wandering-earth",
            "quick-start/demo/the-three-body",
            "quick-start/demo/three-kingdoms",
            "quick-start/demo/round-the-world",
          ],
        },
      ],
    },
    {
      type: "category",
      label: "可视化操作指南",
      items: [
        "user-guide/tugraph-browser",
        "user-guide/tugraph-browser-legacy",
      ],
    },
    "development_guide",
    {
      type: "category",
      label: "安装和运行",
      items: [
        "installation&running/environment",
        "installation&running/environment-mode",
        "installation&running/docker-deployment",
        "installation&running/local-package-deployment",
        "installation&running/cloud-deployment",
        "installation&running/compile",
        "installation&running/tugraph-running",
        "installation&running/high-availability-mode",
      ],
    },
    {
      type: "category",
      label: "实用工具",
      items: [
        "utility-tools/data-import",
        "utility-tools/data-export",
        "utility-tools/backup-and-restore",
        "utility-tools/data-warmup",
        "utility-tools/ha-cluster-management",
        "utility-tools/tugraph-cli",
        "utility-tools/tugraph-datax",
        "utility-tools/tugraph-explorer",
        "utility-tools/restful",
      ],
    },
    {
      type: "category",
      label: "客户端工具",
      items: [
        "client-tools/python-client",
        "client-tools/cpp-client",
        "client-tools/java-client",
        "client-tools/tugraph-ogm",
        "client-tools/bolt-client",
        "client-tools/bolt-console-client",
        "client-tools/restful-api",
        "client-tools/rpc-api",
        "client-tools/restful-api-legacy",
      ],
    },
    {
      type: "category",
      label: "查询语言",
      items: ["query/cypher", "query/gql"],
    },
    {
      type: "category",
      label: "存储过程和分析接口",
      items: [
        {
          type: "category",
          label: "存储过程",
          link: {
            type: "doc",
            id: "olap&procedure/procedure/procedure",
          },
          items: [
            "olap&procedure/procedure/traversal",
            'olap&procedure/procedure/C++-procedure',
            'olap&procedure/procedure/Python-procedure',
            "olap&procedure/procedure/Rust-procedure",
          ],
        },
        {
          type: "category",
          label: "分析接口",
          items: [
            "olap&procedure/olap/tutorial",
            "olap&procedure/olap/olap-base-api",
            "olap&procedure/olap/olap-on-db-api",
            "olap&procedure/olap/olap-on-disk-api",
            "olap&procedure/olap/python-api",
            "olap&procedure/olap/algorithms",
          ],
        },
        {
          type: "category",
          label: "图学习",
          items: [
            "olap&procedure/learn/tutorial",
            "olap&procedure/learn/sampling_api",
            "olap&procedure/learn/training",
            "olap&procedure/learn/heterogeneous_graph",
          ],
        },
      ],
    },
    {
      type: "category",
      label: "运维和权限管理",
      items: [
        "permission/privilege",
        "permission/token",
        "permission/reset_admin_password",
        "permission/monitoring",
        "permission/log",
      ],
    },
    {
      type: "category",
      label: "测试与质量保障",
      items: ["quality/unit-testing", "quality/integration-testing"],
    },
    {
      type: "category",
      label: "贡献者文档",
      items: [
        "contributor-manual/contributing",
        "contributor-manual/community-roles",
        "contributor-manual/individual-cla",
        "contributor-manual/corporate-cla",
        "contributor-manual/roadmap",
      ],
    },
    {
      type: "category",
      label: "最佳实践",
      items: [
        "best-practices/rdbms-to-tugraph",
        "best-practices/learn_practices",
        "best-practices/data_migration",
        "best-practices/selection",
        "best-practices/spatial",
      ],
    },
    "faq",
    "contacts",
  ],
};

export default sidebars;
