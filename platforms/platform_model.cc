/**
 * @file platform_model.cc
 * Physical object that moves through the simulation.
 */

#include <usml/platforms/platform_model.h>
#include <usml/types/wvector1.h>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <cmath>
#include <memory>

using namespace usml::platforms;
using namespace usml::threads;

/**
 * Host velocity in world coordinates.
 */
vector<double> platform_model::host_velocity() const {
    const platform_model* platform = this;
    while (platform->_host != nullptr) {
        platform = platform->_host;
    }
    return platform->_orient.front() * platform->_speed;
}

/**
 * Updates the position and orientation of platform and its children.
 * Just locks object before calling private update_platform() function.
 */
//NOLINTNEXTLINE(misc-no-recursion)
void platform_model::update(time_t time, const wposition1& pos,
                            const orientation& orient, double speed,
                            update_type_enum update_type) {
    write_lock_guard guard(_mutex);
    return update_internals(time, pos, orient, speed, update_type);
}

/**
 * Use the current speed to update the position of the platform.
 */
void platform_model::update(time_t time, update_type_enum update_type) {
    write_lock_guard guard(_mutex);

    // compute relative offset from heading, speed, and elasped time

    const auto dt = double(time - _time);
    bvector offset(_orient.front() * _speed * dt);

    // convert relative offset to world coordinates

    const double rho = _position.rho();
    const double theta = _position.theta();
    const double r_sin_theta = rho * sin(theta);
    const double phi = _position.phi();

    wposition1 pos;
    pos.rho(rho + offset.up());
    pos.theta(theta - offset.front() / rho);
    pos.phi(phi + offset.right() / r_sin_theta);

    update_internals(time, pos, _orient, _speed, update_type);
}

/**
 * Adds a new child to this host at specific position and orientation.
 */
platform_model::key_type platform_model::add_child(
    const platform_model::sptr& child, const bvector& pos,
    const orientation& orient) {
    write_lock_guard guard(_mutex);
    _linkage_manager.add(std::make_shared<linkage>(this, child, pos, orient));
    update_internals(_time, _position, _orient, _speed, NO_UPDATE);
    return child->keyID();
}

/**
 * Removes an existing child from the host.
 */
bool platform_model::remove_child(platform_model::key_type keyID) {
    write_lock_guard guard(_mutex);
    return _linkage_manager.remove(keyID);
}

/**
 * Creates a temporary list of children attached to this platform.
 */
std::list<platform_model::sptr> platform_model::children() {
    read_lock_guard guard(_mutex);
    std::list<platform_model::sptr> list;
    std::list<linkage::sptr> attached_list = _linkage_manager.list();
    for (const auto& link : attached_list) {
        list.push_back(link->child);
    }
    return list;
}

/**
 * Updates the internal state of this platform and its children.
 */
//NOLINTNEXTLINE(misc-no-recursion)
void platform_model::update_internals(time_t time, const wposition1& pos,
                                      const orientation& orient, double speed,
                                      update_type_enum update_type) {
    // update motion of host

    _time = time;
    _position = pos;
    _orient = orient;
    _speed = speed;

    // update motion of children

    std::list<linkage::sptr> attached_list = _linkage_manager.list();
    const double rho = _position.rho();
    const double theta = _position.theta();
    const double r_sin_theta = rho * sin(theta);
    const double phi = _position.phi();
    bvector offset;
    wposition1 posit;
    orientation ori;
    for (const auto& link : attached_list) {
        offset.rotate(_orient, link->position);
        posit.rho(rho + offset.up());
        posit.theta(theta - offset.front() / rho);
        posit.phi(phi + offset.right() / r_sin_theta);
        ori.rotate(_orient, link->orient);
        link->child->update(time, posit, ori, speed, update_type);
    }
}
