/**
 * @file netcdf_coards.h
 * Reads a single COARDS data grid from a netCDF file.
 */
#ifndef USML_NETCDF_COARDS_H
#define USML_NETCDF_COARDS_H

#include <netcdfcpp.h>
#include <usml/ublas/ublas.h>
#include <usml/types/types.h>

namespace usml {
namespace netcdf {

using namespace usml::ublas ;
using namespace usml::types ;

/// @ingroup netcdf_files
/// @{

/**
 * Reads a single COARDS data grid from a netCDF file.
 * Assumes that the entire file should be read into memory.
 *
 * The Cooperative Ocean/Atmosphere Research Data Service
 * (COARDS) is a NOAA/university cooperative for the sharing
 * and distribution of global atmospheric and oceanographic
 * research data sets.  COARDS is also a NAVOCEANO
 * recommended practice for netCDF files.
 *
 * @xref Cooperative Ocean/Atmosphere Research Data Service,
 * "Conventions for the standardization of NetCDF files," May 1995.
 * See http://ferret.wrc.noaa.gov/noaa_coop/coop_cdf_profile.html
 * for more information.
 */
template< class DATA_TYPE, int NUM_DIMS > class netcdf_coards :
    public data_grid<DATA_TYPE,NUM_DIMS>
{
  private:

    /**
     * Construct a seq_vector from NetCDF dimension object.
     * Inspects the data to see if seq_linear or seq_log can be
     * used to optimize the performance of this dimension->
     *
     * @param  file         NetCDF file to process.
     * @param  dimension    NetCDF dimension. The COARDS spec assumes that
     *                      there is a NetCDF variable of the same name.
     * @return              Sequence vector equivalent.
     */
    seq_vector* make_axis( NcFile& file, NcDim* dimension ) {

        // search for this axis in the NetCDF file

        NcVar* axis = file.get_var( dimension->name() ) ;
        if ( axis == 0 ) {
            throw std::invalid_argument("NetCDF variable not found");
        }

        // search for linear or logrithmic pattern in this data

        const int N = (int) axis->num_vals() ;
        bool isLinear = true ;
        bool isLog = true ;
        vector<double> data(N) ;

        double p1 = axis->as_double(0) ; data(0) = p1 ;
        double minValue = p1 ;
        double maxValue = p1 ;

        if ( N > 1 ) {
            double p2 = axis->as_double(1) ; data(1) = p2 ;
            for ( int n=2 ; n < N ; ++n ) {
                double p3 = axis->as_double(n) ; data(n) = p3 ;
                maxValue = p3 ;
                // printf("diff[%d] = %f\n",n,p3-p2);
                if ( abs( ((p3-p2)-(p2-p1)) / p2 ) > 1E-4 ) {
                    isLinear = false ;
                }
                if ( p1 == 0.0 || p2 == 0.0 ||
                    abs( (p3/p2)-(p2/p1) ) > 1E-5 )
                {
                    isLog = false ;
                }
                if ( ! ( isLinear || isLog ) ) break ;
                p1 = p2 ;
                p2 = p3 ;
            }
        }

        // build a new sequence for this axis

        cout << axis->name() << " N=" << N << " minValue=" << minValue << " maxValue=" << maxValue << endl ;
        if ( isLinear ) {
            return new seq_linear( minValue, (maxValue-minValue)/(N-1), N ) ;
        } else if ( isLog ) {
            return new seq_log( minValue, (maxValue/minValue)/(N-1), N ) ;
        }
        return new seq_data( data ) ;
    }

  public:

    /**
     * Extract a named data grid from an open NetCDF file.
     * Creates new memory area for field data.
     * Replaces missing data with fill value.
     *
     * @param  file     	Reference to an open NetCDF file.
     * @param  name     	Name of the data grid to extract (case sensitive).
     * @param  read_fill	Read _FillValue from NetCDF file if true.
     * 						Use NAN as fill value if false.
     */
    netcdf_coards( NcFile& file, NcToken name, bool read_fill=false ) {

        // search for this grid in the NetCDF file

        NcVar* variable = file.get_var( name ) ;
        if ( variable == 0 ) {
            throw std::invalid_argument("NetCDF variable not found");
        }

        // read axis data from NetCDF file.

        size_t N = 1 ;
        for ( int n=0 ; n < NUM_DIMS ; ++n ) {
            seq_vector* ax = make_axis( file, variable->get_dim(n) ) ;
            this->_axis[n] = ax ;
            N *= this->_axis[n]->size() ;
        }

        // extract missing attribute information

        NcError nc_error( NcError::silent_nonfatal ) ;

        DATA_TYPE missing = NAN ;   // default value for missing data
        NcAtt* att = variable->get_att("missing_value") ;
        if ( att ) {
            missing = (DATA_TYPE) att->values()->as_double(0) ;
            delete att ;
        }

        DATA_TYPE filling = NAN ;   // default for fill value
        if ( read_fill ) {
			att = variable->get_att("_FillValue") ;
			if ( att ) filling = att->values()->as_double(0) ;
        }

        // copy interpolant data from the NetCDF file into local memory.
        // replace missing data with fill value

        NcValues* values = variable->values() ;
        this->_data = new DATA_TYPE[N] ;
        for ( unsigned n=0 ; n < N ; ++n ) {
            this->_data[n] = (DATA_TYPE) values->as_double(n) ;
            if ( ! isnan(missing) ) {
            	if ( this->_data[n] == missing ) {
            		this->_data[n] = filling ;
            	}
            }
        }
        delete values ;
    }
} ;

/// @}
}  // end of namespace netcdf
}  // end of namespace usml

#endif
