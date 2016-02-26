/// @file take_sequencer.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_TAKE_SEQUENCER_HPP__
#define __STDEXT_TAKE_SEQUENCER_HPP__

namespace stdext
{

	/// Taking sequencer for bounded sequence
	///
	/// A prefix of a bounded sequence of type \a S is taken. The length of the prefix is determined
	/// by a predictor of type \a C. It The sequencer takes the longest prefix for which the
	/// predictor returns true.
	template <BoundedSequence S, Callable<bool, sequence_type_t<S>> C>
	class bounded_take_sequencer
	{
	
		protected:
	
			S elements;
			C predictor;
	
		public:
	
			/// element type is the element type of the held sequence
			using value_type = sequence_type_t<S>;
	
			/// Attribute constructor
			constexpr bounded_take_sequencer (S elements, C predictor)
				: elements(std::move(elements)), predictor(std::move(predictor))
				noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<C>::value)
			{}
	
			/// Returns if there is any element left in the sequencer
			constexpr operator bool () const
			{
				return decide(predictor, false_type(), elements());
			}
	
			/// Decomposes into the next element in the sequene with a sequencer for the remaining
			/// elements or into an empty optional container, if either the held sequence is empty or the
			/// 
			constexpr optional<std::tuple<value_type, bounded_take_sequencer>> operator () () const
			{
				return mbind([&predictor](auto decomposition)
				{
					auto element = std::move(std::get<1>(decomposition));
					return make_optional(predictor(element), [&predictor](auto element, auto elements)
					{
						auto remainings = bounded_take_sequencer(std::move(elements), predictor);
						return std::make_tuple(std::move(element), std::move(remainings));
					}, std::move(element), std::move(std::get<1>(decomposition));
				}, elements());
			}
	
			/// Folding all elements
			template <typename V, Callable<V, V, value_type> C>
			friend constexpr V fold (C combiner, V value, bounded_take_sequencer elements)
			{
				auto folding = fold([combiner=std::move(combiner), predictor=elements.predictor]
					(auto value, auto element)
				{
					const auto isTaking = predictor(element);
					if (isTaking)
						value = combiner(std::move(value), std::move(element));
					return std::make_tuple(std::move(value), isTaking);
				}, std::move(value), std::move(elements.elements));
				return std::get<0>(folding);
			}

			/// Folding initial elements
			template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
			friend constexpr std::tuple<V, bounded_take_sequencer> fold (C combiner, V value, bounded_take_sequencer elements)
			{
				auto folding = fold([combiner=std::move(combiner), &predictor](auto value, auto element)
				{
					const auto isTaking = predictor(element);
					auto isContinuing = true;
					if (isTaking)
						std::tie(value, isContinuing) = combiner(std::move(value), std::move(element));
					return std::make_tuple(std::move(value), isTaking and isContinuing);
				}, std::move(value), std::move(elements.elements));
				value = std::move(std::get<0>(folding));
				elements = bounded_take_sequencer(std::move(std::get<1>(folding), std::move(elements.predictor));
				return std::make_tuple(std::move(value), std::move(elements));
			}
	};
	
	
	/// Take sequencer for unbounded sequence
	template <UnboundedSequence S, Callable<bool, sequence_type_t<S>> C>
	class unbounded_take_sequencer
	{
	
		private:
	
			S elements;
			C predictor;
	
		public:
	
			using value_type = sequence_type_t<S>;
	
			/// Moves \a elements and \a predictor into the constructed object
			constexpr unbounded_take_sequencer (S elements, C predictor)
				: elements(std::move(elements)), predictor(std::move(predictor))
				noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<C>::value)
			{}
	
			/// Decomposes into the next element with a sequencer for the remaining elements or an empty
			/// optional container, if the predictor has disapproved
			constexpr optional<std::tuple<value_type, unbounded_take_sequencer>> operator () () const
			{
				auto decomposition = elements();
				return make_optional(predictor(std::get<0>(decomposition)), [&predictor](auto element, auto elements)
				{
					auto remainings = unbounded_take_sequencer(std::move(elements), predictor);
					return std::make_tuple(std::move(element), std::move(remainings));
				}, std::move(std::get<0>(decomposition)), std::move(std::get<1>(decomposition)));
			}
	
			/// Folds all elements
			template <typename V, Callable<V, V, value_type> C>
			friend constexpr V fold (C combiner, V value, unbounded_take_sequencer elements)
			{
				auto folding = fold([combiner=std::move(combiner), predictor=elements.predictor]
					(auto value, auto element)
				{
					const auto isTaking = predictor(element);
					if (isTaking)
						value = combiner(std::move(value), std::move(element));
					return std::make_tuple(std::move(value), isTaking);
				}, std::move(value), std::move(elements.elements));
				return std::get<0>(folding);
			}
	
			/// Folds initial elements
			template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
			friend constexpr std::tuple<V, unbounded_take_sequencer> fold (C combiner, V value,
				unbounded_take_sequencer elements)
			{
				auto folding = fold([combiner=std::move(combiner), predictor=elements.predictor]
					(auto value, auto element)
				{
					const auto isTaking = predictor(element);
					auto isContinuing = true;
					if (isTaking)
						std::tie(value, isContinuing) = combiner(std::move(value), std::move(element));
					return std::make_tuple(std::move(value), isTaking and isContinuing);
				}, std::move(value), std::move(elements.elements));
				value = std::move(std::get<0>(folding));
				elements = unbounded_take_sequencer(std::move(std::get<1>(folding)), std::move(elements.predictor));
				return std::make_tuple(std::move(value), std::move(elements));
			}
	};

	

