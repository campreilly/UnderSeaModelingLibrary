/**
 * @file platform_model.h
 * Physical object that moves through the simulation.
 */
#pragma once

#include <bits/types/time_t.h>
#include <usml/managed/managed_obj.h>
#include <usml/managed/manager_template.h>
#include <usml/managed/update_notifier.h>
#include <usml/threads/read_write_lock.h>
#include <usml/types/bvector.h>
#include <usml/types/orientation.h>
#include <usml/types/wposition1.h>

#include <boost/numeric/ublas/vector.hpp>
#include <list>
#include <string>

namespace usml {
namespace platforms {

using namespace usml::types;
using namespace usml::managed;

/// @ingroup platforms
/// @{

/**
 * Physical object that moves through the simulation. Child platforms are
 * attached to to their host using a private map that tracks their relative
 * position and orientation. Child positions and orientations are updated in
 * world coordinates each time the host position or orientation changes.
 * For this reason, child instances can never be shared between hosts.
 *
 * Although this base class implements the update_notifier interface, it does
 * not notify listeners when position or orientation changes. It saves update
 * notifications for "major" changes like acoustic updates in the sub-classes.
 */
class platform_model : public managed_obj<int, platform_model>,
                       public update_notifier<platform_model> {
   public:
    /// Thresholds update type.
    enum update_type_enum {
        TEST_THRESHOLD = 0,  ///< Check thresholds before updating.
        FORCE_UPDATE = 1,    ///< Forces update without checking thresholds.
        NO_UPDATE = 2,       ///< Prevents automatic update
    };

    /**
     * Initialize location and orientation of the platform in world coordinates.
     *
     * @param keyID        	Identification used to find this platform
     * 						instance in platform_manager.
     * @param description   Human readable name for this platform instance.
     * @param time			Time at which platform is being created.
     * @param pos 			Initial location for this platform.
     * @param orient 		Initial orientation for this platform.
     * @param speed			Platform speed (m/s).
     */
    platform_model(platform_model::key_type keyID,
                   const std::string& description, time_t time = 0.0,
                   const wposition1& pos = wposition1(),
                   const orientation& orient = orientation(),
                   double speed = 0.0)
        : managed_obj(keyID, description),
          _time(time),
          _position(pos),
          _orient(orient),
          _speed(speed) {}

    /// Prevent access to copy constructor
    platform_model(const platform_model&) = delete;

    /// Virtual destructor
    virtual ~platform_model() {}

    /// Prevent access to assignment operator
    platform_model& operator=(const platform_model&) = delete;

    /// Mutex to that locks object during changes.
    read_write_lock& mutex() const { return _mutex; }

    /// Platform that controls the motion of this platform.
    const platform_model* host() const {
        return _host;
    }

    /// Time of last update.
    time_t time() const {
        return _time;
    }

    /// Location of the platform in world coordinates.
    wposition1 position() const {
        return _position;
    }

    /// Orientation of the platform in world coordinates.
    orientation orient() const {
        return _orient;
    }

    /// Platform speed in world coordinates (m/s).
    double speed() const {
        return _speed;
    }

    /**
     * Get all of the motion parameters, locked by a common mutex.
     *
     * @param time 		Time of last update.
     * @param position 	Location of the platform in world coordinates.
     * @param orient 	Orientation of the platform in world coords.
     * @param speed		Platform speed (m/s).
     */
    void get_motion(time_t* time = nullptr, wposition1* position = nullptr,
                    orientation* orient = nullptr,
                    double* speed = nullptr) const;

    /**
     * Updates the position and orientation of platform and its children.
     *
     * @param time 			Time at which platform was updated.
     * @param pos 			New location for this platform.
     * @param orient 		New orientation for this platform.
     * @param speed			Platform speed (m/s).
     * @param update_type	Controls testing of thresholds.
     */
    void update(time_t time, const wposition1& pos, const orientation& orient,
                double speed, update_type_enum update_type = TEST_THRESHOLD);

    /**
     * Use the current speed to update the position of the platform.
     *
     * @param time          Time at which platform was updated.
     * @param update_type   Controls testing of thresholds.
     */
    void update(time_t time, update_type_enum update_type = TEST_THRESHOLD);

    /**
     * Attaches a new child platform to this host. Throws an exception if
     * child's keyID is already in the list of attached platforms.
     *
     * @param child     	Reference to the child to be linked to the host.
     * @param pos			Position of child relative to host.
     * @param orient		Orientation of child relative to host.
     * @return              Key used to store this object.
     * @throw duplicate_key If keyID already exists.
     */
    platform_model::key_type add_child(
        const platform_model::sptr& child, const bvector& pos = bvector(),
        const orientation& orient = orientation());

    /**
     * Removes an existing child from the host. Leaves the host unchanged if
     * keyID is not in the linkage map.
     *
     * @param keyID		Identification used to find this child.
     * @return          False if keyID was not found.
     */
    bool remove_child(platform_model::key_type keyID);

    /**
     * Creates a temporary list of children attached to this platform.
     */
    std::list<platform_model::sptr> children();

   protected:
    /**
     * Updates the internal state of this platform and its children. Can be
     * overloaded by derived classes who wish to perform additional calculations
     * as a side effect of motion.
     *
     * @param time          Time at which platform was updated.
     * @param pos           New location for this platform.
     * @param orient        New orientation for this platform.
     * @param speed         Platform speed (m/s).
     * @param update_type	Controls testing of thresholds.
     */
    virtual void update_internals(
        time_t time, const wposition1& pos,
        const orientation& orient = orientation(), double speed = 0.0,
        update_type_enum update_type = TEST_THRESHOLD);

   private:
    /**
     * Private class to manage location and orientation of child relative to
     * host.
     */
    class linkage : public managed_obj<int, linkage> {
       public:
        /**
         * Construct linkage between host and child. Assumes that the child has
         * already been assigned a keyID and description, and these values can
         * be reused for this map.
         *
         * @param host		Reference to the platform that owns this child.
         * @param child		Reference to the child to be linked to the host.
         * @param pos		Position of child relative to host.
         * @param orient	Orientation of child relative to host.
         */
        linkage(const platform_model* host, platform_model::sptr child,
                const bvector& pos, const orientation& orient)
            : managed_obj(child->keyID(), child->description()),
              child(child),
              position(pos),
              orient(orient) {
            child->_host = host;
        }

        /// Reference to the child platform.
        platform_model::sptr child;

        /// Relative location of child relative to platform.
        const bvector position;

        /// Relative orientation of child relative to platform.
        const orientation orient;
    };

    /// Mutex to that locks object during changes.
    mutable read_write_lock _mutex;

    /// Container for location and orientation of platforms on platform.
    manager_template<linkage> _child_manager;

    /// Platform that controls the motion of this platform.
    const platform_model* _host{nullptr};

    /// Time of last update.
    time_t _time{0};

    /// Location of the platform in world coordinates.
    wposition1 _position;

    /// Orientation of the platform in world coordinates.
    orientation _orient;

    /// Platform speed (m/s).
    double _speed{0.0};
};

/// @}
}  // namespace platforms
}  // namespace usml
