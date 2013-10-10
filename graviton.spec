%{!?python_sitelib: %global python_sitelib %(%{__python} -c "from %distutils.sysconfig import get_python_lib; print(get_python_lib())")}
%{!?python_sitearch: %global python_sitearch %(%{__python} -c "from %distutils.sysconfig import get_python_lib; print(get_python_lib(1))")}

Name:           graviton
Version:        0.0.1
Release:        2%{?dist}
Summary:        Graviton pulls your network together.

License:        GPLv3
URL:            http://aether.phrobo.net/
Source0:        %{name}-%{version}.tar.bz2

BuildRequires:  glib2-devel libsoup-devel json-glib-devel cmake
BuildRequires:  libmpdclient-devel avahi-devel avahi-glib-devel
BuildRequires:  gobject-introspection-devel python-devel
BuildRequires:  vala-tools libuuid-devel

%description
Graviton pulls your network together

%package devel
Requires:       %{name} = %{version}
Summary:        Development files for %{name}

%package cli
Requires:       %{name} = %{version}
Summary:        Command line interface for inspecting graviton services

%description cli
Command line interface for inspecting graviton services

%package ping-client
Requires:       %{name} = %{version}
Summary:        Basic ping client example

%description ping-client
Example ping clinet

%package ping-server
Requires:       %{name} = %{version}
Summary:        Basic ping server example

%description ping-server
Example ping server

%package python-client
Requires:       python dbus-python avahi-ui-tools pygobject2 python-requests
Summary:        Python library and tools to interact with %{name}
BuildArch:      noarch

%description python-client
Python library and tools to interact with %{name}

%description devel
Development files for %{name}

%prep
%setup -q


%build
%cmake
make %{?_smp_mflags}
cd bindings/python/
CFLAGS="$RPM_OPT_FLAGS" %{__python} setup.py build


%install
rm -rf $RPM_BUILD_ROOT
%make_install
cd bindings/python/
%{__python} setup.py install -O1 --skip-build --root $RPM_BUILD_ROOT


%files
%doc
%{_libdir}/*.so*
%dir %{_libdir}/graviton/
%{_libdir}/graviton/*

%files cli
%{_bindir}/graviton-cli

%files ping-client
%{_bindir}/graviton-ping-client

%files ping-server
%{_bindir}/graviton-ping-server

%files devel
%{_includedir}/*
%{_libdir}/cmake/*
%{_libdir}/pkgconfig/*
#%{_datadir}/gir-1.0/*
#%{_datadir}/vala/vapi/*

%files python-client
%{python_sitelib}/*
%{_bindir}/graviton-browse
%{_bindir}/graviton-call

%changelog
