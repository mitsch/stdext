/// @file allocator.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_ALLOCATOR_HPP__
#define __STDEXT_ALLOCATOR_HPP__

#include <tuple>
#include <utilities>
#include <stdext/array_view.hpp>
#include <malloc>

namespace stdext
{



	/// Basic allocation information
	///
	/// The basic allocation information implements the concept of \a Allocation. It can be used a
	/// base class for a more information riched data structure. It basically keeps record of the
	/// pointer to the memory allocation and its length in terms of the data type \a T.
	template <typename T> class basic_allocation
	{
		private:

			T* ptr;
			std::size_t count;

		public:

			/// Parameters constructor sets \a ptr and \a count
			constexpr basic_allocation (T * ptr, std::size_t count) noexcept
				: ptr(ptr), count(count)
			{}

			/// Default constructor
			///
			/// An empty allocation, that is an allocation of length zero is constructed.
			constexpr basic_allocation () noexcept
				: ptr(nullptr), count(0)
			{}

			/// Copy assignment
			///
			/// Copy constructor copies the information from \a other
			basic_allocation (const basic_allocation & other) = default;

			/// Default destructor does basically nothing
			~basic_allocation() = default;

			/// Copy assignment
			///
			/// The pointer and the length of \a other is copied.
			basic_allocation& operator = (const basic_allocation & other) = default;

			
			/// Returns the pointer to the allocation
			constexpr T* data () noexcept {return ptr;}

			/// Returns the length of the allocation
			constexpr std::size_t length () noexcept {return count;}

			/// Swaps the pointer and length of the allocation between \a first and \a second
			void swap (basic_allocation & first, basic_allocation & second)
			{
				using std::swap;
				swap(first.ptr, second.ptr);
				swap(first.count, second.count);
			}
	};


	/// Allocation concept
	///
	/// Concept of an allocation is to have provide methods to obtain the pointer to the beginning of
	/// the allocation (\a length) and the amount of elements of type \a T to be contained in the
	/// allocation (\a data).
	template <typename A, typename T> concept bool Allocation = requires ()
	{
		return requires (A a, A, b, T t)
		{
			{a.length()} -> std::size_t,
			{a.data()} -> T*,
			swap(a, b)
		};
	}


	/// Allocator concept
	///
	/// An allocator should allow to allocate memory and to deallocate them as well.
	template <typename T> concept bool Allocator = require ()
	{
		struct custom_type{int;};
		return requires (T t)
		{
			T(std::move(t)),
			t.template <custom_type> allocate(std::size_t(0))
		} and
		Allocation<decltype(std::declval<T>().template<custom_type> allocate(std::size_t(0)))> and
		requires (T t)
		{
			t.deallocate(t.template <custom_type> allocate(std::size_t(0)))
		};
	}


	template <Allocator A, typename T> struct allocation_type
	{
		using type = decltype(std::declval<A>().template allocate<T>(std::size_t(0)));
	};
	template <Allocator A, typename T> using allocation_type_t = typename allocation_type<A, T>;



	/// Exception in the allocation or deallocation process
	class bad_alloc
	{
		
		private:

			const char * reason;

		public:

			explicit constexpr bad_alloc (const char * reason) noexcept
				: reason(reason)
			{}

			constexpr bad_alloc (const bad_alloc & exception) noexcept
				: reason(exception.reason)
			{}

			constexpr const char * what () const
			{
				return reason;
			}
	};


	/// Allocation from system allocator
	template <typename T> class system_allocation : public basic_allocation<T>
	{
		public:
		
			constexpr system_allocation (T * ptr, std::size_t count) noexcept
				: basic_allocation<T>(ptr, count)
			{}

			system_allocation (const system_allocation & other) = default;
			~system_allocation () = default;
			system_allocation& operator = (const system_allocation & other) = default;
	};

	
	/// System allocator
	///
	/// The system allocator relies on the allocation mechanism provided by the standard library; these are namely
	/// the C-routines malloc and free. No extra aligning or manipulation is performed. It is default constructible.
	/// Ownership will always return true.
	struct system_allocator
	{
		constexpr system_allocator () = default;
		constexpr system_allocator (const system_allocator &) = default;
		constexpr system_allocator (system_allocator &&) = default;
		~system_allocator() = default;
		
		template <typename T> system_alignment allocate (const std::size_t count)
		{
			auto * ptr = static_cast<T*>(::malloc(count * sizeof(T)));
			return system_allocation<T>(ptr, ptr != nullptr ? count : 0);
		}
		
		template <typename T> void deallocate (system_allocation<T> chunk)
		{
			::free(chunk.data());
		}
	};



