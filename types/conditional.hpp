/// @file conditional.hpp
/// @module types
/// @library stdext
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef STDEXT__TYPES__CONDITIONAL
#define STDEXT__TYPES__CONDITIONAL

namespace stdext
{
namespace types
{

	template <bool, typename, typename> struct conditional;
	template <typename A, typename B> struct conditional<true, A, B> {using type = A;};
	template <typename A, typename B> struct conditional<false, A, B> {using type = B;};
	template <bool A, typename B, typename C> using conditional_t = typename conditional<A, B, C>::type;

}
}

#endif

