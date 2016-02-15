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
			std::size_t usedLength = 0;


			static void destruct (T * values, std::size_t count)
			{
				assert(values != nullptr or count == 0);
				if (not std::is_trivial_destructible<T>::value)
				{
					for (std::size_t index = 0; index < count; ++index)
						values[index].~T();
				}
			}
	
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

		public:

			/// Default constructor
			///
			/// The allocator will be constructed by default. No memory allocation will be performed. The
			/// array will be empty regarding its capacity and hence its values.
			constexpr array () = default;

			/// Constructor with allocator
			///
			/// The allocator is set to \a allocator. No memory allocation will be performed. The array
			/// will be empty regarding its capacity and hence its values.
			constexpr explicit array (A allocator) noexcept(std::is_nothrow_move_constructible<A>::value)
				: allocator(std::move(allocator))
			{}

			/// Move constructor
			///
			/// The allocator and any allocation will be moved from \a other to the newly constructed
			/// object. All values in \a other will belong to the newly constructed object.
			constexpr array (array && other) = default;

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

			/// Copy constructor
			///
			/// The allocator will be copied from the allocator instance of \a other. If \a other
			/// contains some values, memory will be allocated from the copied allocator object. Values
			/// from \a other will be copied into the nwly constructed object. The array \a other will
			/// not e modified in any way.
			constexpr array (const array & other)
				: array(other.view(), other.allocator)
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



			/// Appending some value constructing with \a arguments
			///
			/// The value will be constructed with \a arguments and appended to the sequence. If more
			/// memory will be required to hold all values, new memory will be allocated. If any
			/// exception is thrown, the precalling state will be restored and the exception will be
			/// rethrown. The value will be constructed in place.
			template <typename ... Args> void append (Args && ... arguments)
				requires std::is_constructible<T, Args ...>::value
			{
				if (usedLength + 1 <= allocation.length())
				{
					new (allocation.data() + usedLength) T(std::forward<Args>(arguments) ...);
					usedLength++;
				}
				else
				{
					auto newAllocation = allocator.template allocate<T>(next_size(usedLength + 1));
					if (newAllocation.length() < usedLength + 1)
					{
						allocator.deallocate(newAllocation);
						throw bad_alloc();
					}
					else if (std::is_nothrow_constructible<T, Args ...>::value)
					{
						for (std::size_t index = 0; index < usedLength; ++index)
							new (newAllocation.data() + index) T(std::move(*(allocation.data() + index)));
						new (newAllocation.data() + usedLength) T(std::forward<Args>(arguments) ...);
					}
					else
					{
						copy(newAllocation.data(), allocation.data(), usedLength, [&](std::size_t index)
						{
							destruct(newAllocation.data(), index);
							allocator.deallocate(newAllocation);
						});
						try
						{
							new (newAllocation.data() + usedLength) T(std::forward<Args>(arguments) ...);
						}
						catch (...)
						{
							destruct(newAllocation.data(), usedLength());
							allocator.deallocate(newAllocation);
							throw;
						}
						destruct(allocation.data(), usedLenght);
					}
					allocator.deallocate(allocation);
					newAllocation = allocation;
					usedLength++;
				}
			}

			/// Appending a sequence of values
			///
			/// All \a values will be appended to the sequence such that the first value in \a values
			/// will be the first appended value, the second value in \a values will be the second
			/// appended value, and so on. If any exception is thrown, the precalling state will be
			/// restored and the exception will be rethrown.
			template <BoundedSequence S>
				requires std::is_constructible<S, sequence_type_t<S>>::value
			void append (S values)
			{
				using value_type = sequence_type_t<decltype(values)>;
				const auto countedValues = length(values)
				constexpr auto value_type_nothrow = std::is_nothrow_constructible<T, value_type>::value;
				constexpr auto move_nothrow = std::is_nothrow_move_constructible<T>::value;
				if (usedLength + countedValues <= allocation.length() and value_type_nothrow)
				{
					fold([data=allocation.data(), limit=allocation.length()](auto index, auto value)
					{
						assert(index < limit);
						new (data + index) T(std::move(value));
						return index + 1;
					}, usedLength, std::move(values));
					usedLength += countedValues;
				}
				else
				{
					const auto requiredLength = usedLength + countedValues;
					const auto desiredLength = align_size(requiredLength);
					auto newAllocation = allocator.template allocate<T>(desiredLength);
					if (newAllocation.length() < requiredLength)
					{
						allocator.deallocate(newAllocation);
						throw bad_alloc();
					}
					else if (move_nothrow and value_type_nothrow)
					{
						for (std::size_t index = 0; index < usedLength; ++index)
							new (newAllocation.data() + index) T(std::move(*(allocation.data() + index)));
						fold([data=newAllocation.data(), requiredLength](auto index, auto value)
						{
							assert(index < requiredLength);
							new (data + index) T(std::move(value));
							return index + 1;
						}, usedLength, std::move(values));						
					}
					else
					{
						copy(newAllocation.data(), allocation.data(), usedLength, [&](std::size_t index)
						{
							destruct(newAllocation.data(), index);
							allocator.deallocate(newAllocation);
						});
						copy(newAllocation.data() + usedLength, std::move(values), [&](std::size_t index)
						{
							destruct(newAllocation.data(), usedLength + index);
							allocator.deallocate(newAllocation);
						});
						destruct(allocation.data(), usedLength);
					}
					allocator.deallocate(allocation);
					allocation = newAllocation;
					usedLength = requiredLength;
				}
			}


			/// Prepending a value constructing with \a arguments
			///
			/// The value will be constructed with \a arguments and prepended to the sequence. If more
			/// memory will be required to hold all values, new memory will be allocated. If any
			/// exception is thrown, the precalling state will be restored and the exception will be
			/// rethrown. The value will be constructed in place.
			template <typename ... As> void prepend (As && ... arguments)
				requires std::is_constructible<T, As ...>::values
			{
				const auto isNothrowMovable = std::is_nothrow_move_constructible<T>::value;
				const auto isNothrowConstructible = std::is_nothrow_constructible<T, As ...>::value;
				if (usedLength < allocation.data() and isNothrowMovable and isNothrowConstructible)
				{
					for (std::size_t index = usedLength; index > 0; --index)
						new (allocation.data() + index) T(std::move(*(allocation.data() + index - 1)));
					new (allocation.data()) T(std::forward<As>(arguments) ...);
				}
				else
				{
					const auto requiredLength = usedLength + 1;
					const auto desiredLength = assign_length(requiredLength);
					auto newAllocation = allocator.template allocate<T>(desiredLength);
					if (newAllocation.length() < requiredLength)
					{
						allocator.deallocate(newAllocation);
						throw bad_alloc();
					}
					else if (isNothrowMovable and isNothrowConstructible)
					{
						for (std::size_t index = usedLength; index > 0; --index)
							new (newAllocation.data() + index) T(std::move(*(allocation.data() + index - 1)));
						new (newAllocation.data()) T(std::forward<As>(arguments) ...);
					}
					else
					{
						copy(newAllocation.data() + 1, allocation.data(), usedLength, [&](auto index)
						{
							destruct(newAllocation.data() + 1, index);
							allocator.deallocate(newAllocation);
						});
						try
						{
							new (newAllocation.data()) T(std::forward<As>(arguments) ...);
						}
						catch (...)
						{
							destruct(newAllocation.data() + 1, usedLength);
							allocator.deallocate(newAllocation);
							throw;
						}
						destruct(allocation.data(), usedLength);
					}
					allocator.deallocate(allocation);
					allocation = newAllocation;
				}
				usedLength++;
			}

			/// Prepending \a values
			///
			/// All \a values will be prepended to the sequence such that the first value in \a values
			/// will be the first value in the eventual sequence, the second value will be the second
			/// value in the eventual sequence, and so on. If any exception is thrown, the precalling
			/// state will be restored and the exception will be rethrown.
			template <BoundedSequence S>
				requires std::is_nothrow_constructible<T, sequence_type_t<S>>
			void prepend (S values)
			{
				using value_type = sequence_type_t<decltype(values)>;
				const auto countedValues = length(values);
				constexpr auto isNothrowValue = std::is_nothrow_constructible<T, value_type>::value;
				constexpr auto isNothrowMove = std::is_nothrow_move_constructible<T>::value;
				if (usedLength + countedValues <= allocation.length() and isNothrowValue and isNothrowMove)
				{
					for (std::size_t index = usedLength; index > 0; --index)
						new (allocation.data() + index + countedValues - 1) T(std::move(*(allocation.data() + index - 1)));
					fold([data=allocation.data(), limit=allocation.length()](auto index, auto value)
					{
						assert(index < limit);
						new (data + index) T(std::move(value));
						return index + 1;
					}, usedLength, std::move(values));
					usedLength += countedValues;
				}
				else
				{
					const auto requiredLength = usedLength + countedValues;
					const auto desiredLength = align_size(requiredLength);
					auto newAllocation = allocator.template allocate<T>(desiredLength);
					if (newAllocation.length() < requiredLength)
					{
						allocator.deallocate(newAllocation);
						throw bad_alloc();
					}
					else if (move_nothrow and value_type_nothrow)
					{
						const auto total = fold([data=newAllocation.data(), limit=countedValues](auto index, auto value)
						{
							assert(index < limit);
							new (data + index) T(std::move(value));
							return index + 1;
						}, std::size_t(0), std::move(values));
						assert(total == countedValues);
						for (std::size_t index = 0; index < usedLength; ++index)
						{
							const auto dest = newAllocation.data() + countedValues + index;
							const auto src = allocation.data() + index;
							new (dest) T(std::move(*src));
						}
					}
					else
					{
						copy(newAllocation.data(), allocation.data(), usedLength, [&](std::size_t index)
						{
							destruct(newAllocation.data(), index);
							allocator.deallocate(newAllocation);
						});
						copy(newAllocation.data() + usedLength, std::move(values), [&](std::size_t index)
						{
							destruct(newAllocation.data(), usedLength + index);
							allocator.deallocate(newAllocation);
						});
						destruct(allocation.data(), usedLength);
					}
					allocator.deallocate(allocation);
					allocation = newAllocation;
					usedLength = requiredLength;
				}
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




	};

}

#endif

