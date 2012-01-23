/*
 * @file wvector.cc
 * World vector in spherical earth coordinates.
 */
#include <usml/types/wposition1.h>

using namespace usml::types;

/**
 * Constructs a new coordinate from matrix dimensions.
 */
wvector::wvector(unsigned rows, unsigned cols) :
_rho(rows, cols), _theta(rows, cols), _phi(rows, cols) {
}

//*********************************
// utilities

/**
 * Compute the dot product between this vector and some other
 * spherical earth vector.
 */
void wvector::dot(const wvector1& other, matrix<double>& result) const {
    noalias(result) = element_prod(_rho * other.rho(),
            1.0 - 2.0 * (abs2(sin(0.5 * (_theta - other.theta())))
            + element_prod(sin(_theta) * sin(other.theta()),
            abs2(sin(0.5 * (_phi - other.phi()))))));
}

/**
 * Dot product divided by the magnitude of the two vectors.
 */
void wvector::dotnorm(const wvector1& other, matrix<double>& result) const {
    // uses the same equation as dot() without the radial terms
    noalias(result) = 1.0 - 2.0 * (abs2(sin(0.5 * (_theta - other.theta())))
            + element_prod(sin(_theta) * sin(other.theta()),
            abs2(sin(0.5 * (_phi - other.phi())))));
}

/**
 * Compute the square of the straight line distance between this point and
 * some other point in space.
 */
void wvector::distance(const wvector1& origin, matrix<double>& result) const {
    distance2(origin, result);
    result = sqrt(result);
}

/**
 * Compute the square of the straight line distance between this point and
 * some other point in space.
 */
void wvector::distance2(const wvector1& origin, matrix<double>& result) const {
    dot(origin, result);
    result = abs(abs2(_rho) + origin.rho() * origin.rho() - 2.0 * result);
}
