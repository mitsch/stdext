/// @file optional.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_OPTIONAL_HPP__
#define __STDEXT_OPTIONAL_HPP__

#include <stdext/callable.hpp>

namespace stdext
{

	struct {} nullopt;


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

			/// Empty constructor
			///
			/// Constructs an empty optional container
			constexpr optional (nullopt) noexcept
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

			constexpr void clean ()
			{
				if (initialised)
					value.~T();
				initialised = false;
			}



			template <typename C, typename ... As>
				requires Callable_<C, T, As ...> and
				         not std::is_void<std::result_of_t<C(T, As ...)>>::value
			constexpr auto map (C mapper, As ... arguments) const
			{
				using U = std::result_of_t<C(T, As ...)>;
				return initialised ? optional<U>(mapper(value, std::move(arguments) ...)) : optional<U>();
			}

			template <typename C, typename ... As>
				requires Callable_<C, T, As ...> and
				         not std::is_void<std::result_of_t<C(T&, As ...)>>::value
			constexpr auto map (C mapper, As ... arguments)
			{
				using U = std::result_of_t<C(T&, As ...)>;
				return initialised ? optional<U>(mapper(value, std::move(arguments) ...)) : optional<U>();
			}

			template <typename C, typename ... As>
				requires Callable_<C, T, As ...> and
				         not std::is_void<std::result_of_t<C(T, As ...)>>::value
			friend constexpr auto fmap (C mapper, optional && value, As ... arguments) const
			{
				using U = std::result_of_t<C(T, As ...)>;
				const auto initialised = value.initialised;
				value.initialised = false;
				return initialised ? optional<U>(mapper(std::move(value.value), std::move(arguments) ...)) : optional<U>();
			}

			template <typename C, typename ... As>
				requires Callable_<C, T, As ...> and
				         not std::is_void<std::result_of_t<C(T, As ...)>>::value
			friend constexpr auto fmap (C mapper, const optional & value, As ... arguments) const
			{
				using U = std::result_of_t<C(T, As ...)>;
				return value.initialised ? optional<U>(mapper(value.value, std::move(arguments) ...)) : optional<U>();
			}


		

			template <typename H, typename M, typename ... As>
				requires Callable_<H, T&, As ...> and
				         Callable_<M, As ...> and
				         not std::is_void_v<std::result_of_t<H(T&, As ...)>> and
				         not std::is_void_v<std::result_of_t<M(As ...)>> and
				         requires(){typedef std::common_type<std::result_of_t<H(T&, As ...)>, std::result_of_t<M(As ...)>>::type;}
			constexpr auto decide (H hitter, M misser, As ... arguments)
			{
				return initialised ? hitter(value, std::move(arguments) ...) : misser(std::move(arguments) ...);
			}
			
			template <typename H, typename M, typename ... As>
				requires Callable_<H, T, As ...> and
				         Callable_<M, As ...> and
				         not std::is_void_v<std::result_of_t<H(T, As ...)>> and
				         not std::is_void_v<std::result_of_t<M(As ...)>> and
				         requires(){typedef std::common_type<std::result_of_t<H(T, As ...)>, std::result_of_t<M(As ...)>>::type;}
			constexpr auto decide (H hitter, M misser, As ... arguments) const
			{
				return initialised ? hitter(value, std::move(arguments) ...) : misser(std::move(arguments) ...);
			}

			template <typename H, typename M, typename ... As>
				requires Callable_<H, T, As ...> and
				         Callable_<M, As ...> and
				         not std::is_void_v<std::result_of_t<H(T, As ...)>> and
				         not std::is_void_v<std::result_of_t<M(As ...)>> and
				         requires(){typedef std::common_type<std::result_of_t<H(T, As ...)>, std::result_of_t<M(As ...)>>::type;}
			friend constexpr auto decide (H hitter, M misser, const optional & opt, As ... arguments)
			{
				return opt.initialised ? hitter(opt.value, std::move(arguments) ...) : misser(std::move(arguments) ...);
			}

			template <typename H, typename M, typename ... As>
				requires Callable_<H, T, As ...> and
				         Callable_<M, As ...> and
				         not std::is_void_v<std::result_of_t<H(T, As ...)>> and
				         not std::is_void_v<std::result_of_t<M(As ...)>> and
				         requires(){typedef std::common_type<std::result_of_t<H(T, As ...)>, std::result_of_t<M(As ...)>>::type;}
			friend constexpr auto decide (H hitter, M misser, optional && opt, As ... arguments)
			{
				return opt.initialised ? hitter(std::move(opt.value), std::move(arguments) ...) : misser(std::move(arguments) ...);
			}




