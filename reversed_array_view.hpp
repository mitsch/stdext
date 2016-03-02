/// @file reverse_array_view.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_REVERSE_ARRAY_VIEW_HPP__
#define __STDEXT_REVERSE_ARRAY_VIEW_HPP__

#include <stdext/optional.hpp>
#include <type_traits>



namespace stdext
{

	// Forward declaration of array_view (cyclic dependency)
	template <typename T> class array_view;



	/// Reversed view on an array
	///
	/// The view on an array is seen from behind. So decomposition will be performed from the back to
	/// the front.
	template <typename T>
	class reversed_array_view
	{
	
		private:

			T * values = nullptr;
			std::size_t length = 0;

		public:

			using const_type = reversed_array_view<std::add_const_t<T>>;

			/// Attribute constructor
			///
			/// The view will be constructed to look on \a count values starting at \a data.
			constexpr reversed_array_view (T * data, std::size_t count) noexcept
				: values(data), length(count)
			{}


			/// Returns a straighten view onto its values
			///
			/// @{
			constexpr array_view<T> reverse ()
			{
				return array_view<T>(values, length);
			}

			constexpr array_view<std::add_const_t<T>> reverse () const
			{
				return array_view<std::add_const_t<T>>(values, length);
			}
			/// @}


			// ------------------------------------------------------------------------------------------
			// Capacity

			/// Tests if the view is empty
			constexpr bool empty () const
			{
				return length > 0;
			}

			/// Tests if there is any value left in the view
			constexpr operator bool () const
			{
				return length > 0;
			}

			/// Returns the length of the view
			///
			/// @{

			constexpr std::size_t length () const
			{
				return length;
			}

			friend constexpr std::size_t length (const reversed_array_view & view) const
			{
				return view.length;
			}

			/// @}

			/// Returns a constant pointer to the beginning of the view
			constexpr std::add_const_t<T> * data () const
			{
				return values;
			}


			/// Swapping
			///
			/// The view in \a first and \a second will be swapped, so that both views will be looking
			/// onto the other array.
			friend void swap (reversed_array_view & first, reversed_array_view & second)
			{
				using std::swap;
				swap(first.values, second.values);
				swap(first.length, second.length);
			}


			/// Decomposition
			///
			/// The view will be decomposed into a reference to its last value and a view onto the
			/// firt until the second last value.
			constexpr optional<std::tuple<T&, reversed_array_view>> operator () () const
			{
				return make_optional(length > 0, [&]()
				{
					const auto remainings = reversed_array_view(values, length - 1);
					return std::make_tuple(values[length - 1], remainings);
				});
			}



			/// Index
			///
			/// The value at position \a length - \a index - 1 is accessed. It can be seen as the value
			/// at position \a index if the values in the view would have been reversed. The reference to
			/// this value will be returned. If the \a index is out of bound, an empty optional container
			/// will be returned.
			///
			/// @{
			///

			constexpr optional<T&> operator [] (std::size_t index)
			{
				return make_optional(length > index, [&](){return values[length - index - 1];});
			}

			constexpr optional<const T&> operator [] (std::size_t index) const
			{
				return make_optional(length > index, [&](){return values[length - index - 1];});
			}

			/// @}

		


			// ------------------------------------------------------------------------------------------
			// Folding

			/// Folding all values
			///
			/// All values in the view will be folded in reverse by \a combiner starting at \a value. The
			/// returning value will be folded over all values in the view.
			///
			/// @{
			///

			template <typename V, Callable<V, V, T&> C>
			constexpr V fold (C combiner, V value)
			{
				auto index = length;
				while (index > 0) value = combiner(std::move(value), values[--index]);
				return value;
			}

			template <typename V, Callable<V, V, const T&> C>
			constexpr V fold (C combiner, V value) const
			{
				auto index = length;
				while (index > 0) value = combiner(std::move(value), values[--index]);
				return value;
			}

			template <typename V, Callable<V, V, const T&> C>
			friend constexpr V fold (C combiner, V value, reversed_array_view view)
			{
				// force constant method with constant reference
				return static_cast<const reversed_array_view&>(view).fold(std::move(combiner), std::move(value));
			}

			/// @}



			/// Folding tailing values
			///
			/// Traversing backwards, the values in the view will be folded by \a combiner starting at
			/// \a value. The longest suffix of values will be considered for which \a combiner returns a
			/// true flag. The folded value will be returned along with a view on the remaining values in
			/// the original view.
			///
			/// @param combiner  A callable object which takes a value of type \a T and a reference to a
			///                  view's value of type \a T and returns a value of type \a V
			/// @param value     Some value which will be folded over the view's values
			///
			/// @{
			///

