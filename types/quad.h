/**
 * @file quad.h
 * Base class that describes the Cartesian pair and the pair
 * extents used in the quadtree class.
 */
#pragma once

#include <usml/types/wposition1.h>

namespace usml {
namespace types {

using boost::numeric::ublas::vector ;

/// @ingroup quadtree
/// @{

    /**
     * Base class that describes the Cartesian pair and the pair
     * extents used in the quadtree class.
     * Quadrants are defined as below:
     *  _________________
     * |        |        |
     * |   1    |   0    |
     * |        |        |
     * |-----------------|
     * |        |        |
     * |   2    |   3    |
     * |________|________|
     *
     * The enumerations are defined for code readability and
     * defined with values for use in accessing elements within
     * the quad's children container.
     */
    enum QUADRANT
    {
        TOP_RIGHT = 0,
        TOP_LEFT = 1,
        BOTTOM_LEFT = 2,
        BOTTOM_RIGHT = 3
    };

/**
 * The quadrant (quad) is defined by a Cartesian pair and a pair of
 * extents. The (x,y) Cartesian pair stored within this class
 * corresponds to the lower left hand corner, with the extents
 * width for x distance and height for y distance, defining the
 * other three corners.
 *        _________________ (x+width,y+height)
 *       |                 |
 *       |                 |
 *       |                 |
 *       |                 |
 *       |________ ________|
 *     (x,y)            (x+width,y)
 *
 * A quad is a node of the quadtree. Each quad has exactly 4
 * children and a memory of it's parent, as well as storing the
 * information within the quadtree that is used for querying
 * and splitting the quadtree.
 *
 * @tparam T    Type of element that is stored in the data container
 * @tparam D    Type of container that stores the data, this is defaulted
 *              to a ublas vector. Any container that is used must have
 *              a size function that returns the current number of elements
 *              stored, operator[], operator(), and templated for it to be
 *              a valid choice for the data container.
 */
template<class T, class D=vector<T> >
class USML_DLLEXPORT quad {

    public:

        /**
         * Declaration of many commonly used types within
         * this class and other related classes.
         */
        typedef T                   value_type ;
        typedef D                   data_container ;
        typedef double              coord_type ;
        typedef std::size_t         size_type ;
        typedef quad<value_type>    self_type ;
        typedef self_type*          node_ptr ;
        typedef const node_ptr      const_node_ptr ;
        typedef vector<node_ptr>    container_type ;

        /**
         * Constructor
         * The four children are defined and set to NULL. The
         * data container is also created empty with the maximum
         * size allowed and the index is set to zero.
         *
         * @param x         x coordinate of this quadrant
         * @param y         y coordinate of this quadrant
         * @param width     maximum extent in x for this quadrant
         * @param height    maximum extent in y for this quadrant
         * @param max_size  maximum number of elements that can be
         *                  stored in the data container
         */
        quad( coord_type x, coord_type y,
              coord_type width, coord_type height,
              size_type max_size )
        : _x(x), _y(y), _w(width), _h(height), _index(0)
        {
            _quadrant = container_type(4, NULL) ;
            _data = data_container(max_size) ;
        }

        /**
         * Copy constructor
         *
         * @param other     pointer to the quad to be copied
         */
        quad( const_node_ptr other ) {
            _x = other->_x ;
            _y = other->_y ;
            _w = other->_w ;
            _h = other->_h ;
            _quadrant = container_type(4, NULL) ;
            _data = data_container( other->size() ) ;
            _parent = other->parent() ;
            _index = 0 ;
        }

        /**
         * Destructor
         */
        virtual ~quad() {}

        /**
         * Return a pointer to the top right quadrant.
         * This is the child of this quadrant that corresponds
         * to the TOP_RIGHT defined above.
         */
        const_node_ptr top_right() {
            return _quadrant(TOP_RIGHT) ;
        }

        /**
         * Redefines the top right quadrant.
         *
         * @param n     new quad to be placed at this child
         */
        void top_right( node_ptr n ) {
            if( _quadrant(TOP_RIGHT) )
                delete _quadrant(TOP_RIGHT) ;
            _quadrant(TOP_RIGHT) = n ;
        }

        /**
         * Return a pointer to the top left quadrant.
         * This is the child of this quadrant that corresponds
         * to the TOP_LEFT defined above.
         */
        const_node_ptr top_left() {
            return _quadrant(TOP_LEFT) ;
        }

