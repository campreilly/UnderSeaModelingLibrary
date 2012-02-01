# Cross platform build environment for the libraries and regression tests of
# the Under Sea Modeling Library (USML). This script traverses the active 
# modules and collects all library source files and headers into a single 
# add_library() call for the libusml target.  It then collects all regression 
# tests into a single add_executable() call for the usml_test target.
# The source_group() command is used to organize the files into subgroups
# in IDE's such as Visual C++.
#
# Builds:
#	- libusml library
#   - usml_test regression test executable
# Tested Generators:
#	- Visual Studio 9 2008 (creates a solution)
#	- NMAKE Makfile (w/ Visual Studio 9 2008)
# Requires:
#	- Boost unit_test_framework 1.41 or better
#	- NetCDF 3.6 or better
#
cmake_minimum_required( VERSION 2.6 )
project( USML CXX )

set( USML_VERSION_MAJOR 0 )
set( USML_VERSION_MINOR 2 )
set( USML_VERSION ${USML_VERSION_MAJOR}.${USML_VERSION_MINOR} )
set( USML_MODULES ublas types ) # netcdf ocean wave_q3d ) 

# compiler options

if( MSVC )							# Visual Studio 9 (2008) options
    set( BUILD_SHARED_LIBS OFF )
    add_definitions( -D_USE_MATH_DEFINES )	# for M_PI etc.
    # add_definitions( /fp:fast )		# fast floating point math
    
else( CMAKE_COMPILER_IS_GNUCXX )	# GNU C++ options
    set( BUILD_SHARED_LIBS ON )
    add_definitions( -ffast-math )	# fast floating point math

endif( MSVC )

include_directories( ${PROJECT_SOURCE_DIR}/.. )

# include Boost library

set( Boost_DEBUG OFF )	  # displays debug msgs during cmake
if ( BUILD_SHARED_LIBS )
    set( Boost_USE_STATIC_LIBS OFF )
    add_definitions( -DBOOST_TEST_DYN_LINK ) 
else ( BUILD_SHARED_LIBS )
    set( Boost_USE_STATIC_LIBS ON )
endif( BUILD_SHARED_LIBS )

find_package( Boost 1.48 REQUIRED COMPONENTS unit_test_framework )
if( Boost_FOUND )
    include_directories( ${Boost_INCLUDE_DIR} )
    set( Boost_DIR ${Boost_INCLUDE_DIR} CACHE PATH "Root directory for Boost library" FORCE )
    # add_definitions( ${Boost_LIB_DIAGNOSTIC_DEFINITIONS} ) # displays lib debug msgs during compile
    if( MSVC )
        add_definitions(/wd4244 /wd4996)	# quiet warning from boost library
    endif( MSVC )
    # add_definitions( -DBOOST_UBLAS_MOVE_SEMANTICS )
endif()

# fix bug in boost/numeric/ublas/vector_expression.hpp lines 1409 through 1417
# function: operator/( vector, scalar )
#
# Should be using the enable_if<> macros just like the operator*() in lines
# 1397 through 1407 (just above it).  Doing so allows further overloading of
# operator/() for other types. 

add_definitions( -DBOOST_UBLAS_CHECK_DIVISION_TYPE )

# include NetCDF library

find_path( NETCDF_DIR ${CMAKE_SHARED_MODULE_PREFFIX}netcdf_c++.${CMAKE_SHARED_MODULE_SUFFIX} )
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
        file( GLOB HDR_LIST ${MOD}${SUFFIX}/*.h )
        set( HEADERS ${HEADERS} ${HDR_LIST} )
        file( GLOB SRC_LIST ${MOD}${SUFFIX}/*.cc )
        set( SOURCES ${SOURCES} ${SRC_LIST} )
        source_group( ${MOD} FILES ${HDR_LIST} ${SRC_LIST} )
    endforeach( MOD )
    
endmacro(FIND_SOURCES)

# build the USML library

set( HEADERS "" )
set( SOURCES "" )
FIND_SOURCES( "${USML_MODULES}" "" )

add_library( libusml ${HEADERS} ${SOURCES} )
target_link_libraries( libusml ${Boost_LIBRARIES} ${NETCDF_LIBRARIES} )

# build the USML regression tests

set( HEADERS "" )
file( GLOB SOURCES usml_test.cc )
source_group( main FILES ${SOURCES} )
FIND_SOURCES( "${USML_MODULES}" "/test" )

add_executable( usml_test ${HEADERS} ${SOURCES} )
target_link_libraries( usml_test libusml )