	/// Fixed amount taking sequencer for bounded sequence
	///
	/// Taking a fixed amount of elements from the sequence is a specialisation of the take
	/// sequence. Independent of the actual elements, a fixed amount of elements is taken from the
	/// held sequence.
	template <BoundedSequence S>
	class bounded_ntake_sequencer
	{
	
		protected:
	
			S elements;
			std::size_t count;
	
		public:
	
			/// Type of the elements is the elements' type of the held sequence
			using value_type = sequence_type_t<S>;
	
			/// Attribute constructor
			///
			/// The \a elements and the \a count of accepting elements are moved into the constructed
			/// object.
			constexpr bounded_ntake_sequencer (S elements, std::size_t count)
				: elements(std::move(elements)), count(count)
				noexcept(std::is_nothrow_move_constructible<S>::value)
			{}
	
			/// Tests if there is any element left in the sequencer
			constexpr operator bool () const
			{
				return count > 0;
			}
			
			/// Decomposition
			///
			/// The next element in the held sequence is returned along with a sequencer for the
			/// remaining elements. If no element is left in the held sequence or all elements are
			/// already taken, an empty optional container will be returned.
			constexpr optional<std::tuple<value_type, bounded_ntake_sequencer>> operator () () const
			{
				auto boxedDecomposition = make_optional(count > 0, [&elements](){return elements()});
				return fmap([count](auto decomposition)
				{
					auto element = std::move(std::get<0>(decomposition));
					auto elements = bounded_ntake_sequencer(std::move(std::get<1>(decomposition)), count - 1);
					return std::make_tuple(std::move(value), std::move(elements));
				}, mflatten(std::move(boxedDecomposition)));
			}
	
			/// Folding all elements
			///
			/// If any element is left in the take \a sequencer, they will be folded by \a combiner
			/// starting with \a value. The returning value will be folded over all these elements.
			template <typename V, Callable<V, V, value_type> C>
			friend constexpr V fold (C combiner, V value, bounded_ntake_sequencer sequencer)
			{
				auto folding = fold([combiner=std::move(combiner)](auto values, auto element)
				{
					const auto count = std::get<1>(values);
					auto value = std::move(std::get<0>(values));
					if (count > 0)
						value = combiner(std::move(value), std::move(element));
					return std::make_tuple(std::make_tuple(std::move(value), count - 1), count > 0);
				}, std::make_tuple(std::move(value), sequencer.count), std::move(sequencer.elements));
				return std::get<0>(folding);
			}
	
			/// Folding initial elements
			///
			/// If any element is left in the take \a sequencer, they will be folded by \a combiner
			/// starting with \a value. Only initial elements for which \a combiner returns a true flag
			/// are considered in the folding. The returning tuple contains the folded value as well a
			/// sequencer for the remaining elements.
			template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
			friend constexpr std::tuple<V, bounded_ntake_sequencer> fold (C combiner, V value,
				bounded_ntake_sequencer sequencer)
			{
				auto folding = fold([combiner=stD::move(combiner)](auto values, auto element)
				{
					const auto count = std::get<1>(values);
					auto value = std::move(std::get<0>(values));
					auto isContinuing = true;
					if (count > 0)
						std::tie(value, isContinuing) = combiner(std::move(value), std::move(element));
					const auto keepOn = isContinuing and count > 0;
					const auto newCount = count - (keepOn and  ? 1 : 0);
					return std:make_tuple(std::make_tuple(std::move(value), newCount), keepOn);
				}, std:make_tuple(std::move(value), sequencer.count), std::move(sequencer.elements));
				value = std::move(std::get<0>(std::get<0>(folding)));
				sequencer = bounded_ntake_sequencer(std::move(std::get<1>(folding)), std::get<1>(std::get<0>(folding)));
				return std::make_tuple(std::move(value), std::move(sequencer));
			}

