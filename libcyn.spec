Name:           libcyn 
Version:        0.0.1 
Release:        1%{?dist}
Summary:        Read-Only Linux system information access library
License:        BSD-3-Clause
URL:            https://github.com/cynthefer/libcyn
Source0:        https://github.com/Cynthefer/libcyn/archive/v%{version}/%{name}-%{version}.tar.gz

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
%make_build PREFIX=%{_prefix}


%install
%make_install PREFIX=%{_prefix}


%files
%license LICENSE
%doc README.md CHANGELOG.md docs/
%{_includedir}/cyn.h
%{_includedir}/libcyn.a
%{_includedir}/libcyn.so.%{version}
%{_includedir}/libcyn.so
%{_includedir}/pkgconfig/libcyn.pc
%{_mandir}/man3/cyn.3*

%changelog
* Thu Nov 26 2026 Cynthefer <cynthefer@gmail.com>
- Initial package for fedora
