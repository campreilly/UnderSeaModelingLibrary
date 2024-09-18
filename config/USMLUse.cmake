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
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(CMAKE_CXX_STANDARD 17)
    if ( ( ${CMAKE_BUILD_TYPE} MATCHES Debug ) ) # disable optimizations
       add_definitions( -g -O0 )
    else()                              	# fast optimizations
	   add_definitions( -g -Ofast -fno-finite-math-only )
    endif()
    add_definitions( -pthread -fPIC )
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
            "-misc-const-correctness,"
	    "-performance-avoid-endl,"
	    "-misc-include-cleaner")
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
        set(CMAKE_CXX_EXTENSIONS OFF)
    endif ( USML_PEDANTIC )
    if ( NOT BUILD_SHARED_LIBS )
        set(CMAKE_FIND_LIBRARY_SUFFIXES .a) 	# prefer static libraries
    endif ( NOT BUILD_SHARED_LIBS )
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

# ignore BOOST deprecated headers

add_definitions("-DBOOST_ALLOW_DEPRECATED_HEADERS")
add_definitions("-DBOOST_BIND_GLOBAL_PLACEHOLDERS")
  
# speed up execution using move semantics

add_definitions( -DBOOST_UBLAS_MOVE_SEMANTICS )

######################################################################
# NetCDF data access library

# C
find_path(NETCDF_INCLUDES_C NAMES netcdf.h
    HINTS ${NETCDF_ROOT} PATH_SUFFIXES include)
find_library(NETCDF_LIBRARIES_C NAMES netcdf
    HINTS ${NETCDF_ROOT} PATH_SUFFIXES lib)

# CXX4
find_path(NETCDF_INCLUDES_CXX4 NAMES netcdf
    HINTS ${NETCDF_CXX4_ROOT} PATH_SUFFIXES include)
find_library(NETCDF_LIBRARIES_CXX4 NAMES netcdf_c++4 netcdf-cxx4
    HINTS ${NETCDF_CXX4_ROOT} PATH_SUFFIXES lib)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (NetCDF DEFAULT_MSG
    NETCDF_LIBRARIES_C
    NETCDF_LIBRARIES_CXX4
    NETCDF_INCLUDES_C
    NETCDF_INCLUDES_CXX4
)

find_program( NETCDF_NCKS ncks )
