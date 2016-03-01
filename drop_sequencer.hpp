/// @file drop_sequencer.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_DROP_SEQUENCER_HPP__
#define __STDEXT_DROP_SEQUENCER_HPP__

namespace stdext
{

	/// Dropping sequencer for bounded sequence
	///
	/// The sequencer is basically the same sequence as \a S without initial elements which
	/// satisfy the dropping criterium of type \a C.
	template <BoundedSequence S, Callable<bool, sequence_type_t<S>> C>
	class bounded_drop_sequencer
	{
	
		private:
	
			S elements;
			C predictor;
			bool initialised;
	
		public:
	
			/// Type of elements is the same as the one of the held sequence
			using value_type = std::result_of_t<C(sequence_type_t<S>)>;
	
			/// Attribute constructor
			///
			/// The \a elements and the dropping criterium \a predictor are moved into the constructed
			/// object. Depending on \a initialised, it will be assumed that dropping has been already
			/// done (initialised is true) or dropping has to be still performed (initialised is false).
			constexpr bounded_drop_sequencer (S elements, C predictor, bool initialised = false)
				: elements(std::move(elements)), predictor(std::move(predictor)), initialised(initialised)
				noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<C>::value)
			{}
	
			/// Tests if there is any element left in the dropping sequencer
			constexpr operator bool () const
			{
				const auto ifInitialisedEmpty = initialsied and elements;
				const auto ifNotInitialisedEmpty = not initialised and std::get<0>(fold([&predictor](auto value, auto element)
				{
					const auto isDropped = predictor(element);
					return std::make_tuple(not isDropped, isDropped);
				}, false, elements));
				return ifInitialisedEmpty or ifNotInitialisedEmpty;
			}
	
			/// Decomposition
			///
			/// The next element in the sequence is returned along side with a sequencer for the
			/// remaining elements. If no next element exists in the held sequence or all elements has
			/// been dropped, an empty optional container will be returned.
			constexpr optional<std::tuple<value_type, bounded_drop_sequencer>> operator () () const
			{
				auto copiedElements = elements;
				if (not initialised)
				{
					copiedElements = std::get<1>(fold([&predictor](auto value, auto element)
					{
						return std::make_tuple(std::move(value), predictor(element));
					}, int(0), std::move(copiedElements)));
				}
				return fmap([&predictor](auto decomposition)
				{
					auto remainings = bounded_drop_sequencer(std::move(std::get<0>(decompositions)), predictor, true);
					return std::make_tuple(std::move(std::get<1>(decompositions)), std::move(remainings));
				}, copiedElements());
			}
	
			/// Folding all elements
			///
			/// All elements in the sequence \a elements are folded by \a combiner with \a value. If
			/// dropping has not been performed, initial elements which satisfy the droppping criterum
			/// will be ignored and all remaining elements will be considered. If dropping has been
			/// performed, all remaining elements will be considered. The return value will be folded
			/// over all these elements.
			template <typename V, Callable<V, V, value_type> C>
			friend constexpr V fold (C combiner, V value, bounded_drop_sequencer elements)
			{
				if (not elements.initialised)
				{
					elements.elements = std::get<1>(fold([predictor=std::move(elements.predictor)]
						(auto value, auto element)
					{
						return std::make_tuple(std::move(value), predictor(element));
					}, int(0), std::move(elements.elements)));
				}
				return fold(std::move(combiner), std::move(value), std::move(elements.elements));
			}
	
			/// Folding initial elements
			///
			/// All initial elements in the sequence \a elements are folded by \a combiner with \a value.
			/// If dropping has not been performed, initial elements which satisfy the dropping criterium
			/// will be ignored. All elements then for which \a combiner returns a true flag, will be 
			/// considered. The same goes for a sequence where dropping has already been performed. The
			/// returned tuple consists of the folded value and a sequencer for the remaining values.
			template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
			friend constexpr std::tuple<V, bounded_drop_sequencer> fold (C combiner, V value, bounded_drop_sequencer elements)
			{
				if (not elements.initialised)
				{
					elements.elements = std::get<1>(fold([predictor=elements.predictor]
						(auto value, auto element)
					{
						return std::make_tuple(std::move(value), predictor(element));
					}, int(0), std::move(elements.elements)));
				}
				auto folding = fold(std::move(combiner), std::move(value), std::move(elements.elements));
				value = std::move(std::get<0>(folding));
				elements = bounded_drop_sequencer(std::move(std::get<1>(folding)), std::move(elements.predictor), true);
				return std::make_tuple(std::move(value), std::move(elements));
			}

