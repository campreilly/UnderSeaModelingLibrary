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
	const seq_vector* envelope_freq,
	size_t src_freq_first,
	const seq_vector* travel_time,
	double reverb_duration,
	double pulse_length,
	double threshold,
	size_t num_azimuths,
	size_t num_src_beams,
	size_t num_rcv_beams,
	double initial_time,
	sensor_model::id_type source_id,
    sensor_model::id_type receiver_id,
    wposition1 src_position,
    wposition1 rcv_position
) :
	_envelope_freq(envelope_freq->clone()),
	_travel_time( travel_time->clip(0.0,reverb_duration) ),
	_pulse_length(pulse_length),
	_threshold( threshold ),
	_num_azimuths(num_azimuths),
	_num_src_beams(num_src_beams),
	_num_rcv_beams(num_rcv_beams),
	_initial_time(initial_time),
	_source_id(source_id),
	_receiver_id(receiver_id),
	_source_position(src_position),
	_receiver_position(rcv_position),
	_envelope_model( _envelope_freq, src_freq_first, _travel_time, _pulse_length, _threshold)
{
    // Store range from source to receiver when eigenverbs were obtained.
    _slant_range = _receiver_position.distance(_source_position);

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
						_envelope_freq->size(), _travel_time->size() ) ;
				(*pr)->clear();
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
	delete _envelope_freq ;
	delete _travel_time ;
}

/**
 * Clone
 */
envelope_collection* envelope_collection::clone()
{
    // Deep copy all data
    envelope_collection* new_collection = NULL;


    return new_collection;
}

/**
 * Adds the intensity contribution for a single combination of source
 * and receiver eigenverbs.
 */
void envelope_collection::add_contribution(
	const eigenverb& src_verb, const eigenverb& rcv_verb,
	const matrix<double>& src_beam, const matrix<double>& rcv_beam,
	const vector<double>& scatter, double xs2, double ys2 )
{
	size_t azimuth = rcv_verb.az_index ;
	bool ok = _envelope_model.compute_intensity(src_verb,rcv_verb,scatter,xs2,ys2) ;
	if ( ok ) {
		for ( size_t s=0 ; s < src_beam.size2() ; ++s ) {
			for ( size_t r=0 ; r < rcv_beam.size2() ; ++r ) {
				for ( size_t f=0 ; f < _envelope_freq->size() ; ++f ) {
					matrix_row< matrix<double> > intensity(_envelope_model.intensity(), f);
					matrix_row< matrix<double> > envelope(*_envelopes[azimuth][s][r], f);
					envelope += src_beam(f, s) * rcv_beam(f, r) * intensity;
				}
				//cout << "_envelopes=" << *_envelopes[azimuth][s][r] << endl;
			}
		}
	}
}

/**
 * Updates the envelope_collection data with the parameters provided.
 */
void envelope_collection::dead_reckon(double delta_time, double slant_range,
                                                            double prev_range) {

    // Set new slant_range
    _slant_range = slant_range;

    // Set new _distance_from_sensor - TODO

    // Set new _depth_offset_from_sensor - TODO

    write_lock_guard guard(_envelopes_mutex);

    // TODO dead_reckon envelopes
//    BOOST_FOREACH ( envelope lope, _envelopes) {
//
//        lope.time = lope.time + delta_time;
//        for (int i = 0; i < lope.frequencies->size(); ++i) {
//            lope.intensity[i] = lope.intensity[i] +
//                (20*log10(prev_range)) - (20*log10(_slant_range));
//        }
//    }
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
	NcDim* freq_dim = nc_file->add_dim("frequency", (long) _envelope_freq->size()) ;
	NcDim* time_dim = nc_file->add_dim("travel_time", (long) _travel_time->size()) ;

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

	freq_var->put( _envelope_freq->data().begin(), (long) _envelope_freq->size());
	time_var->put( _travel_time->data().begin(), (long) _travel_time->size());

	for (size_t a = 0; a < _num_azimuths; ++a) {
		for (size_t s = 0; s < _num_src_beams; ++s) {
			for (size_t r = 0; r < _num_rcv_beams; ++r) {
				for (size_t f = 0; f < _envelope_freq->size(); ++f) {
					matrix_row< matrix<double> > row(*_envelopes[a][s][r], f);
					vector<double> envelope = 10.0*log10(max(row, 1e-30));
					envelopes_var->set_cur((long)a, (long)s, (long)r, (long) f, 0L );
					envelopes_var->put(envelope.data().begin(), 1L, 1L, 1L, 1L,
						(long) _travel_time->size() );
				}
			}
		}
	}

    // close file

    delete nc_file; // destructor frees all netCDF temp variables
}
