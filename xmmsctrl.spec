Summary: command line control utiity for xmms
Name: xmmsctrl
Version: 1.8
Release: 1
URL: http://www.docs.uu.se/~adavid
Source0: %{name}-%{version}.tar.gz
License: GPL
Group: Applications/Multimedia
BuildRoot: %{_tmppath}/%{name}-root
Requires: xmms
BuildRequires: xmms-devel

%description
xmmsctrl is a small utility to control xmms from the command line. Its
goal is to be used coupled with sh to test xmms state and perform an
appropriate action, e.g. if playing then pause else play. The interest
of this is to bind keys in a window manager to have control over xmms
with keys that do play/next/pause, prev, control sound...

%prep
%setup -q

%build
make
%install
rm -rf $RPM_BUILD_ROOT
mkdir -p ${RPM_BUILD_ROOT}/usr/bin
install -m 755  xmmsctrl ${RPM_BUILD_ROOT}/usr/bin
%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc Changelog
%doc README
%doc HELP
%doc samples
/usr/bin/xmmsctrl

%changelog
* Mon Feb 18 2002 Jan IVEN <jan.iven@cern.ch>
- Initial build.


