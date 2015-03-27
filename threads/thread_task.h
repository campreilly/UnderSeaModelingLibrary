/**
 * @file thread_task.h
 * Task that executes in the thread_pool.
 */
#pragma once

#include <usml/usml_config.h>
#include <usml/threads/smart_ptr.h>
#include <cstddef>

namespace usml {
namespace threads {

using std::size_t ;

/** Forward references. */
class thread_pool ;

/// @ingroup threads
/// @{

/**
 * Task that executes in the #thread_pool. The typical use is:
 *
 * - The developer defines a new class that inherits from thread_task.
 * - The sub-class overloads the run() method in the new class,
 *   and adds any methods needed to configure the task.
 * - The developer dynamically creates a pointer to a new instance of
 *   the sub-class using "new".
 * - The developer calls any configure methods needed on the new instance.
 * - The developer passes the thread_task pointer to a new instance to
 *   thread_pool::run().  A shared pointer to the task is returned.
 * - The shared pointer is used to invoke the abort() method if there is a need
 *   to pre-maturely abort this task.  The developers code monitors the
 *   _abort flag to detect when abort() has been invoked.
 *
 * Automatically assigns an identification number for each task when it is
 * created. Sub-classes are responsible for catching their own exceptions.
 * Exceptions that are not caught by the sub-class are ignored.
 * This prevents uncaught exceptions from crashing the thread_pool.
 */
class USML_DECLSPEC thread_task {

    friend class thread_pool ;

private:

    /** Next identification number to be assigned to a task. */
    static size_t _id_next ;

    /** Automatically assigned identification number for this task. */
    size_t _id ;

    /**
     * Safely initiates a task in the thread pool.
     * Traps uncaught exceptions to prevent thread_pool from crashing.
     */
    void start() ;

protected:

    /** Indication that task needs to abort. */
    bool _abort ;

public:

    /**
     * Default constructor, assigns a new id to this task.
     * Creates a sequential task ID number for each new task as it is created.
     * Task IDs wrap around back to zero at the max size of size_t.
     */
    thread_task() ;

    /**
     * Virtual destructor.
     */
    virtual ~thread_task() {}

    /**
     * Automatically assigned identification number for this task.
     * Sequential number assigned when the task is created.
     * Used mostly in debugging.  Should not be over-ridden by sub-classes.
     */
    size_t id() const {
        return _id ;
    }

    /**
     * Sub-classes overload this operator with the task to be executed.
     * Tasks should terminate, as soon as possible, when #_abort is true.
     * Sub-classes are responsible for catching their own exceptions.
     */
    virtual void run() = 0 ;

    /**
     * Indicate that task needs to abort itself.  Sets a protected member
     * variable called #_abort.  Tasks should terminate the execution of their
     * run() method, as soon as possible, when #_abort is true.
     */
    void abort() {
        _abort  = true ;
    }

};

/// @}
} // end of namespace threads
} // end of namespace usml

