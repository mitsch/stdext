/// @file array.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_ARRAY_HPP__
#define __STDEXT_ARRAY_HPP__

#include <stdext/allocator.hpp>
#include <stdext/array_view.hpp>

namespace stdext
{

	/// Container for a sequence values of type \a T with dynamic length
	///
	/// An array is a container for a sequence of values which all are of type \a T. The length of
	/// the sequence is variable and may change dynamically during runtime. Any memory allocation is
	/// performed with allocator \a A.
	template <typename T, Allocator A = system_allocator> class array
	{

		template <Allocator B> friend class array<T, B>;

		private:

			using allocation_type = allocation_type_t<A, T>;

			A allocator;
			allocation_type allocation;
			std::size_t used = 0;


			/// Requests allocation for \a count values
			constexpr allocation_type allocate(std::size_t count)
			{
				return allocator.template allocate<T>(count);
			}

			/// Deallocates \a allocation
			constexpr void deallocate(allocation_type allocation)
			{
				allocator.deallocate(allocation);
			}

			/// Returns a mutable pointer to the values
			constexpr T* data ()
			{
				return allocation.data();
			}

			/// Calls destructor for each value, if type \a T is a non trivial object
			static void destruct (T * values, std::size_t count)
			{
				assert(values != nullptr or count == 0);
				if (not std::is_trivial_destructible<T>::value)
				{
					for (std::size_t index = 0; index < count; ++index)
						values[index].~T();
				}
			}

			/// Constructs \a count value in \a destination by copying instances from \a source
			/// with an exception \a cleaner
			static void copy_construct (T* destination, const T* source, std::size_t count, Callable_<std::size_t> cleaner)
			{
				assert(destination != nullptr or count == 0);
				assert(source != nullptr or count == 0);
				assert(destination != source or (count == 0 and destination == nullptr));
				assert(not (source < destination) or source + count <= destination);
				assert(not (destination < source) or destination + count <= source);

				std::size_t index = 0;
				try
				{
					while (index < count)
					{
						new (destination + index) T(*(source + index));
						++index;
					}
				}
				catch (...)
				{
					cleaner(index);
					throw;
				}
			}

			/// Constructs \a count values in \a destination by copying instances from \a source
			/// without any exception cleaner
			static void copy_construct (T* destination, const T* source, std::size_t count)
			{
				assert(destination != nullptr or count == 0);
				assert(source != nullptr or count == 0);
				assert(destination != source or (count == 0 and destination == nullptr));
				assert(not (source < destination) or source + count <= destination);
				assert(not (destination < source) or destination + count <= source);

				for (std::size_t index = 0; index < count; ++index)
					new (destination + index) T(*(source + index));
			}

			/// Constructs \a count values in \a destination by moving instances from \a source in
			/// forward direction
			static void move_construct (T* destination, T* source, std::size_t count)
			{
				assert(destination != nullptr or count == 0);
				assert(source != nullptr or count == 0);
				assert(destination != source or (count == 0 and destination == nullptr));
				assert(not (source < destination) or not (destination < source + count));

				for (std::size_t index = 0; index < count; ++index)
					new (destination + index) T(std::move(*(source + index)));
			}

			/// Constructs values in \a destination with elements from \a source without any
			/// exception cleaner
			template <BoundedSequence S>
			static void construct (T* destination, S source)
			{
				assert(destination != nullptr);
				
				fold([&](auto index, auto element)
				{
					new (destination + index) T(std::move(element));
					return index + 1;
				}, std::size_t(0), std::move(source));
			}

			/// Constructs values in \a destination with elements from \a source with an exception
			/// \a cleaner
			template <BoundedSequence S, Callable_<std::size_t> C>
			static void construct (T* destination, S source, C cleaner)
			{
				assert(destination != nullptr);

				fold([&](auto index, auto element)
				{
					try
					{
						new (destination + index) T(std::move(element));
					}
					catch (...)
					{
						cleaner(index);
						throw;
					}
					return index + 1;
				}, std::size_t(0), std::move(source));
			}

			/// Assigns \a count values in \a destination by moving instances from \a source in
			/// forward direction
			static void move_assign (T* destination, T* source, std::size_t count)
			{
				assert(destination != nullptr or count == 0);
				assert(source != nullptr or count == 0);
				assert(destination != source or (count == 0 and destination == nullptr));
				assert(not (source < destination) or not(destination < source + count));

				for (std::size_t index = 0; index < count; ++index)
					*(destination + index) = std::move(*(source + index));
			}

			/// Assigns \a count values in \a destination by moving instances from \a source in
			/// backward direction
			static void move_assign_reverse (T* destination, T* source, std::size_t count)
			{
				assert(destination != nullptr or count == 0);
				assert(source != nullptr or count == 0);
				assert(destination != source or (count == 0 and destination == nullptr));
				assert(not (destination < source) or not (source < destination + count));

				if (count > 0)
				{
					do
					{
						--count;
						*(destination + count) = std::move(*(source + count));
					}
					while(count > 0);
				}
			}

			/// Assigns values in \a destination with elements from \a source without any exception
			/// cleaner
			template <BoundedSequence S>
			static S assign (T* destination, S source)
			{
				assert(destination != nullptr or not source);

				auto folding = fold([&](auto index, auto element)
				{
					new (destination + index) T(std::move(element));
					return index + 1;
				}, std::size_t(0), std::move(source));

				return std::get<1>(folding);
			}

			/// Assigns values in \a destination with elements from \a source and an exception \a cleaner
			template <BoundedSequence<T> S, Callable_<std::size_t> C>
			static S assign (T* destination, S source, C cleaner)
			{
				assert(destination != nullptr or not source);

				auto folding = fold([&](auto index, auto element)
				{
					try
					{
						new (destination + index) T(std::move(element));
					}
					catch (...)
					{
						cleaner(index);
						throw;
					}
					return index + 1;
				}, std::size_t(0), std::move(source));

				return std::get<1>(folding);
			}

			/// Assigns \a count values in \a destination to elements from \a source
			template <BoundedSequence<T> S>
			static S assign (T* destination, S source, std::size_t count)
			{
				assert(destination != nullptr or count == 0);

				auto folding = fold([&](auto index, auto element)
				{
					const auto isInBound = index < count;
					const auto nextIndex = index + (isInBound ? 1 : 0);
					if (isInBound) *(destination + index) = std::move(element);
					return std::make_tuple(nextIndex, isInBound);
				}, std::size_t(0), std::move(source));
				assert(std::get<0>(folding) == count);

				return std::get<1>(folding);
			}


		public:


			/// Constructor with allocator
			///
			/// The allocator is set to \a allocator. No memory allocation will be performed. The array
			/// will be empty regarding its capacity and hence its values.
			constexpr explicit array (A allocator)
				: allocator(std::move(allocator))
				noexcept(std::is_nothrow_move_constructible<A>::value)
			{}
			
			/// Move constructor with an array having a different allocator type
			///
			/// The allocator will be default constructed. If \a other contains some values, memory will
			/// be allocated from its own allocator. Values from \a other will be moved into the newly
			/// constructed object.
			template <Allocator B>
			constexpr array (array<T, B> && other);

			/// Move constructor with additional \a allocator
			///
			/// The allocator will be set to \a allocator. If \a other contains some values, memory will
			/// be allocated from its own allocator. Values from \a other will be moved into the newly
			/// constructed object.
			template <Allocator B> constexpr array (array<T, B> && other, A allocator)
				: array(other.view(), std::move(allocator))
			{}

			/// Copy constructor with arbitrary allocator \a B for array \a other
			///
			/// The allocator will be default constructed. If \a other contains some values, memory will
			/// be allocated from its own allocator. Values from \a other will be copied into the newly
			/// constructed object. The array \a other will not be modified in any way.
			template <Allocator B> constexpr array (const array<T, B> & other)
				: array(other.view())
			{}

			/// Copy constructor with arbitrary allocator \a B for array \a other and specified allocator
			///
			/// The allocator will be set by \a allocator. If \a other contains some values, memory will
			/// be allocated from \a allocator. Values from \a other will be copied into the newly
			/// constructed object. The array \a other will not be modified in any way.
			template <Allocator B> constexpr array (const array<T, B> & other, A allocator)
				: array(other.view(), std::move(allocator))
			{}

			/// Copy constructor with sequence \a values
			///
			/// The allocator will be constructed by default. If \a values contains any value, memory
			/// will be allocated from the default constructed allocator. All values in \a values will
			/// be stored in the newly constructed object.
			constexpr explicit array (BoundedSequence<T> values)
				: array(std::move(values), A())
			{}

			/// Constructor with sequence \a values and \a allocator
			///
			/// The allocator will be set to \a allocator. If \a values contains any value, memory will
			/// be allocated from the allocator. All values in \a values will be stored in the newly
			/// constructed object.
			constexpr array (BoundedSequence<T> values, A allocator);

			/// Constructor reserving memory for at least \a count values
			///
			/// The allocator will be constructed by default. If not enough memory could be allocated,
			/// the exception \a bad_alloc will be thrown. No value will be constructed.
			explicit constexpr array (std::size_t count)
				: array(count, A())
			{}

			/// Constructor reserving memory for at least \a count values with \a allocator
			///
			/// The allocator will be set to \a allocator. If not enough memory could be allocated, the
			/// exception \a bad_alloc will be thrown. No value will be constructed.
			constexpr array (std::size_t count, A allocator)
				: allocator(std::move(allocator))
			{
				allocation = allocate(count);
				if (allocation.length() < count)
				{
					allocator.deallocate(allocation);
					throw bad_alloc();
				}
			}

			/// Destructor
			///
			/// All contained values will be destructed. Any allocation will be freed.
			~array()
			{
				destruct(allocation.data(), usedLength);
				allocator.deallocate(allocation);
			}




			// ------------------------------------------------------------------------------------------
			// Assignment

			/// Move assignment
			///
			/// Any value in the own instance will be destructed and any memory will be deallocated. If
			/// any exception is raised, the precalling state will be restored.
			array& operator = (array && other)
			{
				swap(*this, other);
				return *this;
			}

			/// Copy assignment
			///
			/// Any value in the own instance will be destructed and all values of \a other will be
			/// copied. If there is not enough memory, new memory will be allocated. If any exception is
			/// raised, the precalling state will be restored.
			template <Allocator B>
			array& operator = (const array<T, B> & other);

			/// Sequence assignment
			///
			/// Any value in the own instance will be destructed. All \a values will be copied into the
			/// the own memory allocation. If the allocation is not big enough, new memory will be
			/// allocated. If any exception is raised, the precalling statei will be restored.
			array& operator = (BoundedSequence<T> values);




			// ------------------------------------------------------------------------------------------
			// Fill

			/// Constant filling
			///
			/// All values in the array will be assigned to \a constant. The traversion is front to back.
			template <typename U>
				requires std::is_convertible<U, T>::value
			constexpr void fill (U constant)
			{
				view().fill(std::move(constant));
			}

			/// Constant filling in reverse
			///
			/// All values in the array will be assigned to \a constant. The traversio is back to front.
			template <typename U>
				requires std::is_convertible<U, T>::value
			constexpr void fill_reverse (U constant)
			{
				view().fill_reverse(std::move(constant));
			}

			/// Sequence filling
			///
			/// All values in the array will be assigned to their corresponding elements in the \a
			/// sequence. The remaining of the sequence will be returned. The elements will be filled in
			/// traversing from the front to the back.
			template <UnboundedSequence<T> S>
			constexpr S fill (S sequence)
			{
				return view().fill(std::move(sequence));
			}

			/// Sequence filling in reverse
			///
			/// All values in the array will be assigned to their corresponding elements in the \a
			/// sequence. The remaining of the sequence will be returned. The elements will be filled in
			/// traversing from the back to the front.
			template <UnboundedSequence<T> S>
			constexpr S fill_reverse (S sequence)
			{
				return view().fill_reverse(std::mvoe(sequence));
			}

			/// Partial sequence filling
			///
			/// Initial values are assigned to their corresponding elements in the \a sequence. A view
			/// with the initial filled values, a view with the remaining values, a the remaining
			/// sequence.
			template <BoundedSequence<T> S>
			constexpr std::tuple<array_view<T>, array_view<T>, S> fill (S sequence)
			{
				return view().fill(std::move(sequence));
			}

			/// Partial sequence filling in reverse
			///
			/// Tailing values are assigned to their corresponding elements in the \a sequence. A view
			/// with the tailing filled values, a view with the remaining values, a the remaining
			/// sequence.
			template <BoundedSequence<T> S>
			constexpr std::tuple<array_view<T>, array_view<T>, S> fill_reverse (S sequence)
			{
				return view().fill_reverse(std::move(sequence));
			}


	
			// ------------------------------------------------------------------------------------------
			// Access

			/// Indexing
			///
			/// If \a index is in bound, a refernece to the corresponding value will be returned. If \a
			/// index is out of bound, an empty optional container will be returned.
			///
			/// @{

			constexpr optional<T&> operator [] (std::size_t index)
			{
				return make_optional(index < used, [&](){return values[index];});
			}

			constexpr optional<const T&> operator [] (std::size_t index)
			{
				return make_optional(index < used, [&](){return values[index];});
			}

			/// @}

			/// Accessing
			///
			/// Two callable objects, \a hitter and \a misser, are passed on. If \a index is in bound,
			/// then \a hitter will be called with a reference to the corresponding value and with \a
			/// arguments. If \a index is out of bound, then \a misser will be called with \a arguments.
			/// Either way, the result of calling \a hitter or \a misser will be returned.
			///
			/// @{

			template <typename C, typename D, typename ... As>
				requires Callable_<C, T&, As ...> and
				         Callable_<D, As ...> and
				         not std::is_void_v<result_of_t<C(T&, As ...)>> and
				         not std::is_void_v<result_of_t<D(As ...)>> and
				         requires(){typename std::common_type_t<result_of_t<C(T&, As ...)>, result_of_t<D(As ...)>>}
			constexpr auto at (C hitter, D misser, std::size_t index, As ... arguments)
			{
				return index < length ? hitter(values[index], std::move(arguments) ...) : misser(std::move(arguments) ...);
			}

			template <typename C, typename D, typename ... As>
				requires Callable_<C, const T&, As ...> and
				         Callable_<D, As ...> and
				         not std::is_void_v<result_of_t<C(const T&, As ...)>> and
				         not std::is_void_v<result_of_t<D(As ...)>> and
				         requires(){typename std::common_type_t<result_of_t<C(const T&, As ...)>, result_of_t<D(As ...)>>}
			constexpr auto at (C hitter, D misser, std::size_t index, As ... arguments) const
			{
				return index < length ? hitter(values[index], std::move(arguments) ...) : misser(std::move(arguments) ...);
			}

			/// @}









			// ------------------------------------------------------------------------------------------
			// Modifier

			/// Appending one element
			///
			/// One element is appended to the end of the array. The element is constructed with \a
			/// arguments. If any exception is thrown, the array will leave in its original state.
			template <typename ... As>
				requires std::is_constructible<T, As ...>::value
			constexpr void append (As && ... arguments);
		
			/// Appending a sequence of values
			///
			/// A sequence of values is appended to the array. If any exception is thrown, the original
			/// state of the array will be restored.
			template <BoundedSequence S>
				requires std::is_constructible<T, sequence_type_t<S>>::value
			void append (S sequence);
			
			/// Prepeding some value
			///
			/// A value is constructed at the beginning of the array. The original state of the array
			/// will be restored, if an exception is thrown.
			template <typename ... As>
				requires std::is_constructible<T, As ...>::value;
			constexpr void prepend (As && ... arguments);
			
			/// Prepending a sequence of values
			///
			/// All values in \a sequence will be prepended at the beginning of the array. They will be
			/// placed such that the first element in the sequence will be the first value in the array,
			/// the second element in the sequence will be the second value in the array, and so on. The
			/// original state of the array will be restored, if any exception is thrown.
			template <BoundedSequence S>
				requires std::is_constructible<T>, sequence_type_t<S>>::value
			constexpr void prepend (S sequence);

			/// Inserting a value
			///
			/// If \a index is less than the current length, a new value at \a index will be constructed
			/// with \a arguments. All previous values at \a index and above will be shifted up by one.
			/// If \a index equals the current length or is greater, the new value will be appended. The
			/// original state of the array will be restored, if any exception is thrown.
			template <typename ... As>
				requires std::is_constructible<T, As ...>::value
			constexpr void insert (std::size_t index, As && ... arguments);
			
			/// Inserting a sequence
			///
			/// If \a index is less than the current length, a \a sequence of values will be inserted at
			/// \a index and any previous values at \a index or higher will be shifted up by the length
			/// of the \a sequence. If \a index is equal or higher the current length, the \a sequence
			/// will be appended. The original state of the array will be restored, if any exception is
			/// thrown.
			template <BoundedSequence S>
				requires std::is_constructible<T, sequence_type_t<S>>::value
			constexpr void insert (std::size_t index, BoundedSequence S);

			/// Erasing a single value
			///
			/// The value at \a index will be erased and all values above will be shifted down by one. If
			/// \a index is out of bound (not less the length of the array), nothing will be done. The
			/// original state of the array will be restored, if any exception is thrown.
			constexpr void erase (std::size_t index);
			
			/// Erasing a range of values
			///
			/// All \a count values starting at \a index will be erased from the array and any values
			/// which are positioned higher will be shifted down by \a count. If \a index is out of bound
			/// or \a count is too much, they will be adapted. 
			constexpr void erase (std::size_t index, std::size_t count);

			/// Erasing with a predictor
			///
			/// All values for which \a predictor returns will be erased. All remaining values will be
			/// shifted down such that they will be contiguous. The original state of the array will be
			/// restored, if any exception is thrown.
			template <Callable<bool, const T&, std::size_t> C>
			constexpr void erase (C predictor);

			/// Capacity reservation
			///
			/// If no expection is thrown, the capacity will be assured to be able to contain at least
			/// \a count values without any reallocation. If an exception is thrown, the object will
			/// remain its precalling state.
			constexpr void reserve (std::size_t count);

			/// Memory shrinkage
			///
			/// Memory footprint will be reduced to a minimum. If no value is contained, all memory will
			/// be deallocated. If no memory can be allocated, the
			constexpr void shrink ();



			/// Returns the pointer to the memory where the values are stored
			constexpr const T* data () const
			{
				return allocation.data();
			}

			/// Returns how many values are contained by the container right now
			constexpr std::size_t length () const
			{
				return usedLength;
			}

			/// Returns how many values can be contained by the container without reallocation
			constexpr std::size_t capacity () const
			{
				return allocation.length();
			}

			/// Tests if the container is empty
			constexpr bool empty () const
			{
				return usedLength == 0;
			}

			/// Empties the container
			///
			/// If any value is contained, they all will be destructed and the container will be left
			/// emptied. Any allocated memory will not be deallocated.
			void clean ()
			{
				destruct(allocation.data(), usedLength);
				usedLength = 0;
			}

			


			friend void swap (array & first, array & second)
			{
				using std::swap;
				swap(first.allocator, second.allocator);
				swap(first.allocation, second.allocation);
				swap(first.usedLength, second.usedLength);
			}





			/// Accessor
			///
			/// If some value is positioned at \a index in the sequence, it will be returned, otherwise
			/// an empty optional contianer will be returned.
			constexpr optional<T> operator [] (const std::size_t index) const
			{
				return index < usedLength ? optional<T>(*(allocation.data() + index)) : optional<T>();
			}
			


			/// Returns view on the values
			constexpr array_view<T> view ()
			{
				return array_view<T>(allocation.data(), usedLength);
			}

			/// Returns view on the values
			constexpr array_view<const T> view () const
			{
				return array_view<const T>(allocation.data(), usedLength);
			}


			// ------------------------------------------------------------------------------------------
			// Transformation

			/// Transforming
			///
			/// All values in the array will be transformed by \a transformer. The traversal of values
			/// will be front to back. An optional \a variable can be set. It will be passed from
			/// traversion to traversion and its final value will be returned.
			///
			/// @{

			template <Callable<T, T> C>
			constexpr void transform (C transformer)
			{
				for (std::size_t index = 0; index < used; ++index)
					values[index] = transformer(std::move(values[index]));
			}

			template <Callable<T, T, std::size_t> C>
			constexpr void transform (C transformer)
			{
				for (std::size_t index = 0; index < used; ++index)
					values[index] = transformer(std::move(values[index]), index);
			}

			template <typename V, Callable<std::tuple<T, V>, T, V> C>
			constexpr V transform (C transformer, V variable)
			{
				for (std::size_t index = 0; index < used; ++index)
					std::tie(values[index], variable) = transformer(std::move(values[index]), std::move(variable));
				return variable;
			}

			template <typename V, Callable<std::tuple<T, V>, T, V, std::size_t> C>
			constexpr V transform (C transformer, V variable)
			{
				for (std::size_t index = 0; index < used; ++index)
					std::tie(values[index], variable) = transformer(std::move(values[index]), std::move(variable), index);
				return variable;
			}

			//// @}

			
			/// Transforming in reverse
			///
			/// All values in the array will be transformed by \a transformer. The traversal of values
			/// will be back to front. An optional \a variable can be set. It will be passed from
			/// traversion to traversion and its final value will be returned.
			///
			/// @{

			template <Callable<T, T> C>
			constexpr void transform_reverse (C transformer)
			{
				for (std::size_t index = used; index > 0; --index)
					values[index-1] = transformer(std::move(values[index-1]));
			}

			template <Callable<T, T, std::size_t> C>
			constexpr void transform_reverse (C transformer)
			{
				for (std::size_t index = used; index > 0; --index)
					values[index-1] = transformer(std::move(values[index-1]), index-1);
			}

			template <typename V, Callable<std::tuple<T, V>, T, V> C>
			constexpr V transform_reverse (C transformer, V variable)
			{
				for (std::size_t index = used; index > 0; --index)
					std::tie(values[index-1], variable) = transformer(std::move(values[index-1]), std::move(variable));
				return variable;
			}

			template <typename V, Callable<std::tuple<T, V>, T, V, std::size_t> C>
			constexpr V transform_reverse (C transformer, V variable)
			{
				for (std::size_t index = used; index > 0; --index)
					std::tie(values[index-1], variable) = transformer(std::move(values[index-1]), std::move(variable), index-1);
				return variable;
			}

			//// @}











			// ------------------------------------------------------------------------------------------
			// Partition

			/// Stable partition
			///
			/// All values will be rearranged into two contiguous parts which span the whole array. The
			/// first part will contain elements which are conform with the \a predictor. The second part
			/// will contain elements which are not conform with the \a predictor. Within each part,
			/// every two values will have the same order as before.
			///
			/// @param predictor  A callable object which takes a constant reference to some value and
			///                   returns a boolean indicating whether the value is conform with the
			///                   predicate or not.
			///
			/// @note Time complexity is linear with the length of the array.
			///
			template <Callable<bool, const T&> C>
			constexpr std::tuple<array_view<T>, array_view<T>> partition_stable (C predictor)
			{
				return view().partition_stable(std::move(predictor));
			}

			
			/// Unstable partition
			///
			/// All values will be rearranged into two contiguous parts which span the whole array. The
			/// first part will contain elements which are conform with the \a predictor. The second part
			/// will contain elements which are not conform with the \a predictor. It is not guarantied
			/// that two values in the same part will keep their original order.
			///
			/// @param predictor  A callable object which takes a constant reference to some value and
			///                   returns a boolean indicating whether the value is conform with the
			///                   predicate or not.			
			///
			/// @note Time complexity is linear in the length of the array.
			///
			template <Callable<bool, const T&> C>
			constexpr std::tuple<array_view<T>, array_view<T>> partition (C predictor)
			{
				return view().partition(std:move(predictor));
			}

			
			/// Partition with random pivot
			///
			/// The values will be rearranged into two parts. Each element in the first part will be in
			/// right order compared with each element in the second part. The order is defined by \a
			/// comparer, which defines a strict weak order on the values. Both views will be
			/// individually smaller than the array. The views will be a concatenation of the array. If
			/// the array is empty, so will be both resulting views. Values will not hold their original
			/// order.
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
				return view().partition_randomly(std::move(comparer));
			}


