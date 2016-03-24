/// @file text_view.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_TEXT_VIEW_HPP__
#define __STDEXT_TEXT_VIEW_HPP__

#include <stdext/character.hpp>
#include <stdext/optional.hpp>
#include <stdext/array_view.hpp>
#include <tuple>
#include <cstring>
#include <cuchar>

namespace stdext
{

	class text_view
	{
	
		private:

			array_view<const char> encoding;
			mbstate_t state;

		public:


			/// Attribute constructor
			///
			/// Thew view is constructed with a view on the underlying \a bytes.
			constexpr text_view (array_view<const char> encoding, mbstate_t state)
				: encoding(encoding), state(std::move(state))
				noexcept
			{}

			/// C-String constructor
			///
			/// The view is constructed with a null terminated byte string, also known as C-string. The
			/// view will include all characters until the null terminating byte which will be excluded
			/// from the view.
			///
			/// @param characters  Null terminated byte string which will be viewed
			constexpr text_view (const char * characters)
				: text_view (characters, std::strlen(characters))
				noexcept
			{}



			/// Decomposition
			constexpr optional<std::tuple<char32_t, text_view>> operator () () const;


			/// Swapping
			friend constexpr void swap (text_view & first, text_view & second)
			{
				using std::swap;
				swap(first.encoding, second.encoding);
				swap(first.state, second.state);
			}


			/// Non-Emptiness
			constexpr bool operator () const
			{
				return encoding;
			}


			/// Length in characters
			constexpr size_t length () const;


			/// Length in bytes
			constexpr size_t raw_length () const
			{
				return encoding.length();
			}


			/// Data pointer to byte encoding
			constexpr const unsigned char* raw_data () const
			{
				return static_cast<const unsigned char*>(encoding.data());
			}

			/// Encoding view
			constexpr array_view<const unsigned char*> raw_view () const
			{
				return array_view<const unsigned char>(raw_data(), raw_length());
			}



			// ----------------------------------------
			// Folding

			template <typename V, Callable<V, V, char32_t> C>
			constexpr V fold (C combiner, V value) const;

			template <typename V, Callable<std::tuple<V, bool>, V, char32_t> C>
			constexpr std::tuple<V, text_view, text_view> fold (C combiner, V value) const;




			// ----------------------------------------
			// Matching

			template <BoundedSequence<char32_t> S>
			constexpr bool match_prefix (S prefix) const
			{
				const auto splitting = split_prefix(std::move(prefix));
				return not std::get<2>(splitting);
			}

			template <BoundedSequence<char32_t> S>
			constexpr bool match (S other) const
			{
				const auto splitting = split_prefix(std::move(other));
				return not std::get<1>(splitting) and not std::get<2>(splitting);
			}

			// TODO match_infix



			// ----------------------------------------
			// Prefix shrinking

			constexpr void take_prefix (size_t count)
			{
				const auto splitting = split_prefix(count);
				*this = std::get<0>(splitting);
			}

			template <Callable<bool, char32_t> C>
			constexpr void take_prefix (C predictor)
			{
				const auto splitting = split_prefix(std::move(predictor));
				*this = std::get<0>(splitting);
			}

			template <Sequence<char32_t> S>
			constexpr void take_prefix (S other)
			{
				const auto splitting = split_prefix(std::move(other));
				*this = std::get<0>(splitting);
			}

			constexpr void drop_prefix (size_t count)
			{
				const auto splitting = split_prefix(count);
				*this = std::get<1>(splitting);
			}

			template <Callable<bool, char32_t> C>
			constexpr void drop_prefix (C predictor)
			{
				const auto splitting = split_prefix(std::move(predictor));
				*this = std::get<1>(splitting);
			}

			template <Sequence<char32_t> S>
			constexpr void drop_prefix (S other)
			{
				const auto splitting = split_prefix(std::move(other));
				*this = std::get<1>(splitting);
			}




			// ----------------------------------------
			// Prefix shrinking trial

