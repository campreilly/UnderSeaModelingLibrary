/*
 * @file eigenverb_collection.h
 */
#pragma once

#include <usml/eigenverb/eigenverb.h>
//#include <usml/types/quadtree.h>

using namespace usml::types ;
//using namespace usml::waveq3d ;

namespace usml {
namespace eigenverb {

/**
 *
 */
class USML_DECLSPEC eigenverb_collection {

    public:

//        typedef quadtree_type::points<eigenverb,10>     eigenverb_tree ;

        /**
         * Constructor
         */
        eigenverb_collection( const size_t layers = 0 ) ;

        /*
         * Destructor
         */
        virtual ~eigenverb_collection() ;

        /**
         *
         *
         * @param e
         * @param i
         */
        virtual void add_eigenverb(
            eigenverb e, interface_type i ) ;

        /**
         * Returns the list of eigenverbs for the bottom
         * interface
         */
        eigenverb_list bottom() const ;

        /**
         * Returns the list of eigenverbs for the surface
         * interface
         */
        eigenverb_list surface() const ;

        /**
         * Returns the list of eigenverbs for the volume
         * upper interface
         */
        vector<eigenverb_list> upper() const ;

        /**
         * Returns the list of eigenverbs for the l'th volume
         * upper interface
         */
        eigenverb_list upper( size_t l ) const ;

        /**
         * Returns the list of eigenverbs for the volume
         * lower interface
         */
        vector<eigenverb_list> lower() const ;

        /**
         * Returns the list of eigenverbs for the l'th volume
         * lower interface
         */
        eigenverb_list lower( size_t l ) const ;

        /**
         * Returns true if there are volume layers, ie the size
         * of either _upper or _lower are not zero.
         */
        bool volume() const ;

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
