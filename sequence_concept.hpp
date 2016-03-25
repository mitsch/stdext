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
			s.decompose();
		} and
		is_optional<decltype(std::declval<S>().decompose())>::value and
		std::is_same<std::tuple_element<1, decltype(std::declval<S>().decompose())::value_type>::type, S>::value;
	}

	/// Concept of a bounded sequence with value type \a T
	///
	/// Additionally to the constraints of being a bounded sequence, type \a S must produce elements
	/// of type \a T.
	template <typename S, typename T> concept bool BoundedSequence ()
	{
		return BoundedSequence<S> and
		       std::is_convertible<std::tuple_element<0, decltype(std::declval<S>().decompose())::value_type>::type, T>::value;
	}


	/// Concept of a reversible bounded sequence
	///
	/// A reversible bounded sequence is a bounded sequence which can be folded from behind. It must
	/// implement the method fold_reverse both for complete folding and conditional folding.
	template <typename S> concept bool ReversibleBoundedSequence ()
	{
		return BoundedeSequence<S> and
		       requires(S s){s.decompose_reverse();} and
					 is_optional_v<decltype(std::declval<S>().decompose_reverse())> and
		       std::is_same<std::tuple_element<1, decltype()>::type>::value;
	}


	///
	template <typename S> concept bool UnboundedSequence ()
	{
		return require (S s)
		{
			S(s);
			s.decompose();
		} and
		std::is_same<std::tuple_element<1, typename S::value_type>::type, S>::value;
	}

	template <typename S, typename T> concept bool UnboundedSequence ()
	{
		return UnboundedSequence<S> and
		       std::is_convertible<std::tuple_element<0, decltype(std::declval<S>()())>::type, T>::value;
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
	template <typename S> struct sequence_type {};
	template <BoundedSequence S> struct sequence_type<S>
		: std::tuple_element<0, decltype(std::declval<S>()())::value_type> {};
	template <UnboundedSequence S> struct sequence_type<S>
		: std::tuple_element<0, decltype(std::declval<S>()())> {};
	template <typename S> using sequence_type_t = typename sequence_type<S>::type;


	/// Comlete folding of a bounded sequence
	///
	/// All elements in \a sequence are folded by \a combiner starting with \a value. This is a
	/// general implementation which can be specialised for known sequences. The general
	/// implementation decomposes the sequence until no element is left. With each decomposition, the
	/// folding procedure steps forward with the decomposed element. The returning value will be
	/// folded over all elements.
	template <typename V, BoundedSequence S, Callable<V, V, sequence_type_t<S>> C>
	constexpr V fold (C combiner, V value, S sequence)
	{
		auto keepOn = true;
		while (keepOn)
		{
			auto decomposition = sequence.decompose();
			auto decision = decide([&combiner](auto decomposition, V value, S sequence)
			{
				value = combiner(std::move(value), std::move(std::get<0>(decomposition));
				return std::make_tuple(std::move(value), std::move(std::get<1>(decomposition)), true);
			}, [](V value, S sequence)
			{
				return std::make_tuple(std::move(value), std::move(sequence), false);
			}, std::move(decomposition), std::move(value), std::move(sequence));
			value = std::move(std::get<0>(decision));
			sequence = std::move(std::get<1>(decision));
			keepOn = std::get<2>(decision);
		}
		return value;
	}


	/// Partial folding of a bounded sequence
	///
	/// The elements in \a sequence will be folded over \a value by \a combiner. This is a general
	/// implementation which decomposes \a sequence element by element and updates \a value with each
	/// element. The element traversion keeps on as long as \a combiner returns a true flag.The value
	/// returned with the first false flag from \a combiner will be returned along with a sequence
	/// holding all the remaining elements of \a sequence including the element for which \a combiner
	/// has returned the first false flag.
	///
	template <typename V, BoundedSequence S, Callable<std::tuple<V, bool>, V, sequence_type_t<S>> C>
	constexpr std::tuple<V, S> fold (C combiner, V value, S sequence)
	{
		auto keepOn = true;
		while (keepOn)
		{
			auto decomposition = sequence.decompose();
			auto decision = decide([&combiner](auto decomposition, V value, S sequence)
			{
				auto combination = combiner(std::move(value), std::move(std::get<0>(decomposition));
				auto nextSequence = std::get<1>(combination) ? std::move(std::get<1>(decomposition)) : std::move(sequence));
				return std::make_tuple(std::move(std::get<0>(combination)), std::move(nextSequence), std::get<1>(combination));
			}, [](V value, S sequence)
			{
				return std::make_tuple(std::move(value), std::move(sequence), false);
			}, std::move(decomposition), std::move(value), std::move(sequence));
			value = std::move(std::get<0>(decision));
			sequence = std::move(std::get<1>(decision));
			keepOn = std::get<2>(decision);
		}
		return std::make_tuple(value, sequence);
	}


	/// Partial folding of an unbounded sequence
	///
	/// The elements in \a sequence will be folded over \a value by \a combiner. This is a general
	/// implementation which decomposes \a sequence element by element and updates \a value with each
	/// element. The element traversion keeps on as long as \a combiner returns a true flag. The
	/// value returned with the first false flag from \a combiner will be returned along with a
	/// sequence holding all the remaining elements of \a sequence.
	///
	template <typename V, UnboundedSequence S, Callable<std::tuple<V, bool>, V, sequence_type_t<S>> C>
	constexpr std::tuple<V, S> fold (C combiner, V value, S sequence)
	{
		bool keepFolding = true;
		while (keepFolding)
		{
			auto decomposition = sequence.decompose();
			std::tie(value, keepFolding) = combiner(std::move(value), std::move(std::get<0>(decomposition)));
			if (keepFolding)
				sequence = std::move(std::get<1>(decomposition));
		}
		return std::make_tuple(value, sequence);
	}


	/// Complete reverse folding of a bounded sequence
	///
	/// The elements in \a sequence will be folded in reverse over \a value by \a combiner. This is a
	/// general implementation which decomposes \a sequence in reverse element by element and updates
	/// \a value with each element.
	///
	template <typename V, ReversibleBoundedSequence S, Callable<V, V, sequence_type_t<S>> C>
	constexpr V fold_reverse (C combiner, V value, S sequence)
	{
		auto keepOn = true;
		while (keepOn)
		{
			auto decomposition = sequence.decompose_reverse();
			auto decision = decide([&combiner](auto decomposition, V value, S sequence)
			{
				value = combiner(std::move(value), std::move(std::get<0>(decomposition));
				return std::make_tuple(std::move(value), std::move(sequence), true);
			}, [](V value, S sequence)
			{
				return std::make_tuple(std::move(value), std::move(sequence), false);
			}, std::move(decomposition), std::move(value), std::move(sequence));
			value = std::move(std::get<0>(decision));
			sequence = std::move(std::get<1>(decision));
			keepOn = std::get<2>(decision);
		}
		return value;
	}


	/// Partial reverse folding of a bounded sequence
	///
	/// The elements in \a sequence will be folded in reverse over \a value by \a combiner. This is a
	/// general implementaiton which decomposes \a sequence in reverse element byelement and updates
	/// \a value with each element. The element traversion keeps on as long as \a combiner returns a
	/// true flag. The value returned with the first false flag from \a combiner will be returned
	/// along with a sequence holding all the remaining elements of \a sequence.
	///
	template <typename V, ReversibleBoundedSequence S, Callable<std::tuple<V, bool>, V, sequence_type_t<S>> C>
	constexpr std::tuple<V, S> fold_reverse (C combiner, V value, S sequence)
	{
		auto keepOn = true;
		while (keepOn)
		{
			auto decomposition = sequence.decompose_reverse();
			auto decision = decide([&combiner](auto decomposition, V value, S sequence)
			{
				auto combination = combiner(std::move(value), std::move(std::get<0>(decomposition)));
				auto nextSequence = std::get<1>(combination) ? std::move(std::get<1>(decomposition)) : std::move(sequence);
				return std::make_tuple(std::move(std::get<0>(combination)), std::move(nextSequence), std::get<1>(combination));
			}, [](V value, S sequence)
			{
				return std::make_tuple(std::move(value), std::move(sequence), false);
			}, std::move(decomposition), std::move(value), std::move(sequence));
			value = std::move(std::get<0>(decision));
			sequence = std::move(std::get<1>(decision));
			keepOn = std::get<2>(decision);
		}
		return std::make_tuple(std::move(value), std::move(sequence));
	}


	/// Emptiness test for bounded sequence
	///
	/// The bounded \a sequence will be tested on having no element left, aka being empty. This is a
	/// general implementation, which tries to decompose and tests whether the returning optional
	/// container is empty or not. The method can be overloaded for more efficient and more
	/// performant implementation of particular sequences.
	///
	template <BoundedSequence S>
	constexpr bool empty (const S & sequence)
	{
		const auto decomposition = sequence.decompose();
		return not decomposition.good();
	}


	/// Length of bounded sequence
	///
	/// The length of the bounded \a sequence will be computed. This is a general implementation
	/// which computes the length by folding over its elements while counting up. The method can be
	/// overloaded for more efficient and more performant implementation of particular sequences.
	///
	template <BoundedSequence S>
	constexpr std::size_t length (S sequence)
	{
		return fold([](auto counter, auto element)
		{
			(void)element;
			return counter + 1;
		}, std::size_t(0), std::move(sequence));
	}



	/// Filter sequencer for bounded sequences
	///
	/// The sequencer filters out all elements which a predictor does not agree on.
	template <BoundedSequence S, Callable<bool, sequence_type_t<S>> C>
	class bounded_filter
	{

	protected:

		S sequence;
		C predictor;

		std::tuple<optional<value_type>, bool> next (optional<value_type> target, value_type element) const
		{
			const auto isFound = not target.empty();
			if (not isFound and predictor(element))
			{
				target = std::move(element);
			}
			return std::make_tuple(std::move(target), isFound);
		}

		std::tuple<value_type, S> tupleise (value_type element, S sequence) const
		{
			return std::make_tuple(std::move(element), std::move(sequence))
		}

	public:

		using value_type = sequence_type_t<S>;

		/// Attribute constructor
		///
		/// A bounded filter will be constructed with a \a sequence and a \a predictor.
		constexpr bounded_filter (S sequence, C predictor) noexcept
			: sequence(std::move(sequence)), predictor(std::move(predictor))
		{}

		/// Prefix decomposition
		///
		/// If there is any elemnt in the underlying sequence which is conform with the predictor, then
		/// the next element in the sequence conforming with the predictor will be returned along with a
		/// sequencer for the succeeding elements of this particular element. If no element in the
		/// underlying sequence is conform with the predictor, an empty optional container will be
		/// returned.
		///
		constexpr optional<std::tuple<value_type, bounded_filter>> decompose () const
		{
			auto folding = fold(next, optional<value_type>(), sequence);
			return fmap(tupleise, std::move(std::get<0>(folding)), std::move(std::get<1>(folding)));
		}

		/// Suffix decomposition
		///
		/// If there is any elemnt in the underlying sequence which is conform with the predictor, then
		/// the last element in the sequence conforming with the predictor will be returned along with a
		/// sequencer for the preceeding elements of this particular element. If no element in the
		/// underlying sequence is conform with the predictor, an empty optional container will be
		/// returned.
		///
		/// @note This method is only enabled for reversible bounded sequences.
		///
		template <typename=typename std::enable_if<ReversibleBoundedSequence<S>>::type>
		constexpr optional<std::tuple<value_type, bounded_filter>> decompose_reverse () const
		{
			auto folding = fold_reverse(next, optional<value_type>(), sequence);
			return fmap(tupleise, std::move(std::get<0>(folding)), std::move(std::get<1>(folding)));
		}

		/// Emptiness testing
		///
		/// The underlying sequence of \a sequencer will be tested on having no element which is
		/// conforming with the predictor.
		friend constexpr bool empty (const bounded_filter & sequencer)
		{
			auto folding = fold([&sequencer.predictor](auto, auto element)
			{
				const auto isConforming = sequencer.predictor(element);
				return std::make_tuple(not isConforming, not isConforming);
			}, true, sequencer.sequence);
			return std::get<0>(folding);
		}

		/// Length
		///
		/// The amount of conforming elements in the underlying sequence of \a sequencer will be
		/// computed.
		///
		friend constexpr size_t length (const bounded_filter & sequencer)
		{
			return fold([&sequencer.predictor](auto count, auto element)
			{
				return count + predictor(element) ? 1 : 0;
			}, size_t(0), sequencer.sequence);
		}

		/// Complete forward folding
		///
		/// All conforming elements in \a sequencer will be folded over \a value by \a combiner. The
		/// final value of the folding process will be returned. The traversal direction is front to
		/// back.
		///
		template <typename V, Callable<V, V, value_type> C>
		friend constexpr V fold (C combiner, V value, bounded_filter sequencer)
		{
			return fold([&sequencer.predictor](V value, auto element)
			{
				return sequencer.predictor(element) ? combiner(std::move(value), std::move(element)) : value;
			}, std::move(value), std::move(sequencer.sequence));
		}

		/// Partial forward folding
		///
		/// Initial conforming elements in \a sequencer will be folded over \a value by \a combiner. The
		/// final value of the folding process will be returned along with a sequencer for the succeeding
		/// elements. The traversal direction is front to back. The folding stops with the first false
		/// flag returned by \a combiner.
		///
		template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
		friend constexpr std::tuple<V, bounded_filter> fold (C combiner, V value, bounded_filter sequencer)
		{
			auto folding = fold([&sequencer.predictor, &combiner](auto value, auto element)
			{
				return sequencer.predictor(element) ? combiner(std::move(value), std::move(element)) : std::make_tuple(std::move(value), true);
			}, std::move(value), std::move(sequencer.sequence));
			return std::make_tuple(std::move(std::get<0>(folding)), bounded_filter(std::move(std::get<1>(folding)), sequencer.predictor));
		}

		/// Complete backward folding
		///
		/// All conforming elements in \a sequencer will be folded over \a value by \a combiner. The
		/// final value of the folding process will be returned. The traversal direction is back to
		/// front.
		///
		/// @note This method is only enabled for reversible bounded sequences.
		///
		template <typename V, Callable<V, V, value_type> C, typename=typename std::enable_if<ReversibleBoundedSequence<S>>::type>
		friend constexpr V fold_reverse (C combiner, V value, bounded_filter sequencer)
		{
			return fold_reverse([&sequencer.predictor](auto value, auto element)
			{
				return sequencer.predictor(element) ? combiner(std::move(value), std::move(element)) : value;
			}, std::move(value), std::move(sequencer.sequence));
		}

		/// Partial backward folding
		///
		/// Tailing conforming elements in \a sequencer will be folded over \a value by \a combiner. The
		/// final value of the folding process will be returned along with a sequencer for the preceeding
		/// elements. The traversal direction is back to front. The folding stops with the first false
		/// flag returned by \a combiner.
		///
		/// @note This method is only enabled for reversible bounded sequences.
		///
		template <typename V, Callable<std::tuple<V, bool>, V, value_type> C, typename=typename std::enable_if<ReversibleBoundedSequence<S>>::type>
		friend constexpr std::tuple<V, bounded_filter> fold_reverse (C combiner, V value, bounded_filter sequencer)
		{
			auto folding = fold_reverse([&sequencer.predictor, &combiner](auto value, auto element)
			{
				return sequencer.predictor(element) ? combiner(std::move(value), std::move(element)) : std::make_tuple(std::move(value), true);
			}, std::move(value), std::move(sequencer.sequence));
			return std::make_tuple(std::move(std::get<0>(folding)), bounded_filter(std::move(std::get<1>(folding)), sequencer.predictor));
		}

	};




	/// Unbounded filter sequencer
	///
	/// The unbounded filter sequencer takes only elements which are conform with a predictor.
	template <UnboundedSequence S, Callable<bool, sequence_type_t<S>> C>
	class unbounded_filter
	{

	private:

		S sequence;
		C predictor;

	public:

		/// underlying sequence type
		using value_type = sequence_type_t<S>;

		/// Attribute constructor
		///
		/// An unbounded filter sequencer will be constructed with the underlying \a sequence and the \a
		/// predictor.
		constexpr unbounded_filter (S sequence, C predictor)
			noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<C>::value)
			: sequence(std::move(sequence)), predictor(std::move(predictor))
		{}

		/// Prefix decomposition
		///
		/// The next conforming element in the underlying sequence will be returned along with a
		/// sequencer for the succeeding elements. If no element in the sequence is conforming with the
		/// predictor, the method will be trapped into an endless loop.
		///
		constexpr std::tuple<value_type, unbounded_filter> decompose () const
		{
			auto folding = fold([&predictor](auto, auto element)
			{
				return std::make_tuple(int(0), not predictor(element));
			}, int(0), sequence);
			auto decomposition = std::get<1>(folding).decompose();
			return std::make_tuple(std::move(std::get<0>(decomposition)), unbounded_filter(std::move(std::get<1>(decomposition)), predictor));
		}

		/// Partial folding
		///
		/// Initial conforming elements in \a sequencer will be folded over \a value by \a combiner. The
		/// final value of the folding process will be returned along with a sequencer for the succeeding
		/// elements. The traversal direction is front to back. The folding stops with the first false
		/// flag returned by \a combiner.
		///
		template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
		friend constexpr std::tuple<V, unbounded_filter> fold (C combiner, V value, unbounded_filter sequencer)
		{
			auto folding = fold([&sequencer.predictor](auto value, auto element)
			{
				return sequencer.predictor(element) ? combiner(std::move(value), std::move(element)) : std::make_tuple(std::move(value), true);
			}, std::move(value), std::move(sequencer.sequence);
			return std::make_tuple(std::move(std::get<0>(folding)), unbounded_filter(std::move(std::get<1>(folding)), sequencer.predictor));
		}

	};




	

}

#endif
