/// @file intersperse_sequencer.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_INTERSPERSE_SEQUENCER_HPP__
#define __STDEXT_INTERSPERSE_SEQUENCER_HPP__

namespace stdext
{

	
		/// Interspersing sequencer for bounded sequence
		///
		/// The interspersing sequencer holds a sequencer and some element. Between each element of the
		/// sequencer, the special element is interspersed. If the sequence contains only zero or one
		/// element, no special element will be interspersed.
		template <BoundedSequence S, typename E>
		class bounded_intersperse_sequencer
		{
		
			protected:
		
				S sequence;
				E element;
				bool isIntersperseNext;
		
			public:
		
				/// Type of the elements is the common type of the sequence's elements and the interspersed
				/// element
				using value_type = typename std::common_type<sequence_type_t<S>, E>::type;
		
				/// Attribute constructor
				///
				/// The \a sequence and the interspersed \a element are moved into the constructed object.
				/// The flag \a isInterspersedNext indicates whether to put out the sequence's elements or
				/// the interspersing element next.
				constexpr bounded_intersperse_sequencer (S sequence, E interspersedElement, bool isInterspersedNext = false)
					: sequence(std::move(sequence)), interspersedElement(std::move(interspersedElement)),
					  isInterspersedNext(isInterspersedNext)
					noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<E>::value)
				{}
		
				/// Returns if the sequencer delivers any element or not
				constexpr operator bool () const
				{
					return sequence;
				}
		
				/// Decomposition
				///
				/// The next element will be returned along with a sequencer for the remaining elements. The
				/// next value will be either an element from the held sequence or a copy of the
				/// interspersing element.
				constexpr optional<std::tuple<value_type, bounded_intersperse_sequencer>> operator () () const
				{
					return fmap([&](auto decomposition)
					{
						if (isIntersperseNext)
						{
							auto remainings = bounded_intersperse_sequencer(sequence, interspersedElement, false);
							return std::make_tuple(interspersedElement, std::move(remainings));
						}
						else
						{
							auto head = std::move(std::get<0>(decomposition));
							auto remainings = bounded_intersperse_sequencer(std::move(std::get<1>(decomposition)),
								interspersedElement, true);
							return std::make_tuple(std::move(head), std::move(remainings));
						}
					}, sequence());
				}
		
				/// Folding all elements
				///
				/// All elements in \a sequencer are folded by \a combiner starting with \a value. The
				/// returning value will be folded over all elements.
				template <typename V, Callable<V, V, value_type> C>
				friend constexpr V fold (C combiner, V value, bounded_intersperse_sequencer sequencer)
				{
					auto sequence = std::move(sequencer.sequence);
					if (not sequencer.isIntersperseNext and sequence)
					{
						auto decomposition = sequence();
						auto head = std::move(std::get<0>(decomposition));
						sequence = std::move(std::get<1>(decomposition));
						value = combiner(std::mover(value), std::move(head));
					}
					return fold([combiner=std::move(combiner), interspersedElement=std::move(sequencer.interspersedElement)]
						(auto value, auto element)
					{
						value = combiner(std::move(value), interspersedElement);
						value = combiner(std::move(value), std::move(element));
						return value;
					}, std::move(value), std::move(sequence));
				}
		
