/// @file cycle_sequencer.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_CYCLE_SEQUENCER_HPP__
#define __STDEXT_CYCLE_SEQUENCER_HPP__

#include <stdext/sequence_concept.hpp>

namespace stdext
{

	/// Cycle sequencer for bounded sequence
	///
	/// A bounded sequence is infinitely repeated. As soon as the end of the sequence has been
	/// reached, the sequencer will start at the beginning of the sequence.
	template <BoundedSequence S>
	class cycle_sequencer
	{
	
		private:
	
			S original;
			S sequence;
	
		public:
	
			/// Type of the elements is the type of the held sequence's elements
			using value_type = sequence_type_t<S>;
	
			/// Attribute constructor
			constexpr cycle_sequencer (S original, S sequence)
				: original(std::move(original)), sequence(std::move(sequence))
				noexcept(std::is_nothrow_move_constructible<S>::value)
			{}
	
			/// Decomposition
			constexpr std::tuple<value_type, cycle_sequencer> operator () () const
			{
				auto decomposition = decide([](auto decomposition)
				{
					return decomposition;
				},
				[&original]()
				{
					using U = std::tuple<value_type, cycle_sequencer>;
					return decide([](auto decomposition){return decomposition;}, []()->U{assert(false);}, original());
				}, sequence());
				auto remainings = cycle_sequencer(original, std::move(std::get<1>(decomposition)));
				return std::make_tuple(std::move(std::get<0>(decomposition)), std::move(remainings));
			}
	
			/// Folding initial elements
			template <typename V, Callable<std::tuple<V, bool>, V, value_type>
			friend constexpr std:tuple<V, cycle_sequencer> fold (C combiner, V value, cycle_sequencer sequencer)
			{
				auto sequence = std::move(sequencer.sequence);
				auto keepOn = true;
				while (keepOn)
				{
					std::tie(value, sequence) = fold(combiner, std::move(value), std::move(sequence));
					keepOn = not sequence;
					if (keepOn) sequence = sequencer.original;
				}
				sequencer.sequence = std::move(sequence);
				return std::make_tuple(std::move(value), std::move(sequencer));
			}
	};

}

#endif

