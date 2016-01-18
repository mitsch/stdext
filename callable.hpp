/// @file callable.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_CALLABLE_HPP__
#define __STDEXT_CALLABLE_HPP__

#include <type_traits>

namespace stdext
{

	template <typename C, typename R, typename ... Args> concept bool Callable = require ()
	{
		return require(C c, Args ... args)
		{
			c(args ...),
			std::is_convertible<decltype(c(args ...)), R>::value
		};
	}
	
	template <typename C, typename ... Args> concept bool Callable_ = require ()
	{
		return require(C c, Args ... args)
		{
			c(args ...)
		};
	}

}

#endif
