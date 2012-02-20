# Cross platform build environment for the libraries and regression tests of
# the Under Sea Modeling Library (USML). This script traverses the active 
# modules and collects all library source files and headers into a single 
# add_library() call for the libusml target.  It then collects all
# regression tests into a single add_executable() call for the usml_test 
# target. The source_group() command is used to organize the files into
# subgroups in IDE's such as Visual C++.
#
# Builds:
#   - libusml library
#   - usml_test regression test executable
# Tested Generators:
#   - Visual C++ 2010 (creates a solution)
#   - NMAKE Makfile (w/ Visual C++ 2010)
# Requires:
#   - Boost unit_test_framework and math_tr1 1.41 or better
#   - NetCDF 3.6 or better
#
cmake_minimum_required( VERSION 2.6 )
project( USML CXX )

set( USML_VERSION_MAJOR 0 )
set( USML_VERSION_MINOR 2 )
set( USML_VERSION ${USML_VERSION_MAJOR}.${USML_VERSION_MINOR} )
set( USML_MODULES ublas types netcdf ocean wave_q3d ) 

##############################################################################
# LIBUSML LIBRARY

# compiler options

if( MSVC )                		# Visual Studio (2010) options
    set( BUILD_SHARED_LIBS OFF )
    add_definitions( -D_USE_MATH_DEFINES ) # for M_PI etc.
    add_definitions( /fp:fast )        # fast floating point math
    set( USML_LIBRARY libusml ) 
    
else( CMAKE_COMPILER_IS_GNUCXX )	# GNU C++ options
    set( BUILD_SHARED_LIBS ON )
    set_property( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
	"Release" "Debug" "RelWithDebInfo" "MinSizeRel" )
    set( CMAKE_BUILD_TYPE Release CACHE STRING "Type of build" ) # turns on optimizations
    add_definitions( -g -ffast-math )	# fast floating point math
    set( USML_LIBRARY usml ) 

endif()

include_directories( ${PROJECT_SOURCE_DIR}/.. )

# build links to Boost shared library
# uses the BOOST_ROOT variable as a hint for 
# both the include and library files.
 
set( Boost_DEBUG OFF )         # if on -> displays debug msgs during cmake
set( Boost_USE_STATIC_LIBS OFF ) # tells cmake to use dynamic libraries
add_definitions( -DBOOST_ALL_DYN_LINK ) # tells auto-link to use dyn libs

if( MSVC )                # Visual Studio 9 (2008) options
    find_package( Boost 1.48 REQUIRED COMPONENTS 
        unit_test_framework        # for usml_test.exe
        math_tr1 math_tr1f math_tr1l    # support GNU extention like isnan() and round()
        math_c99 math_c99f math_c99l    # required by math_tr1
        )
    if( Boost_FOUND )
        add_definitions(/wd4244 /wd4996) # quiet warning from boost library
        include_directories( ${Boost_INCLUDE_DIR}/boost/tr1/tr1 ${Boost_INCLUDE_DIR} )
        # add_definitions( ${Boost_LIB_DIAGNOSTIC_DEFINITIONS} ) # displays lib debug msgs during compile
    endif( Boost_FOUND )

else( CMAKE_COMPILER_IS_GNUCXX )    # GNU C++ options
    find_package( Boost 1.48 REQUIRED COMPONENTS 
        unit_test_framework        # for usml_test.exe
        )
    if( Boost_FOUND )
        include_directories( ${Boost_INCLUDE_DIR} )
    endif( Boost_FOUND )

endif()


# fix bug in boost/numeric/ublas/vector_expression.hpp lines 1409 through 1417
# function: operator/( vector, scalar )
#
# Should be using the enable_if<> macros just like the operator*() in lines
# 1397 through 1407 (just above it).  Doing so allows further overloading of
# operator/() for other types. 

add_definitions( -DBOOST_UBLAS_CHECK_DIVISION_TYPE )

# build links to NetCDF shared library
# uses the NETCDF_DIR variable as a hint for both the include and library files.

set( NETCDF_CXX "YES" )
find_package( NetCDF 3.6 REQUIRED )
if( NETCDF_FOUND )
    include_directories( ${NETCDF_INCLUDES} ${NETCDF_INCLUDES_CXX} )
    find_program( NETCDF_NCKS ncks "NetCDF Kitchen Sink utility, generates test files")
endif( NETCDF_FOUND )

# macro: searches a module list for headers and sources

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

add_library( ${USML_LIBRARY} ${HEADERS} ${SOURCES} )
target_link_libraries( ${USML_LIBRARY} ${Boost_LIBRARIES} ${NETCDF_LIBRARIES} )

