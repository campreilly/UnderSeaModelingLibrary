/**
 * @file eigenray_collection.cc
 * List of targets and their associated propagation data.
 */

#include <ncvalues.h>
#include <netcdfcpp.h>
#include <usml/ocean/profile_model.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition.h>
#include <usml/types/wposition1.h>
#include <usml/types/wvector.h>
#include <usml/types/wvector1.h>
#include <usml/ublas/math_traits.h>

#include <algorithm>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/storage.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <usml/eigenrays/eigenray_model.h>
#include <usml/eigenrays/eigenray_collection.h>
#include <cmath>
#include <complex>
#include <cstddef>
#include <list>
#include <memory>
#include <utility>

using namespace usml::eigenrays;

/**
 * Initialize the acoustic propagation effects associated
 * with each target.
 */
eigenray_collection::eigenray_collection(seq_vector::csptr frequencies,
                                         const wposition1 &source_pos,
                                         const wposition *target_pos,
                                         int source_id,
                                         const matrix<int> &target_ids)
    : _source_id(source_id),
      _target_ids(target_pos->size1(), target_pos->size2()),
      _source_pos(source_pos),
      _target_pos(target_pos),
      _frequencies(std::move(frequencies)),
      _eigenrays(target_pos->size1(), target_pos->size2()),
      _initial_time(target_pos->size1(), target_pos->size2()),
      _num_eigenrays(0),
      _total(target_pos->size1(), target_pos->size2()) {
    for (size_t t1 = 0; t1 < size1(); ++t1) {
        for (size_t t2 = 0; t2 < size2(); ++t2) {
            eigenray_model loss;
            loss.intensity.resize(_frequencies->size());
            loss.intensity.clear();
            loss.phase.resize(_frequencies->size());
            loss.phase.clear();
            _total(t1, t2) = loss;
            _initial_time(t1, t2) = 0.0;
            if (target_ids.size1() > 0 && target_ids.size2() > 0) {
                _target_ids(t1, t2) = target_ids(t1, t2);
            } else {
                _target_ids(t1, t2) = 0;
            }
        }
    }
}

/**
 * Find eigenrays for a single target in the grid.
 */
eigenray_list eigenray_collection::find_eigenrays(int targetID) const {
    for (size_t t1 = 0; t1 < size1(); ++t1) {
        for (size_t t2 = 0; t2 < size2(); ++t2) {
            if (_target_ids(t1, t2) == targetID || targetID == 0) {
                return eigenrays(t1, t2);
            }
        }
    }
    return {};
}

/**
 * Find fastest eigenray for a single target in the grid.
 */
double eigenray_collection::find_initial_time(int targetID) const {
    for (size_t t1 = 0; t1 < size1(); ++t1) {
        for (size_t t2 = 0; t2 < size2(); ++t2) {
            if (_target_ids(t1, t2) == targetID || targetID == 0) {
                return initial_time(t1, t2);
            }
        }
    }
    return 0.0;
}

/**
 * Notifies the observer that a wave front collision has been detected for
 * one of the targets. Targets are specified by a row and column number.
 * Must be overloaded by sub-classes.
 */
void eigenray_collection::add_eigenray(size_t t1, size_t t2,
                                       eigenray_model::csptr ray, size_t /*runID*/) {
    _eigenrays(t1, t2).push_back(ray);
    auto old_initial = _initial_time(t1, t2);
    auto new_initial = ray->time;
    if (old_initial <= 0.0 || old_initial > new_initial) {
        _initial_time(t1, t2) = new_initial;
    }
    ++_num_eigenrays;
}

/**
 * Compute propagation loss summed over all eigenrays.
 */
