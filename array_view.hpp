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
	/// and its allocation or deallocation. Since the array view is designed to be lightweighted and
	/// to be not responsible for the elements it sees (it does not own them), no move semantics is
	/// supported. No synchronisation is built in either, so two views sharing some elements might
	/// affect each other's elements. Also, views cannot grow but only shrink.
	template <typename T> class array_view
	{
	
		private:

			T * values;
			std::size_t length;


			/// Insertion sorts elements from index \a begin to \a end
			template <Callable<bool, const T&, const T&> C>
			constexpr void insertion_sort (C comparer, std::size_t begin, std::size_t end)
			{
				assert(begin <= end);
				assert(end <= length);

				using std::swap;

				for (auto nextToSort = begin + 1; nextToSort < end; ++nextToSort)
				{
					auto index = nextToSort;
					while (index > begin and comparer(values[index], values[index-1]))
					{
						swap(values[index], values[index-1]);
						--index;
					}
				}
			}

			/// Get median index of three values at indices \a a, \a b and \a c
			template <Callable<bool, const T&, const T&> C>
			constexpr std::size_t index get_median_index (C comparer, std::size_t a, std::size_t b, std::size_t c) const
			{
				assert(a < length);
				assert(b < length);
				assert(c < length);

				const auto ab = comparer(values[a], values[b]);
				const auto bc = comparer(values[b], values[c]);
				const auto ac = comparer(values[a], values[c]);

				if ((ab and bc) or (not ab and not ac and not bc)) return b;
				else if ((ab and not bc and not ac) or (not ab and ac)) return a;
				else return c;
			}


			/// Partitions with pivot taken randomly from the values
			template <Callable<bool, const T&, const T&> C>
			constexpr std::size_t randomly_partition (C comparer, std::size_t begin, std::size_t end)
			{
				assert(begin + 3 < end);
				assert(end <= length);

				using std::swap;

				const auto middle = begin + (end - begin) / 2;
				const auto median = get_median_index(comparer, begin, middle, end-1);
				if (median != end-1) swap(values[median], values[end-1]);

				const T& pivot = values[end-1];	
				const auto bound = partition([&pivot, &comparer](const auto & element)
				{
					return comparer(element, pivot);
				}, begin, end-1);

				return bound;
			}


			/// Partitions with predictor from \a begin to \a end
			template <Callable<bool, const T&> C>
			constexpr std::size_t partition (C predictor, std::size_t begin, std::size_t end)
			{
				assert(begin <= end);
				assert(end <= length);

				using std::swap;

				while (true)
				{
					while (predictor(values[first])) ++first;
					--end;
					while (not predictor(values[end]) --end;

					if (first == end) return first;
					swap(values[first], values[end]);
					++begin;
				}
			}

			/// Builds heap with \a comparer from \a begin to \a end
			template <Callable<bool, const T&, const T&> C>
			constexpr void build_heap (C comparer, std::size_t begin, std::size_t end)
			{
				assert(begin < end);
				assert(end <= length);

				using std::swap;
				const auto firstLeaf = ((end - begin) / 2) + 2 + begin;
				for (auto parent = firstLeaf; parent > begin; --parent)
					heapify(comparer, begin, end, parent-1);
			}

			/// Heapifies down
			template <Callable<bool, const T&, const T&> C>
			constexpr void heapify (C comparer, std::size_t begin, std::size_t end, std::size_t parent)
			{
				assert(begin < end);
				assert(end <= length);
				assert(begin <= parent);
				assert(parent <= end);

				while (parent < end)
				{
					const auto left = parent * 2 + 1;
					const auto right = left + 1;
					
					if (left < end)
					{
						const auto bestChild = (right >= end or comparer(values[left], values[right])) ? left : right;
						if (comparer(values[bestChild], values[parent]))
						{
							swap(values[bestChild], values[parent]);
							parent = bestChild);
						}
					}
					else break;
				}
			}


			/// Introselection
			template <Callable<bool, const T&, const T&> C>
			constexpr void intro_select (C comparer, std::size_t index, std::size_t depth)
			{
				
			}

		public:

			/// Underlying type of the array view
			using value_type = T;




			// ------------------------------------------------------------------------------------------

			/// Default constructor
			///
			/// An empty view is constructed, that is a view pointing to nullptr and having length zero.
			constexpr array_view () noexcept
				: values(nullptr), length(0)
			{}
			
			/// Copy constructor
			///
			/// The view from \a other is copied, so both instances see the same elements. Consequently,
			/// reading with the same parameters will result in the same outcome and writing with one of
			/// the views will affect the other one.
			constexpr array_view (const array_view & other) noexcept = default;
			
			/// Parameter constructor
			///
			/// The view will be on \a values with the next \a count elements. If \a values is nullptr,
			/// \a count has to be zero.
			constexpr array_view (const T * values, const std::size_t count) noexcept
				: values(values), length(count)
			{
				assert(values != nullptr or count == 0);
			}

			/// Destructor
			///
			/// The destructor does nothing; neither destructing its elements nor deallocating the memory.
			~array_view () = default;


			/// Copy assignment
			///
			/// The assignment will make the own instance to be a copy of \a other, so both views will
			/// see the same elements. Consequently, reading with the same parameters will result in the
			/// same outcome and writing with one of the views will affect the other one. All elements
			/// of the former view will be uneffected.
			constexpr array_view& operator = (const array_view & other) = default;
			


			/// Returns whether the view is empty, that is it sees no elements
			constexpr bool empty () const noexcept
			{
				return length == 0;
			}
			
			/// Returns whether the view is not empty, that is it sees some elements
			constexpr operator bool () const noexcept
			{
				return length > 0;
			}
			
			/// Returns the amount of elements the view sees
			constexpr std::size_t length () const noexcept
			{
				return length;
			}

			/// Swaps the view, not its elements, between \a first and \a second
			///
			/// After swapping, \a first will see all the elements which \a second has seen before the
			/// swapping, and vice versa. No element will be altered or changed.
			///
			/// @note Time complexity: constant
			/// @note Space complexity: constant
			friend void swap (array_view & first, array_view & second)
			{
				using std::swap;
				swap(first.values, second.values);
				swap(first.length, second.length);
			}



			/// Index operator
			///
			/// The index operator accesses the value placed at \a index. Rather applying some functions
			/// on a range of values, only one value is accessed.
			/// @{

			constexpr optional<T&> operator [] (std::size_t index)
			{
				return index < length ? optional<T&>(values + index) : optional<T&>();
			}

			constexpr optional<const T&> operator [] (std::size_t index) const
			{
				return index < length ? optional<const T&>(values + index) : optional<const T&>();
			}

			/// @}




			/// Folding
			///
			/// All elements are consecutively folded with \a combiner starting at \a value. The normal
			/// direction is front to back (\a fold) and the reversed direction is back to front
			/// (\a fold_reverse). The folding function \a combine returns either the newly created
			/// value or a tuple of the newly created value and some boolean flag indicating whether to
			/// progress or not in the folding procedure.
			///
			/// @param combiner  Takes the most recent value and the next element and returns either a
			///                  new value or some tuple with the new value and some boolean flag
			///                  indicating whether to proceed
			/// @param value     The initial value which be combined with the first element or returned
			///                  if the view is empty.
			/// 
			/// @{
			///
			
			template <typename V, Callable<V, V, T> C> constexpr V fold (C combiner, V value) const
			{
				for (std::size_t i = 0; i < length; ++i)
					value = combiner(std::move(value), values[i]);
				return value;
			}
			
			template <typename V, Callable<V, V, T, std::size_t> C> constexpr V fold (C combiner, V value) const
			{
				for (std::size_t i = 0; i < length; ++i)
					value = combiner(std::move(value), values[i], i);
				return value;
			}
			
			template <typename V, Callable<std::tuple<V, bool>, V, T> C> constexpr V fold (C combiner, V value) const
			{
				bool keepFolding = true;
				for (std::size_t i = 0; keepFolding and i < length; ++i)
					std::tie(value, keepFolding) = combiner(std::move(value), values[i]);
				return value;
			}
			
			template <typename V, Callable<std::tuple<V, bool>, V, T, std::size_t> C>
			constexpr V fold (C combiner, V value) const
			{
				bool keepFolding = true;
				for (std::size_t i = 0; keepFolding and i < length; ++i)
					std::tie(value, keepFolding) = combiner(std::move(value), values[i], i);
				return value;
			}
			
			template <typename V, Callable<V, V, T> C> constexpr V fold_reverse (C combiner, V value) const
			{
				std::size_t i = length;
				while (i > 0)
					value = combiner(std::move(value), values[--i]);
				return value;
			}
			
			template <typename V, Callable<V, V, T, std::size_t> C> constexpr V fold_reverse (C combiner, V value) const
			{
				std::size_t i = length;
				while (i > 0)
					--i, value = combiner(std::move(value), values[i], i);
				return value;
			}
			
			template <typename V, Callable<std::tuple<V, bool>, V, T> C> constexpr V fold_reverse (C combiner, V value) const
			{
				std::size_t i = length;
				bool keepFolding = true;
				while (keepFoldingi > 0)
					std::tie(value, keepFolding) = combiner(std::move(value), values[--i]);
				return value;
			}
			
			template <typename V, Callable<std::tuple<V, bool>, V, T, std::size_t> C>
			constexpr V fold_reverse (C combiner, V value) const
			{
				std::size_t i = length;
				bool keepFolding = true;
				while (keepFoldingi > 0)
					--i, std::tie(value, keepFolding) = combiner(std::move(value), values[i], i);
				return value;
			}
			
			/// @}


			/// Traversion
			///
			/// Traversion intends to find the appropriate view by going through all possible
			/// combinations of binary view partitions and testing these with \a match. The first
			/// initialised optional container is returned. If \a match returns only empty optional
			/// containers, an empty optional container will be returned. The direction of traversion
			/// is either front to back (\a traverse) or back to front (\a traverse_reverse).
			///
			/// @param match  The decision function takes the preceeding view and the succeding view
			///               as parameters and returns an optional container which, in case of
			///               success, will be initialised with some value.
			///
			/// @note Time complexity: linear in the length of the view
			///
			/// @{
			///
			
			template <typename C>
				requires Callable_<C, array_view, array_view> and
				         is_optional<std::result_of_t<C(array_view, array_view)>>::value
			constexpr auto traverse (C match) const
			{
				using type = std::result_of_t<C(array_view, array_view)>;
				for (std::size_t index = 0; index < length; ++index)
				{
					const auto matching = match(array_view(values, index), array_view(values + index, length - index));
					if (matching) return matching;
				}
				return type();
			}
			
			template <typename C>
				requires Callable_<C, array_view, array_view> and
				         is_optional<std::result_of_t<C(array_view, array_view)>>::value
			constexpr auto traverse_reverse (C match) const
			{
				using type = std::result_of_t<C(array_view, array_view)>;
				for (std::size_t index = 0; index < length; ++index)
				{
					const auto offset = length - index;
					const auto matching = match(array_view(values, offset), array_view(values + offset, index));
					if (matching) return matching;
				}
				return type();
			}

			template <Callable_<array_view, array_view> C>
				requires is_optional<std::result_of_t<C(array_view, array_view)>>::value
			constexpr void traverse (C matcher)
			{
				using type = std::result_of_t<C(array_view, array_view)>;
				for (std::size_t index = 0; index < length; ++index)
				{
					const auto matching = matcher(array_view(values, index), array_view(values + index, length - index));
					if (matching) return matching;
				}
				return type();
			}
			
			template <Callable_<array_view, array_view> C>
				requires is_optional<std::result_of_t<C(array_view, array_view)>>::value
			constexpr void traverse_reverse (C matcher)
			{
				using type = std::result_of_t<C(array_view, array_view)>;
				for (std::size_t index = 0; index < length; ++index)
				{
					cosnt auto offset = length - index;
					const auto matching = matcher(array_view(values, offset), array_view(values + offset, index));
					if (matching) return matching;
				}
				return type();
			}
			
			/// @}


			/// Folding over a traversion
			///
			/// With folding over traversion, two routines are merged into one. Rather than folding
			/// some value over the elements, some \a value is folded over the binary view partitions.
			/// The combination is performed with \a combine. It takes the previous value, the
			/// preceeding view and the succeeding view. The result of \a combine is either the next
			/// value or a tuple of the new value and some boolean flag indicating whether to go on in
			/// the process. The direction of element processing in \a fold_traverse is front to back.
			/// The direction of element processing in \a fold_traverse_reverse is back to front.
			///
			/// @param combine  The function takes some value of type \a V and the preceeding and
			///                 succeeding view on some binary view partition. The result is either a
			///                 new value of type \a V or some tuple with a new value of type \a V and
			///                 and a boolean flag indicating whether to go on in the process.
			/// @param value    The initial value will be combined with the first binary view
			///                 partition.
			///
			///
			/// @note Time complexity: linear in the length of the view
			///
			/// @{
			///
			
			template <typename V, Callable<V, V, array_view, array_view> C>
			constexpr V fold_traverse (C combine, V value) const
			{
				for (std::size_t index = 0; index < length; ++index)
					value = combine(std::move(value), array_view(values, index), array_view(values + index, length - index));
				return value;
			}
			
			template <typename V, Callable<std::tuple<V, bool>, V, array_view, array_view> C>
			constexpr V fold_traverse (C combine, V value) const
			{
				auto keepFolding = true;
				for (std::size_t index = 0; keepFolding and index < length; ++index)
					std::tie(value, keepFolding) =
						combine(std::move(value), array_view(values, index), array_view(values + index, length - index));
				return value;
			}
			
			template <typename V, Callable<V, V, array_view, array_view> C>
			constexpr V fold_traverse_reverse (C combine, V value) const
			{
				for (std::size_t index = 0; index < length; ++index)
				{
					const auto offset = length - index;
					value = combine(std::move(value), array_view(values, offset), array_view(values + offset, index));
				}
				return value;
			}
			
			template <typename V, Callable<std::tuple<V, bool>, V, array_view, array_view> C>
			constexpr V fold_traverse_reverse (C combine, V value) const
			{
				auto keepFolding = true;
				for (std::size_t index = 0; keepFolding and index < length; ++index)
				{
					const auto o = length - index;
					std::tie(value, keepFolding) =
						combine(std::move(value), array_view(values, offset), array_view(values + offset, index));
				}
				return value;
			}
			
			/// @}



			/// Matcher
			///
			///	The view is tested on having either a matching prefix, suffix or a complete match for all
			///	values in the view. If only \a elements i given, the standard equivalence operator is
			///	used to test on equivalence between values in the view and elements in the sequence. If
			///	\a matcher is given, it will be used as equivalence tester. Matching prefix is tested
			///	with \a match_prefix, matching suffix is tested with \a match_suffix and matching the
			/// complete view is tested with \a match.
			///
			/// @{

			template <BoundedSequence<T> S>
			constexpr bool match_prefix (S elements) const
			{
				auto state = fold([values=values, limit=length](auto index, auto element)
				{
					const auto keepGoing = index < limit and values[index] == element;
					return std::make_tuple(index + 1, keepGoing);
				}, std::size_t(0), std::move(elements));
				elements = std::move(std::get<1>(state));
				return not elements;
			}

			template <BoundedSequence S, Callable<bool, const T&, sequence_type_t<S>> C>
			constexpr bool match_prefix (C matcher, S elements) const
			{
				auto state = fold([matcher=std::move(matcher), values=values, limit=length]
					(auto index, auto element)
				{
					const auto keepGoing = index < limit and matcher(values[index], element);
					return std::make_tuple(index + 1, keepGoing);
				}, std::size_t(0), std::move(elements));
				elements = std::move(std::get<1>(state));
				return not elements;
			}

			template <BoundedSequence<T> S>
			constexpr bool match_suffix (S elements) const
			{
				auto state = fold([values=values, limit=length](auto index, auto element)
				{
					const auto keepGoing = index > 0 and values[index] == element;
					return std::make_tuple(index - 1, keepGoing);
				}, length, std::move(elements));
				elements = std::move(std::get<1>(state));
				return not elements;
			}

			template <BoundedSequence S, Callable<bool, const T&, sequence_type_t<S>> C>
			constexpr bool match_suffix (C matcher, S elements) const
			{
				auto state = fold([matcher=std::move(matcher), values=values, limit=length]
					(auto index, auto element)
				{
					const auto keepGoing = index > 0 and matcher(values[index - 1], element);
					return std::make_tuple(index - 1, keepGoing);
				}, length, std::move(elements));
				elements = std::move(std::get<1>(state));
				return not elements;
			}

			template <BoundedSequence<T> S>
			constexpr bool match (S elements) const
			{
				auto state = fold([values=values, limit=length](auto index, auto element)
				{
					const auto keepGoing = index < limit and values[index] == element;
					return std::make_tuple(index + (keepGoing ? 1 : 0), keepGoing);
				}, std::size_t(0), std::move(elements));
				const auto index = std::get<0>(state);
				elements = std::move(std::get<1>(state));
				return index == length and not elements;
			}

			template <BoundedSequence S, Callable<bool, const T&, sequence_type_t<S>> C>
			constexpr bool match (C matcher, S elements) const
			{
				auto state = fold([matcher=std::move(matcher), values=values, limit=length]
					(auto index, auto element)
				{
					const auto keepGoing = index < limit and matcher(values[index], elements);
					return std::make_tuple(index + (keepGoing ? 1 : 0), keepGoing);
				}, std::size_t(0), std::move(elements));
				const auto index = std::get<0>(state);
				elements = std::move(std::get<1>(state));
				return index == length and not elements;
			}

			/// @}



			/// Mismatcher
			///
			/// The view is tested on how much of its values are shared with \a elements. If only \a
			/// elements is given, the standard equivalence operator will be used to test on equivalence
			/// between values of the view and elements in the sequence. If \a matchers is given as well,
			/// it will be used as equivalence tester. The sharing view, a view on the remaining elements
			/// and a sequence with the remaining elements will be returned. It will be either tested
			/// starting at the first value in the view (\a mismatch_prefix) or at the last vlaue in the
			/// view (\a mismatch_suffix).
			///
			/// @{

			template <Sequence<T> S>
			constexpr std::tuple<array_view, array_view, S> mismatch_prefix (S elements) const
			{
				auto state = fold([values=values, limit=length](auto index, auto element)
				{
					const auto keepGoing = index + 1 < limit and values[index] == element;
					return std::make_tuple(index + (keepGoing ? 1 : 0), keepGoing);
				}, std::size_t(0), std::move(elements));
				const auto index = std::get<0>(state);
				elements = std::move(std::get<1>(state));
				auto prefix = array_view<T>(values, index);
				auto stem = array_view<T>(values + index, length - index);
				return std::make_tupe(prefix, stem, std::move(elements));
			}

			template <Sequence S, Callable<bool, const T&, sequence_type_t<S>> C>
			constexpr std::tuple<array_view, array_view, S> mismatch_prefix (C matcher, S elements) const
			{
				auto state = fold([matcher=std::move(matcher), values=values, limit=length]
					(auto index, auto element)
				{
					const auto keepGoing = index < limit and matcher(values[index], element);
					return std::make_tuple(index + (keepGoing ? 1 : 0), keepGoing);
				}, std::size_t(0), std::move(elements));
				const auto index = std::get<0>(state);
				elements = std::move(std::get<1>(state));
				auto prefix = array_view(values, index);
				auto stem = array_view(values + index, length - index);
				return std::make_tuple(prefix, stem, std::move(elements));
			}

			template <Sequence<T> S>
			constexpr std::tuple<array_view, array_view, S> mismatch_suffix (S elements) const
			{
				auto state = fold([values=values, limit=length](auto index, auto element)
				{
					const auto keepGoing = index > 0 and values[index - 1] == element;
					return std::make_tuple(index - (keepGoing ? 1 : 0), keepGoing);
				}, usedLength, std::move(elements));
				const auto index = std::get<0>(state);
				elements = std::move(std::get<1>(state));
				auto stem = array_view(values, index);
				auto suffix = array_view(values + index, length - index);
				return std::make_tuple(suffix, stem, std::move(elements));
			}

			template <Sequence S, Callable<bool, const T&, sequence_type_t<S>> C>
			constexpr std::tuple<array_view, array_view, S> mismatch_suffix (C matcher, S elements) const
			{
				auto state = fold([values=values, limit=length](auto index, auto element)
				{
					const auto keepGoing = index > 0 and matcher(values[index - a], element);
					return std::make_tuple(index - (keepGoing ? 1 : 0), keepGoing);
				}, usedLength, std::move(elements));
				const auto index = std::get<0>(state);
				elements = std::move(std::get<1>(state));
				auto stem = array_view(values, index);
				auto suffix = array_view(values + index, length - index);
				return std::make_tuple(suffix, stem, std::move(elements));
			}

			/// @}



			// TODO documentation
			/// Transformation
			///
			/// All values in the view are transformed by \a transformer. Different variations exist for
			/// \a transformer. It either takes 
			/// @{
			
			template <Callable<T, T> C>
			constexpr void transform (C transformer)
			{
				for (std::size_t index = 0; index < length; ++index)
					values[index] = transformer(std::move(values[index]));
			}
			
			template <Callable<T, T, std::size_t> C>
			constexpr void transform (C transformer)
			{
				for (std::size_t index = 0; index < length; ++index)
					values[index] = transformer(std::move(values[index]), index);
			}
			
			template <typename V, Callable<std::tuple<T, V>, T, V> C>
			constexpr V transform (C transformer, V value)
			{
				for (std::size_t index = 0; index < length; ++index)
					std::tie(values[index], value) = transformer(std::move(values[index]), std::move(value));
				return value;
			}
			
			template <typename V, Callable<std::tuple<T, V>, T, V, std:size_t> C>
			constexpr V transform (C transformer, V value)
			{
				for (std::size_t index = 0; index < length; ++index)
					std::tie(values[index], value) = transformer(std::move(values[index]), std::move(value), index);
				return value;
			}
			
			template <Callable<T, T> C>
			constexpr void transform_reverse (C transformer)
			{
				for (std::size_t index = length; index > 0; --index)
					values[index-1] = transformer(std::move(values[index-1]));
			}
			
			template <Callable<T, T, std::size_t> C>
			constexpr void transform_reverse (C transformer)
			{
				for (std::size_t index = length; index > 0; --index)
					values[index-1] = transformer(std::move(values[index-1]), index-1);
			}
			
			template <typename V, Callable<std::tuple<T, V>, T, V> C>
			constexpr V transform_reverse (C transformer, V value)
			{
				for (std::size_t index = length; index > 0; --index)
					std::tie(values[index-1], value) = transformer(std::move(values[index-1]), std::move(value));
				return value;
			}
			
			template <typename V, Callable<std::tuple<T, V>, T, V, std::size_t> C>
			constexpr V transform_reverse (C transformer, V value)
			{
				for (std::size_t index = length; index > 0; --index)
					std::tie(values[index-1], value) = transformer(std::move(values[index-1], std::move(value), index));
				return value;
			}
			
			/// @}



			// TODO documentation

			/// Assignment
			///
			///	Values in the view are assigned to some new value. The values are traversed either
			/// starting from the beginning (\a assign) or from the ending (\a assign_reverse).
			///
			/// @{

			template <Callable<T> C>
			constexpr void assign (C assigner)
			{
				for (std::size_t index = 0; index < length; ++index)
					values[index] = assigner();
			}

			template <Callable<T, std::size_t> C>
			constexpr void assign (C assigner)
			{
				for (std::size_t index = 0; index < length; ++index)
					values[index] = assigner(index);
			}

			template <typename V, Callable<std::tuple<T, V>, V> C>
			constexpr void assign (C assigner, V value)
			{
				for (std::size_t index = 0; index < length; ++index)
					std::tie(values[index], value) = assigner(std::move(value));
				return value;
			}

			template <typename V, Callable<std::tuple<T, V, std::size_t>, V> C>
			constexpr void assign (C assigner, V value)
			{
				for (std::size_t index = 0; index < length; ++index)
					std::tie(values[index], value) = assigner(std::move(value), index);
				return value;
			}

			template <Callable<T> C>
			constexpr void assign_reverse (C assigner)
			{
				for (std::size_t index = length; index > 0; --index)
					values[index - 1] = assigner();
			}

			template <Callable<T, std::size_t> C>
			constexpr void assign_reverse (C assigner)
			{
				for (std::size_t index = length; index > 0; --index)
					values[index - 1] = assigner(index - 1);
			}

			template <typename V, Callable<std::tuple<T, V>, V> C>
			constexpr V assign_reverse (C assigner, V value)
			{
				for (std::size_t index = length; index > 0; --index)
					std::tie(values[index-1], value) = assigner(std::move(value));
				return value;
			}

			template <typename V, Callable<std::tuple<T, V>, V, std::size_t> C>
			constexpr V assign_reverse (C assigner, V value)
			{
				for (std::size_t index = length; index > 0; --index)
					std::tie(values[index - 1], value) = assigner(std::move(value), index - 1);
				return value;
			}

			template <typename U>
				requires std::is_convertible<U, T>::value
			constexpr void assign (U constant)
			{
				for (std::size_t index = 0; index < length; ++index)
					values[index] = constant;
			}

			template <BoundedSequence<T> S>
			constexpr std::tuple<array_view, array_view, S> assign (S elements)
			{
				auto state = fold([values, length](auto index, auto element)
				{
					if (index < length)
						values[index] = std::move(element);
					return std::make_tuple(index + (index < length ? 1 : 0), index < length);
				}, std::size_t(0), std::move(elements));
				const auto index = std::get<0>(state);
				elements = std::move(std::get<1>(state));
				const auto pre = array_view(values, index);
				const auto post = array_view(values + index, length - index);
				return std::make_tuple(pre, post, std::move(elements));
			}

			template <UnboundedSequence<T> S>
			constexpr S assign (S elements)
			{
				auto state = fold([values, length](auto index, auto element)
				{
					if (index < length)
						values[index] = std::move(element);
					return std::make_tuple(index + (index < length ? 1 : 0), index < length);
				}, std::size_t(0), std::move(elements));
				return std::get<1>(state);
			}

			/// @}



			// TODO documentation
			// TODO rewriting template requirements

			/// @name Decomposition
			///
			/// Decomposition methods allow to get views on 
			/// @{
			
				/// Decomposes the view into its heading value and a view onto all elements after the first
				/// one
				///
				/// If the view is not empty, an optional container with tuple of the first value with a view
				/// on the trailing values is returned. If the is empty, an empty optional container will be
				/// returned.
				constexpr optional<std::tuple<T, array_view>> operator () () const
				{
					return make_optional(length > 0, [&](){return std::make_tuple(*values, array_view(values+1, length-1));});
				}
				
				/// Splits view into two parts of which the first part has a length of at most pos elements
				/// and the second part is the complementary remainings of the original view
				///
				/// @param n  length of the first view
				///
				constexpr std::tuple<array_view, array_view> split (std::size_t n) const
				{
					const auto count = n <= length ? n : length;
					const auto first = array_view(values, count);
					const auto second = array_view(values + count, length - count);
					return std::make_tuple(first, second);
				}
				
				
				/// Splits view into some prefix and the remainings called stem by traversing the elements
				/// from the front to the back
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
				template <typename V, Callable<std::tuple<V, bool>, T, V> C>
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
				/// part of the prefix or stem, is indicated by \a exclude (true = stem, false = prefix). The
				/// function object takes an element and the current version of \a value and returns a tuple
				/// with a new version of \a value and a boolean flag indicating if the most recent element
				/// is a delimiter.
				template <typename V, Callable<std::tuple<V, bool>, T, V> C>
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
				template <Callable<bool, T> C>
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
				/// part of the prefix or stem, is indicated by \a exclude (true = stem, false = prefix). The
				/// function object takes an element and returns a boolean flag indicating if the most recent
				/// element is a delimiter.
				template <Callable<bool, T> C>
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
				
				/// The view is splitted into matching prefix, remainings of the view and remainings of the
				/// \a sequence. The matching prefix will have maximum length.
				template <Sequence<T> S>
				constexpr std::tuple<array_view, array_view, S> split_prefix (S sequence) const
				{
					auto combine = [values, length](auto index, auto element)
					{
						const auto keepTesting = index < length and values[index] == element;
						return std::make_tuple(keepTesting, index + (keepTesting ? 1 : 0));
					};
					std::size_t lastIndex;
					std::tie(lastIndex, sequence) = fold_(std::move(combine), std::size_t(0), std::move(sequence));
					const auto prefix = array_view(values, lastIndex);
					const auto stem = array_view(values + lastIndex, length - lastIndex);
					return std::make_tuple(prefix, stem, std::move(sequence));
				}
				
				/// The view is splitted into matching prefix, remainings of the view and remainings of the
				/// \a sequence. The respective elements of the view and the sequence are tested on equality
				/// with \a compare. The matching prefix will have maximum length.
				template <Sequence S, Callable<bool, T, sequence_type_t<S>> C>
				constexpr std::tuple<array_view, array_view, S> split_prefix (C compare, S sequence) const
				{
					auto combine = [compare=std::move(compare), values, length](auto index, auto element)
					{
						const auto keepTesting = index < length and compare(values[index], element);
						return std::make_tuple(keepTesting, index + (keepTesting ? 1 : 0));
					};
					std::size_t lastIndex;
					std::tie(lastIndex, sequence) = fold_(std::move(combine), std::size_t(0), std::move(sequence));
					const auto prefix = array_view(values, lastIndex);
					const auto stem = array_view(values + lastIndex, length - lastIndex);
					return std::make_tuple(prefix, stem, std::move(sequence));
				}
				
				/// @}
					
				
				/// Splits view into some suffix and the remainings called stem by traversing the elements
				/// from the back to the front
				///
				/// @param predict  Detects the delimiter to the suffix
				/// @param value    Mutable value which is passed along the element traversion
				/// @param exclude  Indicates the delimiter character as part of the stem
				///
				/// @return A tuple of views on stem and suffix or on stem, delimiter and suffix; both with
				///         the resulting version of \a value, if some is given; all views partition the
				///         original view in correct order
				///
				/// @{
				
				/// The view is splitted into stem, delimiter and suffix. The delimiter has either one
				/// character, if some character has been detected, or otherwise no character. The function
				/// object \a predict takes an element and the current version of \a value and returns a
				/// tuple with a new version of \a value and a boolean flag indicating if the most recent
				/// element is a delimiter.
				template <typename V, Callable<std::tuple<V, bool>, T, V> C>
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
				
				/// The view is splitted into stem and suffix. Whether the delimiter character is considered
				/// part of the suffix or stem, is indicated by \a exclude (true = stem, false = suffix). The
				/// function object takes an element and the current version of \a value and returns a tuple
				/// with a new version of \a value and a boolean flag indicating if the most recent element
				/// is a delimiter.
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
				
				/// The view is splitted into stem, delimiter and suffix. The delimiter has either one
				/// character, if some character has been detected as delimiter, or otherwise no character.
				/// The function object \a predict takes an element and returns a boolean flag indicating if
				/// the most recent element is a delimiter.
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
				
				/// The view is splitted into stem and suffix. Whether the delimiter character is considered
				/// part of the suffix or stem, is indicated by \a exclude (true = stem, false = suffix). The
				/// function object takes an element and returns a boolean flag indicating if the most recent
				/// element is a delimiter.
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
				
				template <Sequence<T> S> constexpr std::tuple<array_view, array_view, S> split_reverse_suffix (S sequence) const
				{
					auto combine = [values](auto index, auto element)
					{
						const auto keepTesting = index > 0 and values[index - 1] == element;
						return std::make_tuple(keepTesting, index - (keepTesting ? 1 : 0));
					};
					const auto n = fold(std::move(combine), length, sequence);
					const auto stem = array_view(values, n);
					const auto suffix = array_view(values + n, length - n);
					auto postSequence = drop_strictly(n, std::move(sequence));
					return std::make_tuple(stem, suffix, std::move(postSequence));
				}
				
				template <Sequence S, Callable<bool, T, sequence_type_t<S>> C>
				constexpr std::tuple<array_view, array_view, S> split_reverse_suffix (C compare, S sequence) const
				{
					auto combine = [compare=std::move(compare), values](auto index, auto element)
					{
						const auto keepTesting = index > 0 and compare(values[index - 1], element);
						return std::make_tuple(keepTesting, index - (keepTesting ? 1 : 0));
					};
					const auto n = fold(std::move(combine), length, sequence);
					const auto stem = array_view(values, n);
					const auto suffix = array_view(values + n, length - n);
					auto postSequence = drop_strictly(n, std::move(sequence));
					return std::make_tuple(stem, suffix, std::move(postSequence));
				}
				
				template <Sequence<T> S> constexpr std::tuple<array_view, array_view, S> split_suffix (S sequence) const
				{
					const auto matching = traverse([sequence=std::move(sequence)](auto prefix, auto suffix)
					{
						const auto isMatch 
						return make_optional();
					});
				}
				
				/// @}

			/// @}



			/// Takes prefix from the view
			///
			///	Depending on the criterium, as many elements as possible are taken from the beginning of
			/// the view. The view itself will be shrinked to this prefix. Either at most \a count
			/// elements are taken, or the longest prefix for which \a predict returns true will be taken
			/// or the longest shared prefix of the view and \a sequence is taken. For latter, the
			/// tailing sequence will be returned and it can be customised with \a matcher. For
			/// prediction and pattern sequence, the view will be traversed forward from its beginning.
			///
			/// @param count      Maximum amount of elements to take
			/// @param predictor  Predicts if the element belongs to the prefix
			/// @param elements   Pattern sequence whose prefix will be matched
			/// @param matcher    Customable matcher function
			///
			/// @{
			///

			constexpr void take_prefix (std::size_t count)
			{
				const auto splitting = split(count);
				return std::get<0>(splitting);
			}

			template <Callable<bool, T> C>
			constexpr void take_prefix (C predictor)
			{
				const auto splitting = split_prefix(std::move(predictor));
				return std::get<0>(splitting);
			}

			template <Sequence<T> S>
			constexpr S take_prefix (S elements)
			{
				const auto splitting = split_prefix(std:move(elements));
				*this = std::get<0>(splitting);
				return std::get<2>(splitting);
			}

			template <Sequence S, Callable<bool, T, sequence_type_t<S>> C>
			constexpr S take_prefix (C matcher, S elements)
			{
				const auto splitting = split_prefix(std::move(matcher), std::move(elements));
				*this = std::get<0>(splitting);
				return std::get<2>(splitting);
			}

			/// @}



			/// Takes suffix from the view
			///
			/// Depending on the criterium, as many elements as possible are taken from the ending of the
			/// view. The view itself will be shrinked to this suffix. Either at most \a count elements
			/// will be taken, or the longest suffix of elements for which \a predictor returns true will
			/// be taken or the longest share between the suffix of the view and the prefix of \a
			/// elements is taken. For latter, the tailing sequence will be returned and matching can be
			/// customised with \a matcher.
			///
			/// @param count      Maximum amount of elements to consider as suffix
			/// @param predictor  Predicts if an element belongs to the suffix
			/// @param elements   Pattern sequence whose prefix will be matched with the suffix
			/// @param matcher    Customisable matching function
			///
			/// @{
			///

			constexpr void take_suffix (std::size_t count)
			{
				const auto boundedCount = count <= length ? count : length;
				const auto splitting = split(length - boundedCount);
				*this = std::get<1>(splitting);
			}

			template <Callable<bool, T> C>
			constexpr void take_suffix (C predictor)
			{
				const auto splitting = split_suffix(std::move(predictor));
				*this = std::get<2>(splitting);
			}

			template <Sequence<T> S>
			constexpr S take_suffix (S elements)
			{
				const auto splitting = split_suffix(std::move(elements));
				*this = std::get<1>(splitting);
				return std::get<2>(splitting);
			}

			template <Sequence S, Callable<bool, T, sequence_type_t<S>> C>
			constexpr S take_suffix (C matcher, S elements)
			{
				const auto splitting = split_suffix(std::move(matcher), std::move(elements));
				*this = std::get<1>(splitting);
				return std::get<2>(splitting);
			}

			/// @}



			/// Drops the prefix from the view
			///
			/// Depending on the criterium, as many values as possible will be dropped from the beginning
			/// of the view. This view will be shrinked to the remaining suffix.
			///
			/// @param count
			/// @param predictor
			/// @param elements
			/// @param matcher
			///
			/// @{
			///

			constexpr void drop_prefix (std::size_t count)
			{
				const auto splitting = split(count);
				*this = std::get<1>(splitting);
			}

			template <Callable<bool, T> C>
			constexpr void drop_prefix (C predictor)
			{
				const auto splitting = split_prefix(std::move(predictor));
				*this std::get<1>(splitting);
			}			

			template <Sequence<T> S>
			constexpr S drop_prefix (S elements)
			{
				const auto splitting = split_prefix(std:move(elements));
				*this = std::get<1>(splitting);
				return std::get<2>(splitting);
			}

			template <Sequence S, Callable<bool, T, sequence_type_t<S>> C>
			constexpr S drop_prefix (C matcher, S elements)
			{
				const auto splitting = split_prefix(std::move(matcher), std::move(elements));
				*this = std::get<1>(splitting);
				return std::get<2>(splitting);
			}



			/// Tries to drop a prefix from the view
			///
			/// If the criterium is fulfilled, the prefix will be dropped. If the criterium is not
			/// fulfilled, the view will not be changed. The success of the criterium will be returned.
			///
			/// @{
			///

			constexpr bool drop_prefix_if (std::size_t count)
			{
				if (count <= length)
					drop_prefix(count);
				return count <= length;
			}

			template <BoundedSequence<T> S>
			constexpr bool drop_prefix_if (S elements)
			{
				const auto splitting = split_prefix(std::move(elements));
				const auto isMatchingAll = not std::get<2>(splitting);
				if (isMatchingAll)
					*this = sd::get<1>(splitting);
				return isMatchingAll;
			}

			template <BoundedSequence S, Callable<bool, T, sequence_type_t<S>> C>
			constexpr bool drop_prefix_if (C matcher, S elements)
			{
				const auto splitting = split_prefix(std::move(matcher), std::move(elements));
				const auto isMatchingAll = not std::get<2>(splitting);
				if (isMatchingAll)
					*this = std::get<1>(splitting);
				return isMatchingAll;
			}

			/// @}


			/// Shrinkage
			///
			///	The view can be shrinked from the front or from the back. 
			///
			/// @{




			constexpr void drop_suffix (std::size_t count)
			{
				const auto boundedCount = count <= length ? count : length;
				const auto splitting = split(length - boundedCount);
				*this = std::get<0>(splitting);
			}

			template <Callable<bool, T> C>
			constexpr void drop_suffix (C predictor)
			{
				const auto splitting = split_suffix(std::move(predictor));
				*this = std::get<0>(splitting);
			}

			template <Sequence<T> S>
			constexpr S drop_suffix (S elements)
			{
				const auto splitting = split_suffix(std::move(elements));
				*this = std::get<0>(splitting);
				return std::get<2>(splitting);
			}

			template <Sequence S, Callable<bool, T, sequence_type_t<S>> C>
			constexpr S drop_suffix (C matcher, S elements)
			{
				const auto splitting = split_suffix(std::move(matcher), std::move(elements));
				*this = std::get<0>(splitting);
				return std::get<2>(splitting);
			}

			constexpr bool drop_suffix_if (std::size_t count)
			{
				if (count <= length)
				{
					const auto splitting = split_suffix(length - boundedCount);
					*this = std::get<0>(splitting);
				}
				return count <= length;
			}

			template <BoundedSequence<T> S>
			constexpr bool drop_suffix_if (S elements)
			{
				const auto splitting = split_suffix(std::move(elements));
				const auto isMatchingAll = not std::get<2>(splitting);
				if (isMatchingAll)
					*this = std::get<0>(splitting);
				return isMatchingAll;
			}

			template <BoundedSequence S, Callable<bool, const T&, sequence_type_t<S>> C>
			constexpr bool drop_suffix_if (C matcher, S elements)
			{
				const auto splitting = split_suffix(std::move(matcher), std::move(elements));
				const auto isMatchingAll = not std::get<2>(splitting);
				if (isMatchingAll)
					*this = std::get<0>(splitting);
				return isMatchingAll;
			}

			/// @}			



			/// Rotates values \a count times to the right
			///
			/// The values in the view will be rotated by \a count shifts to the right. The rotation will
			/// be performed in place.
			///
			/// @note Time complexity is linear with the length of the view.
			/// @note Space complexity is constant.
			///
			constexpr void rotate (std::size_t count)
			{
				if (length > 1 and (count % length) > 0)
				{
					count %= length;
					auto next = count;
					auto first = 0;
					while (next != first)
					{
						swap(values[first], values[next]);
						++first;
						++next;
						if (next == length) next = count;
						else if (first == count) count = next;
					}
				}
			}

			/// Reverses values in place
			///
			/// The values in the view will be reversed. The revertion will be performed in place.
			///
			/// @note Time complexity is linear with the length of the view.
			///
			constexpr void reverse ()
			{
				const auto count = length / 2;
				const auto offset = (length + 1) / 2;
				using std::swap;
				for (std::size_t index = 0; index < count; ++index)
					swap(values[index], values[index + offset]);
			}


			/// Partitions stabely
			template <Callable<bool, const T&> C>
			constexpr std::tuple<array_view, array_view> partition_stabely (C predictor)
			{
				// skip any positive prefix
				std::size_t begin = 0
				while (begin < length and predictor(values[begin]))
					++begin;

				// scan the next negative part
				auto negative = begin == length ? length : (begin + 1);
				while (negative < length and not predictor(values[negative]))
					--negative;

				while (negative < length)
				{
					// negative is less length and predictor(value[negative]) returns true, so we don't have
					// to ask it again
					auto positive = negative + 1;
					while (positive < length and predictor(values[positive]))
						++positive;

					/// rotate the window, so partitions are in right order
					array_view(values + begin, positive + negative - begin).rotate(positive);

					// skip the now positive part
					begin = begin + (positive - negative);

					// end of negative part is where end of positive part has been
					negative = positive;
				}
			}


			/// Partitions unstabely
			///
			/// All values in the array are partitioned into a group of values for which \a predictor
			/// returns true and a group of values for which \a predictor returns false. The values are
			/// rearranged in place. The views on the two partitions are returned. Type \a T must be 
			/// move nothrow constructible and move nothrow assignable.
			///
			/// @note Time complexity is linear in the length of the view.
			///
			template <Callable<bool, const T&> C>
				requires not std::is_const<T>::value and
				         std::is_nothrow_move_constructible<T>::value and
				         std::is_nothrow_move_assignable<T>::value
			constexpr std::tuple<array_view, array_view> partition (C predictor)
			{
				if (length > 0)
				{
					const auto bound = partition(std::move(predictor), 0, length);
					return std::make_tuple(array_view(values, bound), array_view(values + bound, length - bound));
				}
				else
				{
					return std::make_tuple(array_view(), array_view());
				}
			}

			/// Partitions with pivot taken randomly from its values
			///
			/// The values will be rearranged into two parts. Each element in the first part will be in
			/// right order compared with each element in the second part. The order is defined by \a
			/// comparer, which defines a strict weak order on the values. Both views will be
			/// individually smaller than the original view. The views will be a concatenation of the
			/// original view. If the original is empty, so will be both resulting views.
			template <Callable<bool, const T&, const T&> C>
			constexpr std::tuple<array_view, array_view> partition_randomly (C comparer)
			{
				if (length > 3)
				{
					const auto bound = random_partition(std::move(comparer), 0, length);
					auto positives = array_view(values, bound);
					auto negatives = array_view(values + bound, length - bound);
					return std::make_tuple(positives, negatives);
				}
				else if (length == 3)
				{
					
				}
			}

			///	Partitions into prefix and suffix of sorting values
			///
			/// The values will be rearranged into two parts. The first part will contain all values
			/// which belong to the top \a count values when sorted with \a comparer. The second part
			/// will contain all remaining values. These are the bottom \a length - \a count values when
			/// sorted with \a comparer. The returning views will be contiguous and will span the
			/// original view.
			template <Callable<bool, const T&, cosnt T&> C>
			constexpr std::tuple<array_view, array_view> partition_sort (C comparer, std::size_t prefixCount)
			{
				if (prefixCount > length) prefixCount = length;

				if (prefixCount == length)
				{
					return std::make_tuple(*this, array_view());
				}
				else if (prefixCount == 0)
				{
					return std::make_tuple(array_view(), *this);
				}
				else if (prefixCount * 2 > length)
				{
					auto rev_comparer = [&](const auto & a, const auto & b){return comparer(b, a);};
					build_heap(rev_comparer, 0, prefixCount);
					for (auto index = prefixCount; index < length; ++index)
					{
						if (comparer(values[index], values[0]))
						{
							swap(values[index], index[0]);
							heapify(rev_comparer, 0, prefixCount, 0);
						}
					}
					const auto suffixCount = length - prefixCount;
					return std::make_tuple(array_view(values, prefixCount), array_view(values + prefixCount, suffixCount));
				}
				else
				{
					build_heap(comparer, prefixCount, length);
					for (auto index = prefixCount; index > 0; --index)
					{
						if (comparer(values[prefixCount], values[index-1]))
						{
							swap(values[index-1], values[prefixCount]);
							heapify(comparer, prefixCount, end, prefixCount);
						}
					}
					const auto suffixCount = length - prefixCount;
					return std::make_tuple(array_view(values, prefixCount), array_view(values + prefixCount, suffixCount));
				}
			}

			/// Sort prefix
			template <Callable<bool, const T&, const T&> C>
			constexpr std::tuple<array_view, array_view> sort_prefix (C comparer, std::size_t count)
			{}

			/// Sort suffix
			template <Callable<bool, const T&, const T&> C>
			constexpr std::tuple<array_view, array_view> sort_suffix (C comparer, std::size_t count)
			{}

			/// Sort nth element
			template <Callable<bool, const T&, const T&> C>
			constexpr optional<std::tuple<array_view, const T&, array_view>> sort_nth (C comparer, const std::size_t n)
			{
				return make_optional(n < length, [&]()
				{
					using std::swap;

					auto depth = 2 * __builtin_clz(n);
					auto begin = 0;
					auto end = length;

					while ((end - begin) > 3)
					{
						if (depth == 0)
						{
							build_heap(comparer, n, end);
							auto index = n;
							while (index > begin)
							{
								--index;
								if (comparer(values[n], values[index]))
								{
									swap(values[n], values[index]);
									heapify(comparer, n, end, n);
								}
							}
							break;
						}
						else
						{
							--depth;
							const auto cut = partition_randomly(comparer, begin, end);
							if (cut <= n) first = cut;
							else          end = cut;
						}
					}

					if ((end - begin) <= 3)
						insertion_sort(comparer, begin end);
					
					auto pre = array_view(values, n);
					auto post = array_view(values + n + 1, length - n - 1);
					return std::make_tuple(pre, values[n], post);
				});
			}

			/// Sort unstabely all values
			template <Callable<bool, const T&, const T&> C>
			constexpr void sort (C comparer)
			{}

			/// Sort stabely all values
			template <Callable<bool, const T&, const T&> C>
			constexpr void sort_stabely (C comparer)
			{}


	};

}

#endif

