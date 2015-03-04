/**
 * @file quadtree.h
 */
#pragma once

#include <usml/types/quad.h>
#include <usml/types/quadtree_functors.h>

namespace usml {
namespace types {

/**
 * A Quadtree is an advanced data structure that is specialized for
 * storing 2-dimensional data. A Quadtree gets its name from the way the
 * information in the tree is stored. Rather then linearly storing data in
 * a list, a Quadtree divides the 2D grid into quadrants, hence Quadtree.
 * The elements are then placed into the quadrants to which they belong to.
 *
 * When a quadrant has reached the criteria to which it needs to be split, it
 * divides the quadrant into four new quadrants and then places the elements
 * into these smaller quadrants. Thus dividing the elements into successively
 * smaller and smaller quadrants. By dividing elements into quadrants
 * in the 2D grid, we can quickly exclude large areas of the grid from being
 * added to the query list and potentially reducing its size.
 *
 * @xref Quadtree, Wikipedia 2015.
 *       See http://en.wikipedia.org/wiki/Quadtree for more information.
 * @xref C. A. Shaffer, Data Structures & Algorithms Analysis in C++,
 *       3rd Edition, (2011), p. 455.
 */
template<class C, class S, class B, class T, std::size_t N>
class USML_DLLEXPORT quadtree {

    public:

        typedef T                   value_type ;
        typedef typename T::coord_type       coord_type ;
        typedef quad<value_type>    node_type ;
        typedef node_type*          node_ptr ;
        typedef value_type*         value_ptr ;
        typedef std::size_t         size_type ;
        typedef C                   criteria_functor ;
        typedef B                   bound_functor ;
        typedef S                   split_functor ;
        typedef quadtree<criteria_functor,split_functor,
                         bound_functor,value_type,N>         self_type ;

        /**
         * Constructor, creates the root node of the tree using
         * the information passed in and sets the parent node to NULL.
         *
         * @param x         Minimum x coordinate for this quadtree
         * @param y         Minimum y coordinate for this quadtree
         * @param width     extent in the x dimension
         * @param height    extent in the y dimension
         */
        quadtree( coord_type x, coord_type y,
                  coord_type width, coord_type height )
        {
            _root = new node_type( x, y, width, height, _size ) ;
            _root->parent( NULL ) ;
        }

        /**
         * Destructor, calls the destroy tree function from the
         * root node, which then recursively destroys each node
         * within the tree.
         */
        virtual ~quadtree() {
            destroy_tree( _root ) ;
        }

        /**
         * Inserts an element into the quadtree. This function
         * first finds the node within the quadtree that is the
         * smallest node that fully contains the element. This node
         * is then checked against the criteria_functor. If the node
         * does not need to be split, the element is immediately added
         * to the node's data container. Otherwise the split_functor
         * is called on the smallest node. Once the node has been split,
         * a couple of checks are used to assure continuity of the tree.
         *
         *  - Firstly, if the node that is being split was the _root node
         *    then we must redefine it for the tree.
         *  - Second, if it was not the _root node, we need to replace the
         *    parent's child node that we have just split.
         *  - Finally, we search among the newly created nodes to find which
         *    of them the element is to be placed in.
         *
         * As elements are inserted on an individual basis there is no need
         * to recursively check the criteria functor.
         *
         * @param v     element to be inserted into the quadtree.
         */
        void insert( value_type v ) {
            node_ptr tmp = find_node( _root, v ) ;
            if( criteria_functor::check( tmp ) ) {
                node_ptr tmp2 = split_functor::apply( tmp ) ;
                if( tmp == _root ) {
                    _root = tmp2 ;
                    delete tmp ;
                } else {
                    if( tmp == tmp->parent()->top_left() ) {
                        tmp->parent()->top_left( tmp2 ) ;
                    } else if( tmp == tmp->parent()->top_right() ) {
                        tmp->parent()->top_right( tmp2 ) ;
                    } else if( tmp == tmp->parent()->bottom_left() ) {
                        tmp->parent()->bottom_left( tmp2 ) ;
                    } else {
                        tmp->parent()->bottom_right( tmp2 ) ;
                    }
                }
                tmp = find_node( tmp2, v ) ;
            }
            tmp->add(v) ;
        }

