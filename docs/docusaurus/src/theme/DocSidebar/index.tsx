import React, { useEffect, useMemo, useRef } from "react";
import DocSidebar from "@theme-original/DocSidebar";
import type DocSidebarType from "@theme/DocSidebar";
import type { WrapperProps } from "@docusaurus/types";
import { useLocation, useHistory } from "react-router-dom";
import { DocSearch } from "@docsearch/react";
import Link from "@docusaurus/Link";
import {
  EN_DOC_OPTIONS,
  EN_TRANSLATIONS,
  ZH_DOC_OPTIONS,
  ZH_TRANSLATIONS,
} from "@site/src/constants";
import { Cascader, Tooltip, ConfigProvider, theme } from "antd";

type Props = WrapperProps<typeof DocSidebarType>;

export default function DocSidebarWrapper(props: Props): JSX.Element {
  const location = useLocation();
  const history = useHistory();
  const { pathname } = location;
  const languages = ["en", "zh"];
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

  const getCurrentLanguage = () => {
    const segments = pathname.split("/");
    return languages.find((lang) => segments.includes(lang)) || "en";
  };

  const getCurrentVersion = () => {
    const segments = pathname.split("/");
    const langIndex = segments.indexOf(getCurrentLanguage());
    const versionIndex = langIndex + 1;

    if (
      segments.length > versionIndex &&
      versions.includes(segments[versionIndex])
    ) {
      return { label: segments[versionIndex], value: segments[versionIndex] };
    }
    return { label: "4.5.1", value: "4.5.1" };
  };


  const onVersionChange = (values) => {
    const [type, version] = values;
    const lang = getCurrentLanguage();
    if (type === "TuGraph_Analytics") {
      window.location.href = `https://geaflow.apache.org/docs/guide`;
      return;
    }

    if (type === "TuGraph_Learn") {
      const learnPath = `/tugraph-db/${lang}/${version}/olap&procedure/learn/tutorial`;
      history.push(learnPath);
      return;
    }

    const prefix = "/tugraph-db";
    const basePath = `${prefix}/${lang}`;

    // 构造新路径
    const newPath = `${basePath}/${version}/guide`;
    history.push(newPath);
  };

  const navigator = useRef({
    navigate({ itemUrl }: { itemUrl?: string }) {
      history.push(itemUrl!);
    },
  }).current;

  const Hit: React.FC = ({ hit, children }) => {
    return <Link to={hit.url}>{children}</Link>;
  };

  const replaceVersionInLink = (baseLink: string): string => {
    const { value } = getCurrentVersion();
    const updatedLink = baseLink.replace(/\/\d+\.\d+\.\d+\//, `/${value}/`);
    return updatedLink;
  };

  const getTranslationsByLanguage = (lang: string) => {
    if (lang === "zh") {
      return ZH_TRANSLATIONS;
    }
    return EN_TRANSLATIONS;
  };

  const getPlaceholderByLanguage = (lang: string) => {
    if (lang === "zh") {
      return "搜索文档";
    }
    return "Search docs";
  };

  const getDescByLanguage = (lang: string) => {
    if (lang === "zh") {
      return "社区版";
    }
    return "Community";
  };

  const getOptions = (lang: string) => {
    if (lang === "zh") {
      return ZH_DOC_OPTIONS;
    }
    return EN_DOC_OPTIONS;
  };

  const customStyles = {
    control: (provided: React.CSSProperties) => ({
      ...provided,
      width: "120px",
      minHeight: "36px",
      height: "36px",
      margin: "10px 4px 0 8px",
    }),
    valueContainer: (provided: React.CSSProperties) => ({
      ...provided,
      padding: "0 8px",
    }),
  };

  useEffect(() => {
    const sendPostMsg = () => {
      window.parent.postMessage({ path: window.location.pathname }, "*");
    };

    const sendPostHashMsg = () => {
      window.parent.postMessage(
        { path: window.location.pathname + window.location.hash },
        "*"
      );
    };

    window.addEventListener("click", sendPostMsg);
    window.addEventListener("hashchange", sendPostHashMsg);

    sendPostMsg();

    return () => {
      window.removeEventListener("click", sendPostMsg);
      window.removeEventListener("hashchange", sendPostHashMsg);
    };
  }, []);

  return (
    <ConfigProvider
      theme={{
        algorithm: theme.darkAlgorithm,
        token: {
          colorBgElevated: 'rgba(30, 30, 35, 0.95)',
          colorBgContainer: 'rgba(30, 30, 35, 0.95)',
          colorBorder: 'rgba(255, 255, 255, 0.12)',
          colorText: 'rgba(244, 244, 245, 0.96)',
          colorTextSecondary: 'rgba(161, 161, 170, 0.92)',
          colorPrimary: '#3b82f6',
          borderRadius: 8,
        },
        components: {
          Cascader: {
            dropdownHeight: 'auto',
            optionSelectedBg: 'rgba(59, 130, 246, 0.2)',
            optionActiveBg: 'rgba(255, 255, 255, 0.05)',
          },
        },
      }}
    >
      <div
        className="docsearch-wrapper"
        style={{
          display: "flex",
          justifyContent: "center",
          flexDirection: "column",
        }}
      >
        <div
          style={{
            display: "flex",
            justifyContent: "space-between",
            marginBottom: "8px",
          }}
        >
          <Cascader
            allowClear={false}
            value={["TuGraph_DB", getCurrentVersion()?.label]}
            options={getOptions(getCurrentLanguage())}
            onChange={onVersionChange}
          >
            <div className="itemWrapper">
              <div className="titleBlock">
                <span className="titleText">TuGraph DB</span>
                <div className="downIcon" />
              </div>
              <div className="contentArea">
                <span id="engineDescription">
                  {getDescByLanguage(getCurrentLanguage())}
                </span>
                <span className="versionLabel">
                  V{getCurrentVersion()?.label}
                </span>
              </div>
            </div>
          </Cascader>

          <Tooltip
            title={getPlaceholderByLanguage(getCurrentLanguage())}
            trigger={["hover", "click"]}
          >
            <div className="searchWrapper">
              <DocSearch
                {...{
                  apiKey: "315fd6a0c1acbdeecd5ba56d8062d00d",
                  indexName: getCurrentLanguage() === 'en' ? 'tugraph_en' : "tugraph_zh",
                  appId: "HO4M21RAQI",
                  searchParameters: {
                    facetFilters: [`version:${getCurrentVersion()?.value}`],
                  },
                  hitComponent: Hit,
                  transformItems: (items) => {
                    console.log('[DocSearch] Raw items from Algolia:', items);
                    return items.map((item, index) => {
                      // 确保 hierarchy 存在,否则 DocSearch 会报错
                      if (!item.hierarchy) {
                        console.error(`[DocSearch] Item ${index} missing hierarchy:`, {
                          objectID: item.objectID,
                          url: item.url,
                          type: item.type,
                          item: item
                        });
                        // 返回一个符合 DocSearch 结构的默认对象
                        return {
                          ...item,
                          hierarchy: {
                            lvl0: item.type || 'Documentation',
                            lvl1: item.url?.split('/').pop() || 'Unknown',
                            lvl2: null,
                            lvl3: null,
                            lvl4: null,
                            lvl5: null,
                            lvl6: null,
                          },
                          url: replaceVersionInLink(
                            "/tugraph-db" + item?.url?.split("/tugraph-db")[1] ?? ""
                          ),
                        };
                      }

                      // 确保 hierarchy 有所有必需的层级
                      const hierarchy = {
                        lvl0: item.hierarchy?.lvl0 || 'Documentation',
                        lvl1: item.hierarchy?.lvl1 || 'Unknown',
                        lvl2: item.hierarchy?.lvl2 || null,
                        lvl3: item.hierarchy?.lvl3 || null,
                        lvl4: item.hierarchy?.lvl4 || null,
                        lvl5: item.hierarchy?.lvl5 || null,
                        lvl6: item.hierarchy?.lvl6 || null,
                      };

                      return {
                        ...item,
                        hierarchy,
                        url: replaceVersionInLink(
                          "/tugraph-db" + item?.url?.split("/tugraph-db")[1] ?? ""
                        ),
                      };
                    });
                  },
                  navigator: navigator,
                  translations: getTranslationsByLanguage(getCurrentLanguage()),
                  placeholder: getPlaceholderByLanguage(getCurrentLanguage()),
                }}
              />
            </div>
          </Tooltip>
        </div>
        <DocSidebar {...props} />
      </div>
    </ConfigProvider>
  );
}
