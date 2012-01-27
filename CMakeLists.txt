# configures the build environment 
# for the libraries and regression tests of
# the Under Sea Modeling Library

cmake_minimum_required( VERSION 2.6 )
project( LIBUSML CXX )

# incorporate Boost library

find_package( Boost 1.41 )
if( Boost_FOUND )
    include_directories( ${Boost_INCLUDE_DIR} )
    set( Boost_DIR ${Boost_INCLUDE_DIR} CACHE PATH "Root directory for Boost library" FORCE )
endif()

# incorporate C++ interface from NetCDF

find_path(NETCDF_DIR netcdf_c++.dll)
set( NETCDF_INCLUDES "${NETCDF_DIR}/src/libsrc" CACHE PATH "Include directory for NetCDF C interface" )
set( NETCDF_INCLUDES_CXX "${NETCDF_DIR}/src/cxx" CACHE PATH "Include directory for NetCDF CXX interface" )
set( NETCDF_CXX "YES" )
find_package( NETCDF 3.6 )
if(NETCDF_FOUND)
    include_directories( ${NETCDF_INCLUDES} ${NETCDF_INCLUDES_CXX} )
endif()