        /**
         * Intializer function for query the quadtree.
         *
         * @tparam Box   class that has public members x, y,
         *               height, and width.
         * @tparam List   container type, the container must support
         *                the push_back function to be a valid container.
         * @param b     query box
         * @param l     pointer to the container to populate the data to
         */
        template<class Box, class List>
        void query( const Box b, List* l ) {
            construct_list( _root, b, l ) ;
        }

        /**
         * Test funciton -- Not permanent
         */
        void print() {
            std::cout << "ROOT"
                      << " x[" << _root->_x << " " << _root->_x+_root->_w << "]"
                      << " y[" << _root->_y << " " << _root->_y+_root->_h << "]"
                      << std::endl ;
            print( _root ) ;
        }

    protected:

        /**
         * Test function
         */
        void print( node_ptr n ) {
            size_type size( n->size() ) ;
            if( n != _root ) {
                std::cout << "\tNEXT LEVEL"
                << " x[" << n->_x << " " << n->_x+n->_w << "]"
                << " y[" << n->_y << " " << n->_y+n->_h << "]"
                << std::endl ;
            }
            for(size_type i=0; i<size; ++i) {
                value_type tmp = n->data(i) ;
                std::cout << tmp << std::endl ;
            }
            if( n->top_left() ) {
                std::cout << "TOP_LEFT:" << std::endl ;
                print( n->top_left() ) ;
            }
            if( n->top_right() ) {
                std::cout << "TOP_RIGHT:" << std::endl ;
                print( n->top_right() ) ;
            }
            if( n->bottom_left() ) {
                std::cout << "BOTTOM_LEFT:" << std::endl ;
                print( n->bottom_left() ) ;
            }
            if( n->bottom_right() ) {
                std::cout << "BOTTOM_RIGHT:" << std::endl ;
                print( n->bottom_right() ) ;
            }
        }

        /**
         * Recursively searches the quadtree and constructs the list
         * of elements that are contained with the queried region
         *
         * @tparam Box  class that has public members x, y,
         *              height, and width.
         * @tparam List   container type, the container must support
         *                the push_back function to be a valid container.
         * @param n     current node to check for inclusion
         * @param b     query box
         * @param l     pointer to the container to populate the data to
         */
        template<class Box, class List>
        void construct_list( node_ptr n, const Box b, List* l ) {
            if( (b.x <= n->_x) && (b.y <= n->_y) &&
                ((n->_x+n->_w) <= (b.x+b.width)) &&
                ((n->_y+n->_h) <= (b.y+b.height)) )
            {
                add_sector( n, l ) ;
            } else {
                if( intersect(Box(n), b) ) {
                    size_type size( n->size() ) ;
                    for(size_type i=0; i<size; ++i) {
                        // Only add the data if it is actually in the box
                        if( b.x <= n->data(i).x && n->data(i).x <= (b.x+b.width) ) {
                            if( b.y <= n->data(i).y && n->data(i).y <= (b.y+b.height) ) {
                                l->push_back( n->data(i) ) ;
                            }
                        }
                    }
                    if( n->top_left() && intersect( Box(n->top_left()), b ) )
                        construct_list( n->top_left(), b, l ) ;
                    if( n->bottom_left() && intersect( Box(n->bottom_left()), b ) )
                        construct_list( n->bottom_left(), b, l ) ;
                    if( n->top_right() && intersect( Box(n->top_right()), b ) )
                        construct_list( n->top_right(), b, l ) ;
                    if( n->bottom_right() && intersect( Box(n->bottom_right()), b ) )
                        construct_list( n->bottom_right(), b, l ) ;
                }
            }
        }

        /**
         * Checks that either box is contained within the other. This
         * function checks as to whether either box intersects with the
         * other and returns true if either one contains at least one
         * corner from the other.
         *
         * @tparam Box   class that has public members x, y,
         *               height, and width.
         * @param b1     a box to check for intersection with
         * @param b2     a box to check for intersection with
         */
        template<class Box>
        bool intersect( const Box& b1, const Box& b2 ) {
            return ( corners(b1,b2) || corners(b2,b1) ) ;
        }

