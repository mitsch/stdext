/// @file carrays.hpp
/// @module types
/// @library stdext
/// @author Michael Koch
/// @copyright CC BY 3.0

#include <stdext/integral/constant.hpp>

#ifndef STDEXT__TYPES__CARRAYS
#define STDEXT__TYPES__CARRAYS

namespace stdext
{
namespace types
{

	template <typename A> struct is_carray : integral::false_constant {};
	template <typename A> struct is_carray<A[]> : integral::true_constant {};
	template <typename A, size_t B> struct is_carray<A[B]> : integral::true_constant {};

	template <typename A> struct remove_extent {using type = A;};
	template <typename A> struct remove_extent<A[]> {using type = A;};
	template <typename A, size_t B> struct remove_extent<A[B]> {using type = A;};

	template <typename A> struct remove_all_extents {using type = A;};
	template <typename A> struct remove_all_extents<A[]> : remove_all_extents<A> {};
	template <typename A, size_t B> struct remove_all_extents<A[B]> : remove_all_extents<A> {};

	template <typename A> struct rank : integral::index_constant<0> {};
	template <typename A> struct rank<A[]> : integral::index_constant<1+rank<A>::value> {};
	template <typename A, size_t B> struct rank<A[B]> : integral::index_constant<1+rank<A>::value> {};
	template <typename A> constexpr auto rank_v = rank<A>::value;

	template <typename A, size_t B = 0> struct extent : integral::index_constant<0> {};
	template <typename A> struct extent<A[], 0> : integral::index_constant<0> {};
	template <typename A, size_t B> struct extent<A[B], 0> : integral::index_constant<B> {};
	template <typename A, size_t B> struct extent<A[], B> : extent<A, B-1> {};
	template <typename A, size_t B, size_t C> struct extent<A[B], C> : extent<A, C-1> {};

}
}

#endif

