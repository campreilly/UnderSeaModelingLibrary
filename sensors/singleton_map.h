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
 * Storage for all the usml/sensors namespace mapped_type data in use by the USML.
 * Defined as a GOF singleton pattern, that has a std::map which implements the
 * insert and find methods. std::map is a sorted associative container that
 * contains key-value pairs with unique keys. This class takes ownership of
 * payload_types that are pointers and deletes them in the destructor.

 * Design Rational
 * The singleton_map was designed to contain a std::map vs inheriting
 * from a std::map. Reasons are as follows: 
 *   General OOD principal to constrain the public API, std::map is wide open. 
 *   Limited API provides for easily extendible child classes. 
 *   Unwrapping std:map calls will all be internal, allowing for cleaner singleton_map calls.
 *
 * @author Ted Burns, AEgis Technologies Inc.
 * @version 1.0
 * @updated 27-Feb-2015 3:15:07 PM
 */

template<class K, class T>
class USML_DLLEXPORT singleton_map
{
public:
    typedef K key_type;
    typedef T mapped_type;
    typedef singleton_map<K,T> self_type;
    typedef singleton_map<K,T>* self_type_ptr;

    /**
    * Singleton Constructor - Creates singleton_map instance just once, then
    * Accessible everywhere.
    * @return  pointer to the instance of the singleton_map<key_type, mapped_type>
    */
    // Static instance method
    static self_type_ptr instance()
    {
        return self_type::_instance ? self_type::_instance : (self_type::_instance = new singleton_map);
    }

    /**
     * Destructor - Deletes the mapped_type's that
     *              are data pointers and clears the map
     */
    virtual ~singleton_map()
    {
        // Check if mapped_type is a pointer
        const std::type_info& check  = typeid(_map.begin()->second);
        const char * type_name = check.name();
        // First char will be a 'P'
        if (type_name[0] == 'P') {

            typename std::map <key_type, mapped_type>::iterator iter;
            for (iter = _map.begin(); iter != _map.end(); ++iter) {

                delete iter->second;
            }
        }

        // Always clear
        _map.clear();
    }

    /**
    * Finds the mapped_type associated with the keyID.
    * @param keyID is the associated key.
    * @return mapped_type as defined by the T template parameter.
    */
    mapped_type find(const key_type keyID) const
    {
        if (_map.count(keyID) == 0) {
            return (NULL);
        }
        return _map.find(keyID)->second;
    }

    /**
    * Inserts the supplied mapped_type into the map with the key provided.
    * @param keyID is the associated key to the mapped_type.
    * @param mapped is mapped_type to be inserted.
    * @return false if keyID was already in the map.
    */
    bool insert(const key_type keyID, mapped_type mapped)
    {
        bool result = false;  // EVAR needs to return correct
                              // when keyID pre-exist
        // Check for Pre-existance
        if (this->find(keyID) == 0) {
            _map.insert(std::pair<key_type, mapped_type>(keyID, mapped));
            result = true;
        }
        return result;
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
     * The std::map that stores the mapped_types by key_type
     */
    std::map <key_type, mapped_type> _map;
};

/**
 * Initialization of private static member _instance
 */
template <class K,class T> singleton_map<K,T>* singleton_map<K,T>::_instance=NULL;

/// @}
} // end of namespace sensors
} // end of namespace usml
