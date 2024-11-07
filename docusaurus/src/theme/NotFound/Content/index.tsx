import React, { useMemo } from "react";
import clsx from "clsx";
import { useLocation } from "react-router-dom";
import type { Props } from "@theme/NotFound/Content";
import Heading from "@theme/Heading";
import { EN_NOT_FOUND_CONFIG, ZH_NOT_FOUND_CONFIG } from "@site/src/constants";

export default function NotFoundContent({ className }: Props): JSX.Element {
  const { pathname } = useLocation();

  const getHomeHref = () => {
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

  const contentConfig = useMemo(() => {
    const lang =
      pathname?.split("/")?.find((item) => ["zh", "en"].includes(item)) || "en";

    if (lang === "en") {
      return EN_NOT_FOUND_CONFIG;
    }
    return ZH_NOT_FOUND_CONFIG;
  }, [pathname]);

  return (
    <main className={clsx(" margin-vert--xl", className)}>
      <div className="row" >
        <div className="col col--6 col--offset-3">
          <Heading as="h1" className="hero__title">
            {contentConfig.title}
          </Heading>
          {contentConfig.descriptions?.map((desc) => (
            <p>{desc}</p>
          ))}
          <a href={getHomeHref()}>
            {contentConfig.homeBtnText}
          </a>
        </div>
      </div>
    </main>
  );
}
