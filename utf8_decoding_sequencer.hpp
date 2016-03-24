/// @file utf8_decoding_sequencer.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_UTF8_DECODING_SEQUENCER_HPP__
#define __STDEXT_UTF8_DECODING_SEQUENCER_HPP__

#include <stdext/optional.hpp>
#include <stdext/sequence.hpp>
#include <tuple>
#include <stdext/utf8_utilities.hpp>

namespace stdext
{
	


	/// Sequencer for bouned decoding utf8 encoded text
	template <BoundedSequence<unsigned char> S>
	class bounded_utf8_decoding_sequencer
	{
	
		private:

			S decodes;

		public:

			constexpr optional<std::tuple<optional<char32_t>, bounded_utf8_decoding_sequencer>> operator () () const
			{
				return fmap([](auto headDecomposition)
				{
					const auto head = std::get<0>(headDecomposition);
					auto tails = std::move(std::get<1>(headDecomposition));
					const auto parsing = internal::parse_utf8_head_byte(head);
					char32_t encode = std::get<0>(parsing);
					auto count = std::get<1>(parsing);

					if (count == 0)
					{
						return std::make_tuple(optional<char32_t>(encode), bounded_utf8_decoding_sequencer(std::move(tail)));
					}
					else if (std::get<1>(parsing) < 4)
					{
						auto folding = fold([&encode](auto count, auto byte)
						{
							const auto isSyntaxCorrect = byte & 0xc0 == 0x80;
							const auto keepOn = count > 0 and isSyntaxCorrect;
							if (keepOn) encode = (encode << 6) | (byte & 0x3f);
							const auto nextCount = count - (keepOn ? 1 : 0);
							return std::make_tuple(nextCount, keepOn);
						}, count, std::move(tails));

						count = std::get<0>(folding);
						tails = std::move(std::get<1>(folding));

						auto optEncode = count == 0 ? 
						if (count == 0)
						{
							return std::make_tuple(optional<char32_t>(encode), bounded_utf8_decoding_sequencer(std::move(tails)));
						}
						else
						{
							tails = 
							return std::make_tuple(optional<char32_t>(), bounded_utf8_decoding_sequencer());
						}
					}
					else
					{
						auto remainings = internal::skip_utf8_tail_bytes(std::move(std::get<1>(headDecomposition)));
						return std::make_tuple(optional<char32_t>(), std::move(remainings));
					}
				}, decodes());
			}
	
	};



	/// Sequencer for unbounded decoding utf8 decoded text
	///
	/// 
	template <UnboundedSequencer<unsigned char> S>
	class unbounded_utf8_decoding_sequencer
	{
	
		private:

			S encodes;

		public:

			/// Attribute constructor
			///
			/// Constructs the decoding sequencer with a byte sequence \a encodes
			constexpr unbounded_utf8_decoding_sequencer (S encodes)
				: encodes(std::move(encodes))
				noexcept(std::is_nothrow_move_constructible<S>::value)
			{}

			/// Decomposition
			///
			/// The next utf8 character is decoded. If it is an invalid character, an empty optional
			/// container will be returned. The returned sequence will contain the remaining bytes
			/// after the decoded character or - if invalid - with the next heading byte.
			constexpr std::tuple<optional<char32_t>, unbounded_utf8_decoding_sequencer> operator () () const
			{
				auto headDecomposition = encodes();
				auto heading = decode_utf8_head(std::get<0>(headDecomposition));
				if (std::get<1>(heading) != 0)
				{
					
				}
			}
	};

}

#endif

