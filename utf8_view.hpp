/// @file utf8_view.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_UTF8_VIEW_HPP__
#define __STDEXT_UTF8_VIEW_HPP__

namespace stdext
{

	/// View on an utf8 encoded text
	///
	/// An object of this class holds at any time a valid utf8 encoding. Contrary to a general
	/// sequence, the view allows for splitting, shrinkage and matching. The encoding sequence in the
	/// view is inmutable.
	class utf8_view
	{
	
	private:

		/// The underlying encoding byte sequence
		array_view<const unsigned char> encoding;

		/// Forward character decoding
		static constexpr std::tuple<char32_t, std::size_t> decode (const unsigned char * start);

		/// Backward character decoding
		static constexpr std::tuple<char32_t, std::size_t> decode_reverse (const unsigned char * start, std::size_t length);

	public:

		explicit constexpr utf8_view (array_view<const unsigned char> encoding)
			: encoding(encoding)
			noexcept
		{}

		constexpr bool empty () const
		{
			return encoding.empty();
		}

		constexpr size_t length () const
		{
			return encoding.fold([](auto count, auto character)
			{
				return count + (character & 0b11000000 == 0b10000000) ? 0 : 1;
			}, size_t(0));
		}

		constexpr size_t raw_length () const
		{
			return encoding.length();
		}

		constexpr const unsigned char * raw_data () const
		{
			return encoding.data();
		}

		constexpr array_view<const unsigned char> raw_view () const
		{
			return encoding;
		}

		constexpr optional<std::tuple<char32_t, utf8_view>> decompose_prefix () const
		{
			return make_optional(encoding.length() > 0, [&encoding]()
			{
				const auto decoding = decode(encoding.data());
				const auto remainingLength = encoding.length() - std::get<1>(decoding);
				const auto offsetData = encoding.data() + std::get<1>(decoding);
				const auto remainings = utf8_view(array_view<const unsigned char>(offsetData, remainingLength));
				return std::make_tuple(std::ge<0>(decoding), remainings);
			});
		}
		
		constexpr optional<std::tuple<char32_t, utf8_view>> decompose_suffix () const
		{
			return make_optional(encoding.length() > 0, [&]()
			{
				const auto decoding = decode_reverse(encoding.data(), encoding.length());
				const auto remainingLength = encoding.length() - std::get<1>(decoding);
				const auto remainings = utf8_view(array_view<const unsigned char>(encoding.data(), remainingLength));
				return std::make_tuple(std::get<0>(decoding), remainings);
			});
		}
		
		template <typename V, Callable<V, V, char32_t> C>
		constexpr V fold (C combiner, V value) const
		{
			std::size_t index = 0;
			auto remainingLength = encoding.length();
			while (remainingLength > 0)
			{
				const auto decoding = decode(encoding.data() + index);
				value = combiner(std::move(value), std::get<0>(decoding));
				index += std::get<1>(decoding);
				remainingLength -= std::get<1>(decoding);
			}
			return value;
		}

		template <typename V, Callable<V, V, char32_t> C>
		constexpr V fold_reverse (C combiner, V value) const
		{
			auto remainingLength = encoding.length();
			while (remainingLength > 0)
			{
				const auto decoding = decode_reverse(encoding.data(), remainingLength);
				value = combiner(std::move(value), std::get<0>(decoding));
				remainingLength -= std::get<1>(decoding);
			}
			return value;
		}

		template <typename V, Callable<std::tuple<V, bool>, V, char32_t> C>
		constexpr std::tuple<V, utf8_view, utf8_view> fold (C combiner, V value) const
		{
			auto keepOn = true;
			std::size_t index = 0;
			auto remainingLength = encoding.length();
			while (remainingLength > 0 and keepOn)
			{
				const auto decoding = decode(encoding.data() + index);
				std::tie(value, keepOn) = combiner(std::move(value), std::get<0>(decoding));
				if (keepOn)
				{
					index += std::get<1>(decoding);
					remainingLength -= std::get<1>(decoding);
				}
			}
			const auto unfolded = utf8_view(encoding.data() + index, remainingLength);
			const auto folded = utf8_view(encoding.data(), index);
			return std::make_tuple(std::move(value), unfolded, folded);
		}

