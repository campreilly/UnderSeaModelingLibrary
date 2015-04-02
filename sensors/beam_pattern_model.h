/**
 *  @file beam_pattern_model.h
 *  Generic interface for beam patterning
 */
#pragma once

#include <usml/types/types.h>
#include <usml/threads/read_write_lock.h>
#include <usml/threads/smart_ptr.h>
#include <boost/numeric/ublas/vector_expression.hpp>

namespace usml {
namespace sensors {

using boost::numeric::ublas::vector;
using namespace usml::types;
using namespace usml::threads;

/// @ingroup beams
/// @{

/**
 * A "beam pattern" computes the gain for an incident wave as a function
 * of incident angles, beam steering angle, and frequency.
 *
 * This class implements an abstract function used to compute both
 * the directivity index of the array and the beam level gain.
 *
 * A beam pattern function is constructed based on the physical spacing
 * of the elements and the wavelength of the incident acoustic energy.
 * This function is then used to compute the array gain in the specific
 * direction of an incident acoustic wave.
 *
 * When the signal is a unidirectional plane wave, hence perfectly coherent,
 * and when the noise is isotropic, the array gain reduces to the
 * directivity index.
 *
 * @xref R.J. Urick, Principles of Underwater Sound, 3rd Edition,
 * (1983), p. 42.
 *
 * Many properties of the beam patterns depend on predetermined values.
 * Such as the beam steering angles, frequency spectrum, and physical
 * arrangement of the elements. Using these, many variables can be
 * pre-computed and cached locally to reduce computation time.
 */
class USML_DECLSPEC beam_pattern_model {

public:

	/**
	 * Data type used for beamId.
	 */
	typedef int id_type;

	/**
	 * Data type used for beamId.
	 */
	typedef shared_ptr<beam_pattern_model> reference;

	/**
	 * Identification used to find this beam pattern in beam_pattern_map.
	 */
	id_type beamID() const {
		return _beamID;
	}

	/**
	 * Identification used to find this beam pattern in beam_pattern_map.
	 */
	void beamID(id_type beamID) {
		_beamID = beamID;
	}

	/**
	 * Computes the beam level gain along a specific DE and AZ direction
	 * for a specific beam steering angle. The DE and AZ are passed in as
	 * Eta/VarPhi values and then transformed to a theta/phi equivalent
	 * that are used for computation.
	 *
	 * NOTE: The choice of return in linear units
	 * is a development choice and may be changed depending on how
	 * this value is used in the final release state.
	 *
	 * @param de            Depression/Elevation angle (rad)
	 * @param az            Azimuthal angle (rad)
     * @param theta         spherical offset in theta from reference axis (rad)
     * @param phi           spherical offset in phi from reference axis (rad)
	 * @param frequencies   List of frequencies to compute beam level for
	 * @param level         Beam level for each frequency (linear units)
	 */
	virtual void beam_level(double de, double az, double theta, double phi,
			const vector<double>& frequencies, vector<double>* level) = 0;

	/**
	 * Accesor to the directivity index
	 *
	 * @param frequencies    list of frequencies
	 * @param level          directivity index for these frequency
	 */
	virtual void directivity_index(const vector<double>& frequencies,
			vector<double>* level) = 0;

	/**
	 * Destructor
	 */
	virtual ~beam_pattern_model() {
	}

protected:

	/**
	 * Reader-write lock for multi-thread access.
	 */
	read_write_lock _mutex;

private:

	/**
	 * Identification used to find this beam pattern in beam_pattern_map.
	 */
	id_type _beamID;

};

/// @}
}// end of namespace sensors
}   // end of namespace usml