void eigenray_collection::sum_eigenrays(bool coherent) {
    for (size_t t1 = 0; t1 < size1(); ++t1) {
        for (size_t t2 = 0; t2 < size2(); ++t2) {
            const eigenray_list &ray_list = eigenrays(t1, t2);
            eigenray_model &total = _total(t1, t2);

            double time = 0.0;
            double source_de = 0.0;
            double source_az_x = 0.0;  // east/west component
            double source_az_y = 0.0;  // north/south component
            double target_de = 0.0;
            double target_az_x = 0.0;  // east/west component
            double target_az_y = 0.0;  // north/south component
            int surface = -1;
            int bottom = -1;
            int caustic = -1;
            double wgt = 0.0;
            double max_a = 0.0;

            // compute at each frequency

            for (size_t f = 0; f < _frequencies->size(); ++f) {
                // sum complex amplitudes over eigenrays

                std::complex<double> phasor(0.0, 0.0);
                for (const auto &ray : ray_list) {
                    // pressure amplitude

                    double a =
                        pow(10.0, ray->intensity(f) / -20.0);  // pressure
                    if (coherent) {
                        double p = TWO_PI * (*_frequencies)(f)*ray->time +
                                   ray->phase(f);
                        p = fmod(p, TWO_PI);  // large phases bad for cos,sin
                        std::complex<double> value(a * cos(p), a * sin(p));
                        phasor += value;
                    } else {
                        phasor += a;
                    }

                    // other eigenray terms

                    a *= a;  // scale by the pressure squared
                    wgt += a;
                    time += a * ray->time;
                    source_de += a * ray->source_de;
                    source_az_x += a * sin(to_radians(ray->source_az));
                    source_az_y += a * cos(to_radians(ray->source_az));
                    target_de += a * ray->target_de;
                    target_az_x += a * sin(to_radians(ray->target_az));
                    target_az_y += a * cos(to_radians(ray->target_az));
                    if (a > max_a) {
                        max_a = a;
                        surface = ray->surface;
                        bottom = ray->bottom;
                        caustic = ray->caustic;
                    }
                }  // end eigenray_list

                // convert back into intensity (dB) and phase (radians) values

                total.intensity(f) = -20.0 * log10(max(1e-15, abs(phasor)));
                total.phase(f) = arg(phasor);
            }  // end frequency

            // weighted average of other eigenray terms

            total.time = time / wgt;
            total.source_de = source_de / wgt;
            total.source_az =
                90.0 - to_degrees(atan2(source_az_y, source_az_x));
            total.target_de = target_de / wgt;
            total.target_az =
                90.0 - to_degrees(atan2(target_az_y, target_az_x));
            total.surface = surface;
            total.bottom = bottom;
            total.caustic = caustic;
        }  // end target size2
    }      // end target size1
}

/**
 * Write eigenray_collection data to to netCDF file.
 */