		template <typename V, Callable<std::tuple<V, bool>, V, char32_t> C>
		constexpr std::tuple<V, utf8_view, utf8_view> fold_reverse (C combiner, V value) const
		{
			auto keepOn = true;
			auto remainingLength = encoding.length();
			while (remainingLength > 0 and keepOn)
			{
				const auto decoding = decode_reverse(encoding.data(), remainingLength);
				std::tie(value, keepOn) = combiner(std::move(value), std::get<0>(decoding));
				if (keepOn)
				{
					remainingLength -= std::get<1>(decoding);
				}
			}
			const auto unfolded = utf8_view(encoding.data(), remainingLength);
			const auto folded = utf8_view(encoding.data() + remainingLength, encoding.length() - remainingLength);
			return std::make_tuple(std::move(value), unfolded, folded);
		}
			



		/// Prefix splitting with amount of characters
		///
		/// The view will be splitted into two partitions. The first partition will hold at most \a
		/// count characters. The second partition will hold all succeeding characters. If the view
		/// holds less than \a count characters, the first partition will be equivalent to the
		/// original view and the second partition will be empty. Along with both partition, the
		/// amount of characters held in the first partition will be returned.
		///
		/// @note Time complexity is linear to the length of the view.
		///
		constexpr std::tuple<utf8_view, utf8_view, size_t> split_prefix (size_t count) const
		{
			const auto folding = encoding.fold([](const auto count, const auto character)
			{
				const auto isHead = (character & 0b11000000) != 0b10000000;
				const auto keepOn = count > 0 or not isHead;
				const auto newCount = isHead and count > 0 ? count - 1 : count;
				return std::make_tuple(newCount, keepOn);
			}, count);
			const auto pre = utf8_view(std::get<2>(folding));
			const auto post = utf8_view(std::get<1>(folding));
			return std::make_tuple(pre, post, count - std::get<0>(folding));
		}

		/// Prefix splitting with predictor
		///
		/// The view will be splitted into two partitions. The first partition will hold the longest
		/// prefix of characters for which \a predictor returns true. The second partition will hold
		/// all succeeding characters. Characters are passed on starting with the first and
		/// traversing through the last one in the view.
		///
		/// @note Time complexity is linear to the length of the view.
		///
		template <Callable<bool, char32_t> C>
		constexpr std::tuple<utf8_view, utf8_view, size_t> split_prefix (C predictor) const
		{
			const auto folding = encoding.fold([predictor=std::move(predictor)](const auto count, const auto character)
			{
				const auto prediction = predictor(character);
				const auto newCount = count + prediction ? 1 : 0;
				return std::make_tuple(newCount, prediction);
			}, size_t(0));
			const auto pre = utf8_view(std::get<2>(folding));
			const auto post = utf8_view(std::get<1>(folding));
			return std::make_tuple(pre, post, std::get<0>(folding));
		}

		/// Prefix splitting with predictor and variable
		///
		/// The view will be splitted into two partitions. The first partition will hold the longest
		/// prefix of characters for which \a predictor returns true. The second partition will hold
		/// all succeeding characters. Characters are passed on starting with the first and
		/// traversing through the last one in the view.
		///
		/// @note Time complexity is linear to the length of the view.
		///
		template <typename V, Callable<std::tuple<V, bool>, V, char32_t> C>
		constexpr std::tuple<utf8_view, utf8_view, V> split_prefix (C predictor, V variable) const;

		/// Prefix splitting with unicode sequence
		///
		/// The view will be splitted into two partitions. The first partition will hold the longest
		/// prefix of characters which \a other shares with this view. The second partition will hold
		/// all the succeeding characters in the view. Both partitions will be returned along with a
		/// sequence holding all remaining elements of \a other.
		///
		/// @note Time complexity is linear to the length of the view.
		///
		template <Sequence<char32_t> S>
		constexpr std::tuple<utf8_view, utf8_view, S> split_prefix (S other) const;





			/// Suffix splitting with amount of characters
			///
			/// The view will be splitted into two partitions. The second partition will hold at most the
			/// last \a count characters. The first partition will hold all preceeding characters. If the
			/// view contains less than \a count characters, then the first partition will be empty and
			/// the second partition will be equivalent to the view. Along with both partitions, the
			/// amount of characters held in the second partition will be returned.
			///
			/// @note Time complexity is linear to the length of the view.
			///
			constexpr std::tuple<utf8_view, utf8_view, std::size_t> split_suffix (std::size_t count) const;

			/// Suffix splitting with a predictor
			///
			/// The view will be splitted into two partitions. The second partition will hold the longest
			/// suffix of characters for which \a predictor returns true. The first partition will hold
			/// all preceeding characters. Characters are passed on starting with the last and traversing
			/// through to the first one in the view.
			///
			/// @note Time complexity is linear to the length of the view.
			///
			template <Callable<bool, char32_t> C>
			constexpr std::tuple<utf8_view, utf8_view> split_suffix (C predictor) const;

