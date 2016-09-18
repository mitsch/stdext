/// @file primitives.hpp
/// @module types
/// @library stdext
/// @author Michael Koch
/// @copyright CC BY 3.0

#include <stdext/integral/constant.hpp>

#ifndef STDEXT__TYPES__PRIMITIVES
#define STDEXT__TYPES__PRIMITIVES

namespace stdext
{
namespace types
{

	template <typename> struct is_void : integral::false_constant {};
	template <> struct is_void<void> : integral::true_constant {};
	template <typename A> constexpr auto is_void_v = is_void<A>::value;

	template <typename> struct is_null_pointer : integral::false_constant {};
	template <> struct is_null_pointer<decltype(nullptr)> : integral::true_constant {};
	template <typename A> constexpr auto is_null_pointer_v = is_pointer<A>::value;

	template <typename> struct is_boolean : integral::false_constant {};
	template <> struct is_boolean<bool> : integral::true_constant {};
	template <typename A> constexpr auto is_boolean_v = is_boolean<A>::value;

	template <typename> struct is_integer : integral::false_constant {};
	template <> struct is_integer<char> : integral::true_constant {};
	template <> struct is_integer<char16_t> : integral::true_constant {};
	template <> struct is_integer<char32_t> : integral::true_constant {};
	template <> struct is_integer<wchar_t> : integral::true_constant {};
	template <> struct is_integer<signed char> : integral::true_constant {};
	template <> struct is_integer<unsigned char> : integral::true_constant {};
	template <> struct is_integer<signed short> : integral::true_constant {};
	template <> struct is_integer<unsigned short> : integral::true_constant {};
	template <> struct is_integer<signed int> : integral::true_constant {};
	template <> struct is_integer<unsigned int> : integral::true_constant {};
	template <> struct is_integer<signed long> : integral::true_constant {};
	template <> struct is_integer<unsigned long> : integral::true_constant {};
	template <> struct is_integer<signed long long> : integral::true_constant {};
	template <> struct is_integer<unsigned long long> : integral::true_constant {};
	template <typename A> constexpr auto is_integer_v = is_integer<A>::value;

	template <typename> struct is_floating_point : integral::false_constant {};
	template <> struct is_floating_point<float> : integral::true_constant {};
	template <> struct is_floating_point<double> : integral::true_constant {};
	template <> struct is_floating_point<long double> : integral::true_constant {};
	template <typename A> constexpr auto is_floating_point_v = is_floating_point<A>::value;

	template <typename> struct is_carray : integral::false_constant {};
	template <typename A> struct is_carray<A[]> : integral::true_constant {};
	template <typename A, size_t B> struct is_carray<A[B]> : integral::true_constant {};
	template <typename A> constexpr auto is_carray_v = is_carray<A>::value;
	
	template <typename A> struct is_enum : integral::boolean_constant<__is_enum(A)> {};
	template <typename A> constexpr auto is_enum_v = is_enum<A>::value;

	template <typename A> struct is_union : integral::boolean_constant<__is_union(A)> {};
	template <typename A> constexpr auto is_union_v = is_union<A>::value;

	template <typename A> struct is_class : integral::boolean_constant<__is_class(A)> {};
	template <typename A> constexpr auto is_class_v = is_class<A>::value;

	template <typename A> struct is_function : integral::false_constant {};
	template <typename A, typename ... B> struct is_function<A(B ...)> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ...) volatile> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ...) const> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ...) const volatile> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ...) &> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ...) volatile &> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ...) const &> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ...) const volatile &> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ...) &&> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ...) volatile &&> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ...) const &&> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ...) const volatile &&> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ......)> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ......) volatile> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ......) const> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ......) const volatile> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ......) &> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ......) volatile &> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ......) const &> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ......) const volatile &> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ......) &&> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ......) volatile &&> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ......) const &&> : integral::true_constant {};
	template <typename A, typename ... B> struct is_function<A(B ......) const volatile &&> : integral::true_constant {};

}
}

#endif

