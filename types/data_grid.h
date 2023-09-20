/**
 * @file data_grid.h
 * N-dimensional data set and its associated axes.
 */
#pragma once

#include <ncvalues.h>
#include <netcdfcpp.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <cstddef>
#include <memory>
#include <sstream>
#include <string>
#include <typeinfo>

using namespace usml::ublas;

namespace usml {
namespace types {

/// @ingroup data_grid
/// @{

/** Type of interpolation used for each axis. */
enum class interp_enum {
    nearest = -1,  // nearest-neighbor interpolation
    linear = 0,    // linear interpolation (default)
    pchip = 1      // Piecewise Cubic Hermite Interpolating Polynomials
};

/**
 * @internal
 * Generalized recursive calculation of N-dimensional array index.
 * Template recursion used to un-wrap loops and increase execution speed.
 * Start recursion with Dim=NUM_DIMS-1.
 * Template specialization terminates recursion.
 * Designed to only be used internally by the data_grid class.
 *
 * @param  axis             Set of axes that define dataset.
 * @param  index            Index number in each dimension.
 */
template <size_t Dim>
inline size_t data_grid_compute_offset(const seq_vector::csptr axis[],
                                       const size_t* index) {
    return index[Dim] +
           axis[Dim]->size() * data_grid_compute_offset<Dim - 1>(axis, index);
}

/**
 * @internal
 * Final term in recursive calculation of N-dimensional array index.
 */
template <>
inline size_t data_grid_compute_offset<0>(const seq_vector::csptr axis[],
                                          const size_t* index) {
    return index[0];
}

/**
 * N-dimensional data set and its associated axes. Immutable interface for
 * sub-classes that support interpolation in any number of dimensions.
 *
 * @param  NUM_DIMS     Number of dimensions in this grid.  Specifying this
 *                      at compile time allows for some loop un-wrapping.
 */
template <size_t NUM_DIMS, class DATA_TYPE = double>
class USML_DLLEXPORT data_grid {
   public:
    /// Shared pointer to constant version of this class.
    typedef std::shared_ptr<const data_grid> csptr;

    /// Shared pointer to editable version of this class.
    typedef std::shared_ptr<data_grid> sptr;

    /// Virtual destructor
    virtual ~data_grid() {}

    /**
     * Extract a reference to the list of axes.
     */
    const seq_vector::csptr* axis_list() const { return _axis; }

    /**
     * Extract a constant shared pointer to one of the axes.
     * Allows sub-classes to take shared ownership of axes data.
     */
    seq_vector::csptr axis_csptr(size_t dim) const { return _axis[dim]; }

    /**
     * Extract a constant reference to one of the axes.
     * Passing it as a reference instead of as a csptr does not give the
     * caller the opportunity to take ownership of this object.
     */
    const seq_vector& axis(size_t dim) const { return *_axis[dim]; }

    /**
     * Extracts a shared pointer to the data member.
     * Allows sub-classes to take shared ownership of axes data.
     */
    std::shared_ptr<const DATA_TYPE[]> data_csptr() const { return _data; }

    /**
     * Extracts a const pointer to the data member.
     */
    const DATA_TYPE* data() const { return _data.get(); }

    /**
     * Extract a data value at a specific combination of indices.
     *
     * @param  index            Index number in each dimension.
     */
    DATA_TYPE data(const size_t* index) const {
        const size_t offset =
            data_grid_compute_offset<NUM_DIMS - 1>(_axis, index);
        return _data.get()[offset];
    }

    /**
     * Retrieve the type of interpolation for one of the axes.
     */
    interp_enum interp_type(size_t dimension) const {
        return _interp_type[dimension];
    }

    /**
     * Define the type of interpolation for one of the axes.
     * Note that linear interpolation requires a minimum of 2 points;
     * pchip requires a minimum of 4 points.
     *
     * @param  dimension        Dimension number to be modified.
     * @param  type             Type of interpolation for this dimension.
     */
    void interp_type(size_t dimension, interp_enum type) {
        const size_t size = this->_axis[dimension]->size();
        if (type > interp_enum::nearest && size < 2) {
            type = interp_enum::nearest;
        } else if (type > interp_enum::linear && size < 4) {
            type = interp_enum::linear;
        } else {
            this->_interp_type[dimension] = type;
        }
    }

