/// @file filter_sequencer.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_FILTER_SEQUENCER_HPP__
#define __STDEXT_FILTER_SEQUENCER_HPP__

namespace stdext
{

	
	/// Bounded filter sequencer
	///
	/// The filter sequencer takes a sequence of type \a S and a predictor object of type \a C and
	/// holds out any element for which the predictor object returns false.
	template <BoundedSequence S, Callable<bool, const T&> C>
	class bounded_filter_sequencer
	{
	
		private:
	
			S elements;
			C predictor;
	
		public:
	
			/// type of the elements in the held sequence
			using value_type = sequence_type_t<S>;
	
			/// Attribute constructor
			///
			/// The \a elements and the \a predictor will be moved into the constructed object.
			constexpr bounded_filter_sequencer (S elements, C predictor)
				: elements(std::move(elements)), predictor(std::move(predictor))
				noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<C>::value)
			{}
	
			/// Tests if any element is left in the sequencer
			constexpr operator bool () const
			{
				return not elements and elements();
			}
	
			/// Decomposition
			constexpr optional<std::tuple<value_type, bounded_filter_sequencer>> operator () () const
			{
				auto folding = fold([&predictor](auto value, auto element)
				{
					const auto keepGoing = not value;
					if (keepGoing)
					{
						const auto isAccepting = predictor(element);
						value = isAccepting ? optional<value_type>(std::move(element)) : optional<value_type>();						
					}
					return std::make_tuple(std::move(value), keepGoing);
				}, optional<value_type>(), elements);
	
				return fmap([&predictor](auto element, auto remainings)
				{
					auto boxedRemainings = bounded_filter_sequencer(std::move(remainings), predictor);
					return std::make_tuple(std::move(element), std::move(boxedRemainings));
				}, std::move(std::get<0>(folding)), std::move(std::get<1>(folding));
			}
	
			/// Folding all elements
			template <typename V, Callable<V, V, value_type> C>
			friend constexpr V fold (C combiner, V value, bounded_filter_sequencer elements)
			{
				return fold([combiner=std::move(combiner), predictor=elements.predictor]
					(auto value, auto element)
				{
					return predictor(element) ? combiner(std::move(value), std::move(element)) : value;
				}, std::move(value), std::move(elements.elements));
			}
	
			/// Folding initial elements
			template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
			friend constexpr std::tuple<V, bounded_filter_sequencer> fold (C combiner, V value,
				bounded_filter_sequencer elements)
			{
				auto folding = fold([combiner=std::move(combiner), predictor=elements.predictor]
					(auto value, auto element)
				{
					return predictor(element) ?
						combiner(std::move(value), std::move(element)) :
						std::make_tuple(std::move(value), true);
				}, std::move(value), std::move(elements.elements));
				value = std::move(std::get<0>(folding));
				elements = bounded_filter_sequencer(std::move(std::get<1>(folding), std::move(element.predictor)));
				return std::make_tuple(std:move(value), std::move(elements));
			}
	};
	
	
	/// Unbounded filter sequencer
	///
	/// The filter sequencer takes a sequence of type \a S and a predictor object of type \a C and
	/// holds out any element for which the predictor object returns false.
	template <UnboundedSequence S, Callable<bool, sequence_type_t<S>> C>
	class unbounded_filter_sequencer
	{
	
		private:
	
			S elements;
			C predictor;
	
		public:
	
			/// Type of the elements in the held sequence
			using value_type = sequence_type_t<S>;
	
			/// Attribute constructor
			///
			/// The \a elements and the filter criterion \a predictor are moved into the constructed
			/// object.
			constexpr unbounded_filter_sequencer (S elements, C predictor)
				: elements(std::move(elements)), predictor(std::move(predictor))
				noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<C>::value)
			{}
	
			/// Decomposition
			constexpr std::tuple<value_type, unbounded_filter_sequencer> operator () () const
			{
				auto folding = fold([&predictor](auto optElement, auto element)
				{
					const auto keepOn = not optElement;
					if (keepOn and predictor(element)) optElement = std::move(element);
					return std::make_tuple(std::move(optElement), keepOn);
				}, optional<value_type>(), elements);
				auto element = std::move(std::get<0>(folding));
				auto remainings = unbounded_filter_sequencer(std::move(std::get<1>(folding)), predictor);
				return std::make_tuple(std::move(element), std::move(remainings));
			}
	
			/// Folding initial elements
			template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
			friend constexpr std::tuple<V, unbounded_filter_sequencer> fold (C combiner, V value,
				unbounded_filter_sequencer elements)
			{
				auto folding = fold([predictor=elements.predictor, combiner=std::move(combiner)]
					(auto value, auto element)
				{
					return predictor(element) ?
						combiner(std::move(value), std::move(element)) :
						std::make_tuple(std::move(value), true);
				}, std::move(value), std::move(elements.elements));
				value = std::move(std::get<0>(folding));
				elements = unbounded_filter_sequencer(std::move(std::get<1>(folding)), std::move(elements.predictor));
				return std::make_tuple(std::move(value), std::move(elements));
			}
	};

}

#endif

