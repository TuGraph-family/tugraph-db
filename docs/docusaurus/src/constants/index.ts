import { DocSearchTranslations } from "@docsearch/react";

const EN_TRANSLATIONS: DocSearchTranslations = {
  button: {
    buttonText: "Search",
    buttonAriaLabel: "Search",
  },
  modal: {
    searchBox: {
      resetButtonTitle: "Clear the query",
      resetButtonAriaLabel: "Clear the query",
      cancelButtonText: "Cancel",
      cancelButtonAriaLabel: "Cancel",
      searchInputLabel: "Search",
    },
    startScreen: {
      recentSearchesTitle: "Recent",
      noRecentSearchesText: "No recent searches",
      saveRecentSearchButtonTitle: "Save this search",
      removeRecentSearchButtonTitle: "Remove this search from history",
      favoriteSearchesTitle: "Favorite",
      removeFavoriteSearchButtonTitle: "Remove this search from favorites",
    },
    errorScreen: {
      titleText: "Unable to fetch results",
      helpText: "You might want to check your network connection.",
    },
    footer: {
      selectText: "to select",
      selectKeyAriaLabel: "Enter key",
      navigateText: "to navigate",
      navigateUpKeyAriaLabel: "Arrow up",
      navigateDownKeyAriaLabel: "Arrow down",
      closeText: "to close",
      closeKeyAriaLabel: "Escape key",
      searchByText: "Search by",
    },
    noResultsScreen: {
      noResultsText: "No results for",
      suggestedQueryText: "Try searching for",
      reportMissingResultsText: "Believe this query should return results?",
      reportMissingResultsLinkText: "Let us know.",
    },
  },
};

const ZH_TRANSLATIONS: DocSearchTranslations = {
  button: {
    buttonText: "搜索",
    buttonAriaLabel: "搜索",
  },
  modal: {
    searchBox: {
      resetButtonTitle: "盒子",
      resetButtonAriaLabel: "清除查询",
      cancelButtonText: "取消",
      cancelButtonAriaLabel: "取消",
      searchInputLabel: "搜索文档",
    },
    startScreen: {
      recentSearchesTitle: "最近的",
      noRecentSearchesText: "没有最近搜索记录",
      saveRecentSearchButtonTitle: "保存此搜索",
      removeRecentSearchButtonTitle: "从历史记录中删除此搜索",
      favoriteSearchesTitle: "最喜欢的",
      removeFavoriteSearchButtonTitle: "从收藏夹中删除此搜索",
    },
    errorScreen: {
      titleText: "无法获取结果",
      helpText: "您可能想检查您的网络连接.",
    },
    footer: {
      selectText: "选择",
      selectKeyAriaLabel: "输入键",
      navigateText: "导航",
      navigateUpKeyAriaLabel: "向上箭头",
      navigateDownKeyAriaLabel: "向下箭头",
      closeText: "关闭",
      closeKeyAriaLabel: "退出键",
      searchByText: "搜索提供",
    },
    noResultsScreen: {
      noResultsText: "未搜索到",
      suggestedQueryText: "尝试搜索",
      reportMissingResultsText: "相信这个查询应该返回结果?",
      reportMissingResultsLinkText: "让我们知道.",
    },
  },
};

const EN_NOT_FOUND_CONFIG = {
  title: "Page Not Found",
  descriptions: [
    "We could not find what you were looking for.",
    "Please contact the owner of the site that linked you to the original URL and let them know their link is broken.",
  ],
  homeBtnText: "Back to Home",
};

const ZH_NOT_FOUND_CONFIG = {
  title: "找不到页面",
  descriptions: [
    "我们找不到您要找的页面",
    "请联系原始链接来源网站的所有者，并告知他们链接已损坏",
  ],
  homeBtnText: "返回首页",
};

const versions = [
  "4.5.2",
  "4.5.1",
  "4.3.2",
  "4.3.1",
  "4.3.0",
  "4.2.0",
  "4.1.0",
  "4.0.1",
  "3.6.0",
  "3.5.1",
];

const EN_DOC_OPTIONS = [
  {
    label: "TuGraph DB Community",
    value: "TuGraph_DB",
    children: versions.map((item) => ({ value: item, label: item })),
  },
  {
    label: "TuGraph Analytics Streaming Graph Computing Engine",
    value: "TuGraph_Analytics",
  },
];

const ZH_DOC_OPTIONS = [
  {
    label: "TuGraph DB 社区版",
    value: "TuGraph_DB",
    children: versions.map((item) => ({ value: item, label: item })),
  },
  {
    label: "TuGraph Analytics 实时图计算引擎",
    value: "TuGraph_Analytics",
  },
];

export {
  EN_TRANSLATIONS,
  ZH_TRANSLATIONS,
  ZH_NOT_FOUND_CONFIG,
  EN_NOT_FOUND_CONFIG,
  ZH_DOC_OPTIONS,
  EN_DOC_OPTIONS,
  versions,
};
