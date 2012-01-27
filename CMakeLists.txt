# cross platform build environment 
# for the libraries and regression tests of
# the Under Sea Modeling Library

cmake_minimum_required( VERSION 2.6 )
project( USML CXX )

set( USML_VERSION_MAJOR 0 )
set( USML_VERSION_MINOR 2 )
set( USML_VERSION ${USML_VERSION_MAJOR}.${USML_VERSION_MINOR} )
set( USML_MODULES ublas ) # types netcdf ocean wave_q3d ) 

# incorporate Boost library

find_package( Boost 1.41 )
if( Boost_FOUND )
    set( Boost_DIR ${Boost_INCLUDE_DIR} CACHE PATH "Root directory for Boost library" FORCE )
    include_directories( ${Boost_INCLUDE_DIR} )
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

# searches a module list for headers and sources

macro( FIND_SOURCES MODULES SUFFIX )
    foreach( MOD ${MODULES} )
        file( GLOB FILE_LIST ${MOD}${SUFFIX}/*.h )
        SET( HEADERS ${HEADERS} ${FILE_LIST} )
        file( GLOB FILE_LIST ${MOD}${SUFFIX}/*.cc )
        SET( SOURCES ${SOURCES} ${FILE_LIST} )
    endforeach( MOD )
endmacro(FIND_SOURCES)

# compliler options

if( MSVC )
    add_definitions(/wd4244 /wd4996) # quiet warning from boost library
else( MSVC )
    SET(BUILD_SHARED_LIBS ON)
endif( MSVC )
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
include_directories( ${PROJECT_SOURCE_DIR}/.. )

# build the library

set( HEADERS "" )
set( SOURCES "" )
FIND_SOURCES( "${USML_MODULES}" "" )

add_library( libusml ${HEADERS} ${SOURCES} )
target_link_libraries( libusml ${NETCDF_LIBRARIES} )

# build the regression tests

set( HEADERS "" )
file( GLOB SOURCES usml_test.cc )
FIND_SOURCES( "${USML_MODULES}" "/test" )

add_executable( usml_test ${HEADERS} ${SOURCES} )
target_link_libraries( usml_test libusml )

