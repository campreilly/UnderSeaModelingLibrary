/**
 * @file usml_config.h
 * Compiler specific setup for USML library
 */
#pragma once

/**
 * Setup Windows DLL export/import prefixes in USML_DECLSPEC.
 * USML_DYN_LINK if shared libraries enabled.
 * CMake automatically defines usml_EXPORT when compiling shared libraries.
*/
#if defined(_MSC_VER) && defined(USML_DYN_LINK)
    #ifdef usml_EXPORTS
        #define USML_DECLSPEC __declspec(dllexport)
	#else
        #define USML_DECLSPEC __declspec(dllimport)
	#endif
    #define USML_DLLEXPORT __declspec(dllexport)
#else
    #define USML_DECLSPEC 
    #define USML_DLLEXPORT 
#endif