##############################################################################
# USML_TEST REGRESSION TEST

# define standard input data files for USML regression tests

set( USML_DATA_DIR ${PROJECT_SOURCE_DIR}/data 
     CACHE PATH "directory for data used in testing" )
set( USML_DATA_BATHYMETRY ${USML_DATA_DIR}/bathymetry/ETOPO1_Ice_g_gmt4.grd 
    CACHE FILEPATH "data file for ETOPO bathymetry" )
set( USML_DATA_TEMP_SEASON ${USML_DATA_DIR}/woa09/temperature_seasonal_1deg.nc
    CACHE FILEPATH "data file for deep WOA temperature profile" )
set( USML_DATA_TEMP_MONTH ${USML_DATA_DIR}/woa09/temperature_monthly_1deg.nc
    CACHE FILEPATH "data file for shallow WOA temperature profile" )
set( USML_DATA_SALT_SEASON ${USML_DATA_DIR}/woa09/salinity_seasonal_1deg.nc
    CACHE FILEPATH "data file for deep WOA salinity profile" )
set( USML_DATA_SALT_MONTH ${USML_DATA_DIR}/woa09/salinity_monthly_1deg.nc
    CACHE FILEPATH "data file for shallow WOA salinity profile" )
add_definitions( 
    -DUSML_DATA_DIR="$USML_DATA_DIR"
    -DUSML_DATA_BATHYMETRY="${USML_DATA_BATHYMETRY}"
    -DUSML_DATA_TEMP_SEASON="${USML_DATA_TEMP_SEASON}"
    -DUSML_DATA_TEMP_MONTH="${USML_DATA_TEMP_MONTH}"
    -DUSML_DATA_SALT_SEASON="${USML_DATA_SALT_SEASON}"
    -DUSML_DATA_SALT_MONTH="${USML_DATA_SALT_MONTH}" 
    )

if( MSVC )                # Visual Studio 9 (2008) options
    set( USML_TEST_DATA_DIR ${CMAKE_CFG_INTDIR} )
else( CMAKE_COMPILER_IS_GNUCXX )    # GNU C++ options
    set( USML_TEST_DATA_DIR ${PROJECT_BINARY_DIR} )
endif()

# generated data files for bathymetry tests

if ( EXISTS ${NETCDF_NCKS} ) 
    set( ETOPO_CMP ${USML_TEST_DATA_DIR}/etopo_cmp )
    add_custom_command( 
        OUTPUT ${ETOPO_CMP}.nc ${ETOPO_CMP}.log
        COMMAND ${NETCDF_NCKS} -O -a -d y,18.0,23.000001 -d x,-160.0,-154.0 
            "${USML_DATA_BATHYMETRY}" "${ETOPO_CMP}.nc"
        COMMAND ncdump "${ETOPO_CMP}.nc" > "${ETOPO_CMP}.log"
    )

    set( ETOPO_CMP2W ${USML_TEST_DATA_DIR}/etopo_cmp2w )
    add_custom_command( 
        OUTPUT ${ETOPO_CMP2W}.nc ${ETOPO_CMP2W}.log
        COMMAND ${NETCDF_NCKS} -O -a -d y,-1.0,2.000001 -d x,179.0,180.0
            "${USML_DATA_BATHYMETRY}" "${ETOPO_CMP2W}.nc"
        COMMAND ncdump "${ETOPO_CMP2W}.nc" > "${ETOPO_CMP2W}.log"
    )

    set( ETOPO_CMP2E ${USML_TEST_DATA_DIR}/etopo_cmp2e )
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
    set( WOA_CMP ${USML_TEST_DATA_DIR}/woa_cmp )
    add_custom_command( 
        OUTPUT ${WOA_CMP}.nc ${WOA_CMP}.log
        COMMAND ${NETCDF_NCKS} -O -a -v depth,time,lat,lon,t_an 
            -d time,165.0,165.0 -d lat,18.0,23.0 -d lon,200.0,206.0
            "${USML_DATA_TEMP_MONTH}" "${WOA_CMP}.nc"
        COMMAND ncdump "${WOA_CMP}.nc" > "${WOA_CMP}.log"
    )

    set( WOA_CMP2 ${USML_TEST_DATA_DIR}/woa_cmp2 )
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

set( HEADERS "" )
file( GLOB SOURCES usml_test.cc )
source_group( main FILES ${SOURCES} )
FIND_SOURCES( "${USML_MODULES}" "/test" )

add_executable( usml_test ${HEADERS} ${SOURCES} )
target_link_libraries( usml_test ${USML_LIBRARY} )

