/**
 * @file eigenray_collection.cc
 * List of targets and their associated propagation data.
 */

#include <usml/eigenrays/eigenray_collection.h>
#include <usml/types/wvector1.h>
#include <usml/ublas/math_traits.h>

#include <algorithm>
#include <boost/numeric/ublas/storage.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <cmath>
#include <complex>
#include <list>
#include <netcdf>

using namespace usml::eigenrays;

/**
 * Initialize the acoustic propagation effects associated
 * with each target.
 */
eigenray_collection::eigenray_collection(const seq_vector::csptr &frequencies,
                                         const wposition1 &source_pos,
                                         const wposition &target_pos,
                                         uint64_t sourceID,
                                         const matrix<uint64_t> &targetIDs,
                                         bool coherent)
    : _sourceID(sourceID),
      _targetIDs(target_pos.size1(), target_pos.size2()),
      _source_pos(source_pos),
      _target_pos(target_pos),
      _frequencies(frequencies),
      _eigenrays(target_pos.size1(), target_pos.size2()),
      _initial_time(target_pos.size1(), target_pos.size2()),
      _num_eigenrays(0),
      _total(target_pos.size1(), target_pos.size2()),
      _coherent(coherent) {
    for (size_t t1 = 0; t1 < size1(); ++t1) {
        for (size_t t2 = 0; t2 < size2(); ++t2) {
            eigenray_model loss;
            loss.intensity.resize(_frequencies->size());
            loss.intensity.clear();
            loss.phase.resize(_frequencies->size());
            loss.phase.clear();
            _total(t1, t2) = loss;
            _initial_time(t1, t2) = 0.0;
            if (targetIDs.size1() > 0 && targetIDs.size2() > 0) {
                _targetIDs(t1, t2) = targetIDs(t1, t2);
            } else {
                _targetIDs(t1, t2) = 0;
            }
        }
    }
}

/**
 * Find eigenrays for a single target in the grid.
 */
eigenray_list eigenray_collection::find_eigenrays(uint64_t targetID) const {
    for (size_t t1 = 0; t1 < size1(); ++t1) {
        for (size_t t2 = 0; t2 < size2(); ++t2) {
            if (_targetIDs(t1, t2) == targetID || targetID == 0) {
                return eigenrays(t1, t2);
            }
        }
    }
    return {};
}

/**
 * Find fastest eigenray for a single target in the grid.
 */
double eigenray_collection::find_initial_time(uint64_t targetID) const {
    for (size_t t1 = 0; t1 < size1(); ++t1) {
        for (size_t t2 = 0; t2 < size2(); ++t2) {
            if (_targetIDs(t1, t2) == targetID || targetID == 0) {
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
                                       eigenray_model::csptr ray,
                                       size_t /*runID*/) {
    _eigenrays(t1, t2).push_back(ray);
    auto old_initial = _initial_time(t1, t2);
    auto new_initial = ray->travel_time;
    if (old_initial <= 0.0 || old_initial > new_initial) {
        _initial_time(t1, t2) = new_initial;
    }
    ++_num_eigenrays;
}

/**
 * Compute propagation loss summed over all eigenrays.
 */
void eigenray_collection::sum_eigenrays() {
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
            int upper = -1;
            int lower = -1;
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
                    if (_coherent) {
                        double p =
                            TWO_PI * (*_frequencies)(f)*ray->travel_time +
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
                    time += a * ray->travel_time;
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
                        upper = ray->upper;
                        lower = ray->lower;
                    }
                }  // end eigenray_list

                // convert back into intensity (dB) and phase (radians) values

                total.intensity(f) = -20.0 * log10(max(1e-15, abs(phasor)));
                total.phase(f) = arg(phasor);
            }  // end frequency

            // weighted average of other eigenray terms

            total.travel_time = time / wgt;
            total.source_de = source_de / wgt;
            total.source_az =
                90.0 - to_degrees(atan2(source_az_y, source_az_x));
            total.target_de = target_de / wgt;
            total.target_az =
                90.0 - to_degrees(atan2(target_az_y, target_az_x));
            total.surface = surface;
            total.bottom = bottom;
            total.caustic = caustic;
            total.upper = upper;
            total.lower = lower;
        }  // end target size2
    }      // end target size1
}

/**
 * Write eigenray_collection data to to netCDF file.
 */
