/// @file utf8_encoder.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_UTF8_ENCODER_HPP__
#define __STDEXT_UTF8_ENCODER_HPP__

#include <stdext/sequence.hpp>

namespace stdext
{


	namespace internal
	{
		
		/// Encodes \a character
		std::tuple<unsigned char[6], std::size_t> encode_utf8 (char32_t character)
		{
			unsigned char encodings[6];
			std::size_t end;
		
			if (character < 0x80)
			{
				encodings[0] = static_cast<unsigned char>(character);
				end = 1;
			}
			else if (character < 0x800)
			{
				encodings[0] = 0xc0 | static_cast<unsigned char>(character & 0x1f);
				encodings[1] = 0x80 | static_cast<unsigned char>(character & 0x3f);
				end = 2;
			}
			else if (character < 0x10000)
			{
				encodings[0] = 0xe0 | static_cast<unsigned char>(character & 0xf000);
				encodings[1] = 0x80 | static_cast<unsigned char>(character & 0xfc0);
				encodings[2] = 0x80 | static_cast<unsigned char>(character & 0x3f);
				end = 3;
			}
			else if (character <= 0x10ffff)
			{
				encodings[0] = 0xf0 | static_cast<unsigned char>(character & 0x1c0000);
				encodings[1] = 0x80 | static_cast<unsigned char>(character & 0x3f00);
				encodings[2] = 0x80 | static_cast<unsigned char>(character & 0xfc0);
				encodings[3] = 0x80 | static_cast<unsigned char>(character & 0x3f);
				end = 4;
			}
			else if (character <= 0x4000000)
			{
				encodings[0] = 0xf8 | static_cast<unsigned char>(character & 0x3000000);
				encodings[1] = 0x80 | static_cast<unsigned char>(character & 0xfc0000);
				encodings[2] = 0x80 | static_cast<unsigned char>(character & 0x3f00);
				encodings[3] = 0x80 | static_cast<unsigned char>(character & 0xfc0);
				encodings[4] = 0x80 | static_cast<unsigned char>(character & 0x3f);
				end = 5;
			}
			else
			{
				encodings[0] = 0xfc | static_cast<unsigned char>(character & 0x40000000);
				encodings[1] = 0x80 | static_cast<unsigned char>(character & 0x3f000000);
				encodings[2] = 0x80 | static_cast<unsigned char>(character & 0xfc0000);
				encodings[3] = 0x80 | static_cast<unsigned char>(character & 0x3f00);
				encodings[4] = 0x80 | static_cast<unsigned char>(character & 0xfc0);
				encodings[5] = 0x80 | static_cast<unsigned char>(character & 0x3f);
				end = 6;
			}
			return std::make_tuple(encodings, end);
		}

	}

