/// @file concat_sequencer.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_CONCAT_SEQUENCER_HPP__
#define __STDEXT_CONCAT_SEQUENCER_HPP__

namespace stdext
{
	
	///	Concatenation sequencer for bounded sequence
	template <BoundedSequence ... Bs>
		requires sizeof...(Bs) >= 2 and
		         requires(){typename std::common_type<sequence_type_t<Bs> ...>::type;}
	class bounded_concat_sequencer
	{
	
		private:
	
			std::tuple<Bs ...> sequences;
	
		public:
	
			/// Type of the elements is the common type of all sequences' element type
			using value_type = typename std::common_type<sequence_type_t<Bs> ...>::type;
	
			/// Attribute constructor
			constexpr bounded_concat_sequencer (std::tuple<Bs ...> sequences)
				: sequences(std::move(sequences))
				noexcept(conjunction_v<std::is_nothrow_move_constructible<Bs>::value ...>)
			{}
	
			/// Tests if there is any element left in the concatenated sequences
			constexpr operator bool () const
			{
				constexpr auto indices = index_sequence_for<Bs ...>();
				auto combinator = [&sequences](auto value, auto index){return value or std::get<index>(sequences);};
				return indices.fold(std::move(combinator), false);
			}
	
			/// Decomposition
			constexpr optional<std::tuple<value_type, bounded_concat_sequencer>> operator () () const
			{
				using U = optional<std::tuple<value_type, bounded_concat_sequencer>>;
				constexpr auto indices = index_sequence_for<Bs ...>();
				auto folding = indices.fold([](auto values, auto index)
				{
					auto head = std::move(std::get<0>(values));
					auto sequences = std::move(std::get<1>(values));
					if (not head)
					{
						auto optDecomposition = std::get<index>(sequences)();
						std::tie(head, std::get<index>(sequences)) = decide([](auto decomposition, auto sequence)
						{
							auto head = std::move(std::get<0>(decomposition));
							auto tail = std::move(std::get<1>(decomposition));
							return std::make_tuple(make_optional(std::move(head)), std::move(tail));
						}, [](auto sequence)
						{
							return std::make_tuple(U(), std::move(sequence));
						}, std::move(optDecomposition), std::move(std::get<index>(sequences)));
					}
					return std::make_tuple(std::move(head), std::move(sequences));
				}, std::make_tuple(U(), sequences));
				return fmap([](auto head, auto sequences)
				{
					auto remainings = bounded_concat_sequencer(std::move(sequences));
					return std::make_tuple(std::move(head), std::move(remainings));
				}, std::move(std::get<0>(folding)), std::move(std::get<1>(folding)));
			}
	
			/// Folding all elements
			///
			/// All elements in \a sequencer will be folded by \a combiner starting with \a value. The
			/// folded value will be returned.
			template <typename V, Callable<V, V, value_type> C>
			friend constexpr V fold (C combiner, V value, bounded_concat_sequencer sequencer)
			{
				constexpr auto indices = index_sequence_for<Bs ...>();
				return indices.fold([combiner=std::move(combiner), &](auto value, auto inex)
				{
					return fold(combiner, std::move(value), std::move(std::get<index>(sequencer.sequences)))
				}, std::move(values));
			}
	
			/// Folding initial elements
			template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
			friend constexpr std::tuple<V, bounded_concat_sequencer sequencer> fold (C combiner, V value
				bounded_concat_sequencer sequencer)
			{
				constexpr auto indices = index_sequence_for<Bs ...>();
				auto folding = indices.fold([&combiner](auto values, auto index)
				{
					auto value = std::move(std::get<0>(values));
					auto sequences = std::move(std::get<1>(values));
					auto keepOn = std::get<2>(values);
					if (keepOn)
					{
						std::tie(value, std::get<index>(sequences)) = fold(combiner, std::move(value),
							std::move(std::get<index(sequences)));
						keepOn = not std::get<index>(sequences);
					}
					return std::make_tuple(std::move(value), std::move(), keepOn);
				}, std::make_tuple(std::move(value), std::move(sequencer.sequences), true);
				value = std::move(std::get<0>(folding));
				sequencer = bounded_concat_sequencer(std::move(std::get<1>(folding)));
				return std::make_tuple(std::move(value), std::move(sequencer));
			}

			/// Reversly folding all elements
			///
			/// All elements in \a sequencer are folded in reverse by \a combiner over \a value. The
			/// folded value will be returned.
			template <typename V, Callable<V, V, value_type> C>
			friend constexpr V fold_reverse (C combiner, V value, bounded_concat_sequencer sequencer)
			{
				const auto indices = revert(index_sequence_for<Bs ...>());
				auto & sequences = sequencer.sequences;
				return indices.fold([&combiner, &sequences](auto index, auto value)
				{
					return fold_reverse(combiner, std::move(value), std::move(std::get<index>(sequences)));
				}, std::move(value));
			}
	}
	
	
	/// Concatenation sequencer for unbounded sequence
	template <UnboundedSequence S, BoundedSequence ... Ss>
	class unbounded_concat_sequencer
	{
	