			/// Suffix splitting with a predictor and a variable
			///
			/// The view will be splitted into two partitions. The second partition will hold the longest
			/// suffix of characters for which \a predictor returns true. The first partition will hold
			/// all preceeding characters. Characters are passed on starting with the last and traversing
			/// through to the first one in the view.
			///
			/// @note Worst time complexity is linear to the length of the view.
			///
			template <typename V, Callable<std::tuple<V, bool>, V, char32_t> C>
			constexpr std::tuple<utf8_view, utf8_view, V> split_suffix (C predictor, V variable) const;

			/// Suffix splitting with a bounded unicode sequence
			///
			/// The view will be splitted into two partitions. The second partition will hold the longest
			/// suffix of characters which is shared between \a other and the view. Both partitions will
			/// be returned along with a sequence for all preceeding elements in \a other.
			///
			/// @note Worst time complexity is linear to the length of the view.
			///
			template <BoundedSequence<char32_t> S>
			constexpr auto split_suffix (S other) const;




			/// Prefix taking with predictor
			template <Callable<bool, char32_t> C>
			void take_prefix (C predictor)
			{
				const auto splitting = split_prefix(std::move(predictor));
				*this = std::get<0>(splitting);
			}

			/// Prefix taking with sequence
			template <Sequence<char32_t> S>
			void take_prefix (S other)
			{
				const auto splitting = split_prefix(std::move(other));
				*this = std::get<0>(splitting);
			}

			/// Suffix taking with predictor
			template <Callable<bool, char32_t> C>
			void take_suffix (C predictor)
			{
				const auto splitting = split_suffix(std::move(predictor));
				*this = std::get<1>(splitting);
			}

			/// Prefix dropping with predictor
			template <Callable<bool, char32_t> C>
			void drop_prefix (C predictor)
			{
				const auto splitting = split_prefix(std::move(predictor));
				*this = std::get<1>(splitting);
			}

			/// Prefix dropping with sequence
			template <Sequence<char32_t> S>
			void drop_prefix (S other)
			{
				const auto splitting = split_prefix(std::move(other));
				*this = std::get<1>(splitting);
			}

			/// Suffix dropping with predictor
			template <Callable<bool, char32_t> C>
			void drop_suffix (C predictor)
			{
				const auto splitting = split_suffix(std::move(predictor));
				*this = std::get<0>(splitting);
			}



			
			constexpr bool try_take_prefix (std::size_t count)
			{
				const auto splitting = split_prefix(count);
				if (std::get<2>(splitting) == count)
					*this = std::get<0>(splitting);
				return std::get<2>(splitting) == count;
			}

			constexpr bool try_take_prefix (BoundedSequence<char32_t> prefix)
			{
				const auto splitting = split_prefix(std::move(prefix));
				if (not std::get<2>(splitting))
					*this = std::get<0>(splitting);
				return not std::get<2>(splitting);
			}

			constexpr bool try_drop_prefix (std::size_t count)
			{
				const auto splitting = split_prefix(count);
				if (std::get<2>(splitting) == count)
					*this = std::get<1>(splitting);
				return std::get<2>(splitting) == count;
			}
			constexpr bool try_drop_prefix (BoundedSequence<char32_t> prefix)
			{
				const auto splitting = split_prefix(std::move(prefix));
				if (not std::get<2>(splitting))
					*this = std::get<1>(splitting);
				return not std::get<2>(splitting);
			}

			constexpr bool try_take_suffix (std::size_t count);
			constexpr bool try_take_suffix (BoundedSequence<char32_t> suffix);
			constexpr bool try_drop_suffix (std::size_t count);
			constexpr bool try_drop_suffix (BoundedSequence<char32_t> suffix);


			
			/// Prefix spanning
			///
			/// The view will be decomposed into two partitions. The first partition will be the longest
			/// prefix which only contains elements of \a characters. The second partition will be the
			/// appendix of the view.
			///
			/// @param characters  Bounded sequence of unicode characters
			///
			template <BoundedSequence<char32_t> S>
			constexpr std::tuple<utf8_view, utf8_view> span_prefix (S characters) const
			{
				const auto splitting = split_prefix([characters=std::move(characters)](char32_t element)
				{
					return fold([element=std::move(element)](auto, auto character)
					{
						const auto isEqual = element == character;
						return std::make_tuple(isEqual, not isEqual);
					}, false, characters);
				});
				return std::make_tuple(std::get<0>(splitting), std::get<1>(splitting));
			}

