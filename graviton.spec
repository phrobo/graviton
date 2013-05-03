Name:           graviton
Version:        0.0.1
Release:        1%{?dist}
Summary:        Graviton pulls your network together.

License:        GPLv3
URL:            http://aether.phrobo.net/
Source0:        %{name}-%{version}.tar.bz2

BuildRequires:  glib2-devel libsoup-devel json-glib-devel cmake
BuildRequires:  libmpdclient-devel avahi-devel avahi-glib-devel

%description
Graviton pulls your network together

%package devel
Requires:       %{name} = %{version}
Summary:        Development files for %{name}

%description devel
Development files for %{name}

%package plugin-mpd
Requires:       %{name} = %{version}
Requires:       mpd
Summary:        MPD plugin for Graviton

%description plugin-mpd
MPD plugin for Graviton

%prep
%setup -q


%build
%cmake
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
%make_install


%files
%doc
%{_bindir}/graviton-server
%{_libdir}/*.so*
%dir %{_datadir}/graviton/
%dir %{_datadir}/graviton/plugins/
%{_datadir}/graviton/plugins/info.so

%files devel
%{_includedir}/*
%{_libdir}/cmake/*
%{_libdir}/pkgconfig/*

%files plugin-mpd
%{_datadir}/graviton/plugins/mpd.so

%changelog
