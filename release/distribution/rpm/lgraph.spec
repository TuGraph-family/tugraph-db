Name:           lgraph
Version: 1.3.1
Release:        1%{?dist}
Summary:        LightningGraph Database

Group:          Applications/Databases
License:        LICENSE
URL:            https://fma.ai
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %_topdir/BUILDROOT
BuildRequires:  gcc
Requires:       bash
Prefix:         /usr/local
provides:       libc.so.6(GLIBC_PRIVATE)(64bit)

%description
LightningGraph is a graph database that supports fast lookup and batch update.

%prep
%setup -q


%build
#%configure
#make %{?_smp_mflags}


%install
#make install DESTDIR=%{buildroot}
rm -rf %{buildroot}/*
mkdir -p %{buildroot}/%{prefix}
cp -r usr/local/* %{buildroot}/%{prefix}
#mkdir -p %{buildroot}/%{_bindir}
#install -m 0755 bin/* %{buildroot}/%{_bindir}


%files
%defattr (-,root,root,0755)
#%doc
%license LICENSE
%{prefix}/bin/
%{prefix}/include/
%{prefix}/lib64/
%{prefix}/etc/
%{prefix}/share/


%changelog
* Wed Mar 13 2019 wangtao <wangtao.waves@gmail.com> - 1.3.1
- Web service and graph visualization.
  User management, graph query, graph visualization and other modules are integrated
  into the web service.
- Server service.
  The lgraph server can run in daemon mode.
- Continuous improvement of OpenCypher.

* Sun Aug  5 2018 wangtao <wangtao.waves@gmail.com> - 1.2.0
- Add cypher.
- Add OLAP interface to C++ API.

* Fri Jul 27 2018 wangtao <wangtao.waves@gmail.com> - 1.1.1
- Add embedded C++ API.
- Add C++/python plugins.
- Modify embedded python API.
- Remove RPC server & python RPC client.

* Fri Apr 13 2018 wangtao <wangtao.waves@gmail.com> - 1.1.0
- Initial release.


