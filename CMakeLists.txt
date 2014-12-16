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

######################################################################
# set USML defaults

option( BUILD_SHARED_LIBS "build and utilize shared libraries" ON )
option( USML_PEDANTIC "maximize warnings, treat warning as errors" OFF )
option( USML_BUILD_TESTS "build all Tests" ON )
option( USML_BUILD_STUDIES "build all Studies" OFF )
option( USML_BUILD_ADDITIONS "build all Additions" OFF )

######################################################################
# Visual C++ compiler options

if( MSVC )
    add_definitions( -fp:fast )     # fast floating point math
    add_definitions( -MP )          # multi-processor compilation
    add_definitions( -D_USE_MATH_DEFINES ) # symbols like M_PI
    add_definitions(                # quiet some harmless warning
        -D_CRT_SECURE_NO_WARNINGS
        -wd4244 -wd4996 -wd4018 -wd4251 )
    if ( BUILD_SHARED_LIBS )
        add_definitions( -DUSML_DYN_LINK )
    endif ( BUILD_SHARED_LIBS )
    if ( USML_PEDANTIC )
        add_definitions( -WX )
    endif ( USML_PEDANTIC )

######################################################################
# GNU C++ options
#
# Warning: Some compilers, at -O0 optimization levels, fail to properly 
# chain multiple boost::ublas operations into a single evaluation.  
# When this happens, the result is equal to the first operation.

else( CMAKE_COMPILER_IS_GNUCXX )
    set_property( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Release Debug )
    if( NOT CMAKE_BUILD_TYPE )
        set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE )
    endif()
    if ( ( ${CMAKE_BUILD_TYPE} MATCHES Debug ) ) # min optimizations
	add_definitions( -g -O1 )	
    else()                              	# max optimizations
	add_definitions( -g -ffast-math )
    endif()
    if ( USML_PEDANTIC )                	# standards compliance
        add_definitions( -std=c++98 -pedantic -Wall -Werror
	    -Wno-long-long -Wno-sign-compare)
    endif ( USML_PEDANTIC )
    if ( NOT BUILD_SHARED_LIBS )
        set(CMAKE_FIND_LIBRARY_SUFFIXES .a) 	# prefer static libraries
    endif ( NOT BUILD_SHARED_LIBS )
endif()

set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DUSML_DEBUG" )
include_directories( ${PROJECT_SOURCE_DIR}/.. )

######################################################################
# Boost C++ utility libraries for ublas and unit_test_framework
#
# The BOOST_ROOT variable can be used to over-ride default Boost version,
# but it must be set immediately after the cache is cleared, and before
# the FindBoost script is executed.

set( Boost_DEBUG OFF )
unset( Boost_INCLUDE_DIR CACHE ) # force FindBoost.cmake to reload each time
include ( FindBoost )

if ( BUILD_SHARED_LIBS )
    set( Boost_USE_STATIC_LIBS OFF )
    add_definitions( -DBOOST_ALL_DYN_LINK )
    set( BOOST_REALPATH ON )	# use version suffix in *.so name
else ( BUILD_SHARED_LIBS )
    set( Boost_USE_STATIC_LIBS ON )
endif ( BUILD_SHARED_LIBS )
set( Boost_REALPATH ON )	# use version suffix in *.so name

set(Boost_ADDITIONAL_VERSIONS
	"1.49" "1.49.0" "1.50" "1.50.0" "1.51" "1.51.0" 
	"1.52" "1.52.0" "1.53" "1.53.0" "1.55" "1.55.0"
	"1.56" "1.56.0" "1.57" "1.57.0"
)
find_package( Boost 1.41 REQUIRED COMPONENTS
    unit_test_framework        # for usml_test.exe
    )
if( Boost_FOUND )
    include_directories( ${Boost_INCLUDE_DIR} )
endif( Boost_FOUND )

# fix bug in boost/numeric/ublas/vector_expression.hpp lines 1409 through 1417
# function: operator/( vector, scalar )
#
# Should be using the enable_if<> macros just like the operator*() in lines
# 1397 through 1407 (just above it).  Doing so allows further overloading of
# operator/() for other types.  Same idea applies to matrix_expression.hpp.

add_definitions( -DBOOST_UBLAS_CHECK_DIVISION_TYPE )

######################################################################
# NetCDF data access library
#
# Uses the NETCDF_DIR (variable or environment) to
# over-ride the root of the include, lib, and bin directories.

if( NOT DEFINED NETCDF_DIR AND DEFINED ENV{NETCDF_DIR} )
    set( NETCDF_DIR $ENV{NETCDF_DIR} CACHE PATH "Root of NetCDF library" )
endif(NOT DEFINED NETCDF_DIR AND DEFINED ENV{NETCDF_DIR})

if( IS_DIRECTORY ${NETCDF_DIR}/include )
    list( APPEND CMAKE_INCLUDE_PATH $ENV{NETCDF_DIR}/include )
endif(IS_DIRECTORY ${NETCDF_DIR}/include)

if( IS_DIRECTORY ${NETCDF_DIR}/lib )
    list( APPEND CMAKE_LIBRARY_PATH $ENV{NETCDF_DIR}/lib )
endif(IS_DIRECTORY ${NETCDF_DIR}/lib )

set( NETCDF_CXX ON )
find_package( NetCDF 3.6 REQUIRED )
if( NETCDF_FOUND )
    include_directories( ${NETCDF_INCLUDES} ${NETCDF_INCLUDES}/.. )
    set( NETCDF_NCKS ${NETCDF_INCLUDES}/../bin/ncks )
endif( NETCDF_FOUND )

######################################################################
# macro: searches a module list for headers and sources

macro( FIND_SOURCES MODULES SUFFIX )
    foreach( MOD ${MODULES} )
        message( STATUS "processing ${MOD}${SUFFIX}" )
		file( GLOB HDR_LIST ${MOD}${SUFFIX}/*.h )
        set( HEADERS ${HEADERS} ${HDR_LIST} )
        file( GLOB SRC_LIST ${MOD}${SUFFIX}/*.cc )
        set( SOURCES ${SOURCES} ${SRC_LIST} )
        source_group( ${MOD} FILES ${HDR_LIST} ${SRC_LIST} )
	    if ( NOT ${SUFFIX} MATCHES "" )
            install(FILES ${HDR_LIST} DESTINATION include/usml/${MOD})
        endif()
    endforeach( MOD )
endmacro(FIND_SOURCES)

######################################################################
# USML shared libraries

unset( HEADERS )
unset( SOURCES )
FIND_SOURCES( "${PACKAGE_MODULES}" "" )

set( TARGET usml )
add_library( ${TARGET} ${HEADERS} ${SOURCES} )
target_link_libraries( ${TARGET} ${Boost_LIBRARIES} ${NETCDF_LIBRARIES} ${HDF5_hdf5_LIBRARY} ${ZLIB_LIBRARY} )
set_target_properties( ${TARGET} PROPERTIES VERSION ${PACKAGE_VERSION} )
install(TARGETS usml DESTINATION lib)
install(FILES usml_config.h DESTINATION include/usml)

######################################################################
# USML Tests

if (USML_BUILD_TESTS)
    include (config/usmlBuildTest.cmake)
endif (USML_BUILD_TESTS)

######################################################################
# USML studies

if (USML_BUILD_STUDIES)
    include (config/usmlBuildStudies.cmake)
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
