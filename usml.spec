Summary:  The goal of this program is to be able to install the Under Sea Modeling Library (usml) files.

Name:  usml 
Version: 0.5.9
Release: 0%{dist}
Source: usml.tgz
Group: Development/Libraries
License: BSD 2
Buildroot: /var/tmp/%{name}-buildroot

BuildRequires: cmake >= 2.6
BuildRequires: gcc
BuildRequires: boost
BuildRequires: netcdf

Requires: libgcc
Prefix: /usr/local

%define debug_package %{nil}

%description
This will install the binaries for the usml.

%package devel 
Group: Development/Headers
Summary: Headers to compile against the usml library


Requires: usml == %{version}-%{release}
%description devel
This will install the headers for the libdogapi.

################### PREP - Used to get source ready to build #######################

#%prep

%setup -n usml

#%patch


################### BUILD - Shell script used to build source code #######################

%build
if [ -d %{buildroot} ]; then 
	rm -Rf %{buildroot}
fi

%define boost_location %(dirname $(rpm -ql boost | head -n 1))
%define netcdf_location %(rpm -ql netcdf | head -n 1)
%define cmake %(rpm -ql cmake | grep bin/cmake)
%define cpack %(rpm -ql cmake | grep bin/cpack)
%define ctest %(rpm -ql cmake | grep bin/ctest)
%define ccmake %(rpm -ql cmake | grep bin/ccmake)

if [ -e CMakeCache.txt ]; then
	rm CMakeCache.txt
fi

sed -i "s/set_property/#set_property/g" CMakeLists.txt

cmake -D CMAKE_INSTALL_PREFIX:PATH=%{buildroot}%{prefix} \
	-D BOOST_LIBRARYDIR:PATH=%{boost_location} \
	-D Boost_INCLUDE_DIR:PATH=%{boost_location}/include \
	-D Boost_LIBRARY_DIR:PATH=%{boost_location}/lib \
	-D Boost_LIBRARY_DIRS:PATH=%{boost_location}/lib \
	-D Boost_UNIT_TEST_FRAMEWORK_LIBRARY:FILEPATH=%{boost_location}/lib/libboost_unit_test_framework.so \
	-D Boost_UNIT_TEST_FRAMEWORK_LIBRARY_DEBUG:FILEPATH=%{boost_location}/lib/libboost_unit_test_framework.so \
	-D Boost_UNIT_TEST_FRAMEWORK_LIBRARY_RELEASE:FILEPATH=%{boost_location}/lib/libboost_unit_test_framework.so \
	-D NETCDF_INCLUDES:PATH=%{netcdf_location}/include \
	-D NETCDF_INCLUDES_CXX:PATH=%{netcdf_location}/include \
	-D NETCDF_LIBRARIES:STRING=%{netcdf_location}/lib/libnetcdf_c++.so\;%{netcdf_location}/lib/libnetcdf.so \
	-D NETCDF_LIBRARIES_C:FILEPATH=%{netcdf_location}/lib/libnetcdf.so \
	-D NETCDF_LIBRARIES_CXX:FILEPATH=%{netcdf_location}/lib/libnetcdf_c++.so \
	-D usml_LIB_DEPENDS:STATIC=general\;%{netcdf_location}/lib/libboost_unit_test_framework.so\;general\;%{netcdf_location}/lib/libnetcdf_c++.so\;general\;%{netcdf_location}/lib/libnetcdf.so \
	-D usml_BINARY_DIR:STATIC=$RPM_BUILD_DIR/usml \
	-D usml_SOURCE_DIR:STATIC=$RPM_BUILD_DIR/usml \
	-D CMAKE_MODULE_PATH:FILEPATH=$RPM_BUILD_DIR/usml/config \
	-D CMAKE_CACHEFILE_DIR:INTERNAL=$RPM_BUILD_DIR/usml \
	-D CMAKE_CPACK_COMMAND:INTERNAL=%{cpack} \
	-D CMAKE_CTEST_COMMAND:INTERNAL=%{ctest} \
	-D CMAKE_EDIT_COMMAND:INTERNAL=%{ccmake} \
	-D CMAKE_COMMAND:INTERNAL=%{cmake} \
	-D USML_BUILD_TESTS:BOOL=OFF .

