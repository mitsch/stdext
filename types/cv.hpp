/// @file cv.hpp
/// @module types
/// @library stdext
/// @author Michael Koch
/// @copyright CC BY 3.0

#include <stdext/integral/constant.hpp>

#ifndef STDEXT__TYPES__CV
#define STDEXT__TYPES__CV

namespace stdext
{
namespace types
{

	template <typename> is_const : integral::false_constant {};
	template <typename A> is_const<A const> : integral::true_constant {};
	template <typename A> constexpr auto is_const_v = is_const<A>::value;

	template <typename> is_volatile : integral::false_constant {};
	template <typename A> is_volatile<A volatile> : integral::true_constant {};
	template <typename A> constexpr auto is_volatile_v = is_volatile<A>::value;

	template <typename A> struct remove_const {using type = A;};
	template <typename A> struct remove_const<A const> {using type = A;};
	template <typename A> using remove_const_t = typename remove_const<A>::type;

	template <typename A> struct remove_volatile {using type = A;};
	template <typename A> struct remove_volatile<A volatile> {using type = A;};
	template <typename A> using remove_volatile_t = typename remove_volatile<A>::type;

	template <typename A> struct remove_cv {using type = remove_volatile_t<remove_const_t<A>>;};
	template <typename A> using remove_cv_t = typename remove_cv<A>::type;

	template <typename A> struct add_const {using type = A const;};
	template <typename A> using add_const_t = typename add_const<A>::type;

	template <typename A> struct add_volatile {using type = A volatile;};
	template <typename A> using add_volatile_t = typename add_volatile<A>::type;

	template <typename A> struct add_cv {using type = add_volatile_t<add_const_t<A>>;};
	template <typename A> using add_cv_t = typename add_cv<A>::type;

}
}

#endif

