/**
@mainpage Under Sea Modeling Library (USML)

<h2>External Dependencies</h2>
This current version of this package has been tested in
the following environments:

- Ubuntu Linux 10.04 LTS (Lucid Lynx, 32 and 64-bit)
   - GNU C++ compiler (4.4.3)
   - boost 1.40.0 from http://www.boost.org
   - netcdf 3.6.3 & nco 3.9.9 from http://www.unidata.ucar.edu/software/netcdf
   - Doxygen 1.6.3 from http://www.doxygen.org

<h2>Build Process</h2>
This model is built using GNU make and the following standard targets:

   - clean   - remove compiled code and package documentation from directory
   - all     - build all compile targets (to support "make clean all")
   - default - same as all

These targets can be used in individual directories or from the root of the
usml package to apply them to all directories.  The root directory also
includes a "doc" target that invokes Doxygen to build documentation from
the source code.

The build process uses two environment variables:

   - USML_ROOT - parent of USML directory (required)
   - USML_DEBUG - turn on debugging if defined

If you build the "all" target from the root of the usml package,
it will generate a shared library called "libusml.so" and a regression
test executable called "usml_test".  The regression test should run without
errors if build was successful.  You may need to change the LD_LIBRARY_PATH
environment variable if the shared library can not be found.

The CPPFLAGS options for -pedantic -std=c++98 -Wall -Werror (all defined in
Symbols.mk) are not required. They are only included force the original 
developers to maximize cross compiler compatibility.

<h2>Style Guide</h2>
This library adopts the coding style used by Boost and the 
C++ standard library:

   - class names are lower case with underscores between words
   - private member variables start with an underscore
   - get/set accessors use the name of the property with no underscore

This library supports the following additions that go beyond
the Boost style elements:

   - classes, attributes, and methods are documented with 
     JavaDoc compatible comments in the code

Example:

@verbatim
    /**
     * This is a simple class that illustrates key features of
     * the coding style guide.
     */
    class my_simple_class {

    private:

        /* Storage for the current value. */
        int _value ;

    public:

        /**
         * Retrieve the current value.
         *
         * @return         copy of the current value
         */
        int value() {
            return _value ;
        }

        /**
         * Define the current value.
         *
         * @param  v       new value to store
         */
        void value( int v ) {
            _value = v ;
        }
    } ;
@endverbatim
*/
