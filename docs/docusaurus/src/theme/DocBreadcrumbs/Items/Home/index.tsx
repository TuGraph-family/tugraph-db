import React from "react";
import Link from "@docusaurus/Link";
import { translate } from "@docusaurus/Translate";

import { useLocation } from "@docusaurus/router";

export default function HomeBreadcrumbItem(): JSX.Element {
  const { pathname } = useLocation();

  const homeHref = () => {
    try {
      const pathSegments = pathname
        .split("/")
        .filter((segment) => segment !== "");

      // 截取前三个路径段
      const basePathSegments = pathSegments.slice(0, 3);
      const basePath = `/${basePathSegments.join("/")}/guide`;
      return basePath;
    } catch (error) {
      console.error("Invalid URL:", error);
      return "";
    }
  };

  const homeText = () => {
    const lang =
      pathname?.split("/")?.find((item) => ["zh", "en"].includes(item)) || "en";
    if (lang === "en") {
      return "Docs";
    }
    return "文档";
  };

  return (
    <li className="breadcrumbs__item">
      <Link
        aria-label={translate({
          id: "theme.docs.breadcrumbs.home",
          message: "Home page",
          description: "The ARIA label for the home page in the breadcrumbs",
        })}
        className="breadcrumbs__link"
        href={homeHref()}
      >
        <div style={{ color: "#1a1b2572" }}>{homeText()}</div>
      </Link>
    </li>
  );
}
