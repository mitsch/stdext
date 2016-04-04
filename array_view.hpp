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

		T * values = nullptr;
		size_t length = 0;


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

		/// Reverts values from \a begin to \a end
		constexpr void reverse (std::size_t begin, std::size_t end)
		{
			using std::swap;
			--end;
			while (begin < end)
			{
				swap(values[begin], values[end]);
				++begin;
				--end;
			}
		}

		/// Rotates values from \a begin to \a end \a count times
		constexpr void rotate (std::size_t begin, std::size_t end, std::size_t middle)
		{
			assert(begin <= middle);
			assert(middle <= end);
			assert(end <= length);

			const auto _length = end - begin;
			const auto count  = middle - begin;
			if ((end - begin) > 1 and middle > begin and end > middle)
			{
				auto next = middle;
				while (next != begin)
				{
					swap(values[begin], values[next]);
					++begin;
					++next;
					if (next == end) next = middle;
					else if (begin == middle) middle = next;
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

		/// Min heap selection
		///
		/// Within the range \a begin to \a middle, a max heap will be built which will contain the
		/// \a middle - \a begin lowest values of the range \a begin to \a end. The order is given
		/// by \a comparer which defines a strict weak ordering.
		template <Callable<bool, const T&, const T&> C>
		constexpr void min_heap_select (C comparer, std::size_t begin, std::size_t middle, std::size_t end)
		{
			assert(begin < middle);
			assert(middle <= end);
			assert(end <= length);

			auto rev_comparer = [&](const auto & b, const auto & b){return comparer(b, a);};
			build_heap(rev_comparer, begin, middle);
			for (auto index = middle; index < end; ++index)
			{
				if (comparer(values[index], values[begin]))
				{
					swap(values[index], values[begin]);
					heapify(rev_comparer, begin, middle, begin);
				}
			}
		}

		/// Max heap selection
		///
		/// Within the range \a middle to \a end, a min heap will be built which will contain the
		/// \a end - \a middle biggest values of the range \a begin to \a end. The order is given by
		/// \a comparer which defines a strict weak ordering.
		template <Callable<bool, const T&, const T&> C>
		constexpr void max_heap_select (C comparer, std::size_t begin, std::size_t middle, std::size_t end)
		{
			assert(begin <= middle);
			assert(middle < end);
			assert(end <= length);

			build_heap(comparer, middle, end);
			for (auto index = begin; index < middle; ++index)
			{
				if (comparer(values[middle], values[index]))
				{
					swap(values[index], values[middle]);
					heapify(comparer, middle, end, middle);
				}
			}
		}

		/// Heap selection
		template <Callable<bool, const T&, cosnt T&> C>
		constexpr void heap_select (C comparer, std::size_t begin, std::size_t end, std::size_t n)
		{
			assert(begin <= n);
			assert(n < end);
			assert(end <= length);

			using std::swap;

			const auto middle = begin + (end - begin) / 2;
			if (n < middle)
			{
				max_heap_select(std::move(comparer), begin, n, end);
			}
			else
			{
				min_heap_select(std::move(comparer), begin, n+1, end);
				swap(values[begin], values[n]);
			}
		}

		/// Heap popping
		///
		/// All values in the heap structure spanning from \a begin to \a end are popped and placed
		/// in reverse order given by \a comparer. Eventually, all values will be sorted in place in
		/// reverse order.
		template <Callable<bool, const T&, const T&> C>
		constexpr void pop_total_heap (C comparer, std::size_t begin, std::size_t end)
		{
			assert(begin <= end);
			assert(end <= length);

			using std::swap;

			auto index = end;
			while (index - begin > 3)
			{
				--index;
				swap(values[begin], values[begin+1]);
				heapify(comparer, begin, index, begin);
			}


			if (index - begin == 3)
			{
				if (comparer(values[begin+1], values[begin+2]))
				{
					swap(values[begin], values[begin+2]);
				}
				else
				{
					auto tmp = std::move(values[begin]);
					values[begin] = std::move(values[begin+1]);
					values[begin+1] = std::move(values[begin+2]);
					values[begin+2] = std::move(tmp);
				}
			}
			else if (index - begin == 2)
			{
				swap(values[begin], values[begin+1]);
			}
		}

		template <Callable<bool, const T&, const T&> C>
		constexpr intro_select (C comparer, std::size_t begin, std::size_t end, std::size_t limit, std::size_t n)
		{
			assert(begin <= end);
			assert(end <= length);
			assert(begin <= n);
			assert(n < end);

			using std::swap;

			while (end - begin > 3)
			{
				if (limit == 0)
				{
					heap_select(comparer, begin, end, n);
					return;
				}
				else
				{
					--limit;
					const auto cut = partition_randomly(comparer, begin, end);
					if (cut <= n) begin = cut;
					else          end = cut;
				}
			}
			insertion_sort(comparer, begin, end);
		}


		template <Callable<bool, const T&, const T&> C>
		constexpr void intro_sort (C comparer, std::size_t begin, std::size_t end, std::size_t limit, std::size_t minLength)
		{
			assert(begin <= end);
			assert(end <= length);

			while (end - begin > minLength)
			{
				if (limit == 0)
				{
					const auto rev_comparer = [&](const auto & a, const auto & b){return comparer(b, a);};
					build_heap(rev_comparer, begin, end);
					pop_heap(rev_comparer, begin, end);
				}
				else
				{
					--limit;
					const auto cut = partition_randomly(comparer, begin, end);
					intro_sort(comparer, begin, end, limit, minLength);
					end = cut;
				}
			}
		}


		template <Callable<bool, const T&, const T&> C>
		constexpr void merge_sort (C comparer, std::size_t begin, std::size_t end, std::size_t minLength)
		{
			if (end - begin > minLength)
			{
				const auto middle = begin + (end - begin) / 2;
				merge_sort(comparer, begin, middle, minLength);
				merge_sort(comparer, middle, end, minLength);
				merge_sorted(comparer, begin, middle, end);
				sort_merge_inplace(comparer, begin, middle, end);
			}
			else
			{
				insertion_sort(comparer, begin, end);
			}
		}


		template <Callable<bool, const T&, const T&> C>
		constexpr void sort_merge_inplace (C comparer, std::size_t begin, std::size_t middle, std::size_t end)
		{
			assert(begin <= middle);
			assert(middle <= end);
			assert(end <= length);

			using std::swap;

			while (begin < middle and middle < end)
			{

				// skip all values on the left part which are lower than the first on the right part
				while (begin < middle and not comparer(values[middle], values[begin]))
					++begin;

				// now swap from right to left as long the heading right element is less than the left one
				auto right = middle;
				while (right < end and begin < middle and not comparer(values[right], values[begin]))
				{
					swap(values[begin], values[right]);
					++begin;
					++right;
				}

				// handle three partitions until we got two again
				while (begin < middle and right < end)
				{
					if (not comparer(values[middle], values[begin]) and not comparer(values[right], values[begin]))
					{
						++begin;
					}
					else if (not comparer(values[begin], values[middle]) and not comparer(values[right], values[middle]))
					{
						auto tmp = std::move(values[begin]);
						values[begin] = std::move(values[middle]);
						auto index = middle;
						while (index + 1 < right and comparer(values[index + 1], tmp))
						{
							values[index] = std::move(values[index + 1];
							++index;
						}
						values[index] = std::move(tmp);
						++begin;
					}
					else
					{
						auto tmp = std::move(values[begin]);
						values[begin] = std::move(values[right]);
						auto index = right;
						while (index > middle and comparer(tmp, values[index - 1]))
						{
							values[index] = std::move(values[index - 1];
							--index;
						}
						values[index] = std::move(tmp);
						++begin;
						++right;
					}
				}

				if (begin == middle) middle = right;
			}
		}


		/// Partitions with pivot taken randomly from the values
		template <Callable<bool, const T&, const T&> C>
		constexpr std::size_t partition_randomly (C comparer, std::size_t begin, std::size_t end)
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


	public:

		/// Underlying type of the array view
		using value_type = T;


		/// Attribute constructor
		///
		/// The view will be on \a values with the next \a count elements. If \a values is nullptr,
		/// \a count has to be zero.
		constexpr array_view (T * values, const size_t count)
			noexcept
			: values(values), length(count)
		{
			assert(values != nullptr or count == 0);
		}

		constexpr array_view (std::initializer_list<T> values)
			noexcept
			: values(values.begin()), length(values.size())
		{}


		/// Returns whether the view is empty, that is it sees no elements
		constexpr bool empty () const
		{
			return length == 0;
		}

		/// Returns the amount of elements in the view
		constexpr size_t length () const
		{
			return length;
		}

		/// Swaps the view, not its elements, between \a first and \a second
		///
		/// After swapping, \a first will see all the elements which \a second has seen before the
		/// swapping, and vice versa. No element will be altered or changed.
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
		constexpr optional<T&> operator [] (size_t index)
		{
			return index < length ? optional<T&>(values + index) : optional<T&>();
		}

		/// Accessing
		///
		/// The element at \a index is tried to be accessed. If the \a index is in bound, the
		/// corresponding value will be passed by reference to \a hitter. If \a index is out of
		/// bound, \a misser will be called.
		template <typename C, typename D, typename ... As>
			requires Callable_<C, T&, As ...> and
			         Callable_<D, As ...> and
			         not std::is_void_v<result_of_t<C(T&, As ...)>> and
			         not std::is_void_v<result_of_t<D(As ...)>> and
							 requires(){typename std::common_type_t<result_of_t<C(T&, As ...)>, result_of_t<D(As ...)>>}
		constexpr auto at (C hitter, D misser, std::size_t index, As ... attributes) const
		{
			return index < length ? hitter(values[index], std::move(attributes) ...) : misser(std::move(attributes) ...);
		}



		/// Prefix decomposition
		///
		/// If the view is not empty, a reference to the next value will be returned along with a
		/// view for the succeeding values. If the view is empty, an empty optional container will be
		/// returned.
		///
		constexpr optional<std::tuple<T&, array_view>> decompose () const
		{
			using U = optional<std::tuple<T&, array_view>>;
			return length == 0 ? U() : U(std::make_tuple(*values, array_view(values + 1, length - 1)));
		}

		/// Suffix decomposition
		///
		/// If the view is not empty, a reference to the last value will be returned along with a
		/// view for the preceeding values. If the view is empty, an empty optional container will be
		/// returned.
		///
		constexpr optional<std::tuple<T&, array_view>> decompose_reverse () const
		{
			using U = optional<std::tuple<T&, array_view>>;
			return length == 0 ? U() : U(std::make_tuple(values[length - 1], array_view(values, length - 1)));
		}


		/// Complete forward folding
		///
		/// All values in the view are consecutively folded over \a value by \a combiner. The
		/// traversion direction is front to back. The completely folded value will be returned.
		///
		template <typename V, Callable<V, V, T> C>
		constexpr V fold (C combiner, V value) const
		{
			for (size_t index = 0; index < length; ++index)
			{
				value = combiner(std::move(value), values[index]);
			}
			return value;
		}

		/// Complete backward folding
		///
		/// All values in the view are consecutively folded over \a value by \a combiner. The
		/// traversion direction is back to front. The completely folded value will be returned.
		///
		template <typename V, Callable<V, V, T> C>
		constexpr V fold_reverse (C combiner, V value) const
		{
			auto index = length;
			while (index-- > 0)
			{
				value = combiner(std::move(value), values[index]);
			}
			return value;
		}

		/// Partial forward folding
		///
		/// All values in the view are consecutively folded over \a value by \a combiner. The
		/// traversion direction is front to back. The traversion stops with the first false flag
		/// returned by \a combiner.
		///
		template <typename V, Callable<std::tuple<V, bool>, V, T> C>
		constexpr std::tuple<V, array_view, array_view> fold (C combiner, V value) const
		{
			auto index = 0;
			auto keepOn = true;
			while (index < length)
			{
				std::tie(value, keepOn) = combiner(std::move(value), values[index]);
				if (keepOn) ++index;
			}
			return std::make_tuple(std::move(value), array_view(values, index), array_view(values + index, length - index));
		}

		/// Partial backward folding
		///
		/// All values in the view are consecutively folded over \a value by \a combiner. The
		/// traversion direction is back to front. The traversion stops with the first false flag
		/// returned by \a combiner.
		///
		template <typename V, Callable<std::tuple<V, bool>, V, T> C>
		constexpr std::tuple<V, array_view, array_view> fold_reverse (C combiner, V value) const
		{
			auto index = length;
			auto keepOn = true;
			while (index > 0 and keepOn)
			{
				--index;
				std::tie(value, keepOn) = combiner(std::move(value), values[index]);
				if (not keepOn) ++index;
			}
			return std::make_tuple(std::move(value), array_view(values, index), array_view(values + index, length - index));
		}





		/// Prefix splitting with count
		///
		/// The view will be splitted into two partitions. If the view has more than \a count values, the
		/// first partition will hold the first \a count values and the second partition will hold all
		/// succeeding values of the view. If the view has equal or less than \a count values, then the
		/// first partition will correspond to the view and the second partition will be empty.
		///
		constexpr std::tuple<array_view, array_view> split_prefix (size_t count) const
		{
			const auto maxCount = count < length ? count : length;
			const auto pre = array_view(values, maxCount);
			const auto post = array_view(values + maxCount, length - maxCount);
			return std::make_tuple(pre, post);
		}

		/// Prefix splitting with prediction
		///
		/// The view will be splitted into two partitions. The first partition will be the longest prefix
		/// of values in the view for which \a predictor returns true. The second partition will hold all
		/// succeeding values of the view.
		///
		template <Callable<bool, T> C>
		constexpr std::tuple<array_view, array_view> split_prefix (C predictor) const
		{
			auto index = size_t(0);
			while (index < length and predictor(values[index]))
			{
				++index;
			}
			const auto pre = array_view(values, index);
			const auto post = array_view(values + index, length - index);
			return std::make_tuple(pre, post);
		}

		/// Prefix splitting with sequence
		///
		/// The view will be splitted into two partitions. The first partition will be the longest
		/// sharing prefix with \a sequence and the second partition will be the succeeding values of the
		/// view. Along with both partitions, a sequence with the remaining elements will be returned.
		///
		template <Sequence<T> S>
		constexpr std::tuple<array_view, array_view, S> split_prefix (S sequence) const
		{
			auto folding = fold([&values](auto index, auto element)
			{
				const auto keepOn = index < length and values[index] == element;
				const auto nextIndex = index + keepOn ? 1 : 0;
				return std::make_tuple(nextIndex, keepOn);
			}, size_t(0), std::move(sequence));
			const auto pre = array_view(values, std::get<0>(folding));
			const auto post = array_view(values + std::get<0>(folding), length - std::get<0>(folding));
			return std::make_tuple(pre, post, std::move(std::get<1>(folding)));
		}

		/// Prefix splitting with sequence and matcher
		///
		/// The view will be splitted into two partitions. The first partition will be the longest
		/// sharing prefix with \a sequence and the second partition will be the succeeding values of the
		/// view. Along with both partitions, a sequence with the remaining elements will be returned.
		/// The values of the view and the elements of \a sequence will be matched by \a matcher.
		///
		template <Sequence S, Callable<bool, T, value_type> C>
		constexpr std::tuple<array_view, array_view, S> split_prefix (C matcher, S sequence) const
		{
			auto folding = fold([&values, &matcher](auto index, auto element)
			{
				const auto keepOn = index < length and matcher(values[index], element);
				const auto nextIndex = index + keepOn ? 1 : 0;
				return std::make_tuple(nextIndex, keepOn);
			}, size_t(0), std::move(sequence));
			const auto pre = array_view(values, std::get<0>(folding));
			const auto post = array_view(values + std::get<0>(folding), length - std::get<0>(folding));
			return std::make_tuple(pre, post, std::move(std::get<1>(folding)));
		}






		/// Suffix splitting with count
		///
		/// The view will be splitted into two partitions. If the view has more than \a count values, the
		/// second partition will hold the last \a count values and the first partition will hold all
		/// preceeding values of the view. If the view has equal or less than \a count values, then the
		/// second partition will correspond to the view and the first partition will be empty.
		///
		constexpr std::tuple<array_view, array_view> split_suffix (size_t count) const
		{
			const auto maxOffset = count < length ? (length - count) : 0;
			const auto pre = array_view(values, maxOffset);
			const auto post = array_view(values + maxOffset, length - maxOffset);
			return std::make_tuple(pre, post);
		}

		/// Suffix splitting with prediction
		///
		/// The view will be splitted into two partitions. The second partition will be the longest
		/// suffix of values in the view for which \a predictor returns true. The first partition will
		/// hold all preceeding values of the view.
		///
		template <Callable<bool, T> C>
		constexpr std::tuple<array_view, array_view> split_suffix (C predictor) const
		{
			auto index = length;
			while (index > 0 and predictor(values[index-1]))
			{
				--index;
			}
			const auto pre = array_view(values, index);
			const auto post = array_view(values + index, length - index);
			return std::make_tuple(pre, post);
		}

		/// Suffix splitting with sequence
		///
		/// The view will be splitted into two partitions. The second partition will be the longest
		/// sharing suffix with \a sequence and the first partition will be the preceeding values of the
		/// view. Along with both partitions, a sequence with the preceeding elements will be returned.
		///
		template <ReversibleBoundedSequence<T> S>
		constexpr std::tuple<array_view, array_view, S> split_suffix (S sequence) const
		{
			auto folding = fold_reverse([&values](auto index, auto element)
			{
				const auto keepOn = index > 0 and values[index-1] == element;
				const auto nextIndex = index - keepOn ? 1 : 0;
				return std::make_tuple(nextIndex, keepOn);
			}, length, std::move(sequence));
			const auto pre = array_view(values, std::get<0>(folding));
			const auto post = array_view(values + std::get<0>(folding), length - std::get<0>(folding));
			return std::make_tuple(pre, post, std::move(std::get<1>(folding)));
		}

		/// Suffix splitting with sequence and matcher
		///
		/// The view will be splitted into two partitions. The second partition will be the longest
		/// sharing suffix with \a sequence and the first partition will be the preceeding values of the
		/// view. Along with both partitions, a sequence with the preceeding elements will be returned.
		/// The values of the view and the elements of \a sequence will be matched by \a matcher.
		///
		template <ReversibleBoundedSequence S, Callable<bool, T, value_type> C>
		constexpr std::tuple<array_view, array_view, S> split_suffix (C matcher, S sequence) const
		{
			auto folding = fold_reverse([&values, &matcher](auto index, auto element)
			{
				const auto keepOn = index > 0 and matcher(values[index-1], element);
				const auto nextIndex = index - keepOn ? 1 : 0;
				return std::make_tuple(nextIndex, keepOn);
			}, size_t(0), std::move(sequence));
			const auto pre = array_view(values, std::get<0>(folding));
			const auto post = array_view(values + std::get<0>(folding), length - std::get<0>(folding));
			return std::make_tuple(pre, post, std::move(std::get<1>(folding)));
		}




		/// Prefix matching with sequence
		///
		/// The view will be tested on having \a sequence as its prefix. Matching is performed with the
		/// equivalence operator.
		///
		template <BoundedSequence<T> S>
		constexpr bool match_prefix (S sequence) const
		{
			const auto splitting = split_prefix(std::move(sequence));
			return empty(std::get<2>(splitting));
		}

		/// Prefix matching with sequence and matcher
		///
		/// The view will be tested on having \a sequence as its prefix. Matching is performed with \a
		/// matcher.
		///
		template <BoundedSequence S, Callable<bool, T, sequence_type_t<S>> C>
		constexpr bool match_prefix (C matcher, S sequence) const
		{
			const auto splitting = split_prefix(std::move(matcher), std::move(sequence));
			return empty(std::get<2>(splitting));
		}

		/// Suffix matching with sequence
		///
		/// The view will be tested on having \a sequence as its suffix. Matching is performed with the
		/// equivalence operator.
		///
		template <ReversibleBoundedSequence<T> S>
		constexpr bool match_suffix (S sequence) const
		{
			const auto splitting = split_suffix(std::move(sequence));
			return empty(std::get<2>(splitting));
		}

		/// Suffix matching with sequence and matcher
		///
		/// The view will be tested on having \a sequence as its suffix. Matching is performed with \a
		/// matcher.
		///
		template <ReversibleBoundedSequence S, Callable<bool, T, sequence_type_t<S>> C>
		constexpr bool match_suffix (C matcher, S sequence) const
		{
			const auto splitting = split_suffix(std::move(matcher), std::move(sequence));
			return empty(std::get<2>(splitting));
		}

		/// Complete matching with sequence
		///
		/// The view will be tested on matching exactly \a sequence. Matching values of the view and
		/// elements of \a sequence will be performed by the equivalence operator.
		///
		template <BoundedSequence<T> S>
		constexpr bool match (S sequence) const
		{
			const auto splitting = split_prefix(std::move(sequence));
			return std::get<1>(splitting).empty() and empty(std::get<2>(splitting));
		}

		/// Complete matching with sequence and matcher
		///
		/// The view will be tested on matching exactly \a sequence. Matching values of the view and
		/// elements of \a sequence will be performed by the equivalence operator.
		///
		template <BoundedSequence S, Callable<bool, T, sequence_type_t<S>> C>
		constexpr bool match (C matcher, S sequence) const
		{
			const auto splitting = split_prefix(std::move(matcher), std::move(sequence));
			return std::get<1>(splitting).empty() and empty(std::get<2>(splitting));
		}





		/// Forward breaking based on delimiter set
		///
		/// The view will be broken into two parts. The first part will be the preceeding part before
		/// the first occurrence of any element of \a sequence in the view. The second part will be the
		/// succeeding part after the first occurrence of any element of \a sequence in the view. The
		/// found element will be returned along with the first and the second part. The elements of \a
		/// sequence and the values of the view will be tested on equivalence with the default
		/// equivalence operator.
		///
		template <BoundedSequence<T> S>
		constexpr std::tuple<optional<T>, array_view, array_view> break_prefix (S sequence) const
		{
			// TODO test T on equivalence comparability
			for (size_t index = 0; index < length; ++index)
			{
				auto folding = sequence.fold([&index, &values](auto, auto element)
				{
					const auto isEquivalent = values[index] == element;
					return std::make_tuple(isEquivalent, not isEquivalent);
				}, false);
				if (std::get<0>(folding))
				{
					const auto pre = array_view(values, index);
					const auto post = array_view(values + index + 1, length - index - 1);
					return std::make_tuple(make_optional(values[index]), pre, post);
				}
			}
			return std::make_optional(optional<T>(), *this, array_view());
		}

		/// Forward breaking based on delimiter set with custom matcher
		///
		/// The view will be broken into two parts. The first part will be the preceeding part before
		/// the first occurrence of any element of \a sequence in the view. The second part will be the
		/// succeeding part after the first occurrence of any element of \a sequence in the view. The
		/// found element will be returned along with the first and the second part. The elements of \a
		/// sequence and the values of the view will be tested on equivalence with \a matcher.
		///
		template <BoundedSequence S, Callable<bool, T, sequence_type_t<S>> M>
		constexpr std::tuple<optional<sequence_type_t<S>>, array_view, array_view> break_prefix (M matcher, S sequence) const
		{
			for (size_t index = 0; index < length; ++index)
			{
				auto folding = sequence.fold([&index, &values, &matcher](auto, auto element)
				{
					const auto isEquivalent = matcher(values[index], element);
					return std::make_tuple(make_optional_if(isEquivalent, std::move(element)), not isEquivalent);
				}, optional<sequence_type_t>());
				if (not std::get<0>(folding).empty())
				{
					const auto pre = array_view(values, index);
					const auto post = array_view(values + index + 1, length - index - 1);
					return std::make_tuple(std::move(std::get<0>(folding)), pre, post);
				}
			}
			return std::make_optional(optional<sequence_type_t<S>>(), *this, array_view());
		}

		/// Forward breaking based on matcher
		///
		/// The view will be traversed from the beginning to the end. For each value, the \a matcher
		/// will be called with the value, a view onto the preceeding values and a view onto the
		/// succeeding values. The traversion will be stopped as soon as soon as \a matcher returns a
		/// non empty optional container. If \a matcher will only return empty optional container, an
		/// empty optional container will be returned.
		///
		template <Callable_<T, array_view, array_view> M>
			requires is_optional_v<result_of_t<M(T, array_view, array_view)>>
		constexpr result_of_t<M(T, array_view, array_view)> break_prefix (M matcher) const
		{
			for (size_t index = 0; index < length; ++index)
			{
				const auto pre = array_view(values, index);
				const auto post = array_view(values + index + 1, length - index - 1);
				auto matching = matcher(values[index], pre, post);
				if (not matching.empty())
				{
					return matching;
				}
			}
			return result_of_t<M(T, array_view, array_view)>();
		}

		/// Forward breaking based on delimiter sequence
		///
		/// The view will be broken into two parts. The first part will be the preceeding part before
		/// the first occurrence of \a sequence in the view. The second part will be the succeeding
		/// part after the first occurrence of \a sequence in the view. If the \a sequence is found, a
		/// true flag will be returned along with the first and the second part. If the \a sequence is
		/// not found, a false flag will be returned along with the original view and an empty view. If
		/// the \a sequence is empty, a true flag will be returned along with the original view and an
		/// empty view.
		///
		template <BoundedSequence S<T>>
		constexpr std::tuple<bool, array_view, array_view> break_sub_prefix (S sequence) const
		{
			// TODO there is a more efficient linear substring search, but simple approach is correct as well
			return decide([&](auto decomposition)
			{
				auto decompostion = break_prefix([&decomposition](auto && middle, array_view pre, array_view post)
				{
					if (middle == std::get<0>(decomposition))
					{
						auto splitting = post.split_prefix(std::get<1>(decomposition));
						if (std::get<2>(splitting).empty())
						{
							return make_optional(std::make_tuple(pre, std::get<1>(splitting)));
						}
						else
						{
							return optional<std::tuple<array_view, array_view>>();
						}
					}
					else
					{
						return optional<std::tuple<array_view, array_view>>();
					}
				});
				return decomposition.decide([](auto decomposition)
				{
					return std::tuple_cat(std::tie(true), decomposition);
				}, [&this]()
				{
					return std::make_tuple(false, *this, array_view());
				});
			}, [&this]()
			{
				return std::make_tuple(true, *this, array_view());
			}, sequence.decompose());
		}





		
		/// Backward breaking based on delimiter set
		///
		/// The view will be broken into two parts. The first part will be the preceeding part of the
		/// last occurrence of any element of \a sequence in the view. The second part will be the
		/// succeeding part of the last occurrence of any element of \a sequence in the view. The
		/// found element will be returned along with the first and the second part. The elements of \a
		/// sequence and the values of the view will be tested on equivalence with the default
		/// equivalence operator.
		///
		template <BoundedSequence<T> S>
		constexpr std::tuple<optional<T>, array_view, array_view> break_suffix (S sequence) const
		{
			// TODO test T on equivalence comparability
			size_t index = length;
			while (index-- > 0)
			{
				auto folding = sequence.fold([&index, &values](auto, auto element)
				{
					const auto isEquivalent = values[index] == element;
					return std::make_tuple(isEquivalent, not isEquivalent);
				}, false);
				if (std::get<0>(folding))
				{
					const auto pre = array_view(values, index);
					const auto post = array_view(values + index + 1, length - index - 1);
					return std::make_tuple(make_optional(values[index]), pre, post);
				}
			}
			return std::make_optional(optional<T>(), array_view(), *this);
		}

		/// Backward breaking based on delimiter set with custom matcher
		///
		/// The view will be broken into two parts. The first part will be the preceeding part of the
		/// last occurrence of any element of \a sequence in the view. The second part will be the
		/// succeeding part of the last occurrence of any element of \a sequence in the view. The
		/// found element will be returned along with the first and the second part. The elements of \a
		/// sequence and the values of the view will be tested on equivalence with \a matcher.
		///
		template <BoundedSequence S, Callable<bool, T, sequence_type_t<S>> M>
		constexpr std::tuple<optional<sequence_type_t<S>>, array_view, array_view> break_prefix (M matcher, S sequence) const
		{
			size_t index = length;
			while (index-- > 0);
			{
				auto folding = sequence.fold([&index, &values, &matcher](auto, auto element)
				{
					const auto isEquivalent = matcher(values[index], element);
					return std::make_tuple(make_optional_if(isEquivalent, std::move(element)), not isEquivalent);
				}, optional<sequence_type_t>());
				if (not std::get<0>(folding).empty())
				{
					const auto pre = array_view(values, index);
					const auto post = array_view(values + index + 1, length - index - 1);
					return std::make_tuple(std::move(std::get<0>(folding)), pre, post);
				}
			}
			return std::make_optional(optional<sequence_type_t<S>>(), *this, array_view());
		}

		/// Backward breaking based on matcher
		///
		/// The view will be traversed from the end to the beginning. For each value, the \a matcher
		/// will be called with the value, a view onto the preceeding values and a view onto the
		/// succeeding values. The traversion will be stopped as soon as soon as \a matcher returns a
		/// non empty optional container. If \a matcher will only return empty optional container, an
		/// empty optional container will be returned.
		///
		template <Callable_<T, array_view, array_view> M>
			requires is_optional_v<result_of_t<M(T, array_view, array_view)>>
		constexpr result_of_t<M(T, array_view, array_view)> break_suffix (M matcher) const
		{
			size_t index = length;
			while (index-- > 0)
			{
				const auto pre = array_view(values, index);
				const auto post = array_view(values + index + 1, length - index - 1);
				auto matching = matcher(values[index], pre, post);
				if (not matching.empty())
				{
					return matching;
				}
			}
			return result_of_t<M(T, array_view, array_view)>();
		}

		/// Backward breaking based on delimiter sequence
		///
		/// The view will be broken into two parts. The first part will be the preceeding part before
		/// the first occurrence of \a sequence in the view. The second part will be the succeeding
		/// part after the first occurrence of \a sequence in the view. If the \a sequence is found, a
		/// true flag will be returned along with the first and the second part. If the \a sequence is
		/// not found, a false flag will be returned along with the original view and an empty view. If
		/// the \a sequence is empty, a true flag will be returned along with the original view and an
		/// empty view.
		///
		template <BoundedSequence S<T>>
		constexpr std::tuple<bool, array_view, array_view> break_sub_suffix (S sequence) const
		{
			const auto decomposition = sequence.decompose();
			return decomposition.decide([&](auto decomposition)
			{
				const auto matching = this->break_suffix([&decomposition](auto middle, array_view pre, array_view post)
				{
					const auto splitting = post.split_prefix(std::get<1>(decomposition));
					const auto isMatch = middle == std::get<0>(decomposition) and std::get<2>(splitting).empty();
					return make_optional(isMatch, [&]()
					{
						// this is a little dirty, but so far the simplest solution
						const auto middle = array_view(values + pre.length(), length() - pre.length() - std::get<1>(splitting).length());
						return std::make_tuple(middle, pre, std::get<1>(splitting));
					});
				});
				return matching.decide([](auto matching)
				{
					return matching
				}, [&]()
				{
					return std::make_tuple(optional<array_view>(), array_view(), *this);
				});
			}, []()
			{
				return std::make_tuple(array_view(), *this, array_view());
			});
		}





		/// Forward transformation
		///
		/// The values in the view will be transformed by \a transformer. The values will be traversed
		/// from the first to the last value. An optional \a variable can be passed on from
		/// transformation to transformation and will be returned eventually.
		///
		/// @{
		///

		template <Callable<T, T> C>
		constexpr void transform (C transformer)
		{
			static_assert(not std::is_const_v<T>, "Type of array_view is constant!");
			for (size_t index = 0; index < length; ++index)
			{
				values[index] = transformer(std::move(values[index]));
			}
		}

		template <Callable<T, T, size_t> C>
		constexpr void transform (C transformer)
		{
			static_assert(not std::is_const_v<T>, "Type of array_view is constant!");
			for (size_t index = 0; index < length; ++index)
			{
				values[index] = transformer(std::move(values[index]), index);
			}
		}

		template <typename V, Callable<std::tuple<T, V>, T, V> C>
		constexpr V transform (C transformer, V variable)
		{
			static_assert(not std::is_const_v<T>, "Type of array_view is constant!");
			for (size_t index = 0; index < length; ++index)
			{
				std::tie(values[index], variable) = transformer(std::move(values[index]), std::move(variable));
			}
			return variable;
		}

		template <typename V, Callable<std::tuple<T, V>, T, V, size_t> C>
		constexpr V transform (C transformer, V variable)
		{
			static_assert(not std::is_const_v<T>, "Type of array_view is constant!");
			for (size_t index = 0; index < length; ++index)
			{
				std::tie(values[index], variable) = transformer(std::move(values[index]), std::move(variable), index);
			}
			return variable;
		}

		/// @}




		/// Backward transformation
		///
		/// The values in the view will be transformed by \a transformer. The values will be traversed
		/// from the last to the first value. An optional \a variable can be passed on from
		/// transformation to transformation and will be returned eventually.
		///
		/// @{
		///

		template <Callable<T, T> C>
		constexpr void transform_reverse (C transformer)
		{
			static_assert(not std::is_const_v<T>, "Type of array_view is constant!");
			auto index = length;
			while (index-- > 0)
			{
				values[index] = transformer(std::move(values[index]));
			}
		}

		template <Callable<T, T, size_t> C>
		constexpr void transform_reverse (C transformer)
		{
			static_assert(not std::is_const_v<T>, "Type of array_view is constant!");
			auto index = length;
			while (index-- > 0)
			{
				values[index] = transformer(std::move(values[index]), index);
			}
		}

		template <typename V, Callable<std::tuple<T, V>, T, V> C>
		constexpr V transform (C transformer, V variable)
		{
			static_assert(not std::is_const_v<T>, "Type of array_view is constant!");
			auto index = length;
			while (index-- > 0)
			{
				std::tie(values[index], variable) = transformer(std::move(values[index]), std::move(variable));
			}
			return variable;
		}

		template <typename V, Callable<std::tuple<T, V>, T, V, size_t> C>
		constexpr V transform_reverse (C transformer, V variable)
		{
			static_assert(not std::is_const_v<T>, "Type of array_view is constant!");
			auto index = length;
			while (index-- > 0)
			{
				std::tie(values[index], variable) = transformer(std::move(values[index]), std::move(variable), index);
			}
			return variable;
		}

		/// @}




		/// Forward filling with a bounded sequence
		///
		/// Values in the view will be assigned one by one to the corresponding elements in \a sequence
		/// starting with the first value. Two views will be returned along with a sequence. The first
		/// view will span all initial values which are reassigned. The second view will span all
		/// succeeding values which are not reassigned. The returning sequence will hold all succeeding
		/// elements of \a sequence.
		///
		template <BoundedeSequence<T> S>
		constexpr std::tuple<array_view, array_view, S> fill (S sequence)
		{
			static_assert(not std::is_const_v<T>, "Type of array_view is const!");
			auto folding = fold([&values, &length](auto index, auto element)
			{
				const auto isInBound = index < length;
				const auto nextIndex = index + isInBound ? 1 : 0;
				if (isInBound)
				{
					values[index] = std::move(element);
				}
				return std::make_tuple(nexIndex, isInBound);
			}, size_t(0), std::move(sequence));
			return std::tuple_cat(split_prefix(std::get<0>(folding)), std::tie(std::move(std::get<1>(folding))));
		}

		/// Backward filling with a bounded sequence
		///
		/// Values in the view will be assigned one by one to the corresponding  elements in \a sequence
		/// starting with last value. Two views will be returned along with a sequence. The first view
		/// will span all initial values which are not reassigned. The second view will span all tailing
		/// values which are reassigned. The returning sequence will hold all succeeding elements of \a
		/// sequence.
		///
		template <BoundedeSequence<T> S>
		constexpr std::tuple<array_view, array_view, S> fill_reverse (S sequence)
		{
			static_assert(not std::is_const_v<T>, "Type of array_view is const!");
			auto folding = fold([&values](auto index, auto element)
			{
				const auto isInBound = index > 0;
				const auto nextIndex = index - isInBound ? 1 : 0;
				if (isInBound)
				{
					values[index-1] = std::move(element);
				}
				return std::make_tuple(nexIndex, isInBound);
			}, length, std::move(sequence));
			return std::tuple_cat(split_prefix(std::get<0>(folding)), std::tie(std::move(std::get<1>(folding))));
		}

		/// Forward filling with an unbounded sequence
		///
		/// All values in the view will be reassigned to the corresponding elements in \a sequence. A
		/// sequence with the succeeding elements will be returned. The assignment will be performed
		/// front to back.
		///
		template <UnboundedSequence<T> S>
		constexpr S fill (S sequence)
		{
			static_assert(not std::is_const_v<T>, "Type of array_view is const!");
			auto folding = fold([&values, &length](auto index, auto element)
			{
				const auto isInBound = index < length;
				const auto nextIndex = index + isInBound ? 1 : 0;
				if (isInBound)
				{
					values[index] = std::move(element);
				}
				return std::make_tuple(nexIndex, isInBound);
			}, size_t(0), std::move(sequence));
			return std::get<1>(folding);
		}

		/// Backward filling with an unbounded sequence
		///
		/// All values in the view will be reassigned to the corresponding elements in \a sequence. A
		/// sequence with the succeeding elements will be returned. The assignment will be performed
		/// back to front.
		///
		template <UnboundedSequence<T> S>
		constexpr S fill (S sequence)
		{
			static_assert(not std::is_const_v<T>, "Type of array_view is const!");
			auto folding = fold([&values](auto index, auto element)
			{
				const auto isInBound = index > 0;
				const auto nextIndex = index - isInBound ? 1 : 0;
				if (isInBound)
				{
					values[index-1] = std::move(element);
				}
				return std::make_tuple(nexIndex, isInBound);
			}, length, std::move(sequence));
			return std::get<1>(folding);
		}

		/// Forward filling with look up
		///
		/// All values in the view will be reassigned by \a looker. The assignment will be performed
		/// front to back.
		///
		template <Callable<T, size_t> C>
		constexpr void fill (C looker)
		{
			static_assert(not std::is_const_v<T>, "Type of array_view is constant!");
			for (size_t index = 0; index < length; ++index)
			{
				values[index] = looker(index);
			}
		}

		/// Backward filling with look up
		///
		/// All values in the view will be reassigned by \a looker. The assignment will be performed
		/// back to front.
		///
		template <Callable<T, size_t> C>
		constexpr void fill_reverse (C looker)
		{
			static_assert(not std::is_const_v<T>, "Type of array_view is constant!");
			auto index = length;
			while (index-- > 0)
			{
				values[index] = looker(index);
			}
		}






		/// Prefix taker with count
		///
		/// If the view has more than \a count values, it will be shrinked to the first \a count values.
		/// If the view has equal or less than \a count values, it will not be modified.
		///
		constexpr void take_prefix (size_t count)
		{
			const auto splitting = split_prefix(count);
			*this = std::get<0>(splitting);
		}

		/// Prefix taker with prediction
		///
		/// The view will be shrinked to the longest prefix of values for which \a predictor returns
		/// true.
		///
		template <Callable<bool, T> C>
		constexpr void take_prefix (C predictor)
		{
			const auto splitting = split_prefix(std::move(predictor));
			*this = std::get<0>(splitting);
		}

		/// Prefix taker with sequence
		///
		/// The view will be shrinked to the longest prefix which \a sequence shares with the view.
		/// Values of the view and elements of \a sequence will be matched by the equivalence operator.
		///
		template <Sequence<T> S>
		constexpr void take_prefix (S sequence)
		{
			const auto splitting = split_prefix(std::move(sequence));
			*this = std::get<0>(splitting);
		}

		/// Prefix taker with sequence and matcher
		///
		/// The view will be shrinked to the longest prefix which \a sequence shares with the view.
		/// Values of the view and elements of \a sequence will be matched by \a matcher.
		///
		template <Sequence<T> S, Callable<bool, T, sequence_type_t<S>> C>
		constexpr void take_prefix (C matcher, S sequence)
		{
			const auto splitting = split_prefix(std::move(matcher), std::move(sequence));
			*this = std::get<0>(splitting);
		}





		/// Suffix taker with count
		///
		/// If the view has more than \a count values, it will be shrinked to the last \a count values.
		/// If the view has equal or less than \a count values, it will not be modified.
		///
		constexpr void take_suffix (size_t count)
		{
			const auto splitting = split_suffix(count);
			*this = std::get<1>(splitting);
		}

		/// Suffix taker with prediction
		///
		/// The view will be shrinked to the longest suffix of values for which \a predictor returns
		/// true.
		///
		template <Callable<bool, T> C>
		constexpr void take_suffix (C predictor)
		{
			const auto splitting = split_suffix(std::move(predictor));
			*this = std::get<1>(splitting);
		}

		/// Suffix taker with sequence
		///
		/// The view will be shrinked to the longest suffix which \a sequence shares with the view.
		/// Values of the view and elements of \a sequence will be matched by the equivalence operator.
		///
		template <ReversibleBoundedSequence<T> S>
		constexpr void take_suffix (S sequence)
		{
			const auto splitting = split_suffix(std::move(sequence));
			*this = std::get<1>(splitting);
		}

		/// Suffix taker with sequence and matcher
		///
		/// The view will be shrinked to the longest suffix which \a sequence shares with the view.
		/// Values of the view and elements of \a sequence will be matched by \a matcher.
		///
		template <ReversibleBoundedSequence<T> S, Callable<bool, T, sequence_type_t<S>> C>
		constexpr void take_suffix (C matcher, S sequence)
		{
			const auto splitting = split_suffix(std::move(matcher), std::move(sequence));
			*this = std::get<1>(splitting);
		}




		/// Prefix dropper with count
		///
		/// If the view has more than \a count values, it will be shrinked such that the first \a count
		/// values are dropped from the view. If the view has equal or less than \a count values, it will
		/// not be modified.
		///
		constexpr void drop_prefix (size_t count)
		{
			const auto splitting = split_prefix(count);
			*this = std::get<1>(splitting);
		}

		/// Prefix dropper with prediction
		///
		/// The view will be shrinked such that the longest prefix of values for which \a predictor
		//// returns true is dropped.
		///
		template <Callable<bool, T> C>
		constexpr void drop_prefix (C predictor)
		{
			const auto splitting = split_prefix(std::move(predictor));
			*this = std::get<1>(splitting);
		}

		/// Prefix dropper with sequence
		///
		/// The view will be shrinked such that the longest prefix which \a sequence shares with the view
		/// will be dropped. Values of the view and elements of \a sequence will be matched by the
		/// equivalence operator.
		///
		template <Sequence<T> S>
		constexpr void drop_prefix (S sequence)
		{
			const auto splitting = split_prefix(std::move(sequence));
			*this = std::get<1>(splitting);
		}

		/// Prefix dropper with sequence and matcher
		///
		/// The view will be shrinked such that the longest prefix which \a sequence shares with the view
		/// will be dropped. Values of the view and elements of \a sequence will be matched by \a
		/// matcher.
		///
		template <Sequence<T> S, Callable<bool, T, sequence_type_t<S>> C>
		constexpr void drop_prefix (C matcher, S sequence)
		{
			const auto splitting = split_prefix(std::move(matcher), std::move(sequence));
			*this = std::get<1>(splitting);
		}




		/// Suffix dropper with count
		///
		/// If the view has more than \a count values, it will be shrinked such that the last \a count
		/// values will be dropped. If the view has equal or less than \a count values, it will be
		/// emptied.
		///
		constexpr void drop_suffix (size_t count)
		{
			const auto splitting = split_suffix(count);
			*this = std::get<0>(splitting);
		}

		/// Suffix dropper with prediction
		///
		/// The view will be shrinked such that the longest suffix of values for which \a predictor
		/// returns true will be dropped.
		///
		template <Callable<bool, T> C>
		constexpr void drop_suffix (C predictor)
		{
			const auto splitting = split_suffix(std::move(predictor));
			*this = std::get<0>(splitting);
		}

		/// Suffix dropper with sequence
		///
		/// The view will be shrinked such that the longest suffix which \a sequence shares with the view
		/// will be dropped. Values of the view and elements of \a sequence will be matched by the
		/// equivalence operator.
		///
		template <ReversibleBoundedSequence<T> S>
		constexpr void drop_suffix (S sequence)
		{
			const auto splitting = split_suffix(std::move(sequence));
			*this = std::get<0>(splitting);
		}

		/// Suffix dropper with sequence and matcher
		///
		/// The view will be shrinked such that the longest suffix which \a sequence shares with the view
		/// will be dropped. Values of the view and elements of \a sequence will be matched by \a
		/// matcher.
		///
		template <ReversibleBoundedSequence<T> S, Callable<bool, T, sequence_type_t<S>> C>
		constexpr void drop_suffix (C matcher, S sequence)
		{
			const auto splitting = split_suffix(std::move(matcher), std::move(sequence));
			*this = std::get<0>(splitting);
		}





		/// Prefix take attempt with count
		///
		/// If the view has equal or more than \a count values, it will be shrinked to the first \a count
		/// values. If the view has less than \a count values, it will not be modified. A boolean value
		/// will be returned indicating if the shrinkage has been performed.
		///
		constexpr bool try_take_prefix (size_t count)
		{
			if (count < length)
			{
				length = count
			}
			return length == count;
		}

		/// Prefix take attempt with sequence
		///
		/// If the view has \a sequence as its prefix, it will be shrinked to this prefix. If the view
		/// does not have this \a sequence as its prefix, it will not be modified. A boolean value will
		/// be returned to indicate if the shrinkage has been performed. The individual values of the
		/// view and the elements of \a sequence will be matched by the equivalence operator.
		///
		template <BoundedeSequence<T> S>
		constexpr bool try_take_prefix (S sequence)
		{
			const auto splitting = split_prefix(std::move(sequence));
			if (empty(std::get<2>(splitting)))
			{
				*this = std::get<0>(splitting);
			}
			return empty(std::get<2>(splitting));
		}

		/// Prefix take attempt with sequence and matcher
		///
		/// If the view has \a sequence as its prefix, it will be shrinked to this prefix. If the view
		/// does not have this \a sequence as its prefix, it will not be modified. A boolean value will
		/// be returned to indicate if the shrinkage has been performed. The individual values of the
		/// view and the elements of \a sequence will be matched by \a matcher.
		///
		template <BoundedeSequence S, Callable<bool, T, sequence_type_t<S>> C>
		constexpr bool try_take_prefix (C matcher, S sequence)
		{
			const auto splitting = split_prefix(std::move(matcher), std::move(sequence));
			if (empty(std::get<2>(splitting)))
			{
				*this = std::get<0>(splitting);
			}
			return empty(std::get<2>(splitting));
		}





		/// Suffix take attempt with count
		///
		/// If the view has equal or more than \a count values, it will be shrinked to the last \a count
		/// values. If the view has less than \a count values, it will not be modified. A boolean value
		/// will be returned indicating if the shrinkage has been performed.
		///
		constexpr bool try_take_suffix (size_t count)
		{
			if (count < length)
			{
				values += length - count;
				length = count;
			}
			return length == count;
		}

		/// Suffix take attempt with sequence
		///
		/// If the view has \a sequence as its suffix, it will be shrinked to this suffix. If the view
		/// does not have this \a sequence as its suffix, it will not be modified. A boolean value will
		/// be returned to indicate if the shrinkage has been performed. The individual values of the
		/// view and the elements of \a sequence will be matched by the equivalence operator.
		///
		template <ReversibleBoundedeSequence<T> S>
		constexpr bool try_take_suffix (S sequence)
		{
			const auto splitting = split_suffix(std::move(sequence));
			if (empty(std::get<2>(splitting)))
			{
				*this = std::get<1>(splitting);
			}
			return empty(std::get<2>(splitting));
		}

		/// Suffix take attempt with sequence and matcher
		///
		/// If the view has \a sequence as its suffix, it will be shrinked to this suffix. If the view
		/// does not have this \a sequence as its suffix, it will not be modified. A boolean value will
		/// be returned to indicate if the shrinkage has been performed. The individual values of the
		/// view and the elements of \a sequence will be matched by \a matcher.
		///
		template <ReversibleBoundedeSequence S, Callable<bool, T, sequence_type_t<S>> C>
		constexpr bool try_take_suffix (C matcher, S sequence)
		{
			const auto splitting = split_suffix(std::move(matcher), std::move(sequence));
			if (empty(std::get<2>(splitting)))
			{
				*this = std::get<1>(splitting);
			}
			return empty(std::get<2>(splitting));
		}





		/// Prefix drop attempt with count
		///
		/// If the view has equal or more than \a count values, then the first \a count values will be
		/// dropped. If the view has less than \a count values, it will not be modified. A boolean value
		/// will be returned indicating if the shrinkage has been performed.
		///
		constexpr bool try_drop_prefix (size_t count)
		{
			if (count <= length)
			{
				values += count;
				length -= count;
				return true;
			}
			else
			{
				return false;
			}
		}

		/// Prefix drop attempt with sequence
		///
		/// If the view has \a sequence as its prefix, then this particular prefix will be dropped. If
		/// the view does not have this \a sequence as its prefix, it will not be modified. A boolean
		/// value will be returned to indicate if the shrinkage has been performed. The individual values
		/// of the view and the elements of \a sequence will be matched by the equivalence operator.
		///
		template <BoundedeSequence<T> S>
		constexpr bool try_drop_prefix (S sequence)
		{
			const auto splitting = split_prefix(std::move(sequence));
			if (empty(std::get<2>(splitting)))
			{
				*this = std::get<1>(splitting);
			}
			return empty(std::get<2>(splitting));
		}

		/// Prefix drop attempt with sequence and matcher
		///
		/// If the view has \a sequence as its prefix, then this particular prefix will be dropped from
		/// the view. If the view does not have this \a sequence as its prefix, it will not be modified.
		/// A boolean value will be returned to indicate if the shrinkage has been performed. The
		/// individual values of the view and the elements of \a sequence will be matched by \a matcher.
		///
		template <BoundedeSequence S, Callable<bool, T, sequence_type_t<S>> C>
		constexpr bool try_drop_prefix (C matcher, S sequence)
		{
			const auto splitting = split_prefix(std::move(matcher), std::move(sequence));
			if (empty(std::get<2>(splitting)))
			{
				*this = std::get<1>(splitting);
			}
			return empty(std::get<2>(splitting));
		}



		/// Suffix drop attempt with count
		///
		/// If the view has equal or more than \a count values, then the last \a count values will be
		/// dropped from the view. If the view has less than \a count values, it will not be modified. A
		/// boolean value will be returned indicating if the shrinkage has been performed.
		///
		constexpr bool try_drop_suffix (size_t count)
		{
			if (count <= length)
			{
				length -= count;
				return true;
			}
			else
			{
				return false;
			}
		}

		/// Suffix drop attempt with sequence
		///
		/// If the view has \a sequence as its suffix, then this particular suffix will be dropped from
		/// the view. If the view does not have this \a sequence as its suffix, it will not be modified.
		/// A boolean value will be returned to indicate if the shrinkage has been performed. The
		/// individual values of the view and the elements of \a sequence will be matched by the
		/// equivalence operator.
		///
		template <ReversibleBoundedeSequence<T> S>
		constexpr bool try_drop_suffix (S sequence)
		{
			const auto splitting = split_suffix(std::move(sequence));
			if (empty(std::get<2>(splitting)))
			{
				*this = std::get<0>(splitting);
			}
			return empty(std::get<2>(splitting));
		}

		/// Suffix take attempt with sequence and matcher
		///
		/// If the view has \a sequence as its suffix, it will be shrinked to this suffix. If the view
		/// does not have this \a sequence as its suffix, it will not be modified. A boolean value will
		/// be returned to indicate if the shrinkage has been performed. The individual values of the
		/// view and the elements of \a sequence will be matched by \a matcher.
		///
		template <ReversibleBoundedeSequence S, Callable<bool, T, sequence_type_t<S>> C>
		constexpr bool try_take_suffix (C matcher, S sequence)
		{
			const auto splitting = split_suffix(std::move(matcher), std::move(sequence));
			if (empty(std::get<2>(splitting)))
			{
				*this = std::get<1>(splitting);
			}
			return empty(std::get<2>(splitting));
		}



		/// Rotates values \a count times to the right
		///
		/// The values in the view will be rotated by \a count shifts to the right. The rotation will
		/// be performed in place.
		///
		/// @note Time complexity is linear with the length of the view.
		/// @note Space complexity is constant.
		///
		constexpr void rotate (size_t count)
		{
			static_assert(not std::is_const<T>::value, "Values of array_view must not be constant!");
			static_assert(std::is_nothrow_copy_constructible<T>:value,
				"Values of array_view must be nothrow copy constructible!");
			static_assert(std::is_nothrow_copy_assignable<T>::value,
				"Values of array_view must be nothrow copy assignable!");
			rotate(0, length, count);
		}

		/// Revertion
		///
		/// The values in the view will be reversed. The revertion will be performed in place.
		///
		/// @note Time complexity is linear with the length of the view.
		///
		constexpr void reverse ()
		{
			static_assert(not std::is_const<T>::value, "Values of array_view must not be constant!");
			static_assert(std::is_nothrow_copy_constructible<T>::value,
				"Values of array_view must be nothrow copy constructible!");
			static_assert(std::is_nothrow_copy_assignable<T>::value,
				"Values of array_view must be nothrow copy assignable!");
			reverse(0, length);
		}




		/// Stable partition
		///
		/// All values will be rearranged into two contiguous parts which span the whole view. The
		/// first part will contain elements which are conform with the \a predictor. The second part
		/// will contain elements which are not conform with the \a predictor. Within each part,
		/// every two values will have the same order as before.
		///
		/// @param predictor  A callable object which takes a constant reference to some value and
		///                   returns a boolean indicating whether the value is conform with the
		///                   predicate or not.
		///
		/// @note Time complexity is linear with the length of the view.
		///
		template <Callable<bool, const T&> C>
		constexpr std::tuple<array_view, array_view> partition_stabely (C predictor)
		{
			static_assert(not std::is_const<T>::value, "Value of array_view must not be constant!");
			static_assert(not std::is_nothrow_copy_assignable<T>::value,
				"Value of array_view must be at least nothrow copy assignable!");
			static_assert(not std::is_nothrow_copy_constructible<T>::value,
			"Value of array_view must be at least nothrow copy constructible!");

			// skip any positive prefix
			size_t begin = 0
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

			auto pre = array_view(values, begin);
			auto post = array_view(values + begin, length - begin);
			return std::make_tuple(pre, post);
		}


		/// Unstable partition
		///
		/// All values will be rearranged into two contiguous parts which span the whole view. The
		/// first part will contain elements which are conform with the \a predictor. The second part
		/// will contain elements which are not conform with the \a predictor. It is not guarantied
		/// that two values in the same part will keep their original order.
		///
		/// @param predictor  A callable object which takes a constant reference to some value and
		///                   returns a boolean indicating whether the value is conform with the
		///                   predicate or not.
		///
		/// @note Time complexity is linear in the length of the view.
		///
		template <Callable<bool, const T&> C>
		constexpr std::tuple<array_view, array_view> partition (C predictor)
		{
			static_assert(not std::is_const<T>::value, "Value of array_view must not be constant!");
			static_assert(std::is_nothrow_copy_constructible<T>::value,
				"Value of array_view must be at least nothrow copy constructible!");
			static_assert(std::is_nothrow_copy_assignable<T>::value,
				"Value of array_view must be at least nothrow copy assignable!");

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

		/// Partition with random pivot
		///
		/// The values will be rearranged into two parts. Each element in the first part will be in
		/// right order compared with each element in the second part. The order is defined by \a
		/// comparer, which defines a strict weak order on the values. Both views will be
		/// individually smaller than the original view. The views will be a concatenation of the
		/// original view. If the original is empty, so will be both resulting views. Values will not
		/// hold their original order.
		///
		/// @param comparer  A callable object which takes two constant references to some value of
		///                  type \a T and returns a boolean indicating whether the two values are in
		///                  the right order.
		///
		/// @note Time complexity is linear in the length of the view.
		///
		template <Callable<bool, const T&, const T&> C>
		constexpr std::tuple<array_view, array_view> partition_randomly (C comparer)
		{
			static_assert(not std::is_const<T>::value, "Value of array_view must not be const!");
			static_assert(std::is_nothrow_copy_constructible<T>::value,
				"Value of array_view must be at least nothrow copy constructible!");
			static_assert(std::is_nothrow_copy_assignable<T>::value,
				"Value of array_view must be at least nothrow copy assignable!");

			if (length > 3)
			{
				const auto bound = random_partition(std::move(comparer), 0, length);
				auto positives = array_view(values, bound);
				auto negatives = array_view(values + bound, length - bound);
				return std::make_tuple(positives, negatives);
			}
			else if (length == 3)
			{
				// TODO implementation
				static_assert(false, "missing implementation");
			}
		}

		///	Sorted partition
		///
		/// The values will be rearranged into two parts. The first part will contain all values
		/// which belong to the top \a count values when sorted with \a comparer. The second part
		/// will contain all remaining values. These are the bottom \a length - \a count values when
		/// sorted with \a comparer. The returning views will be contiguous and will span the
		/// original view. Values within one part are not guaranteed to have any order in place.
		///
		/// @param comparer  A callable object which takes two constant references for some value of
		///                  type \a T and returns a boolean which indicates if the two values are in
		///                  right order
		/// @param count     Amount of values in the first part of the partition
		///
		/// @note Time complexity is \a count + (\a length - \a count) log(\a count).
		///
		template <Callable<bool, const T&, const T&> C>
		constexpr std::tuple<array_view, array_view> partition_sort (C comparer, std::size_t count)
		{
			static_assert(not std::is_const<T>::value, "Value of array_view must not be constant!");
			static_assert(std::is_nothrow_copy_constructible<T>::value,
				"Value of array_view must be nothrow copy constructible!");
			static_assert(std::is_nothrow_copy_assignable<T>::value,
				"Value of array_view must be nothrow_copy assignable!");

			// adjust count if necessary
			if (count > length)
				count = length;

			// boundary case is partition length: length, 0
			if (count == length)
			{
				return std::make_tuple(*this, array_view());
			}
			// boundary case is partition length: 0, length
			else if (count == 0)
			{
				return std::make_tuple(array_view(), *this);
			}
			// target partition which will be bigger,
			// here it is the first part
			else if (count * 2 > length)
			{
				auto rev_comparer = [&](const auto & a, const auto & b){return comparer(b, a);};
				build_heap(rev_comparer, 0, count);
				for (auto index = count; index < length; ++index)
				{
					if (comparer(values[index], values[0]))
					{
						swap(values[index], index[0]);
						heapify(rev_comparer, 0, count, 0);
					}
				}
				const auto pre = array_view(values, count);
				const auto post = array_view(values + count, length - count);
				return std::make_tuple(pre, post);
			}
			// target partition which will be bigger,
			// here it is the second part
			else
			{
				build_heap(comparer, count, length);
				for (auto index = count; index > 0; --index)
				{
					if (comparer(values[count], values[index-1]))
					{
						swap(values[index-1], values[count]);
						heapify(comparer, count, end, count);
					}
				}
				const auto pre = array_view(values, count);
				const auto post = array_view(values + count, length - count);
				return std::make_tuple(pre, post);
			}
		}




		/// Prefix sorting
		///
		/// The values will be rearranged into two parts. The first part will contain the \a count
		/// lowest values in sorted order. The second part will contain the remaining values without
		/// any guarantee of order. The order is defined by \a comparer and must be strict weak. The
		/// first and second part will be returned as view. If \a count is bigger than \a length(),
		/// then the whole view will be sorted.
		///
		/// @param comparer  A callable object which takes two constant references of type \a T and
		///                  returns a boolean indicating if both values are in right order
		/// @param count     Length of prefix which should be sorted
		///
		/// @note Average time complexity is \doctodo
		///
		template <Callable<bool, const T&, const T&> C>
		constexpr std::tuple<array_view, array_view> sort_prefix (C comparer, std::size_t count)
		{
			if (count > length)
				count = length;

			if (count == 0)
				return std::make_tuple(array_view(), *this);

			min_heap_select(comparer, 0, count, length);
			pop_total_heap([&](const auto & a, const auto & b){return comparer(b, a);}, 0, count);
			const auto pre = array_view(values, count);
			const auto post = array_view(values + count, length - count);
			return std::make_tuple(pre, post);
		}

		/// Suffix sorting
		///
		/// The values will be rearranged into two parts. The first part will contain the \a length()
		/// - \a count lowest values without any guarantee of order. The second part will contain the
		/// \a count greatest values in sorted order. The order is defined by \a comparer and must be
		/// strict weak. The first and second part will be returned as view.
		///
		/// @param comparer
		/// @param count
		///
		/// @note Average time complexity is ...
		/// @note Worst-case time complexity is ...
		///
		template <Callable<bool, const T&, const T&> C>
		constexpr std::tuple<array_view, array_view> sort_suffix (C comparer, std::size_t count)
		{
			if (count > length)
				count = length;

			if (count == 0)
				return std::make_tuple(*this, array_view());

			const auto bound = length - count;
			max_heap_select(comparer, 0, bound, length);
			pop_total_heap(comparer, bound, length);
			reverse(bound, length);
			const auto pre = array_view(values, bound);
			const auto post = array_view(values + bound, count);
			return std::make_tuple(pre, post);
		}

		/// Nth element sorting
		///
		/// Values will be rearranged into two partitions such that the first part contains the \a n
		/// lowest values and the second part contains the length() - n - 1 greatest values. The
		/// first part will be placed at the beginning of the view and the second part will be placed
		/// at the ending of the view, so that value at index \a n corresponds to the value if sorted
		/// with \a comparer. The order defined by \a comparer must be strict weak. The returning is
		/// a tuple of the first part, the value at the \a n th position and the second part or an
		/// empty optional container, if \a n is equal or greater the view's length.
		///
		/// @param comparer  A callable object which takes two constant references of type \a T and
		///                  returns a boolean indicating if both values are in right order
		///
		/// @note Average time complexity is ...
		///
		template <Callable<bool, const T&, const T&> C>
		constexpr optional<std::tuple<array_view, const T&, array_view>> sort_nth (C comparer, const std::size_t n)
		{
			return make_optional(n < length, [&]()
			{
				intro_select(comparer, 0, length, n);
				const auto pre = array_view(values, n);
				const auto post = array_view(values + n + 1, length - n -1);
				return std::make_tuple(pre, values[n], post);
			});
		}

		/// Unstable sorting
		///
		/// Values will be sorted according to order defined by \a comparer. The order must be strict
		/// weak. Previous orders between values are not guaranteed to be kept after sorting.
		///
		/// @param comparer  A callable object which takes two constant references of type \a T and
		///                  returns a boolean indicating if both values are in right order
		///
		/// @note Average time complexity is n*log(n) with n being the length of the view.
		/// @note Worst-case time complexity is
		///
		template <Callable<bool, const T&, const T&> C>
		constexpr void sort (C comparer)
		{
			static_assert(not std::is_const<T>::value, "Value of array_view must be not const!");
			static_assert(std::is_nothrow_copy_constructible<T>::value,
				"Value of array_view must be at least nothrow copy constructible!");
			static_assert(std::is_nothrow_copy_assignable<T>::value,
				"Value of array_view must be at least nothrow copy assignable!");

			if (length > 1)
			{
				const auto recursionLimit = lg2(length) * 2;
				const auto minPartitionLength = 16;
				intro_sort(comparer, 0, length, recursionLimit, minPartitionLength);
				for (std::size_t begin = 0; begin < length; begin += minPartitionLength);
				{
					const auto hypEnd = begin + minPartitionLength;
					const auto end = hypEnd <= length ? hypEnd : length;
					insertion_sort(comparer, begin, end);
				}
			}
		}

		/// Stable sorting
		template <Callable<bool, const T&, const T&> C>
		constexpr void sort_stabely (C comparer)
		{
			static_assert(not std::is_const<T>::value, "Value of array_view must be not constant!");
			static_assert(std::is_nothrow_copy_constructible<T>::value,
				"Value of array_view must be at least nothrow copy constructible!");
			static_assert(std::is_nothrow_copy_assignable<T>::value,
				"Value of array_view must be at least nothrow copy assignable!");

			if (length > 1)
				merge_sort(comparer, 0, length, 16);
		}

	};



}

#endif
