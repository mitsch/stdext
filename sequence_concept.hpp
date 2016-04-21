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
			decompose(s);
		} and
		is_optional<decltype(decompose(std::declval<S>()))>::value and
		std::is_same<std::tuple_element<1, decltype(decompose(std::declval<S>()))::value_type>::type, S>::value;
	}

	/// Concept of a bounded sequence with value type \a T
	///
	/// Additionally to the constraints of being a bounded sequence, type \a S must produce elements
	/// of type \a T.
	template <typename S, typename T> concept bool BoundedSequence ()
	{
		return BoundedSequence<S> and
		       std::is_convertible<std::tuple_element<0, decltype(decompose(std::declval<S>()))::value_type>::type, T>::value;
	}


	/// Concept of a reversible bounded sequence
	///
	/// A reversible bounded sequence is a bounded sequence which can be folded from behind. It must
	/// implement the method fold_reverse both for complete folding and conditional folding.
	template <typename S> concept bool ReversibleBoundedSequence ()
	{
		return BoundedeSequence<S> and
		       requires(S s){decompose_reverse(s)} and
					 is_optional_v<decltype(decompose_reverse(std::declval<S>()))> and
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












	/// Prediction based take sequencer for bounded sequence
	///
	/// The prediction based take sequencer takes all initial elements of a bounded sequence which are
	/// conform with a predictor.
	template <BoundedSequence S, Callable<bool, sequence_type_t<S>> C>
	class bounded_taker
	{

	private:

		S sequence;
		C predictor;

	public:

		/// Sequence type of the underlying sequence
		using value_type = sequence_type_t<S>;

		/// Attribute constructor
		///
		/// The take sequencer will be constructed with the \a sequence and the \a predictor.
		constexpr bounded_taker (S sequence, C predictor)
			noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<C>::value)
			: sequence(std::move(sequence)), predictor(std::move(predictor))
		{}

		/// Prefix decomposition
		///
		/// If the sequence is not empty or the next element is not conform with the prediction, then an
		/// empty optional container will be returned. If the view is not empty and the next element is
		/// conform with the prediction, then it will be returned along with a sequencer for the remaining
		/// elements in the sequence.
		////
		constexpr optional<std::tuple<value_type, bounded_taker>> decompose () const
		{
			return mbind([&predictor](auto decomposition)
			{
				return make_optional(predictor(std::get<0>(demposition)), [&predictor](auto head, auto tails)
				{
					return std::make_tuple(std::move(head), bounded_taker(std::move(tails), predictor));
				}, std::move(std::get<0>(decomposition)), std::move(std::get<1>(decomposition)));
			}, sequence.decompose());
		}

		/// Sequencer getter
		///
		/// A constant reference of the held sequence is returned.
		constexpr const S& get_sequence () const
		{
			return sequence;
		}

		/// Predictor getter
		///
		/// A constant reference of the predictor is returned.
		constexpr const C& get_predictor () const
		{
			return predictor;
		}

	};

	/// Emptiness test of bounded take sequencer
	///
	/// A bounded take sequencer will be  empty, if either the held sequence is empty or the next
	/// element in the held sequence is not conform with the held predictor.
	///
	template <BoundedSequence S, Callable<bool, sequence_type_t<S>> C>
	constexpr bool empty (const bounded_taker<S, C> & sequencer)
	{
		const auto & sequence = sequencer.get_sequence();
		const auto & predictor = sequencer.get_predictor();
		return decide([&predictor](auto decomposition)
		{
			return not predictor(std::get<0>(decomposition));
		}, []()
		{
			return true;
		}, sequence.decompose());
	}

	/// Length of a bounded take sequencer
	///
	/// All initial conforming elements in the held sequence will be counted.
	///
	template <BoundedSequence S, Callable<bool, sequence_type_t<S>> C>
	constexpr size_t length (const bounded_taker<S, C> & sequencer)
	{
		const auto & predictor = sequencer.get_predictor();
		const auto folding = fold([&predictor](auto count, auto element)
		{
			const auto isConform = predictor(element);
			const auto nextCount = count + isConform ? 1 : 0;
			return std::make_tuple(nextCount, isConform);
		}, size_t(0), sequencer.get_sequence());
		return std::get<0>(folding);
	}

	/// Complete forward folding of a bounded take sequencer
	///
	/// All initial conforming elements in the held sequence will be folded over \a value by \a
	/// combiner. The final value will be returned. The elements of the held sequence will be
	/// traversed in forward direction.
	///
	template <BoundedSequence S, Callable<bool, sequence_type_t<S>> C, typename V, Callable<V, V, sequence_type_t<S>> D>
	constexpr V fold (D combiner, V value, bounded_taker<S, C> sequencer)
	{
		const auto & predictor = sequencer.get_predictor();
		const auto folding = fold([&predictor, &combiner](auto value, auto element)
		{
			return predictor(element) ? std::make_tuple(combiner(std::move(value), std::move(element)), true) : std::make_tuple(std::move(value), false);
		}, std::move(value), sequencer.get_sequence());
		return std::get<0>(folding);
	}

	/// Partial forward folding of a bounded take sequencer
	///
	/// Initial conforming elements in the held sequence will be folded over \a value by \a combiner.
	/// The final value will be returned. The elements of the held sequence will be traversed in
	/// forward direction. The traversion will stop with the first false flag returned from \a
	/// combiner.
	///
	template <BoundedSequence S, Callable<bool, sequence_type_t<S>> C, typename V, Callable<std::tuple<V, bool>, V, sequence_type_t<S>> D>
	constexpr std::tuple<V, S> fold (D combiner, V value, bounded_taker<S, C> sequencer)
	{
		const auto & predictor = sequencer.get_predictor();
		auto folding = fold([&predictor, &combiner](auto value, auto element)
		{
			return predictor(element) ? combiner(std::move(value), std::move(element)) : std::make_tuple(std::move(value), false);
		}, std::move(value), sequencer.get_sequence());
		return std::make_tuple(std::move(std::get<0>(folding)), bounded_taker(std::move(std::get<1>(folding)), predictor));
	}



	/// Take sequencer for unbounded sequences

	


	/// Prediction based bounded drop sequencer for bounded sequences
	///
	/// The prediction based bounded drop sequencer ignores initial elements of a held sequence as
	/// long as they are conform with a predictor.
	///
	template <BoundedSequence S, Callable<bool, sequence_type_t<S>> C>
	class bounded_dropper
	{
	
	private:

		S sequence;
		C predictor;
		bool dropped;

	public:

		/// Sequence type of held sequence
		using value_type = sequence_type_t<S>;

		/// Attribute constructor
		///
		/// The bounded take sequencer will be constructed with a \a sequence and a \a predictor as well
		/// with a flag indicating if dropping had already been performed.
		///
		constexpr bounded_dropper (S sequence, C predictor, bool dropped = false)
			noexcept(std::is_nothrow_move_constructible<S>::vaue and std::is_nothrow_move_constructible<C>::value)
			: sequence(std::move(sequence)), predictor(std::move(predictor)), dropped(dropped)
		{}

		/// Prefix decomposition
		///
		/// If dropping has not already been performed, all initial elements conforming with the
		/// predictor will be dropped first. If thhe view contains still some elements, the next
		/// element will be returned along with a sequence for the succeeding elements. If no elements
		/// are then left, an empty optional container will be returned.
		///
		constexpr optional<std::tuple<value_type, bounded_dropper>> decompose () const
		{
			auto mapping = [&predictor](auto decomposition)
			{
				return std::make_tuple(std::move(std::get<0>(decomposition)), bounded_taker(std::move(std::get<1>(decomposition)), predictor));
			};
			if (not dropped)
			{
				auto folding = fold([&predictor](auto, auto element)
				{
					return std::make_tuple(int(0), predictor(element));
				}, int(0), sequence);
				return fmap(mapping, std::get<0>(folding).decompose());
			}
			else
			{
				return fmap(mapping, sequence.decompose());
			}
		}

		/// Emptiness test
		///
		/// The 
	
	};



}

#endif