void eigenray_collection::write_netcdf(const char *filename,
                                       const char *long_name) const {
    // clang-format off
    auto *nc_file = new NcFile(filename, NcFile::Replace);
    if (long_name != nullptr) {
        nc_file->add_att("long_name", long_name);
    }
    nc_file->add_att("Conventions", "COARDS");

    // dimensions

    NcDim *freq_dim = nc_file->add_dim("frequency", (long)_frequencies->size());
    NcDim *row_dim = nc_file->add_dim("rows", (long)_target_pos->size1());
    NcDim *col_dim = nc_file->add_dim("cols", (long)_target_pos->size2());
    NcDim *eigenray_dim = nc_file->add_dim(
        "eigenrays", (long)(_num_eigenrays + _total.size1() * _total.size2()));

    // coordinates

    NcVar *src_id_var = nc_file->add_var("source_id", ncLong);
    NcVar *src_lat_var = nc_file->add_var("source_latitude", ncDouble);
    NcVar *src_lng_var = nc_file->add_var("source_longitude", ncDouble);
    NcVar *src_alt_var = nc_file->add_var("source_altitude", ncDouble);

    NcVar *target_var = nc_file->add_var("target_id", ncShort, row_dim, col_dim);
    NcVar *latitude_var = nc_file->add_var("latitude", ncDouble, row_dim, col_dim);
    NcVar *longitude_var = nc_file->add_var("longitude", ncDouble, row_dim, col_dim);
    NcVar *altitude_var = nc_file->add_var("altitude", ncDouble, row_dim, col_dim);
    NcVar *initial_time_var = nc_file->add_var("initial_time", ncDouble, row_dim, col_dim);

    NcVar *freq_var = nc_file->add_var("frequency", ncDouble, freq_dim);
    NcVar *proploss_index_var = nc_file->add_var("proploss_index", ncLong, row_dim, col_dim);
    NcVar *eigenray_index_var = nc_file->add_var("eigenray_index", ncLong, row_dim, col_dim);
    NcVar *eigenray_num_var = nc_file->add_var("eigenray_num", ncLong, row_dim, col_dim);

    NcVar *intensity_var = nc_file->add_var("intensity", ncDouble, eigenray_dim, freq_dim);
    NcVar *phase_var = nc_file->add_var("phase", ncDouble, eigenray_dim, freq_dim);
    NcVar *time_var = nc_file->add_var("travel_time", ncDouble, eigenray_dim);
    NcVar *source_de_var = nc_file->add_var("source_de", ncDouble, eigenray_dim);
    NcVar *source_az_var = nc_file->add_var("source_az", ncDouble, eigenray_dim);
    NcVar *target_de_var = nc_file->add_var("target_de", ncDouble, eigenray_dim);
    NcVar *target_az_var = nc_file->add_var("target_az", ncDouble, eigenray_dim);
    NcVar *surface_var = nc_file->add_var("surface", ncShort, eigenray_dim);
    NcVar *bottom_var = nc_file->add_var("bottom", ncShort, eigenray_dim);
    NcVar *caustic_var = nc_file->add_var("caustic", ncShort, eigenray_dim);
    NcVar *upper_var = nc_file->add_var("upper", ncShort, eigenray_dim);
    NcVar *lower_var = nc_file->add_var("lower", ncShort, eigenray_dim);

    // units

    src_id_var->add_att("units", "count");
    src_lat_var->add_att("units", "degrees_north");
    src_lng_var->add_att("units", "degrees_east");
    src_alt_var->add_att("units", "meters");
    src_alt_var->add_att("positive", "up");

    target_var->add_att("units", "count");
    latitude_var->add_att("units", "degrees_north");
    longitude_var->add_att("units", "degrees_east");
    altitude_var->add_att("units", "meters");
    altitude_var->add_att("positive", "up");
    initial_time_var->add_att("units", "seconds");

    freq_var->add_att("units", "hertz");
    proploss_index_var->add_att("units", "count");
    eigenray_index_var->add_att("units", "count");
    eigenray_num_var->add_att("units", "count");

    intensity_var->add_att("units", "dB");
    phase_var->add_att("units", "radians");
    time_var->add_att("units", "seconds");

    source_de_var->add_att("units", "degrees");
    source_de_var->add_att("positive", "up");
    source_az_var->add_att("units", "degrees_true");
    source_az_var->add_att("positive", "clockwise");

    target_de_var->add_att("units", "degrees");
    target_de_var->add_att("positive", "up");
    target_az_var->add_att("units", "degrees_true");
    target_az_var->add_att("positive", "clockwise");

    surface_var->add_att("units", "count");
    bottom_var->add_att("units", "count");
    caustic_var->add_att("units", "count");
    upper_var->add_att("units", "count");
    lower_var->add_att("units", "count");

    // write source parameters

    int n;
    double v;
    n = _source_id;					src_id_var->put(&n);
    v = _source_pos.latitude(); 	src_lat_var->put(&v);
    v = _source_pos.longitude();	src_lng_var->put(&v);
    v = _source_pos.altitude(); 	src_alt_var->put(&v);

    const long num_freqs = (long) _frequencies->size();
    freq_var->put((*_frequencies).data().begin(), num_freqs);

    // write target parameters

    target_var->put(_target_ids.data().begin(),
    		(long) _target_ids.size1(), (long) _target_ids.size2());
    const long rows = (long) _target_pos->size1();
    const long cols = (long) _target_pos->size2();
    latitude_var->put(_target_pos->latitude().data().begin(), rows, cols);
    longitude_var->put(_target_pos->longitude().data().begin(), rows, cols);
    altitude_var->put(_target_pos->altitude().data().begin(), rows, cols);
    initial_time_var->put(_initial_time.data().begin(), rows, cols);

    // write propagation loss and eigenrays to disk

    int record = 0;  // current record number
    for (long t1 = 0; t1 < (long)_target_pos->size1(); ++t1) {
        for (long t2 = 0; t2 < (long)_target_pos->size2(); ++t2) {
            int num = int(_eigenrays(t1, t2).size());
            proploss_index_var->set_cur(t1, t2);
            eigenray_index_var->set_cur(t1, t2);
            eigenray_num_var->set_cur(t1, t2);

            proploss_index_var->put(&record, 1, 1);  // 1st rec = summed PL
            int next_rec = record + 1;
            eigenray_index_var->put(&next_rec, 1,
                                    1);  // followed by list of rays
            eigenray_num_var->put(&num, 1, 1);

            auto iter = _eigenrays(t1, t2).begin();

            for (int n = -1; n < num; ++n) {

                // set record number for each eigenray data element

                intensity_var->set_cur(record);
                phase_var->set_cur(record);
                time_var->set_cur(record);
                source_de_var->set_cur(record);
                source_az_var->set_cur(record);
                target_de_var->set_cur(record);
                target_az_var->set_cur(record);
                surface_var->set_cur(record);
                bottom_var->set_cur(record);
                caustic_var->set_cur(record);
                upper_var->set_cur(record);
                lower_var->set_cur(record);
                ++record;

                // case 1 : write propagation loss summed over all eigenrays

                if (n < 0) {
                    const eigenray_model *ray = &_total(t1, t2);
                    intensity_var->put(ray->intensity.data().begin(), 1, num_freqs);
                    phase_var->put(ray->phase.data().begin(), 1, num_freqs);
                    time_var->put(&ray->time, 1);
                    source_de_var->put(&ray->source_de, 1);
                    source_az_var->put(&ray->source_az, 1);
                    target_de_var->put(&ray->target_de, 1);
                    target_az_var->put(&ray->target_az, 1);
                    surface_var->put(&ray->surface, 1);
                    bottom_var->put(&ray->bottom, 1);
                    caustic_var->put(&ray->caustic, 1);
                    upper_var->put(&ray->upper, 1);
                    lower_var->put(&ray->lower, 1);

                    // case 2 : write individual eigenray

                } else {
                    eigenray_model::csptr ray = *iter++;
                    intensity_var->put(ray->intensity.data().begin(), 1, num_freqs);
                    phase_var->put(ray->phase.data().begin(), 1, num_freqs);
                    time_var->put(&ray->time, 1);
                    source_de_var->put(&ray->source_de, 1);
                    source_az_var->put(&ray->source_az, 1);
                    target_de_var->put(&ray->target_de, 1);
                    target_az_var->put(&ray->target_az, 1);
                    surface_var->put(&ray->surface, 1);
                    bottom_var->put(&ray->bottom, 1);
                    caustic_var->put(&ray->caustic, 1);
                    upper_var->put(&ray->upper, 1);
                    lower_var->put(&ray->lower, 1);

                }  // if sum or individual
            }      // loop over # of eigenrays
        }          // loop over target# t2
    }              // loop over target# t1

    // close file

    delete nc_file;  // destructor frees all netCDF temp variables
                     // clang-format on
}

