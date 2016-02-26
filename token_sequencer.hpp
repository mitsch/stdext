/// @file token_sequencer.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_TOKEN_SEQUENCER_HPP__
#define __STDEXT_TOKEN_SEQUENCER_HPP__

#include <stdext/sequence_concept.hpp>
#include <stdext/callable.hpp>
#include <stdext/optional.hpp>
#include <tuple>
#include <utility>
#include <type_traits>

namespace stdext
{


	/// Token sequencer for bounded sequence
	template <BoundedSequene S, typename T, Callable<std::tuple<T, bool>, T, sequence_type_t<S>> C>
	class bounded_token_sequencer
	{
	
		private:

			C combiner;
			T init;
			optional<T> next;
			S sequence;

		public:

			/// Moves \a combiner, initial element value \a init, a potential \a next element and the \a
			/// sequence into the constructed object
			constexpr bounded_token_sequencer (C combiner, T init, T next, S sequence)
				: combiner(std::move(combiner)), init(std::move(init)), next(std::move(next)), sequence(std::move(sequence))
				noexcept(std::is_nothrow_move_constructible<C>::value and
				         std::is_nothrow_move_constructible<S>::value and
								 std::is_nothrow_move_constructible<T>::value)
			{}

			/// Tests if there is any token left in the sequencer
			constexpr operator bool () const
			{
				return next or sequence;
			}

			/// Decomposes sequence into the next token with a sequencer for the remaining elements or
			/// into an empty optional container if no token is left
			constexpr optional<std::tuple<T, bounded_token_sequencer>> operator () () const
			{
				return decide([&](auto element)
				{
					auto tail = bounded_token_sequencer(combiner, init, optional<T>(), sequence);
					return std::make_tuple(std::move(element), std::move(tail));
				}, [&]()
				{
					auto folding = fold(combiner, init, sequence);
					return make_optional(sequence, [&](auto folding)
					{
						auto head = std::move(std::get<0>(folding));
						auto tail = bounded_token_sequencer(combiner, init, optional<T>(), std::move(std::get<1>(folding)));
						return std::make_tuple(std::move(head), std::move(tail));
					}, std::move(folding));
				}, next);
			}


			/// Folds all tokens in the \a sequencer with \a combiner over \a value
			template <typename V, Callable<V, V, T> C>
			friend constexpr V fold (C combiner, V value, bounded_token_sequencer sequencer)
			{
				// First check on the optional next element
				value = decide([&](auto element, auto value)
				{
					return combiner(std::move(value), std::move(element));
				}, [](auto value)
				{
					return value;
				}, sequencer.next, std::move(value));

				// If there is any element in sequence, they will be folded
				if (sequencer.sequence)
				{
					auto folding = fold([&](auto values, auto element)
					{
						auto oValue = std::move(std::get<0>(values));
						auto iValue = std::move(std::get<1>(values));
					
						auto iCombination = sequencer.combiner(std::move(iValue), element);
						iValue = std::move(std::get<0>(iCombination));
						const auto keepOn = std::get<1>(iCombination);
					
						if (not keepOn)
						{
							oValue = combiner(std::move(oValue), std::(iValue));
							iValue = sequencer.combiner(sequencer.init, std::move(element));
						}
					
						return std::make_tuple(std::move(oValue), st::move(iValue));
					}, std::make_tuple(std::move(value), sequencer.inner), std::move(sequencer.sequence));
					value = std::move(std::get<0>(folding));
					value = combiner(std::move(value), std::move(std::get<1>(folding)));
				}

				return value;
			}


			/// Folds initial elements
			template <typename V, Callable<std::tuple<V, bool>, V, T> C>
			friend constexpr std::tuple<V, bounded_token_sequencer> fold (C combiner, V value, bounded_token_sequencer sequencer)
			{
				// Considers a potential next element
				auto nextDecision = decide([&](auto element, auto value)
				{
					return combiner(std::move(value), std::move(element));
				}, [](auto value)
				{
					return std::make_tuple(std::moe(value), true);
				}, sequencer.next, std::move(value));
				value = std::move(std::get<0>(nextDecision));
				const auto passedNext = std::get<1>(nextDecision);

				// if the next element is not accepted, we will return here
				if (not passedNext) return std::make_tuple(std::move(value), std::move(sequencer));

				auto folding = fold([&](auto values, auto element)
				{
					auto oValue = std::move(std::get<0>(values));
					auto iValue = std::move(std::get<1>(values));

					auto iCombination = sequencer.combiner(std::move(iValue), element);
					iValue = std::move(std::get<0>(iCombination));
					const auto keepOnInner = std::get<1>(iCombination);
					auto keepOn = keepOnInner;

					if (not keepOnInner)
					{
						auto oCombination = combiner(std::move(oValue), iValue);
						oValue = std::move(std::get<0>(oCombination));
						const auto keepOnOuter = std::get<1>(oCombination);
						keepOn = keepOnOuter;
						if (keepOnOuter) iValue = sequencer.combiner(sequencer.init, std::move(element));
					}

					return std::make_tuple(std::make_tuple(std::move(oValue), std::move(iValue)), keepOn);
				}, std::make_tuple(std::move(value), sequencer.init), std::move(sequencer.sequence));
				value = std::move(std::get<0>(std::get<0>(folding)));
				auto iValue = std::move(std::get<1>(std::get<0>(folding)));
				sequencer.sequence = std::move(std::get<1>(folding));

				auto postCombination = (not sequencer.sequence) ?
					combiner(std::move(value), std::move(iValue)) :
					std::make_tuple(std::move(value), false);
				if (std::get<1>(postCombination)) sequencer.sequence.clear();
				else sequencer.sequence = std::move(std:.get<0>(postCombination));

				return std::make_tuple(std::move(value), std::move(sequencer));
			}


