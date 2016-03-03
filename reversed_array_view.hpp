/// @file reverse_array_view.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_REVERSE_ARRAY_VIEW_HPP__
#define __STDEXT_REVERSE_ARRAY_VIEW_HPP__

#include <stdext/array_view.hpp>



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

			array_view<T> view;

		public:

			/// Attribute constructor
			///
			/// The \a view will be copied into the constructed object.
			constexpr reversed_array_view (array_view<T> view) noexcept
				: view(view)
			{}


			/// Decomposition
			///
			/// The view will be decomposed into a reference to its last value and a view onto the
			/// firt until the second last value.
			constexpr optional<std::tuple<T&, reversed_array_view>> operator () () const
			{
				return view.decompose_suffix();
			}

			/// Tests if any value is left in the view
			constexpr operator bool () const
			{
				return static_cast<bool>(view.view);
			}

			/// Folding all values
			///
			/// All values in the view will be folded in reverse by \a combiner starting at \a value. The
			/// returning value will be folded over all values in the view.
			template <typename V, Callable<V, V, const T&> C>
			friend constexpr V fold (C combiner, V value, reversed_array_view view)
			{
				return fold_reverse(std::move(combiner), std::move(value), view.view);
			}

			/// Folding tailing values
			///
			/// Traversing backwards, the values in the view will be folded by \a combiner starting at
			/// \a value. The longest suffix of values will be considered for which \a combiner returns a
			/// true flag. The folded value will be returned along with a view on the remaining values in
			/// the original view.
			template <typename V, Callable<std::tuple<V, bool>, V, const T&> C>
			friend constexpr std::tuple<V, reversed_array_view> fold (C combiner, V value, reversed_array_view view)
			{
				auto folding = fold_reverse(std::move(combiner), std::move(value), view.view);
				const auto remainings = reversed_array_view(std::get<1>(folding));
				return std::make_tuple(std::move(std::get<0>(folding)), remainings);
			}

			/// Returns the length
			friend constexpr std::size_t length (reversed_array_view view)
			{
				return length(view.view);
			}
	};

}

#endif

