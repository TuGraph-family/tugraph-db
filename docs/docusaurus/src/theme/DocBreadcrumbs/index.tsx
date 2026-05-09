import React, { useMemo, type ReactNode } from 'react';
import { Breadcrumb } from 'antd';
import { useBreadcrumb } from '../hooks/useBreadcrumbHistory';
import DocBreadcrumbs from '@theme-original/DocBreadcrumbs';
import type DocBreadcrumbsType from '@theme/DocBreadcrumbs';
import type { WrapperProps } from '@docusaurus/types';
import Link from "@docusaurus/Link";
import { useHistory } from '@docusaurus/router';

type Props = WrapperProps<typeof DocBreadcrumbsType>;

export default function DocBreadcrumbsWrapper(props: Props): ReactNode {
  const history = useHistory();
  const { breadcrumbs, } = useBreadcrumb()

  const displayItems = useMemo(() => {
    return (breadcrumbs || []).map((i, idx, arr) => {
      return {
        key: i.href,
        title: idx === arr.length - 1
          ? <span style={{ color: '#fff' }}>{i.label}</span>
          : <span style={{ color: '#d4d4d8', }} onClick={() => history.push(i.href)}>{i.label}</span>,
      }
    })

  }, [breadcrumbs])


  return (
    <>
      {displayItems.length > 0 && (
        <Breadcrumb items={displayItems} style={{ marginBottom: 16 }} />
      )}
      <div style={{ display: 'none' }}>
        <DocBreadcrumbs {...props} />
      </div>
    </>
  );
}