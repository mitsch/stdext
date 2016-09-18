/// @file declval.hpp
/// @module types
/// @library stdext
/// @author Michael Koch
/// @copyright CC BY 3.0

#include <stdext/types/reference.hpp>

#ifndef STDEXT__TYPES__DECLVAL
#define STDEXT__TYPES__DECLVAL

namespace stdext
{
namespace types
{

	template <typename A> typename add_rvalue_reference_t<A> declval ();

}
}

#endif

