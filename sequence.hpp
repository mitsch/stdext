/// @file sequence.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_SEQUENCE_HPP__
#define __STDEXT_SEQUENCE_HPP__

#include <tuple>
#include <stdext/optional.hpp>

namespace stdext
{

	/// Concept of a sequence
	///
	/// A sequence is a definition of values which are ordered. The definition can be mathematically inspired
	/// or based on some data.
	template <typename S> concept bool BoundedSequence ()
	{
		return require (S s)
		{
			S(s);
			S(std::move(s));
			s();
		} and
		is_optional<decltype(std::declval<S>()())>::value and
		std::is_same<std::tuple_element<1, decltype(std::declval<S>()())::value_type>::type, S>::value;
	}

	template <typename S, typename T> concept bool BoundedSequence ()
	{
		return BoundedSequence<S> and
		       std::is_convertible<std::tuple_element<0, decltype(std::declval<S>()())::value_type>::type, T>::value;
	}

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

	template <typename S, typename T> concept bool IndexedBoundedSequence ()
	{
		return IndexedBoundedSequence<S> and
		       std::is_convertible<std::tuple_element<0, decltype(std::declval<S>()())::value_type>::type, T>::value
	}

	template <typename S> concept bool UnboundedSequence ()
	{
		return require (S s)
		{
			S(s);
			S(std::move(s));
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



	template <typename S> struct sequence_type {};
	template <BoundedSequence S> struct sequence_type<S> :
		std::tuple_element<0, decltype(std::declval<S>()())::value_type> {};
	template <UnboundedSequence S> struct sequence_type<S> :
		std::tuple_element<0, decltype(std::declval<S>()())> {};
	template <typename S> using sequence_type_t = typename sequence_type<S>::type;

	
	template <typename V, BoundedSequence S, Callable<V, V, sequence_type_t<S>> C>
	constexpr V fold (C combine, V value, S sequence)
	{
		bool keepFolding = true;
		auto onElement = [combine=std::move(combine)](auto concatenation, V value, S sequence)
		{
			value = combine(std::move(value), std::move(std::get<0>(concatenation)));
			return std::make_tuple(std::move(value), std::move(std::get<1>(concatenation)), true);
		};
		auto onEnd = [](V value, S sequence)
		{
			return std::make_tuple(std::move(value), std::move(sequence), false);
		};
		while (keepFolding)
		{
			auto nextConcatenation = sequence();
			std::tie(value, sequence, keepFolding) = decide(onElement, onEnd, std::move(nextConcatenation));
		}
		return value;
	}

	template <typename V, BoundedSequence S, Callable<std::tuple<V, bool>, V, sequence_type_t<S>> C>
	constexpr V fold (C combine, V value, S sequence)
	{
		bool keepFolding = true;
		auto onElement = [combine=std::move(combine)](auto concatenation, V value, S sequence)
		{
			auto combination = combine(std::move(value), std::move(std::get<0>(concatenation)));
			return std::make_tuple(std::move(std::get<0>(combination)), std::move(std::get<1>(concatenation)), std::get<1>(combination));
		};
		auto onEnd = [](V value, S sequence)
		{
			return std::make_tuple(std::move(value), std::move(sequence), false);
		};
		while (keepFolding)
		{
			auto nextConcatenation = sequence();
			std::tie(value, sequence, keepFolding) = decide(onElement, onEnd, std::move(nextConcatenation));
		}
		return value;
	}

	template <typename V, UnboundedSequence S, Callable<std::tuple<V, bool>, V, sequence_type_t<S>> C>
	constexpr V fold (C combine, V value, S sequence)
	{
		bool keepFolding = true;
		while (keepFolding)
		{
			auto concatenation = sequence();
			std::tie(value, keepFolding) = combine(std::move(value), std::move(std::get<0>(concatenation)));
			sequence = std::move(std::get<1>(concatenation));
		}
		return value;
	}

}

#endif

