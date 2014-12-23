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
cmake_minimum_required( VERSION 2.6 )
project( usml CXX )

file(STRINGS config/VERSION.txt PACKAGE_VERSION)
message(STATUS "USML version " ${PACKAGE_VERSION} )
set( PACKAGE_MODULES ublas types netcdf ocean waveq3d)
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/config" )

if( MSVC )	# work around for missing netcdf_c++ on windows
    set( PACKAGE_MODULES ${PACKAGE_MODULES} netcdf/msvc )
endif( MSVC )	

######################################################################
# setup USML

option( USML_BUILD_TESTS "build all Tests" ON )
option( USML_BUILD_STUDIES "build all Studies" OFF )
option( USML_BUILD_ADDITIONS "build all Additions" OFF )

include ( usmlBuildSetup )
include_directories( ${PROJECT_SOURCE_DIR}/.. )

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
	if ( NOT ${SUFFIX} MATCHES "" AND NOT ${MOD} MATCHES "netcdf/msvc" )
            install(FILES ${HDR_LIST} DESTINATION include/usml/${MOD})
        endif()
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
set_target_properties( ${TARGET} PROPERTIES VERSION ${PACKAGE_VERSION} )
install(TARGETS usml DESTINATION lib)
install(FILES usml_config.h DESTINATION include/usml)

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
