/**
 *  @file map_template.h
 *  Definition of the Class map_template
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
 * Container for all the usml/sensors namespace mapped_type data in use by the USML.
 * Defined as a template, that has a std::map which implements the erase,
 * find, insert and update methods. std::map is a sorted associative container that
 * contains key-value pairs with unique keys. This class takes ownership of
 * mapped_type that are pointers and deletes them in the destructor.

 * Design Rational
 * The map_template was designed to contain a std::map vs inheriting
 * from a std::map. Reasons are as follows: 
 *   General OOD principal to constrain the public API, std::map is wide open. 
 *   Limited API provides for easily extendible child classes. 
 *   Unwrapping std:map calls will all be internal, allowing for cleaner map_template calls.
 *
 * @author Ted Burns, AEgis Technologies Inc.
 * @version 1.0
 * @updated 27-Feb-2015 3:15:07 PM
 */

template<class K, class T>
class USML_DLLEXPORT map_template
{

public:

    typedef K key_type;
    typedef T mapped_type;
    typedef map_template<K,T> self_type;
    typedef map_template<K,T>* self_type_ptr;

    /**
     * Default Constructor
     */
    map_template(){}

    /**
     * Destructor - Deletes the mapped_type's that
     *              are data pointers and clears the map
     */
    virtual ~map_template()
    {
        // Check if mapped_type is a pointer
        const std::type_info& check  = typeid(_map.begin()->second);
        const char * type_name = check.name();
        // First char will be a 'P'
        if (type_name[0] == 'P') {

            typename std::map <key_type, mapped_type>::iterator iter;
            for (iter = _map.begin(); iter != _map.end(); ++iter)
            {
                delete iter->second;
            }
        }

        // Always clear
        _map.clear();
    }

    /**
    * Erases the mapped_type associated with the keyID.
    * If the mapped_type is a pointer it deletes it from the heap.
    * @param keyID is the associated key.
    */
    void erase(const key_type keyID)
    {
        if (_map.count(keyID) == 0) {
            return;
        }
        // Check if mapped_type is a pointer
        const std::type_info& check  = typeid(_map.begin()->second);
        const char * type_name = check.name();
        // First char will be a 'P'
        if (type_name[0] == 'P') {
            mapped_type mapped = _map.find(keyID)->second;
            delete mapped;
        }
        _map.erase(keyID);
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

    /**
     * Updates a pre-existing mapped_type
     * specified by the keyID.
     * @param keyID the ID for the mapped_type in the map.
     * @param mapped mapped_type
     */
    bool update(const key_type keyID, mapped_type mapped)
    {
        // EVAR needs to return correct error code
        // when keyID does not pre-exist
        bool result = false;

        // Check for Pre-existance
        if (find(keyID) != 0)
        {
            // Delete first then add
            erase(keyID);
            result = insert(keyID, mapped);
        }
        return result;
    }

private:
    /**
     * Prevent access to copy constructor
     */
    map_template(map_template const&);

    /**
     * Prevent access to assignment operator
     */
    map_template& operator=(map_template const&);

    /**
     * The std::map that stores the mapped_types by key_type
     */
    std::map <key_type, mapped_type> _map;
};

/// @}
} // end of namespace sensors
} // end of namespace usml
