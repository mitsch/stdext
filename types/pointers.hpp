/// @file pointers.hpp
/// @module types
/// @library stdext
/// @author Michael Koch
/// @copyright CC BY 3.0

#include <stdext/integral/constant.hpp>
#include <stdext/types/primitives.hpp>
#include <stdext/types/references.hpp>

#ifndef STDEXT__TYPES__POINTERS
#define STDEXT__TYPES__POINTERS

namespace stdext
{
namespace types
{

	template <typename> struct is_pointer : integral::false_constant {};
	template <typename A> struct is_pointer<A*> : integral::true_constant {};
	template <typename A> constexpr auto is_pointer_v = is_pointer<A>::value;

	template <typename A> struct remove_pointer {using type = A;};
	template <typename A> struct remove_pointer<A*> {using type = A;};
	template <typename A> struct remove_pointer<A* const> {using type = A;};
	template <typename A> struct remove_pointer<A* volatile> {using type = A;};
	template <typename A> struct remove_pointer<A* const volatile> {using type = A;};
	template <typename A> using remove_pointer_t = typename remove_pointer<A>::type;

	template <typename A, bool = is_function_v<A>> struct __add_pointer {using type = remove_reference_t<A>*;};
	template <typename A> struct __add_pointer<A, true> struct __add_pointer {using type = A;};
	template <typename A, typename ... B> struct __add_pointer<A(B ...), true> {using type = A(*)(B ...);};
	template <typename A, typename ... B> struct __add_pointer<A(B ..., ...), true> {using type = A(*)(B ..., ...);};
	template <typename A> struct add_pointer : __add_pointer<A> {};
	template <typename A> using add_pointer_t = typename add_pointer<A>::type;

}
}

#endif

