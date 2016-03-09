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

			/// Copies \a count values from \a source to \a destination
			static void copy (T * destination, const T * source, std::size_t count, Callable_<std::size_t> cleaner)
			{
				for (std::size_t index = 0; index < count; ++index)
				{
					try
					{
						new (destination + index) T(source[index]);
					}
					catch (...)
					{
						cleaner(index);
						throw;
					}
				}
			}

			static void copy (T * destination, IndexedBoundedSequence values, Callable_<std::size_t> cleaner)
			{
				const auto valuesCount = values.length();
				fold([cleaner=std::move(cleaner), data=destination, limit=valuesCount](auto index, auto value)
				{
					assert(index < limit);
					try
					{
						new (data + index) T(std::move(value));
					}
					catch (...)
					{
						cleaner(index);
						throw;
					}
					return index + 1;
				}, std::size_t(0), std::move(values));
			}

			template <Allocator B>
			static std::tuple<allocation_type, std::size_t> construct (array<T, B> && values, A & allocator)
			{
				allocation_type allocation;
				if (values.length() > 0)
				{
					allocation = allocator.template allocate<T>(values.length());
					if (allocation.length() < values.length())
					{
						allocator.deallocate(allocation);
						throw bad_alloc();
					}
					move(allocation.data(), );
					else if (std::is_nothrow_move_constructible<T>::value)
					{
						auto valuesAllocation = values.allocation;
						for (std::size_t index = 0; index < values.length(); ++index)
							new (allocation.data() + index) T(std::move(values.))
					}
				}
				return std::make_tuple(allocation, values.length());
			}

			template <Allocator B>
			static std::tuple<allocation_type, std::size_t> construct (BoundedSequence<T> values, A & allocator)
			{
				allocation_type allocation;
				const auto count = length(values);
				if (count > 0)
				{
					allocation = allocator.template allocate<T>(count);
					if (allocation.length() < count)
					{
						allocator.deallocate(allocation);
						throw bad_alloc();
					}
					else
					{
						fold([allocation=allocation](auto index, auto value)
						{
							try
							{
								new (allocation.data() + index) T(std::move(value));
							}
							catch (...)
							{
								destruct(ptr, index);
								allocator.deallocate(allocation);
								throw;
							}
						}, std::size_t(0), std::move(values))
					}
				}
				return std::make_tuple(allocation, count);
			}

			template <typename ... As>
			static constexpr void move_and_append (T* destination, const T* source, std::size_t count, As&& ... arguments)
			{
				assert(std::is_nothrow_move_constructible<T>::value);
				assert(std::is_nothrow_constructible<T, As ...>::value);

				for (std::size_t index = 0; index < count; ++index)
					new (destination + index) T(std::move(*(source + index)));
				new (destination + count) T(std::forward<As>(arguments) ...);
			}


		public:


			/// Constructor with allocator
			///
			/// The allocator is set to \a allocator. No memory allocation will be performed. The array
			/// will be empty regarding its capacity and hence its values.
			constexpr explicit array (A allocator) noexcept(std::is_nothrow_move_constructible<A>::value)
				: allocator(std::move(allocator))
			{}
			
			/// Move constructor with an array having a different allocator type
			///
			/// The allocator will be default constructed. If \a other contains some values, memory will
			/// be allocated from its own allocator. Values from \a other will be moved into the newly
			/// constructed object.
			template <Allocator B> constexpr array (array<T, B> && other)
			{
				if (other.length() > 0)
				{
					auto allocation = allocator.template allocate<T>(other.length());
					if (allocation.length() < other.length())
					{
						allocator.deallocate(allocation);
						throw bad_alloc();
					}
					else
					{
						const auto count = other.length();
						other.remove([&](auto value, auto index)
						{
							assert(index < count);
							try { new (allocation.data() + index) T(std::move(value)); }
							catch (...)
							{
								destruct(allocation.data(), index);
								allocator.deallocate(allocation);
								throw;
							}
						});
						this->allocation = allocation;
						usedLength = count;
					}
				}
			}

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
			{
				std::tie(allocation, usedLength) = construct(std::move(values), allocator);
			}

			/// Constructor with sequence \a values and \a allocator
			///
			/// The allocator will be set to \a allocator. If \a values contains any value, memory will
			/// be allocated from the allocator. All values in \a values will be stored in the newly
			/// constructed object.
			constexpr array (BoundedSequence<T> values, A allocator)
				: allocator(std::move(allocator))
			{
				std::tie(allocation, usedLength) = construct(std::move(values), this->allocator);
			}

			/// Constructor reserving memory for at least \a count values
			///
			/// The allocator will be constructed by default. If not enough memory could be allocated,
			/// the exception \a bad_alloc will be thrown. No value will be constructed.
			explicit constexpr array (std::size_t count)
				: usedLength(0)
			{
				allocation = allocator.template allocate<T>(count);
				if (allocation.length() < count)
				{
					allocator.deallocate(allocation);
					throw bad_alloc();
				}
			}

			/// Constructor reserving memory for at least \a count values with \a allocator
			///
			/// The allocator will be set to \a allocator. If not enough memory could be allocated, the
			/// exception \a bad_alloc will be thrown. No value will be constructed.
			constexpr array (std::size_t length, A allocator)
				: allocator(std::move(allocator)), usedLength(0)
			{
				allocation = this->allocator.template allocate<T>(count);
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
			template <Allocator B> array& operator = (const array<T, B> & other)
			{
				constexpr auto isNothrowCopyable = std::is_nothrow_copy_constructible<T>::value;
				auto otherView = other.view();
				if (allocation.length() >= other.length() and isNothrowCopyable)
				{
					destruct(allocation.data(), allocation.length());
					auto combiner = [](auto ptr, const T & value)
					{
						new (ptr) T(value);
						return ptr+1;
					};
					otherView.fold(combiner, allocation.data());
					usedLength = other.length();
				}
				else
				{
					auto newAllocation = allocator.template allocate<T>(other.length());
					if (newAllocation.length() < other.length())
					{
						allocator.deallocate(newAllocation);
						throw bad_alloc();
					}
					else
					{
						auto combiner = [&](auto index, const T& value)
						{
							try { new (newAllocation.data() + index) T(value); }
							catch (...)
							{
								destruct(newAllocation.data(), index);
								allocator.deallocate(newAllocation);
								throw;
							}
							return index + 1;
						};
						otherView.fold(combiner, std::size_t(0));
						destruct(allocation.data(), usedLength);
						allocator.deallocate(allocation);
						allocation = newAllocation;
						usedLength = other.length();
					}
				}
				return *this;
			}

			/// Sequence assignment
			///
			/// Any value in the own instance will be destructed. All \a values will be copied into the
			/// the own memory allocation. If the allocation is not big enough, new memory will be
			/// allocated. If any exception is raised, the precalling statei will be restored.
			array& operator = (BoundedSequence values)
			{
				using value_type = sequence_type_t<decltype(values)>;
				constexpr auto isNothrow = std::is_nothrow_constructible<T, value_type>::value;
				const auto countValues = length(values);
				if (allocation.length() >= countValues and isNothrow)
				{
					destruct(allocation.data(), allocation.length());
					auto combiner = [limit=countValues, data=allocation.data()](auto index, auto value)
					{
						assert(index < limit);
						new (data + index) T(std::move(value));
						return index + 1;
					};
					fold(combiner, std::size_t(0), std::move(values));
					usedLength = countValues;
				}
				else
				{
					auto newAllocation = allocator.template allocate<T>(countValues);
					if (newAllocation.length() < countValues)
					{
						allocator.deallocate(newAllocation);
						throw bad_alloc();
					}
					else
					{
						auto combiner = [limit=countValues, &](auto index, auto value)
						{
							assert(index < limit);
							try { new (newAllocation.data() + index) T(std::move(value)); }
							catch (...)
							{
								destruct(newAllocation.data(), index);
								allocator.deallocate(newAllocation);
								throw;
							}
							return index + 1;
						};
						fold(combiner, std::size_t(0), std::move(values));
						destruct(allocation.data(), usedLength);
						allocator.deallocate(allocation);
						allocation = newAllocation;
						usedLength = countValues;
					}
				}
				return *this;
			}



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

			template <typename ... As>
				requires std::is_constructible<T, As ...>::value
			constexpr void insert (std::size_t index, As && ... arguments);
			
			template <BoundedSequence S>
				requires std::is_constructible<T, sequence_type_t<S>>::value
			constexpr void insert (std::size_t index, BoundedSequence S);

			constexpr void erase (std::size_t index);
			
			constexpr void erase (std::size_t index, std::size_t count);





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

			
			/// Capacity reservation
			///
			/// If no expection is thrown, the capacity will be assured to be able to contain at least
			/// \a count values without any reallocation. If an exception is thrown, the object will
			/// remain its precalling state.
			void reserve (std::size_t count)
			{
				if (allocation.length() < count)
				{
					auto newAllocation = allocator.template allocate<T>(count);
					if (newAllocation.length() < count)
					{
						allocator.deallocate(newAllocation);
						throw bad_alloc();
					}
					else if (std::is_nothrow_move_constructible<T>::value)
					{
						for (std::size_t index = 0; index < usedLength; ++index)
							new (newAllocation.data() + index) T(std::move(*(allocation.data() + index)));
					}
					else
					{
						copy(newAllocation.data(), allocation.data(), usedLength, [&](auto index)
						{
							destruct(newAllocation.data(), index);
							allocator.deallocate(newAllocation);
						});
						destruct(allocation.data(), usedLength);
					}
					allocator.deallocate(allocation);
					newAllocation = allocation;
				}
			}

			/// Memory shrinkage
			///
			/// Memory footprint will be reduced to a minimum. If no value is contained, all memory will
			/// be deallocated. If no memory can be allocated, the
			void shrink_to_fit ()
			{
				if (allocation.length() > 0 and usedLength == 0)
				{
					allocator.deallocate(allocation);
					allocation = allocation_type();					
				}
				else if (allocation.length() and usedLength > 0)
				{
					auto newAllocation = allocator.template allocate<T>(usedLength);
					if (newAllocation.length() < usedLength)
					{
						allocator.deallocate(newAllocation);
					}
					else if (newAllocation.length() >= allocation.length())
					{
						allocator.deallocate(newAllocation);
					}
					else if (std::is_nothrow_move_constructible<T>::value)
					{
						for (std::size_t index = 0; index < usedLength; ++index)
							new (newAllocation.data() + index) T(std::move(*(allocation.data() + index)));
						allocator.deallocate(allocation);
						allocation = newAllocation;
					}
					else
					{
						bool onSuccess = true;
						for (std::size_t index = 0; onSuccess and index < usedLength; ++index)
						{
							try
							{
								new (newAllocation.data() + index) T(*(allocation.data() + index));
							}
							catch (...)
							{
								destruct(newAllocation.data(), index);
								allocator.deallocate(newAllocation);
								onSuccess = false;
							}
						}

						if (onSuccess)
						{
							destruct(allocation.data(), usedLength);
							allocator.deallocate(allocation);
							allocation = newAllocation;
						}
					}
				}
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

			


			/// Removes all values
			///
			/// All values are removed from the sequence and passed on to \a mover. Sidealong, \a value
			/// is passed through. Depending on \a forward, the values are traversed from front to back
			/// or from back to front.
			template <typename V> constexpr V remove (Callable<V, V, T> mover, V value, bool forward)
			{
				const auto isNothrow = std::is_nothrow_move_constructible<T>::value;
				if (isNothrow)
				{
					if (forward) for (std::size_t index = 0; index < usedLength; ++index)
						value = mover(std::move(value), std::move(*(allocation.data() + index)));
					else for (std::size_t index = usedLength; index > 0; --index)
						value = mover(std::move(value), std::move(*(allocation.data() + index - 1)));
				}
				else
				{
					if (forward) for (std::size_t index = 0; index < usedLength; ++index)
						value = mover(std::move(value), T(*(allocation.data() + index)));
					else for (std::size_t index = 0; index > 0; --index)
						value = mover(std::move(value), T(*(allocation.data() + index - 1)));
					destruct(allocation.data(), usedLength);
				}
				usedLength = 0;
				return value;
			}

			template <typename V> constexpr V remove (Callable<std::tuple<optional<T>, V>, V, T> mover, V value, bool forward)
			{
				const auto isNothrow = std::is_nothrow_move_constructible<T>::value;
				if (isNothrow and forward)
				{
					std::size_t nextIndex = 0;
					if (forward) for (std::size_t index = 0; index < usedLength; ++index)
					{
						auto replacing = move(std::move(value), std::move(*(allocation.data() + index)));
						value = std::move(std::get<1>(replacing));
						nextIndex = decide([p=allocation.data()](auto value, auto nextIndex)
						{
							
						}, [](auto nextIndex)
						{
							return nextIndex;
						}, std::move(std::get<0>(replacing)), nextIndex);
					}
				}
			}

			/// Removes a span of values
			///
			/// All values starting at index \a pos to inclusively index \a pos + \a count - 1 are
			/// removed and passed on 
			template <typename V> constexpr V remove (Callable<V, V, T> mover, V value, std::size_t pos, std::size_t count, bool forward)
			{
				
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
			move(allocation.data() + used, std::move(sequence), count);
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
				move(newAllocation.data(), allocation.data(), used);
				move(newAllocation.data() + used, std::move(sequence), count);
			}
			else
			{
				copy(newAllocation.data(), allocation.data(), used, [&](auto index)
				{
					destruct(newAllocation.data(), index);
					allocator.deallocate(newAllocation);
				});
				
				move(newAllocation.data() + used, std::move(sequence), count, [&](auto index)
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
			move_construct(allocation.data() + used, std::move(sequence), count - used);
			used += count;
		}
		else if (used + count <= allocation.length() and (used == 0 or isNothrowMoveConstructible) and isNothrowMoveAssignable)
		{
			const auto gap = used - count;
			move_construct(allocation.data() + used, allocation.data() + gap, count);
			move_assign(allocation.data() + count, allocation.data(), gap);
			move_assign(allocation.data(), std::move(sequence), count);
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
				move_construct(newAllocation.data(), std::move(sequence), count);
				move_construct(newAllocation.data() + count, allocation.data(), used);
			}
			else
			{
				move_construct(newAllocation.data(), std::move(sequence), count, [&](auto index)
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
			move_construct(allocation.data() + used, std::move(sequence), count);
			used += count;
		}
		else if (used + count <= allocation.length() and used > index and index + count <= used and isNothrowMoveConstructible and isNothrowConstructible)
		{
			move_construct(allocation.data() + index + count, allocation.data() + index, used - index);
			copy_construct(allocation.data() + index, std::move(sequence), count);
			used += count;
		}
		else if (used + count <= allocation.length() and used > index and index + count > used and isNothrowMoveConstructible and isNothrowMoveAssignable and isNothrowConstructible)
		{
			const auto offset = index + count;
			move_construct(allocation.data() + used, allocation.data() + offset, used - offset);
			copy_construct(allocation.data() + offset, allocation.data() + index, count);
			copy_construct(allocation.data() + index, std::move(sequence), count);
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
				move_construct(newAllocation.data(), std::move(sequence), count);
			}
			else if (used > 0 and isNothrowCopyConstructible and isNothrowConstructible)
			{
				copy_construct(newAllocation.data(), allocation.data(), index);
				move_construct(newAllocation.data() + index, std::move(sequence), count);
				copy_construct(newAllocation.data() + index + count, allocation.data() + index, used - index);
			}
			else
			{
				copy_construct(newAllocation.data(), allocation.data(), index, [&](auto lastIndex)
				{
					destruct(newAllocation.data(), lastIndex);
					deallocate(newAllocation);
				});
				move_construct(newAllocation.data() + index, std::move(sequence), count, [&](auto lastIndex)
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
		
	}

	// ----------------------------------------------------------------------------------------------
	// Erasing values marked by a predictor

	template <typename T, Allocator A>
	template <Callable<bool, const T&> C>
	constexpr void array<T, A>::erase (C predictor)
	{
		
		
		const auto count = fold([&predictor](auto count, const T& value)
		{
			return count + (predictor(value) ? 1 : 0);
		}, std::size_t(0));


	}

}

#endif

