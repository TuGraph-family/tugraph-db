import React from 'react';
import DocSidebar from '@theme-original/DocSidebar';
import type DocSidebarType from '@theme/DocSidebar';
import type {WrapperProps} from '@docusaurus/types';
import { useLocation, useHistory } from 'react-router-dom';
import { Select } from 'antd';

type Props = WrapperProps<typeof DocSidebarType>;

export default function DocSidebarWrapper(props: Props): JSX.Element {

  const location = useLocation();
  const history = useHistory();
  const { pathname } = location;
  const languages = ['en', 'zh'];
  const versions = ['latest', '4.5.0', '4.3.2', '4.3.1'];

  const getCurrentLanguage = () => {
    const segments = pathname.split('/');
    return languages.find(lang => segments.includes(lang)) || 'en';
  };
  
  const getCurrentVersion = () => {
    const segments = pathname.split('/');
    const langIndex = segments.indexOf(getCurrentLanguage());
    const versionIndex = langIndex + 1;
    
    if (segments.length > versionIndex && versions.includes(segments[versionIndex])) {
      return segments[versionIndex];
    }
    return '4.5.0';
  };
  
  const onVersionChange = (version) => {
    const lang = getCurrentLanguage();
    const prefix = '/tugraph-db';
    const basePath = `${prefix}/${lang}`;
  
    // 移除现有版本号部分
    const segments = pathname.split('/');
    const langIndex = segments.indexOf(lang);
    const versionIndex = langIndex + 1;
    const remainingPath = segments.slice(versionIndex + (versions.includes(segments[versionIndex]) ? 1 : 0)).join('/');
  
    // 构造新路径
    const newPath = version === '4.5.0'
      ? `${basePath}/${remainingPath}`
      : `${basePath}/${version}/${remainingPath}`;
  
    history.push(newPath);
  };

  return (
    <div style={{display: 'flex', justifyContent: 'center', flexDirection: 'column'}}>
      <Select
        defaultValue={getCurrentVersion()}
        style={{ width: 120, padding: '8px 0 0 8px' }}
        options={versions.map(item => ({value: item, label: item}))}
        onChange={onVersionChange}
      />
      <DocSidebar {...props} />
    </div>
  );
}
