/**
 * @file beampattern_utilities.cc
 * A collection of utilities for helping to build and configure beampatterns.
 */
#include <usml/beampatterns/beampattern_utilities.h>

namespace usml::beampatterns {

using namespace usml::types;

/**
 * Provides the element locations of a circular planar array on the
 * front/right plane
 */
void bp_con_ring(vector<double> radii, vector<int> num_elements,
        vector<double> offsets, matrix<double> *elem_locations) {
    (*elem_locations).resize(norm_1(num_elements), 3);

    int i = 0;
    for (int n = 0; n < radii.size(); n++) {
        seq_linear anglesv(0, 2 * M_PI / num_elements(n), num_elements(n));
        vector<double> angle = anglesv;
        angle = offsets(n) - angle;

        for (int j = 0; j < num_elements(n); j++) {
            (*elem_locations)(i, 0) = radii(n) * cos(angle(j));
            (*elem_locations)(i, 1) = radii(n) * sin(angle(j));
            (*elem_locations)(i, 2) = 0;
            i++;
        }
    }
}

/**
 * Provides the element locations of a uniformly spaced array in
 * 3 dimensions
 */
void bp_con_uniform(int num_e_front, double spacing_front,
        int num_e_right, double spacing_right, int num_e_up, double spacing_up,
        matrix<double> *elem_locations) {
    (*elem_locations).resize((size_t)num_e_front * num_e_right * num_e_up, 3);
    size_t i = 0;
    for (int u = 0; u < num_e_up; u++) {
        for (int e = 0; e < num_e_right; e++) {
            for (int n = 0; n < num_e_front; n++) {
                (*elem_locations)(i, 0) = ((double) n
                        - ((double) num_e_front - 1.0) / 2.0) * spacing_front;
                (*elem_locations)(i, 1) = ((double) e
                        - ((double) num_e_right - 1.0) / 2.0) * spacing_right;
                (*elem_locations)(i, 2) = ((double) u
                        - ((double) num_e_up - 1.0) / 2.0) * spacing_up;
                i++;
            }
        }
    }
}

}

