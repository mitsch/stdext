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
			auto decomposition = decompose(sequence);
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
			decomposition = decompose(sequence);
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
			auto decomposition = decompose(sequence);
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
			auto decomposition = decompose_reverse(sequence);
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
			auto decomposition = decompose_reverse(sequence);
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
		const auto decomposition = decompose(sequence);
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






	/// Bounded filter sequencer
	template <BoundedSequence S, Callable<bool, sequence_type_t<S>> C>
	class bounded_filter
	{
	
	protected:

		S sequence;
		C predictor;

	public:

		using value_type = sequence_type_t<S>;

		constexpr bounded_filter (S sequence, C predictor)
			noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<C>::value)
			: sequence(std::move(sequence)), predictor(std::move(predictor))
		{}

		constexpr optional<std::tuple<value_type, bounded_filter>> decompose () const
		{
			using U = optional<std::tuple<value_type, bounded_filter>>;
			auto folding = sequence.fold([&predictor](auto result, auto element)
			{
				const auto isFound = not result.empty();
				if (not isFound and predictor(element))
				{
					result = std::move(element);
				}
				return std::make_tuple(std::move(result), isFound);
			}, U());
			return std::make_tuple(std::move(std::get<0>(folding)), bounded_filter(std::move(std::get<1>(folding)), predictor));
		}

		constexpr bool empty () const
		{
			const auto folding = sequence.fold([&predictor](auto, auto element)
			{
				const auto isPredicted = predictor(element);
				return std::make_tuple(isPredicted, isPredicted);
			}, false);
			return not std::get<0>(folding);
		}

		constexpr size_t length () const
		{
			const auto count = sequence.fold([&predictor](auto count, auto element)
			{
				return count + predictor(element) ? 1 : 0;
			}, size_t(0));
			return count;
		}

		template <typename V, Callable<V, V, value_type> D>
		constexpr V fold (D combiner, V value) const
		{
			return sequence.fold([&predictor, &combiner](auto value, auto element)
			{
				return predictor(element) ? combiner(std::move(value), std::move(element)) : std::move(value);
			}, std::move(value));
		}

		template <typename V, Callable<tuple<V, bool>, V, value_type> C>
		constexpr std::tuple<V, bounded_filter, bounded_filter> fold (D combiner, V value) const
		{
			auto folding = sequence.fold([&predictor, &combiner](auto value, auto element)
			{
				return predictor(element) ? combiner(std::move(value), std::move(element)) : std::make_tuple(std::move(value), true);
			}, std::move(value));
			const auto pre = bounded_filter(std::move(std::get<1>(folding)), predictor);
			const auto post = bounded_filter(std::move(std::get<2>(folding)), predictor);
			return std::make_tuple(std::move(std::get<0>(folding)), pre, post);
		}
	
	};



	template <ReversibleBoundedSequence S, Callable<bool, sequence_type_t<S>> P>
	class reversible_bounded_filter : public bounded_filter<S, P>
	{
	
	public:

		constexpr reversible_bounded_filter (S sequence, P predictor)
			noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<P>::value)
			: bounded_filter(std::move(sequence), std::move(predictor))
		{}

		constexpr reversible_bounded_filter (bounded_filter sequencer)
			noexcept(std::is_nothrow_move_constructible<bounded_filter>::value)
			: bounded_filter(std::move(sequencer))
		{}

		template <typename V, Callable<std::tuple<V, true>, V, value_type> C>
		constexpr std::tuple<V, reversible_bounded_filter, reversible_bounded_filter> fold (C combiner, V value) const
		{
			auto folding = bounded_filter::fold(std::move(combiner), std::move(value));
			const auto pre = reversible_bounded_filter(std::move(std:get<1>(folding)));
			const auto post = reversible_bounded_filter(std::move(std::get<2>(folding)));
			return std::make_tuple(std::move(std::get<0>(folding)), pre, post);
		}

		template <typename V, Callable<V, V, value_type> C>
		constexpr V fold_reverse (C combiner, V value) const
		{
			return sequence.fold_reverse([&combiner, &predictor](auto value, auto element)
			{
				return predictor(element) ? combiner(std::move(value), std::move(element)) : std::move(value);
			}, std::move(value));
		}

		template <typename V, Callable<std::tuple<V, bool>, V, value_type > C>
		constexpr std::tuple<V, reversible_bounded_filter, reversible_bounded_filter> fold_reverse (C combiner, V value) const
		{
			auto folding = sequence.fold_reverse([&combiner, &predictor](auto value, auto element)
			{
				return predictor(element) ? combiner(std::move(value), std::move(element)) : std::make_tuple(std::move(value), true);
			}, std::move(value));
			const auto pre = reversible_bounded_filter(std::move(std::get<1>(folding)), predictor);
			const auto post = reversible_bounded_filter(std::move(std::get<2>(folding)), predictor);
			return std::make_tuple(std::move(std::get<0>(folding)), pre, post);
		}
	
	};

	
	template <UnboundedSequence S, Callable<bool, sequence_type_t<S>> P>
	class unbounded_filter
	{
	
	private:

		S sequence;
		P predictor;

	public:

		using value_type = sequence_type_t<S>;

		constexpr unbounded_filter (S sequence, P predictor)
			noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<P>::value)
			: sequence(std::move(sequence)), predictor(std::move(predictor))
		{}

		constexpr std::tuple<value_type, unbounded_filter> decompose () const
		{
			S sequence = this->sequence;
			auto decomposition = sequence.decompose();
			while (not predictor(std::get<0>(decomposition)))
			{
				sequence = std::move(std::get<1>(decomposition));
			}
			return std::make_tuple(std::move(std::get<0>(decomposition)), unbounded_filter(std::move(std::get<1>(decomposition)), predictor));
		}

		template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
		constexpr std::tuple<V, unbounded_filter> fold (C combiner, V value) const
		{
			auto folding = sequence.fold([&predictor, &combiner](auto value, auto element)
			{
				return predictor(element) ? combiner(std::move(value), std::move(element)) : std::make_tuple(std::move(value), true);
			}, std::move(value));
			return std::make_tuple(std::move(std::get<0>(folding)), unbounded_filter(std::move(std::get<1>(folding)), predictor));
		}
	
	};






	template <BoundedSequence S, Callable_<sequence_type_t<S>> T>
	class bounded_transformer
	{
	
	protected:

		S sequence;
		T transformer;

	public:

		using value_type = result_of_t<T(sequence_type_t<S>)>;

		constexpr bounded_transformer (S sequence, T transformer)
			noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<T>::value)
			: sequence(std::move(sequence)), transformer(std::move(transformer))
		{}

		constexpr bool empty () const
		{
			return sequence.empty();
		}

		constexpr size_t length () const
		{
			return sequence.length();
		}

		constexpr optional<std::tuple<value_type, bounded_transformer>> decompose () const
		{
			return fmap([&transformer](auto decomposition)
			{
				return std::make_tuple(transformer(std::move(std::get<0>(decomposition))), bounded_transformer(std::move(std::get<1>(decomposition)), transformer))
			}, sequence.decompose());
		}

		template <typename V, Callable<V, V, value_type> C>
		constexpr V fold (C combiner, V value) const
		{
			return sequence.fold([&transformer, &combiner](auto value, auto element)
			{
				return combiner(std::move(value), transformer(std::move(element)));
			}, std::move(value));
		}

		template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
		constexpr std::tuple<V, bounded_transformer, bounded_transformer> fold (C combiner, V value) const
		{
			auto folding = sequence.fold([&transformer, &combiner](auto value, auto element)
			{
				return combiner(std::move(value), transformer(std::move(element)));
			}, std::move(value));
			const auto pre = bounded_transformer(std::move(std::get<1>(folding)), transformer);
			const auto post = bounded_transformer(std::move(std::get<2>(folding)), transformer);
			return std::make_tuple(std::move(std::get<0>(folding)), pre, post);
		}

	};

	template <ReversibleBoundedSequence S, Callable_<sequence_type_t<S>> T>
	class reversible_bounded_transformer : public bounded_transformer<S, T>
	{
	
	public:

		constexpr reversible_bounded_transformer (S sequence, T transformer)
			noexcept(std::is_nothrow_constructible<bounded_transformer, S, T>::value)
			: bounded_transformer(std::move(sequence), std::move(transformer))
		{}

		template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
		constexpr std::tuple<V, reversible_bounded_transformer, reversible_bounded_transformer> fold (C combiner, V value) const
		{
			auto folding = bounded_transformer::fold(std::move(combiner), std::move(value));
			const auto pre = reversible_bounded_transformer(std::move(std::get<1>(folding)), transformer);
			const auto post = reversible_bounded_transformer(std::move(std::get<2>(folding)), transformer);
			return std::make_tuple(std::move(std::get<0>(folding)), pre, post);
		}

		template <typename V, Callable<V, V, value_type> C>
		constexpr V fold_reverse (C combiner, V value) const
		{
			return sequence.fold_reverse([&transformer, &combiner](auto value, auto element)
			{
				return combiner(std::move(value), transformer(std::move(element)));
			}, std::move(value));
		}

		template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
		constexpr std::tuple<V, reversible_bounded_transformer, reversible_bounded_transformer> fold_reverse (C combiner, V value) const
		{
			auto folding = sequence.fold_reverse([&transformer, &combiner](auto value, auto element)
			{
				return combiner(std::move(value), transformer(std::move(element)));
			}, std::move(value));
			const auto pre = reversible_bounded_transformer(std::move(std::get<1>(folding)), transformer);
			const auto post = reversible_bounded_transformer(std::move(std::get<2>(folding)), transformer);
			return std::make_tuple(std::move(std::get<0>(folding)), pre, post);
		}
	
	};

	template <UnboundedSequence S, Callable_<sequence_type_t<S>> T>
	class unbounded_transformer
	{
	
	private:

		S sequence;
		T transformer;

	public:

		using value_type = result_of_t<T(sequence_type_t<S>)>;

		constexpr unbounded_transformer (S sequence, T transformer)
			noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<T>::value)
			: sequence(std::move(sequence)), transformer(std::move(transformer))
		{}

		constexpr std::tuple<value_type, unbounded_transformer> decompose () const
		{
			auto decomposition = sequence.decompose();
			return std::make_tuple(transformer(std::move(std::get<0>(decomposition))), unbounded_transformer(std::move(std::get<1>(decomposition)), transformer));
		}

		template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
		constexpr std::tuple<V, unbounded_transformer> fold (C combiner, V value) const
		{
			auto folding = sequence.fold([&transformer, &combiner](auto value, auto element)
			{
				return combiner(std::move(value), transformer(std::move(element)));
			}, std::move(value));
			return std::make_tuple(std::move(std::get<0>(folding)), unbounded_transformer(std::move(std::get<1>(folding)), transformer));
		}
	
	};






	template <BoundedSequence S>
	class bounded_ntaker
	{
	
	protected:

		S sequence;
		size_t count;
		bool truncated;

		constexpr std::tuple<S, size_t> truncate () const
		{
			auto folding = sequence.fold([limit=count](auto count, auto)
			{
				const auto isInBound = count < limit;
				return std::make_tuple(count + isInBound ? 1 : 0, isInBound);
			}, size_t(0));
			return std::make_tuple(std::move(std::get<1>(folding)), std::get<0>(folding));
		}

	public:

		using value_type = sequence_type_t<S>;

		constexpr bounded_ntaker (S sequence, size_t count, bool truncated)
			noexcept(std::is_nothrow_move_constructible<S>::value)
			: sequence(std::move(sequence)), count(count), truncated(truncated)
		{}
	
		constexpr bool empty () const
		{
			return count == 0 or (not truncated and sequence.empty());
		}

		constexpr size_t length () const
		{
			if (count == 0 or truncated)
			{
				return count;
			}
			else
			{
				const auto sequenceLength = sequence.length();
				return count < sequenceLength ? count : sequenceLength;
			}
		}

		constexpr optional<std::tuple<value_type, bounded_ntaker>> decompose () const
		{
			using U = optional<std::tuple<value_type, bounded_ntaker>>;
			return count == 0 ? U() : fmap([count, &truncated](auto decomposition)
			{
				auto remainings = bounded_ntaker(std::move(std::get<0>(decomposition)), count - 1, truncated);
				return std::make_tuple(std::move(std::get<0>(decomposition)), std::move(remainings));
			}, sequence.decompose());
		}

		template <typename V, Callable<V, V, value_type> C>
		constexpr V fold (C combiner, V value) const
		{
			if (truncated)
			{
				return sequence.fold(std::move(combiner), std::move(value));
			}
			else
			{
				auto count = this->count;
				auto folding = sequence.fold([&count](auto value, auto element)
				{
					return count-- > 0 ? combiner(std::move(value), std::move(element)) : std::make_tuple(std::move(value), false);
				}, std::move(value));
				return std::get<0>(folding);
			}
		}

		template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
		constexpr std::tuple<V, bounded_ntaker, S> fold (C combiner, V value) const
		{
			if (truncated)
			{
				auto count = size_t(0);
				auto folding = sequence.fold([&count, &combiner](auto value, auto element)
				{
					const auto combination = combiner(std::move(value), std::move(element));
					if (std::get<1>(combination)) ++count;
					return combination;
				}, std::move(value));
				auto unfolded = bounded_ntaker(std::move(std::get<1>(folding)), true);
				return std::make_tuple(std::move(std::get<0>(folding)), std::move(unfolded), std::move(std::get<2>(folding)));
			}
			else
			{
				auto count = this->count;
				auto folding = sequence.fold([&count](auto value, auto element)
				{
					if (count > 0)
					{
						const auto combination = combiner(std::move(value), std::move(element));
						if (std::get<1>(combination)) --count;
						return combination;
					}
					else
					{
						std::make_tuple(std::move(value), false);
					}
				}, std::move(value));
				auto unfolded = bounded_ntaker(std::move(std::move(std::get<1>(folding))), count, false);
				return std::make_tuple(std::move(std::get<0>(folding)), std::move(unfolded), std::move(std::get<2>(folding)));
			}
		}

	};

	template <ReversibleBoundedSequence S>
	class reversible_bounded_ntaker : public bounded_ntaker<S>
	{

	public:

		constexpr reversible_bounded_ntaker (S sequence, size_t count, bool truncated)
			noexcept(std::is_nothrow_move_constructible<S>::value)
			: bounded_ntaker(std::move(sequence), count, truncated)
		{}

		constexpr reversible_bounded_ntaker (bounded_ntaker<S> sequencer)
			noexcept(std::is_nothrow_move_constructible<S>::value)
			: bounded_ntaker<S>(std::move(sequencer))
		{}

		constexpr optional<std::tuple<value_type, reversible_bounded_ntaker>> decompose () const
		{
			return fmap([](auto decomposition)
			{
				auto remainings = reversible_bounded_ntaker(std::move(std::get<1>(decomposition)));
				return std::make_tuple(std::move(std::get<0>(decomposition)), std::move(remainings));
			}, bounded_ntaker::decompose());
		}

		constexpr optional<std::tuple<value_type, reversible_bounded_ntaker>> decompose_reverse () const
		{
			const auto truncation = truncated ? std::tie(sequence, count) : truncate();
			return fmap([count=std::get<1>(trunction)](auto decomposition)
			{
				auto remainings = reversible_bounded_ntaker(std::move(std::get<1>(decomposition)), count - 1, true);
				return std::make_tuple(std::move(std::get<0>(decomposition)), std::move(remainings));
			}, std::get<0>(trunction).decompose_reverse());
		}

		template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
		constexpr std::tuple<V, reversible_bounded_ntaker, S> fold (C combiner, V value) const
		{
			auto folding = bounded_ntaker::fold(std::move(combiner), std::move(value));
			auto unfolded = reversible_bounded_ntaker(std::move(std::get<1>(folding)));
			return std::make_tuple(std::move(std::get<0>(folding)), std::move(unfolded), std::move(std::get<2>(folding)));
		}

		template <typename V, Callable<V, V, value_type> C>
		constexpr V fold_reverse (C combiner, V value) const
		{
			const auto truncation = truncated ? std::tie(sequence, count) : truncate();
			return std::get<0>(truncation).fold_reverse(std::move(combiner), std::move(value));
		}

		template <typename V, Calalble<std::tuple<V, bool>, V, value_type> C>
		constexpr std::tuple<V, S, S> fold_reverse (C combiner, V value) const
		{
			const auto truncation = truncated ? std::tie(sequence, count) : truncate();
			return std::get<0>(truncation).fold_reverse(std::move(combiner), std::move(value));
		}
	
	};

	template <UnboundedSequence S>
	class unbounded_ntaker
	{
	
	private:

		S sequence;
		size_t count;

	public:

		using value_type = sequence_type_t<S>;

		constexpr unbounded_ntaker (S sequence, size_t count)
			noexcept(std::is_nothrow_move_constructible<S>::value)
			: sequence(std::move(sequence)), count(count)
		{}

		constexpr bool empty () const
		{
			return count == 0;
		}

		constexpr bool length () const
		{
			return count;
		}

		constexpr optional<std::tuple<value_type, unbounded_ntaker>> decompose () const
		{
			return make_optional(count > 0, [&]()
			{
				auto decomposition = sequence.decompose();
				auto remainings = unbounded_ntaker(std::move(std::get<1>(decomposition)), count - 1);
				return std::make_tuple(std::move(std::get<0>(decomposition)), std::move(remainings));
			});
		}

		template <typename V, Callable<V, V, value_type> C>
		constexpr V fold (C combiner, V value) const
		{
			auto count = this->count;
			const auto folding = sequence.fold([&combiner, &count](auto value, auto element)
			{
				if (count > 0) value = combiner(std::move(value), std::move(element));
				return std::make_tuple(std::move(value), count-- > 0);
			}, std::move(value));
			return std::get<0>(folding);
		}
		
		template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
		constexpr std::tuple<V, unbounded_ntaker, unbounded_ntaker> fold (C combiner, V value) const
		{
			auto count = this->count;
			auto folding = sequence.fold([&combiner, &count](auto value, auto element)
			{
				if (count > 0)
				{
					const auto combination = combiner(std::move(value), std:move(element));
					if (std::get<1>(combination)) --count;
					return combination;
				}
				else
				{
					return std::make_tuple(std::move(value), false);
				}
			}, std::move(value));
			auto unfolded = unbounded_ntaker(std::move(std::get<1>(folding)), this->count - count);
			auto folded = unbounded_ntaker(std::move(std::get<2>(folding)), count);
			return std::make_tuple(std::move(std::get<0>(folding)), std::move(unfolded), std::move(folded));
		}
	
	};






	template <Sequence S, Callable<bool, sequence_type_t<S>> P>
	class bounded_taker
	{
	
	protected:

		S sequence;
		P predictor;

	public:

		using value_type = sequence_type_t<S>;

		constexpr bounded_taker (S sequence, P predictor)
			noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<P>::value)
			: sequence(std::move(sequence)), predictor(std::move(predictor))
		{}

		constexpr bool empty () const
		{
			return decide([&predictor](auto decomposition)
			{
				return not predictor(std::get<0>(decomposition));
			}, true_type(), sequence.decompose());
		}

		constexpr size_t length () const
		{
			return sequence.fold([&predictor](auto count, auto element)
			{
				const auto isConform = predictor(element);
				return std::make_tuple(count + isConform ? 1 : 0, isConform);
			}, size_t(0));
		}

		constexpr optional<value_type, bounded_taker> decompose () const
		{
			return mbind([&predictor](auto decomposition)
			{
				return make_optional(predictor(std::get<0>(decomposition), [&predictor](auto head, auto tails)
				{
					return std::make_tuple(std::move(head), bounded_taker(std::move(tails), predictor));
				}, std::move(std::get<0>(decomposition)), std::move(std::get<1>(decomposition)));
			}, sequence.decompose());
		}

		template <typename V, Callable<V, V, value_type> C>
		constexpr V fold (C combiner, V value) const
		{
			auto folding = sequence.fold([&predictor, &combiner](auto value, auto element)
			{
				return predictor(element) ? std::make_tuple(combiner(std::move(value), std::move(element)), true) : std::make_tuple(std::move(value), false);
			}, std::move(value));
			return std::get<0>(folding);
		}

		template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
		constexpr std::tuple<V, bounded_taker, bounded_ntaker> fold (C combiner, V value) const
		{
			auto count = size_t(0);
			auto folding = sequence.fold([&predictor, &combiner, &count](auto value, auto element)
			{
				if (predictor(element))
				{
					const auto combination = combiner(std::move(value), std::move(element));
					if (std::get<1>(combination)) ++count;
					return combination;
				}
				else
				{
					return std::make_tuple(std::move(value), false);
				}
			}, std::move(value));
			const auto folded = bounded_ntaker(std::move(std::get<2>(folding)), predictor);
			const auto unfolded = bounded_taker(std::move(std::get<1>(folding)), predictor);
			return std::make_tuple(std::move(std::get<0>(folding)), unfolded, folded);
		}

	};

	template <ReversibleboundedSequence S, Callable<bool, sequence_type_t<S>> P>
	cĺass reversible_bounded_taker : public bounded_taker<S, P>
	{

	private:

		bool truncated;

		constexpr S truncate () const
		{
			const auto folding = sequence.fold([&predictor](auto, auto element)
			{
				return std::make_tuple(int(0), predictor(element));
			}, int(0));
			return std::get<1>(folding);
		}

	public:

		constexpr reversible_bounded_taker (S sequence, P predictor, bool truncated = false)
			noexcept(std::is_nothrow_constructible<bounded_taker, S, P>::value)
			: bounded_taker(std::move(sequence), std::move(predictor)), truncated(truncated)
		{}

		constexpr reversible_bounded_taker (bounded_taker sequencer, bool truncated)
			noexcept(std::is_nothrow_move_constructible<bounded_taker>::value)
			: bounded_taker(std::move(sequencer)), truncated(truncated)
		{}

		constexpr size_t length () const
		{
			return truncated ? sequence.length() : bounded_taker::length();
		}

		constexpr optional<std::tuple<value_type, reversible_bounded_taker>> decompose () const
		{
			auto decomposition = bounded_taker::decompose();
			auto remainings = reversible_bounded_taker(std::move(std::get<1>(decomposition)), truncated);
			return std::make_tuple(std::move(std::get<0>(decomposition)), std::move(remainings));
		}

		constexpr optional<std::tuple<value_type, reversible_bounded_taker>> decompose_reverse () const
		{
			const auto truncating = truncated ? truncate() : sequence;
			return fmap([&predictor](auto decomposition)
			{
				auto remainings = reversible_bounded_taker(std::move(std::get<1>(decomposition)), predictor, true);
				return std::make_tuple(std::move(std::get<0>(decomposition)), std::move(remainings));
			}, truncating.decompose_reverse());
		}

		template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
		constexpr std::tuple<V, reversible_bounded_taker, reversible_bounded_taker> fold (C combiner, V value) const
		{
			auto folding = bounded_taker::fold(std::move(combiner), std::move(value));
			const auto folded = reversible_bounded_taker(std::move(std::get<1>(folding)), true);
			const auto unfolded = reversible_bounded_taker(std::move(std::get<2>(folding)), truncated);
			return std::make_tuple(std::move(std::get<0>(folding)), folded, unfolded);
		}

		template <typename V, Callable<V, V, value_type> C>
		constexpr V fold_reverse (C combiner, V value) const
		{
			const auto truncating = truncated ? sequence : truncate();
			return truncating.fold_reverse([&combiner](auto value, auto element)
			{
				return combiner(std::move(value), std::move(element));
			}, std::move(value));
		}
		
		template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
		constexpr std:tuple<V, reversible_bounded_taker, reversible_bounded_taker> fold_reverse (C combiner, V value) const
		{
			const auto truncating = truncated ? sequence : truncate();
			auto folding = truncating.fold_reverse([&combiner](auto value, auto element)
			{
				return combiner(std::move(value), std::move(element));
			}, std::move(value));
			const auto folded = reversible_bounded_taker(std::move(std::get<1>(folding)), predictor, true);
			const auto unfolded = reversible_bounded_taker(std::move(std::get<2>(folding)), predictor, true);
			return std::make_tuple(std::move(std::get<0>(folding)), folded, unfolded);
		}

	};

	template <UnboundedSequene S, Callable<bool, sequence_type_t<S>> P>
	class unbounded_taker
	{
	
	private:

		S sequence;
		P predictor;

	public:

		using value_type = sequence_type_t<S>;

		constexpr unbounded_taker (S sequence, P predictor)
			noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<P>::value)
			: sequence(std::move(sequence)), predictor(std::move(predictor))
		{}

		constexpr bool empty () const
		{
			auto decomposition = sequence.decompose();
			return not predictor(std::get<0>(decomposition));
		}

		constexpr size_t length () const
		{
			auto folding = sequence.fold([&predictor](auto count, auto element)
			{
				const auto isConform = predictor(element);
				return std::make_tuple(count + isConform ? 1 : 0, isConform);
			}, size_t(0));
			return std::get<0>(folding);
		}
	
		constexpr optional<std::tuple<value_type, unbounded_taker>> decompose () const
		{
			auto decomposition = sequence.decompose();
			return make_optional(predictor(std::get<0>(decomposition)), [&predictor](auto head, auto tails)
			{
				return std::make_tuple(std::move(head), unbounded_taker(std::move(tails), predictor));
			}, std::move(std::get<0>(decomposition)), std::move(std::get<1>(decomposition)));
		}

		template <typename V, Callable<V, V, value_type> C>
		constexpr V fold (C combiner, V value) const
		{
			return sequence.fold([&combiner, &predictor](auto value, auto element)
			{
				return predictor(element) ? std::make_tuple(combiner(std::move(value), std::move(element)), true) : std::make_tuple(std::move(value), false);
			}, std::move(value));
		}

		template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
		constexpr std::tuple<V, unbounded_taker, unbounded_taker> fold (C combiner, V value) const
		{
			auto folding = sequence.fold([&predictor, &combiner](auto value, auto element)
			{
				return predictor(element) ? combiner(std::move(value), std::move(element)) : std::make_tuple(std::move(value), false);
			}, std::move(value));
			auto folded = unbounded_taker();
			auto unfolded = unbounded_taker(std::move(std::get<1>(folding)), predictor);
			return std::make_tuple(std::move(std::get<0>(folding)), std::move(unfolded), std::move(folded));
		}

	};





	template <BoundedSequence S, Callable<bool, sequence_type_t<S>> P>
	class bounded_dropper
	{
	
	protected:

		S sequence;
		P predictor;
		bool truncated;

		constexpr S truncate () const
		{
			auto folding = sequence.fold([&predictor](auto, auto element)
			{
				return std::make_tuple(int(0), predictor(element));
			}, int(0));
			return std::get<2>(folding);
		}

	public:

		using value_type = sequence_type_t<S>;

		constexpr bounded_dropper (S sequence, P predictor, bool truncated = false)
			noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<P>::value)
			: sequence(std::move(sequence)), predictor(std::move(predictor)), truncated(truncated)
		{}

		constexpr bool empty () const
		{
			return truncated ? sequence.empty() : truncate().empty();
		}

		constexpr size_t length () const
		{
			return truncated ? sequence.length() : truncate().length();
		}

		constexpr optional<std::tuple<value_type, bounded_dropper>> decompose () const
		{
			const auto sequence = truncated ? this->sentence : truncate();
			return fmap([&predictor](auto decomposition)
			{
				auto remainings = bounded_dropper(std::move(std::get<1>(decomposition)), predictor, true);
				return std::make_tuple(std::move(std::get<0>(decomposition)), std::move(remainings));
			}, sequence.decompose())
		}

		template <typename V, Callable<V, V, value_type> C>
		constexpr V fold (C combiner, V value) const
		{
			const auto sequence = truncated ? this->sequence : truncate();
			return sequence.fold(std::move(combiner), std::move(value));
		}

		template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
		constexpr std::tuple<V, bounded_dropper, bounded_dropper> fold (C combiner, V value) const
		{
			const auto sequence = truncated ? this->sequence : truncate();
			return sequence.fold(std::move(combiner), std::move(value));
		}
	
	};

	template <ReversibleBoundedSequence S, Callable<bool, sequence_type_t<S>> P>
	class reversible_bounded_dropper : public bounded_dropper<S, P>
	{
	
	private:

		constexpr reversible_bounded_dropper (bounded_dropper<S, P> sequencer)
			noexcept(std::is_nothrow_move_constructible<bounded_dropper<S, p>>::value)
			: bounded_dropper<S, P>(std::move(sequencer))
		{}
	
		constexpr optional<std::tuple<value_type, reversible_bounded_dropper>> decompose () const
		{
			auto decomposition = bounded_dropper::decompose();
			return fmap([](auto decomposition)
			{
				auto remainings = reversible_bounded_dropper(std::move(std::get<1>(decomposition)));
				return std::make_tuple(std::move(std::get<0>(decomposition)), std::move(remainings));
			}, std::move(decomposition));
		}

		constexpr optional<std::tuple<value_type, reversible_bounded_dropper>> decompose_reverse () const
		{
			const auto sequence = truncated ? this->sequence : truncate();
			return fmap([&predictor](auto decomposition)
			{
				auto remainings = reversible_bounded_dropper(std::move(std::get<1>(decomposition)));
				return std::make_tuple(std::move(std::get<0>(decomposition)), std::move(remainings));
			}, sequence.decompose_reverse());
		}

		template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
		constexpr std::tuple<V, reversible_bounded_dropper, reversible_bounded_dropper> fold (C combiner, V value) const
		{
			auto folding = bounded_dropper::fold(std::move(combiner), std::move(value));
			auto folded = reversible_bounded_dropper(std::move(std::get<1>(folding)));
			auto unfolded = reversible_bounded_dropper(std::move(std::get<2>(folding)));
			return std::make_tuple(std::move(std::get<0>(folding)), std:move(folded), std::move(unfolded));
		}

		template <typename V, Callable<V, V, value_type> C>
		constexpr V fold_reverse (C combiner, V value) const
		{
			const auto sequence = truncated ? this->sequence : truncate();
			return sequence.fold_reverse(std::move(combiner), std::move(value));
		}

		template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
		constexpr std::tuple<V, reversible_bounded_dropper, reversible_bounded_dropper> fold_reverse (C combiner, V value) const
		{
			const auto sequence = truncated ? this->sequence : truncate();
			auto folding = sequence.fold_reverse(std::move(combiner), std::move(value));
			auto folded = reversible_bounded_dropper(std::move(std::get<1>(folding)));
			auto unfolded = reversible_bounded_dropper(std::move(std::get<2>(folding)));
			return std::make_tuple(std::move(std::get<0>(folding)), std:move(folded), std::move(unfolded));
		}

	};

	template <UnboundedSequence S, Callable<bool, sequence_type_t<S>> P>
	class unbounded_dropper
	{
	
	private:

		S sequence;
		P predictor;
		bool truncated;

		constexpr S truncate () const
		{
			const auto folding = sequence.fold([&predictor](auto, auto element)
			{
				return std::make_tuple(int(0), predictor(element));
			}, int(0));
			return std::get<2>(folding);
		}

	public:
	
		using value_type = sequence_type_t<S>;

		constexpr unbounded_dropper (S sequence, P predictor, bool truncated = false)
			noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<P>::value)
			: sequence(std::move(sequence)), predictor(std::move(predictor)), truncated(truncated)
		{}

		constexpr std::tuple<value_type, unbounded_dropper> decompose () const
		{
			const auto sequence = truncated ? this->sequence : truncate();
			auto decomposition = sequence.decompose();
			auto remainings = unbounded_dropper(std::move(std::get<1>(decomposition)), predictor, true);
			return std::make_tuple(std::move(std::get<0>(decomposition)), std::move(remainings));
		}

		template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
		constexpr std::tuple<V, unbounded_dropper> fold (C combiner, V value) const
		{
			const auto sequence = truncated ? this->sequence : truncate();
			auto folding = sequence.fold(std::move(combiner), std::move(value));
			auto remainings = unbounded_dropper(std::move(std::get<1>(folding)), predictor, true);
			return std::make_tuple(std::move(std::get<0>(decomposition)), std::move(remainings));
		}

	};






	template <BoundedSequence S, typename T>
	class bounded_intersperser
	{
	
	private:

		S sequence;
		T character;
		bool nextCharacter;

	public:

		using value_type = std::common_type_t<sequence_type_t<S>, T>;

		constexpr bounded_intersperser (S sequence, T character, bool nextCharacter = false)
			noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<T>::value)
			: sequence(std::move(sequence)), character(std::move(character)), nextCharacter(nextCharacter)
		{}

		constexpr optional<std::tuple<value_type, bounded_intersperser>> decompose () const
		{
			if (nextCharacter and not sequence.empty())
			{
				
			}
			else if (
			{
				
			}
		}

		constexpr bool empty () const
		{
			return sequence.empty();
		}

		constexpr size_t length () const
		{
			const auto sequenceLength = sequence.length();
			return sequenceLength == 0 ? 0 : (2 * sequenceLength - nextCharacter ? 0 : 1);
		}

		template <typename V, Callable<V, V, value_type> C>
		constexpr V fold (C combiner, V value) const
		{
			auto sequence = this->sequence;
			if (not nextCharacter)
			{
				std::tie(value, sequence) = decide([&combiner](auto decomposition, auto value, auto sequence)
				{
					value = combiner(std::move(value), std::move(std::get<0>(decomposition)));
					return std::make_tuple(std::move(value), std::move(std::get<1>(decomposition)), std::move(sequence));
				}, [](auto value, auto sequence)
				{
					return std::make_tuple(std::move(value), std::move(sequence));
				}, sequence.decompose(), std::move(value), sequence);
			}

			return sequence.fold([&combiner, &character](auto value, auto element)
			{
				value = combiner(std::move(value), character);
				return combiner(std::move(value), std::move(element));
			}, std::move(value));
		}

		template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
		constexpr std::tuple<V, bounded_>
	
	};


}

#endif
