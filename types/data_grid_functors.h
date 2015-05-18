#pragma once

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>

namespace usml {
namespace types {

/**
 * Base functor for derivatives
 */
template<typename T>
struct derivative {
    typedef T  argument_type ;

    static void compute(
        argument_type d0, argument_type d1, argument_type dd0, argument_type dd1,
        const argument_type w0, const argument_type w1, bool deriv,
        argument_type& m, argument_type& dm )
    {
        if( d0 * d1 > 0.0 ) {
            m = (w0 + w1) / ( w0 / d0 + w1 / d1 ) ;
        }
        if( deriv && dd0 * dd1 > 0.0 ) {
            dm = (w0 + w1) / ( w0 / dd0 + w1 / dd1 ) ;
        }
    }
};

/**
 * Specialized functor for derivatives
 * used for ublas::vector<T>
 */
template<typename T>
struct derivative< boost::numeric::ublas::vector<T> > {
    typedef T  value_type ;
    typedef std::size_t    size_type ;
    typedef boost::numeric::ublas::vector<value_type>  argument_type ;

    static void compute(
        argument_type d0, argument_type d1, argument_type dd0, argument_type dd1,
        const value_type w0, const value_type w1, bool deriv,
        argument_type& m, argument_type& dm )
    {
        size_type size( d0.size() ) ;
        for(size_type i=0; i<size; ++i) {
            if( d0(i) * d1(i) > 0.0 ) {
                m(i) = (w0 + w1) / ( w0 / d0(i) + w1 / d1(i) ) ;
            }
            if( deriv && dd0(i) * dd1(i) > 0.0 ) {
                dm(i) = (w0 + w1) / ( w0 / dd0(i) + w1 / dd1(i) ) ;
            }
        }
    }
};

/**
 * Specialized functor for derivatives
 * used for ublas::vector<T>
 */
template<typename T>
struct derivative< boost::numeric::ublas::matrix<T> > {
    typedef T  value_type ;
    typedef std::size_t    size_type ;
    typedef boost::numeric::ublas::matrix<value_type>  argument_type ;

    static void compute(
        argument_type d0, argument_type d1, argument_type dd0, argument_type dd1,
        const value_type w0, const value_type w1, bool deriv,
        argument_type& m, argument_type& dm )
    {
        size_type size1( d0.size1() ) ;
        size_type size2( d0.size2() ) ;
        for(size_type i=0; i<size1; ++i) {
            for(size_type j=0; j<size2; ++j) {
                if( d0(i,j) * d1(i,j) > 0.0 ) {
                    m(i,j) = (w0 + w1) / ( w0 / d0(i,j) + w1 / d1(i,j) ) ;
                }
                if( deriv && dd0(i,j) * dd1(i,j) > 0.0 ) {
                    dm(i,j) = (w0 + w1) / ( w0 / dd0(i,j) + w1 / dd1(i,j) ) ;
                }
            }
        }
    }
};

/**
 * Base functor for end point derivatives
 */
template<typename T>
struct end_point_derivative {
    typedef T  argument_type ;

    static void compute(
        argument_type d0, argument_type d1, argument_type dd0, argument_type dd1,
        bool deriv, argument_type& m, argument_type& dm )
    {
        if( m * d0 < 0.0 ) {
            m = 0.0 ;
        } else if( (d0*d1 < 0.0) && (abs(m) > abs(3.0*d1)) ) {
            m = 3.0*d1 ;
        }
        if( deriv ) {
            if( dm * dd0 < 0.0 ) {
                dm = 0.0 ;
            } else if( (dd0*dd1 < 0.0) && (abs(dm) > abs(3.0*dd0)) ) {
                dm = 3.0*dd0 ;
            }
        } else {
            dm = argument_type(0) ;
        }
    }
};

/**
 * Specialized functor for end point derivatives
 * used for ublas::vector<T>
 */
template<typename T>
struct end_point_derivative< boost::numeric::ublas::vector<T> > {
    typedef T  value_type ;
    typedef std::size_t    size_type ;
    typedef boost::numeric::ublas::vector<value_type> argument_type ;
    
    static void compute(
        argument_type d0, argument_type d1, argument_type dd0, argument_type dd1,
        bool deriv, argument_type& m, argument_type& dm )
    {
        size_type size( d0.size() ) ;
        for(size_type i=0; i<size; ++i) {
            if( m(i) * d0(i) < 0.0 ) {
                m(i) = 0.0 ;
            } else if( (d0(i)*d1(i) < 0.0) && (abs(m(i)) > abs(3.0*d0(i))) ) {
                m(i) = 3.0*d0(i) ;
            }
            if( deriv ) {
                if( dm(i) * dd0(i) < 0.0 ) {
                    dm(i) = 0.0 ;
                } else if( (dd0(i)*dd1(i) < 0.0) && (abs(dm(i)) > abs(3.0*dd0(i))) ) {
                    dm(i) = 3.0*dd0(i) ;
                }
            } else {
                dm(i) = value_type(0) ;
            }
        }
    }
};

/**
 * Specialized functor for end point derivatives
 * used for ublas::matrix<T>
 */
template<typename T>
struct end_point_derivative< boost::numeric::ublas::matrix<T> > {
    typedef T  value_type ;
    typedef std::size_t    size_type ;
    typedef boost::numeric::ublas::matrix<value_type> argument_type ;
    
