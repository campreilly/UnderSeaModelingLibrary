/**
 * @file quadtree_functors.h
 *
 * This is where all quadtree functors should be included. While C++11 and newer
 * have access to functors, C++98 and older do not. So in order to allow full
 * customization of the quadtree class, pseudo-functors were created. These functors
 * are structures that have static functions and therefore need no instantiation.
 * These functions are then passed into the class as the template parameters and
 * allow the user to create and customize the quadtree to their own designs.
 *
 * With the provided quadtree class, the user must specify three functors:
 *
 * - Criteria functor: This functor tells the quadtree class when a given node
 *                     needs to be split. This functor must return a bool and the
 *                     static function must be called check.
 * - Split_functor: This functor does the work of splitting the elements of a
 *                  quadrant from the quadtree when it meets the splitting
 *                  criteria. This functor must return a newly constructed
 *                  node with the elements divided accordingly and the static
 *                  function must be called apply.
 * - Bound_functor: This functor defines how each element in the quadtree fits
 *                  within a quadrant. This function must have four separate
 *                  static functions, namely, top, right, left, bottom and must
 *                  all return bool.
 *
 * Using these three functors the user can define any specialized version of the
 * quadtree. For example, the user could define functors that split and bound
 * shapes (not included at this time) within the quadtree by defining split_functor
 * and bound_functors that apply to shapes. At present quadtree functors are only
 * provided for points in a 2D space and a criteria of maximum size.
 */
#pragma once

#include <cstddef>
#include <usml/eigenverb/eigenverb.h>

namespace usml {
namespace types {

/**
 * Forward declaration of quad class
 */
template<class T, class D> class quad ;

/**
 * Criteria functor
 *
 * Determines if the size of a node is greater
 * than the allowed size.
 *
 * @tparam T    element stored of the quadtree
 * @tparam N    maximum number of elements in each quad
 */
template<class T, std::size_t N>
struct USML_DLLEXPORT max_count {
    typedef quad<T>*    argument_type ;
    typedef bool        result_type ;

    static const std::size_t size = N ;
    static result_type
    check( argument_type a ) {
        return (a->size() >= size) ;
    }
};

/**
 * Split functor
 *
 * When the size of a node has reached the maximum number
 * of elements allowed, this functor divides all of the
 * elements into the parent or children node. The dividing
 * is done by checking if the element fits within one of
 * the children quadrants and if not it is placed in the
 * parent node. The newly created node is then passed
 * back.
 *
 * @tparam B    bounding functor
 * @tparam T    element stored within the quadtree
 */
template<class B, class T>
struct USML_DLLEXPORT split_points {
    typedef T                 value_type ;
    typedef B                 bound_functor ;
    typedef quad<value_type>  node_type ;
    typedef node_type*        argument_type ;
    typedef argument_type     result_type ;
    typedef std::size_t       size_type ;
    typedef typename node_type::coord_type  coord_type ;

    static result_type
    apply( argument_type a ) {
        size_type size( a->size() ) ;
        argument_type new_node = new node_type( a ) ;
        new_node->create_children( size ) ;

        // split the elemnts into each node
        for(size_type i=0; i<size; ++i) {
            value_type curr = a->data(i) ;
            bool left = bound_functor::left( a, curr ) ;
            bool right = bound_functor::right( a, curr ) ;
            bool top = bound_functor::top( a, curr ) ;
            bool bottom = bound_functor::bottom( a, curr ) ;
            if( (left || right) && (top || bottom) ) {
                if( left ) {
                    if( top ) {
                        new_node->top_left()->add( curr ) ;
                    } else {
                        new_node->bottom_left()->add( curr ) ;
                    }
                } else {
                    if( top ) {
                        new_node->top_right()->add( curr ) ;
                    } else {
                        new_node->bottom_right()->add( curr ) ;
                    }   // end if/else top
                }   // end if/else left
            } else {
                new_node->add( curr ) ;
            }   // end if/else in parent
        }   // end for in i
        return new_node ;
    }   // end function apply
};

/**
 * Bound functor
 *
 * This function checks whether the elements fits within
 * open intervals of
 *      [x,x+0.5*w) (x+0.5*w,x+w]
 * and
 *      [y,y+0.5*h) (y+0.5*h,y+h]
 * This allows the quadtree to determine if an element is
 * within multiple quadrants or exclusive to one. This
 * functor is specific to points on a 2D grid and thus
 * the element must have an x and y public member variable.
 *
 * @tparam T    element stored within the quadtree
 */
template<class T>
struct USML_DLLEXPORT bound_box {
    typedef T                    value_type ;
    typedef quad<value_type>*    bounding_node ;
    typedef bool                 result_type ;

    static result_type
    left( bounding_node n, value_type v )
    {
        return ( v.x < (n->_x + 0.5 * n->_w) ) ;
    }

    static result_type
    right( bounding_node n, value_type v )
    {
        return ( v.x > (n->_x + 0.5 * n->_w) ) ;
    }

    static result_type
    top( bounding_node n, value_type v )
    {
        return ( v.y > (n->_y + 0.5 * n->_h) ) ;
    }

    static result_type
    bottom( bounding_node n, value_type v )
    {
        return ( v.y < (n->_y + 0.5 * n->_h) ) ;
    }
};

}   // end of namespace types
}   // end of namespace usml
