/// @file integral_constant.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_INTEGRAL_CONSTANT_HPP__
#define __STDEXT_INTEGRAL_CONSTANT_HPP__

namespace stdext
{

	/// Typed value
	template <typename T, T V> struct integral_constant
	{
	
		using value_type = T;
		using type = typed_value;

		static constexpr T value = V;

		constexpr operator value_type () const noexcept
		{
			return value;
		}

		template <typename ... Ts>
		constexpr value_type operator () (Ts && ...) const noexcept
		{
			return value;
		}
	};


	template <bool value> using bool_constant = integral_constant<bool, value>;
	using true_type = bool_constant<true>;
	using false_type = bool_constant<false>;

	template <typename ... Ts> struct conjunction;
	template <> struct conjunction<> : true_type {};
	template <typename T, typename ... Ts> struct conjunction<T, Ts ...> : _conjunction<T::value, Ts ...> {};
	template <bool v, typename ... Ts> struct _conjunction;
	template <typename ... Ts> struct _conjunction<true, Ts ...> : conjunction<Ts ...> {};
	template <typename ... Ts> struct _conjunction<false, Ts ...> : false_type {};
	template <typename ... Ts> auto conjunction_v = conjunction<Ts ...>::value;

	template <typename ... Ts> struct disjunction;
	template <> struct disjunction<> : false_type {};
	template <typename T, typename ... Ts> struct disjunction<T, Ts ...> : _disjunction<T::value, Ts ...> {};
	template <bool v, typename ... Ts> struct _disjunction;
	template <typename ... Ts> struct _disjunction<true, Ts ...> : true_type {};
	template <typename ... Ts> struct _disjunction<false, Ts ...> : disjunction<Ts ...> {};
	template <typename ... Ts> auto disjunction_v = disjunction<Ts ...>::value;

	template <typename T> struct negate : bool_constant<not T::value> {};
	template <typename T> auto negate_v = negate<T>::value;


	template <std::size_t N> using index_constant = integral_constant<std::size_t, N>;

}

#endif

