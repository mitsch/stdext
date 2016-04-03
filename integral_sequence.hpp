/// @file integral_sequence.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __INTEGRAL_SEQUENCE_HPP__
#define __INTEGRAL_SEQUENCE_HPP__

#include <stdext/callable.hpp>
#include <stdext/integral_constant.hpp>

namespace stdext
{

	/// Integral sequence
	///
	/// A sequence contains values of some integral type. 
	template <typename T, T ... Vs> struct integral_sequence
	{

		using value_type = T;

		constexpr std::size_t length () const
		{
			return sizeof...(Vs);
		}

		/// 
		template <typename V, typename C, typename ... Args> constexpr V fold (
			C combine, V value, const Args & ... args) const
			require conjunction_v<bool_constant<Callable<C, V, integral_constant<T, Vs>, const Args & ...>> ...>
		{
			// holder is a meta structure to traverse through the sequence
			template <typename U, U ... us> struct holder;
			template <typename U, U u, U ... us> struct holder<U, u, us ...>
			{
				constexpr V fold (C combine, V value, const Args & ... args) const
				{
					value = combine(std::move(value), integral_constant<U, u>(), args ...);
					return holder<U, us ...>().fold(std::move(combine), std::move(value), args ...);
				}
			};
			template <typename U> struct holder<U>
			{
				constexpr V fold (C combine, V value, const Args & ... args) const
				{
					return value;
				}
			};

			return holder<T, Vs ...>().fold(std::move(combine), std::move(value), args ...);
		}

		template <typename C, typename V>
		constexpr auto compose (C caller, V init) const
		{
			template <typename U, U ... us> struct holder;
			template <typename U, U u, U ... us> struct holder<U, u, us ...>
			{
				constexpr auto compose (auto value) const
				{
					const auto nextHolder = holder<U, us ...>();
					return nextHolder.compose(caller(std::move(value), integral_constant<U, u>()));
				}
			};
			template <typename U> struct holder<U>
			{
				constexpr auto compose (auto value) const
				{
					return value;
				}
			};

			return holder<T, Vs ...>().compose(std::move(init));
		}

		/// Applies all values in \a Vs and all values in \a args as parameters on functor object \a f
		/// in following order: first, all values in \a args and then all values of \a Vs are positioned.
		/// The values of \a Vs are embedded in instances of typed_value<T>.
		/// \code
		/// typed_sequence<std::size_t, 0, 1, 2> s;
		/// struct foo {};
		/// bool empty = s.apply([](foo f, auto ... indices){return sizeof...(indices) == 0;}, foo());
		/// \endCode
		template <typename F, typename ... Args> constexpr auto apply (F f, Args && ... args) const
		{
			return f(std::forward<Args>(args) ..., integral_constant<T, Vs>() ...);
		}

		/// Applie all values in \a Vs and all values in \a args as parameters on functor object \a f
		/// in following order: first, all values of \a Vs and then all values in \a args are positioned.
		/// The values of \a Vs are embedded in instances of typed_values<T>.
		/// \code
		/// typed_sequence<bool, true, false, false, true> s;
		/// bool andResult = s.apply([](auto ... typedBools)
		/// {
		///   return conjunction_v<typedBools ...>;
		/// }, typed_false(), typed_true());
		/// \endcode
		template <typename F, typename ... Args> constexpr auto apply_ (F f, Args && ... args) const
		{
			return f(integral_constant<T, Vs>() ..., std::forward<Args>(args) ...);
		}



	};

	/// Concatenates two integral sequences \a first and \a second
	template <typename T, T ... As, T ... Bs> constexpr integral_sequence<T, As ..., Bs ...> concat (
		const integral_sequence<T, As ...> first, const integral_sequence<T, Bs ...> second)
	{
		return integral_sequence<T, As ..., Bs ...>();
	}

	/// Prepends an integral constant \a first to an integral sequence \a second
	template <typename T, T A, T ... Bs> constexpr integral_sequence<T, A, Bs ...> concat (
		const integral_constant<T, A> first, const integral_sequence<T, Bs ...> second)
	{
		return integral_sequence<T, A, Bs ...>();
	}

	/// Appends an integral constant \a second to an integral sequence \a second
	template <typename T, T ... As, T B> constexpr integral_sequence<T, As ..., B> concat (
		const integral_sequence<T, As ...> first, const integral_constant<T, B> second)
	{
		return integral_sequence<T, As ..., B>();
	}

	/// Reverts elements in the integral sequence of \a s
	template <typename T, T ... Ts> constexpr auto revert (const integral_sequence<T, Ts ...> s)
	{
		template <typename H, T ... Us> struct _stacker;
		template <T ... Hs, T U, T ... Us> struct _stacker<integral_sequence<T, Hs ...>, U, Us ...>
			: _stacker<integral_sequence<T, U, Hs ...>, Us ...> {};
		template <T ... Hs> struct _stacker<integral_sequence<T, Hs ...>>
			{using type = integral_sequence<T, Hs ...>;};
		return _stacker<integral_sequence<T>, Ts ...>::type();
	}

	template <std::size_t ... indices> using index_sequence = integral_sequence<std::size_t, indices ...>;

	template <typename T, T n, T o, T ... ts> struct _integral_sequence_maker
		: _integral_sequence_maker<T, n - 1, o + 1, ts ..., o> {};
	template <typename T, T o, T ... ts> struct _integral_sequence_maker<T, 0, o, ts ...>
		{using type = integral_sequence<T, ts ...>;};
	template <typename T, T n, T o = 0> using make_integral_sequence = _integral_sequence_maker<T, n, o>::type;

	template <std::size_t n, std::size_t o = 0> using make_index_sequence = make_integral_sequence<std::size_t, n, o>;

	template <typename ... Ts> using index_sequence_for = make_index_sequence<sizeof...(Ts), 0>;

}

#endif

