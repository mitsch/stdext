/// @file type_list.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_TYPE_LIST_HPP__
#define __STDEXT_TYPE_LIST_HPP__

#include <stdext/integral_constant.hpp>

namespace stdext
{

	template <typename ... As>
	struct type_list
	{
		template <typename ... A> using list_appending = type_list<As ..., A ...>;
		template <typename ... A> using list_prepending = type_list<A ..., As ...>;
		template <typename A> using appending = type_list<As ..., A>;
		template <typename A> using prepending = type_list<A, As ...>;
		template <size_t A, typename ... Bs> element_getter;
		template <typename B, typename ... Bs> element_getter<0, B, Bs ...> {using type = B;};
		template <size_t A, typename B, typename ... Bs> element_getter<A, B, Bs ...> : element_getter<A-1, Bs ...> {};
		template <size_t A> using element = typename element_getter<A, As ...>::type;
		
	
		template <typename A>
		constexpr auto append (A value) const
		{
			return type_list<As ..., A>();
		}

		template <typename ... Bs>
		constexpr auto append (type_list<Bs ...> values) const
		{
			return type_list<As ..., Bs ...>();
		}

		template <typename A>
		constexpr auto prepend (A value) const
		{
			return type_list<A, As ...>();
		}

		template <typename ... Bs>
		constexpr auto prepend (type_list<Bs ...> values) const
		{
			return type_list<Bs ..., As ...>();
		}

		template <size_t I>
		constexpr auto get (index_constant<I> index) const
		{
			static_assert(I < sizeof...(As), "index is out of bound");
			template <size_t J, typename ... Bs> struct holder;
			template <typename B, typename ... Bs> struct holder<0, B, Bs ...> {using type = B;};
			template <size_t J, typename B, typename ... Bs> struct holder<J, B, Bs ...> : holder<J-1, Bs ...> {};
			return holder<I, As ...>::type();
		}
	
	};

}

#endif

