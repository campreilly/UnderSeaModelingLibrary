/*
 * @file eigenverb_collection.h
 */
#pragma once

#include <usml/eigenverb/eigenverb.h>
#include <usml/types/quadtree.h>

using namespace usml::types ;

namespace usml {
namespace eigenverb {

typedef quadtree_type<eigenverb,100>::points   eigenverb_tree ;

/**
 *
 */
class USML_DECLSPEC eigenverb_collection {

    public:

        /**
         * Constructor
         *
         * @param lon
         * @param lat
         * @param lon_range
         * @param lat_range
         * @param layers
         */
        eigenverb_collection(
                double lon,
                double lat,
                double lon_range, double lat_range,
                size_t layers = 0 ) ;

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
        eigenverb_tree bottom() const ;

        /**
         * Returns the list of eigenverbs for the surface
         * interface
         */
        eigenverb_tree surface() const ;

        /**
         * Returns the list of eigenverbs for the volume
         * upper interface
         */
        vector<eigenverb_tree*> upper() const ;

        /**
         * Returns the list of eigenverbs for the l'th volume
         * upper interface
         */
        eigenverb_tree upper( size_t l ) const ;

        /**
         * Returns the list of eigenverbs for the volume
         * lower interface
         */
        vector<eigenverb_tree*> lower() const ;

        /**
         * Returns the list of eigenverbs for the l'th volume
         * lower interface
         */
        eigenverb_tree lower( size_t l ) const ;

        /**
         * Returns true if there are volume layers, ie the size
         * of either _upper or _lower are not zero.
         */
        bool volume() const ;

    protected:

        /**
         * List of all the eigenverbs for bottom boundary collisions
         */
        eigenverb_tree* _bottom ;

        /**
         * List of all the eigenverbs for surface boundary collisions
         */
        eigenverb_tree* _surface ;

        /**
         * Vector of eigenverb lists for upper volume layer collisions
         */
        vector<eigenverb_tree*> _upper ;

        /**
         * Vector of eigenverb lists for lower volume layer collisions
         */
        vector<eigenverb_tree*> _lower ;
};

}   // end of namespace waveq3d
}   // end of namespace usml
