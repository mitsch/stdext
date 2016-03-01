/// @file reverse_array_view.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_REVERSE_ARRAY_VIEW_HPP__
#define __STDEXT_REVERSE_ARRAY_VIEW_HPP__

#include <stdext/optional.hpp>
#include <type_traits>



namespace stdext
{

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

	};

}

#endif

