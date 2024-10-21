import {themes as prismThemes} from 'prism-react-renderer';
import type {Config} from '@docusaurus/types';
import type {Options as DocsOptions} from '@docusaurus/plugin-content-docs';
import type * as Preset from '@docusaurus/preset-classic';

const config: Config = {
  title: 'TuGraph 文档中心',
  tagline: '高性能图数据库TuGraph由蚂蚁集团和清华大学共同研发，历经蚂蚁实际业务场景锤炼，在国际图数据库基准测试中获得性能第一',
  favicon: 'https://mdn.alipayobjects.com/huamei_qcdryc/afts/img/A*AbamQ5lxv0IAAAAAAAAAAAAADgOBAQ/original',

  // Set the production url of your site here
  url: 'https://tugraph.tech/',
  // Set the /<baseUrl>/ pathname under which your site is served
  // For GitHub pages deployment, it is often '/<projectName>/'
  baseUrl: '/tugraph-db',

  // GitHub pages deployment config.
  // If you aren't using GitHub pages, you don't need these.
  organizationName: 'facebook', // Usually your GitHub org/user name.
  projectName: 'Tugraph Docs', // Usually your repo name.

  onBrokenLinks: 'warn',
  onBrokenMarkdownLinks: 'warn',
  onBrokenAnchors: 'warn',

  trailingSlash: false,

  markdown: {
    format: 'md',
    mermaid: true,
  },

  i18n: {
    defaultLocale: 'zh-CN',
    locales: ['zh-CN', 'en-US'],
  },

  presets: [
    [
      'classic',
      {
        docs: {
          sidebarPath: './sidebars.ts',
          path: './docs/zh-CN/source',
          routeBasePath: 'zh/latest',
          showLastUpdateAuthor: true,
          showLastUpdateTime: true,
        },
        theme: {
          customCss: './src/css/custom.css',
        },
      } satisfies Preset.Options,
    ],
  ],

  plugins: [
    [
      'content-docs',
      {
        id: 'latest-en',
        sidebarPath: './sidebarsEn.ts',
        path: './docs/en-US/source',
        routeBasePath: 'en/latest',
        showLastUpdateAuthor: true,
        showLastUpdateTime: true,
      } satisfies DocsOptions,
    ],
    [
      'content-docs',
      {
        id: '4-5-0_en',
        sidebarPath: './versions_sidebars/version-4.5.0_en.ts',
        path: './versions/version-4.5.0/en-US/source',
        routeBasePath: 'en',
        showLastUpdateAuthor: true,
        showLastUpdateTime: true,
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      'content-docs',
      {
        id: '4-5-0_zh',
        sidebarPath: './versions_sidebars/version-4.5.0_zh.ts',
        path: './versions/version-4.5.0/zh-CN/source',
        routeBasePath: 'zh',
        showLastUpdateAuthor: true,
        showLastUpdateTime: true,
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      'content-docs',
      {
        id: '4-3-2_en',
        sidebarPath: './versions_sidebars/version-4.3.2_en.ts',
        path: './versions/version-4.3.2/en-US/source',
        routeBasePath: 'en/4.3.2',
        showLastUpdateAuthor: true,
        showLastUpdateTime: true,
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      'content-docs',
      {
        id: '4-3-2_zh',
        sidebarPath: './versions_sidebars/version-4.3.2_zh.ts',
        path: './versions/version-4.3.2/zh-CN/source',
        routeBasePath: 'zh/4.3.2',
        showLastUpdateAuthor: true,
        showLastUpdateTime: true,
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      'content-docs',
      {
        id: '4-3-1_en',
        sidebarPath: './versions_sidebars/version-4.3.1_en.ts',
        path: './versions/version-4.3.1/en-US/source',
        routeBasePath: 'en/4.3.1',
        showLastUpdateAuthor: true,
        showLastUpdateTime: true,
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
    [
      'content-docs',
      {
        id: '4-3-1_zh',
        sidebarPath: './versions_sidebars/version-4.3.1_zh.ts',
        path: './versions/version-4.3.1/zh-CN/source',
        routeBasePath: 'zh/4.3.1',
        showLastUpdateAuthor: true,
        showLastUpdateTime: true,
        editCurrentVersion: false,
      } satisfies DocsOptions,
    ],
  ],

  themeConfig: {
    // Replace with your project's social card‘
    image: 'img/docusaurus-social-card.jpg',
    tableOfContents: {
      maxHeadingLevel: 5,
    },
    navbar: {
      logo: {
        alt: 'Tugraph Site Logo',
        src: 'https://mdn.alipayobjects.com/huamei_qcdryc/afts/img/A*AbamQ5lxv0IAAAAAAAAAAAAADgOBAQ/original',
      },
      items: [
        {
          label: '切换语言',
          type: 'docsVersionDropdown',
          position: 'right',
          items: [
            {
              label: '中文',
              to: '/zh/guide',
            },
            {
              label: '英文',
              to: '/en/guide',
            },
          ],
        },
        
        {
          label: 'Version',
          type: 'dropdown',
          position: 'right',
          items: [
            {
              label: 'master',
              to: '/zh/guide',
            },
            {
              label: '4.3.2',
              to: '/zh/4.3.2/guide',
            },
          ],
        },
        {
          href: 'https://github.com/TuGraph-family/tugraph-db',
          label: 'GitHub',
          position: 'right',
        }
      ],
    },
    footer: {
      style: 'dark',
      links: [
        {
          title: 'Docs',
          items: [
            {
              label: 'Tutorial',
              to: '/zh/guide',
            },
          ],
        },
        {
          title: 'More',
          items: [
            {
              label: 'GitHub',
              href: 'https://github.com/TuGraph-family/tugraph-db',
            },
          ],
        },
      ],
      copyright: `Copyright © ${new Date().getFullYear()} My Project, Inc. Built with Docusaurus.`,
    },
    prism: {
      theme: prismThemes.github,
      darkTheme: prismThemes.dracula,
    },
  } satisfies Preset.ThemeConfig,
};

export default config;