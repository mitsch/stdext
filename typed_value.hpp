/// @file typed_value.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_TYPED_VALUE_HPP__
#define __STDEXT_TYPED_VALUE_HPP__

namespace stdext
{

	/// Typed value
	template <typename T, T V> struct typed_value
	{
	
		using value_type = T;
		using type = typed_value;

		static constexpr T value = V;

		constexpr operator value_type () const noexcept
		{
			return value;
		}

		constexpr value_type operator () () const noexcept
		{
			return value;
		}
	};


	template <bool value> using typed_bool = typed_value<bool, value>;
	using typed_true = typed_bool<true>;
	using typed_false = typed_bool<false>;

	template <typename ... Ts> struct conjunction;
	template <> struct conjunction<> : typed_true {};
	template <typename T, typename ... Ts> struct conjunction<T, Ts ...> : _conjunction<T::value, Ts ...> {};
	template <bool v, typename ... Ts> struct _conjunction;
	template <typename ... Ts> struct _conjunction<true, Ts ...> : conjunction<Ts ...> {};
	template <typename ... Ts> struct _conjunction<false, Ts ...> : typed_false {};
	template <typename ... Ts> auto conjunction_v = conjunction<Ts ...>::value;

	template <typename ... Ts> struct disjunction;
	template <> struct disjunction<> : typed_false {};
	template <typename T, typename ... Ts> struct disjunction<T, Ts ...> : _disjunction<T::value, Ts ...> {};
	template <bool v, typename ... Ts> struct _disjunction;
	template <typename ... Ts> struct _disjunction<true, Ts ...> : typed_true {};
	template <typename ... Ts> struct _disjunction<false, Ts ...> : disjunction<Ts ...> {};
	template <typename ... Ts> auto disjunction_v = disjunction<Ts ...>::value;

	template <typename T> struct negate : typed_bool<not T::value> {};
	template <typename T> auto negate_v = negate<T>::value;

}

#endif

