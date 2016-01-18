/// @file utf8.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_UTF8_HPP__
#define __STDEXT_UTF8_HPP__

#include <stdext/allocator.hpp>
#include <stdext/array.hpp>
#include <stdext/range.hpp>
#include <stdext/utf8_view.hpp>

namespace stdext
{

	/// Text container with utf8 encoding
	///
	/// 
	template <Allocator A>
	class utf8
	{
	
		private:
		
			stdext::array<unsigned char, A> characters;
			std::size_t decodedLength;
			
		public:
		
		
			/// @name Accessors
			/// @{
			
				template <typename V, Callable<V, V, char32_t> C> V fold (C combine, V value) const
				{}
				
				template <typename V, Callable<std::tuple<V, bool>, V, char32_t> C>  V fold (C combine, V value) const
				{}
				
				template <typename V, Callable<V, V, char32_t> C> V fold_reverse (C combine, V value) const
				{}
				
				template <typename V, Callable<std::tuple<V, bool>, V, char32_t> C> V fold_reverse (C combine, V value) const
				{}
			
			/// @}
			
			
		
			/// @name Modifiers
			/// @{
				
				void append (const char32_t character)
				{}
				
				void append (const char32_t * characters)
				{}
				
				void append (IndexedBoundedRange<char32_t> characters)
				{}
				
				void append (const utf8 & characters)
				{}
				
				void prepend (const char32_t character)
				{}
				
				void prepend (const char32_t * characters)
				{}
				
				void prepend (IndexedBoundedRange<char32_t> characters)
				{}
				
				void prepend (const utf8 & characters)
				{}
				
				void insert (const std::size_t pos, const char32_t character)
				{}
				
				void insert (const std::size_t pos, const char32_t * characters)
				{}
				
				void insert (const std::size_t pos, IndexedBoundedRange<char32_t> characters)
				{}
				
				void insert (const std::size_t pos, const utf8 & characters)
				{}
				
				void erase (const std::size_t pos, const std::size_t count)
				{}
				
			/// @}
	
	
			/// @Viewers
			/// @{
			
				constexpr utf8_view view () const
				{
					return utf8_view(characters.view());
				}
				
				constexpr utf8_view view (const std::size_t pos) const
				{}
				
				constexpr utf8_view view (const std::size_t pos, const std::size_t count) const
				{}
			
			/// @}
	
	};

}

#endif