	/// Encoding Sequencer for utf8 formating
	///
	/// The encoder allows to transform a sequence \a S of unicode characters into a sequence of
	/// bytes which are a multibyte utf8 formating of the unicode characters. Logically, it is a
	/// flattening of a one-to-many mapping of unicode characters.
	template <BoundedSequence<char32_t> S> class utf8_encoder
	{
	
		private:

			S source;
			unsigned char encodings[6];
			std::size_t index = 0;
			std::size_t end = 0;

			/// Attribute constructor
			///
			/// The attribute constructor sets all attributes. 
			utf8_encoder(S source, unsigned char encodings[6], std::size_t index, std::size_t end)
				: source(std::move(source)), encodings(encodings), index(index), end(end)
				noexcept(std::is_nothrow_move_constructible<S>::value)
			{}



		public:

			/// Sequence constructor
			///
			/// The sequence will be set to \a source.
			constexpr utf8_encoder (S source)
				: source(std::move(source))
				noexcept(std::is_nothrow_move_constructible<S>::value)
			{}

			/// Copy constructor
			///
			/// The state of \a other will be copied, so both instances will behave the same way.
			constexpr utf8_encoder (const utf8_encoder & other) = default;

			/// Destructor
			constexpr ~utf8_encoder () = default;

			/// Copy assignment
			///
			/// The state of \a other will be copied, so both instances will behave the same way.
			utf8_encoder& operator = (const utf8_encoder& other) = default;
	
			/// Swapping
			///
			/// The states of \a first and \a second are swapped, so both will behave the the other way.
			friend void swap (utf8_encoder & first, utf8_encoder & second)
			{
				using std::swap;
				swap(first.source, second.source);
				swap(first.currentEncodings, second.currentEncodings);
				swap(first.index, second.index);
				swap(first.end, second.end);
			}

			/// Decomposition
			///
			/// The sequence is decomposed into the next byte and a sequence with the remaining bytes. If
			/// the current sequence is empty, the returning optional container will be empty as well.
			optional<std::tuple<unsigned char, utf8_encoder>> operator () const
			{
				if (index < end)
				{
					auto nextState = utf8_encoder(source, encodings, index + 1, end);
					return make_optional(std::make_tuple(encodings[index], nextState));
				}
				else
				{
					return fmap([](auto value)
					{
						auto nextState = utf8_encoder(std::move(std::get<1>(value)));
						std::tie(nextState.encodings, nextState.end) = internal::encode_utf8(std::get<0>(value));
						nextState.index = 1;
						return std::make_tuple(nextState.encodings[0], std::move(nextState));
					}, source());
				}
			}

			/// Folding all values
			///
			/// All values in \a sequence are folded by \a combiner starting at \a value. The returning
			/// value is folded over all values in \a sequence.
			template <typename V> friend V fold (Callable<V, V, unsigned char> combiner, V value, utf8_encoder sequence)
			{
				for (std::size_t i = sequence.index; i < sequence.end; ++i)
					value = combiner(std::move(value), sequence.encodings[i]);
				return fold([combiner=std::move(combiner)](auto value, auto character)
				{
					unsigned char encodings[4];
					std::size_t count;
					std::tie(encodings, count) = internal::encode_utf8(character);
					for (std::size_t i = 0; i < count; ++i)
						value = combiner(std::move(value), encodings[i]);
					return value;
				}, std::move(value), std::move(sequence.source));
			}

			/// Folding a prefix of \a values
			///
			/// The starting \a value is folded over all initial values in \a sequence for which \a
			/// combiner returns a true flag. The returning value will be folded over all these values.
			/// The remaining sequence will be returned additionally.
			template <typename V> friend std::tuple<V, utf8_encoder>
			fold (Callable<std::tuple<bool, V>, V, unsigned char> combiner, V value, utf8_encoder sequence)
			{
				auto keepOn = true;
				while (keepOn and sequence.index < sequence.end)
				{
					std::tie(keepOn, value) = combiner(std::move(value), sequence.encodings[sequence.index]);
					++sequence.index;
				}
				if (keepOn)
				{
					auto result = std::tie(std::tie(sequence.encodings, sequence.index, sequence.end), sequence.source);
					result = fold([combiner=std::move(combiner)](auto value, auto character)
					{
						unsigned char encodings[4];
						std::size_t count;
						std::size_t index = 0;
						bool keepOn = true;
						std::tie(encodings, count) = internal::encode_utf8(character);
						while (keepOn and index < count)
						{
							std::tie(keepOn, value) = combiner(std::move(value), encodings[index]);
							++index;
						}
						return std::make_tuple(std::make_tuple(encodings, index, count), keepOn);
					}, std::make_tuple(unsigned char[4]{}, std::size_t(0), std::size_t(0)), std::move(sequence.source));
				}
				return std::make_tuple(value, sequence);
			}

			/// Returns the length in bytes
			friend std::size_t length (utf8_encoder sequence)
			{
				const auto sourceLength = fold([](auto counter, auto character)
				{
					const auto encodingLength = std::get<1>(internal::encode_utf8(character));
					return encodingLength + counter;
				}, std::size_t(0), std::move(sequence.source));
				return sequence.end - sequence.index + sourceLength;
			}
	};