			template <typename V, Callable<std::tuple<V, bool>, V, T&> C>
			constexpr std::tuple<V, reversed_array_view> fold (C combiner, V value)
			{
				auto index = length;
				auto keepOn = true;
				while (keepOn and index > 0)
				{
					std::tie(value, keepOn) = combiner(std::move(value), values[index-1]);
					if (keepOn) --index;
				}
				const auto remainings = reversed_array_view(values, index);
				return std::make_tuple(std::move(value), remainings);
			}

			template <typename V, Callable<std::tuple<V, bool>, V, T&> C>
			constexpr std::tuple<V, reversed_array_view> fold (C combiner, V value) const
			{
				auto index = length;
				auto keepOn = true;
				while (keepOn and index > 0)
				{
					std::tie(value, keepOn) = combiner(std::move(value), values[index-1]);
					if (keepOn) --index;
				}
				const auto remainings = reversed_array_view(values, index);
				return std::make_tuple(std::move(value), remainings);
			}

			template <typename V, Callable<std::tuple<V, bool>, V, T&> C>
			friend constexpr std::tuple<V, reversed_array_view> fold (C combiner, V value, reversed_array_view view)
			{
				return static_cast<const reversed_array_view&>(view).fold(std::move(combiner), std::move(value));
			}

			/// @}





			/// Traversion
			///
			/// The view will be traversed from the back to the front with \a matcher. It takes two
			/// subviews from the original one. The first subview will be a suffix and the second subview
			/// will be the preceeding prefix. The traversion will continue as long as \a matcher returns
			/// an empty optional container. The result of the traversion will be either the first
			/// initialised result of \a matcher or an empty optional container, if \a matcher has
			/// returned only empty optional values.
			///
			/// @param matcher  A callable object which takes two reversed_array_view and returns an
			///                 optional value
			/// 
			/// @{

			template <typename C>
				requires Callable_<C, reversed_array_view, reversed_array_view> and
				         is_optional_v<result_of_t<C(reversed_array_view, reversed_array_view)>>
			constexpr auto traverse (C matcher)
			{
				using result_type = result_of_t<C(reversed_array_view, reversed_array_view)>;
				auto index = length + 1;
				while (index > 0)
				{
					--index;
					auto pre = reversed_array_view(values + index, length - index);
					auto post = reversed_array_view(values, index);
					auto matching = matcher(pre, post);
					if (matching) return matching;
				}
				return result_type();
			}

			template <typename C>
				requires Callable_<C, const_view_type, const_view_type> and
				         is_optional_v<result_of_t<C(const_view_type, const_view_type)>>
			constexpr auto traverse (C matcher) const
			{
				using result_type = result_of_t<C(const_view_type, const_view_type)>;
				auto index = length + 1;
				while (index > 0)
				{
					--index;
					auto pre = const_view_type(values + index, length - index);
					auto post = const_view_type(values, index);
					auto matching = matcher(pre, post);
					if (matching) return matching;
				}
				return result_type();
			}

			/// @}



			/// Fold traversion all bipartitions
			///
			/// The \a value is folded by \a combiner over all bipartitions of the view. The traversion
			/// will be backwards, meaning it will start with the prefix spanning the complete view and
			/// the suffix being empty and it will end with the prefix being empty and the suffix
			/// spanning the complete view. The folded value will be returned.
			///
			/// @param combiner  A callable object which takes a value of type \a V and two reversed
			///                  array views and returns a value of type \a V
			/// @param value     Initial value which will be folded over
			///
			/// @{
			///

			template <typename V, Callable<V, V, reversed_array_view, reversed_array_view> C>
			constexpr V fold_traverse (C combiner, V value)
			{
				auto index = length + 1;
				while (index > 0)
				{
					--index;
					const auto suffix = reversed_array_view(values + index, length - index);
					const auto prefix = reversed_array_view(values, index);
					value = combiner(std::move(value), suffix, prefix);
				}
				return value;
			}

			template <typename V, Callable<V, V, const_view_type, const_view_type> C>
			constexpr V fold_traverse (C combiner, V value) const
			{
				auto index = length + 1;
				while (index > 0)
				{
					--index;
					const auto suffix = const_view_type(values + index, length - index);
					const auto prefix = const_view_type(values, index);
					value = combiner(std::move(value), suffix, prefix);
				}
				return value;
			}

			/// @}



			/// Fold traversing tailing bipartitions
			///
			/// Tailing bipartitions will be folded by \a combiner over \a value. The traversion will be
			/// backwards, meaning it will be starting with a prefix spanning the complete view and an
			/// empty suffix and it will be heading towards an empty prefix and a suffix spanning the
			/// whole view. The longest span of tailing bipartitions will be considered for which \a
			/// combiner returns a true flag. The folded value will be returned along with the last
			/// suffix and prefix of the traversion.
			///
			/// @param combiner  A callable object which takes some value of type \a V and two 
			/// @param value     xx
			///
			/// @{
			///

