/// @file transform_sequencer.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_TRANSFORM_SEQUENCER_HPP__
#define __STDEXT_TRANSFORM_SEQUENCER_HPP__

#include <stdext/sequence_concept.hpp>
#include <stdext/callable.hpp>

namespace stdext
{

	/// Transforming sequencer for bounded sequences
	///
	/// A bounded transforming sequencer holds a sequence of elements which it transforms and some
	/// callable object of type \a C.
	template <BoundedSequence S, Callable_<sequence_type_t<S>> C>
	class bounded_transform_sequencer
	{
	
		protected:
	
			S elements;
			C transformer;
	
		public:
	
			/// The value type is the output type of applying the transformer
			using value_type = std::result_of_t<C(sequence_type_t<S>)>;
	
	
			/// Attribute constructor
			///
			/// All attributes, that is \a elements and \a transformer are moved into the constructed
			/// object.
			constexpr bounded_transform_sequencer (S elements, C transformer)
				: elements(std::move(elements)), transformer(std::move(transformer))
				noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<T>::value)
			{}
	
			/// Tests if the sequence contains any element
			constexpr operator bool () const
			{
				return elements;
			}
	
			/// Decomposition
			///
			/// The transformation of the next element in the boxed sequence and a sequencer for the
			/// remaining elements is returned. If the boxed sequence is empty, an empty optional
			/// container is returned.
			constexpr optional<std::tuple<value_type, bounded_transform_sequencer>> operator () () const
			{
				return fmap([&transformer](auto decomposition)
				{
					auto tValue = transformer(std::move(std::get<0>(decomposition)));
					return std::make_tuple(std::move(tValue), std::move(std::get<1>(decomposition)));
				}, elements());
			}
	
			/// Full folding
			///
			/// All elements in the sequence \a elements are folded  by \a combiner starting at \a value.
			template <typename V, Callable<V, V, value_type> C>
			friend constexpr V fold (C combiner, V value, bounded_transform_sequencer elements)
			{
				return fold([combiner=std::move(combiner), transformer=std::move(elements.transformer)]
					(auto value, auto element)
				{
					return combiner(std::move(value), transformer(std::move(element)));
				}, std::move(value), std::move(elements.elements));
			}
	
			/// Partial folding
			///
			/// Initial elements in the sequence \a elements are folded by \a combiner starting at \a
			/// value. Initial elements are folded as long as \a combiner returns a true flag. The
			/// returning tuple consists of the folded value and a sequencer with the remaining elements
			/// of the boxed sequence.
			template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
			friend constexpr std::tuple<V, bounded_transform_sequencer> fold (C combiner, V value,
				bounded_transform_sequencer elements)
			{
				auto folding = fold([combiner=std::move(combiner), transformer=elements.transformer]
					(auto value, auto element)
				{
					return combiner(std::move(value), transformer(std::move(element)));
				}, std::move(value), std::move(elements.elements));
				value = std::move(std::get<0>(folding));
				elements = bounded_transform_sequencer(std::move(std::get<1>(folding)), std::move(elements.transformer));
				return std::make_tuple(std::move(value), std::move(elements));
			}

			/// Reverse folding
			///
			/// All elements in the sequence \a elements are folded in reverse by \a combiner starting at
			/// \a value. The folded value will be returned.
			template <typename V, Callable<V, V, value_type> C>
			friend constexpr V fold_reverse (C combiner, V value, bounded_transform_sequencer elements)
			{
				const auto & transformer = elements.transformer;
				return fold_reverse([combiner=std::move(combiner), transformer=std::move(elements.transformer)]
					(auto value, auto element)
				{
					return combiner(std::move(value), transformer(std::move(element)));
				}, std::move(value), std::move(elements.elements));
			}

			/// Computes the amount of the elements
			friend constexpr std::size_t length (const bounded_transform_sequencer & sequencer)
			{
				return length(sequencer.sequence);
			}
	
	};
	
	
	
	/// Transforming sequencer for unbounded sequence
	///
	/// The sequencer holds a sequence and some callable object, referred to as transformer, which
	/// takes an element of the held sequence and returns some new value of possibly different type.
	/// The transforming sequencer appears as a sequence of the transformed element from the held
	/// sequence.
	template <UnboundedSequence S, Callable_<sequence_type_t<S>> C>
	class unbounded_transform_sequencer
	{
	
		protected:
	
			S elements;
			C transformer;
	
		public:
	
			/// type of the elements
			using value_type = std::result_of_t<C(sequence_type_t<S>)>;
	
			/// Attribute constructor
			///
			/// The \a sequence and the \a transformer are moved into the constructed object.
			constexpr unbounded_transform_sequencer (S elements, C transformer)
				: elements(std::move(elements)), transformer(std::move(transformer))
				noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<C>::value)
			{}
	
			/// Decomposition
			///
			/// The next element and a sequencer with the remaining elements is returned.
			constexpr std::tuple<value_type, unbounded_transform_sequencer> operator () () const
			{
				auto decomposition = elements();
				return std::make_tuple(transformer(std::get<0>(decomposition)), std::get<1>(decomposition));
			}
	
			/// Folding initial elements
			template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
			friend constexpr std::tuple<V, unbounded_transform_sequencer> fold (C combiner, V value,
				unbounded_transform_sequencer elements)
			{
				auto folding = fold([combiner=std::move(combiner), transformer=elements.transformer]
					(auto value, auto element)
				{
					return combiner(std::move(value), transformer(std::move(element)));
				}, std::move(value), std::move(elements.elements));
				value = std::move(std::get<0>(folding));
				elements = unbounded_transform_sequencer(std::move(std::get<1>(folding)), std::move(elements.transformer));
				return std::make_tuple(std::move(value), std::move(elements));
			}
	};
	
}

#endif

