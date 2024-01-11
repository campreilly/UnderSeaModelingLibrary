/**
 * @file gen_grid_utils.h
 * Utilities for supporting interpolation with uBLAS vectors and matrices in
 * gen_grid objects.
 */
#pragma once

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

namespace usml {
namespace types {

/**
 * Base functor for pchip derivatives.
 * Supports any scalar type that supports floating point mathematics.
 */
template <typename T>
struct derivative {
    typedef T argument_type;

    /**
     * Compute derivative for shape preserving piecewise Cubic Hermite
     * Interpolating Polynomial (PCHIP).
     *
     * If d0 and d1 have opposite signs or if either of them is zero, then this
     * is a discrete local minimum or maximum, and the derivative is zero. If d0
     * and d1 have the same sign, but the two intervals have different lengths,
     * then the derivative is a weighted harmonic mean, with weights determined
     * by the lengths of the two intervals.
     *
     * 						d0 = (y1-y0)/(x1-x0)
     *
     * 						d1 = (y2-y1)/(x2-x1)
     *
     * 						(w0+w1)/m = w0/d0 + w1/d1
     *
     * where w0 = 2 h0 + h1 and w1 = h0 + 2 w1.
     *
     * @xref Cleve Moler, Numerical Computing with MATLAB, Chapter 3.4
     * Shape-Preserving Piecewise Cubic,
     *
     * @param d0	Forward derivative from k-1 to k
     * @param d1	Forward derivative from k to k+1
     * @param dd0	Second derivative from k-1 to k+1
     * @param dd1	Second derivative from k to k+2
     * @param w0    Interval weight from k-1 to k
     * @param w1    Interval weight from k to k+1
     * @param deriv	True if second derivative needs to be interpolated
     * @param m		Estimated derivative in interval k to k+1
     * @param dm	Estimated second derivative in interval k to k+1
     */
    static void compute(argument_type d0, argument_type d1, argument_type dd0,
                        argument_type dd1, const argument_type w0,
                        const argument_type w1, bool deriv, argument_type& m,
                        argument_type& dm) {
        if (d0 * d1 > 0.0) {
            m = (w0 + w1) / (w0 / d0 + w1 / d1);
        }
        if (deriv && dd0 * dd1 > 0.0) {
            dm = (w0 + w1) / (w0 / dd0 + w1 / dd1);
        }
    }
};

/**
 * Specialized functor for pchip derivatives used for complex scalars.
 */
template <typename T>
struct derivative<std::complex<T> > {
    typedef std::complex<T> argument_type;

    static void compute(argument_type d0, argument_type d1, argument_type dd0,
                        argument_type dd1, const argument_type w0,
                        const argument_type w1, bool deriv, argument_type& m,
                        argument_type& dm) {
        if (abs(d0) * abs(d1) > 0.0) {
            m = (w0 + w1) / (w0 / d0 + w1 / d1);
        }
        if (deriv && abs(d0) * abs(dd1) > 0.0) {
            dm = (w0 + w1) / (w0 / dd0 + w1 / dd1);
        }
    }
};

/**
 * Specialized functor for pchip derivatives used for ublas::vector<T>.
 */
template <typename T>
struct derivative<boost::numeric::ublas::vector<T> > {
    typedef T value_type;
    typedef std::size_t size_type;
    typedef boost::numeric::ublas::vector<value_type> argument_type;

    static void compute(argument_type d0, argument_type d1, argument_type dd0,
                        argument_type dd1, const value_type w0,
                        const value_type w1, bool deriv, argument_type& m,
                        argument_type& dm) {
        size_type size(d0.size());
        for (size_type i = 0; i < size; ++i) {
            if (d0(i) * d1(i) > 0.0) {
                m(i) = (w0 + w1) / (w0 / d0(i) + w1 / d1(i));
            }
            if (deriv && dd0(i) * dd1(i) > 0.0) {
                dm(i) = (w0 + w1) / (w0 / dd0(i) + w1 / dd1(i));
            }
        }
    }
};

/**
 * Specialized functor for pchip derivatives used for ublas::matrix<T>.
 */
template <typename T>
struct derivative<boost::numeric::ublas::matrix<T> > {
    typedef T value_type;
    typedef std::size_t size_type;
    typedef boost::numeric::ublas::matrix<value_type> argument_type;

