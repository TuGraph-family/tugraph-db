import React, { useEffect, useMemo, useRef } from "react";
import DocSidebar from "@theme-original/DocSidebar";
import type DocSidebarType from "@theme/DocSidebar";
import type { WrapperProps } from "@docusaurus/types";
import { useLocation, useHistory } from "react-router-dom";
import Select from "antd/lib/select/index";
import { DocSearch } from "@docsearch/react";
import Link from "@docusaurus/Link";
import { EN_TRANSLATIONS, ZH_TRANSLATIONS } from "@site/src/constants";

type Props = WrapperProps<typeof DocSidebarType>;

export default function DocSidebarWrapper(props: Props): JSX.Element {
  const location = useLocation();
  const history = useHistory();
  const { pathname } = location;
  const languages = ["en", "zh"];
  const versions = [
    "4.5.0",
    "4.3.2",
    "4.3.1",
    "4.3.0",
    "4.2.0",
    "4.1.0",
    "4.0.1",
    "4.0.0",
    "3.6.0",
    "3.5.1",
    "3.5.0",
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
      return segments[versionIndex];
    }
    return "4.5.0";
  };

  const formatDocSearchVersion = (tag: string) => {
    return tag.replace(/docs-(\d+\.\d+\.\d+)|docs-latest_zh/g, (match, version) => {
      
      if (['3.5.1', '3.5.0'].includes(version)) {
        return 'docs-3-6-0';
      }

      if (['4.0.1', '4.0.0'].includes(version)) {
        return 'docs-4-1-0';
      }

      if (['4.3.2', '4.3.1', '4.3.0', '4.2.0'].includes(version)) {
        return 'docs-4-5-0';
      }

      return `docs-${version.replace(/\./g, '-')}`;
    });
  };

  const onVersionChange = (version) => {
    const lang = getCurrentLanguage();
    const prefix = "/tugraph-db";
    const basePath = `${prefix}/${lang}`;

    // 移除现有版本号部分
    const segments = pathname.split("/");
    const langIndex = segments.indexOf(lang);
    const versionIndex = langIndex + 1;
    const remainingPath = segments
      .slice(versionIndex + (versions.includes(segments[versionIndex]) ? 1 : 0))
      .join("/");

    // 构造新路径
    const newPath = `${basePath}/${version}/${remainingPath}`;
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
    const currentVersion = getCurrentVersion();
    const updatedLink = baseLink.replace(/\/\d+\.\d+\.\d+\//, `/${currentVersion}/`);
    return updatedLink;
  };

  const getDocSearchKey = useMemo(() => {

    const currentVersion = getCurrentVersion();

    if (['4.1.0', '4.0.1', '4.0.0', '3.6.0', '3.5.1', '3.5.0'].includes(currentVersion)) {
      return {
        apiKey: "7d995257839cea75cceb969a6d96e40a",
        indexName: "zhongyunwanio",
        appId: "FHM90YCZ2Z",
      }
    }
    
    return {
      apiKey: "829a7e48ddbd6916e159c003391543a0",
      indexName: "zhongyunwanio",
      appId: "DGYVABHR0M",
    }
  }, [location.pathname]);

  useEffect(() => {
    window.addEventListener("click", () => {
      const currentPath = window.location.pathname;
      window.parent.postMessage({ path: currentPath }, "*");
    });
    window.addEventListener("hashchange", () => {
      const currentPath = window.location.pathname;
      const hash = window.location.hash;
      window.parent.postMessage({ path: currentPath + hash }, "*");
    });
  }, []);

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

  return (
    <div
      style={{
        display: "flex",
        justifyContent: "center",
        flexDirection: "column",
      }}
    >
      <div style={{ display: "flex", alignItems: "center" }}>
        <Select
          defaultValue={getCurrentVersion()}
          style={{ width: 120, margin: "10px 4px 8px 8px" }}
          options={versions.map((item) => ({ value: item, label: item }))}
          onChange={onVersionChange}
        />
        <div style={{ margin: "10px 4px 8px 8px" }}>
          <DocSearch
            {...{
              ...getDocSearchKey,
              searchParameters: {
                facetFilters: [
                  formatDocSearchVersion(
                    `docusaurus_tag:docs-${getCurrentVersion()}_${getCurrentLanguage()}-current`
                  ),
                ],
              },
              hitComponent: Hit,
              transformItems: (items) => {
                return items.map(item => {
                  return {
                    ...item,
                    url: replaceVersionInLink('/tugraph-db' + item?.url?.split('/tugraph-db')[1] ?? '')
                  };
                });
              },
              navigator: navigator,
              translations: getTranslationsByLanguage(getCurrentLanguage()),
              placeholder: getPlaceholderByLanguage(getCurrentLanguage()),
            }}
          />
        </div>
      </div>
      <DocSidebar {...props} />
    </div>
  );
}
