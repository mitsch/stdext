/// @file utf8_encoding_sequencer.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_UTF8_ENCODING_SEQUENCER_HPP__
#define __STDEXT_UTF8_ENCODING_SEQUENCER_HPP__

#include <stdext/sequence.hpp>
#include <stdext/utf8_utilities.hpp>

namespace stdext
{


	template <BoundedSequence<char32_t> S>
	class utf8_encoding_sequencer
	{
	
		private:

			S decodes;
			unsigned char encodes[4];
			unsigned int next;

		public:

			constexpr optional<std::tuple<unsigned char, utf8_encoding_sequencer>> operator () () const
			{
				if (next < 4)
				{
					return make_optional(std::make_tuple(encodes[next], utf8_encoding_sequencer(decodes, encodes, next + 1)));
				}
				else
				{
					return fmap([](auto decomposition)
					{
						const auto encoding = encode_to_utf8(std::get<0>(decomposition));
						const auto encodes = std::get<0>(encoding);
						const auto next = std::get<1>(encoding);
						return std::make_tuple(encodes[next], utf8_encoding_sequencer(decodes, encodes, next + 1));
					}, encodes());
				}
			}

			template <typename V, Callable<V, V, unsigned char> C>
			friend constexpr V fold (C combiner, V value, utf8_encoding_sequencer elements)
			{
				switch (elements.next)
				{
					case 1: value = combiner(std::move(value), elements.encodes[1]);
					case 2: value = combiner(std::move(value), elements.encodes[2]);
					case 3: value = combiner(std::move(value), elements.encodes[3]);
				}

				return fold([&combiner](V value, char32_t decode)
				{
					auto encoding = encode_utf8(decode);
					switch (std::get<1>(encoding))
					{
						case 0: value = combiner(std::move(value), std::get<0>(encoding)[0]);
						case 1: value = combiner(std::move(value), std::get<0>(encoding)[1]);
						case 2: value = combiner(std::move(value), std::get<0>(encoding)[2]);
						case 3: value = combiner(std::move(value), std::get<0>(encoding)[3]);
					}
				}, std::move(value), std::move(elements.decodes));
			}

			template <typename V, Callable<std:tuple<V, bool>, V, unsigned char> C>
			friend constexpr std::tuple<V, utf8_encoding_sequencer> fold (C combiner, V value, utf8_encoding_sequencer elements)
			{
				bool keepOn = true;
				switch (elements.next)
				{
					case 1:
						std::tie(value, keepOn) = combiner(std::move(value), elements.encodes[1]);
						if (not keepOn) elements.next = 1, break;
					case 2:
						std::tie(value, keepOn) = combiner(std::move(value), elements.encodes[2]);
						if (not keepOn) elements.next = 2, break;
					case 3:
						std::tie(value, keepOn) = combiner(std::move(value), elements.encodes[3]);
						if (not keepOn) elements.next = 3, break;
				}

				if (not keepOn)
					return std::make_tuple(std::move(value), std::move(elements));

				unsigned int lastNext = 4;
				auto folding = fold([&combiner, &lastNext](V value, char32_t decode)
				{
					auto keepOn = true;
					auto encoding = encode_utf8(decode);
					switch (std::get<1>(encoding))
					{
						case 0:
							std::tie(value, keepOn) = combiner(std::move(value), std::get<0>(encoding)[0]);
							if (not keepOn) lastNext = 
					}
				}, std::move(value), std::move(elements.decodes));
			}

	};

}

#endif

