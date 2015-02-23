/**
 *  @file singleton_map.h
 *  Definition of the Class singleton_map
 *  Created on: 10-Feb-2015 12:49:08 PM
 */

#pragma once

#include <map>
#include <typeinfo>

#include <usml/usml_config.h>

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/** 
 * Storage for all the usml/sensors namespace data payloads in use by the USML.
 * It is defined as a GOF singleton pattern, that has a std::map which implements
 * the insert and find methods. std::map is a sorted associative container that
 * contains key-value pairs with unique keys.
 * This class takes ownership of payload_types that are pointers and deletes
 * them in the destructor.
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
 * @updated 20-Feb-2015 4:31:15 PM
 */

template<class K, class P>
class USML_DLLEXPORT singleton_map
{
public:
    typedef K                   key_type ;
    typedef P                   payload_type ;
    typedef singleton_map<K,P>  self_type;
    typedef singleton_map<K,P>* self_type_ptr ;

    /**
    * Singleton Constructor - Creates singleton_map instance just once, then
    * Accessible everywhere.
    * @return  pointer to the instance of the singleton_map<key_type, payload_type>
    */
    // Static instance method
    static self_type_ptr instance()
    {
        return self_type::_instance ? self_type::_instance : (self_type::_instance = new singleton_map);
    }

    /**
     * Destructor - Deletes payload_type's that are data pointers
     *              and clears the map
     */
    virtual ~singleton_map()
    {
        // Check if payload is a pointer
        const std::type_info& check  = typeid(_map.begin()->second);
        const char * type_name = check.name();
        // First char will be a 'P'
        if (type_name[0] == 'P') {

            typename std::map <key_type, payload_type>::iterator iter;
            for (iter = _map.begin(); iter != _map.end(); ++iter) {

                delete iter->second;
            }
        }

        // Always clear
        _map.clear();
    }

    /**
    * finds the payload_type associated with the keyID.
    * @param keyID is the associated key.
    * @return payload_type as defined by the P template parameter.
    */
    payload_type find (const key_type keyID) const
    {
        if (_map.count(keyID) == 0) {
            return (NULL);
        }
        return _map.find(keyID)->second;
    }

    /**
    * Inserts the supplied payload_type into the map with the key provided.
    * @param keyID is the associated key to the payload_type.
    * @param payload to be inserted.
    */
    void insert(key_type keyID, payload_type payload)
    {
        _map.insert(std::pair<key_type, payload_type >(keyID, payload) );
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
    std::map <key_type, payload_type> _map;
};

/**
 * Initialization of private static member _instance
 */
template <class K,class P> singleton_map<K,P>* singleton_map<K,P>::_instance=NULL;

/// @}
} // end of namespace sensors
} // end of namespace usml
