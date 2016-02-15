/// @file utf8.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_UTF8_HPP__
#define __STDEXT_UTF8_HPP__

#include <stdext/allocator.hpp>
#include <stdext/array.hpp>
#include <stdext/sequence.hpp>
#include <stdext/utf8_view.hpp>
#include <stdext/utf8_encoder.hpp>
#include <stdext/utf8_decoder.hpp>

namespace stdext
{

	/// Text container with utf8 encoding
	///
	/// The utf8 container holds an array of bytes which encodes a text into utf8 formating. The
	/// bytes are hold by an array whose allocator will be of type \a A. 
	template <Allocator A>
	class utf8
	{
	
		private:
	
			stdext::array<unsigned char, A> characters;
			std::size_t decodedLength = 0;


		public:
		
			/// Default constructor
			///
			/// The default constructor initialises an empty container. That is no memory will be
			/// allocated and no value will be set. The allocator will be constructed by default.
			constexpr utf8 () = default;

			/// Allocator constructor
			///
			/// The \a allocator will be moved in as the allocator for the byte sequence. No memory will
			/// be allocated and the container will be empty.
			explicit constexpr utf8 (A allocator)
				: characters(std::move(allocator))
				noexcept(std::is_nothrow_move_constructible<A>::value)
			{}

			/// Reservation constructor
			///
			/// The allocator will be default constructed. Memory for at least \a count bytes will be
			/// reserved. No character will be set, so the container will be empty.
			explicit constexpr utf8 (std::size_t count)
				: characters(count)
			{}

			/// Reservation constructor with allocator
			///
			/// The allocator will be moved in as the allocator for the byte sequence. Memory for at
			/// least \a count bytes will be reserved. No character will be set, so the container will be
			/// empty.
			constexpr utf8 (std::size_t count, A allocator)
				: characters(count, std::move(allocator))
			{}

			/// Copy constructor
			///
			/// The copy constructor copies the text from \a other, as well the allocator. Memory for the
			/// copied text is requested from the copied allocator.
			constexpr utf8 (const utf8 & other) = default;

			/// Copy constructor with allocator
			///
			/// The copy constructor copies the text from \a other. The \a allocator is used for the
			/// newly constructed array.
			template <Allocator B> constexpr utf8 (const utf8<B> & other, A allocator)
				: characters{other.characters, std::move(allocator)}, decodedLength(other.decodedLength)
			{}

			/// Copy constructor with default constructed allocator
			///
			/// The copy constructor copies the text from \a other. The allocator for the newly
			/// constructed array will be constructed by default.
			template <Allocator B> explicit constexpr utf8 (const utf8<B> & other)
				: characters(other.characters), decodedLength(other.decodedLength)
			{}

			/// Move constructor
			///
			/// The text from \a other will be moved to the constructed object, as well the allocator and
			/// any allocated memory. The \a other object will be left empty.
			constexpr utf8 (utf8 && other) = default;

			/// Move constructor with allocator
			constexpr utf8 (utf8 && other, A allocator)
				: utf8(other, std::move(allocator)
			{}




	
			/// Returns the length of the contained text in characters
			constexpr std::size_t length () const
			{
				return decodedLength;
			}

			/// Returns the length of the contained text in bytes
			constexpr std::size_t raw_length () const
			{
				return characters.length();
			}

			/// Returns the raw capacity, that is how many bytes can be stored before any reallocation
			constexpr std::size_t raw_capacity () const
			{
				return characters.capacity();
			}


			

			/// Appending
			///
			/// Unicode characters are encoded into utf8 and these multi-bytes are appended to the end of
			/// the internal byte sequence. If any exception is raised, the precalling state will be
			/// restored.
			///
			/// @param character  A single unicode character
			/// @param text       A sequence of unicode character or utf8 formated text
			///
			/// @{
			void append (const char32_t character)
			{
				append(array_view<const char32_t>(&character, 1));
			}

			void append (const char32_t * text)
			{
				auto count = 0;
				while (text != nullptr and text[count] != 0)
					++count;
				append(array_view<const char32_t>(text, count));
			}

			void append (BoundedSequence<char32_t> text)
			{				
				characters.append(encode_utf8(std::move(text)));
			}

			template <Allocator B> void append (const utf8<B> & text)
			{
				append(text.view());
			}

			template <Allocator B> void append (utf8<B> && text)
			{
				append(text.view());
			}

			void append (utf8_view text)
			{
				characters.append(text.raw_view());
				decodedLength += text.length();
			}
			/// @}



			/// Prepending
			///
			/// Unicode characters are encoded into utf8 and these multi-bytes are prepended to the end
			/// of the internal byte sequence. If any exception is raised, the precalling state will be
			/// restored.
			///
			/// @param character  A single unicode character
			/// @param text       A sequence of unicode character or utf8 formated text
			///
			/// @{
			void prepend (const char32_t character)
			{
				prepend(array_view<const char32_t>(&characters, 1));
			}
				
			void prepend (const char32_t * characters)
			{
				std::size_t count = 0;
				while (characters != nullptr and characters[count] != 0)
					++count;
				prepend(array_view<const char32_t>(characters, count));
			}
				
			void prepend (BoundedRange<char32_t> characters)
			{
				characters.prepend(encode_utf8(std::move(characters)));
			}
				
			template <Allocator B> void prepend (const utf8<B> & text)
			{
				prepend(text.view());
			}

			template <Allocator B> void prepend (utf8<B> && text)
			{
				prepend(text.view());
			}

			void prepend (utf8_view text)
			{
				characters.prepend(text.raw_view());
				decodedLength += text.length();
			}
			/// @}	
				
	};

}

#endif