			/// Suffix spanning
			///
			/// The view will be decomposed into two partitions. The second partition will be the longest
			/// suffix which only contains elements of \a characters. The first partition will be the
			/// prependix of the view.
			///
			/// @param characters  Bounded sequence of unicode characters
			///
			template <BoundedSequence<char32_t> S>
			constexpr std::tuple<utf8_view, utf8_view> span_suffix (S characters) const
			{
				const auto splitting = split_suffix([characters=std::move(characters)](char32_t element)
				{
					return fold([element=std::move(element)](auto, auto character)
					{
						const auto isEqual = character == elemenet;
						return std::make_tuple(isEqual, not isEqual);
					}, false, characters);
				});
			}




			/// Prefix matching
			///
			/// The view is tested on having \a characters as its prefix. The \a characters must be not
			/// longer than the view itself and must match the beginning of the view.
			///
			/// @param characters  Bounded sequence of unicode characters
			///
			template <BoundedSequence<char32_t> S>
			constexpr bool match_prefix (S characters) const
			{
				const auto splitting = split_prefix(std::move(characters));
				return std::get<2>(splitting);
			}

			/// Suffix matching
			///
			/// The view is tested on having \a characters as its suffix.
			template <BoundedSequence<char32_t> S>
			constexpr bool match_suffix (S characters) const
			{
				const auto splitting = split_suffix(std::move(characters));
				return std::get<2>(splitting);
			}

			template <BoundedSequence<char32_t> S>
			constexpr bool match_infix (S characters) const
			{
				const auto splitting = split_first_infix(std::move(characters));
				return splitting;
			}

			template <BoundedSequence<char32_t> S>
			constexpr bool match (S characters) const
			{
				const auto splitting = split_prefix(std::move(characters));
				return not std::get<1>(splitting) and not std::get<2>(splitting);
			}

	};

	// ----------------------------------------------------------------------------------------------
	// Decoding

	constexpr std::tuple<char32_t, std::size_t> utf8_view::decode (const unsigned char * data)
	{
		assert(data != nullptr);
		if (*data & 0x80 == 0x00)
		{
			return std::make_tuple(static_cast<char32_t>(*data), 1);
		}
		else if (*data & 0xe0 == 0xc0)
		{
			auto character = static_cast<uint_least32_t>(*data & 0x1f);
			character = (character << 6) | static_cast<uint_least32_t>(*(data + 1) & 0x3f);
			return std::make_tuple(static_cast<char32_t>(character), 2);
		}
		else if (*data & 0xf0 == 0xe0)
		{
			auto character = static_cast<uint_least32_t>(*data & 0x0f);
			character = (character << 6) | static_cast<uint_least32_t>(*(data + 1) & 0x3f);
			character = (character << 6) | static_cast<uint_least32_t>(*(data + 2) & 0x3f);
			return std::make_tuple(static_cast<char32_t>(character), 3);
		}
		else
		{
			auto character = static_cast<uint_least32_t>(*data & 0x07);
			character = (character << 6) | static_cast<uint_least32_t>(*(data + 1) & 0x3f);
			character = (character << 6) | static_cast<uint_least32_t>(*(data + 2) & 0x3f);
			character = (character << 6) | static_cast<uint_least32_t>(*(data + 3) & 0x3f);
			return std::make_tuple(static_cast<char32_t>(character), 4);
		}
	}

	// ----------------------------------------------------------------------------------------------
	// Reverse decoding

	constexpr std::tuple<char32_t, std::size_t> utf8_view::decode_reverse (const unsigned char * data, std::size_t length)
	{
		assert(data != nullptr);
		assert(length > 0);
		if (data[length-1] & 0x80 == 0x00)
		{
			return std::make_tuple(static_cast<char32_t>(data[length-1]), 1);
		}
		else if (data[length-2] & 0x80 == 0x00)
		{
			auto character = static_cast<uint_least32_t>(data[length-2] & 0x1f);
			character = (character << 6) | static_cast<uint_least32_t>(data[length-1] & 0x3f);
			return std::make_tuple(static_cast<char32_t>(character), 2);
		}
		else if (data[length-3] & 0x80 == 0x00)
		{
			auto character = static_cast<uint_least32_t>(data[length-3] & 0x0f);
			character = (character << 6) | static_cast<uint_least32_t>(data[length-2] & 0x3f);
			character = (character << 6) | static_cast<uint_least32_t>(data[length-1] & 0x3f);
			return std::make_tuple(static_cast<char32_t>(character), 3);
		}
		else
		{
			auto character = static_cast<uint_least32_t>(data[length-4] & 0x07);
			character = (character << 6) | static_cast<uint_least32_t>(data[length-3] & 0x3f);
			character = (character << 6) | static_cast<uint_least32_t>(data[length-2] & 0x3f);
			character = (character << 6) | static_cast<uint_least32_t>(data[length-1] & 0x3f);
			return std::make_tuple(static_cast<char32_t>(character), 4);
		}
	}