    /**
     * Returns the edge_limit flag for the requested dimension.
     */
    bool edge_limit(size_t dimension) const { return _edge_limit[dimension]; }

    /**
     * Set the edge_limit flag for the requested dimension.
     * Note: Default is true
     *
     * @param  dimension        Dimension number to be modified.
     * @param  flag             Limits locations when true.
     */
    void edge_limit(size_t dimension, bool flag) {
        this->_edge_limit[dimension] = flag;
    }

    /**
     * Multi-dimensional interpolation with the derivative calculation.
     * So many calculations are shared between the determination of an
     * interpolate value and its derivative, that it is computationally
     * efficient to compute them both at the same time.
     *
     * Limit interpolation to axis domain if _edge_limit turned on for that
     * dimension.  Allow extrapolation if _edge_limit turned off.
     *
     * @param   location    Location at which field value is desired. Must
     *                      have the same rank as the data grid or higher.
     *                      WARNING: The contents of the location vector
     *                      may be modified if edge_limit() is true for
     *                      any dimension.
     * @param   derivative  If this is not nullptr, the first derivative
     *                      of the field at this point will also be computed.
     * @return              Value of the field at this point.
     */
    virtual DATA_TYPE interpolate(const double location[],
                                  DATA_TYPE* derivative = nullptr) const = 0;

    /**
     * Interpolation 1-D specialization where the arguments, and results,
     * are matrix<DATA_TYPE>.  This is used frequently in the WaveQ3D model
     * to interpolate environmental parameters.
     *
     * @param   x           First dimension of location.
     * @param   result      Interpolated values at each location (output).
     * @param   dx          First dimension of derivative (output).
     */
    void interpolate(const matrix<double>& x, matrix<DATA_TYPE>* result,
                     matrix<DATA_TYPE>* dx = nullptr) const {
        double location[1];
        double derivative[1];
        for (size_t n = 0; n < x.size1(); ++n) {
            for (size_t m = 0; m < x.size2(); ++m) {
                location[0] = x(n, m);
                if (dx == nullptr) {
                    (*result)(n, m) = double(interpolate(location));
                } else {
                    (*result)(n, m) = double(interpolate(location, derivative));
                    (*dx)(n, m) = double(derivative[0]);
                }
                assert(!std::isnan((*result)(n, m)));
            }
        }
    }

    /**
     * Interpolation 2-D specialization where the arguments, and results,
     * are matrix<DATA_TYPE>.  This is used frequently in the WaveQ3D model
     * to interpolate environmental parameters.
     *
     * @param   x           First dimension of location.
     * @param   y           Second dimension of location.
     * @param   result      Interpolated values at each location (output).
     * @param   dx          First dimension of derivative (output).
     * @param   dy          Second dimension of derivative (output).
     */
    void interpolate(const matrix<double>& x, const matrix<double>& y,
                     matrix<DATA_TYPE>* result, matrix<DATA_TYPE>* dx = nullptr,
                     matrix<DATA_TYPE>* dy = nullptr) const {
        double location[2];
        double derivative[2];
        for (size_t n = 0; n < x.size1(); ++n) {
            for (size_t m = 0; m < x.size2(); ++m) {
                location[0] = x(n, m);
                location[1] = y(n, m);
                if (dx == nullptr || dy == nullptr) {
                    (*result)(n, m) = double(interpolate(location));
                } else {
                    (*result)(n, m) = double(interpolate(location, derivative));
                    (*dx)(n, m) = double(derivative[0]);
                    (*dy)(n, m) = double(derivative[1]);
                }
                assert(!std::isnan((*result)(n, m)));
            }
        }
    }

