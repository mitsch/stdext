/// @file sequence_concept.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_SEQUENCE_CONCEPT_HPP__
#define __STDEXT_SEQUENCE_CONCEPT_HPP__

#include <tuple>
#include <stdext/optional.hpp>
#include <stdext/callable.hpp>

namespace stdext
{

	/// Concept of a bounded sequence
	///
	/// A bounded sequence is a definition of finite many values which are ordered. The definition
	/// can be mathematically inspired or be based on some data. A bounded sequence must be copyable
	/// and movable. The bounded sequence is decomposed into its first value and the remaining ones
	/// by the function operator. It is called without any parameter and returns an optional
	template <typename S> concept bool BoundedSequence ()
	{
		return require (S s)
		{
			S(s);
			s = S(s);
			s();
			{s} -> bool
		} and
		is_optional<decltype(std::declval<S>()())>::value and
		std::is_same<std::tuple_element<1, decltype(std::declval<S>()())::value_type>::type, S>::value;
	}

	/// Concept of a bounded sequence with value type \a T
	///
	/// Additionally to the constraints of being a bounded sequence, type \a S must produce elements
	/// of type \a T.
	template <typename S, typename T> concept bool BoundedSequence ()
	{
		return BoundedSequence<S> and
		       std::is_convertible<std::tuple_element<0, decltype(std::declval<S>()())::value_type>::type, T>::value;
	}

	/// Concept of an indexed bounded sequence
	///
	/// An indexed bounded sequence must require the concept of a bounded sequence. All elements in
	/// the sequence shall be accessible via the index operator. Accessing elements via the index
	/// operator must return an optional container
	template <typename S> concept bool IndexedBoundedSequence ()
	{
		return BoundedSequence<S> and
		       require (S s){s[std::size_t(0)];} and
					 require (S s){{s.length()} -> std::size_t;} and
		       is_optional<decltype(std::declval<S>()[std::size_t(0)])>::value and
					 std::is_same<
					 	std::tuple_element<0, decltype(std::declval<S>()())::value_type>::type,
		       	decltype(std::declval<S>()[std::size_t(0)])::value_type
						>::value;
	}

	/// Concept of an indexed unbounded sequence
	///
	/// Additionally to the constraints of being an unbounded sequence, type \a S must produce
	/// elements of type \a T.
	template <typename S, typename T> concept bool IndexedBoundedSequence ()
	{
		return IndexedBoundedSequence<S> and
		       std::is_convertible<std::tuple_element<0, decltype(std::declval<S>()())::value_type>::type, T>::value
	}

	/// 
	template <typename S> concept bool UnboundedSequence ()
	{
		return require (S s)
		{
			S(s);
			s();
		} and
		std::is_same<std::tuple_element<1, typename S::value_type>::type, S>::value;
	}

	template <typename S, typename T> concept bool UnboundedSequence ()
	{
		return UnboundedSequence<S> and
		       std::is_convertible<std::tuple_element<0, decltype(std::declval<S>()())>::type, T>::value;
	}

	template <typename S> concept bool IndexedUnboundedSequence ()
	{
		return UnboundedSequence<S> and
		       require(S s){s[std::size_t(0)]} and
					 std::is_same<
		       	decltype(std::declval<S>()[std::size_t(0)]),
		       	std::tuple_element<0, decltype(std::declval<S>()())>::type>::value;
	}

	template <typename S, typename T> concept bool IndexedUnboundedSequence ()
	{
		return IndexedUnboundedSequence<S> and
		       std::is_convertible<decltype(std::declval<S>()[std::size_t(0)]), T>::value;
	}


	template <typename S> concept bool Sequence ()
	{
		return BoundedSequence<S> or UnboundedSequence<S>;
	}

	template <typename S, typename T> concept bool Sequence ()
	{
		return BoundedSequence<S, T> or UnboundedSequence<S, T>;
	}


	/// Trait of sequence's element type
	///
	/// 
	template <typename S> struct sequence_type {};
	template <BoundedSequence S> struct sequence_type<S> :
		std::tuple_element<0, decltype(std::declval<S>()())::value_type> {};
	template <UnboundedSequence S> struct sequence_type<S> :
		std::tuple_element<0, decltype(std::declval<S>()())> {};
	template <typename S> using sequence_type_t = typename sequence_type<S>::type;