        /**
         * Redefines the top_left quadrant.
         *
         * @param n     new quad to be placed at this child
         */
        void top_left( node_ptr n ) {
            if( _quadrant(TOP_LEFT) != NULL )
                delete _quadrant(TOP_LEFT) ;
            _quadrant(TOP_LEFT) = n ;
        }

        /**
         * Return a pointer to the bottom left quadrant.
         * This is the child of this quadrant that corresponds
         * to the BOTTOM_LEFT defined above.
         */
        const_node_ptr bottom_left() {
            return _quadrant(BOTTOM_LEFT) ;
        }

        /**
         * Redefines the bottom left quadrant.
         *
         * @param n     new quad to be placed at this child
         */
        void bottom_left( node_ptr n ) {
            if( _quadrant(BOTTOM_LEFT) )
                delete _quadrant(BOTTOM_LEFT) ;
            _quadrant(BOTTOM_LEFT) = n ;
        }

        /**
         * Return a pointer to the bottom right quadrant.
         * This is the child of this quadrant that corresponds
         * to the BOTTOM_RIGHT defined above.
         */
        const_node_ptr bottom_right() {
            return _quadrant(BOTTOM_RIGHT) ;
        }

        /**
         * Redefines the bottom right quadrant.
         *
         * @param n     new quad to be placed at this child
         */
        void bottom_right( node_ptr n ) {
            if( _quadrant(BOTTOM_RIGHT) )
                delete _quadrant(BOTTOM_RIGHT) ;
            _quadrant(BOTTOM_RIGHT) = n ;
        }

        /**
         * Returns the number of elements in this quadrant's
         * data container.
         */
        size_type size() const {
            return _index ;
        }

        /**
         * Returns the i'th element of the data container.
         *
         * @param i     index of the element.
         */
        value_type data( size_type i ) {
            return _data[i] ;
        }

        /**
        * Adds an item to the back of the data container.
        * This is the custom version of push_back. See
        * comment for private member variable _index
        * for more details.
        *
        * @param t      element to be put in this quadrant's
        *               data container
        */
        void add( value_type t ) {
            _data(_index++) = t ;
        }

        /**
         * Returns a node pointer to the parent
         * of this quad.
         */
        const_node_ptr parent() {
            return _parent ;
        }

        /**
         * Sets the parent node of this quad.
         *
         * @param p     parent node pointer
         */
        void parent( node_ptr p ) {
            _parent = p ;
        }

        /**
         * Creates the children nodes from the parent node by
         * evenly dividing the parent into four new quadrants.
         *
         * @param s      max size of the data container
         */
        void create_children( size_type s ) {
            coord_type nw = 0.5 * _w, nh = 0.5 * _h ;
                // Top left quadrant
            _quadrant(TOP_LEFT) =
                    new self_type( _x, _y+nh, nw, nh, s ) ;
            _quadrant(TOP_LEFT)->_parent = this ;
                // Bottom left quadrant
            _quadrant(BOTTOM_LEFT) =
                    new self_type( _x, _y, nw, nh, s ) ;
            _quadrant(BOTTOM_LEFT)->_parent = this ;
                // Top right quadrant
            _quadrant(TOP_RIGHT) =
                    new self_type( _x+nw, _y+nh, nw, nh, s ) ;
            _quadrant(TOP_RIGHT)->_parent = this ;
                // Bottom right quadrant
            _quadrant(BOTTOM_RIGHT) =
                    new self_type( _x+nw, _y, nw, nh, s ) ;
            _quadrant(BOTTOM_RIGHT)->_parent = this ;
        }

        /**
         * Information concerning the corner and extent
         * of this quad node. These member variables were
         * chosen to be left public because of the number of
         * times that they will be accessed during quadtree
         * traversing and querying.
         */
        coord_type _x ;
        coord_type _y ;
        coord_type _w ;
        coord_type _h ;

    private:

        /**
         * Container for the quadrant nodes
         */
        container_type  _quadrant ;

        /**
         * Container for the elements stored within
         * this quadrant.
         */
        data_container _data ;

        /**
         * Pointer to the parent node. This is used when a new
         * node is added in the tree or when a node is split
         * As the new node replaces one of the parent's children
         * when being split, the correct link between the parent
         * and the new child node must be adjusted.
         */
        node_ptr _parent ;

        /**
         * Index of the next slot for an element that can be inserted
         * within the data_container. This was added because boost's
         * vectors did not have a push_back method. So to accommodate this
         * and still be able to take advantage of boost's vectors
         * a pseudo-push_back created using an internal variable to keep track
         * of where the 'back' was.
         */
        size_type _index ;
};

/// @}
}   // end of namespace types
}   // end of namespace usml