			constexpr bool try_take_prefix (size_t count)
			{
				const auto splitting = split_prefix(count);
				if (std::get<2>(splitting) == count)
					*this = std::get<0>(splitting);
				return std::get<2>(splitting) == count;
			}

			template <BoundedSequence<char32_t> S>
			constexpr bool try_take_prefix (S other)
			{
				const auto splitting = split_prefix(std::move(other));
				if (not std::get<2>(splitting))
					*this = std::get<0>(splitting);
				return not std::get<2>(splitting);
			}

			constexpr bool try_drop_prefix (size_t count)
			{
				const auto splitting = split_prefix(count);
				if (std::get<2>(splitting) == count)
					*this = std::get<1>(splitting);
				return std::get<2>(splitting) == count;
			}

			template <BoundedSequence<char32_t> S>
			constexpr bool try_drop_prefix (S other)
			{
				const auto splitting = split_prefix(std::move(other));
				if (not std::get<2>(splitting))
					*this = std::get<1>(splitting);
				return not std::get<2>(splitting);
			}




			// ----------------------------------------
			// Prefix splitting

			constexpr std::tuple<text_view, text_view, size_t> split_prefix (size_t count) const;

			template <Callable<bool, char32_t> C>
			constexpr std::tuple<text_view, text_view> split_prefix (C predictor) const;
			
			template <typename V, Callable<std::tuple<V, bool>, V, char32_t> C>
			constexpr std::tuple<text_view, text_view, V> split_prefix (C predictor, V variable) const;
			
			template <Sequence<char32_t> S>
			constexpr std::tuple<text_view, text_view, S> split_prefix (S other) const;

			

	};

	// ----------------------------------------------------------------------------------------------
	// Length for sequence

	constexpr size_t length (text_view characters)
	{
		return characters.length();
	}

	
	// ----------------------------------------------------------------------------------------------
	// Length in characters

	constexpr size_t text_view::length () const
	{
		auto postState = state;
		auto index = size_t(0);
		auto count = size_t(0);

		while (index < encoding.length)
		{
			const auto step = std::mbrlen(encoding.data() + index, encoding.length() - index, &postState);
			index += step == 0 ? 1 : step;
			++count;
		}

		return count;
	}


	// ----------------------------------------------------------------------------------------------
	// Forward folding for sequence

	template <typename V, Callable<V, V, char32_t> C>
	constexpr V fold (C combiner, V value, text_view characters)
	{
		return characters.fold(std::move(combiner), sd::move(value));
	}

	template <typename V, Callable<std::tuple<V, bool>, V, char32_t> C>
	constexpr V fold (C combiner, V value, text_view characters)
	{
		const auto folding = characters.fold(std::move(combiner), std::move(value));
		return std::make_tuple(std::get<0>(folding), std::get<2>(folding));
	}



	// ----------------------------------------------------------------------------------------------
	// Forward folding

	template <typename V, Callable<V, V, char32_t> C>
	constexpr V text_view::fold (C combiner, V value) const
	{
		auto postState = state;
		auto index = size_t(0);
		char32_t character;
		
		while (index < encoding.length())
		{
			const auto step = std::mbrtoc32(&character, encoding.data() + index, encoding.length() - index, &postState);
			value = combiner(std::move(value), character);
			index += step == 0 ? 1 : step;
		}

		return value;
	}

	template <typename V, Callable<std::tuple<V, bool>, V, char32_t> C>
	constexpr std::tuple<V, text_view, text_view> text_view::fold (C combiner, V value) const
	{
		auto postState = state;
		auto index = size_t(0);
		char32_t character;
		auto keepOn = true;

		while (index < encoding.length())
		{
			const auto formerPostState = postState;
			const auto step = std::mbrtoc32(&character, encoding.data() + index, encoding.length() - index, &postState);
			std::tie(value, keepOn) = combiner(std::move(value), character);
			if (keepOn)
			{
				index += (step == 0 ? 1 : step);
			}
			else
			{
				postState = formerPostState;
				break;
			}
		}

		const auto splitting = encoding.split_prefix(index);
		const auto pre = text_view(std::get<0>(splitting), state);
		const auto post = text_view(std::get<1>(splitting), postState);

		return std::make_tuple(std::move(value), pre, post);
	}


