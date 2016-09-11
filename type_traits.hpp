/// @file type_traits.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef STDEXT__TYPE_TRAITS__HPP
#define STDEXT__TYPE_TRAITS__HPP

#include <stdext/integral/constant.hpp>

namespace stdext
{

	template <typename> struct is_void : false_constant {};
	template <> struct is_void<void> : true_constant {};
	template <typename A> constexpr auto is_void_v = is_void<A>::value;

	template <typename> struct is_nullptr : false_constant {}; 
	template <> struct is_nullptr<decltype(nullptr)> : true_constant {};
	template <typename A> constexpr auto is_nullptr_v = is_nullptr<A>::value;

	template <typename> struct is_integral : false_constant {};
	template <> struct is_integral<bool> : true_constant {};
	template <> struct is_integral<char> : true_constant {};
	template <> struct is_integral<signed char> : true_constant {};
	template <> struct is_integral<unsigned char> : true_constant {};
	template <> struct is_integral<char16_t> : true_constant {};
	template <> struct is_integral<char32_t> : true_constant {};
	template <> struct is_integral<wchar_t> : true_constant {};
	template <> struct is_integral<short> : true_constant {};
	template <> struct is_integral<unsigned short> : true_constant {};
	template <> struct is_integral<int> : true_constant {};
	template <> struct is_integral<unsigned int> : true_constant {};
	template <> struct is_integral<long> : true_constant {};
	template <> struct is_integral<unsigned long> : true_constant {};
	template <> struct is_integral<long long> : true_constant {};
	template <> struct is_integral<unsigned long long> : true_constant {};
	template <typename A> struct is_integral<const A> : is_integral<A> {};
	template <typename A> struct is_integral<volatile A> : is_integral<A> {};
	template <typename A> struct is_integral<const volatile A> : is_integral<A> {};

	template <typename> struct is_floating_point : false_constant {};
	template <> struct is_floating_point<float> : true_constant {};
	template <> struct is_floating_point<double> : true_constant {};
	template <> struct is_floating_point<long double> : true_constant {};
	template <typename A> struct is_floating_point<const A> : is_floating_point<A> {};
	template <typename A> struct is_floating_point<volatile A> : is_floating_point<A> {};
	template <typename A> struct is_floating_point<const volatile A> : is_floating_point<A> {};

}

#endif
