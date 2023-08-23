/**
 * @example managed/test/manager_test.cc
 */
#include <usml/managed/managed.h>

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <memory>

using namespace boost::unit_test;
using namespace usml::managed;

using std::cout;
using std::endl;

BOOST_AUTO_TEST_SUITE(manager_test)

/**
 * Type of object to be managed.
 */
class test_object : public managed_obj<int, const test_object> {
   public:
    test_object(key_type keyID, const std::string& description)
        : managed_obj(keyID, description) {}
    virtual ~test_object() { cout << "~test_object: #" << keyID() << endl; }
};

/**
 * Displays message when manager changes.
 */
class test_manager_listener : public manager_listener<test_object> {
   public:
    void notify_add(const test_object* object) const override {
        cout << "add: " << object->keyID() << " " << object->description()
             << endl;
    }
    void notify_remove(typename test_object::key_type keyID) const override {
        cout << "remove: " << keyID << endl;
    }
};

/**
 * Displays message when object changes.
 */
class test_object_listener : public update_listener<test_object> {
   public:
    void notify_update(const test_object* object) override {
        cout << "update: " << object->keyID() << " " << object->description()
             << endl;
    }
};

/**
 * @ingroup bistatic_test
 * @{
 */

/**
 * Test the ability to add and remove objects from manager_template.
 * If this test executes correctly, it should print notices for adding,
 * updating, and removing the object.  Memory leak tests should show that
 * no objects remain in memory when the test is complete.
 */
BOOST_AUTO_TEST_CASE(add_remove) {
    cout << "=== manager_test: add_remove ===" << endl;

    // initialize manager

    manager_template<test_object> manager;
    test_manager_listener mgr_listener;
    manager.add_listener(&mgr_listener);

    // initialize object

    test_object::sptr object(new test_object(999, "object"));
    test_object_listener obj_listener;

    update_notifier<test_object> test_notifier;
    test_notifier.add_listener(&obj_listener);

    // test change notifications

    manager.add(object);
    test_notifier.notify_update(object.get());

    // test lookups

    test_object::sptr lookup = manager.find(999);
    BOOST_CHECK_EQUAL(lookup->keyID(), 999);
    lookup = manager.find(777);
    BOOST_CHECK(lookup == nullptr);

    std::list<test_object::sptr> list = manager.list();
    BOOST_CHECK_EQUAL(list.size(), 1);
    BOOST_CHECK_EQUAL(list.begin()->get()->keyID(), 999);

    manager.remove(object->keyID());

    // remove all listeners (optional)

    test_notifier.remove_listener(&obj_listener);
    manager.remove_listener(&mgr_listener);
}

/// @}
BOOST_AUTO_TEST_SUITE_END()
