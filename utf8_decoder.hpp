/// @file utf8_decoder.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_UTF8_DECODER_HPP__
#define __STDEXT_UTF8_DECODER_HPP__

namespace stdext
{

	namespace internal
	{

		/// 
		std::tuple<char32_t, unsigned int> decode_head_utf8 (unsigned char byte)
		{
			if ((byte & 0x80) == 0x00)
				return std::make_tuple(static_cast<char32_t>(byte), 0);
			else if ((byte & 0xe0) == 0xc0)
				return std::make_tuple(static_cast<char32_t>(byte & 0x1f), 1);
			else if ((byte & 0xf0) == 0xe0)
				return std::make_tuple(static_cast<char32_t>(byte & 0x0f), 2);
			else if ((byte & 0xf8) == 0xf0)
				return std::make_tuple(static_cast<char32_t>(byte & 0x07), 3);
			else if ((byte & 0xfc) == 0xf8)
				return std::make_tuple(static_cast<char32_t>(byte & 0x03), 4);
			else
				return std::make_tuple(static_cast<char32_t>(byte & 0x01), 5);
		}
	
	}

	/// Bounded decoding sequencer for utf8 formating
	///
	/// The decoder allows to decode utf8 formated text. It expects a bounded sequence of byte values
	/// and delivers a sequence of unicode characters.
	template <BoundedSequence<unsigned char> S> class utf8_decoder
	{
	
		private:

			S source;

		public:

			/// Sequence constructor
			///
			/// The sequence \a source is set as byte sequence.
			constexpr utf8_decoder (S source)
				: source(std::move(source))
				noexcept(std::is_nothrow_move_constructible<S>::value)
			{}

			/// Copy constructor
			///
			/// The state of \a other is copied such that both instances will show the same behaviour
			/// afterwards.
			constexpr utf8_decoder (const utf8_decoder & other) = default;

			/// Destructor
			~utf8_decoder () = default;

			/// Copy assignment
			///
			/// The state of \a other is copied such that both instances will show the same behaviour
			/// afterwards.
			constexpr utf8_decoder& operator = (const utf8_decoder & other) = default;

			/// Decomposition
			///
			/// The sequence is decomposed into the next decoded unicode character and a sequence of the
			/// remaining encoded characters.
			constexpr optional<std::tuple<char32_t, utf8_decoder>> operator () () const
			{
				return sequence([](auto decomposition)
				{
					char32_t character;
					unsigned int count;
					std::tie(character, count) = internal::decode_head_utf8(std::get<0>(decomposition);
					auto initState = make_optional(std::make_tuple(character, count, std::move(std::get<1>(decomposition))))
					auto parsings = sequence_while([](auto state)
					{
						return std::get<1>(state) > 0;
					}, [](auto state)
					{
						auto character = std::get<0>(state);
						auto count = std::get<1>(state);
						auto remainings = std::move(std::get<2>(state));
						return sequence([](auto decomposition, auto character, auto count)
						{
							const auto newCharacter = (character << 6) | (std::get<0>(decomposition) & 0x3f);
							return std::make_tuple(newCharacter, count-1, std::move(std::get<2>(decomposition)));
						}, remainings(), character, count);
					}, std::move(initState));
					return fmap([](auto state)
					{
						return std::make_tuple(std::get<0>(state), std::move(std::get<2>(state)));
					}, std::move(parsings));
				}, source());
			}

			/// Folding all characters
			///
			/// All characters in \a text are folded by \a combiner starting at \a value. The returning
			/// value is folded over all characters in \a text.
			template <typename V> friend constexpr V fold (Callable<V, V, char32_t> combiner, V value, utf8_decoder text)
			{
				return std::get<2>(fold([combiner=std::move(combiner)](auto state, auto byte)
				{
					char32_t character = std::get<0>(state);
					unsigned int count = std::get<1>(state);
					V value = std::move(std::get<2>(state));

					if (count == 0)
						std::tie(character, count) = internal::decode_head_utf8(byte);
					else
					{
						character = (character << 6) | (byte & 0x3f);
						count--;
					}
					if (count == 0)
						value = combiner(std::move(value), character);

					return std::make_tuple(character, count, std::move(value));
				}, std::make_tuple(char32_t{}, unsigned int(0), std::move(value)), std::move(text.source)));
			}
	
			/// Folding initial characters
			///
			/// All initial characters in \a text for which \a combiner returns a true flag, are folded
			/// by \a combiner starting at \a value. The returning value will be folded over all these
			/// initial characters. A sequence with the remaining characters will be returned as well.
			template <typename V> friend constexpr std::tuple<V, utf8_decoder> fold (
				Callable<std::tuple<V, bool>, V, char32_t> combiner, V value, utf8_decoder text)
			{
				auto result = fold([](auto state, auto byte)
				{
					char32_t character = std::get<0>(state);
					unsigned int count = std::get<1>(state);
					V value = std::move(std::get<2>(state));
					bool keepOn = true;

					if (count == 0)
						std::tie(character, count) = internal::decode_head_utf8(byte);
					else
					{
						character = (character << 6) | (byte & 0x3f);
						count--;
					}
					if (count == 0)
						std::tie(value, keepOn) = combiner(std::move(value), character);

					return std::make_tuple(std::make_tuple(character, count, std::move(value)), keepOn);
				}, std::make_tuple(char32_t{}, unsigned int(0), std::move(value)), std::move(text.source));
				return std::make_tuple(std::move(std::get<2>(std::get<0>(result))), std::move(std::get<1>(result)));
			}

			/// Returns the length of characters
			friend constexpr std::size_t length (utf8_decoder text)
			{
				return std::get<0>(fold([](auto state, auto byte)
				{
					auto amount = std::get<0>(state);
					auto count = std::get<1>(state);

					if (count == 0)
					{
						amount++;
						if (byte & 0xc0 == 0x80)
							count = 1;
						else if (byte & 0xe0 == 0xc0)
							count = 2;
						else if (byte & 0xf0 == 0xe0)
							count = 3;
						else if (byte & 0xf8 == 0xf0)
							count = 4;
						else if (byte & 0xfc == 0xf8)
							count = 5;
					}
					else
					{
						count--;
					}

					return std::make_tuple(amount, count);
				}, std::make_tuple(std::size_t(0), unsigned int(0)), std::move(text.source)));
			}

	};



	/// Unbounded decoding sequencer for utf8 formating
	///
	/// The decoder allows to decode utf8 formated text. It expects an unbounded sequence of byte
	/// values and delivers a sequence of unicode characters.
	template <UnboundedSequence<unsigned char> S> class utf8_decoder
	{
	
		private:

			S source;

		public:

			/// Sequence constructor
			///
			/// The sequence \a source is taken as source to read bytes.
			explicit constexpr utf8_decoder (S source)
				: source(std::move(source))
				noexcept(std::is_nothrow_move_constructible<S>::value)
			{}

			/// Copy constructor
			///
			/// The state of \a other is copied such that the behaviour of both instances will be the
			/// same afterwards.
			constexpr utf8_decoder (const utf8_decoder & other) = default;
	
			/// Destructor
			~utf8_decoder () = default;

			/// Copy assignment
			///
			/// The state of \a other is copied such that the behaviour of both instance will be
			/// identical.
			constexpr utf8_decoder& operator = (const utf8_decoder & other) = default;

			/// Swapping
			///
			/// The states of \a first and \a second are replaced by each other such that both instances
			/// show the behaviour of the other afterwards.
			friend constexpr void swap (utf8_decoder & first, utf8_decoder & second)
			{
				using std::swap;
				swap(first.source, second.source);
			}

			/// Decomposition
			///
			/// The sequence is decomposed into the next unicode character and a sequence with the
			/// remaining values of the source sequence.
			constexpr std::tuple<char32_t, utf8_decoder> operator () () const
			{
				char32_t character;
				unsigned int count;
				auto decomposition = source();
				std::tie(character, count) = internal::decode_head_utf8(std::get<0>(decomposition));
				for (unsigned int i = 0; i < count; ++i)
				{
					decomposition = std::get<1>(decomposition)();
					character = (character << 6) | (std::get<0>(decomposition) & 0x3f);
				}
				return std::make_tuple(character, std::move(std::get<1>(decomposition)));
			}

			/// Folding
			///
			/// All initial values in \a text for which \a combiner returns a true flag, are folded over
			/// \a value by \a combiner. The returning tuple has the resulting value and a sequence with
			/// the remaining values.
			template <typename V, Callable<std::tuple<V, bool>, V, char32_t> C>
			friend constexpr std::tuple<V, utf8_decoder> fold (C combiner, V value, utf8_decoder text)
			{
				auto result = std::tie(std::tie(std::ignore, std::ignore, value), text);
				result = fold([combiner=std::move(combiner)](auto state, auto byte)
				{
					auto count = std::get<0>(state);
					auto character = std::get<1>(state);
					auto value = std::move(std::get<2>(state));
					auto keepOn = true;

					if (count == 0)
					{
						std::tie(character, count) = internal::decode_head_utf8(byte);
					}
					else
					{
						character = (character << 6) | (byte & 0x3f);
						count--;
					}

					if (count == 0)
						std::tie(value, keepOn) = combiner(std::move(value), character);

					return std::make_tuple(std::make_tuple(count, character, std::move(value)), keepOn);
				}, std::make_tuple(unsigned int(0), char32_t(0), std::move(value)), std::move(text.source));
				return std::make_tuple(std::move(value), std::move(text));
			}
	};

	/// Makes a sequencer for decoding a byte sequence into a Unicode sequence
	template <UnboundedSequence<unsigned char> S> utf8_decoder<S> decode_utf8 (S sequence)
	{
		return utf8_decoder<S>(std::move(sequence));
	}

}

#endif

