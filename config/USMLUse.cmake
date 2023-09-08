# Common CMake options for compiling the Under Sea Modeling Library (USML)
# and systems based on USML.  Currently it sets up:
#
#  - CMake option variables for BUILD_SHARED_LIBS and USML_PEDANTIC 
#  - Compiler options for MSVC and GNUCXX
#  - Configuration options for Boost C++ utility libraries
#  - Configuration options for NetCDF data access library

######################################################################
# CMake option variables for USML

option( BUILD_SHARED_LIBS "build and utilize shared libraries" ON )
option( USML_PEDANTIC "maximize warnings, treat warning as errors" OFF )

######################################################################
# Visual C++ compiler options

if( MSVC )
    add_definitions( -fp:fast )     # fast floating point math
    add_definitions( -MP )          # multi-processor compilation
    add_definitions( -D_USE_MATH_DEFINES ) # symbols like M_PI
    add_definitions(                # quiet some harmless warning
        -D_WIN32_WINNT=0x0501
        -D_CRT_SECURE_NO_WARNINGS
        -wd4244 -wd4996 -wd4018 -wd4251 )
    # add_definitions(                # workaround for error in
    #    -wd4005 )		              # boost::geometry 1.58
    add_definitions( -DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE )
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
    set(CMAKE_CXX_STANDARD 17)
    if ( ( ${CMAKE_BUILD_TYPE} MATCHES Debug ) ) # disable optimizations
       add_definitions( -g -O0 )
    else()                              	# fast optimizations
	   add_definitions( -g -Ofast -fno-finite-math-only )
    endif()
    add_definitions( -pthread )
    if ( USML_PEDANTIC )                	# standards compliance
        add_definitions( -pedantic -Wall -Werror -Wno-long-long -Wno-sign-compare)
        string(CONCAT CMAKE_CXX_CLANG_TIDY "clang-tidy;--fix;--checks="
            "boost-*,bugprone-*,concurrency-*,misc-*,modernize-*,"
            "performance-*,portability-*,readability-*,"
            "-bugprone-easily-swappable-parameters,"
            "-bugprone-exception-escape,"
            "-clang-analyzer-security.FloatLoopCounter,"
            "-modernize-avoid-c-arrays,"
            "-modernize-use-trailing-return-type,"
            "-modernize-pass-by-value,"
            "-readability-identifier-length,"
            "-readability-magic-numbers,"
            "-misc-const-correctness")
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
        set(CMAKE_CXX_EXTENSIONS OFF)
    endif ( USML_PEDANTIC )
    if ( NOT BUILD_SHARED_LIBS )
        set(CMAKE_FIND_LIBRARY_SUFFIXES .a) 	# prefer static libraries
    endif ( NOT BUILD_SHARED_LIBS )
endif()

# if debug is the build type then add any extra debug flags requires
set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DUSML_DEBUG" )
if( USML_DEBUG_EIGENVERBS )
	set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_EIGENVERBS" )
endif()
if( USML_DEBUG_ENVELOPE )
	set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_ENVELOPE" )
endif()
if( USML_DEBUG_EIGENRAYS )
	set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_EIGENRAYS" )
endif()
if( USML_DEBUG_EIGENRAYS_DETAIL )
	set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_EIGENRAYS_DETAIL" )
endif()	

######################################################################
# Boost C++ utility libraries for ublas and unit_test_framework
#
# The BOOST_ROOT variable can be used to over-ride default Boost version,
# but it must be set immediately after the cache is cleared, and before
# the FindBoost script is executed.

set( Boost_DEBUG OFF )

if ( BUILD_SHARED_LIBS AND NOT MSVC )
    set( Boost_USE_STATIC_LIBS OFF )
    add_definitions( -DBOOST_ALL_DYN_LINK )
    set( BOOST_REALPATH ON )	# use version suffix in *.so name
else ( )
    set( Boost_USE_STATIC_LIBS ON )
endif()

find_package( Boost 1.74 REQUIRED COMPONENTS
    unit_test_framework	# just for usml_test.exe
    timer               # just for usml_test.exe
    system
    regex
    program_options
)

include_directories( ${Boost_INCLUDE_DIR} )

######################################################################
# NetCDF data access library

if( NOT DEFINED NETCDF_DIR AND DEFINED ENV{NETCDF_DIR} )
    set( NETCDF_DIR $ENV{NETCDF_DIR} CACHE PATH "Root of NetCDF library" )
endif()
if( IS_DIRECTORY ${NETCDF_DIR}/include )
    list( APPEND CMAKE_INCLUDE_PATH $ENV{NETCDF_DIR}/include )
endif()
if( IS_DIRECTORY ${NETCDF_DIR}/lib )
    list( APPEND CMAKE_LIBRARY_PATH $ENV{NETCDF_DIR}/lib )
endif()
if( NOT MSVC )
   set( NETCDF_CXX ON )
endif()
find_package( NetCDF 3.6 MODULE REQUIRED )
find_program( NETCDF_NCKS ncks )