			/// Computes the amout of elements
			friend constexpr std::size_t length (const bounded_drop_sequencer & sequencer)
			{
				if (sequencer.initialised)
				{
					return length(sequencer.sequence);
				}
				else
				{
					auto folding = fold([&sequencer.predictor](auto value, auto element)
					{
						return std::make_tuple(std::move(value), sequencer.predictor(element));
					}, int(0), sequencer.sequence);
					return length(std::get<1>(folding));
				}
			}
	};
	
	
	/// Dropping sequencer for unbounded sequence
	///
	/// The sequencer is basically the same sequence as \a S without initial elements which
	/// satisfy the dropping criterium of type \a C.
	template <UnboundedSequence S, Callable<bool, sequence_type_t<S>> C>
	class unbounded_drop_sequencer
	{
	
		private:
	
			S elements;
			C predictor;
			bool initialised;
	
		public:
	
			/// Type of the elements is the same as of the held sequence
			using value_type = sequence_type_t<S>;
	
			/// Attribute constructor
			///
			/// The \a elements and the dropping criterium \a predictor are moved into the constructed
			/// object. Depending on \a initialised, it will be assumed that dropping has been already
			/// performed (initialised is true) or dropping has still to be performed (initialised is
			/// false).
			constexpr unbounded_drop_sequencer (S elements, C predictor, bool initialised = false)
				: elements(std::move(elements)), predictor(std::move(predictor)), initialised(initialised)
				noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<C>::value)
			{}
	
			/// Decomposition
			///
			/// The next element is returned along with the sequencer for the remaining elements. If
			/// dropping has not been performed, initial elements which satisfy the dropping criterium
			/// will be ignored and the first element afterwards will be returned.
			constexpr std::tuple<value_type, unbounded_drop_sequencer> operator () () const
			{
				auto copiedElements = elements;
				if (not initialised)
				{
					copiedElements = fold([&predictor](auto value, auto element)
					{
						return std::make_tuple(std::move(value), predictor(element));
					}, int(0), std::move(copiedElements));
				}
				auto decomposition = copiedElements();
				auto remainings = unbounded_drop_sequencer(std::move(std::get<1>(decomposition)), predictor, true);
				return std::make_tuple(std::move(std::get<0>(decomposition)), std::move(remainings));
			}
	
			/// Folding
			///
			/// If dropping has still to be performed, all initial elements which satisfy the dropping
			/// criterium will be ignored. All initial elements after dropping for which \a combiner
			/// returns a true flag will be considered in the folding. The folding will be performed by
			/// \a combiner with \a value. The returning tuple will consist of the folded value and a
			/// sequencer for the remaining elements.
			template <typename V, Callable<std::tuple<V, bool>, V , value_type> C>
			friend constexpr std::tuple<V, unbounded_drop_sequencer> fold (C combiner, V value,
				unbounded_drop_sequencer elements)
			{
				if (not elements.initialised)
				{
					elements.elements = std::get<1>(fold([&elements.predictor](auto value, auto element)
					{
						return std::make_tuple(std::move(value), predictor(element));
					}, int(0), std::move(elements.elements)));
				}
				auto folding = fold(std::move(combiner), std::move(value), std::move(elements.elements));
				value = std::move(std::get<0>(folding));
				elements = unbounded_drop_sequencer(std::move(std::get<1>(folding)), predictor, true);
				return std::make_tuple(std::move(value), std::move(elements));
			}
	};
	
	
	
	
	
	/// Fixed amount dropping sequencer for bounded sequence
	///
	/// A fixed amount of elements is ignored before continuing with the held sequence.
	template <BoundedSequence S>
	class bounded_ndrop_sequencer
	{
	
		protected:
	
			S elements;
			std::size_t count;
	
		public:
	
			/// Type of the elements is the elements' type of the held sequence
			using value_type = sequence_type_t<S>;
	
			/// Attribute constructor
			///
			/// The \a elements and the \a count of the dropped elements is moved into the constructed
			/// object.
			constexpr bounded_ndrop_sequencer (S elements, std::size_t count)
				: elements(std::move(elements)), count(count)
				noexcept(std::is_nothrow_move_constructible<S>::value)
			{}
	
			/// Tests if there is any element left in the dropping sequencer
			constexpr operator bool () const
			{
				return 
			}
	
			/// Decomposition
			///
			/// The next element in the dropping sequencer is returned along with a sequencer for the
			/// remaining elements. If no element is left in the dropping sequence, an empty optional
			/// container will be returned.
			constexpr std::tuple<value_type, bounded_ndrop_sequencer> operator () () const
			{
				auto folding = fold([](auto count, auto element)
				{
					return std::make_tuple(count - 1, count > 0);
				}, count, elements);
				const auto remainings = std::move(std::get<1>(folding));
				return fmap([](auto decomposition)
				{
					auto head = std::move(std::get<0>(decomposition));
					auto tail = std::move(std::get<1>(decomposition));
					auto remainings = bounded_ndrop_sequencer(std::move(tail), 0);
					return std::make_tuple(std::move(head), std::move(remainings));
				}, remainings());
			}
	
			/// Folding all elements
			///
			/// If any element has to be dropped, it will be dropped. Any remaining elements will be
			/// folded by \a combiner starting with \a value. The returning value will be folded over all
			/// these elements.
			template <typename V, Callable<V, V, value_type> C>
			friend constexpr V fold (C combiner, V value, bounded_ndrop_sequencer sequencer)
			{
				auto folding = fold([](auto count, auto element)
				{
					return std::make_tuple(count - 1, count > 0);
				}, sequencer.count, std::move(sequencer.elements));
				return fold(std::move(combiner), std::move(value), std::move(std::get<1>(folding)));
			}
	
			/// Folding initial elements
			///
			/// If any element has to be dropped, it will be dropped. Any remaining elements will be
			/// folded by \a combiner starting with \a value. Only initial elements for which \a combiner
			/// returns a true flag will be considered. The returning tuple consists of the folded value
			/// and a sequencer for the remaining values.
			template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
			friend constexpr std::tuple<V, bounded_ndrop_sequencer> fold (C combiner, V value,
				bounded_ndrop_sequencer sequencer)
			{
				auto dropFolding = fold([](auto count, auto element)
				{
					return std::make_tuple(count - 1, count > 0);
				}, sequencer.count, std::move(sequencer.elements));
				auto folding = fold(std::move(combiner), std::move(value), std::move(std::get<1>(dropFolding)));
				value = std::move(std::get<0>(folding));
				sequencer = bounded_ndrop_sequencer(std::move(std::get<1>(folding)), 0);
				return std::make_tuple(std::move(value), std::move(sequencer));
			}

			/// Computes the amount of left elements
			friend constexpr std::size_t length (const bounded_ndrop_sequencer & sequencer)
			{
				const auto length = length(sequencer.sequence);
				return length > count ? (length - count) : 0;
			}
	};
	
	
	
	/// Fixed amount dropping sequencer for unbounded sequence
	///
	/// A fixed amount of elements is dropped before continuing with the held sequence.
	template <UnboundedSequence S>
	class unbounded_ndrop_sequencer
	{
	
		protected:
	
			S elements;
			std::size_t count;
	
		public:
	
			/// Type of the elements is the elements' type of the held sequence
			using value_type = sequence_type_t<S>;
	
			/// Attribute constructor
			///
			/// The \a elements and the \a count of dropped elements is moved into the constructed object.
			constexpr unbounded_ndrop_sequencer (S elements, std::size_t count)
				: elements(std::move(elements)), count(count)
				noexcept(std::is_nothrow_move_constructible<S>::value)
			{}
	
			/// Decomposition
			///
			/// The next element is returned along with a sequencer for the remaining elements.
			constexpr std::tuple<value_type, unbounded_ndrop_sequencer> operator () () const
			{
				auto droppedFolding = fold([](auto count, auto element)
				{
					return std::make_tuple(count - 1, count > 0);
				}, count, elements);
				const auto innerRemainings = std::move(std::get<1>(droppedFolding));
				auto decomposition = innerRemainings();
				auto head = std::move(std::get<0>(decomposition));
				auto remainings = unbounded_ndrop_sequencer(std::move(std::get<1>(decomposition)), 0);
				return std::make_tuple(std::move(head), std::move(remainings));
			}

			/// Folding initial elements
			///
			/// Initial elements are folded by \a combiner starting with \a value. Only initial elements
			/// for which \a combiner returns a true flag will be considered. If dropping has not been
			/// performed, it will be performed before the folding. The returning tuple consists of the
			/// folded value and a sequencer for the remaining elements.
			template <typename V, Callable<std::tuple<V, bool>, V, value_type> C>
			friend constexpr std::tuple<V, unbounded_ndrop_sequencer> fold (C combiner, V value,
				unbounded_ndrop_sequencer sequencer)
			{
				auto droppedFolding = fold([](auto count, auto element)
				{
					return std::make_tuple(count - 1, count > 0);
				}, sequencer.count, std::move(sequencer.elements));
				auto folding = fold(std::move(combiner), std::move(value), std::move(std::get<1>(droppedFolding)));
				auto head = std::move(std::get<0>(folding));
				auto tail = unbounded_ndrop_sequencer(std::move(std::get<1>(folding)), 0);
				return std::make_tuple(std::move(head), std::move(tail));
			}
	};
		

}

#endif