				/// Folding initial elements
				///
				/// All initial elements are folded by \a combiner starting with \a value. Only initial
				/// elements for which \a combiner returns a true flag will be considered. The returning
				/// tuple consists of the folded value and a sequencer for the remaining elements.
				template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
				friend constexpr std::tuple<V, bounded_intersperse_sequencer> fold (C combiner, V value,
					bounded_intersperse_sequencer sequencer)
				{
					auto sequence = std::move(sequencer.sequence);
					auto element = std::move(sequencer.element);
					auto isIntersperseNext = sequencer.isIntersperseNext;
					auto keepOn = static_cast<bool>(sequence);
					if (not isIntersperseNext and keepOn)
					{
						auto decomposition = sequence();
						auto head = std::move(std::get<0>(decomposition));
						auto tail = std::move(std::get<1>(decomposition));
						std::tie(value, keepOn) = combiner(std::move(value), std::move(head));
						if (keepOn) sequence = std::move(tail);
						isIntersperseNext = keepOn;
					}
					if (keepOn)
					{
						auto folding = fold([combiner=std::move(combiner), &element](auto values, auto element)
						{
							auto value = std::move(std::get<0>(values));
							auto intersperseCombination = combiner(std::move(value), element);
							value = std::move(std::get<0>(intersperseCombination));
							auto keepOn = std::get<1>(intersperseCombination);
							auto doIntersperseNext = not keepOn;
							if (keepOn)
							{
								auto elementCombination = combiner(std::move(value), std::move(element));
								value = std::move(std::get<0>(elementCombination));
								keepOn = std::get<1>(elementCombination);
								doIntersperseNext = keepOn;
							}
							return std::make_tuple(std::make_tuple(std::move(value), doIntersperseNext), keepOn);
						}, std::make_tuple(std::move(value), true), std::move(sequence));
						value = std::move(std::get<0>(std::get<0>(folding)));
						isIntersperseNext = std::get<1>(std::get<0>(folding));
						sequence = std::move(std::get<1>(folding));
					}
					auto remainings = bounded_intersperse_sequencer(std::move(sequence), std::move(element), isInterspersedNext);
					return std::make_tuple(std::move(value), std::move(remainings));
				}
		};
		
		
		
		/// Intersperse sequencer for unbounded sequence
		template <UnboundedSequence S, typename E>
		class unbounded_intersperse_sequencer
		{
		
			protected:
		
				S sequence;
				E element;
				bool isIntersperseNext;
		
		
			public:
		
				/// Type of the elements is the common type of the held sequence's elements and the
				/// interspersing element
				using value_type = typename std::common_type<sequence_type_t<S>, E>::type;
		
				/// Attribute constructor
				///
				/// The \a sequence and the interspersing \a element is moved into the constructed object.
				/// The flag \a isIntersperseNext indicates whether to put out next the interspersing element
				/// (isIntersperseNext is true) or an element of the sequence (isIntersperseNext is false).
				constexpr unbounded_intersperse_sequencer (S sequence, E element, bool isIntersperseNext)
					: sequence(std::move(sequence)), element(std::move(element)), isInterspersNext(isIntersperseNext)
					noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<E>::value)
				{}
		
				/// Decomposition
				///
				/// The next element will be returned along with a sequencer for the remaining elements. The
				/// next element will be either a copy of the interspersing element or the next element in
				/// the held sequence.
				constexpr std::tuple<value_type, unbounded_intersperse_sequencer> operator () () const
				{
					if (isIntersperseNext)
					{
						auto remainings = unbounded_intersperse_sequencer(sequence, element, false);
						return std::make_tuple(element, std::move(remainings));
					}
					else
					{
						auto decomposition = sequence();
						auto head = std::move(std::get<0>(decomposition));
						auto tail = std::move(std::get<1>(decomposition));
						auto remainings = unbounded_intersperse_sequencer(std::move(tail), element, true);
						return std::make_tuple(std::move(head), std::move(remainings));
					}
				}
		
				// TODO implementation
				/// Fold initial elements
				template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
				friend constexpr std::tuple<V, unbounded_intersperse_sequencer> fold (C combiner, V value,
					unbounded_intersperse_sequencer sequencer)
				{
					auto keepOn = true;
					if (not sequencer.isIntersperseNext)
					{
						auto decomposition = sequencer.sequence();
						auto head = std::move(std::get<0>(decomposition));
						auto tail = std::move(std::get<1>(decomposition));
						std::tie(value, keepOn) = combiner(std::move(value), std::move(head));
						value = std::move(std::get<0>(combination));
						sequencer.isIntersperseNext = 
						if (keepOn) sequencer.sequence = std::move(tail);
					}
					if (keepOn)
					{
						auto folding = fold([combiner=std::move(combiner), &sequencer.element]
							(auto values, auto element)
						{
							auto value = std::move(std::get<0>(values));
							auto firstCombination = combiner(std::move(value), sequencer.element);
							value = std::move(std::get<0>(firstCombination));
							const auto passedFirst = std::get<1>(firstCombination);
							if (passedFirst)
							{}
						}, std::make_tuple(std::move(value), true), std::move(sequence));
						value = std::move(std::get<0>(std::get<0>(folding)));
						sequencer.isIntersperseNext = std::get<1>(std::get<0>(folding));
						sequencer.sequence = std::move(std::get<1>(folding));
					}
					return std::make_tuple(std::move(value), std::move(sequencer));
				}
		};
		

}

#endif

