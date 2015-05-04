/**
 * @file envelope_collection.cc
 * Computes the reverberation envelope time series for all combinations of
 * receiver azimuth, source beam number, receiver beam number.
 */
#include <usml/eigenverb/envelope_collection.h>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <netcdfcpp.h>

using namespace usml::eigenverb;

/**
 * Reserve memory in which to store results as a series of
 * nested dynamic arrays.
 */
envelope_collection::envelope_collection(
	const seq_vector* transmit_freq,

	size_t num_times,
	double time_step,
	double pulse_length,
	double threshold,
	size_t num_azimuths,
	size_t num_src_beams,
	size_t num_rcv_beams
) :
	_transmit_freq(transmit_freq->clone()),
	_travel_time(0.0,time_step,num_times),
	_pulse_length(pulse_length),
	_threshold( threshold ),
	_num_azimuths(num_azimuths),
	_num_src_beams(num_src_beams),
	_num_rcv_beams(num_rcv_beams),
	_envelope_model( _transmit_freq, &_travel_time, _pulse_length, _threshold)
{
	_envelopes = new matrix<double>***[_num_azimuths];
	matrix<double>**** pa = _envelopes;
	for (size_t a = 0; a < _num_azimuths; ++a, ++pa) {
		*pa = new matrix<double>**[_num_src_beams];
		matrix<double>*** ps = *pa;
		for (size_t s = 0; s < _num_src_beams; ++s, ++ps) {
			*ps = new matrix<double>*[_num_rcv_beams];
			matrix<double>** pr = *ps;
			for (size_t r = 0; r < _num_rcv_beams; ++r, ++pr) {
				*pr = new matrix< double >(
						_transmit_freq->size(), _travel_time.size() ) ;
			}
		}
	}
}

/**
 * Delete dynamic memory in each of the nested dynamic arrays.
 */
envelope_collection::~envelope_collection() {
	matrix<double>**** pa = _envelopes;
	for (size_t a = 0; a < _num_azimuths; ++a, ++pa) {
		matrix<double>*** ps = *pa;
		for (size_t s = 0; s < _num_src_beams; ++s, ++ps) {
			matrix<double>** pr = *ps;
			for (size_t r = 0; r < _num_rcv_beams; ++r, ++pr) {
				delete *pr ;
			}
			delete[] *ps ;
		}
		delete[] *pa ;
	}
	delete[] _envelopes ;
}

/**
 * Adds the intensity contribution for a single combination of source
 * and receiver eigenverbs.
 */
void envelope_collection::add_contribution(
	const vector<double>& scatter,
	const matrix<double>& src_beam, const matrix<double>& rcv_beam,
	const eigenverb& src_verb, const eigenverb& rcv_verb )
{
	size_t azimuth = rcv_verb.az_index ;
	bool ok = _envelope_model.compute_intensity(scatter,src_verb,rcv_verb) ;
	if ( ok ) {
		for ( size_t s=0 ; s < src_beam.size2() ; ++s ) {
			for ( size_t r=0 ; r < rcv_beam.size2() ; ++r ) {
				for ( size_t f=0 ; f < _transmit_freq->size() ; ++f ) {
					matrix_row< matrix<double> > envelope( *_envelopes[azimuth][s][r], f);
					matrix_row< matrix<double> > intensity( _envelope_model.intensity(), f);
					envelope += src_beam(f,s) * rcv_beam(f,r) * intensity ;
				}
			}
		}
	}
}

/**
 * Writes the envelope data to disk
 */
void envelope_collection::write_netcdf(const char* filename) const {
	NcFile* nc_file = new NcFile(filename, NcFile::Replace);

	nc_file->add_att("pulse_length", _pulse_length ) ;
	nc_file->add_att("threshold", _threshold ) ;

	// dimensions

	NcDim* azimuth_dim = nc_file->add_dim("azimuth", (long) _num_azimuths ) ;
	NcDim* src_beam_dim = nc_file->add_dim("src_beam", (long) _num_src_beams ) ;
	NcDim* rcv_beam_dim = nc_file->add_dim("rcv_beam", (long) _num_rcv_beams ) ;
	NcDim* freq_dim = nc_file->add_dim("frequency", (long) _transmit_freq->size()) ;
	NcDim* time_dim = nc_file->add_dim("travel_time", (long) _travel_time.size()) ;

	// variables

	NcVar* freq_var = nc_file->add_var("frequency", ncDouble, freq_dim);
	NcVar* time_var = nc_file->add_var("travel_time", ncDouble, time_dim);
	NcVar* envelopes_var = nc_file->add_var("intensity", ncDouble,
		azimuth_dim, src_beam_dim, rcv_beam_dim, freq_dim, time_dim ) ;

	// units

	time_var->add_att("units", "seconds");
	freq_var->add_att("units", "hertz");
	envelopes_var->add_att("units", "dB");

	// data

	freq_var->put( _transmit_freq->data().begin(), (long) _transmit_freq->size());
	time_var->put( _travel_time.data().begin(), (long) _travel_time.size());

	for (size_t a = 0; a < _num_azimuths; ++a) {
		for (size_t s = 0; s < _num_src_beams; ++s) {
			for (size_t r = 0; r < _num_rcv_beams; ++r) {
				for (size_t f = 0; f < _transmit_freq->size(); ++f) {
					matrix_row< matrix<double> > row(*_envelopes[a][s][r], f);
					vector<double> envelope = 10.0*log10(max(row, 1e-30));
					envelopes_var->set_cur((long)a, (long)s, (long)r, (long) f, 0L );
					envelopes_var->put(envelope.data().begin(), 1L, 1L, 1L, 1L,
						(long) _travel_time.size() );
				}
			}
		}
	}

    // close file

    delete nc_file; // destructor frees all netCDF temp variables
}
