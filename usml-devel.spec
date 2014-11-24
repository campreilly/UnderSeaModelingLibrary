Summary:  The goal of this program is to be able to install the Under Sea Modeling Library (usml) devel files.

Name:  usml-devel
Version: 0.5.8
Release: 0%{dist}
Source: usml.tgz
License: none
Buildroot: /var/tmp/%{name}-buildroot

Requires: usml == %{version}
Prefix: /usr/local

%define debug_package %{nil}

%description
This will install the devel headers for the usml.


################### PREP - Used to get source ready to build #######################

#%prep

%setup -n usml

#%patch


################### BUILD - Shell script used to build source code #######################

%build


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

install -m 0644 $RPM_BUILD_DIR/usml/usml_config.h %{buildroot}%{prefix}/include/usml/.
install -m 0644 $RPM_BUILD_DIR/usml/netcdf/*.h %{buildroot}%{prefix}/include/usml/netcdf/.
install -m 0644 $RPM_BUILD_DIR/usml/ocean/*.h %{buildroot}%{prefix}/include/usml/ocean/.
install -m 0644 $RPM_BUILD_DIR/usml/types/*.h %{buildroot}%{prefix}/include/usml/types/.
install -m 0644 $RPM_BUILD_DIR/usml/ublas/*.h %{buildroot}%{prefix}/include/usml/ublas/.
install -m 0644 $RPM_BUILD_DIR/usml/waveq3d/*.h %{buildroot}%{prefix}/include/usml/waveq3d/.


#################### Clean - Clean Up Phase  #####################################################

%clean
rm -rf %{buildroot}


#################### Optional pre and post Install/Uninstall Scripts  ############################

#%pre

%post

#%preun

%postun


#################### Files - Verify Files are installed  #########################################

%files
%defattr(644, root, root)

#verify the main exe
%dir %{prefix}/include/usml
%{prefix}/include/usml/*


###################  Changelog #############

%changelog
* Thu Nov 12 2014 Ted Burns, AEgis Technologies<tburns@aegistg.com>
Version 0.5.8
	- Removed boost::math::isnan calls, replaced with std::isnan
* Thu Oct 15 2014 Ted Burns, AEgis Technologies<tburns@aegistg.com>
Version 0.5.7
	- Added parameter to data_grid_mackenzie.constuct method to prevent deletion of temp & sal.
* Thu Oct 2 2014 Ted Burns, AEgis Technologies<tburns@aegistg.com>
Version 0.5.6
	- Updated for new distribution with Reverb.
* Thu Sep 25 2014 Scott Stallard <scott.stallard@navy.mil> 
- Created this install program for the usml-devel headers

