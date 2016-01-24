/// @file optional.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_OPTIONAL_HPP__
#define __STDEXT_OPTIONAL_HPP__

#include <stdext/callable.hpp>

namespace stdext
{

	/// Optionally initialised value
	///
	/// The container holds at most one value. It can be used to express that some value might not
	/// be initialised. Such behaviour can be found in C with the null pointer or in other languages
	/// with nil assignment. Rather than defining some extra state in the value to be not initialised,
	/// this state will be excluded and declared through the container.
	template <typename T> class optional
	{
	
		private:

			bool initialised;
			union{T value;};

		public:

			using value_type = T;


			/// Default constructor
			///
			/// Constructs an empty optional container
			constexpr optional () noexcept
				: initialised(false)
			{}

			/// Constructs an initialised optional container with \a value
			optional (T value) noexcept(std::is_nothrow_move_constructible<T>::value)
			{
				new (&value) T(std::move(value));
				initialised = true;
			}

			/// Copy constructor
			///
			/// Constructs a new instance with a copy of the content of \a other
			optional (const optional & other) noexcept(std::is_nothrow_copy_constructible<T>::value)
			{
				if (other.initialised)
					new (&value) T(other.value);
				initialised = other.initialised;
			}

			/// Move constructor
			///
			/// Constructs a new instance with the content of \a other
			optional (optional && other)
			{
				if (other.initialised)
					new (&value) T(std::move(other.value));
				initialised = other.initialised;
			}

			/// Destructs contained value if some is initialised
			~optional ()
			{
				if (initialised)
					value.~T();
			}


			/// Copy assignment
			///
			/// If the container \a other has some value, it will be copied into our own instance; otherwise
			/// our own instance will be empty after the assignment
			optional& operator = (const optional& other)
				noexcept(std::is_nothrow_copy_assignable<T>::value and
				         std::is_nothrow_destructible<T>::value and
				         std::is_nothrow_copy_constructible<T>::value)
			{
				if (initialised and other.initialised)
					value = other.value;
				else if (initialised and not other.initialised)
					value.~T();
				else if (not initialised and other.initialised)
					new (&value) T(other.value);

				initialised = other.initialised;
				return *this;
			}
			
			/// Move assignment
			///
			/// If the container \a other has some value, it will be taken over; otherwise, the own instance
			/// will be empty after the assignment
			optional& operator = (optional && other)
				noexcept(std::is_nothrow_destructible<T>::value and
				         std::is_nothrow_move_assignable<T>::value and
				         std::is_nothrow_move_constructible<T>::value)
			{
				if (initialised and other.initialised)
					value = std::move(other.value);
				else if (initialised and not other.initialised)
					value.~T();
				else if (not initialised and other.initialised)
					new (&value) T(std::move(other.value));

				initialised = other.initialised;
				other.initialised = false;
				return *this;
			}

			/// Assigns \a value to own container
			optional& operator = (T value)
				noexcept(std::is_nothrow_move_assignable<T>::value and std::is_nothrow_move_constructible<T>::value)
			{
				if (initialised)
					this->value = std::move(value);
				else
					new (&value) T(std::move(value));
				this->initialised = true;
				return *this;
			}

			/// Swaps the content between \a first and \a second
			friend void swap (optional & first, optional & second)
			{
				using std::swap;

				if (first.initialised and second.initialised)
					swap(first.value, second.value);
				else if (first.initialised and not second.initialised)
					new (&second.value) T(std::move(first.value));
				else if (not first.initialised and second.initialised)
					new (&first.value) T(std::move(second.value);

				swap(first.initialised, second.initialised);
			}
	

			/// Tests if the container is not initialised
			constexpr bool empty () const
			{
				return not initialised;
			}

			/// Tests if the container is initialised
			constexpr operator bool () const
			{
				return initialised;
			}


			/// Functor operator
			///
			/// If the container has some value initialised, it will call the function object \a f with
			/// the initialised value as parameter. The type of the parameter can be passed by either
			/// value or constant lvalue reference to the function object \a f. The result 
			template <typename F> constexpr auto operator () (F f) const
				requires Callable_<F, T> and not std::is_void<decltype(f(value))>::value
				-> optional<decltype(f(value))>
			{
				using U = decltype(f(value));
				return initialised ? optional<U>(f(value)) : optional<U>();
			}

			template <typename F> friend constexpr auto fmap (F f, optional && o)
				requires Callable_<F, T> and not std::is_void<decltype(f(o.value))>::value
			{
				using U = decltype(f(value));
				return initialised ? optional<U>(f(std::move(o.value))) : optional<U>();
			}

			template <typename F> friend constexpr auto fmap (F f, const optional & o)
				requires Callable_<F, T> and not std::is_void<decltype(f(o.value))>::value
			{
				using U = decltype(f(value));
				return initialised ? optional<U>(f(o.value)) : optional<U>();
			}


			template <typename F, typename G> constexpr auto operator () (F f, G g) const
				requires Callable_<F, T> and not std::is_void<decltype(f(value))>::value and
				         Callable_<G> and not std::is_void<decltype(g())>::value and
				         requires(){std::common_type<decltype(f(value)), decltype(g())>::type}
			{
				return initialised ? f(value) : g();
			}

			template <typename F, typename G> friend constexpr auto decide (F f, G g, const optional & o)
				requires Callable_<F, T> and not std::is_void<decltype(f(value))>::value and
				         Callable_<G> and not std::is_void<decltype(g())>::value and
				         requires(){std::common_type<decltype(f(value)), decltype(g())>::type}
			{
				return o.initialised ? f(o.value) : g();
			}

			template <typename F, typename G> friend constexpr auto decide (F f, G g, optional && o)
				requires Callable_<F, T> and not std::is_void<decltype(f(value))>::value and
				         Callable_<G> and not std::is_void<decltype(g())>::value and
				         requires(){std::common_type<decltype(f(value)), decltype(g())>::type}
			{
				return o.initialised ? f(std::move(o.value)) : g();
			}

	};


	template <typename T> constexpr optional<T> make_optional (T t)
	{
		return optional<T>(std::move(t));
	}

	template <typename T> constexpr optional<T> make_optional (const bool initialise, T && t)
	{
		return initialise ? optional<T>(std::move(t)) : optional<T>();
	}

	template <typename T> constexpr optional<T> make_optional (const bool initialise, const T & t)
	{
		return initialise ? optional<T>(t) : optional<T>();
	}

}

#endif