	/// Encoding Sequencer for utf8 formating
	///
	/// The encoder allows to transform a sequence \a S of unicode characters into a sequence of
	/// bytes which are a multibyte utf8 formating of the unicode characters. Logically, it is a
	/// flattening of a one-to-many mapping of unicode characters.
	template <UnboundedSequence<char32_t> S> class utf8_encoder
	{
	
		private:

			S source;
			unsigned char encodings[6];
			std::size_t index;
			std::size_t end;

		public:
	
			/// Sequence constructor
			///
			/// The sequence will be set to \a source.
			constexpr utf8_encoder (S source)
				: source(std::move(source))
				noexcept(std::is_nothrow_move_constructible<S>::value)
			{}

			/// Copy constructor
			///
			/// The state of \a other will be copied, so both instances will behave the same way.
			constexpr utf8_encoder (const utf8_encoder & other) = default;

			/// Destructor
			constexpr ~utf8_encoder () = default;

			/// Copy assignment
			///
			/// The state of \a other will be copied, so both instances will behave the same way.
			utf8_encoder& operator = (const utf8_encoder& other) = default;
	
			/// Swapping
			///
			/// The states of \a first and \a second are swapped, so both will behave the the other way.
			friend void swap (utf8_encoder & first, utf8_encoder & second)
			{
				using std::swap;
				swap(first.source, second.source);
				swap(first.currentEncodings, second.currentEncodings);
				swap(first.index, second.index);
				swap(first.end, second.end);
			}

			/// Decomposition
			///
			/// The sequence is decomposed into the next byte and a sequence with the remaining bytes.
			std::tuple<unsigned char, utf8_encoder> operator () const
			{
				if (index < end)
				{
					auto nextState = utf8_encoder(source, encodings, index + 1, end);
					return make_optional(std::make_tuple(encodings[index], nextState));
				}
				else
				{
					auto decomposition = source();
					auto nextState = utf8_encoder(std::move(std::get<1>(decomposition)));
					std::tie(nextState.encodings, nextState.end) = internal::encode_utf8(std::get<0>(decomposition));
					nextState.index = 1;
					return std::make_tuple(nextState.encodings[0], std::move(nextState));
				}
			}

			/// Folding a prefix of \a values
			///
			/// The starting \a value is folded over all initial values in \a sequence for which \a
			/// combiner returns a true flag. The returning value will be folded over all these values.
			/// The remaining sequence will be returned additionally.
			template <typename V> friend std::tuple<V, utf8_encoder>
			fold (Callable<std::tuple<V, bool>, V, unsigned char> combiner, V value, utf8_encoder sequence)
			{
				auto keepOn = true;
				while (keepOn and sequence.index < sequence.end)
				{
					std::tie(keepOn, value) = combiner(std::move(value), sequence.encodings[sequence.index]);
					++sequence.index;
				}
				if (keepOn)
				{
					auto result = std::tie(std::tie(sequence.encodings, sequence.index, sequence.end), sequence.source);
					result = fold([combiner=std::move(combiner)](auto value, auto character)
					{
						unsigned char encodings[4];
						std::size_t count;
						std::size_t index = 0;
						bool keepOn = true;
						std::tie(encodings, count) = internal::encode_utf8(character);
						while (keepOn and index < count)
						{
							std::tie(keepOn, value) = combiner(std::move(value), encodings[index]);
							++index;
						}
						return std::make_tuple(std::make_tuple(encodings, index, count), keepOn);
					}, std::make_tuple(unsigned char[6]{}, std::size_t(0), std::size_t(0)), std::move(sequence.source));
				}
				return std::make_tuple(value, sequence);
			}
	};



	/// Creates a sequence for \a sequence which will return utf8 formated bytes
	template <BoundedSequence<char32_t> S> utf8_encoder<S> encode_utf8 (S sequence)
	{
		return utf8_encoder<S>(std::move(sequence));
	}

	/// Creates a sequence for \a sequence which will return utf8 formated bytes
	template <UnboundeSequence<char32_t> S> utf8_encoder<S> encode_utf8 (S sequence)
	{
		return utf8_encoder<S>(std::move(sequence));
	}

}

#endif

