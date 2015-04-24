/**
 * @file envelope_collection.cc
 */
#include <usml/eigenverb/envelope_collection.h>

using namespace usml::eigenverb;

/**
 * Reserve memory in which to store results as a series of
 * nested dynamic arrays.
 */
envelope_collection::envelope_collection(
	shared_ptr<seq_vector> source_freq,
	shared_ptr<seq_vector> envelope_freq,
	double threshold,
	size_t num_azimuths,
	size_t num_src_beams,
	size_t num_rcv_beams
) :
	_source_freq(source_freq),
	_envelope_freq(envelope_freq),
	_threshold( threshold ),
	_num_azimuths(num_azimuths),
	_num_src_beams(num_src_beams),
	_num_rcv_beams(num_rcv_beams)
{
	_envelopes = new envelope_model***[_num_azimuths];
	envelope_model**** pa = _envelopes;
	for (size_t a = 0; a < _num_azimuths; ++a, ++pa) {
		*pa = new envelope_model**[_num_src_beams];
		envelope_model*** ps = *pa;
		for (size_t s = 0; s < _num_src_beams; ++s, ++ps) {
			*ps = new envelope_model*[_num_rcv_beams];
			envelope_model** pr = *ps;
			for (size_t r = 0; r < _num_rcv_beams; ++r, ++pr) {
				*pr = new envelope_model(
						_source_freq, _envelope_freq, _threshold ) ;
			}
		}
	}
}

/**
 * Delete dynamic memory in each of the nested dynamic arrays.
 */
envelope_collection::~envelope_collection() {
	envelope_model**** pa = _envelopes;
	for (size_t a = 0; a < _num_azimuths; ++a, ++pa) {
		envelope_model*** ps = *pa;
		for (size_t s = 0; s < _num_src_beams; ++s, ++ps) {
			envelope_model** pr = *ps;
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
 * Writes the envelope data to disk
 *
 * TODO Write envelope_collection data to netCDF files.
 */
void envelope_collection::write_netcdf(const char* filename) {

}