	template <template T, Allocation A, Allocation B> class fallback_allocation : public basic_allocation<T>
	{
		private:

			union
			{
				A prinaryAllocation;
				B fallbackAllocation;
			};
			bool primary;

		public:

			explicit constexpr fallback_allocation (A allocation) noexcept
			{
				new (&primaryAllocation) A(std::move(allocation));
				primary = true;
			}

			explicit constexpr fallback_allocation (B allocation) noexcept
			{
				new (&fallbackAllocation) B(std::move(allocation));
				primary = false;
			}

			constexpr fallback_allocation (fallback_allocation && allocation) noexcept
			{
				if (allocation.primary)
					new (&primaryAllocation) A(std::move(allocation.primaryAllocation));
				else
					new (&fallbackAllocation) B(std::move(allocation.fallbackAllocation));
				primary = allocation.primary;
			}

			constexpr T* data () const
			{
				return primary ? primaryAllocation.data() : fallbackAllocation.data();
			}

			constexpr std::size_t length () const
			{
				return primary ? primaryAllocation.length() : fallbackAllocation.length();
			}

			void decide (Callable_<A> primaryCall, Callable_<B> fallbackCall)
			{
				if (primary)
					primaryCall(primaryAllocation);
				else
					fallbackCall(fallbackAllocation);
			}
	};


	
	template <Allocator A, Allocator B>
		requires not std::is_convertible<A, B>::value and not std::is_convertible<B, A>::value
	class fallback_allocator
	{
	
		private:

			A primaryAllocator;
			B fallbackAllocator;
	
		public:

			template <typename T> using primary_allocation_type = allocation_type_t<A, T>;
			template <typename T> using fallback_allocation_type = allocation_type_t<B, T>;
			template <typename T> using allocation_type =
				fallback_allocation<T, primary_allocation_type<T>, fallback_allocation_type<T>>;

			constexpr fallback_allocator (A primaryAllocator, B fallbackAllocator) noexcept
				: primaryAllocator(std::move(primaryAllocator)),
				  fallbackAllocator(std::move(fallbackAllocator))
			{}

			fallback_allocator (fallback_allocator && allocator) noexcept
				: primaryAllocator(std::move(allocator.primaryAllocator)),
				  fallbackAllocator(std::move(allocator.fallbackAllocator))
			{}

			template <typename T> allocation_type<T> allocate (const std::size_t count)
			{
				auto primaryAllocation = primaryAllocator.template allocate<T>(count);
				if (primaryAllocation.length() < count)
				{
					primaryAllocator.deallocate(primaryAllocation);
					return allocation_type(fallbackAllocator.template allocate<T>(count));
				}
				return allocation_type(primaryAllocation);
			}

			template <typename T> void deallocate (allocation_type<T> allocation)
			{
				allocation.decide([&](auto allocation)
				{
					primaryAllocator.dealocate(allocation);
				}, [&](auto allocation)
				{
					fallbackAllocator.deallocate(allocation);
				});
			}
	};
	

	/// Allocation on some stack
	template <typename T> class stack_allocation : basic_allocation<T>
	{
		public:

			constexpr stack_allocation (T * ptr, std::size_t count) noexcept
				: stack_allocation(ptr, count)
			{}

			stack_allocation () = default;
			stack_allocation (const stack_allocation & other) = default;
			~stack_allocation () = default;
			stack_allocation& operator = (const stack_allocation & other) = default;
	};




	/// Allocator on local stack with \a N bytes
	///
	/// 
	template <std::size_t N> class stack_allocator
	{
		
		private:

			unsigned char data[N];
			bool allocated;

		public:

			constexpr stack_allocator () noexcept
				: allocated(false)
			{}

			constexpr stack_allocator (stack_allocator && allocator)
				noexcept()
				: 
			{
				
			}
		
			template <typename T> constexpr stack_allocation<T> allocate (const std::size_t count)
			{
				if (count * sizeof(T) <= N and not allocated)
					return stack_allocation<T>(static_cast<T*>(data), count);
				else
					return stack_allocation<T>(nullptr, 0);
			}

			template <typename T> constexpr void deallocate (stack_allocation<T> allocation)
			{
				auto allocationView = allocation.view();
				if (allocationView.data() != nullptr)
				{
					if (allocationView.data() == data and allocationView.length() == N and allocated)
						allocated = false;
					else if (allocationView.data() == data and allocationView.length() == N and not allocated)
						throw bad_alloc("apparent double deallocation");
					else
						throw bad_alloc("apparent deallocation of foreign chunk");
				}
			}
	};



}

#endif
