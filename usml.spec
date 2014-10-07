Summary:  The goal of this program is to package the USML files for delivery.  This spec file will generate an .rpm file after having the rpmbuild (-ba) command used upon it.

Name:  usml 
Version: 0.5.6
Release: 0%{dist}
Source: usml-%{version}.tgz
Group: Development/Libraries
License: BSD 2
Buildroot: /var/tmp/%{name}-buildroot

BuildRequires: cmake >= 2.6
BuildRequires: gcc
BuildRequires: boost-devel
BuildRequires: netcdf-devel

Requires: libgcc
Prefix: /usr/local

%define debug_package %{nil}

%description 
This will install the Under Sea Modeling Library (USML)

################### PREP - Used to get source ready to build #######################
#%prep

# Set the QA_RPATHS as follows in the env
# QA_RPATHS=0x0012; export QA_RPATHS

%setup -n usml

#%patch

################### BUILD - Shell script used to build source code #######################

%build
if [ -d %{buildroot} ]; then 
	rm -Rf %{buildroot}
fi

%define boost_location %(rpm -ql boost | head -n 1)
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

#Create the USML Directories
mkdir -p %{buildroot}%{prefix}/lib

#Copy the files and set the Attributes
install -m 0644 $RPM_BUILD_DIR/usml/libusml.so.%{version} %{buildroot}%{prefix}/lib


#################### Clean - Clean Up Phase  #####################################################
%clean
rm -rf %{buildroot}

#################### Optional pre and post Install/Uninstall Scripts  ############################

#%pre

%post
cd $RPM_INSTALL_PREFIX/lib
/bin/ln -sf libusml.so.%{version} libusml.so

/sbin/ldconfig $RPM_INSTALL_PREFIX/lib

#%preun

%postun
cd $RPM_INSTALL_PREFIX/lib
/bin/rm libusml.so

#################### Files - Verify Files are installed  #########################################

%files
%defattr(-,safuser,users)

####################################### Verify #############################
#verify the Under Sea Modeling Library
%dir %{prefix}
%dir %{prefix}/lib
%{prefix}/lib/libusml.so.%{version}

#%dir %{prefix}/usml/data
#%{prefix}/usml/data/*

###################  Changelog #############

%changelog
* Thu Oct 2 2014 Ted Burns, AEgis Technologies<tburns@aegistg.com>
Version 0.5.6
	- Updated for new distribution with Reverb.
* Thu Sep 25 2014 Scott Stallard <scott.stallard@navy.mil>
Version 0.5.5
- Created this install program for the usml library
