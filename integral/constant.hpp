/// @file constant.hpp
/// @module integral
/// @library stdex
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT__INTEGRAL__CONSTANT
#define __STDEXT__INTEGRAL__CONSTANT

#include <stddef.h>

namespace stdext
{
namespace integral
{

	/// Typed integral
	///
	/// Each integral value can be used as a template parameter. The constant is a type which holds
	/// exactly one value of type \a A. Any object is free of any attribute, so it can be optimised
	/// to consume no memory. 
	template <typename A, A B> struct constant
	{

		using value_type = A;
		using type = constant;

		static constexpr A value = B;

		constexpr operator value_type () const noexcept
		{
			return value;
		}

		template <typename ... C>
		constexpr value_type operator () (C && ...) const noexcept
		{
			return value;
		}
	};

	template <bool A> using bool_constant = constant<bool, A>;
	using true_type = bool_constant<true>;
	using false_type = bool_constant<false>;

	template <typename ... A> struct conjunction;
	template <bool A, typename ... B> struct _conjunction;
	template <> struct conjunction<> : true_type {};
	template <typename A, typename ... B> struct conjunction<A, B ...> : _conjunction<A::value, B ...> {};
	template <typename ... A> struct _conjunction<true, A ...> : conjunction<A ...> {};
	template <typename ... A> struct _conjunction<false, A ...> : false_type {};
	template <typename ... A> constexpr auto conjunction_v = conjunction<A ...>::value;

	template <typename ... A> struct disjunction;
	template <bool A, typename ... B> struct _disjunction;
	template <> struct disjunction<> : false_type {};
	template <typename A, typename ... B> struct disjunction<A, B ...> : _disjunction<A::value, B ...> {};
	template <typename ... A> struct _disjunction<true, A ...> : true_type {};
	template <typename ... A> struct _disjunction<false, A ...> : disjunction<A ...> {};
	template <typename ... A> constexpr auto disjunction_v = disjunction<A ...>::value;

	template <typename A> struct negate : bool_constant<not A::value> {};
	template <typename A> auto negate_v = negate<A>::value;


	template <size_t A> using index_constant = constant<size_t, A>;

	template <size_t A, char ... B> struct index_constant_counter {using type = index_constant<A>;};
	template <size_t A, char B, char ... C> struct index_constant_counter<A, B, C ...> : index_constant_counter<A * 10 + (B - '0'), C ...> {};

	template <char ... A> constexpr auto operator "" _i ()
	{
		return index_constant_counter<0, A ...>::type();
	}

	template <size_t A, size_t B> constexpr index_constant<A + B> operator + (index_constant<A>, index_constant<B>)
	{
		return index_constant<A + B>();
	}

	template <typename A> struct is_constant : false_type {};
	template <typename A, A B> struct is_constant<integral_constant<A, B>> : true_type {};

}
}

#endif