	// ----------------------------------------------------------------------------------------------
	// Length decoding

	constexpr std::size_t utf8_view::length (const unsigned char * data)
	{
		assert(data != nullptr);
		if (*data & 0x80 == 0x00)
		{
			return 1;
		}
		else if (*data & 0xe0 == 0xc0)
		{
			return 2;
		}
		else if (*data & 0xf0 == 0xe0)
		{
			return 3;
		}
		else
		{
			return 4;
		}
	}

	// ----------------------------------------------------------------------------------------------
	// Reverse length decoding

	constexpr std::size_t utf8_view::length_reverse (const unsigned char * data, std::size_t length)
	{
		assert(data != nullptr);
		assert(length > 0);
		if (data[length-1] & 0x80 == 0x00)
		{
			return 1;
		}
		else if (data[length-2] & 0x80 == 0x00)
		{
			return 2;
		}
		else if (data[length-3] & 0x80 == 0x00)
		{
			return 3;
		}
		else
		{
			return 4;
		}
	}

	// ----------------------------------------------------------------------------------------------
	// Prefix splitting with amount of characters

	constexpr std::tuple<ut8f_view, utf8_view, std::size_t> utf8_view::split_prefix (std::size_t count) const
	{
		const auto splitting = bytes.split_prefix([](auto count, auto byte)
		{
			const auto isHeadByte = byte & 0xc0 != 0x80;
			const auto keepOn = not isHeadByte or count > 0;
			const auto nextCount = count - isHeadByte ? 1 : 0;
			return std::make_tuple(nextCount, keepOn);
		}, count);
		const auto pre = utf8_view(std::get<0>(splitting));
		const auto post = utf8_view(std::get<1>(splitting));
		return std::make_tuple(pre, post, std::get<2>(splitting));
	}

	// ----------------------------------------------------------------------------------------------
	// Prefix splitting  with utf8_view

	constexpr std::tuple<utf8_view, utf8_view, utf8_view> utf8_view::split_prefix (utf8_view other) const
	{
		const auto splitting = bytes.split_prefix(other.bytes);
		const auto shared = utf8_view(std::get<0>(splitting));
		const auto postThis = utf8_view(std::get<1>(splitting));
		const auto postOther = utf8_view(std::get<2>(splitting));
		return std::make_tuple(shared, postThis, postOther);
	}

	// ----------------------------------------------------------------------------------------------
	// Prefix splitting with general unicode sequence

