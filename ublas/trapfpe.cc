/**
 * @file trapfpe.cc
 * Trap floating point exceptions for debugging.
 */
#if defined(USML_DEBUG)
#if defined(__GNUC__)

/**
 * Uses GCC constructor attributes (a C language extension) to invoke the
 * feenableexcept() function when the shared library is loaded.
 * @see http://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html#Function-Attributes
 */
#include <cfenv> /* floating-point control environment */
static void __attribute__((constructor)) trapfpe() {
    feenableexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW);
    // feenableexcept(FE_DIVBYZERO | FE_OVERFLOW);
}

#endif
#endif
