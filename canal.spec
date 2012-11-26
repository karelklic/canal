Name: canal
Version: 2
Release: 1%{?dist}
Summary: LLVM static analysis framework
Group: Development/Libraries
License: GPLv3+
Source0: https://fedorahosted.org/released/canal/canal-%{version}.tar.xz
BuildRequires: asciidoc xmlto
BuildRequires: llvm-devel

%description
LLVM static analysis framework.

%prep
%setup -q

%build
%configure
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}

%check
make check

%files

%changelog
* Thu Mar  8 2012 Karel Klíč <kklic@redhat.com> - 2-1
- Upstream package spec file