		protected:
	
			std::tuple<Ss ...> boundedSequences;
			S unboundedSequence;
	
		public:
	
			/// Type of the elements is the common type of all sequences' element types
			using value_type = typename std::common_type<sequence_type_t<S>, sequence_type_t<Ss> ...>::type;
	
			/// Attribute constructor
			constexpr unbounded_concat_sequencer (std::tuple<Ss ...> boundedSequences, S unboundedSequence)
				: boundedSequences(std::move(boundedSequences)), unboundedSequence(std::move(unboundedSequence))
				noexcept(conjunction_v<std::is_nothrow_move_constructible<Ss> ...> and
				                       std::is_nothrow_move_constructible<S>::value>)
			{}
	
			/// Decomposition
			constexpr std::tuple<value_type, unbounded_concat_sequencer> operator () () const
			{
				constexpr auto boundedIndices = index_sequence_for<Bs ...>();
				auto boundedFolding = boundedIndices.fold([](auto values, auto index)
				{
					auto head = std::move(std::get<0>(values));
					auto sequences = std::move(std::get<1>(values));
					if (not head)
					{
						auto optDecomposition = sequences();
						std::tie(head, std::get<index>(sequences)) = decide([](auto decomposition, auto sequences)
						{
							auto element = std::move(std::get<0>(decomposition));
							auto remainings = std::move(std::get<1>(decomposition));
							return std::make_tuple(make_optional(std::move(element)), std::move(remainings));
						}, [](auto sequences)
						{
							return std::make_tuple(optional<value_type>(), std::move(sequences));
						}, std::move(optDecomposition), std::move(std::get<index>(sequences)));
					}
					return std::make_tuple(std::move(head), std::move(tail));
				}, std::make_tuple(optional<value_type>(), boundedSequences));
				auto head = std::move(std::get<0>(boundedFolding));
				auto tail = std::move(std::get<1>(boundedFolding));
				return decide([&](auto head, auto boundedSequences,)
				{
					auto remainings = unbounded_concat_sequencer(std::move(boundedSequences), unboundedSequence);
					return std::make_tuple(std::move(head), std::move(remainings));
				}, [&](auto boundedSequences)
				{
					auto decomposition = unboundedSequence();
					auto head = std::move(std::get<0>(decomposition));
					auto remainings = unbounded_concat_sequencer(std::move(boundedSequences), std::move(std::get<1>(decomposition)));
					return std::make_tuple(std::move(head), std::move(remainings));
				}, std::move(head), std::move(tail));
			}
	
			/// Folding initial elements
			template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
			friend constexpr std::tuple<V, unbounded_concat_sequencer> fold (C combiner, V value,
				unbounded_concat_sequencer sequencer)
			{
				constexpr auto indices = index_sequence_for<Bs ...>();
				auto boundedFolding = indices.fold([&](auto values, auto index)
				{
					auto value = std::move(std::get<0>(values));
					auto sequences = std::move(std::get<1>(values));
					auto keepOn = std::get<2>(values);
					if (keepOn)
					{
						std::tie(value, std::get<index>(sequences) = fold(combiner, std::move(value), std::get<index>(sequences));
						keepOn = not std::get<index>(sequences);
					}
					return std::make_tuple(std::move(value), std::move(sequences), keepOn);
				}, std::make_tuple(std::move(std::move(value)), std::move(sequencer.boundedSequences), true));
				value = std::move(std::get<0>(boundedFolding));
				auto boundedSequences = std::move(std::get<1>(boundedFolding));
				auto keepOn = std::get<2>(boundedFolding);
				auto unboundedSequence = std::move(sequencer.unboundedSequence);
				std::tie(value, unboundedSequence) = keepOn ?
					fold(std::move(combiner), std::move(value), std::move(unboundedSequence)) :
					std::make_tuple(std::move(value), std::move(unboundedSequence));
				auto remainings = unbounded_concat_sequencer(std::move(boundedSequences), std::move(unboundedSequence));
				return std::make_tuple(std::move(value), std::move(remainings);
			}
	};

}

#endif