make usml install/fast


#################### INSTALL STEP  #######################################################

%install

#non-devel files
%{__mkdir_p} %{buildroot}%{prefix}/lib

%{__install} -m 0644 $RPM_BUILD_DIR/usml/libusml.so.%{version} %{buildroot}%{prefix}/lib

#devel files
%{__mkdir_p} %{buildroot}%{prefix}/include/usml/netcdf
%{__mkdir_p} %{buildroot}%{prefix}/include/usml/ocean
%{__mkdir_p} %{buildroot}%{prefix}/include/usml/types
%{__mkdir_p} %{buildroot}%{prefix}/include/usml/ublas
%{__mkdir_p} %{buildroot}%{prefix}/include/usml/waveq3d

%{__install} -m 0644 $RPM_BUILD_DIR/usml/usml_config.h %{buildroot}%{prefix}/include/usml/.
%{__install} -m 0644 $RPM_BUILD_DIR/usml/netcdf/*.h %{buildroot}%{prefix}/include/usml/netcdf/.
%{__install} -m 0644 $RPM_BUILD_DIR/usml/ocean/*.h %{buildroot}%{prefix}/include/usml/ocean/.
%{__install} -m 0644 $RPM_BUILD_DIR/usml/types/*.h %{buildroot}%{prefix}/include/usml/types/.
%{__install} -m 0644 $RPM_BUILD_DIR/usml/ublas/*.h %{buildroot}%{prefix}/include/usml/ublas/.
%{__install} -m 0644 $RPM_BUILD_DIR/usml/waveq3d/*.h %{buildroot}%{prefix}/include/usml/waveq3d/.




#################### Clean - Clean Up Phase  #####################################################

%clean
rm -rf %{buildroot}


#################### Optional pre and post Install Scripts  ############################
%pre
#if [ "$1" = "1" ]; then
	#perform tasks to prepare for the initial installation
	

#elif [ "$1" = "2" ]; then
if [ "$1" = "2" ]; then
	#perform whatever maintenance must occur before the upgrade begins
	#remove old sybolic link
	%{__rm}  $RPM_INSTALL_PREFIX/lib/libusml.so
fi


%post
#if [ "$1" = "1" ]; then
	#perform tasks for after the initial installation

#elif [ "$1" = "2" ]; then
	#perform whatever maintenance must occur after the upgrade
#fi
cd $RPM_INSTALL_PREFIX/lib
%{__ln_s} libusml.so.%{version} libusml.so

/sbin/ldconfig $RPM_INSTALL_PREFIX/lib



######################### Optional pre and post Uninstall sections ##################################################
%preun
#if [ "$1" = "1" ]; then
	#perform tasks to prepare for an upgrade(called after the upgraded rpm has been installed)

#if [ "$1" = "0" ]; then
	#perform tasks to prepare for an uninstall
	#%{__rm} %{prefix}/lib/libdogapi.so
#fi



%postun
cd $RPM_INSTALL_PREFIX/lib
#if [ "$1" = "1" ]; then
	#perform tasks to prepare for an upgrade(called after the upgraded rpm has been installed)
	
#elif [ "$1" = "0" ]; then
if [ "$1" = "0" ]; then
	#perform tasks to prepare for an uninstall
	%{__rm} libusml.so
fi



#################### Files - Verify Files are installed  #########################################

%files
%defattr(644, root, root) 

%{prefix}/lib/libusml.so.%{version}

%files devel
%defattr(644, root, root) 

%{prefix}/include/usml/*

###################  Changelog #############

%changelog
* Thu Nov 24 2014 Ted Burns, AEgis Technologies<tburns@aegistg.com>
Version 0.5.9
	- Added checkEigenray callback to the eigenrayListener.
	- Updated scripts for new usml.spec file.
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
- Created this install program for the usml library

