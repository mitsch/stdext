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
	/// The view on an array is a lightweighted object which does not manage the underlying memory
	/// and its allocation or deallocation.
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





			template <typename S>
				requires Callable<



			/// Calls function object \a set for each element in the view and assigns the result to each
			/// respective element in the view
			template <typename S>
				requires Callable<S, T>
			void apply (S set)
			{
				for (std::size_t i = 0; i < length; ++i)
					values[i] = set();
			}


			/// Calls function object \a set for each element in the view starting with the first one and
			/// traversing through the view
			///
			/// @param counter  mutable value which will be passed along the traversion
			/// @param set      asd
			/// @param sequence 
			///
			/// @return value of \a counter variable after traversion of all elements
			///
			/// @{

			/// @param set function object called for each element with \a counter value; the returning
			///            value is a tuple with the new value for the respective element and a new value
			///            for the \a counter variable
			template <typename C, typename S>
				requires Callable<S, std::tuple<T, C>, C>
			C apply (S set, C counter)
			{
				for (std::size_t i = 0; i < length; ++i)
					std::tie(values[i], counter) = set(std::move(counter));
				return counter;
			}

			/// @param set function object called for each element with the value of the respective
			///            element and the \a counter value; the returning value is a tuple with the new
			///            value for the respective element and a new value for the \a counter variable
			template <typename C, typename S>
				requires Callable<S, std::tuple<T, C>, T, C>
			C apply (S set, C counter)
			{
				for (std::size_t i = 0; i < length; ++i)
					std::tie(values[i], counter) = set(std::move(values[i]), std::move(counter));
				return counter;
			}

			/// @param set function object called for each element with the value of the respective
			///            element and the \a counter value; the returning value is a tuple with the new
			///            value for the respective element, a new value for the \a counter variable and
			///            a boolean flag which indicates if the traversion should be continued
			template <typename C, typename S>
				requires Callable<S, std::tuple<T, C, bool>, T, C>
			C apply (S set, C counter)
			{
				bool keepSetting = true;
				for (std::size_t i = 0; keepSetting and i < length; ++i)
					std::tie(values[i], counter, keepSetting) = set(std::move(values[i]), std::move(counter));
				return counter;
			}

			template <typename S>
				requires Sequence<S, T>
			S assign (S sequence)
			{
				auto splitting = split(length, std::move(sequence));
				fold([](T * ptr, auto value)
				{
					*ptr = std::move(value);
					return ptr + 1;
				}, values, std::move(std::get<0>(splitting)));
				return std::get<1>(splitting);
			}

			/// @}



			/// Calls function object \a set for each element in the view starting with the last one and
			/// traversing in reverse through the view 
			///
			/// @param counter  mutable value which will be passed along the traversion
			///
			/// @return value of \a counter variable after traversion of all elements
			///
			/// @{
			
			/// @param set function object called for each element with \a counter value; the returning
			///            value is a tuple with the new value for the respective element and a new value
			///            for the \a counter variable
			template <typename C, typename S>
				requires Callable<S, std::tuple<T, C>, C>
			C apply_reverse (S set, C counter)
			{
				std::size_t i = length;
				while (i > 0)
					std::tie(values[--i], counter) = set(std::move(counter));
				return counter;
			}

			/// @param set function object called for each element with the value of the respective
			///            element and the \a counter value; the returning value is a tuple with the new
			///            value for the respective element and a new value for the \a counter variable
			template <typename C, typename S>
				requires Callable<S, std::tuple<T, C>, T, C>
			C apply_reverse (S set, C counter)
			{
				std::size_t i = length;
				while (i > 0)
					--i, std::tie(values[i], counter) = set(std::move(values[i]), std::move(counter));
				return counter;
			}

			/// @param set function object called for each element with the value of the respective
			///            element and the \a counter value; the returning value is a tuple with the new
			///            value for the respective element, a new value for the \a counter variable and
			///            a boolean flag which indicates if the traversion should be continued
			template <typename C, typename S>
				requires Callable<S, std::tuple<T, C, bool>, T, C>
			C apply_reverse (S set, C counter)
			{
				std::size_t i = length;
				bool keepSetting = true;
				while (keepSetting and i > 0)
					--i, std::tie(values[i], counter, keepSetting) = set(std::move(values[i]), std::move(counter));
				return counter;
			}

			/// @}


			
			template <BoundedSequence<T> S> S apply_reverse (S elements);
			template <UnboundedSequence<T> S> S apply_reverse (S elements);





			template <typename S>
				requires BoundedSequence<S, T>
			constexpr bool has_prefix (S prefix) const
			{
				const auto counter = fold([values, length](auto counter, auto value)
				{
					const auto index = std::get<0>(counter);
					const auto keepTesting = index < length and values[index] == value;
					const auto isInTune = not (index < length) or values[index] == value;
					return std::make_tuple(keepTesting, std::make_tuple(index + 1, isInTune));
				}, std::make_tuple(0, true), std::move(prefix));
				return std::get<1>(counter);
			}

			template <typename S, typename C>
				requires BoundedSequence<S> and Callable<C, bool, T, sequence_type_t<S>>
			constexpr bool has_prefix (C compare, S prefix) const
			{
				const auto counter = fold([compare = std::move(compare), values, length](auto counter, auto value)
				{
					const auto index = std::get<0>(counter);
					const auto keepTesting = index < length and compare(values[index], value);
					const auto isInTune = not (index < length) or compare(values[index], value);
					return std::make_tuple(keepTesting, std::make_tuple(index + 1, isInTune));
				}, std::make_tuple(0, true), std::move(prefix));
				return std::get<1>(counter);
			}



			template <typename S>
				requires BoundedSequence<S, T>
			constexpr bool has_suffix (S suffix) const
			{
				const auto suffixLength = length(suffix);
				auto combine = [values, length](auto counter, auto value)
				{
					const auto index = std::get<0>(counter);
					const auto keepTesting = index < length and values[index] == value;
					const auto isInTune = not (index < length) or values[index] == value;
					return std::make_tuple(keepTesting, std::make_tuple(index + 1, isInTune));
				};
				const auto start = std::make_tuple(length - suffixLength, true);
				return suffixLength <= length and std::get<1>(fold(std::move(combine), start, std::move(suffix)));
			}

			template <typename S, typename C>
				requires BoundedSequence<S> and Callable<C, bool, T, sequence_type_t<S>>
			constexpr bool has_suffix (C compare, S suffix) const
			{
				const auto suffixLength = length(suffix);
				auto combine = [compare=std::move(compare), values, length](auto counter, auto value)
				{
					const auto index = std::get<0>(counter);
					const auto keepTesting = index < length and compare(values[index], value);
					const auto isInTune = not (index < length) or compare(values[index], value);
					return std::make_tuple(keepTesting, std::make_tuple(index + 1, isInTune));
				};
				const auto start = std::make_tuple(length - suffixLength, true);
				return suffixLength <= length and std::get<1>(fold(std::move(combine), start, std::move(suffix)));
			}





			constexpr std::tuple<array_view, array_view> split (std::size_t pos) const
			{
				const auto _pos = pos <= length ? pos : length;
				const auto first = array_view(values, _pos);
				const auto second = array_view(values + _pos, length - _pos);
				return std::make_tuple(first, second);
			}
			

			/// Splits view into some prefix and the remainings called stem
			///
			/// @param predict  Detects the delimiter to the prefix
			/// @param value    Mutable value which is passed along the element traversion
			/// @param exclude  Indicates the delimiter character as part of the stem
			///
			/// @return A tuple of views on prefix and stem or on prefix, delimiter and stem; both with
			///         the resulting version of \a value, if some is given; all views partition the
			///         original view in correct order
			///
			/// @{

			/// The view is splitted into prefix, delimiter and stem. The delimiter has either one character,
			/// if some character has been detected, or otherwise no character. The function object \a predict
			/// takes an element and the current version of \a value and returns a tuple with a new version
			/// of \a value and a boolean flag indicating if the most recent element is a delimiter.
			template <typename C, typename V>
				requires Callable<C, std::tuple<V, bool>, T, V>
			constexpr std::tuple<array_view, array_view, array_view, V> split_prefix (C predict, V value) const
			{
				auto foundSplit = false;
				auto index = std::size_t(0);

				while (not foundSplit and index < length)
				{
					std::tie(value, foundSplit) = predict(values[index], std::move(value));
					index = index + (foundSplit ? 0 : 1);
				}

				const auto delimiterLength = index < length ? 1 : 0;
				const auto prefix = array_view(values, index);
				const auto delimiter = array_view(values + index, delimiterLength);
				const auto stem = array_view(values + index + delimiterLength, length - index - delimiterLength);
				return std::make_tuple(prefix, delimiter, stem, std::move(value));
			}
			
			/// The view is splitted into prefix and stem. Whether the delimiter character is considered
			/// part of the prefix or stem, is indicated by \a exclude (true = stem, false = suffix). The
			/// function object takes an element and the current version of \a value and returns a tuple
			/// with a new version of \a value and a boolean flag indicating if the most recent element
			/// is a delimiter.
			template <typename C, typename V>
				requires Callable<C, std::tuple<V, bool>, T, V>
			constexpr std::tuple<array_view, array_view, V> split_prefix (C predict, V value, bool exclude) const
			{
				array_view hypPrefix;
				array_view hypDelimiter;
				std::tie(hypPrefix, hypDelimiter, std::ignore, value) = split_prefix(std::move(predict), std::move(value));
				const auto prefixLength = hypPrefix.length + (exclude ? 0 : hypDelimiter.length());
				const auto prefix = array_view(values, prefixLength);
				const auto stem = array_view(values + prefixLength, length - prefixLength);
				return std::make_tuple(prefix, stem, std::move(value));
			}
		
			/// The view is splitted into prefix, delimiter and stem. The delimiter has either one
			/// character, if some character has been detected as delimiter, or otherwise no character.
			/// The function object \a predict takes an element and returns a boolean flag indicating if
			/// the most recent element is a delimiter.
			template <typename C>
				requires Callable<C, bool, T>
			constexpr std::tuple<array_view, array_view, array_view> split_prefix (C predict) const
			{
				auto foundSplit = true;
				auto index = std::size_t(0);

				while (index < length and not predict(values[index]))
					++index;

				const auto delimiterLength = index < length ? 1 : 0;
				const auto prefix = array_view(values, index);
				const auto delimiter = array_view(values + index, delimiterLength);
				const auto stem = array_view(values + index + delimiterLength, length - index - delimiterLength);
				return std::make_tuple(prefix, delimiter, stem);
			}

			/// The view is splitted into prefix and stem. Whether the delimiter character is considered
			/// part of the prefix or stem, is indicated by \a exclude (true = stem, false = suffix). The
			/// function object takes an element and returns a boolean flag indicating if the most recent
			/// element is a delimiter.
			template <typename C>
				requires Callable<C, bool, T>
			constexpr std::tuple<array_view, array_view> split_prefix (C predict, bool exclude) const
			{
				array_view hypPrefix;
				array_view hypDelimiter;
				std::tie(hypPrefix, hypDelimiter, std::ignore) = split_prefix(std::move(predict));
				const auto prefixLength = hypPrefix.length + (exclude ? 0 : hypDelimiter.length());
				const auto prefix = array_view(values, prefixLength);
				const auto stem = array_view(values + prefixLength, length - prefixLength);
				return std::make_tuple(prefix, stem);
			}

			/// @}


			template <typename C, typename V>
				requires Callable<C, std::tuple<V, bool>, T, V>
			constexpr std::tuple<array_view, array_view, array_view, V> split_suffix (C predict, V value) const
			{
				auto foundSplit = false;
				auto index = std::size_t(length);

				while (not foundSplit and index > 0)
				{
					std::tie(value, foundSplit) = predict(values[index - 1], std::move(value));
					index = index - (foundSplit ? 0 : 1);
				}

				const auto delimiterLength = index < length ? 1 : 0;
				const auto suffixLength = length - index;
				const auto stemLength = length - index - delimiterLength;
				const auto stem = array_view(values, stemLength);
				const auto delimiter = array_view(values + stemLength, delimiterLength);
				const auto suffix = array_view(values + stemLength + delimiterLength, suffixLength);
				return std::make_tuple(stem, delimiter, suffix, std::move(value));
			}
			
			template <typename C, typename V>
				requires Callable<C, std::tuple<V, bool>, T, V>
			constexpr std::tuple<array_view, array_view, V> split_suffix (C predict, V value, bool exclude) const
			{
				array_view hypStem;
				array_view hypDelimiter;
				std::tie(hypStem, hypDelimiter, std::ignore, value) = split_suffix(std::move(predict), std::move(value));
				const auto stemLength = hypStem.length + (exclude ? hypDelimiter.length() : 0);
				const auto stem = array_view(values, stemLength);
				const auto suffix = array_view(values + stemLength, length - stemLength);
				return std::make_tuple(stem, suffix, std::move(value));
			}

			template <typename C>
				requires Callable<C, bool, T>
			constexpr std::tuple<array_view, array_view, array_view> split_suffix (C predict) const
			{
				auto foundSplit = false;
				auto index = std::size_t(length);

				while (index > 0 and not predict(values[index - 1]))
					--index;

				const auto delimiterLength = index < length ? 1 : 0;
				const auto suffixLength = length - index;
				const auto stemLength = length - index - delimiterLength;
				const auto stem = array_view(values, stemLength);
				const auto delimiter = array_view(values + stemLength, delimiterLength);
				const auto suffix = array_view(values + stemLength + delimiterLength, suffixLength);
				return std::make_tuple(stem, delimiter, suffix, std::move(value));
			}
			
			template <typename C>
				requires Callable<C, bool, T>
			constexpr std::tuple<array_view, array_view> split_suffix (C predict, bool exclude) const
			{
				array_view hypStem;
				array_view hypDelimiter;
				std::tie(hypStem, hypDelimiter, std::ignore, value) = split_suffix(std::move(predict));
				const auto stemLength = hypStem.length + (exclude ? hypDelimiter.length() : 0);
				const auto stem = array_view(values, stemLength);
				const auto suffix = array_view(values + stemLength, length - stemLength);
				return std::make_tuple(stem, suffix);
			}


	};

}

#endif

