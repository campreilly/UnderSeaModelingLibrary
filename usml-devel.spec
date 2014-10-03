Summary:  The goal of this program is to be able to install the Under Sea Modeling Library (usml) devel files.

Name:  usml-devel
Version: 0.5.6
Release: 0%{dist}
Source: usml-%{version}.tgz
License: BSD 2
Buildroot: /var/tmp/%{name}-buildroot

Requires: usml == %{version}
Prefix: /usr/local

%define debug_package %{nil}

%description
This will install the devel headers for the usml and overlad two boost header files


################### PREP - Used to get source ready to build #######################

%prep

%define _unpackaged_files_terminate_build 0 

%setup -n usml

#%patch


################### BUILD - Shell script used to build source code #######################

%build

# Needed for boost header files that require a patch
# boost/numeric/ublas/matrix_expression.hpp
# boost/numeric/ublas/vector_expression.hpp
%define boost_devel_location %(rpm -ql boost-devel | head -n 1)


#################### INSTALL STEP  #######################################################

%install

#Remove the directory if it exists
if [ -d %{buildroot} ]; then 
	rm -Rf %{buildroot}
fi

#Create the Directories
mkdir -p %{buildroot}%{prefix}/include/usml/netcdf
mkdir -p %{buildroot}%{prefix}/include/usml/ocean
mkdir -p %{buildroot}%{prefix}/include/usml/types
mkdir -p %{buildroot}%{prefix}/include/usml/ublas
mkdir -p %{buildroot}%{prefix}/include/usml/waveq3d
mkdir -p %{buildroot}%{boost_devel_location}/numeric/ublas

install -m 0644 $RPM_BUILD_DIR/usml/netcdf/*.h %{buildroot}%{prefix}/include/usml/netcdf/.
install -m 0644 $RPM_BUILD_DIR/usml/ocean/*.h %{buildroot}%{prefix}/include/usml/ocean/.
install -m 0644 $RPM_BUILD_DIR/usml/types/*.h %{buildroot}%{prefix}/include/usml/types/.
install -m 0644 $RPM_BUILD_DIR/usml/ublas/*.h %{buildroot}%{prefix}/include/usml/ublas/.
install -m 0644 $RPM_BUILD_DIR/usml/waveq3d/*.h %{buildroot}%{prefix}/include/usml/waveq3d/.

install -m 0644 $RPM_BUILD_DIR/usml/config/*.hpp %{buildroot}%{boost_devel_location}/numeric/ublas/.


#################### Clean - Clean Up Phase  #####################################################

%clean
#rm -rf %{buildroot}


#################### Optional pre and post Install/Uninstall Scripts  ############################

#%pre

%post

#%preun

%postun


#################### Files - Verify Files are installed  #########################################

%files
%defattr(644, root, root)

#verify the directories and files
%dir %{prefix}/include/usml
%{prefix}/include/usml/*
%dir %{boost_devel_location}/numeric/ublas/
%{boost_devel_location}/numeric/ublas/*


###################  Changelog #############

%changelog
* Thu Oct 2 2014 Ted Burns, AEgis Technologies<tburns@aegistg.com>
Version 0.5.6
	- Added boost header files that require a patch for usml to build
* Thu Sep 25 2014 Scott Stallard <scott.stallard@navy.mil> 
Version 0.5.6
	- Created this install program for the usml-devel headers