/**
 * Adjust eigenrays for small changes in source/target geometry.
 */
eigenray_list eigenray_collection::dead_reckon(
    size_t t1, size_t t2, const wposition1 &source_new,
    const wposition1 &target_new, const profile_model::csptr &profile) const {
    eigenray_list eigenrays =
        dead_reckon_one(_eigenrays(t1, t2), _source_pos, source_new, profile);
    return dead_reckon_one(eigenrays, wposition1(*_target_pos, t1, t2),
                           target_new, profile);
}

/**
 * Adjust eigenrays for small changes in the geometry of a single sensor.
 */
eigenray_list eigenray_collection::dead_reckon_one(
    const eigenray_list &eigenrays, const wposition1 &oldpos,
    const wposition1 &newpos, const profile_model::csptr &profile) {
    // compute position change in local tangent plane

    double dir[3] = {newpos.rho() - oldpos.rho(),
                     newpos.theta() - oldpos.theta(),
                     newpos.phi() - oldpos.phi()};
    dir[1] *= oldpos.rho();
    dir[2] *= oldpos.rho() * sin(oldpos.theta());

    // short cut if change very small

    if (dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2] < 1e-6) {
        return eigenrays;
    }

    // compute sound speed at original position

    matrix<double> c(1, 1);
    wposition position(1, 1);
    position.rho(0, 0, oldpos.rho());
    position.theta(0, 0, oldpos.theta());
    position.phi(0, 0, oldpos.phi());
    profile->sound_speed(position, &c);
    const double sound_speed = c(0, 0);

    // apply changes to each eigenray

    eigenray_list new_list;
    for (const auto &ray : eigenrays) {
        auto *new_ray = new eigenray_model(*ray);

        // compute ray direction in local tangent plane

        const double de = to_radians(ray->source_de);
        const double az = to_radians(ray->source_az);
        const double cos_de = cos(de);
        const double sin_de = sin(de);
        const double cos_az = cos(az);
        const double sin_az = sin(az);
        const double raydir[3] = {sin_de, -cos_de * cos_az, cos_de * sin_az};

        // change in range is proportional to the component of
        // slant range along the direction of the ray

        const double dr =
            (dir[0] * raydir[0] + dir[1] * raydir[1] + dir[2] * raydir[2]);
        new_ray->time = ray->time + dr / sound_speed;

        // compute change in intensity along ray path
        // approximating TL = 20*log10(r) + alpha * r + b

        matrix<double> distance(1, 1);
        matrix<vector<double> > atten(1, 1);
        atten(0, 0).resize(ray->frequencies->size());
        const double r1 = ray->time * sound_speed;
        const double r2 = r1 + dr;

        for (int f = 0; f < ray->frequencies->size(); ++f) {
            // remove old spreading and attenuation
            distance(0, 0) = r1;
            profile->attenuation(position, ray->frequencies, distance, &atten);
            const double offset =
                ray->intensity[f] - 20.0 * log10(r1) - atten(0, 0)[f];

            // add new spreading and attenuation
            distance(0, 0) = r2;
            profile->attenuation(position, ray->frequencies, distance, &atten);
            new_ray->intensity[f] = 20.0 * log10(r2) + atten(0, 0)[f] + offset;
        }
        new_list.push_back(eigenray_model::csptr(new_ray));
    }
    return new_list;
}
