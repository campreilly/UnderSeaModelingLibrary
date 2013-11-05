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

set( PACKAGE_VERSION_MAJOR 0 )
set( PACKAGE_VERSION_MINOR 4 )
set( PACKAGE_VERSION ${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR} )
set( PACKAGE_MODULES ublas types netcdf ocean waveq3d)
set( NETCDF_NCKS /usr/bin/ncks )

######################################################################
# set USML defaults

option( BUILD_SHARED_LIBS "build and utilize shared libraries" ON )
option( GPROF_PROFILER "build for profiling at function level timing" OFF )
option( USML_PEDANTIC "maximize warnings, treat warning as errors" ON )
option( Boost_FORCE_SHAREDLIB "Use Boost shared libraries" OFF )
option( USML_PEDANTIC "maximize warnings, treat warning as errors" ON )
option( USML_BUILD_TESTS "build all Tests" ON )
option( USML_BUILD_STUDIES "build all Studies" ON )

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

else( CMAKE_COMPILER_IS_GNUCXX )
    set_property( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Release Debug )
    if( NOT CMAKE_BUILD_TYPE )
        set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE )
    endif()
    if ( ( ${CMAKE_BUILD_TYPE} MATCHES Debug ) ) # no optimizations
        if ( PROFILE_TIME )                	# Profile Function Timing
            add_definitions( -pg -O0 )
            set ( CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -pg" )
        else()
            add_definitions( -O0 )
        endif ( PROFILE_TIME )
    else()                              	# max optimizations
        if ( PROFILE_TIME )                	# Profile Function Timing
            add_definitions( -pg -ffast-math)
            set ( CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -pg" )
        else()
            add_definitions( -g -ffast-math )
        endif ( PROFILE_TIME )
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
#include ( FindBoost )

if ( BOOST_ROOT )
    set( BOOST_NO_SYSTEM_PATHS ON )
    set( BOOST_INCLUDEDIR ${BOOST_ROOT}/include )
    set( BOOST_LIBRARYDIR ${BOOST_ROOT}/lib )
endif()

if ( BUILD_SHARED_LIBS OR Boost_FORCE_SHAREDLIB )
    set( Boost_USE_STATIC_LIBS OFF )
    add_definitions( -DBOOST_ALL_DYN_LINK )
    set( BOOST_REALPATH ON )	# use version suffix in *.so name
else ( BUILD_SHARED_LIBS )
    set( Boost_REALPATH ON )	# use version suffix in *.so name
else ()
    set( Boost_USE_STATIC_LIBS ON )
endif ( BUILD_SHARED_LIBS OR Boost_FORCE_SHAREDLIB )

set(Boost_ADDITIONAL_VERSIONS
	"1.49" "1.49.0" "1.50" "1.50.0" "1.51" "1.51.0" "1.52" "1.520")
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
# USML_TEST regression test suite

if (USML_BUILD_TESTS)

	# define standard input and output data files for USML regression tests

	set( USML_DATA_DIR ${PROJECT_SOURCE_DIR}/data
	     CACHE PATH "directory for data used in testing" )
	set( USML_TEST_DIR ${PROJECT_SOURCE_DIR}
	     CACHE PATH "root directory for testing outputs" )
	foreach( MOD ${PACKAGE_MODULES} )
	    file(MAKE_DIRECTORY ${USML_TEST_DIR}/${MOD}/test)
	endforeach( MOD )

	# generated data files for bathymetry tests

	if ( EXISTS ${NETCDF_NCKS} )
	    set( USML_DATA_BATHYMETRY ${USML_DATA_DIR}/bathymetry/ETOPO1_Ice_g_gmt4.grd
		CACHE FILEPATH "data file for ETOPO bathymetry" )
	    set( ETOPO_CMP ${PROJECT_SOURCE_DIR}/netcdf/test/etopo_cmp )
	    add_custom_command(
	        OUTPUT ${ETOPO_CMP}.nc ${ETOPO_CMP}.log
	        COMMAND ${NETCDF_NCKS} -O -a -d y,18.0,23.000001 -d x,-160.0,-154.0
	            "${USML_DATA_BATHYMETRY}" "${ETOPO_CMP}.nc"
	        COMMAND ncdump "${ETOPO_CMP}.nc" > "${ETOPO_CMP}.log"
	    )

	    set( ETOPO_CMP2W ${PROJECT_SOURCE_DIR}/netcdf/test/etopo_cmp2w )
	    add_custom_command(
	        OUTPUT ${ETOPO_CMP2W}.nc ${ETOPO_CMP2W}.log
	        COMMAND ${NETCDF_NCKS} -O -a -d y,-1.0,2.000001 -d x,179.0,180.0
	            "${USML_DATA_BATHYMETRY}" "${ETOPO_CMP2W}.nc"
	        COMMAND ncdump "${ETOPO_CMP2W}.nc" > "${ETOPO_CMP2W}.log"
	    )

	    set( ETOPO_CMP2E ${PROJECT_SOURCE_DIR}/netcdf/test/etopo_cmp2e )
	    add_custom_command(
	        OUTPUT ${ETOPO_CMP2E}.nc ${ETOPO_CMP2E}.log
	        COMMAND ${NETCDF_NCKS} -O -a -d y,-1.0,2.000001 -d x,-179.9834,-178.0
	            "${USML_DATA_BATHYMETRY}" "${ETOPO_CMP2E}.nc"
	        COMMAND ncdump "${ETOPO_CMP2E}.nc" > "${ETOPO_CMP2E}.log"
	    )

	    add_custom_target( ETOPO_TEST_DATA ALL DEPENDS
		${ETOPO_CMP}.nc ${ETOPO_CMP2W}.nc ${ETOPO_CMP2E}.nc )
	endif()

	# generated data files for profile tests

	if ( EXISTS ${NETCDF_NCKS} )
	    set( USML_DATA_TEMP_MONTH ${USML_DATA_DIR}/woa09/temperature_monthly_1deg.nc
		    CACHE FILEPATH "data file for shallow WOA temperature profile" )
	    set( WOA_CMP ${PROJECT_SOURCE_DIR}/netcdf/test/woa_cmp )
	    add_custom_command(
	        OUTPUT ${WOA_CMP}.nc ${WOA_CMP}.log
	        COMMAND ${NETCDF_NCKS} -O -a -v depth,time,lat,lon,t_an
	            -d time,165.0,165.0 -d lat,18.0,23.0 -d lon,200.0,206.0
	            "${USML_DATA_TEMP_MONTH}" "${WOA_CMP}.nc"
	        COMMAND ncdump "${WOA_CMP}.nc" > "${WOA_CMP}.log"
	    )

	    set( WOA_CMP2 ${PROJECT_SOURCE_DIR}/netcdf/test/woa_cmp2 )
	    add_custom_command(
	        OUTPUT ${WOA_CMP2}.nc ${WOA_CMP2}.log
	        COMMAND ${NETCDF_NCKS} -O -a -v depth,time,lat,lon,t_an
	            -d time,0.0,0.0 -d lat,0.5,0.5 -d lon,0.5,0.5
	            "${USML_DATA_TEMP_MONTH}" "${WOA_CMP2}.nc"
	        COMMAND ncdump "${WOA_CMP2}.nc" > "${WOA_CMP2}.log"
	    )

	    add_custom_target( WOA_TEST_DATA ALL DEPENDS ${WOA_CMP}.nc ${WOA_CMP2}.nc )
	endif()


	# build the USML regression tests
	unset( HEADERS )
	file( GLOB SOURCES usml_test.cc )
	source_group( main FILES ${SOURCES} )
	FIND_SOURCES( "${PACKAGE_MODULES}" "/test" )

	add_executable( usml_test ${HEADERS} ${SOURCES} )
	target_link_libraries( usml_test usml )
	set_property(
	   TARGET usml_test
	   PROPERTY COMPILE_DEFINITIONS
		USML_DATA_DIR="${USML_DATA_DIR}"
		USML_TEST_DIR="${USML_TEST_DIR}"
   )

endif (USML_BUILD_TESTS)

######################################################################
# USML studies

if (USML_BUILD_STUDIES)

#add_executable( cmp_speed studies/cmp_speed/cmp_speed.cc )
#target_link_libraries( cmp_speed usml )

add_executable( analytic_wedge studies/analytic_wedge/analytic_wedge.cc )
target_link_libraries( analytic_wedge usml )

add_executable( pekeris_waveguide studies/pekeris/pekeris_waveguide.cc )
target_link_libraries( pekeris_waveguide usml )

add_executable( ray_speed studies/ray_speed/ray_speed.cc )
target_link_libraries( ray_speed usml )

add_executable( eigenray_extra_test studies/eigenray_extra/eigenray_extra_test.cc )
target_link_libraries( eigenray_extra_test usml )

add_executable( pedersen_test studies/pedersen/pedersen_test.cc )
target_link_libraries( pedersen_test usml )

add_executable( malta_movie studies/malta_movie/malta_movie.cc )
target_link_libraries( malta_movie usml )

add_executable( malta_rays studies/malta_movie/malta_rays.cc )
target_link_libraries( malta_rays usml )

set_property(
   TARGET analytic_wedge pekeris_waveguide ray_speed eigenray_extra_test pedersen_test malta_movie malta_rays
   PROPERTY COMPILE_DEFINITIONS
	USML_DATA_DIR="${USML_DATA_DIR}"
	USML_STUDIES_DIR="${USML_STUDIES_DIR}"
   )

