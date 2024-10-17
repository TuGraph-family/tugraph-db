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
  baseUrl: '/tugraph-db/',

  // GitHub pages deployment config.
  // If you aren't using GitHub pages, you don't need these.
  organizationName: 'facebook', // Usually your GitHub org/user name.
  projectName: 'Tugraph Docs', // Usually your repo name.

  onBrokenLinks: 'warn',
  onBrokenMarkdownLinks: 'warn',

  markdown: {
    format: 'md'
  },

  presets: [
    [
      'classic',
      {
        docs: {
          sidebarPath: './sidebars.ts',
          path: './docs/zh-CN/source',
          routeBasePath: 'zh',
          // Please change this to your repo.
          // Remove this to remove the "edit this page" links.
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
        id: 'en',
        path: './docs/en-US/source',
        routeBasePath: 'en',
        editCurrentVersion: true,
        sidebarPath: './sidebarsEn.ts',
        showLastUpdateAuthor: true,
        showLastUpdateTime: true,
      } satisfies DocsOptions,
    ]
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
        // href: 'https://tugraph.tech/'
      },
      items: [
        {
          href: 'https://github.com/TuGraph-family/tugraph-db',
          label: 'GitHub',
          position: 'right',
        },
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