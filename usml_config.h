/**
 * @file usml_config.h
 * Compiler specific setup for USML library
 */
#ifndef USML_CONFIG_H
#define USML_CONFIG_H

/**
 * Setup Windows DLL export/import prefixes in USML_DECLSPEC.
 * USML_DYN_LINK if shared libraries enabled.
 * USML_DLL_EXPORT is defined if currently building a DLL.
*/
#if defined(USML_DYN_LINK)
    #ifdef USML_DLL_EXPORT
        #define USML_DECLSPEC __declspec(dllexport)
    #else
        #define USML_DECLSPEC __declspec(dllimport)
    #endif
#else
    #define USML_DECLSPEC 
#endif

#endif