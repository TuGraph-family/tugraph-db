import React, { useEffect, type ReactNode } from 'react';
import StructuredData from '@theme-original/DocBreadcrumbs/StructuredData';
import type StructuredDataType from '@theme/DocBreadcrumbs/StructuredData';
import type { WrapperProps } from '@docusaurus/types';
import { useBreadcrumb, type BreadcrumbItem } from '../../hooks/useBreadcrumbHistory';

type Props = WrapperProps<typeof StructuredDataType>;

export default function StructuredDataWrapper(props: Props): ReactNode {
  const { breadcrumbs } = props;
  const { setBreadcrumbHistory } = useBreadcrumb()

  // 当 breadcrumbs 变化时，存储最后一项（当前页面）
  useEffect(() => {
    if (breadcrumbs && breadcrumbs.length > 0) {
      const lastBreadcrumb = breadcrumbs[breadcrumbs.length - 1] as BreadcrumbItem;
      setBreadcrumbHistory(lastBreadcrumb)
    }
  }, [breadcrumbs]);

  return (
    <>
      <StructuredData {...props} />
    </>
  );
}