	template <Sequence<char32_t> S>
	constexpr std::tuple<utf8_view, utf8_view, S> utf8_view::split_prefix (S other) const
	{
		auto folding = fold([&](auto index, auto character)
		{
			if (index < bytes.length())
			{
				const auto decoding = decode(bytes.data() + index);
				const auto isEqual = std::get<0>(decoding) == character;
				const auto nextIndex = index + isEqual ? std::get<1>(decoding) : 0;
				return std::make_tuple(nextIndex, isEqual);
			}
			else
			{
				return std::make_tuple(index, false);
			}
		}, std::size_t(0), std::move(other));

		const auto splitting = bytes.split_prefix(std::get<0>(folding));
		const auto shared = utf8_view(std::get<0>(splitting));
		const auto postThis = utf8_view(std::get<1>(splitting));

		return std::make_tuple(shared, postThis, std::move(std::get<1>(folding));
	}

	// ----------------------------------------------------------------------------------------------
	// Prefix splitting with predictor

	template <Callable<bool, char32_t> C>
	constexpr std::tuple<utf8_view, utf8_view> utf8_view::split_prefix (C predictor) const
	{
		auto index = std::size_t(0);
		while (index < bytes.length())
		{
			const auto decoding = decode(bytes.data() + index);
			const auto isAccepted = predictor(std::get<0>(decoding));
			if (isAccepted) index += std::get<1>(decoding);
			else break;
		}
		const auto splitting = bytes.split_prefix(index);
		const auto pre = utf8_view(std::get<0>(splitting));
		const auto post = utf8_view(std::get<1>(splitting));
		return std::make_tuple(pre, post);
	}

	// ----------------------------------------------------------------------------------------------
	// Prefix splitting with predictor and variable

	template <typename V, Callable<std::tuple<V, bool>, V, char32_t> C>
	constexpr std::tuple<utf8_view, utf8_view, V> utf8_view::split_prefix (C predictor, V variable) const
	{
		auto index = std::size_t(0);
		while (index < bytes.length())
		{
			const auto decoding = decode(bytes.data() + index);
			bool keepOn;
			std::tie(variable, keepOn) = predictor(std::move(variable), std::get<0>(decoding));
			if (keepOn) index += std::get<1>(decoding);
			else break;
		}
		const auto splitting = bytes.split_prefix(index);
		const auto pre = utf8_view(std::get<0>(splitting));
		const auto post = utf8_view(std::get<1>(splitting));
		return std::make_tuple(pre, post, std::move(variable));
	}

	// ----------------------------------------------------------------------------------------------
	// Suffix splitting with amount of characters

	constexpr std::tuple<utf8_view, utf8_view, std::size_t> utf8_view::split_suffix (std::size_t count) const
	{
		const auto splitting = bytes.split_suffix([](auto count, auto byte)
		{
			const auto isHeadByte = byte & 0xc0 != 0x80;
			const auto keepOn = count > 0;
			const auto nextCount = count - isHeadByte ? 1 : 0;
			return std::make_tuple(nextCount, keepOn);
		}, count);
		const auto pre = utf8_view(std::get<0>(splitting));
		const auto post = utf8_view(std::get<1>(splitting));
		return std::make_tuple(pre, post, std::get<2>(splitting));
	}


	// ----------------------------------------------------------------------------------------------
	// Suffix splitting with utf8_view

	constexpr std::tuple<utf8_view, utf8_view, utf8_view> utf8_view::split_suffix (utf8_view other) const
	{
		const auto splitting = bytes.split_suffix(other.bytes);
		const auto preThis = utf8_view(std::get<0>(splitting));
		const auto shared = utf8_view(std::get<1>(splitting));
		const auto preOther = utf8_view(std::get<2>(splitting));
		return std::make_tuple(preThis, shared, preOther);
	}

	// ----------------------------------------------------------------------------------------------
	// Suffix splitting with a bounded unicode sequence

	template <BoundedSequence<char32_t> S>
	constexpr std::tuple<utf8_view, utf8_view, S> utf8_view::split_suffix (S other) const
	{
		// TODO implementation
	}

	// ----------------------------------------------------------------------------------------------
	// Suffix splitting with predictor

	template <Callable<bool, char32_t> C>
	constexpr std::tuple<utf8_view, utf8_view> utf8_view::split_suffix (C predictor) const
	{
		auto remainingLength = bytes.length();
		while (remainingLength > 0)
		{
			const auto decoding = decode_reverse(bytes.data(), remainingLength);
			const auto isAccepted = predictor(std::get<0>(decoding));
			if (isAccepted) remainingLength -= std::get<1>(decoding);
			else break;
		}

		const auto splitting = bytes.split_prefix(remainingLength);
		const auto pre = utf8_view(std::get<0>(splitting));
		const auto post = utf8_view(std::get<1>(splitting);
		return std::make_tuple(pre, post);
	}

	// ----------------------------------------------------------------------------------------------
	// Suffix splitting with predictor and variable

	template <typename V, Callable<std::tuple<V, bool>, V, char32_t> C>
	constexpr std::tuple<utf8_view, utf8_view, V> utf8_view::split_suffix (C predictor, V variable) const
	{
		auto remainingLength = bytes.length();
		while (remainingLength > 0)
		{
			bool keepOn;
			const auto decoding = decode_reverse(bytes.data(), remainingLength);
			std::tie(variable, keepOn) = predictor(std::move(variable), std::get<0>(decoding));
			if (keepOn) remainingLength -= std::get<1>(decoding);
			else break;
		}

		const auto splitting = bytes.split_prefix(remainingLength);
		const auto pre = utf8_view(std::get<0>(splitting));
		const auto post = utf8_view(std::get<1>(splitting);
		return std::make_tuple(pre, post, std::move(variable));
	}

}

#endif
