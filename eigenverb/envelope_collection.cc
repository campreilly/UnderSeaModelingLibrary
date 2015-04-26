/**
 * @file envelope_collection.cc
 */
#include <usml/eigenverb/envelope_collection.h>
#include <boost/numeric/ublas/matrix_proxy.hpp>

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
void envelope_collection::add_constribution(
	size_t azimuth, const vector<double>& scatter,
	const matrix<double>& src_beam, const matrix<double>& rcv_beam,
	const eigenverb& src_verb, const eigenverb& rcv_verb )
{
	bool ok = _envelope_model.conpute_intensity(scatter,src_verb,rcv_verb) ;
	if ( ok ) {
		for ( size_t s=0 ; s < src_beam.size1() ; ++s ) {
			for ( size_t r=0 ; r < rcv_beam.size1() ; ++r ) {
				for ( size_t f=0 ; f < _transmit_freq->size() ; ++f ) {
					matrix_row< matrix<double> > envelope( *_envelopes[azimuth][s][r], f);
					matrix_row< matrix<double> > intensity( _envelope_model.intensity(), f);
					envelope += src_beam(s,f) * rcv_beam(r,f) * intensity ;
				}
			}
		}
	}
}

/**
 * Writes the envelope data to disk
 *
 * TODO Write envelope_collection data to netCDF files.
 */
void envelope_collection::write_netcdf(const char* filename) const {

}

