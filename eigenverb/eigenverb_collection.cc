/*
 * @file eigenverb_collection.cc
 */

#include <usml/eigenverb/eigenverb_collection.h>

using namespace usml::eigenverb ;

/**
 * Cosntructor
 */
eigenverb_collection::eigenverb_collection(
    const size_t layers )
    : _upper(layers), _lower(layers)
{

}

/**
 * Destructor
 */
eigenverb_collection::~eigenverb_collection()
{

}

/**
 * Adds an eigenverb to the collection
 */
void eigenverb_collection::add_eigenverb(
    eigenverb e, interface_type i )
{
    switch(i) {
        case BOTTOM:
            _bottom.push_back( e ) ;
    //                    _bottom.insert( e ) ;
            break;
        case SURFACE:
            _surface.push_back( e ) ;
    //                    _surface.insert( e ) ;
            break;
        case VOLUME_UPPER:
        {
            size_t something = 0 ;
            _upper(something).push_back( e ) ;
    //                    _upper(something).insert( e ) ;
        }
            break;
        case VOLUME_LOWER:
        {
            size_t something = 0 ;
            _lower(something).push_back( e ) ;
    //                    _lower(something).insert( e ) ;
        }
            break;
        default:
            throw std::invalid_argument(
                    "Invalid interface type. Must be one defined in eigenverb.h") ;
            break;
    }
}

/**
 * Returns the list of eigenverbs for the bottom
 * interface
 */
eigenverb_list eigenverb_collection::bottom() const
{
    return _bottom ;
}

/**
 * Returns the list of eigenverbs for the surface
 * interface
 */
eigenverb_list eigenverb_collection::surface() const
{
    return _surface ;
}

/**
 * Returns the list of eigenverbs for the volume
 * upper interface
 */
vector<eigenverb_list> eigenverb_collection::upper() const
{
    return _upper ;
}

/**
 * Returns the list of eigenverbs for the l'th volume
 * upper interface
 */
eigenverb_list eigenverb_collection::upper( size_t l ) const
{
    return _upper(l) ;
}


/**
 * Returns the list of eigenverbs for the volume
 * lower interface
 */
vector<eigenverb_list> eigenverb_collection::lower() const
{
    return _lower ;
}

/**
 * Returns the list of eigenverbs for the l'th volume
 * lower interface
 */
eigenverb_list eigenverb_collection::lower( size_t l ) const
{
    return _lower(l) ;
}

/**
 * Determines if there are volumes layers
 */
bool eigenverb_collection::volume() const {
    if( _upper.size() != 0 )
        return true ;
    return false ;
}
