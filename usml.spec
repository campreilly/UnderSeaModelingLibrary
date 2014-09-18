Summary:  The goal of this program is to package the USML files for delivery.  This spec file will generate an .rpm file after having the rpmbuild (-ba) command used upon it.

Name:  usml
Version: 0.05
Release: 5%{?dist}
Group:  Acoustic Model
Source0: usml.tgz
License: BSD 3.0
Buildroot: /var/tmp/%{name}-buildroot
Prefix: /usr/local

BuildRequires: netcdf >= 4.1.3
BuildRequires: boost >= 1.41
BuildRequires: libstdc++


%define debug_package %{nil}

%description 
This will install the Under Sea Modeling Library (USML)

################### PREP - Used to get source ready to build #######################
%prep

%setup -n usml
rm -rf ../usml-release
mv usml-release ../usml-release

################### BUILD - Shell script used to build source code #######################
%build
#Begin 


cd scripts
./build.sh


#################### INSTALL STEP  #######################################################

%install
#Remove the directory if it exists
if [ -d %{buildroot} ]; then 
	rm -Rf %{buildroot}
fi

#Create the USML Directories
#mkdir -p %{buildroot}/usr
mkdir -p %{buildroot}%{prefix}
mkdir -p %{buildroot}%{prefix}/lib

#Copy the files and set the Attributes
#install -m 0755 %_builddir/usml-release/libusml.a %{buildroot}%{prefix}/lib
install -m 0755 %_builddir/usml-release/libusml.so.0.5 %{buildroot}%{prefix}/lib
#install -m 0755 %_builddir/usml-release/libusml.so %{buildroot}%{prefix}/lib
#install -m 0755 %_builddir/usml/data/* %{buildroot}%{prefix}/usml/data


#################### Clean - Clean Up Phase  #####################################################
%clean
rm -rf %_builddir/usml-release
rm -rf %{buildroot}

#################### Optional pre and post Install/Uninstall Scripts  ############################

%pre

%post
/bin/chown -R safuser:safuser %{prefix}/lib/libusml.*

#%preun
#/bin/rm -rf %{prefix}

%postun
/bin/rm -Rf %{prefix}/lib/libusml.*

#################### Files - Verify Files are installed  #########################################

%files

%defattr(-,safuser,users)

####################################### Verify #############################
#verify the Under Sea Modeling Library
%dir %{prefix}/lib
#%{prefix}/lib/libusml.a
%{prefix}/lib/libusml.so
%{prefix}/lib/libusml.so.0.5

#%dir %{prefix}/usml/data
#%{prefix}/usml/data/*

###################  Changelog #############

%changelog
* Wed Sep 16 2014 Ted Burns, AEgis Technologies<tburns@aegistg.com>
- Package development_0.05 branch for LVCMS delivery.