    static void compute(argument_type d0, argument_type d1, argument_type dd0,
                        argument_type dd1, const value_type w0,
                        const value_type w1, bool deriv, argument_type& m,
                        argument_type& dm) {
        size_type size1(d0.size1());
        size_type size2(d0.size2());
        for (size_type i = 0; i < size1; ++i) {
            for (size_type j = 0; j < size2; ++j) {
                if (d0(i, j) * d1(i, j) > 0.0) {
                    m(i, j) = (w0 + w1) / (w0 / d0(i, j) + w1 / d1(i, j));
                }
                if (deriv && dd0(i, j) * dd1(i, j) > 0.0) {
                    dm(i, j) = (w0 + w1) / (w0 / dd0(i, j) + w1 / dd1(i, j));
                }
            }
        }
    }
};

/**
 * Base functor for pchip end point derivatives.
 */
template <typename T>
struct end_point_derivative {
    typedef T argument_type;

    static void compute(argument_type d0, argument_type d1, argument_type dd0,
                        argument_type dd1, bool deriv, argument_type& m,
                        argument_type& dm) {
        if (m * d0 < 0.0) {
            m = 0.0;
        } else if ((d0 * d1 < 0.0) && (abs(m) > abs(3.0 * d1))) {
            m = 3.0 * d1;
        }
        if (deriv) {
            if (dm * dd0 < 0.0) {
                dm = 0.0;
            } else if ((dd0 * dd1 < 0.0) && (abs(dm) > abs(3.0 * dd0))) {
                dm = 3.0 * dd0;
            }
        } else {
            dm = argument_type(0);
        }
    }
};

/**
 * Specialized functor for pchip end point derivatives used for complex scalars.
 */
template <typename T>
struct end_point_derivative<std::complex<T> > {
    typedef std::complex<T> argument_type;

    static void compute(argument_type d0, argument_type d1, argument_type dd0,
                        argument_type dd1, bool deriv, argument_type& m,
                        argument_type& dm) {
        if (!deriv) {
            dm = argument_type(0);
        }
    }
};

/**
 * Specialized functor for pchip end point derivatives used for
 * ublas::vector<T>.
 */
template <typename T>
struct end_point_derivative<boost::numeric::ublas::vector<T> > {
    typedef T value_type;
    typedef std::size_t size_type;
    typedef boost::numeric::ublas::vector<value_type> argument_type;

    static void compute(argument_type d0, argument_type d1, argument_type dd0,
                        argument_type dd1, bool deriv, argument_type& m,
                        argument_type& dm) {
        size_type size(d0.size());
        for (size_type i = 0; i < size; ++i) {
            if (m(i) * d0(i) < 0.0) {
                m(i) = 0.0;
            } else if ((d0(i) * d1(i) < 0.0) &&
                       (abs(m(i)) > abs(3.0 * d0(i)))) {
                m(i) = 3.0 * d0(i);
            }
            if (deriv) {
                if (dm(i) * dd0(i) < 0.0) {
                    dm(i) = 0.0;
                } else if ((dd0(i) * dd1(i) < 0.0) &&
                           (abs(dm(i)) > abs(3.0 * dd0(i)))) {
                    dm(i) = 3.0 * dd0(i);
                }
            } else {
                dm(i) = value_type(0);
            }
        }
    }
};

/**
 * Specialized functor for pchip end point derivatives used for
 * ublas::matrix<T>.
 */
template <typename T>
struct end_point_derivative<boost::numeric::ublas::matrix<T> > {
    typedef T value_type;
    typedef std::size_t size_type;
    typedef boost::numeric::ublas::matrix<value_type> argument_type;

