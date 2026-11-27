Name:           libcyn 
Version:        0.0.2 
Release:        1%{?dist}
Summary:        Read-Only Linux system information access library
License:        BSD-3-Clause
URL:            https://github.com/cynthefer/libcyn
Source0:        %{url}/archive/v%{version}/%{name}-%{version}.tar.gz

BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  pkgconfig
Requires:       curl
Requires:       dmidecode

%description
libcyn is a robust, dependency-free C library for querying Linux system
information (users, hardware, network, storage, and OS details) entirely
without privilege escalation.

It outputs nothing and spawns no shells. It requires no root privileges. You
ask for a struct, and it fills it with validated, zero-terminated strings.

%prep
%autosetup

%build
%make_build PREFIX=%{_prefix} LIBDIR=%{_libdir}

%install
%make_install PREFIX=%{_prefix} LIBDIR=%{_libdir}

%files
%license LICENSE
%doc README.md CHANGELOG.md docs/
%{_includedir}/cyn.h
%{_libdir}/libcyn.a
%{_libdir}/libcyn.so.*
%{_libdir}/pkgconfig/libcyn.pc
%{_mandir}/man3/cyn.3*

%changelog
* Fri Nov 27 2026 Ntando Khanyile <cynthefer@gmail.com> - 0.0.2-1
- Initial package for fedora