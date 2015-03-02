# Cross platform build environment for the libraries and regression tests of
# the Under Sea Modeling Library (USML).
#
# This script traverses the active modules and collects all library source
# files and headers into a single add_library() call for the usml target.
# It then collects all regression tests into a single add_executable()
# call for the usml_test target. Several of these tests require data
# files to be generated, and many of these require the use of NCKS.
# Finally, it builds the USML studies.
#
# The source_group() command is used to organize the files into
# subgroups in IDE's such as Visual C++ and Eclipse.  Note that the
# Code::Blocks generator does not currently support the source_group()
# command.
#
# The install target sets up USMLConfig.cmake and the other files
# needed for users to successfully execute a find_packages( USML ).
# This includes a USMLUse.cmake file that sets of Boost and NetCDF.
#
# The uninstall target removes the files and directories created by
# the install process.
#
cmake_minimum_required( VERSION 2.6 )
project( usml CXX )

######################################################################
# setup USML

file(STRINGS config/VERSION.txt PACKAGE_VERSION)
message(STATUS "USML version ${PACKAGE_VERSION}" )
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/config" )

set( PACKAGE_MODULES ublas types netcdf ocean sensors eigenverb waveq3d)
if( MSVC )	# work around for missing netcdf_c++ on windows
    set( PACKAGE_MODULES ${PACKAGE_MODULES} netcdf/msvc )
endif( MSVC )	

option( USML_BUILD_TESTS "build all Tests" ON )
option( USML_BUILD_STUDIES "build all Studies" OFF )
option( USML_BUILD_ADDITIONS "build all Additions" OFF )

include ( USMLUse )
include_directories(
    ${PROJECT_SOURCE_DIR}/..
    ${Boost_INCLUDE_DIR}
    ${NETCDF_INCLUDES} )

######################################################################
# macro: searches a module list for headers and sources

macro( FIND_SOURCES MODULES SUFFIX )
    foreach( MOD ${MODULES} )
        message( STATUS "processing ${MOD}${SUFFIX}" )
        file( GLOB HDR_LIST ${MOD}${SUFFIX}/*.h )
        file( GLOB SRC_LIST ${MOD}${SUFFIX}/*.cc )
        set( HEADERS ${HEADERS} ${HDR_LIST} )
        set( SOURCES ${SOURCES} ${SRC_LIST} )
        source_group( ${MOD} FILES ${HDR_LIST} ${SRC_LIST} )
	   install(FILES ${HDR_LIST} DESTINATION include/usml/${MOD})
    endforeach( MOD )
endmacro(FIND_SOURCES)

######################################################################
# USML shared libraries

unset( HEADERS )
unset( SOURCES )
FIND_SOURCES( "${PACKAGE_MODULES}" "" )

if( MSVC )	# work around for missing netcdf_c++ on windows
    include_directories( "netcdf/msvc" )
endif( MSVC )	

set( TARGET usml )
add_library( ${TARGET} ${HEADERS} ${SOURCES} )
target_link_libraries( ${TARGET} ${Boost_LIBRARIES} ${NETCDF_LIBRARIES} )
set_target_properties( ${TARGET} PROPERTIES VERSION ${PACKAGE_VERSION}
                       DEBUG_POSTFIX "_d")

install( TARGETS usml DESTINATION lib )
install( FILES usml_config.h DESTINATION include/usml )

######################################################################
# Install config files for find_package()

if( NOT DEFINED USML_DIR AND DEFINED ENV{USML_DIR} )
    set( USML_DIR $ENV{USML_DIR} CACHE PATH "Root of USML library" )
endif(NOT DEFINED USML_DIR AND DEFINED ENV{USML_DIR})

if( DEFINED USML_DIR )
    set( CMAKE_INSTALL_PREFIX ${USML_DIR} CACHE PATH "Install path prefix" FORCE )
endif(DEFINED USML_DIR)

set( INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include )
set( INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib )
if ( WIN32 AND NOT CYGWIN )
  set( INSTALL_CMAKE_DIR ${CMAKE_INSTALL_PREFIX}/CMake )
else()
  set( INSTALL_CMAKE_DIR ${CMAKE_INSTALL_PREFIX}/lib/cmake/USML )
endif()
configure_file( 
    "${CMAKE_CURRENT_SOURCE_DIR}/config/USMLConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/USMLConfig.cmake"
    IMMEDIATE @ONLY )
configure_file( 
    "${CMAKE_CURRENT_SOURCE_DIR}/config/USMLConfigVersion.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/USMLConfigVersion.cmake"
    IMMEDIATE @ONLY )
install( FILES
  ${PROJECT_BINARY_DIR}/USMLConfig.cmake
  ${PROJECT_BINARY_DIR}/USMLConfigVersion.cmake 
  config/USMLUse.cmake
  config/FindNetCDF.cmake
  DESTINATION ${INSTALL_CMAKE_DIR} )

######################################################################
# Add uninstall target to project

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

######################################################################
# USML Tests

if (USML_BUILD_TESTS)
    include ( usmlBuildTest )
endif (USML_BUILD_TESTS)

######################################################################
# USML studies

if (USML_BUILD_STUDIES)
    include ( usmlBuildStudies )
endif(USML_BUILD_STUDIES)

######################################################################
# USML Additions
#
# Uses the USML_ADDITIONS_ROOT (variable or environment) to
# over-ride the root of the Additions directory.

if (USML_BUILD_ADDITIONS)
    if( NOT DEFINED USML_ADDITIONS_DIR AND DEFINED ENV{USML_ADDITIONS_ROOT} )
        set( USML_ADDITIONS_DIR $ENV{USML_ADDITIONS_ROOT} CACHE PATH "Root of USML Additions Dir" )
    else()
	 set( USML_ADDITIONS_DIR "" CACHE PATH "USML Additions directory" )
    endif( NOT DEFINED USML_ADDITIONS_DIR AND DEFINED ENV{USML_ADDITIONS_ROOT} )
    include (${USML_ADDITIONS_DIR}/usmlBuildAdditions.cmake OPTIONAL)
endif(USML_BUILD_ADDITIONS)
