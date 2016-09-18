/// @file relationships.hpp
/// @module types
/// @library stdext
/// @author Michael Koch
/// @copyright CC BY 3.0

#include <stdext/integral/constant.hpp>

#ifndef STDEXT__TYPES__RELATIONSHIPS
#define STDEXT__TYPES__RELATIONSHIPS

namespace stdext
{
namespace types
{

	template <typename A, typename B> struct is_same : integral::false_constant {};
	template <typename A> struct is_same<A, A> : integral::true_constant {};
	template <typename A, typename B> constexpr auto is_same_v = is_same<A, B>::value;

	template <typename A, typename B> struct is_base_of

}
}

#endif