			///	Sorted partition
			///
			/// The values will be rearranged into two parts. The first part will contain all values
			/// which belong to the top \a count values when sorted with \a comparer. The second part
			/// will contain all remaining values. These are the bottom \a length - \a count values when
			/// sorted with \a comparer. The returning views will be contiguous and will span the array
			/// Values within one part are not guaranteed to have any order in place.
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
				return view().partition_sort(std::move(comparer), count);
			}





			// ------------------------------------------------------------------------------------------
			// Sorting

			/// Prefix sorting
			///
			/// The values will be rearranged into two parts. The first part will contain the \a count
			/// lowest values in sorted order. The second part will contain the remaining values without
			/// any guarantee of order. The order is defined by \a comparer and must be strict weak. The
			/// first and second part will be returned as view. If \a count is bigger than \a length(),
			/// then the whole array will be sorted.
			///
			/// @param comparer  A callable object which takes two constant references of type \a T and
			///                  returns a boolean indicating if both values are in right order
			/// @param count     Length of prefix which should be sorted
			///
			/// @note Average time complexity is \doctodo
			///
			template <Callable<bool, const T&, const T&> C>
			constexpr std::tuple<array_view<T>, array_view<T>> sort_prefix (C comparer, std::size_t count)
			{
				return view().sort_prefix(std::move(comparer), count);
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
			constexpr std::tuple<array_view<T>, array_view<T>> sort_suffix (C comparer, std::size_t count)
			{
				return view().sort_suffix(std::move(comparer), count);
			}

			/// Nth element sorting
			///
			/// Values will be rearranged into two partitions such that the first part contains the \a n
			/// lowest values and the second part contains the length() - n - 1 greatest values. The
			/// first part will be placed at the beginning of the array and the second part will be
			/// placed at the ending of the array, so that value at index \a n corresponds to the value
			/// if sorted with \a comparer. The order defined by \a comparer must be strict weak. The
			/// returning is a tuple of the first part, the value at the \a n th position and the second
			/// part or an empty optional container, if \a n is equal or greater the array's length.
			///
			/// @param comparer  A callable object which takes two constant references of type \a T and
			///                  returns a boolean indicating if both values are in right order
			///
			/// @note Average time complexity is ...
			///
			template <Callable<bool, const T&, const T&> C>
			constexpr optional<std::tuple<array_view<T>, T&, array_view<T>>> sort_nth (C comparer, const std::size_t n)
			{
				return view().sort_nth(std::move(comparer), n);
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
				return view().sort(std::move(comparer));
			}

			/// Stable sorting
			template <Callable<bool, const T&, const T&> C>
			constexpr void sort_stable (C comparer)
			{
				return view().sort_stable(std::move(comparer));
			}

	};
	
	
	// ----------------------------------------------------------------------------------------------
	// Appending a value
	
	template <typename T, Allocator A>
	template <typename ... As>
	constexpr void array<T, A>::append (As && ... arguments)
	{
		constexpr auto isNothrowMoveConstructible = std::is_nothrow_move_constructible<T>::value;
		constexpr auto isNothrowConstructible = std::is_nothrow_constructible<T, As ...>::value;
		
		if (used < allocation.length())
		{
			new (allocation.data() + used) T(std::forward<As>(arguments) ...);
			++used;
		}
		else
		{
			const auto requiredLength = used + 1;
			const auto desiredLength = align_length(requiredLength);
			auto newAllocation = allocator.template allocate<T>(desiredLength);
			
			if (newAllocation.length() < requiredLength)
			{
				allocator.deallocate(newAllocation);
				throw bad_alloc();
			}
			else if (isNothrowMoveConstructible and isNothrowConstructible)
			{
				move(newAllocation.data(), allocation.data(), used);
				new (newAllocation.data() + used) T(std::forward<As>(arguments) ...);
			}
			else
			{
				copy(newAllocation.data(), allocation.data(), used, [&](auto index)
				{
					destruct(newAllocation.data(), index);
					allocator.deallocate(newAllocation);
				});
				
				try
				{
					new (newAllocation.data() + used) T(std::forward<As>(arguments) ...);
				}
				catch (...)
				{
					destruct(newAllocation.data(), used);
					allocator.deallocate(newAllocation);
					throw;
				}
			}
			
			destruct(allocation.data(), used);
			allocator.deallocate(allocation);
			allocation = newAllocation;
			++used;
		}
	}
	
	
	// ----------------------------------------------------------------------------------------------
	// Appending a sequence
	
	template <typename T, Allocator A>
	template <BoundedSequence S>
	constexpr void array<T, A>::append (S sequence)
	{
		using element_type = sequence_type_t<S>;
		constexpr auto isNothrowMoveConstructible = std::is_nothrow_move_constructible<T>::value;
		constexpr auto isNothrowConstructible = std::is_nothrow_constructible<T, element_type>::value;
		const auto count = length(sequence);
		
		if (count == 0)
			return;
		
		if (used + count <= allocation.length() and (count == 1 or isNothrowConstructible))
		{
			construct(allocation.data() + used, std::move(sequence));
			used += count;
		}
		else
		{
			const auto requiredLength = used + count;
			const auto desiredLength = align_length(requiredLength);
			auto newAllocation = allocator.template allocate(desiredLength);
			
			if (newAllocation.length() < requiredLength)
			{
				allocator.deallocate(newAllocation);
				throw bad_alloc();
			}
			else if (isNothrowConstructible and isNothrowMoveConstructible)
			{
				move_construct(newAllocation.data(), allocation.data(), used);
				construct(newAllocation.data() + used, std::move(sequence));
			}
			else
			{
				copy(newAllocation.data(), allocation.data(), used, [&](auto index)
				{
					destruct(newAllocation.data(), index);
					allocator.deallocate(newAllocation);
				});
				
				construct(newAllocation.data() + used, std::move(sequence), [&](auto index)
				{
					destruct(newAllocation.data(), used + index);
					allocator.deallocate(newAllocation);
				});
			}
			
			destruct(allocation.data(), used);
			allocator.deallocate(allocation);
			allocation = newAllocation;
			used += count;
		}
	}

	// ----------------------------------------------------------------------------------------------
	// Prepending a value
	
	template <typename T, Allocator A>
	template <typename ... As>
	constexpr void array<T, A>::prepend (As && ... arguments)
	{
		constexpr auto isNothrowMoveConstructible = std::is_nothrow_move_constructible<T>::value;
		constexpr auto isNothrowMoveAssignable = std::is_nothrow_move_assignable<T>::value;
		constexpr auto isNothrowConstructible = std::is_nothrow_constructible<T, As ...>::value;
		
		if (used < allocation.length() and isNothrowMoveConstructible and isNothrowConstructible and (isNothrowMoveAssignable or used == 1))
		{
			if (used > 0)
				new (allocation.data() + used) T(std::move(*(allocation.data() + used - 1)));
			move_assignable_reverse(allocation.data() + 1, allocation.data(), used - 1);
			new (allocation.data()) T(std::forward<As>(arguments) ...);
			++used;
		}
		else
		{
			const auto requiredLength = used + 1;
			const auto desiredLength = align_length(requiredLength);
			auto newAllocation = allocator.template allocate<T>(desiredLength);
			
			if (newAllocation.length() < requiredLength)
			{
				allocator.deallocate(newAllocation);
				throw bad_alloc();
			}
			else if (isNothrowConstructible and (used == 0 or isNothrowMoveConstructible))
			{
				new (newAllocation.data()) T(std::forward<As>(arguments) ...);
				move_construct(newAllocation.data() + 1, allocation.data(), used);
			}
			else
			{
				try
				{
					new (newAllocation.data()) T(std::forward<As>(arguments) ...);
				}
				catch (...)
				{
					destruct(newAllocation.data(), 1);
					allocator.deallocate(newAllocation);
					throw;
				}
				
				copy_construct(newAllocation.data() + 1, allocation.data(), used);
			}
			
			destruct(allocation.data(), used);
			allocator.deallocate(allocation);
			allocation = newAllocation;
			++used;
		}
	}	
	
	// ----------------------------------------------------------------------------------------------
	// Prepending a sequence
	
	template <typename T, Allocator A>
	template <BoundedSequence S>
	constexpr void array<T, A>::prepend (S sequence)
	{
		using element_type = sequence_type_t<S>;
		constexpr auto isNothrowMoveConstructible = std::is_nothrow_move_constructible<T>::value;
		constexpr auto isNothrowMoveAssignable = std::is_nothrow_move_assignable<T>::value;
		constexpr auto isNothrowConstructible = std::is_nothrow_constructible<T, S>::value;
		const auto count = length(sequence);
		
		if (count == 0)
			return;
			
		if (used + count <= allocation.length() and (used == 0 or isNothrowMoveConstructible) and used <= count)
		{
			move_construct(allocation.data() + count, allocation.data(), used);
			sequence = move_assign(allocation.data(), std::move(sequence), used);
			construct(allocation.data() + used, std::move(sequence));
			used += count;
		}
		else if (used + count <= allocation.length() and (used == 0 or isNothrowMoveConstructible) and isNothrowMoveAssignable)
		{
			const auto gap = used - count;
			move_construct(allocation.data() + used, allocation.data() + gap, count);
			move_assign(allocation.data() + count, allocation.data(), gap);
			assign(allocation.data(), std::move(sequence));
			used += count;
		}
		else
		{
			const auto requiredLength = used + count;
			const auto desiredLength = align_length(requiredLength);
			auto newAllocation = allocator.template allocate<T>(desiredLength);
			
			if (newAllocation.length() < requiredLength)
			{
				allocator.deallocate(newAllocation);
				throw bad_alloc();
			}
			else if ((isNothrowMoveConstructible or used == 0) and isNothrowConstructible)
			{
				construct(newAllocation.data(), std::move(sequence));
				move_construct(newAllocation.data() + count, allocation.data(), used);
			}
			else
			{
				construct(newAllocation.data(), std::move(sequence), [&](auto index)
				{
					destruct(newAllocation.data(), index);
					allocator.deallocate(newAllocation);
				});
				
				copy_construct(newAllocation.data() + count, allocation.data(), used, [&](auto index)
				{
					destruct(newAllocation.data(), count + index);
					allocator.deallocate(newAllocation);
				});
			}
			
			destruct(allocation.data(), used);
			allocator.deallocate(allocation);
			allocation = newAllocation;
			used += count;
		}
	}

	// ----------------------------------------------------------------------------------------------
	// Inserting a value

	template <typename T, Allocator A>
	template <typename ... As>
	constexpr void array::insert (std::size_t index, As && ... arguments)
	{
		constexpr auto isNothrowMoveConstructible = std::is_nothrow_move_constructible<T>::value;
		constexpr auto isNothrowMoveAssignable = std::is_nothrow_move_assignable<T>::value;
		constexpr auto isNothrowConstructible = std::is_nothrow_constructible<T, As ...>::value;
		constexpr auto isNothrow = isNothrowMoveConstructible and isNothrowMoveAssignable and isNothrowConstructible;
		constexpr auto isNothrowCopyConstructible = std::is_nothrow_copy_constructible<T>::value;

		if (used < allocation.length() and index >= used and isNothrowConstructible)
		{
			new (allocation.data() + used) T(std::forward<As>(arguments) ...);
			++used;
		}
		else if (used < allocation.length() and index < used and isNothrow)
		{
			new (allocation.data() + used) T(std::move(*(allocation.data() + used - 1)));
			move_assign_reverse(allocation.data() + index + 1, allocation.data() + index, used - index - 1);
			*(allocation.data() + index) = T(std::forward<As>(arguments) ...);
			++used;
		}
		else
		{
			const auto requiredLength = used + 1;
			const auto desiredLength = align_length(requiredLength);
			auto newAllocation = allocate(desiredLength);

			if (newAllocation.length() < requiredLength)
			{
				deallocate(newAllocation);
				throw bad_alloc();
			}
			else if (isNothrowCopyConstructible and isNothrowConstructible)
			{
				copy_construct(newAllocation.data(), allocation.data(), index);
				new (newAllocation.data() + index) T(std::forward<As>(arguments) ...);
				copy_construct(newAllocation.data() + index + 1, allocation.data() + index, used - index);
			}
			else
			{
				copy_construct(newAllocation.data(), allocation.data(), index, [&](auto lastIndex)
				{
					destruct(newAllocation.data(), lastIndex);
					deallocate(newAllocation);
				});

				try
				{
					new (newAllocation.data() + index) T(std::forward<As>(arguments) ...);
				}
				catch (...)
				{
					destruct(newAllocation.data(), index);
					deallocate(newAllocation);
				}

				copy_construct(newAllocation.data() + index + 1, allocation.data() + index, used - index, [&](auto lastIndex)
				{
					destruct(newAllocation.data(), index + 1 + lastIndex);
					deallocate(newAllocation);
				});
			}

			destruct(allocation.data(), used);
			deallocate(allocation);
			allocation = newAllocation;
			++used;
		}
	}
	

	// ----------------------------------------------------------------------------------------------
	// Inserting of a sequence

	template <typename T, Allocator A>
	template <BoundedSequence S>
	constexpr void array<T, A>::insert (std::size_t index, S sequence)
	{
		using element_type = sequence_type_t<S>;
		constexpr auto isNothrowMoveConstructible = std::is_nothrow_move_constructible<T>::value;
		constexpr auto isNothrowMoveAssignable = std::is_nothrow_move_assignable<T>::value;
		constexpr auto isNothrowCopyConstructible = std::is_nothrow_copy_constructible<T>::value;
		constexpr auto isNothrowConstructible = std::is_nothrow_constructible<T, element_type>::value;
		const auto count = length(sequence);

		if (count == 0)
			return;

		if (used + count <= allocation.length() and used <= index and isNothrowConstructible)
		{
			construct(allocation.data() + used, std::move(sequence));
			used += count;
		}
		else if (used + count <= allocation.length() and used > index and index + count <= used and isNothrowMoveConstructible and isNothrowConstructible)
		{
			move_construct(allocation.data() + index + count, allocation.data() + index, used - index);
			construct(allocation.data() + index, std::move(sequence));
			used += count;
		}
		else if (used + count <= allocation.length() and used > index and index + count > used and isNothrowMoveConstructible and isNothrowMoveAssignable and isNothrowConstructible)
		{
			const auto offset = index + count;
			move_construct(allocation.data() + used, allocation.data() + offset, used - offset);
			copy_construct(allocation.data() + offset, allocation.data() + index, count);
			construct(allocation.data() + index, std::move(sequence));
			used += count;
		}
		else
		{
			const auto requiredLength = used + count;
			const auto desiredLength = align_length(requiredLength);
			auto newAllocation = allocate(desiredLength);

			if (newAllocation.length() < requiredLength)
			{
				deallocate(newAllocation);
				throw bad_alloc();
			}
			else if (used == 0 and isNothrowConstructible)
			{
				construct(newAllocation.data(), std::move(sequence));
			}
			else if (used > 0 and isNothrowCopyConstructible and isNothrowConstructible)
			{
				copy_construct(newAllocation.data(), allocation.data(), index);
				construct(newAllocation.data() + index, std::move(sequence));
				copy_construct(newAllocation.data() + index + count, allocation.data() + index, used - index);
			}
			else
			{
				copy_construct(newAllocation.data(), allocation.data(), index, [&](auto lastIndex)
				{
					destruct(newAllocation.data(), lastIndex);
					deallocate(newAllocation);
				});
				construct(newAllocation.data() + index, std::move(sequence), [&](auto lastIndex)
				{
					destruct(newAllocation.data(), index + lastIndex);
					deallocate(newAllocation);
				});
				copy_construct(newAllocation.data() + index + count, allocation.data() + index, used - index, [&](auto lastIndex)
				{
					destruct(newAllocation.data(), index + count + lastIndex);
					deallocate(newAllocation);
				});
			}

			destruct(allocation.data(), used);
			deallocate(allocation);
			allocation = newAllocation;
			used += count;
		}
	}

	// ----------------------------------------------------------------------------------------------
	// Erasing one value

	template <typename T, Allocator A>
	constexpr void array<T, A>::erase (std::size_t index)
	{
		constexpr auto isNothrowMoveAssignable = std::is_nothrow_move_assignable<T>::value;
		constexpr auto isNothrowCopyConstructible = std::is_nothrow_copy_constructible<T>::value;

		if (index < user and isNothrowMoveAssignable)
		{
			move_assign(allocation.data() + index, allocation.data() + index + 1, used - index - 1);
			(allocation.data() + used - 1)->~T();
			--used;
		}
		else if (index < user)
		{
			auto newAllocation = allocate(allocation.length());

			if (newAllocation.length() < used - 1)
			{
				deallocate(newAllocation);
				throw bad_alloc();
			}
			else if (isNothrowCopyConstructible)
			{
				copy_construct(newAllocation.data(), allocation.data(), index);
				copy_construct(newAllocation.data() + index, allocation.data() + index + 1, used - index - 1);
			}
			else
			{
				copy_construct(newAllocation.data(), allocation.data(), index, [&](auto lastIndex)
				{
					destruct(newAllocation.data(), lastIndex);
					deallocate(newAllocation);
				});
				copy_construct(newAllocation.data() + index, allocation.data() + index + 1, used - index - 1, [&](auto lastIndex)
				{
					destruct(newAllocation.data(), index + lastIndex);
					deallocate(newAllocation);
				});
			}

			destruct(allocation.data(), used);
			deallocate(allocation);
			allocation = newAllocation;
			--used;
		}
	}

	// ----------------------------------------------------------------------------------------------
	// Erasing range of values

	template <typename T, Allocator A>
	constexpr void array<T, A>::erase (std::size_t index, std::size_t count)
	{
		constexpr auto isNothrowMoveAssignable = std::is_nothrow_move_assignable<T>::value;
		constexpr auto isNothrowCopyConstructible = std::is_nothrow_copy_constructible<T>::value;

		const auto guarded_count = (index + count > user) ? (user - index) : count;
		const auto tailing = used - index - guarded_count;
		
		if (index < user and tailing == 0)
		{
			destruct(allocation.data() + index, guarded_count);
			used -= guarded_count;
		}
		else if (index < user and tailing > 0 and isNothrowMoveAssignable)
		{
			move_assign(allocation.data() + index, allocation.data() + index + guarded_count, tailing);
			destruct(allocation.data() + index + tailing, count);
			used -= guarded_count;
		}
		else if (index < user)
		{
			auto newAllocation = allocate(allocation.length());

			if (newAllocation.length() < index + tailing)
			{
				deallocate(newAllocation);
				throw bad_alloc();
			}
			else if (isNothrowCopyConstructible)
			{
				copy_construct(newAllocation.data(), allocation.data(), index);
				copy_construct(newAllocation.data() + index, allocation.data() + index + guarded_count, tailing);
			}
			else
			{
				copy_construct(newAllocation.data(), allocation.data(), index, [&](auto lastIndex)
				{
					destruct(newAllocation.data(), lastIndex);
					deallocate(newAllocation);
				});
				copy_construct(newAllocation.data() + index, allocation.data() + index + guarded_count, tailing,
					[&](auto lastIndex)
				{
					destruct(newAllocation.data(), index + lastIndex);
					deallocate(newAllocation);
				});
			}

			destruct(allocation.data(), used);
			deallocate(allocation);
			allocation = newAllocation;
			used -= guarded_count;
		}
	}

	// ----------------------------------------------------------------------------------------------
	// Erasing values marked by a predictor

	template <typename T, Allocator A>
	template <Callable<bool, const T&, std::size_t> C>
	constexpr void array<T, A>::erase (C predictor)
	{
		constexpr auto isNothrowMoveAssignable = std::is_nothrow_move_assignable<T>::value;
		constexpr auto isNothrowCopyConstructible = std::is_nothrow_copy_constructible<T>::value;
		const auto count = fold([&predictor](auto count, const T& value, std::size_t index)
		{
			return count + (predictor(value, index) ? 1 : 0);
		}, std::size_t(0));
		assert(count <= used);

		if (count > 0 and isNothrowMoveAssignable)
		{
			std::size_t nextFreeIndex = 0;
			while (nextFreeIndex < used and predictor(*(allocation.data() + nextFreeIndex), nextFreeIndex))
				++nextFreeIndex;

			std::size_t index = nextFreeIndex == used ? used : (nextFreeIndex + 1);
			while (index < used)
			{
				if (not predictor(*(allocation.data() + index), index))
				{
					*(allocation.data() + nextFreeIndex) = std::move(*(allocation.data() + index));
					++nextFreeIndex;
				}
				++index;
			}

			destruct(allocation.data() + nextFreeIndex, used - nextFreeIndex);
		}
		else if (count > 0)
		{
			auto newAllocation = allocate(allocation.length());

			if (newAllocation.length() < used - count)
			{
				deallocate(newAllocation);
				throw bad_alloc();
			}
			else if (isNothrowCopyConstructible)
			{
				std::size_t index = 0;
				std::size_t newIndex = 0;
				const auto values = allocation.data();

				while (index < used)
				{
					if (not predictor(values[index], index))
					{
						new (newAllocation.data() + newIndex) T(values[index]);
						++newIndex;
					}
					++index;
				}
			}
			else
			{
				std::size_t index = 0;
				std::size_t newIndex = 0;
				const auto values = allocation.data();

				while (index < used)
				{
					if (not predictor(values[index], index))
					{
						try
						{
							new (newAllocation.data() + newIndex) T(values[index]);
						}
						catch (...)
						{
							destruct(newAllocation.data(), newIndex);
							deallocate(newAllocation);
							throw;
						}
						++newIndex;
					}
					++index;
				}
			}

			destruct(allocation.data(), used);
			deallocate(allocation);
			allocation = newAllocation;
			used -= count;
		}
	}


	// ----------------------------------------------------------------------------------------------
	// Memory shrinkage

	template <typename T, Allocator A>
	constexpr void array<T, A>::shrink ()
	{
		constexpr auto isNothrowMoveConstructible = std::is_nothrow_move_constructible<T>::value;
		constexpr auto isNothrowCopyConstructible = std::is_nothrow_copy_constructible<T>::value;

		if (used == 0 and allocation.length() > 0)
		{
			deallocate(allocation);
			allocation = allocation_type();
		}
		else if (used > 0)
		{
			auto newAllocation = allocate(used);
			
			if (newAllocation.length() >= allocation.length())
			{
				deallocate(newAllocation);
				throw bad_alloc();
			}
			else if (isNothrowMoveConstructible)
			{
				move_construct(newAllocation.data(), allocation.data(), used);
			}
			else if (isNothrowCopyConstructible)
			{
				copy_construct(newAllocation.data(), allocation.data(), used);
			}
			else
			{
				copy_construct(newAllocation.data(), allocation.data(), used, [&](auto index)
				{
					destruct(newAllocation.data(), index);
					deallocate(newAllocation);
				});
			}
			
			destruct(allocation.data(), used);
			deallocate(allocation);
			allocation = newAllocation;
		}
	}

	// ----------------------------------------------------------------------------------------------
	// Reserving minimum of capacity

	template <typename T, Allocator A>
	constexpr void array<T, A>::reserve (std::size_t count)
	{
		constexpr auto isNothrowMoveConstructible = std::is_nothrow_move_constructible<T>::value;
		constexpr auto isNothrowCopyConstructible = std::is_nothrow_copy_constructible<T>::value;

		if (count > allocation.length())
		{
			auto newAllocation = allocate(count);

			if (newAllocation.length() < count)
			{
				deallocate(newAllocation);
				throw bad_alloc();
			}
			else if (isNothrowMoveConstructible)
			{
				move_construct(newAllocation.data(), allocation.data(), used);
			}
			else if (isNothrowCopyConstructible)
			{
				copy_construct(newAllocation.data(), allocation.data(), used);
			}
			else
			{
				copy_construct(newAllocation.data(), allocation.data(), used, [&](auto index)
				{
					destruct(newAllocation.data(), index);
					deallocate(newAllocation);
				});
			}

			destruct(allocation.data(), used);
			deallocate(allocation);
			allocation = newAllocation;
		}
	}

	// ----------------------------------------------------------------------------------------------
	// Move constructor for array with different allocator type

	template <typename T, Allocator A>
	template <Allocator B>
	constexpr array<T, A>::array (array<T, B> && other)
	{
		if (other.length() > 0)
		{
			const auto requiredLength = other.used;
			const auto desiredLength = align_length(requiredLength);
			auto newAllocation = allocate(desiredLength);

			if (newAllocation.length() < requiredLength)
			{
				deallocate(newAllocation);
				throw bad_alloc();
			}
			else if (std::is_nothrow_move_constructible<T>::value)
			{
				move_construct(newAllocation.data(), other.data(), requiredLength);
			}
			else if (std::is_nothrow_copy_constructible<T>::value)
			{
				copy_construct(newAllocaiton.data(), other.data(), requiredLength);
			}
			else
			{
				copy_construct(newAllocation.data(), other.data(), requiredLength, [&](auto index)
				{
					destruct(newAllocation.data(), index);
					deallocate(newAllocation);
				});
			}

			allocation = newAllocation;
			used = requiredLength;
		}
		else
		{
			used = 0;
		}
	}


	// ----------------------------------------------------------------------------------------------
	// Constructs with a sequence of elements and an allocator

	template <typename T, Allocator A>
	template <BoundedSequence<T> S>
	constexpr array<T, A>::array (S elements, A allocator)
		: allocator(std::move(allocator))
	{
		const auto count = length(elements);

		if (count > 0)
		{
			auto newAllocation = allocate(count);
			
			if (newAllocation.length() < count)
			{
				deallocate(newAllocation);
				throw bad_alloc();
			}
			else if (std::is_nothrow_move_constructible<T>::value)
			{
				construct(newAllocation.data(), std::move(elements));
			}
			else
			{
				construct(newAllocation.data(), std::move(elements), [&](auto index)
				{
					destruct(newAllocation.data(), index);
					deallocate(newAllocation);
				});
			}
			
			allocation = newAllocation;
		}
		used = count;
	}


	// ----------------------------------------------------------------------------------------------
	// Copy assignment

	template <typename T, Allocator A>
	template <Allocator B>
	constexpr array<T, A>& array<T, A>::operator = (const array<T, B> & other)
	{
		constexpr auto isNothrowAssign = std::is_nothrow_copy_assignable<T>::value;
		constexpr auto isNothrowConstruct = std::is_nothrow_copy_constructible<T>::value;

		if (other.used == 0 and used > 0)
		{
			destruct(allocation.data(), used);
			used = 0;
		}
		else if (other.used > 0 and used >= other.used and isNothrowAssign)
		{
			copy_assign(allocation.data(), other.data(), other.used);
			destruct(allocation.data() + other.used, used - other.used);
			used = other.used;
		}
		else if (other.used > 0 and allocation.length() >= other.used and used == 0 and isNothrowConstruct)
		{
			copy_construct(allocation.data(), other.data(), other.used);
			used = other.used;
		}
		else if (other.used > 0 and allocation.length() >= other.used and used > 0 and isNothrowAssign and isNothrowConstruct)
		{
			copy_assign(allocation.data(), other.data(), used);
			copy_construct(allocation.data() + used, other.data() + used, other.used - used);
			used = other.used;
		}
		else if (other.used > 0)
		{
			auto newAllocation = allocate(other.used);
			if (newAllocation.length() < other.used)
			{
				destruct(newAllocation);
				throw bad_alloc();
			}
			else if (isNothrowConstruct)
			{
				copy_construct(newAllocation.data(), other.data(), other.used);
			}
			else
			{
				copy_construct(newAllocation.data(), other.data(), other.used, [&](auto index)
				{
					destruct(newAllocation.data(), index);
					deallocate(newAllocation);
				});
			}

			destruct(allocation.data(), used);
			deallocate(allocation);
			allocation = newAllocation;
			used = other.used;
		}

		return *this;
	}


	// ----------------------------------------------------------------------------------------------
	// Sequence assignment

	template <typename T, Allocator A>
	template <BoundedSequence<T> S>
	array<T, A>& array<T, A>::operator = (S elements)
	{
		constexpr auto isNothrowAssign = std::is_nothrow_move_assignable<T>::value;
		constexpr auto isNothrowConstruct = std::is_nothrow_move_constructible<T>::value;
		const auto count = length(elements);

		if (count == 0 and used > 0)
		{
			destruct(allocation.data(), used);
			used = 0;
		}
		else if (count > 0 and used >= count and isNothrowAssign)
		{
			assign(allocation.data(), std::move(elements));
			destruct(allocation.data() + count, used - count);
			used = count;
		}
		else if (count > 0 and allocation.length() >= count and used == 0 and isNothrowConstruct)
		{
			construct(allocation.data(), std::move(elements));
			used = count;
		}
		else if (count > 0 and allocation.length() >= count and used > 0 and isNothrowConstruct and isNothrowAssign)
		{
			elements = assign(allocation.data(), std::move(elements), used);
			construct(allocation.data() + used, std::move(elements));
			used = count;
		}
		else if (count > 0)
		{
			auto newAllocation = allocate(count);
			if (newAllocation.length() < count)
			{
				destruct(newAllocation);
				throw bad_alloc();
			}
			else if (isNothrowConstruct)
			{
				construct(newAllocation.data(), std::move(elements));
			}
			else
			{
				construct(newAllocation.data(), std::move(elements), [&](auto index)
				{
					destruct(newAllocation.data(), index);
					deallocate(newAllocation);
				});
			}

			destruct(allocation.data(), used);
			deallocate(allocation);
			allocation = newAllocation;
			used = count;
		}

		return *this;
	}

}

#endif

