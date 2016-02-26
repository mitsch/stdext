/// @file group_sequencer.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_GROUP_SEQUENCER_HPP__
#define __STDEXT_GROUP_SEQUENCER_HPP__

namespace stdext
{
	
	/// Group sequencer for bounded sequence
	///
	/// The sequencer groups contiguous elements based on an equivalence criterium of type \a C. The
	/// sequence which will be grouped is of type \a S.
	template <BoundedSequence S, Callable<bool, sequence_type_t<S>, sequence_type_t<S>> C>
	class bounded_group_sequencer
	{
	
		private:
	
			S elements;
			C matcher;
	
		public:
	
			/// Type of elements is a bounded sequence
			using value_type = bounded_ntake_sequencer<S>;
	
			/// Attribute constructor
			///
			/// The \a elements, an optional \a head and the \a matcher for the equivalence relation is
			/// moved into the constructed object.
			constexpr bounded_group_sequencer (S elements, C matcher)
				: elements(std::move(elements)), head(std::move(head)), matcher(std::move(matcher))
				noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<C>::value)
			{}
	
			/// Tests if there is any group left in the sequencer
			constexpr operator bool () const
			{
				return elements;
			}
	
			/// Decomposition
			constexpr optional<std::tuple<value_type, bounded_group_sequencer>> operator () () const
			{
				return fmap([&](auto decomposition)
				{
					auto head = std::move(std::get<0>(decomposition));
					auto tail = std::move(std::get<1>(decomposition));
					std::size_t count = 1;
	
					std::tie(count, tail) = fold([&matcher, &head](auto count, auto element)
					{
						const auto isMatching = matcher(head, element);
						const auto newCount = count + (isMatching ? 1 : 0);
						return std::make_tuple(newCount, isMatching);
					}, count, std::move(tail));
	
					auto taker = value_type(elements, count);
					auto remainings = bounded_group_sequencer(std::move(tail), matcher);
					return std::make_tuple(std::move(taker), std::move(remainings));
				}, elements());
			}
	
			/// Folding all elements
			template <typename V, Callable<V, V, value_type> C>
			friend constexpr V fold (C combiner, V value, bounded_group_sequencer sequencer)
			{
				auto sequence = std::move(sequencer.sequence);
				bool keepOn = sequence;
				while (keepOn)
				{
					std::tie(value, sequence, keepOn) = decide([&combiner, &sequencer.matcher]
						(auto decomposition, auto sequence, auto value)
					{
						auto head = std::move(std::get<0>(decomposition));
						auto remainings = std::move(std::get<1>(decomposition));
						auto folding = fold([&sequencer.matcher, &head](auto count, auto element)
						{
							const auto isMatching = sequencer.matcher(head, element);
							return std::make_tuple(count + (isMatching ? 1 : 0), isMatching);
						}, std::size_t(1), std::move(remains));
						const auto count = std::get<0>(folding);
						remainings = std::move(std::get<1>(folding));
						auto grouping = value_type(std::move(sequence), count);
						value = combiner(std::move(value), std::move(grouping));
						return std::make_tuple(std::move(value), std::move(remainings), static_cast<bool>(remainings));
					}, [](auto sequence, auto value)
					{
						return std::make_tuple(std::move(value), std::move(sequence), false);
					}, sequence(), std::move(sequence), std::move(value));
				}
				return value;
			}
	
			/// Folding initial elements
			template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
			friend constexpr std::tuple<V, bouned_group_sequencer sequencer)
			{
				auto sequence = std::move(sequencer.sequence);
				auto keepOn = static_cast<bool>(sequence);
				while (keepOn)
				{
					std::tie(value, sequence, keepOn) = decide([&combiner, &sequencer.matcher]
						(auto decomposition, auto sequence, auto value)
					{
						auto head = std::move(std::get<0>(decomposition));
						auto remainings = std::move(std::get<1>(decomposition));
						auto folding = fold([&sequencer.matcher, &head](auto count, auto element)
						{
							const auto isMatching = sequencer.matcher(head, element);
							return std::make_tuple(count + (isMatching ? 1 : 0), isMatching);
						}, std::size_t(1), std::move(remains));
						const auto count = std::get<0>(folding);
						remainings = std::move(std::get<1>(folding));
						auto grouping = value_type(std::move(sequence), count);
						bool keepOn;
						std::tie(value, keepOn) = combiner(std::move(value), std::move(grouping));
						return std::make_tuple(std::move(value), keepOn ? std::move(remainings) : std::move(sequence), keepOn);
					}, [](auto sequence, auto value)
					{
						return std::make_tuple(std::move(value), std::move(sequence), false);
					}, sequence(), std::move(sequence), std::move(value));
				}
				return std::make_tuple(std::move(value), std::move(sequence));
			}
	};
	
	
	
	template <UnboundedSequence S, Callable<bool, sequence_type_t<S>, sequence_type_t<S>> C>
	class unbounded_group_sequencer
	{
	
		protected:
	
			S sequence;
			C matcher;
	
		public:
	
			using value_type = unbounded_ntake_sequencer<S>;
	
			constexpr unbounded_group_sequencer (S sequence, C matcher)
				: sequence(std::move(sequence)), matcher(std::move(matcher))
				noexcept(std::is_nothrow_move_constructible<S>::value)
			{}
	
			constexpr std::tuple<value_type, unbounded_group_sequencer> operator () () const
			{
				auto decomposition = sequence();
				auto head = std::move(std::get<0>(decomposition));
				auto tail = std::move(std::get<1>(decomposition));
				auto folding = fold([&matcher, &head](auto count, auto element)
				{
					const auto isMatching = matcher(head, element);
					return std::make_tuple(count + (isMatching ? 1 : 0), isMatching);
				}, std::size_t(1), std::move(tail));
				const auto count = std::get<0>(folding);
				tail = std::move(std::get<1>(folding));
				auto grouping = value_type(sequence, count);
				return std::make_tuple(std::move(grouping), std::move(tail));
			}
	
			/// Folding initial elements
			template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
			friend constexpr std::tuple<V, unbounded_group_sequencer> fold (C combiner, V value,
				unbounded_group_sequencer sequencer)
			{
				auto sequence = std::move(sequencer.sequence);
				auto keepOn = true
				while (keepOn)
				{
					auto decomposition = sequence();
					auto head = std::move(std::get<0>(decomposition));
					auto tail = std::move(std::get<1>(decomposition));					
					auto folding = fold([&sequencer.matcher, &head](auto count, auto element)
					{
						const auto isMatching = matcher(head, element);
						return std::make_tuple(count + (isMatching ? 1 : 0), isMatching);
					}, std::size_t(1), std::move(tail));
					const auto count = std::get<0>(folding);
					tail = std::move(std::get<1>(folding));
					auto grouping = value_type(std::move(tail), count);
					std::tie(value, keepOn) = combiner(std::move(value), std::move(grouping));
					if (keepOn)
						sequence = std::move(tail);
				}
				return std::make_tuple(std::move(value), std::move(sequence));
			}
	};
		

}

#endif

