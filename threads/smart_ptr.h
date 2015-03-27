/**
 * @file smart_ptr.h
 * Portable versions of smart pointers.
 */
#pragma once

/**
 * Use std:: versions of shared_ptr and unique_ptr for
 * compilers that support C++0x and beyond.
 */
#if __cplusplus > 199711L

	#include <memory>

	namespace usml {
	namespace threads {

	using std::shared_ptr ;
	using std::unique_ptr ;

/**
 * Use boost:: versions of shared_ptr and unique_ptr for
 * compilers that do not yet support the std:: versions.
 */
#else

	#include <boost/shared_ptr.hpp>
	#include <boost/scoped_ptr.hpp>

	namespace usml {
	namespace threads {

	/**
	 * Creates an alias for shared_ptr while we wait for C++ 11 to become
	 * standard in gcc.
	 */
	using boost::shared_ptr ;

	/**
	 * Creates an alias for unique_ptr while we wait for C++ 11 to become
	 * standard in gcc. To maintain compatibility with version of
	 * C++ prior to 11 and boost versions prior to 1.57, we implement
	 * unique_ptr using the Boost scoped pointer.  It performs the same memory
	 * management tasks as unique_ptr, but can not handle arrays at this time.
	 */
	template<typename T> class unique_ptr: public boost::scoped_ptr<T> {
	public:
		explicit unique_ptr<T>(T * p = 0) : boost::scoped_ptr<T>(p) {}
	};

#endif

/// @}
} // end of namespace threads
} // end of namespace usml
