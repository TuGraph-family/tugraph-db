import React, { useEffect } from "react";
import DocSidebar from "@theme-original/DocSidebar";
import type DocSidebarType from "@theme/DocSidebar";
import type { WrapperProps } from "@docusaurus/types";
import { useLocation, useHistory } from "react-router-dom";
import { Select } from "antd";
import { DocSearch } from '@docsearch/react';

type Props = WrapperProps<typeof DocSidebarType>;

export default function DocSidebarWrapper(props: Props): JSX.Element {
  const location = useLocation();
  const history = useHistory();
  const { pathname } = location;
  const languages = ["en", "zh"];
  const versions = [
    "latest",
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
    const newPath =
      version === "4.5.0"
        ? `${basePath}/${remainingPath}`
        : `${basePath}/${version}/${remainingPath}`;

    history.push(newPath);
  };

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

  return (
    <div
      style={{
        display: "flex",
        justifyContent: "center",
        flexDirection: "column",
      }}
    >
      <div style={{display: 'flex', alignItems: 'center'}}>
        <Select
          defaultValue={getCurrentVersion()}
          style={{ width: 120, margin: "10px 4px 8px 8px" }}
          options={versions.map((item) => ({ value: item, label: item }))}
          onChange={onVersionChange}
        />
        <div style={{ margin: "10px 4px 8px 8px" }}>
          <DocSearch
            {...{
              apiKey: "829a7e48ddbd6916e159c003391543a0",
              indexName: "zhongyunwanio",
              appId: "DGYVABHR0M",
              searchParameters: {
                facetFilters: [`docusaurus_tag:docs-${getCurrentVersion()}_${getCurrentLanguage()}-current`],
              },
              transformItems: (items) => {
                return items.map(item => {
                  return {
                    ...item,
                    url: '/docs' + item?.url?.split('/tugraph-db')[1] ?? ''
                  };
                })
              },
            }}
          />
        </div>
      </div>
      <DocSidebar {...props} />
    </div>
  );
}