			/// Computes amount of elements
			friend constexpr std::size_t length (const bounded_ntake_sequencer & sequencer)
			{
				const auto innerLength = length(sequencer.sequence);
				return innerLength < sequencer.count ? innerLength : sequencer.count;
			}
	};
	
	
	/// Fixed amount take sequencer for unbounded sequence
	///
	/// Taking a fixed amount of elements from the sequence is a specialisation of the take
	/// sequence. Independent of the actual elements, a fixed amount of elements is taken from the
	/// held sequence.
	template <UnboundedSequence S>
	class unbounded_ntake_sequencer
	{
	
		protected:
	
			S sequence;
			std::size_t count;
	
		public:
	
			/// Type of elements is type of elements from the held sequence
			using value_type = sequence_type_t<S>;
	
			/// Attribute constructor
			///
			/// The \a elements and the \a count of elements to consider are moved into the constructed
			/// object.
			constexpr unbounded_ntake_sequencer (S sequence, std::size_t count)
				: sequence(std::move(sequence)), count(count)
				noexcept(std::is_nothrow_move_constructible<S>::value)
			{}
	
			/// Decomposition
			///
			/// The next element is returned along with a sequencer for the remaining elements.
			constexpr optional<std::tuple<value_type, unbounded_ntake_sequencer>> operator () () const
			{
				return make_optional(count > 0, [&]()
				{
					auto decomposition = sequence();
					auto remainings = unbounded_ntake_sequencer(std::move(std::get<1>(decomposition)), count - 1);
					return std::make_tuple(std::move(std::get<0>(decomposition)), std::move(remainings);
				});
			}
	
			/// Folding all elements
			///
			/// If any element is left in the take \a sequencer, they will be folded by \a combiner
			/// starting with \a value. The returning value will be folded over all these elements.
			template <typename V, Callable<V, V, value_type> C>
			friend constexpr V fold (C combiner, V value, unbounded_ntake_sequencer sequencer)
			{
				auto folding = fold([combiner=std::move(combiner)](auto values, auto element)
				{
					auto count = std::get<1>(values);
					auto value = std::move(std::get<0>(values));
					if (count > 0)
						value = combiner(std::move(value), std::move(element));
					return std::make_tuple(std::make_tuple(std::move(value), count - 1), count > 0);
				}, std::make_tuple(std::move(value), sequencer.count), std::move(sequencer.sequence));
				return std::get<0>(folding);
			}
	
			/// Folding initial elements
			///
			/// If any element is left in the take \a sequencer, they will be folded by \a combiner
			/// starting with \a value. Only initial elements for which \a combiner returns a true flag
			/// will be considered in the folding. The returning tuple consists of the folded value and
			/// a sequencer for the remaining elements.
			template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
			friend constexpr std::tuple<V, unbounded_ntake_sequencer> fold (C combiner, V value,
				unbounded_ntake_sequencer sequencer)
			{
				auto folding = fold([combiner=std::move(combiner)](auto values, auto element)
				{
					auto value = std::move(std::get<0>(values));
					auto count = std::get<1>(values);
					auto isContinuing = true;
					if (count > 0);
						std::tie(value, isContinuing) = combiner(std::move(value), std::move(element));
					const auto keepOn = isContinuing and count > 0;
					const auto newCount = count - (keepOn ? 1 : 0);
					return std::make_tuple(std::make_tuple(std::move(value), newCount), keepOn);
				}, std::make_tuple(std::move(value), sequencer.count), std::move(sequencer.sequence));
				value = std::move(std::get<0>(std::get<0>(folding)));
				sequencer = unbounded_ntake_sequencer(std::move(std::get<1>(folding)), std::get<1>(std::get<0>(folding)));
				return std::make_tuple(std::move(value), std::move(sequencer));
			}

			/// Computes amount of elements
			friend constexpr std::size_t length (const unbounded_ntake_sequencer & sequencer)
			{
				return sequencer.count;
			}
	};
	

}

#endif

