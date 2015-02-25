/*
 * @file eigenverb_collection.h
 */
#pragma once

#include <usml/eigenverb/eigenverb.h>
//#include <usml/types/quadtree.h>

namespace usml {
namespace eigenverb {

using namespace usml::types ;
using namespace usml::waveq3d ;

/**
 *
 */
class USML_DECLSPEC eigenverb_collection {

    public:

//        typedef quadtree_type::points<eigenverb,10>     eigenverb_tree ;

        /**
         * Constructor
         */
        eigenverb_collection( const size_t layers = 0 )
            : _upper(layers), _lower(layers)
        {}

        /*
         * Destructor
         */
        virtual ~eigenverb_collection() {}

        /**
         *
         */
        virtual void add_eigenverb( eigenverb e, interface_type i ) {
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
                    size_t something = 0 ;
                    _upper(something).push_back( e ) ;
//                    _upper(something).insert( e ) ;
                    break;
                case VOLUME_LOWER:
                    size_t something = 0 ;
                    _lower(something).push_back( e ) ;
//                    _lower(something).insert( e ) ;
                    break;
                default:
                    throw std::invalid_argument(
                            "Invalid interface type. Must be one defined in eigenverb.h") ;
                    break;
            }
        }

    protected:

        /**
         * List of all the eigenverbs for bottom boundary collisions
         */
        eigenverb_list _bottom ;
//        eigenverb_tree _bottom ;

        /**
         * List of all the eigenverbs for surface boundary collisions
         */
        eigenverb_list _surface ;
//        eigenverb_tree _surface ;

        /**
         * Vector of eigenverb lists for upper volume layer collisions
         */
        vector<eigenverb_list> _upper ;
//        vector<eigenverb_tree> _upper ;

        /**
         * Vector of eigenverb lists for lower volume layer collisions
         */
        vector<eigenverb_list> _lower ;
//        vector<eigenverb_tree> _lower ;
};

}   // end of namespace waveq3d
}   // end of namespace usml