void eigenray_collection::write_netcdf(const char *filename,
                                       const char *long_name) const {
    netCDF::NcFile nc_file(filename, netCDF::NcFile::replace);
    if (long_name != nullptr) {
        nc_file.putAtt("long_name", long_name);
    }
    nc_file.putAtt("Conventions", "COARDS");

    // dimensions

    // clang-format off
    netCDF::NcDim row_dim = nc_file.addDim("rows", _target_pos.size1());
    netCDF::NcDim col_dim = nc_file.addDim("cols", _target_pos.size2());
    netCDF::NcDim eigenray_dim = nc_file.addDim( "eigenrays", _num_eigenrays + _total.size1() * _total.size2());
    netCDF::NcDim freq_dim = nc_file.addDim("frequencies", _frequencies->size());
    const std::vector< netCDF::NcDim > row_col_dims{ row_dim, col_dim };
    const std::vector< netCDF::NcDim > ray_freq_dims{ eigenray_dim, freq_dim };

    // coordinates

    netCDF::NcVar src_id_var = nc_file.addVar("sourceID", netCDF::NcUint64());
    netCDF::NcVar src_lat_var = nc_file.addVar("source_latitude", netCDF::NcDouble());
    netCDF::NcVar src_lng_var = nc_file.addVar("source_longitude", netCDF::NcDouble());
    netCDF::NcVar src_alt_var = nc_file.addVar("source_altitude", netCDF::NcDouble());

    netCDF::NcVar target_var = nc_file.addVar("targetID", netCDF::NcUint64(), row_col_dims);
    netCDF::NcVar latitude_var = nc_file.addVar("latitude", netCDF::NcDouble(), row_col_dims);
    netCDF::NcVar longitude_var = nc_file.addVar("longitude", netCDF::NcDouble(), row_col_dims);
    netCDF::NcVar altitude_var = nc_file.addVar("altitude", netCDF::NcDouble(), row_col_dims);
    netCDF::NcVar initial_time_var = nc_file.addVar("initial_time", netCDF::NcDouble(), row_col_dims);

    netCDF::NcVar freq_var = nc_file.addVar("frequencies", netCDF::NcDouble(), freq_dim);
    netCDF::NcVar proploss_index_var = nc_file.addVar("proploss_index", netCDF::NcInt(), row_col_dims);
    netCDF::NcVar eigenray_index_var = nc_file.addVar("eigenray_index", netCDF::NcInt(), row_col_dims);
    netCDF::NcVar eigenray_num_var = nc_file.addVar("eigenray_num", netCDF::NcInt(), row_col_dims);

    netCDF::NcVar intensity_var = nc_file.addVar("intensity", netCDF::NcDouble(), ray_freq_dims);
    netCDF::NcVar phase_var = nc_file.addVar("phase", netCDF::NcDouble(), ray_freq_dims);
    netCDF::NcVar time_var = nc_file.addVar("travel_time", netCDF::NcDouble(), eigenray_dim);
    netCDF::NcVar source_de_var = nc_file.addVar("source_de", netCDF::NcDouble(), eigenray_dim);
    netCDF::NcVar source_az_var = nc_file.addVar("source_az", netCDF::NcDouble(), eigenray_dim);
    netCDF::NcVar target_de_var = nc_file.addVar("target_de", netCDF::NcDouble(), eigenray_dim);
    netCDF::NcVar target_az_var = nc_file.addVar("target_az", netCDF::NcDouble(), eigenray_dim);
    netCDF::NcVar surface_var = nc_file.addVar("surface", netCDF::NcShort(), eigenray_dim);
    netCDF::NcVar bottom_var = nc_file.addVar("bottom", netCDF::NcShort(), eigenray_dim);
    netCDF::NcVar caustic_var = nc_file.addVar("caustic", netCDF::NcShort(), eigenray_dim);
    netCDF::NcVar upper_var = nc_file.addVar("upper", netCDF::NcShort(), eigenray_dim);
    netCDF::NcVar lower_var = nc_file.addVar("lower", netCDF::NcShort(), eigenray_dim);
    // clang-format on

    // units

    src_lat_var.putAtt("units", "degrees_north");
    src_lng_var.putAtt("units", "degrees_east");
    src_alt_var.putAtt("units", "meters");
    src_alt_var.putAtt("positive", "up");

    latitude_var.putAtt("units", "degrees_north");
    longitude_var.putAtt("units", "degrees_east");
    altitude_var.putAtt("units", "meters");
    altitude_var.putAtt("positive", "up");
    initial_time_var.putAtt("units", "seconds");

    freq_var.putAtt("units", "hertz");
    proploss_index_var.putAtt("units", "count");
    eigenray_index_var.putAtt("units", "count");
    eigenray_num_var.putAtt("units", "count");

    intensity_var.putAtt("units", "dB");
    phase_var.putAtt("units", "radians");
    time_var.putAtt("units", "seconds");

    source_de_var.putAtt("units", "degrees");
    source_de_var.putAtt("positive", "up");
    source_az_var.putAtt("units", "degrees_true");
    source_az_var.putAtt("positive", "clockwise");

    target_de_var.putAtt("units", "degrees");
    target_de_var.putAtt("positive", "up");
    target_az_var.putAtt("units", "degrees_true");
    target_az_var.putAtt("positive", "clockwise");

    surface_var.putAtt("units", "count");
    bottom_var.putAtt("units", "count");
    caustic_var.putAtt("units", "count");
    upper_var.putAtt("units", "count");
    lower_var.putAtt("units", "count");

    // write source parameters

    size_t n;
    double v;
    n = _sourceID;
    src_id_var.putVar(&n);
    v = _source_pos.latitude();
    src_lat_var.putVar(&v);
    v = _source_pos.longitude();
    src_lng_var.putVar(&v);
    v = _source_pos.altitude();
    src_alt_var.putVar(&v);
    freq_var.putVar((*_frequencies).data().begin());

    // write target parameters

    target_var.putVar(_targetIDs.data().begin());
    latitude_var.putVar(_target_pos.latitude().data().begin());
    longitude_var.putVar(_target_pos.longitude().data().begin());
    altitude_var.putVar(_target_pos.altitude().data().begin());
    initial_time_var.putVar(_initial_time.data().begin());

    // write propagation loss and eigenrays to disk

    std::vector<size_t> row_col_index(2);
    std::vector<size_t> ray_index(1);
    std::vector<size_t> ray_freq_index(2);
    std::vector<size_t> ray_freq_count(2);
    ray_freq_count[0] = 1;
    ray_freq_count[1] = _frequencies->size();

    size_t record = 0;  // current record number
    for (size_t t1 = 0; t1 < _target_pos.size1(); ++t1) {
        row_col_index[0] = t1;
        for (size_t t2 = 0; t2 < _target_pos.size2(); ++t2) {
            row_col_index[1] = t2;
            size_t num = _eigenrays(t1, t2).size();
            size_t next_rec = record + 1;

            proploss_index_var.putVar(row_col_index, &record);
            eigenray_index_var.putVar(row_col_index, &next_rec);
            eigenray_num_var.putVar(row_col_index, &num);

            auto iter = _eigenrays(t1, t2).begin();
            for (int n = -1; n < (int)num; ++n) {
                ray_index[0] = record++;
                ray_freq_index[0] = ray_index[0];
                ray_freq_index[1] = 0;
                const eigenray_model *ray;
                if (n < 0) {  // summed over all eigenrays
                    ray = &_total(t1, t2);
                } else {  // individual eigenray
                    ray = (*iter++).get();
                }
                intensity_var.putVar(ray_freq_index, ray_freq_count,
                                     ray->intensity.data().begin());
                phase_var.putVar(ray_freq_index, ray_freq_count,
                                 ray->phase.data().begin());
                time_var.putVar(ray_index, &ray->travel_time);
                source_de_var.putVar(ray_index, &ray->source_de);
                source_az_var.putVar(ray_index, &ray->source_az);
                target_de_var.putVar(ray_index, &ray->target_de);
                target_az_var.putVar(ray_index, &ray->target_az);
                surface_var.putVar(ray_index, &ray->surface);
                bottom_var.putVar(ray_index, &ray->bottom);
                caustic_var.putVar(ray_index, &ray->caustic);
                upper_var.putVar(ray_index, &ray->upper);
                lower_var.putVar(ray_index, &ray->lower);

            }  // loop over # of eigenrays
        }      // loop over target# t2
    }          // loop over target# t1
}

/**
 * Adjust eigenrays for small changes in source/target geometry.
 */
eigenray_list eigenray_collection::dead_reckon(
    size_t t1, size_t t2, const wposition1 &source_new,
    const wposition1 &target_new, const profile_model::csptr &profile) const {
    eigenray_list eigenrays =
        dead_reckon_one(_eigenrays(t1, t2), _source_pos, source_new, profile);
    return dead_reckon_one(eigenrays, wposition1(_target_pos, t1, t2),
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
        new_ray->travel_time = ray->travel_time + dr / sound_speed;

        // compute change in intensity along ray path
        // approximating TL = 20*log10(r) + alpha * r + b

        matrix<double> distance(1, 1);
        matrix<vector<double> > atten(1, 1);
        atten(0, 0).resize(ray->frequencies->size());
        const double r1 = ray->travel_time * sound_speed;
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