	// ----------------------------------------------------------------------------------------------
	// Prefix splitting after an amount of characters

	constexpr std::tuple<text_view, text_view, std::size_t> text_view::split_prefix (std::size_t count) const
	{
		auto postState = state;
		auto index = size_t(0);
		auto actualCount = size_t(0);

		while (index < encoding.length() and actualCount < count)
		{
			const auto step = std::mbrlen(encoding.data() + index, encoding.length() - index, &postState);
			index += step == 0 ? 1 : step;
			++actualCount;
		}

		const auto pre = text_view(array_view<const char>(encoding.data(), index), state);
		const auto post = text_view(array_view<const char>(encoding.data() + index, encoding.length() - index), postState);
		return std::make_tuple(pre, post, actualCount);
	}

	// ----------------------------------------------------------------------------------------------
	// Prefix splitting with predictor

	template <Callable<bool, char32_t> C>
	constexpr std::tuple<text_view, text_view> text_view::split_prefix (C predictor) const
	{
		auto postState = state;
		auto index = size_t(0);

		while (index < encoding.length())
		{
			char32_t character;
			auto nextPostState = postState;
			const auto step = std::mbrtoc32(&character, encoding.data() + index, encoding.length() - index, &nextPostState);
			const auto isAccepted = predictor(character);
			if (isAccepted)
			{
				index += step == 0 ? 1 : step;
				postState = nextPostState;
			}
			else
			{
				break;
			}
		}

		const auto pre = text_view(array_view<const char>(encoding.data(), index), state);
		const auto post = text_view(array_view<const char>(encoding.data() + index, encoding.length() - index), postState);
		return std::make_tuple(pre, post);
	}

	// ----------------------------------------------------------------------------------------------
	// Prefix splitting with predictor and variable

	template <typename V, Callable<std::tuple<V, bool>, V, char32_t> C>
	constexpr std::tuple<text_view, text_view, V> text_view::split_prefix (C predictor, V variable) const
	{
		auto postState = state;
		auto index = size_t(0);

		while (index < encoding.length())
		{
			char32_t character;
			bool isAccepted;
			auto formerPostState = postState;
			const auto step = std::mbrtoc32(&character, encoding.data() + index, encoding.length() - index, &postState);
			std::tie(isAccepted, variable) = predictor(std::move(variable), character);
			if (isAccepted)
			{
				index += step == 0 ? 1 : step;
			}
			else
			{
				postState = formerPostState;
				break;
			}
		}

		const auto pre = text_view(array_view<const char>(encoding.data(), index), state);
		const auto post = text_view(array_view<const char>(encoding.data() + index, encoding.length() - index), postState);
		return std::make_tuple(pre, post, std::move(variable));
	}

	// ----------------------------------------------------------------------------------------------
	// Prefix splitting with unicode sequence

	template <Sequence<char32_t> S>
	constexpr std::tuple<text_view, text_view, S> text_view::split_prefix (S other) const
	{
		auto postState = state;
		auto folding = fold([&](auto index, auto element)
		{
			if (index < encoding.length())
			{
				char32_t character;
				auto formerPostState = postState;
				const auto step = std::mbrtoc32(&character, encoding.data() + index, encoding.length() - index, &postState);
				if (character == element)
				{
					index += step == 0 ? 1 : step;
				}
				else
				{
					postState = formerPostState;
				}
				return std::make_tuple(index, character == element);
			}
			else
			{
				return std::make_tuple(encoding.length(), false);
			}
		}, size_t(0), std::move(other));

		const auto splitting = encoding.split_prefix(std::get<0>(folding));
		const auto pre = text_view(std::get<0>(splitting), state);
		const auto post = text_view(std::get<1>(splitting), postState);
		return std::make_tuple(pre, post, std::move(std::get<1>(folding)));
	}

}

#endif

