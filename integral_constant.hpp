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


	template <size_t N> using index_constant = integral_constant<size_t, N>;

	template <size_t A, char ... Bs> struct index_constant_counter {using type = index_constant<A>;};
	template <size_t A, char B, char ... Bs> struct index_constant_counter : index_constant<A * 10 + (B - '0'), Bs ...> {};

	template <char ... Cs> constexpr auto operator "" _i ()
	{
		return index_constant_counter<0, Cs ...>::type();
	}

	template <size_t A, size_t B> constexpr index_constant<A + b> operator + (index_constant<A>, index_constant<B>)
	{
		return index_constant<A + B>();
	}

	template <typename T> struct is_integral_constant : false_type {};
	template <typename T, T value> struct is_integral_constant<integral_constant<T, value>> : true_type {};

}

#endif

