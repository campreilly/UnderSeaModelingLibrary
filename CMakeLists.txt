# Cross platform build environment 
# for the libraries and regression tests of
# the Under Sea Modeling Library (USML).
#
# Builds:
#	libusml library
#     usml_test regression test executable
# Tested with:
#	Visual Studio 9 (2008) Project File Generator
#	Visual Studio 9 (2008) NMAKE Generator
# Requires:
#	Boost unit_test_framework 1.41 or better
#	NetCDF 3.6 or better
#
cmake_minimum_required( VERSION 2.6 )
project( USML CXX )

set( USML_VERSION_MAJOR 0 )
set( USML_VERSION_MINOR 2 )
set( USML_VERSION ${USML_VERSION_MAJOR}.${USML_VERSION_MINOR} )
set( USML_MODULES ublas ) # types netcdf ocean wave_q3d ) 

# platform specific options

if( MSVC )
    set( BUILD_SHARED_LIBS OFF )
    add_definitions(-D_USE_MATH_DEFINES)	# for M_PI etc.
else( MSVC )
    set( BUILD_SHARED_LIBS ON )
endif( MSVC )

# configure Boost library usage

set( Boost_DEBUG OFF )	  # displays debug msgs during cmake
if ( BUILD_SHARED_LIBS )
    set( Boost_USE_STATIC_LIBS OFF )
    add_definitions( -DBOOST_TEST_DYN_LINK ) 
else ( BUILD_SHARED_LIBS )
    set( Boost_USE_STATIC_LIBS ON )
endif( BUILD_SHARED_LIBS )

find_package( Boost 1.41 REQUIRED COMPONENTS unit_test_framework )
if( Boost_FOUND )
    include_directories( ${Boost_INCLUDE_DIR} )
    set( Boost_DIR ${Boost_INCLUDE_DIR} CACHE PATH "Root directory for Boost library" FORCE )
    # add_definitions( ${Boost_LIB_DIAGNOSTIC_DEFINITIONS} ) # displays lib debug msgs during compile
    if( MSVC )
        add_definitions(/wd4244 /wd4996)	# quiet warning from boost library
    endif( MSVC )
endif()

# configure NetCDF library usage

find_path( NETCDF_DIR netcdf_c++.dll )
set( NETCDF_INCLUDES "${NETCDF_DIR}/src/libsrc" CACHE PATH "Include directory for NetCDF C interface" )
set( NETCDF_INCLUDES_CXX "${NETCDF_DIR}/src/cxx" CACHE PATH "Include directory for NetCDF CXX interface" )
set( NETCDF_CXX "YES" )
find_package( NETCDF 3.6 )
if( NETCDF_FOUND )
   include_directories( ${NETCDF_INCLUDES} ${NETCDF_INCLUDES_CXX} )
endif()

# searches a module list for headers and sources

macro( FIND_SOURCES MODULES SUFFIX )
    foreach( MOD ${MODULES} )
        file( GLOB FILE_LIST ${MOD}${SUFFIX}/*.h )
        SET( HEADERS ${HEADERS} ${FILE_LIST} )
        file( GLOB FILE_LIST ${MOD}${SUFFIX}/*.cc )
        SET( SOURCES ${SOURCES} ${FILE_LIST} )
    endforeach( MOD )
endmacro(FIND_SOURCES)

# build the USML library

include_directories( ${PROJECT_SOURCE_DIR}/.. )
set( HEADERS "" )
set( SOURCES "" )
FIND_SOURCES( "${USML_MODULES}" "" )

add_library( libusml ${HEADERS} ${SOURCES} )
target_link_libraries( libusml ${Boost_LIBRARIES} ${NETCDF_LIBRARIES} )

# build the USML regression tests

set( HEADERS "" )
file( GLOB SOURCES usml_test.cc )
FIND_SOURCES( "${USML_MODULES}" "/test" )

add_executable( usml_test ${HEADERS} ${SOURCES} )
target_link_libraries( usml_test libusml )
