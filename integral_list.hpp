/// @file integral_list.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_INTEGRAL_LIST_HPP__
#define __STDEXT_INTEGRAL_LIST_HPP__

#include <stdext/callable.hpp>
#include <stdext/integral_constant.hpp>

namespace stdext
{

	/// List of integrals
	///
	/// The list contains values of type \a A without direct initialisation.
	template <typename A, A ... B>
	struct integral_list
	{
		
		template <typename C, typename ... D> struct appender;
		template <A ... C, A D, typename  ... E> struct appender<integral_list<A, C ...>, integral_constant<A, D>, E ...> : appender<integral_list<A, C ..., D>, E ...> {};
		template <A ... C, A ... D, typename ... E> struct appender<integral_list<A, C ...>, integral_list<A, D ...>, E ...> : appender<integral_list<A, C ..., D ...>, E ...> {};
		template <A ... C> struct appender<integral_list<A, C ...>> {using type = integral_list<A, C ....>;};
		template <typename ... C> using appending = typename appender<integral_list<A, B ...>, C ...>::type;


		template <typename C, typename D, A E> struct one_inserter;
		template <A C, A ... D, A ... E> struct one_inserter<integral_list<A, C, D ...>, integral_list<A, E ...>, C> {using type = integral_list<A, E ..., C, D ...>;};
		template <A C, A ... D, A ... E, A F> requires C < F struct one_inserter<integral_list<A, C, D ...>, integral_list<A, E ...>, A F> : one_inserter<integral_list<A, D ...>, integral_list<A, E ..., C>, F> {};
		template <A C, A ... D, A ... E, A F> requires C > F struct one_inserter<integral_list<A, C, D ...>, integral_list<A, E ...>, A F> {using type = integral_list<A, E ..., F, C, D ...>;};
		template <A ... C, A D> struct one_inserter<integral_list<A>, integral_list<A, C ...>, A D> {using type = integral_list<A, C ..., D>;};



		/// 
		template <typename V, typename C, typename ... Args>
			requires conjunction_v<bool_constant<Callable<C, V, integral_constant<T, Vs>, const Args & ...>> ...>
		constexpr V fold (C combine, V value, const Args & ... args) const
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

		/// Applies all values in \a Vs and all values in \a args as parameters on functor object \a f
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

		template <T a> constexpr is_element (integral_constant<T, a> value) const
		{
			template <T ... t> struct holder
			{
				static constexpr auto is_element (integral_constant<T, a> value) const {return false_type();}
			};
			template <T h, T ... t> struct holder<h, t ...>
			{
				static constexpr auto is_element (integral_constant<T, a> value) const {return holder<t...>::is_element(value);}
			};
			template <T ... t> struct holder<a, t ...>
			{
				static constexpr auto is_element (integral_constant<T, a> value) const {return true_type();}
			};
			return holder<Vs ...>::is_element(value);
		}

		template <T t> constexpr auto append (integral_constant<T t> value) const
		{
			return integral_list<T, Vs ..., t>();
		}

		template <T ... ts> constexpr auto append (integral_list<T, ts ...> values) const
		{
			return integral_list<T, Vs ..., ts ...>();
		}

		template <T t> constexpr auto prepend (integral_constant<T t> value) const
		{
			return integral_list<T, t, Vs ...>();
		}

		template <T ... ts> constexpr auto prepend (integral_list<T, ts ...> values) const
		{
			return integral_list<T, ts ..., Vs ...>();
		}

		constexpr auto reverse () const
		{
			template <T ... as> struct holder;
			template <T a, T ... as> struct holder<a, as ...>
			{
				template <T ... bs>
				constexpr auto reverse (integral_list<T, bs ...> stack) const
				{return holder<as ...>::reverse(stack.append(integral_constant<T, a>()));}
			};
			template <> struct holder<>
			{
				template <T ... bs>
				constexpr auto reverse (integral_list<T, bs ...> stack) const
				{return stack;}
			};
			return holder<Vs ...>::reverse(integral_list<T>());
		}

		template <T t>
		constexpr auto insert (integral_constant<T, t> value) const
		{
			template <T ... as> struct holder
			{
				template <T ... bs>
				constexpr auto insert (integral_constant<T, t> value, integral_list<T, bs ...> stack) const
				{return stack.append(value);}
			};
			template <T a, T ... as> struct holder<a, as ...>
			{
				template <T ... bs>
				constexpr auto insert (integral_constant<T, t> value, integral_list<T, bs ...> stack, false_type) const
				{return holder<as ...>::insert(value, stack.append(integral_constant<T, a>()));}

				template <T ... bs>
				constexpr auto insert (integral_constant<T, t> value, integral_list<T, bs ...> stack, true_type) const
				{return stack.append(value).append(integral_list<T, bs ...>());}

				template <T ... bs>
				constexpr auto insert (integral_constant<T, t> value, integral_list<T, bs ...> stack) const
				{return insert(value, stack, boolean_type<t < a>());}
			};
			template <T ... as> struct holder<t, as ...>
			{
				template <T ... bs>
				constexpr auto insert (integral_constant<T, t> value, integral_list<T, bs ...> stack) const
				{return *this;}
			};
			return holder<Vs ...>::insert(value, integral_list<T>());
		}
	};



	template <std::size_t ... indices> using index_list = integral_list<std::size_t, indices ...>;

	template <typename T, T n, T o, T ... ts> struct _integral_list_maker : _integral_list_maker<T, n - 1, o + 1, ts ..., o> {};
	template <typename T, T o, T ... ts> struct _integral_list_maker<T, 0, o, ts ...> {using type = integral_list<T, ts ...>;};
	template <typename T, T n, T o = 0> using make_integral_list = _integral_list_maker<T, n, o>::type;

	template <std::size_t n, std::size_t o = 0> using make_index_list = make_integral_list<std::size_t, n, o>;

	template <typename ... Ts> using index_list_for = make_index_list<sizeof...(Ts), 0>;

}

#endif

