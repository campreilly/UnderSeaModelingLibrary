/**
 *  @file singleton_map.h
 *  Definition of the Class singleton_map
 *  Created on: 10-Feb-2015 12:49:08 PM
 */

#pragma once

#include <map>

#include <usml/usml_config.h>

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/** 
 * Storage for all the usml/sensors namespace data payloads in use by the USML.
 * It is defined as a GOF singleton pattern, that has a std::map which implements
 * the insert and find methods. std::map is a sorted associative container that
 * contains key-value pairs with unique keys. This class takes ownership of all
 * heap payload pointers inserted, and deletes them in the destructor.
 *
 * Design Rational
 *  The singleton_map was designed to contain a std::map vs inheriting from a std::map.
 *  Reasons are as follows:
 *      General OOD principal to constrain the public API, std::map is wide open.
 *      Limited API provides for easily extendible child classes.
 *      Unwrapping std:map calls will all be internal, allowing for cleaner singleton_map calls.
 *
 * @author Ted Burns, AEgis Technologies Inc.
 * @version 1.0
 * @created 10-Feb-2015 12:49:08 PM
 */

template<class K, class P>
class USML_DLLEXPORT singleton_map
{

public:
    typedef K                   key_type ;
    typedef P*                  payload_ptr ;
    typedef singleton_map<K,P>  self_type;
    typedef singleton_map<K,P>* self_type_ptr ;

    /**
    * Singleton Constructor - Creates singleton_map instance just once, then
    * Accessible everywhere.
    * @return  pointer to the instance of the singleton_map.
    */
    // Static instance method
    static self_type_ptr instance()
    {
        return self_type::_instance ? self_type::_instance : (self_type::_instance = new singleton_map);

//        if( self_type::_instance == NULL )
//            self_type::_instance = new singleton_map();
//        return self_type::_instance;
    }

    /**
     * Destructor - Deletes pointers to payload's heap data.
     *   Only accessible by by subclasses
     */
    virtual ~singleton_map()
    {
       typename std::map <const key_type, const payload_ptr>::iterator iter;

       for (iter = _map.begin(); iter != _map.end(); ++iter) {
           delete iter->second;
       }
       _map.clear();
    }


    /**
    * finds the payload associated with the keyID.
    * @param keyID is the associated key.
    * @return const payload pointer.
    */
    const payload_ptr find(const key_type keyID)
    {
        if (_map.count(keyID) == 0) {
            return (NULL);
        }
        return _map.find(keyID)->second;
    }

    /**
    * Inserts the supplied payload into the map with the key provided.
    * @param keyID is the associated key to the payload.
    * @param payload to be inserted.
    */
    void insert(const key_type keyID, const payload_ptr payload)
    {
        _map.insert(std::pair<const key_type, const payload_ptr >(keyID, payload) );
    }


protected:
    /**
     * Default Constructor
     *   Protected to only allow subclasses access.
     */
    singleton_map(){}


private:
    /**
     * Prevent access to copy constructor
     */
    singleton_map(singleton_map const&);

    /**
     * Prevent access to assignment operator
     */
    singleton_map& operator=(singleton_map const&);

    /**
     * The singleton access pointer.
     */
    static self_type_ptr _instance;

    /**
     * The std::map that stores the payload's by key_type
     */
    std::map <const key_type, const payload_ptr> _map;
};

/**
 * Initialization of private static member _instance
 */
template <class K,class P> singleton_map<K,P>* singleton_map<K,P>::_instance=NULL;

/// @}
} // end of namespace sensors
} // end of namespace usml
