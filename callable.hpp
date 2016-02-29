/// @file callable.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_CALLABLE_HPP__
#define __STDEXT_CALLABLE_HPP__

namespace stdext
{


	template <typename C, typename R, typename ... Args> concept bool Callable = requires ()
	{
		return requires(C c, Args ... args)
		{
			{c(args ...)} -> R
		};
	}
	
	template <typename C, typename ... Args> concept bool Callable_ = requires ()
	{
		return require(C c, Args ... args)
		{
			c(args ...)
		};
	}

	template <typename T>
	struct result_of
	{};

	template <typename T, typename ... Args>
	struct result_of<T(Args ...)>
	{
		template <typename U> static U&& invoke ();
		using type = decltype(invoke<T>()(invoke<Args>() ...));
	};

}

#endif