    /**
     * Interpolation 3-D specialization where the arguments, and results,
     * are matrix<double>.  This is used frequently in the WaveQ3D model
     * to interpolate environmental parameters.
     *
     * @param   x           First dimension of location.
     * @param   y           Second dimension of location.
     * @param   z           Third dimension of location.
     * @param   result      Interpolated values at each location (output).
     * @param   dx          First dimension of derivative (output).
     * @param   dy          Second dimension of derivative (output).
     * @param   dz          Third dimension of derivative (output).
     */
    void interpolate(const matrix<double>& x, const matrix<double>& y,
                     const matrix<double>& z, matrix<DATA_TYPE>* result,
                     matrix<DATA_TYPE>* dx = nullptr,
                     matrix<DATA_TYPE>* dy = nullptr,
                     matrix<DATA_TYPE>* dz = nullptr) const {
        double location[3];
        double derivative[3];
        for (size_t n = 0; n < x.size1(); ++n) {
            for (size_t m = 0; m < x.size2(); ++m) {
                location[0] = x(n, m);
                location[1] = y(n, m);
                location[2] = z(n, m);
                if (dx == nullptr || dy == nullptr || dz == nullptr) {
                    (*result)(n, m) = double(interpolate(location));
                } else {
                    (*result)(n, m) = double(interpolate(location, derivative));
                    (*dx)(n, m) = double(derivative[0]);
                    (*dy)(n, m) = double(derivative[1]);
                    (*dz)(n, m) = double(derivative[2]);
                }
                assert(!std::isnan((*result)(n, m)));
            }
        }
    }

    /**
     * Output data_grid to netcdf file.
     * @param filename      name of the netcdf file to output to
     */
    void write_netcdf(const char* filename) const {
        NcFile* file = new NcFile(filename, NcFile::Replace);

        vector<const NcDim*> axis_dim(NUM_DIMS);
        vector<NcVar*> axis_var(NUM_DIMS);
        const NcDim** axis_size = new const NcDim*[NUM_DIMS];
        long* data_size = new long[NUM_DIMS];
        // Note: Using size_t instead of long for this variable
        // doesn't work on VC++ 32 bit, data_var->put() requires long.
        NcType type = ncDouble;
        const std::type_info& dt = typeid(double);
        const char* dtype = dt.name();
        if (dtype == typeid(int).name()) {
            type = ncInt;
        } else if (dtype == typeid(float).name()) {
            type = ncFloat;
        } else if (dtype == typeid(double).name()) {
            type = ncDouble;
        }

        NcVar* earth_radius = file->add_var("earth_radius", ncDouble, 0);
        for (size_t i = 0; i < NUM_DIMS; ++i) {
            std::stringstream ss;
            ss << "axis" << i << "\0";
            axis_dim[i] =
                file->add_dim((ss.str()).c_str(), (long)_axis[i]->size());
            axis_size[i] = axis_dim[i];
            axis_var[i] = file->add_var((ss.str()).c_str(), type, axis_dim[i]);
            data_size[i] = (long)_axis[i]->size();
        }
        NcVar* data_var = file->add_var("data", type, NUM_DIMS, &*axis_size);

        earth_radius->put(&wposition::earth_radius, 1);
        for (size_t i = 0; i < NUM_DIMS; ++i) {
            axis_var[i]->put(
                boost::numeric::ublas::vector<double>(*_axis[i]).data().begin(),
                data_size[i]);
        }
        data_var->put(_data.get(), data_size);

        // clean up after file completion
        delete file;
        delete[] axis_size;
        delete[] data_size;
    }

   protected:
    /// Initialize parameters for sub-classes.
    data_grid() {
        for (size_t n = 0; n < NUM_DIMS; ++n) {
            this->_interp_type[n] = interp_enum::linear;
            this->_edge_limit[n] = true;
        }
    }

    /// Axis associated with each dimension of the data grid.
    seq_vector::csptr _axis[NUM_DIMS];

    /// Defines the type of interpolation for each axis.
    interp_enum _interp_type[NUM_DIMS];

    /// Limits locations to values inside axis when true.
    bool _edge_limit[NUM_DIMS];

    /**
     * Multi-dimensional data stored as a linear array in column major order.
     * This format is used to support an N-dimensional data set
     * with any number of dimensions.
     */
    std::shared_ptr<const DATA_TYPE[]> _data;
};

}  // end of namespace types
}  // end of namespace usml