    static void compute(
        argument_type d0, argument_type d1, argument_type dd0, argument_type dd1,
        bool deriv, argument_type& m, argument_type& dm )
    {
        size_type size1( d0.size1() ) ;
        size_type size2( d0.size2() ) ;
        for(size_type i=0; i<size1; ++i) {
            for(size_type j=0; j<size2; ++j) {
                if( m(i,j) * d0(i,j) < 0.0 ) {
                    m(i,j) = 0.0 ;
                } else if( (d0(i,j)*d1(i,j) < 0.0) && (abs(m(i,j)) > abs(3.0*d0(i,j))) ) {
                    m(i,j) = 3.0*d0(i,j) ;
                }
                if( deriv ) {
                    if( dm(i,j) * dd0(i,j) < 0.0 ) {
                        dm(i,j) = 0.0 ;
                    } else if( (dd0(i,j)*dd1(i,j) < 0.0) && (abs(dm(i,j)) > abs(3.0*dd0(i,j))) ) {
                        dm(i,j) = 3.0*dd0(i,j) ;
                    }
                } else {
                    dm(i,j) = value_type(0) ;
                }
            }
        }
    }
};

/**
 * Base functor sets values to zero
 */
template<typename T>
struct initialize {
    typedef T  argument_type ;
    typedef std::size_t    size_type ;
    static void zero( argument_type& a1, const argument_type s )
    {
        a1 = argument_type(0) ;
    }
    static void zero( argument_type& a1, argument_type& a2,
                      const argument_type s )
    {
        a1 = argument_type(0) ;
        a2 = argument_type(0) ;
    }
    static void zero( argument_type& a1, argument_type& a2,
                      argument_type& a3, const argument_type s )
    {
        a1 = argument_type(0) ;
        a2 = argument_type(0) ;
        a3 = argument_type(0) ;
    }
    static void zero( argument_type& a1, argument_type& a2,
                      argument_type& a3, argument_type& a4,
                      const argument_type s )
    {
        a1 = argument_type(0) ;
        a2 = argument_type(0) ;
        a3 = argument_type(0) ;
        a4 = argument_type(0) ;
    }
    static void value( argument_type& a, const argument_type s, argument_type v )
    {
        a = v ;
    }
};

/**
 * Specialized functor that sets values to zero
 * for ublas::vector<T>
 */
template<typename T>
struct initialize< boost::numeric::ublas::vector<T> > {
    typedef T  value_type ;
    typedef std::size_t    size_type ;
    typedef boost::numeric::ublas::vector<value_type>  argument_type ;
    static void zero( argument_type& a1, const argument_type s )
    {
        a1.resize( s.size() ) ;
        a1.clear() ;
    }
    static void zero( argument_type& a1, argument_type& a2,
                      const argument_type s )
    {
        a1.resize( s.size() ) ;
        a1.clear() ;
        a2.resize( s.size() ) ;
        a2.clear() ;
    }
    static void zero( argument_type& a1, argument_type& a2,
                      argument_type& a3, const argument_type s )
    {
        a1.resize( s.size() ) ;
        a1.clear() ;
        a2.resize( s.size() ) ;
        a2.clear() ;
        a3.resize( s.size() ) ;
        a3.clear() ;
    }
    static void zero( argument_type& a1, argument_type& a2,
                      argument_type& a3, argument_type& a4,
                      const argument_type s )
    {
        a1.resize( s.size() ) ;
        a1.clear() ;
        a2.resize( s.size() ) ;
        a2.clear() ;
        a3.resize( s.size() ) ;
        a3.clear() ;
        a4.resize( s.size() ) ;
        a4.clear() ;
    }
    static void value( argument_type& a, const argument_type s, value_type v )
    {
        a = boost::numeric::ublas::vector<value_type>(s.size(), v) ;
    }
};

/**
 * Specialized functor that sets values to zero
 * for ublas::matrix<T>
 */
template<typename T>
struct initialize< boost::numeric::ublas::matrix<T> > {
    typedef T  value_type ;
    typedef std::size_t    size_type ;
    typedef boost::numeric::ublas::matrix<value_type>  argument_type ;
    static void zero( argument_type& a1, const argument_type s )
    {
        a1.resize( s.size1(), s.size2() ) ;
        a1.clear() ;
    }
    static void zero( argument_type& a1, argument_type& a2,
                      const argument_type s )
    {
        a1.resize( s.size1(), s.size2() ) ;
        a1.clear() ;
        a2.resize( s.size1(), s.size2() ) ;
        a2.clear() ;
    }
    static void zero( argument_type& a1, argument_type& a2,
                      argument_type& a3, const argument_type s )
    {
        a1.resize( s.size1(), s.size2() ) ;
        a1.clear() ;
        a2.resize( s.size1(), s.size2() ) ;
        a2.clear() ;
        a3.resize( s.size1(), s.size2() ) ;
        a3.clear() ;
    }
    static void zero( argument_type& a1, argument_type& a2,
                      argument_type& a3, argument_type& a4,
                      const argument_type s )
    {
        a1.resize( s.size1(), s.size2() ) ;
        a1.clear() ;
        a2.resize( s.size1(), s.size2() ) ;
        a2.clear() ;
        a3.resize( s.size1(), s.size2() ) ;
        a3.clear() ;
        a4.resize( s.size1(), s.size2() ) ;
        a4.clear() ;
    }
    static void value( argument_type& a, const argument_type s, value_type v )
    {
        a = boost::numeric::ublas::scalar_matrix<value_type>( s.size1(), s.size2(), v) ;
    }
};

}   // end of namespace types
}   // end of namespace usml
