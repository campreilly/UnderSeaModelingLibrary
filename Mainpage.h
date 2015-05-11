/**
 * @file
 * Simple class that illustrates key features of
 * the coding style guide.

@mainpage Under Sea Modeling Library (USML)

The Under Sea Modeling Library (USML) is a collection of C++ software
development modules for sonar modeling and simulation.
The Wavefront Queue 3D (WaveQ3D) model is the component of USML 
that computes acoustic transmission loss in the ocean using 
Hybrid Gaussian Beams in Spherical/Time Coordinates.
At this time, most of the other modules provide support to WaveQ3D.

<h2>Style Guide</h2>
This library adopts the coding style used by Boost and the 
C++ standard library:

   - class names are lower case with underscores between words
   - class private/protected member variables start with an underscore
   - get/set accessors use the name of the data member with no underscore
   - local method variables will not start with an underscore

This library supports the following additions that go beyond
the Boost style elements:

   - classes, attributes, and methods are documented with 
     JavaDoc compatible comments in the code.

Example:

@verbatim
    /**
     * This is a simple class that illustrates key features of
     * the coding style guide.
     */
    class my_simple_class {

    public:

        /**
        * Constructor - Create the my_simple_class
        */
        my_simple_class(int input_val) :
            _special_value (0.0)    {

             int local_val = 0;
            _value = input_val + local_val;
        }

        /**
        * Default Constructor - Create the my_simple_class with all members defaulted
        */
        my_simple_class() :
            _special_value (0.0),
            _value(0)   {
        }

        /**
         * Get or retrieve the current value.
         *
         * @return         copy of the current value
         */
        int value() {
            return _value ;
        }

        /**
         * Set or define the current value.
         *
         * @param  v       new value to store
         */
        void value( int v ) {
            _value = v ;
        }

    protected:

        /* Storage for the special value */
        double  _special_value ;

    private:

        /* Storage for the value. */
        int _value ;

    } ;
@endverbatim
*/