        /**
         * Determines if any of the four corners from box 1
         * are within box 2. This check implies that box 2
         * intersects box 1.
         *
         * @tparam Box   class that has public members x, y,
         *               height, and width.
         * @param b1     the box to check intersection against
         * @param b2     the box to check for intersection
         */
        template<class Box>
        bool corners( const Box b1, const Box b2 ) {
            bool lower_x = (b2.x <= b1.x) && (b1.x < (b2.x+b2.width)) ;
            bool lower_y = (b2.y <= b1.y) && (b1.y < (b2.y+b2.height)) ;
            // Is the SW corner of the node in the box?
            if( lower_x && lower_y ) return true ;
            bool upper_x = (b2.x < (b1.x+b1.width)) && ((b1.x+b1.width) < (b2.x+b2.width)) ;
            // Is the SE corner of the node in the box?
            if( upper_x && lower_y ) return true ;
            bool upper_y = (b2.y < (b1.y+b1.height)) && ((b1.y+b1.height) < (b2.y+b2.height)) ;
            // Is the NW corner of the node in the box?
            if( lower_x && upper_y ) return true ;
            // Is the NE corner of the node in the box?
            if( upper_x && upper_y ) return true ;
            // None of the node's corners are in the box, so this node doesn't intersect the box
            return false ;
        }

        /**
         * Adds an entire sector of elements to the list because
         * this region of the quadtree is entirely contained within
         * the query box and so every element must be added. Recursively
         * moves down the tree from the parent node and adds every
         * element to the List l.
         *
         * @tparam List   container type, the container must support
         *                the push_back function to be a valid container.
         * @param n     parent node of the sector
         * @param l     pointer to the container to populate the data to
         */
        template<class List>
        void add_sector( node_ptr n, List* l ) {
            size_type size( n->size() ) ;
            for(size_type i=0; i<size; ++i) {
                l->push_back( n->data(i) ) ;
            }
            if( n->top_left() ) add_sector( n->top_left(), l ) ;
            if( n->bottom_left() ) add_sector( n->bottom_left(), l ) ;
            if( n->top_right() ) add_sector( n->top_right(), l ) ;
            if( n->bottom_right() ) add_sector( n->bottom_right(), l ) ;
        }

        /**
         * Recursively searches the quadtree for the smallest
         * quad that fully contains this item. If the item does
         * not fit within one quadrant exclusively, then the item
         * is placed within the parent node.
         *
         * @param node  current node to be checked against
         * @param item  element that needs the smallest quad
         */
        node_ptr find_node( node_ptr node, value_type item ) {
            bool left = bound_functor::left( node, item ) ;
            bool right = bound_functor::right( node, item ) ;
            bool top = bound_functor::top( node, item ) ;
            bool bottom = bound_functor::bottom( node, item ) ;
            node_ptr result = node ;
            if( (left || right) && (top || bottom) ) {
                if( left ) {
                    if( top ) {
                        if( node->top_left() ) {
                            result = find_node( node->top_left(), item ) ;
                        }
                    } else {
                        if( node->bottom_left() ) {
                            result = find_node( node->bottom_left(), item ) ;
                        }
                    }
                } else {
                    if( top ) {
                        if( node->top_right() ) {
                            result = find_node( node->top_right(), item ) ;
                        }
                    } else {
                        if( node->bottom_right() ) {
                            result = find_node( node->bottom_right(), item ) ;
                        }
                    }
                }
            }
            return result ;
        }

    private:

        /**
         * The root node of the quadtree
         */
        node_ptr _root ;

        /**
         * Maximum number of elements to be stored within
         * each node of the tree.
         */
        static const size_type _size = N ;

        /**
         * Recursively deletes every node within the quadtree
         *
         * @param node  node to be deleted
         */
        void destroy_tree( node_ptr node ) {
            if( node->top_right() )
                destroy_tree( node->top_right() ) ;
            if( node->top_left() )
                destroy_tree( node->top_left() ) ;
            if( node->bottom_left() )
                destroy_tree( node->bottom_left() ) ;
            if( node->bottom_right() )
                destroy_tree( node->bottom_right() ) ;
            delete node ;
    }
};

/**
 * Specialized definitions of Quadtrees
 * Having the specialized typedef defines available reduces the need to clutter
 * code with multiple instances of the same type of templated quadtree, hence reducing
 * code bloat.
 */
template<class T, std::size_t N>
struct quadtree_type {
    typedef quadtree<max_count<T,N>, split_points<bound_box<T>,T>,bound_box<T>,T,N> points ;
};

}   // end of namespace types
}   // end of namespace usml