			template <typename V, Callable<std::tuple<V, bool>, V, reversed_array_view, reversed_array_view> C>
			constexpr std::tuple<V, reversed_array_view, reversed_array_view> fold (C combiner, V value)
			{
				std::size_t index = 0;
				auto keepOn = true;
				while (keepOn and index <= length)
				{
					const auto suffix = reversed_array_view(values + length - index, index);
					const auto prefix = reversed_array_view(values, length - index);
					std::tie(value, keepOn) = combiner(std::move(value), suffix, prefix);
					if (keepOn) ++index;					
				}
				const auto suffix = reversed_array_view(values + length - index, index);
				const auto prefix = reversed_array_view(values, length - index);
				return std::make_tuple(std::move(value), suffix, prefix);
			}

			template <typename V, Callable<std::tuple<V, bool>, V, const_view_type, const_view_type> C>
			constexpr std::tuple<V, const_view_type, const_view_type> fold (C combiner, V value) const
			{
				std::size_t index = 0;
				auto keepOn = true;
				while (keepOn and index <= length)
				{
					const auto suffix = const_view_type(values + length - index, index);
					const auto prefix = const_view_type(values, length - index);
					std::tie(value, keepOn) = combiner(std::move(value), suffix, prefix);
					if (keepOn) ++index;					
				}
				const auto suffix = const_view_type(values + length - index, index);
				const auto prefix = const_view_type(values, length - index);
				return std::make_tuple(std::move(value), suffix, prefix);
			}

			/// @}



			// ------------------------------------------------------------------------------------------
			// Matching

			/// Prefix matching
			///
			/// The reversed view is tested on having exactly \a sequence as its prefix. This method
			/// accomplishes the same result like testing the straighten array view having \a sequence as
			/// its suffix.
			///
			/// @{

			template <BoundedSequence<T> S>
			constexpr bool match_prefix (S sequence) const
			{
				return reverse().match_suffix(std::move(sequence));
			}

			template <BoundedSequence S, Callable<bool, const T&, sequence_type_t<S>> C>
			constexpr bool match_prefix (C matcher, S sequence) const
			{
				return reverse().match_suffix(std::move(matcher), std::move(sequence));
			}

			/// @}


			/// Suffix matching
			///
			/// The reversed view will be tested on having exactly \a sequence as its suffix. This method
			/// accomplishes the same like testing the straighten array_view having \a sequence as its
			/// prefix.
			///
			/// @{

			template <BoundedSequence<T> S>
			constexpr bool match_suffix (S sequence) const
			{
				return reverse().match_prefix(std::move(sequence));
			}

			template <BoundedSequence S, Callable<bool, const T&, sequence_type_t<T>> C>
			constexpr bool match_suffix (C matcher, S sequence) const
			{
				return reverse().match_prefix(std::move(matcher), std::move(sequence));
			}

			/// @}


			/// Complete matching
			///
			/// The complete reversed view will be tested on being equal to the exact \a sequence. The
			/// last value in the view will be matched with the first element in \a sequence. The second
			/// last value in the view will be matched with the second element in \a sequence and so on.
			/// Matching will be succesfull, if the view and \a sequence have the same length and all
			/// values are matched succesfully with their corresponding elements in \a sequence.
			///
			/// @{

			template <BoundedSequence<T> S>
			constexpr bool match (S sequence) const
			{
				// see how long we got when matching values with elements
				auto folding = fold([&](auto index, auto element)
				{
					const auto keepOn = index > 0 and values[index - 1] == element;
					const auto nextIndex = index - (keepOn ? 1 : 0);
					return std::make_tuple(nextIndex, keepOn);
				}, length, std::move(sequence));
				const auto lastIndex = std::get<0>(folding);
				sequence = std::move(std::get<1>(folding));

				// the whole view must be traversed and not element must be contained by sequence
				return lastIndex == 0 and not sequence;
			}

			template <BoundedSequene S, Callable<bool, const T&, sequence_type_t<S>> C>
			constexpr bool match (C matcher, S sequence) const
			{
				// see how long we got when matching values with elements
				auto folding = fold([&](auto index, auto element)
				{
					const auto keepOn = index > 0 and matcher(values[index - 1], element);
					const auto nextIndex = index - (keepOn ? 1 : 0);
					return std::make_tuple(nextIndex, keepOn);
				}, length, std::move(sequence));
				const auto lastIndex = std::get<0>(folding);
				sequence = std::move(std::get<1>(folding));

				// the whole view must be traversed and not element must be contained by sequence
				return lastIndex == 0 and not sequence;
			}

