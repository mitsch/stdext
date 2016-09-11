/// @file sfinae.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef STDEXT__SFINAE__HPP
#define STDEXT__SFINAE__HPP

namespace stdext
{

	/*
		The SFINAE is a mechanism in C++ to support disabling certain structures depending on a condition.
		As long static if is not incorporated into the C++ language, SFINAE will be necessary to allow for refinements
		of structures such as methods or classes.
	*/

	/// Maps all to type \a void
 	template <class ...> using void_t = void;

	/// Disables an expression if the condition is not true
	///
	/// @{
	template <bool, typename = void> struct enable_if {};
	template <typename A> struct enable_if<true, A> {using type = A;};
	template <bool A, typename B> using enable_if_t = typename enable_if<A, B>::type;
	/// @}

}

#endif