    static void compute(argument_type d0, argument_type d1, argument_type dd0,
                        argument_type dd1, bool deriv, argument_type& m,
                        argument_type& dm) {
        size_type size1(d0.size1());
        size_type size2(d0.size2());
        for (size_type i = 0; i < size1; ++i) {
            for (size_type j = 0; j < size2; ++j) {
                if (m(i, j) * d0(i, j) < 0.0) {
                    m(i, j) = 0.0;
                } else if ((d0(i, j) * d1(i, j) < 0.0) &&
                           (abs(m(i, j)) > abs(3.0 * d0(i, j)))) {
                    m(i, j) = 3.0 * d0(i, j);
                }
                if (deriv) {
                    if (dm(i, j) * dd0(i, j) < 0.0) {
                        dm(i, j) = 0.0;
                    } else if ((dd0(i, j) * dd1(i, j) < 0.0) &&
                               (abs(dm(i, j)) > abs(3.0 * dd0(i, j)))) {
                        dm(i, j) = 3.0 * dd0(i, j);
                    }
                } else {
                    dm(i, j) = value_type(0);
                }
            }
        }
    }
};

/**
 * Base functor to initialize values.
 */
template <typename T>
struct initialize {
    typedef T argument_type;
    typedef std::size_t size_type;

    /**
     * Compute scalar of all zeros.
     *
     * @param model		Template type and size of return value
     * (ignored).
     * @return			Scalar of zeros, same type and size as "model".
     */
    static argument_type zero(const argument_type model) {
        return argument_type(0);
    }

    /**
     * Compute array of all zeros.
     *
     * @param array		1D array of zeros
     * @param size		Number of elements in 1D array.
     */
    static void zero_n(argument_type* array, size_t size) {
        std::fill_n(array, size, (argument_type)0.0);
    }

    /**
     * Copy value into a scalar.
     *
     * @param model		Template type and size of return value
     * (ignored).
     * @param value		Value to be copied.
     * @return			Copy of value, same type and size as "model".
     */
    static argument_type value(const argument_type model, argument_type value) {
        return value;
    }
};

/**
 * Specialized functor to initialize values for ublas::vector<T>.
 */
template <typename T>
struct initialize<boost::numeric::ublas::vector<T> > {
    typedef T value_type;
    typedef std::size_t size_type;
    typedef boost::numeric::ublas::vector<value_type> argument_type;
    /**
     * Compute vector of all zeros.
     * @param model		Template type and size of return value.
     * @return			Vector of zeros, same type and size as "model".
     */
    static argument_type zero(const argument_type model) {
        argument_type array;
        array.resize(model.size());
        array.clear();
        return array;
    }

    /**
     * Compute array of vectors that contain all zeros.
     *
     * @param array		1D array of vectors
     * @param size		Number of elements in 1D array.
     */
    static void zero_n(argument_type* array, size_t size) {
        for (size_t m = 0; m < size; ++m) {
            array[m].clear();
        }
    }

    /**
     * Copy value into a vector.
     *
     * @param model		Template for type and size of return value.
     * @param value		Value to be copied.
     * @return			Copy of value, same type and size as "model".
     */
    static argument_type value(const argument_type model, value_type value) {
        return boost::numeric::ublas::vector<value_type>(model.size(), value);
    }
};

/**
 * Specialized functor to initialize values for ublas::matrix<T>.
 */
template <typename T>
struct initialize<boost::numeric::ublas::matrix<T> > {
    typedef T value_type;
    typedef std::size_t size_type;
    typedef boost::numeric::ublas::matrix<value_type> argument_type;

    /**
     * Compute matrix of all zeros.
     * @param model		Template type and size of return value.
     * @return			Matrix of zeros, same type and size as "model".
     */
    static argument_type zero(const argument_type model) {
        argument_type value;
        value.resize(model.size1(), model.size2());
        value.clear();
        return value;
    }

    /**
     * Compute array of vectors that contain all zeros.
     *
     * @param array		1D array of vectors
     * @param size		Number of elements in 1D array.
     */
    static void zero_n(argument_type* array, size_t size) {
        for (size_t m = 0; m < size; ++m) {
            array[m].clear();
        }
    }

    /**
     * Copy value into a matrix.
     *
     * @param model		Template for type and size of return value.
     * @param value		Value to be copied.
     * @return			Copy of value, same type and size as "model".
     */
    static argument_type value(const argument_type model, value_type value) {
        return boost::numeric::ublas::scalar_matrix<value_type>(
            model.size1(), model.size2(), value);
    }
};

}  // end of namespace types
}  // end of namespace usml