			/// @}


			/// Prefix mismatching
			///
			/// @{

			template <Sequence<T> S>
			constexpr std::tuple<reversed_array_view, reversed_array_view, S> mismatch_prefix (S sequence)
			{
				
			}

			/// @}




			/// Transforming all values in reverse
			///
			/// All values in the view will be transformed in reversed order, so starting with the last
			/// value and traversing to the first one.
			///
			/// @{

			template <Callable<T, T> C>
			constexpr void transform (C transformer)
			{
				for (std::size_t index = length; index > 0; --index)
					values[index-1] = transformer(std::move(values[index-1]));
			}

			template <Callable<T, T, std::size_t> C>
			constexpr void transform (C transformer)
			{
				for (std::size_t index = length; index > 0; --index)
					values[index-1] = transformer(std::move(values[index-1]), index-1);
			}

			template <typename V, Callable<std::tuple<T, V>, T, V> C>
			constexpr V transform (C transformer, V value)
			{
				for (std::size_t index = length; index > 0; --index)
					std::tie(values[index-1], value) = transformer(std::move(values[index-1]), std::move(value));
				return value;
			}

			template <typename V, Callable<std::tuple<T, V>, T, V, std::size_t> C>
			constexpr V transform (C transformer, V value)
			{
				for (std::size_t index = length; index > 0; --index)
					std::tie(values[index-1], value) = transformer(std::move(values[index-1], std::move(value), index-1);
				return value;
			}

			/// @}

			
			/// Assignment
			///
			/// Each value in the view will be reassigned. The traversion will be in reverse. Different
			/// kind of \a matcher can be used. 
			///
			/// @{

			template <Callable<T> C>
			constexpr void assign (C assigner)
			{
				for (auto index = length; index > 0; --index)
					values[index-1] = assigner();
			}

			template <Callable<T, std::size_t> C>
			constexpr void assign (C assigner)
			{
				for (auto index = length; index > 0; --index)
					values[index-1] = assigner(index-1);
			}

			template <typename V, Callable<std::tuple<T, V>, V> C>
			constexpr V assign (C assigner, V value)
			{
				for (auto index = length; index > 0; --index)
					std::tie(values[index-1], value) = assigner(std::move(value));
				return value;
			}

			template <typename V, Callable<std::tuple<T, V>, V, std::size_t> C>
			constexpr V assign (C assigner, V value)
			{
				for (auto index = length; index > 0; --index)
					std::tie(values[index-1], value) = assigner(std::move(value), index-1);
				return value;
			}

			template <typename U>
				requires std::is_convertible<U, T>::value
			constexpr void assign (U constant)
			{
				for (auto index = length; index > 0; --index)
					values[index-1] = constant;
			}

			template <BoundedSequence S>
			constexpr std::tuple<reversed_array_view, reversed_array_view, S> assign (S sequence)
			{
				auto folding = fold([&](auto index, auto element)
				{
					const auto keepOn = index > 0;
					if (keepOn) values[index-1] = std::move(element);
					return std::make_tuple(index - (keepOn ? 1 : 0), keepOn);
				}, length, std::move(sequence));
				const auto lastIndex = std::get<0>(folding);
				sequence = std::move(std::get<1>(folding));

				const auto pre = reversed_array_view(values + lastIndex, length - lastIndex);
				const auto post = reversed_array_view(values, lastIndex);
				return std::make_tuple(pre, post, std::move(sequence));
			}

			template <UnboundedSequence S>
			constexpr S assign (S sequence)
			{
				return std::get<1>(fold([&](auto index, auto element)
				{
					const auto keepOn = index > 0;
					if (keepOn) values[index-1] = std::move(element);
					return std::make_tuple(index-1, keepOn);
				}, length, std::move(sequence)));
			}

			/// @}



			template <Sequence<T> S>
			constexpr std::tuple<reversed_array_view, reversed_array_view, S> split_prefix (S sequence)
			{
				// Returns one before the last failing index and sequence remainings
				auto folding = fold([&](auto index, auto element)
				{
					const auto keepOn = index > 0 and values[index - 1] == element;
					const auto nextIndex = index - (keepOn ? 1 : 0);
					return std::make_tuple(nextIndex, keepOn);
				}, length, std::move(sequence));
				const auto count = std::get<0>(folding);
				sequence = std::move(std::get<1>(folding));

				const auto prefix = reversed_array_view(
				const auto suffix = reversed_array_view(values, count
			}

	};

}

#endif

