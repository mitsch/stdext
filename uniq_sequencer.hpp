/// @file uniq_sequencer.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_UNIQ_SEQUENCER_HPP__
#define __STDEXT_UNIQ_SEQUENCER_HPP__

#include <src/stdext/sequence_concept.hpp>

namespace stdext
{

	/// Uniq sequencer for bounded sequence
	template <BoundedSequence S, Callable<bool, const T&, const T&> C>
	class bounded_uniq_sequencer
	{
	
		private:

			C matcher;
			S sequence;
			bool isNextPulled;

		public:

			/// Element type is the element type of the held sequence
			using value_type = sequence_type_t<S>;

			/// Moves \a matcher and \a sequence into the constructed object and indicates by \a
			/// isNextPulledif the next element in the sequence has already been seen (true) or has not
			/// been seen (false)
			constexpr bounded_uniq_sequencer (C matcher, S sequence, bool isNextPulled)
				: matcher(std::move(matcher)), sequence(std::move(sequence)), isNextPulled(isNextPulled)
				noexcept(std::is_nothrow_move_constructible<C>::value and std::is_nothrow_move_constructible<S>::value)
			{}
	
			/// Decomposes sequencer into the next element which will be different from the previously
			/// decomposed element, and a sequencer for the remaining elements
			constexpr optional<std::tuple<value_type, bounded_uniq_sequencer>> operator () () const
			{
				return mbind([&](auto decomposition)
				{
					auto head = std::move(std::get<0>(decomposition));
					auto tail = std::move(std::get<1>(decomposition));

					if (isNextPulled)
					{
						auto folding = fold([&](auto value, auto element)
						{
							if (not value and not matcher(head, element))
								value = std::move(element);
							return std::make_tuple(std::move(value), not value);
						}, optional<value_type>(), std::move(tail));
						return fmap([&](auto head, auto tail)
						{
							auto remainings = bounded_uniq_sequencer(matcher, std::move(tail), true);
							return std::make_tuple(std::move(head), std::move(remainigs));
						}, std::move(std::get<0>(folding)), std::move(std::get<1>(folding)));
					}
					else
					{
						auto remainings = bounded_uniq_sequencer(matcher, sequence, true);
						return make_optional(std::make_tuple(std::move(head), std::move(remainings)));
					}
				}, sequence());
			}


			/// Folds all elements in \a sequence with \a combiner over \a value
			template <typename V, Callable<V, V, value_type> C>
			friend constexpr V fold (C combiner, V value, bounded_uniq_sequencer sequencer)
			{
				return decide([combiner=std::move(combiner)](auto decomposition, auto value)
				{
					auto head = std::move(std::get<0>(decomposition));
					auto tail = std::move(std::get<1>(decomposition));

					if (not sequencer.isNextPulled)
						value = combiner(std::move(value), head);

					return fold([&head, &matcher, combiner=std::move(combiner)](auto value, auto element)
					{
						if (not matcher(head, element))
						{
							value = combiner(std::move(value), element);
							head = std::move(element);
						}
						return value;
					});
				},
				[](auto value)
				{
					return value;
				}, sequencer(), std::move(value));
			}


			/// Folds initial elements
			template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
			friend constexpr std::tuple<V, bounded_uniq_sequencer> fold (C combiner, V value, bounded_uniq_sequencer sequencer)
			{
				
			}
	};

}

#endif