	/// Folding all elements of a bounded sequence
	///
	/// All elements in \a sequence are folded by \a combiner starting with \a value. This is a
	/// general implementation which can be specialised for known sequences. The general
	/// implementation decomposes the sequence until no element is left. With each decomposition, the
	/// folding procedure steps forward with the decomposed element. The returning value will be
	/// folded over all elements.
	template <typename V, BoundedSequence S, Callable<V, V, sequence_type_t<S>> C>
	constexpr V fold (C combiner, V value, S sequence)
	{
		auto keepFolding = true;
		while (keepFolding)
		{
			auto decision = decide([&combiner](auto decomposition, auto value, auto sequence)
			{
				value = combiner(std::move(value), std::move(std::get<0>(decomposition));
				return std::make_tuple(std::move(value), std::move(std::get<1>(decomposition)), true);
			}, [](auto value, auto sequence)
			{
				return std::make_tuple(std::move(value), std::move(sequence), false);
			}, sequence(), std::move(value), std::move(sequence));

			value = std::move(std::get<0>(decision));
			sequence = std::move(std::get<1>(decision));
			keepFolding = std::get<2>(decision);
		}
		return value;
	}


	/// 
	template <typename V, BoundedSequence S, Callable<std::tuple<V, bool>, V, sequence_type_t<S>> C>
	constexpr std::tuple<V, S> fold (C combiner, V value, S sequence)
	{
		bool keepFolding = true;
		auto onElement = [combiner](auto concatenation, V value, S sequence)
		{
			auto nextElement = std::move(std::get<0>(concatenation));
			auto nextSequence = std::move(std::get<1>(concatenation));
			auto combination = combiner(std::move(value), std::move(nextElement));
			value = std::move(std::get<0>(combination));
			auto keepGoing = std::get<1>(combination);
			auto returningSequence = keepGoing ? std::move(nextSequence) : std::move(sequence);
			return std::make_tuple(std::move(value), std::move(returningSequence), keepGoing);
		};
		auto onEnd = [](V value, S sequence)
		{
			return std::make_tuple(std::move(value), std::move(sequence), false);
		};
		while (keepFolding)
		{
			auto concatenation = sequence();
			std::tie(value, sequence, keepFolding) = decide(onElement, onEnd, std::move(concatenation),
				std::move(value), std::move(sequence));
		}
		return std::make_tuple(value, sequence);
	}

	template <typename V, UnboundedSequence S, Callable<std::tuple<V, bool>, V, sequence_type_t<S>> C>
	constexpr V fold (C combine, V value, S sequence)
	{
		bool keepFolding = true;
		while (keepFolding)
		{
			auto concatenation = sequence();
			std::tie(value, keepFolding) = combine(std::move(value), std::move(std::get<0>(concatenation)));
			if (keepFolding)
				sequence = std::move(std::get<1>(concatenation));
		}
		return std::make_tuple(value, sequence);
	}

	/// Folding all elements of indexed bounded sequence
	///
	/// Elements of \a sequence starting at \a index are folded by \a combiner. Initially, folding
	/// starts off with \a value. All elements from \a index to the end are folded and returned.
	template <typename V, IndexedBoundedSequence S, Callable<V, V, sequence_type_t<S>> C>
	constexpr V fold (C combiner, V value, const S & sequence, std::size_t index)
	{
		const auto length = sequence.length();
		for (; index < length; ++index)
		{
			value = decide([&](auto element, auto value)
			{
				return combiner(std::move(value), std::move(element));
			}, [](auto value)
			{
				return value;
			}, sequence[index], std::move(value));
		}
		return value;
	}

	/// Folding initial elements of indexed bounded sequence
	///
	/// Elements of \a sequence starting at \a index are folded by \a combiner. Initially, folding
	/// starts off with \a value. As long as \a combiner returns a true flag, folding will continue.
	/// The returning tuple is the folded value and the first index for which element \a combiner has
	/// returned a false flag.
	template <typename V, IndexedBoundedSequence S, Callable<std::tuple<V, bool>, V, sequence_type_t<S>> C>
	constexpr std::tuple<V, std::size_t> fold (C combiner, V value, const S & sequence, std::size_t index)
	{
		const auto length = sequence.length();
		auto keepOn = true;
		while (keepOn and index < length)
		{
			std::tie(value, keepOn) = decide([&](auto element, auto value)
			{
				return combiner(std::move(value), std::move(element));
			}, [](auto value)
			{
				return std::make_tuple(std::move(value), true);
			}, sequence[index], std::move(value));
			if (keepOn) ++index;
		}
		return std::make_tuple(std::move(value), index);
	}

	/// Folding initial elements of indexed unbounded sequence
	///
	/// Elements of \a sequence starting at \a index are folded by \a combiner. Initially, folding
	/// starts off with \a value. As long as \a combiner returns a true flag, folding will continue.
	/// The returning tuple is the folded value and the first index for which element \a combiner has
	/// returned a false flag.
	template <typename V, IndexedUnboundedSequence S, Callable<std::tuple<V, bool>, V, sequence_type_t<S>> C>
	constexpr std::tuple<V, std::size_t> fold (C combiner, V value, const S & sequence, std::size_t index)
	{
		auto keepOn = true;
		while (keepOn)
		{
			std::tie(value, keepOn) = combiner(std::move(value), sequence[index]);
			if (keepOn) ++index;
		}
		return std::make_tuple(std::move(value), index);
	}


	
	template <typename S>
		requires BoundedSequence<S>
	constexpr std::size_t length (S sequence)
	{
		return fold([](auto counter, auto value)
		{
			return counter + 1;
		}, std::size_t(0), std::move(sequence));
	}

	template <typename S>
		requires IndexedBoundedSequence<S>
	constexpr std::size_t length (S sequence)
	{
		return s.length();
	}

}

#endif

