/// @file array_view.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_ARRAY_VIEW_HPP__
#define __STDEXT_ARRAY_VIEW_HPP__

#include <stdext/callable.hpp>
#include <stdext/sequence.hpp>
#include <stdext/optional.hpp>
#include <tuple>
#include <cstdlib>

namespace stdext
{

	/// View on an arbitrary array
	///
	/// The view on an array is a lightweighted object which does not manage the
	/// underlying memory and its
	template <typename T> class array_view
	{
	
		private:

			T * values;
			std::size_t length;

		public:

			using value_type = T;

			constexpr array_view () noexcept;
			constexpr array_view (const array_view &) noexcept;
			constexpr array_view (const T * values, const std::size_t count) noexcept;
			
			~array_view () noexcept = default;
	
			constexpr array_view& operator = (const array_view &) noexcept = default;
			


			constexpr bool empty () const noexcept
			{
				return length == 0;
			}

			constexpr operator bool () const noexcept
			{
				return length > 0;
			}

			constexpr std::size_t length () const noexcept
			{
				return length;
			}




			friend void swap (array_view & first, array_view & second)
			{
				using std::swap;
				swap(first.values, second.values);
				swap(first.length, second.length);
			}

			constexpr optional<std::tuple<T, array_view>> operator () () const
			{
				return make_optional(length > 0, [&](){return std::make_tuple(*values, array_view(values+1, length-1));});
			}

			template <typename V, Callable<V, V, T> C> constexpr V fold (C combine, V value) const
			{
				for (std::size_t i = 0; i < length; ++i)
					value = combine(std::move(value), values[i]);
				return value;
			}

			template <typename V, Callable<std::tuple<V, bool>, V, T> C> constexpr V fold (C combine, V value) const
			{
				bool keepFolding = true;
				for (std::size_t i = 0; keepFolding and i < length; ++i)
					std::tie(value, keepFolding) = combine(std::move(value), values[i]);
				return value;
			}

			template <typename V, Callable<V, V, T> C> constexpr V fold_reverse (C combine, V value) const
			{
				std::size_t i = length;
				while (i > 0)
					value = combine(std::move(value), values[--i]);
				return value;
			}

			template <typename V, Callable<std::tuple<V, bool>, V, T> C> constexpr V fold_reverse (C combine, V value) const
			{
				std::size_t i = length;
				bool keepFolding = true;
				while (keepFoldingi > 0)
					std::tie(value, keepFolding) = combine(std::move(value), values[--i]);
				return value;
			}








			template <Callable<T> S> void apply (S set)
			{
				for (std::size_t i = 0; i < length; ++i)
					values[i] = set();
			}

			template <typename C, Callable<std::tuple<T, C>, C> S> C apply (S set, C counter)
			{
				for (std::size_t i = 0; i < length; ++i)
					std::tie(values[i], counter) = set(std::move(counter));
				return counter;
			}

			template <typename C, Callable<std::tuple<T, C>, T, C> S> C apply (S set, C counter)
			{
				for (std::size_t i = 0; i < length; ++i)
					std::tie(values[i], counter) = set(std::move(values[i]), std::move(counter));
				return counter;
			}

			template <typename C, Callable<std::tuple<T, C, bool>, T, C> S> C apply (S set, C counter)
			{
				bool keepSetting = true;
				for (std::size_t i = 0; keepSetting and i < length; ++i)
					std::tie(values[i], counter, keepSetting) = set(std::move(values[i]), std::move(counter));
				return counter;
			}


			template <typename C, Callable<std::tuple<T, C>, C> S> C apply_reverse (S set, C counter)
			{
				std::size_t i = length;
				while (i > 0)
					std::tie(values[--i], counter) = set(std::move(counter));
				return counter;
			}

			template <typename C, Callable<std::tuple<T, C>, T, C> S> C apply_reverse (S set, C counter)
			{
				std::size_t i = length;
				while (i > 0)
					--i, std::tie(values[i], counter) = set(std::move(values[i]), std::move(counter));
				return counter;
			}

			template <typename C, Callable<std::tuple<T, C, bool>, T, C> S> C apply_reverse (S set, C counter)
			{
				std::size_t i = length;
				bool keepSetting = true;
				while (keepSetting and i > 0)
					--i, std::tie(values[i], counter, keepSetting) = set(std::move(values[i]), std::move(counter));
				return counter;
			}

			template <BoundedSequence<T> S> S apply (S elements);
			template <BoundedSequence<T> S> S apply_reverse (S elements);
			template <UnboundedSequence<T> S> S apply (S elements);
			template <UnboundedSequence<T> S> S apply_reverse (S elements);


	};

}

#endif

