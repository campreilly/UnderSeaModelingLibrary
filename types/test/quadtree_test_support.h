#include <iostream>

/**
 * Point class that stores a Cartesian pair, (x,y)
 */
struct point {
    typedef double  coord_type ;
    point() {}
    point( coord_type __x, coord_type __y )
        : x(__x), y(__y)
    {}
    coord_type x ;
    coord_type y ;
    friend std::ostream& operator<< (std::ostream& os, point& p) ;
    bool operator== (const point& rhs) {
        return ( x == rhs.x && y == rhs.y ) ;
    }
    bool operator!= ( const point& rhs) {
        return !(*this == rhs) ;
    }
};

std::ostream& operator<< (std::ostream& os, point& p) {
    os << "(" << p.x << ", " << p.y << ")" ;
    return os ;
}

/**
 * Class to construct boxes for querying and intersecting
 */
struct box {
    typedef double  coord_type ;
    box( coord_type _x, coord_type _y,
         coord_type _w, coord_type _h )
        : x(_x), y(_y), width(_w), height(_h)
    {}
    template<class N>
    box( N* n )
        : x(n->_x), y(n->_y), width(n->_w), height(n->_h)
    {}
    coord_type x ;
    coord_type y ;
    coord_type width ;
    coord_type height ;
};

/**
 * Compares two containers by checking that every element in
 * each container is equal to the other and their sizes are
 * equal.
 */
template<class L>
bool compare_list( const L& truth, const L& result ) {
    bool ans = true ;
    size_t size( truth.size() ) ;
    if( size == result.size() ) {
        typename L::const_iterator t = truth.begin() ;
        typename L::const_iterator r = result.begin() ;
        while( t != truth.end() ) {
            if( (t->x != r->x) || (t->y != r->y) )
                ans = false ; break ;
            t++ ;
            r++ ;
        }
    } else {
        ans = false ;
    }
    return ans ;
}

/**
 * Prints the elements within the container L
 */
template<class L>
void print_list( const L& out ) {
    typename L::const_iterator i = out.begin() ;
    while( i != out.end() ) {
        point p = *i ;
        std::cout << p << std::endl ;
        i++ ;
    }
}

/**
 * Sorting function for a container of points
 */
template<class P>
bool sort_by_x( const P& p1, const P& p2 ) {
    if( p1.x == p2.x )
        return p1.y < p2.y ;
    else
        return p1.x < p2.x ;
}
