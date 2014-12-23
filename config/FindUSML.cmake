# Find USML - Find the USML includes and library
#
#  USML_INCLUDES    - where to find USML include files
#  USML_LIBRARIES   - Link these libraries when using USML
#  USML_FOUND       - True if USML found

if ( NOT ( USML_INCLUDES AND USML_LIBRARIES ) )
    find_path( USML_INCLUDES usml/usml_config.h 
        HINTS /usr/local/include USML_DIR 
        ENV USML_DIR)
    find_library( USML_LIBRARIES usml )
    set (USML_FOUND TRUE)
endif()