			template <typename B, typename ... As>
				requires Callable_<B, T, As ...> and
				         is_tuple_v<std::result_of_t<B(T, As ...)>>
			constexpr auto bind (B binder, As ... arguments) const
			{
				using U = std::result_of_t<B(T, As ...)>;
				return initialised ? binder(value, std::move(arguments) ... ) : U();
			}

			template <typename B, typename ... As>
				requires Callable_<B, T&, As ...> and
				         is_tuple_v<std::result_of_t<B(T&, As ...)>>
			constexpr auto bind (B binder, As ... arguments)
			{
				using U = std::result_of_t<B(T&, As ...)>;
				return initialised ? binder(value, std::move(arguments) ... ) : U();
			}

			template <typename B, typename ... As>
				requires Callable_<B, T, As ...> and
				         is_tuple_v<std::result_of_t<B(T, As ...)>>
			friend constexpr auto bind (B binder, const optional & opt, As ... arguments)
			{
				using U = std::result_of_t<B(T, As ...)>;
				return opt.initialised ? binder(opt.value, std::move(arguments) ... ) : U();
			}

			template <typename B, typename ... As>
				requires Callable_<B, T, As ...> and
				         is_tuple_v<std::result_of_t<B(T, As ...)>>
			friend constexpr auto bind (B binder, optional && opt, As ... arguments)
			{
				using U = std::result_of_t<B(T, As ...)>;
				return opt.initialised ? binder(std::move(opt.value), std::move(arguments) ... ) : U();
			}

			



			friend constexpr optional mflatten (optional<optional> value)
			{
				return mbind([](auto v){return v;}, std::move(value));
			}





			friend constexpr optional bind_while (Callable<optional, T> mapper, optional value)
			{
				while (value.initialised)
					value = mapper(std::move(value.value));
				return value;
			}

			friend constexpr optional bind_while (Callable<bool, T> predicter, Callable<optional, T> mapper, optional value)
			{
				while (value.initialised and predicter(static_cast<const T&>(value.value)))
					value = mapper(std::move(value.value));
				return value;
			}

			template <typename C, typename ... Ts>
				requires Callable_<C, Ts ...> and
		             not std::is_void_v<std::result_of_t<C(Ts ...)>>
			friend constexpr auto join_fmap (C mapper, optional<Ts> values ...)
			{
				using U = std::result_of_t<C(Ts ...)>;
				const auto initialised = indices.fold([initialisations=std::make_tuple(values.initialised, ...)]
					(auto flag, auto index)
				{
					return flag and std::get<index>(initialisations);
				}, true);
				return initialised ? optional<U>(mapper(std::move(values.value) ...)) : optional<U>();
			}

			template <typename M, typename D, typename ... Ts>
				requires Callable_<M, Ts ...> and
				         Callable_<D> and
		             not std::is_void_v<std::result_of_t<M(Ts ...)>> and
								 not std::is_void_v<std::result_of_t<D()>> and
								 requires(){std::common_type<std::result_of_t<M(Ts ...)>, std::result_of_t<D()>>::type}
			friend constexpr auto join_decide (M mover, D defaulter, optional<Ts> values ...)
			{
				using U = std::common_type_t<std::result_of_t<M(Ts ...)>, std::result_of_t<D()>>;
				const auto initialised = indices.fold([initialisations=std::make_tuple(values.initialised, ...)]
					(auto flag, auto index)
				{
					return flag and std::get<index>(initialisations);
				}, true);
				return initialised ? mover(std::move(values.value) ...)) : defaulter();
			}
			
			template <typename C, typename ... Ts>
				requires Callable_<C, Ts ...> and
		             is_tuple_v<std::result_of_t<C(Ts ...)>>
			friend constexpr auto join_sequence (C mapper, optional<Ts> values ...)
			{
				using U = std::result_of_t<C(Ts ...)>;
				const auto initialised = indices.fold([initialisations=std::make_tuple(values.initialised, ...)]
					(auto flag, auto index)
				{
					return flag and std::get<index>(initialisations);
				}, true);
				return initialised ? mapper(std::move(values.value) ...) : U();
			}

	;


	template <typename T> constexpr optional<T> make_optional (T t)
	{
		return optional<T>(std::move(t));
	}

	template <typename T> constexpr optional<T> make_optional_if (const bool conditional, T object)
	{
		return conditional ? optional<T>(std::move(object)) : optional<T>();
	}

	template <typename C, typename ... Args>
		requires Callable_<C, Args ...>
	constexpr auto make_optional (const bool initialise, C create, Args ... args)
	{
		using type = decltype(create(std::move(args) ...));
		return initialise ? optional<type>(create(std::move(args) ...)) : optional<type>();
	}

	




	template <typename T> struct is_optional : false_type {};
	template <typename T> struct is_optional<const T> : is_optional<T> {};
	template <typename T> struct is_optional<volatile T> : is_optional<T> {};
	template <typename T> struct is_optional<optional<T>> : true_type {};

}

#endif

