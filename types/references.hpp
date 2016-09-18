/// @file references.hpp
/// @module types
/// @library stdext
/// @author Michael Koch
/// @copyright CC BY 3.0

#include <stdext/integral/constant.hpp>

#ifndef STDEXT__TYPES__REFERENCES
#define STDEXT__TYPES__REFERENCES

namespace stdext
{
namespace types
{

	template <typename> struct is_lvalue_reference : integral::false_constant {};
	template <typename A> struct is_lvalue_reference<A&> : integral::true_constant {};
	template <typename A> constexpr auto is_lvalue_reference_v = is_lvalue_reference<A>::value;

	template <typename> struct is_rvalue_reference : integral::false_constant {};
	template <typename A> struct is_rvalue_reference<A&&> : integral::true_constant {};
	template <typename A> constexpr auto is_rvalue_reference_v = is_rvalue_reference<A>::value;

	template <typename> struct is_reference : integral::false_constant {};
	template <typename A> struct is_reference<A&> : integral::true_constant {};
	template <typename A> struct is_reference<A&&> : integral::true_constant {};
	template <typename A> constexpr auto is_reference_v = is_reference<A>::value;

	template <typename A> struct remove_reference {using type = A;};
	template <typename A> struct remove_reference<A&> {using type = A;};
	template <typename A> struct remove_reference<A&&> {using type = A;};
	template <typename A> using remove_reference_t = typename remove_reference<A>::type;


	template <typename A> __is_referenceable : integral::negation<integral::disjunction<is_void<A>, integral::conjunction<is_function<A>, integral::disjunction<is_const<A>, is_volatile<A>, is_reference<A>>>>> {};

	template <typename A, bool = __is_referenceable<A>::value> struct __add_lvalue_reference {using type = A;};
	template <typename A> struct __add_lvalue_reference<A, true> {using type = A&;};
	template <typename A> struct add_lvalue_reference : __add_lvalue_reference<A> {};
	template <typename A> using add_lvalue_reference_t = typename add_lvalue_reference<A>::type;

	template <typename A, bool = __is_referenceable<A>::value> struct __add_rvalue_reference {using type = A;};
	template <typename A> struct __add_rvalue_reference<A, true> {uing type = A&&;};
	template <typename A> struct add_rvalue_reference : __add_rvalue_reference<A> {};
	template <typename A> using add_rvalue_reference_t = typename add_rvalue_reference<A>::type;

}
}

#endif

