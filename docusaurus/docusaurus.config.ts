import { themes as prismThemes } from "prism-react-renderer";
import type { Config } from "@docusaurus/types";
import type { Options as DocsOptions } from "@docusaurus/plugin-content-docs";
import type * as Preset from "@docusaurus/preset-classic";

const config: Config = {
  title: "TuGraph 文档中心",
  tagline:
    "高性能图数据库TuGraph由蚂蚁集团和清华大学共同研发，历经蚂蚁实际业务场景锤炼，在国际图数据库基准测试中获得性能第一",
  favicon:
    "https://mdn.alipayobjects.com/huamei_qcdryc/afts/img/A*AbamQ5lxv0IAAAAAAAAAAAAADgOBAQ/original",

  // Set the production url of your site here
  url: "https://tugraph.tech/",
  // Set the /<baseUrl>/ pathname under which your site is served
  // For GitHub pages deployment, it is often '/<projectName>/'
  baseUrl: "/tugraph-db",

  // GitHub pages deployment config.
  // If you aren't using GitHub pages, you don't need these.
  organizationName: "facebook", // Usually your GitHub org/user name.
  projectName: "Tugraph Docs", // Usually your repo name.

  onBrokenLinks: "warn",
  onBrokenMarkdownLinks: "warn",
  onBrokenAnchors: "warn",

  trailingSlash: false,

  markdown: {
    format: "md",
    mermaid: true,
  },

  i18n: {
    defaultLocale: "zh-CN",
    locales: ["zh-CN", "en-US"],
  },

  presets: [
    [
      "classic",
      {
        docs: {
          sidebarPath: "./sidebars.ts",
          path: "../docs/zh-CN/source",
          routeBasePath: "zh/latest",
        },
        theme: {
          customCss: "./src/css/custom.css",
        },
      } satisfies Preset.Options,
    ],
  ],

  plugins: [
    [
      "content-docs",
      {
        id: "latest-en",
        sidebarPath: "./sidebarsEn.ts",
        path: "../docs/en-US/source",
        routeBasePath: "en/latest",
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "4-5-0_en",
        sidebarPath: "./versions_sidebars/version-4.5.0_en.ts",
        path: "./versions/version-4.5.0/en-US/source",
        routeBasePath: "en",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "4-5-0_zh",
        sidebarPath: "./versions_sidebars/version-4.5.0_zh.ts",
        path: "./versions/version-4.5.0/zh-CN/source",
        routeBasePath: "zh",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "4-3-2_en",
        sidebarPath: "./versions_sidebars/version-4.3.2_en.ts",
        path: "./versions/version-4.3.2/en-US/source",
        routeBasePath: "en/4.3.2",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "4-3-2_zh",
        sidebarPath: "./versions_sidebars/version-4.3.2_zh.ts",
        path: "./versions/version-4.3.2/zh-CN/source",
        routeBasePath: "zh/4.3.2",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "4-3-1_en",
        sidebarPath: "./versions_sidebars/version-4.3.1_en.ts",
        path: "./versions/version-4.3.1/en-US/source",
        routeBasePath: "en/4.3.1",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "4-3-1_zh",
        sidebarPath: "./versions_sidebars/version-4.3.1_zh.ts",
        path: "./versions/version-4.3.1/zh-CN/source",
        routeBasePath: "zh/4.3.1",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "4-3-0_en",
        sidebarPath: "./versions_sidebars/version-4.3.0_en.ts",
        path: "./versions/version-4.3.0/en-US/source",
        routeBasePath: "en/4.3.0",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "4-3-0_zh",
        sidebarPath: "./versions_sidebars/version-4.3.0_zh.ts",
        path: "./versions/version-4.3.0/zh-CN/source",
        routeBasePath: "zh/4.3.0",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "4-2-0_en",
        sidebarPath: "./versions_sidebars/version-4.2.0_en.ts",
        path: "./versions/version-4.2.0/en-US/source",
        routeBasePath: "en/4.2.0",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "4-2-0_zh",
        sidebarPath: "./versions_sidebars/version-4.2.0_zh.ts",
        path: "./versions/version-4.2.0/zh-CN/source",
        routeBasePath: "zh/4.2.0",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "4-1-0_en",
        sidebarPath: "./versions_sidebars/version-4.1.0_en.ts",
        path: "./versions/version-4.1.0/en-US/source",
        routeBasePath: "en/4.1.0",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "4-1-0_zh",
        sidebarPath: "./versions_sidebars/version-4.1.0_zh.ts",
        path: "./versions/version-4.1.0/zh-CN/source",
        routeBasePath: "zh/4.1.0",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "4-0-1_en",
        sidebarPath: "./versions_sidebars/version-4.0.1_en.ts",
        path: "./versions/version-4.0.1/en-US/source",
        routeBasePath: "en/4.0.1",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "4-0-1_zh",
        sidebarPath: "./versions_sidebars/version-4.0.1_zh.ts",
        path: "./versions/version-4.0.1/zh-CN/source",
        routeBasePath: "zh/4.0.1",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "4-0-0_en",
        sidebarPath: "./versions_sidebars/version-4.0.0_en.ts",
        path: "./versions/version-4.0.0/en-US/source",
        routeBasePath: "en/4.0.0",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "4-0-0_zh",
        sidebarPath: "./versions_sidebars/version-4.0.0_zh.ts",
        path: "./versions/version-4.0.0/zh-CN/source",
        routeBasePath: "zh/4.0.0",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "3-6-0_en",
        sidebarPath: "./versions_sidebars/version-3.6.0_en.ts",
        path: "./versions/version-3.6.0/en-US/source",
        routeBasePath: "en/3.6.0",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "3-6-0_zh",
        sidebarPath: "./versions_sidebars/version-3.6.0_zh.ts",
        path: "./versions/version-3.6.0/zh-CN/source",
        routeBasePath: "zh/3.6.0",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "3-5-1_en",
        sidebarPath: "./versions_sidebars/version-3.5.1_en.ts",
        path: "./versions/version-3.5.1/en-US/source",
        routeBasePath: "en/3.5.1",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "3-5-1_zh",
        sidebarPath: "./versions_sidebars/version-3.5.1_zh.ts",
        path: "./versions/version-3.5.1/zh-CN/source",
        routeBasePath: "zh/3.5.1",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "3-5-0_en",
        sidebarPath: "./versions_sidebars/version-3.5.0_en.ts",
        path: "./versions/version-3.5.0/en-US/source",
        routeBasePath: "en/3.5.0",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      "content-docs",
      {
        id: "3-5-0_zh",
        sidebarPath: "./versions_sidebars/version-3.5.0_zh.ts",
        path: "./versions/version-3.5.0/zh-CN/source",
        routeBasePath: "zh/3.5.0",
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
  ],

  themeConfig: {
    // Replace with your project's social card‘
    image: "img/docusaurus-social-card.jpg",
    tableOfContents: {
      maxHeadingLevel: 5,
    },
    navbar: {
      logo: {
        alt: "Tugraph Site Logo",
        src: "https://mdn.alipayobjects.com/huamei_qcdryc/afts/img/A*AbamQ5lxv0IAAAAAAAAAAAAADgOBAQ/original",
      },
      items: [
        {
          label: "切换语言",
          type: "docsVersionDropdown",
          position: "right",
          items: [
            {
              label: "中文",
              to: "/zh/guide",
            },
            {
              label: "英文",
              to: "/en/guide",
            },
          ],
        },

        {
          label: "Version",
          type: "dropdown",
          position: "right",
          items: [
            {
              label: "master",
              to: "/zh/guide",
            },
            {
              label: "4.3.2",
              to: "/zh/4.3.2/guide",
            },
          ],
        },
        {
          href: "https://github.com/TuGraph-family/tugraph-db",
          label: "GitHub",
          position: "right",
        },
      ],
    },
    prism: {
      theme: prismThemes.github,
      darkTheme: prismThemes.dracula,
    },
  } satisfies Preset.ThemeConfig,
};

export default config;