			/// Computes the amount of tokens in the sequencer
			friend constexpr std::size_t length (const bounded_token_sequencer & sequencer)
			{
				auto folding = fold([&](auto values, auto element)
				{
					auto value = std::move(std::get<0>(values));
					auto count = std::move(std::get<1>(values));
					auto combination = sequencer.combiner(std::move(value), std::move(element));
					value = std::move(std::get<0>(combination));
					const auto keepOn = std::get<1>(combination);
					if (not keepOn)
					{
						value = sequencer.init;
						count++;
					}
					return std::make_tuple(std::move(value), std::move(count));
				}, std::size_t(0), sequencer.sequence);
				return std::get<1>(folding) + 1;
			}
	};




	/// Token sequencer for unbounded sequence
	template <UnboundedSequence S, typename T, Callable<std::tuple<T, bool>, T, sequence_type_t<S>> C>
	class unbounded_token_sequencer
	{
	
		private:

			C combiner;
			T init;
			optional<T> next;
			S sequence;

		public:
	
			/// Moves \a combiner, the initial combination value \a init, an optional \a next element and
			/// the \a sequence into the constructed object
			constexpr unbounded_token_sequencer (C combiner, T init, optional<T> next, S sequence)
				:combiner(std::move(combiner)), init(std::move(init)), next(std::move(next)), sequence(std::move(sequence))
				noexcept(std::is_nothrow_move_constructible<C>::value and
				         std::is_nothrow_move_constructible<T>::value and
								 std::is_nothrow_move_constructible<S>::value)
			{}

			/// Decomposes sequencer into the folding of the next group and a sequencer for the remaining
			/// elements to be grouped
			constexpr std::tuple<T, unbounded_token_sequencer> operator () () const
			{
				return decide([&](auto element)
				{
					auto tail = unbounded_token_sequencer(combiner, init, optional<T>(), sequence);
					return std::make_tuple(std::move(element), std::move(tail));
				}, [&]()
				{
					auto folding = fold(combiner, init, sequence);
					auto head = std::move(std::get<0>(folding));
					auto tail = unbounded_token_sequencer(combiner, init, optional<T>(), std::move(std::get<1>(folding)));
					return std::make_tuple(std::move(head), std::move(tail));
				}, next);
			}

			/// Folds initial elements
			template <typename V, Callable<std::tuple<V, bool>, V, T> C>
			friend constexpr std::tuple<V, unbounded_token_sequencer> fold (C combiner, V value,
				unbounded_token_sequencer sequencer)
			{
				// First, check if there is a next element from a potentially last folding
				auto nextDecision = decide([&](auto element, auto value)
				{
					return combiner(std::move(value), std::move(element));
				}, [](auto value)
				{
					return std::make_tuple(std::move(value), true);
				}, sequencer.next, std::move(value));
				value = std::move(std::get<0>(nextDecision));
				const auto passedNext = std::get<1>(nextDecision);
				if (not passedNext) return std::make_tuple(std::move(value), std::move(sequencer));

				// Go through sequence and fold inner value
				auto folding = fold([&](auto values, auto element)
				{
					auto oValue = std::move(std::get<0>(values));
					auto iValue = std::move(std::get<1>(values));
					
					auto iCombination = sequencer.combiner(std::move(iValue), element);
					iValue = std::move(std::get<0>(iCombination));
					const auto keepOnInner = std::get<1>(iCombination));
					auto keepOn = true;

					if (not keepOnInner)
					{
						auto oCombination = combiner(std::move(oValue), iValue);
						oValue = std::move(std::get<0>(oCombination));
						const auto keepOnOuter = std::get<1>(oCombination);
						keepOn = keepOnOuter;
						if (keepOnOuter) iValue = std::get<0>(sequencer.combiner(sequencer.init, std::move(element)));
					}
					return std::make_tuple(std::make_tuple(std::move(oValue), std::move(iValue)), keepOn);
				}, std::move(std::move(value), sequencer.init), std::move(sequencer.sequence));
				value = std::move(std::get<0>(std::get<0>(folding)));
				auto iValue = std::move(std::get<1>(std::get<0>(folding)));
				sequencer.sequence = std::move(std::get<1>(folding));

				// If the inner sequence is empty, we should fold it
				auto postCombination = (not sequencer.sequence) ?
					combiner(std::move(value), iValue) :
					std::make_tuple(std::move(value), false);
				value = std::move(std::get<0>(postCombination));
				if (std::get<1>(postCombination)) sequencer.next.clear();
				else sequencer.next = std::move(iValue);

				return std::make_tuple(std::move(value), std::move(sequencer));
			}
	};

}

#endif

