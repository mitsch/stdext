/// @file sequence.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_SEQUENCE_HPP__
#define __STDEXT_SEQUENCE_HPP__

#include <tuple>
#include <stdext/optional.hpp>

namespace stdext
{

	/// Concept of a bounded sequence
	///
	/// A bounded sequence is a definition of finite many values which are ordered. The definition
	/// can be mathematically inspired or be based on some data. A bounded sequence must be copyable
	/// and movable. The bounded sequence is decomposed into its first value and the remaining ones
	/// by the function operator. It is called without any parameter and returns an optional
	template <typename S> concept bool BoundedSequence ()
	{
		return require (S s)
		{
			S(s);
			S(std::move(s));
			s();
			{s} -> bool
		} and
		is_optional<decltype(std::declval<S>()())>::value and
		std::is_same<std::tuple_element<1, decltype(std::declval<S>()())::value_type>::type, S>::value;
	}

	/// Concept of a bounded sequence with value type \a T
	///
	/// Additionally to the constraints of being a bounded sequence, type \a S must produce elements
	/// of type \a T.
	template <typename S, typename T> concept bool BoundedSequence ()
	{
		return BoundedSequence<S> and
		       std::is_convertible<std::tuple_element<0, decltype(std::declval<S>()())::value_type>::type, T>::value;
	}

	/// Concept of an indexed bounded sequence
	///
	/// An indexed bounded sequence must require the concept of a bounded sequence. All elements in
	/// the sequence shall be accessible via the index operator. Accessing elements via the index
	/// operator must return an optional container
	template <typename S> concept bool IndexedBoundedSequence ()
	{
		return BoundedSequence<S> and
		       require (S s){s[std::size_t(0)];} and
					 require (S s){{s.length()} -> std::size_t;} and
		       is_optional<decltype(std::declval<S>()[std::size_t(0)])>::value and
					 std::is_same<
					 	std::tuple_element<0, decltype(std::declval<S>()())::value_type>::type,
		       	decltype(std::declval<S>()[std::size_t(0)])::value_type
						>::value;
	}

	template <typename S, typename T> concept bool IndexedBoundedSequence ()
	{
		return IndexedBoundedSequence<S> and
		       std::is_convertible<std::tuple_element<0, decltype(std::declval<S>()())::value_type>::type, T>::value
	}

	template <typename S> concept bool UnboundedSequence ()
	{
		return require (S s)
		{
			S(s);
			S(std::move(s));
			s();
		} and
		std::is_same<std::tuple_element<1, typename S::value_type>::type, S>::value;
	}

	template <typename S, typename T> concept bool UnboundedSequence ()
	{
		return UnboundedSequence<S> and
		       std::is_convertible<std::tuple_element<0, decltype(std::declval<S>()())>::type, T>::value;
	}

	template <typename S> concept bool IndexedUnboundedSequence ()
	{
		return UnboundedSequence<S> and
		       require(S s){s[std::size_t(0)]} and
					 std::is_same<
		       	decltype(std::declval<S>()[std::size_t(0)]),
		       	std::tuple_element<0, decltype(std::declval<S>()())>::type>::value;
	}

	template <typename S, typename T> concept bool IndexedUnboundedSequence ()
	{
		return IndexedUnboundedSequence<S> and
		       std::is_convertible<decltype(std::declval<S>()[std::size_t(0)]), T>::value;
	}

	template <typename S> concept bool Sequence ()
	{
		return BoundedSequence<S> or UnboundedSequence<S>;
	}

	template <typename S, typename T> concept bool Sequence ()
	{
		return BoundedSequence<S, T> or UnboundedSequence<S, T>;
	}



	template <typename S> struct sequence_type {};
	template <BoundedSequence S> struct sequence_type<S> :
		std::tuple_element<0, decltype(std::declval<S>()())::value_type> {};
	template <UnboundedSequence S> struct sequence_type<S> :
		std::tuple_element<0, decltype(std::declval<S>()())> {};
	template <typename S> using sequence_type_t = typename sequence_type<S>::type;

	
	template <typename V, BoundedSequence S, Callable<V, V, sequence_type_t<S>> C>
	constexpr V fold (C combine, V value, S sequence)
	{
		bool keepFolding = true;
		auto onElement = [combine=std::move(combine)](auto concatenation, V value, S sequence)
		{
			value = combine(std::move(value), std::move(std::get<0>(concatenation)));
			return std::make_tuple(std::move(value), std::move(std::get<1>(concatenation)), true);
		};
		auto onEnd = [](V value, S sequence)
		{
			return std::make_tuple(std::move(value), std::move(sequence), false);
		};
		while (keepFolding)
		{
			auto nextConcatenation = sequence();
			std::tie(value, sequence, keepFolding) = decide(onElement, onEnd, std::move(nextConcatenation));
		}
		return value;
	}


	/// 
	template <typename V, BoundedSequence S, Callable<std::tuple<V, bool>, V, sequence_type_t<S>> C>
	constexpr std::tuple<V, S> fold (C combiner, V value, S sequence)
	{
		bool keepFolding = true;
		auto onElement = [combiner](auto concatenation, V value, S sequence)
		{
			auto nextElement = std::move(std::get<0>(concatenation));
			auto nextSequence = std::move(std::get<1>(concatenation));
			auto combination = combiner(std::move(value), std::move(nextElement));
			value = std::move(std::get<0>(combination));
			auto keepGoing = std::get<1>(combination);
			auto returningSequence = keepGoing ? std::move(nextSequence) : std::move(sequence);
			return std::make_tuple(std::move(value), std::move(returningSequence), keepGoing);
		};
		auto onEnd = [](V value, S sequence)
		{
			return std::make_tuple(std::move(value), std::move(sequence), false);
		};
		while (keepFolding)
		{
			auto concatenation = sequence();
			std::tie(value, sequence, keepFolding) = decide(onElement, onEnd, std::move(concatenation),
				std::move(value), std::move(sequence));
		}
		return std::make_tuple(value, sequence);
	}

	template <typename V, UnboundedSequence S, Callable<std::tuple<V, bool>, V, sequence_type_t<S>> C>
	constexpr V fold (C combine, V value, S sequence)
	{
		bool keepFolding = true;
		while (keepFolding)
		{
			auto concatenation = sequence();
			std::tie(value, keepFolding) = combine(std::move(value), std::move(std::get<0>(concatenation)));
			if (keepFolding)
				sequence = std::move(std::get<1>(concatenation));
		}
		return std::make_tuple(value, sequence);
	}


	
	template <typename S>
		requires BoundedSequence<S>
	constexpr std::size_t length (S sequence)
	{
		return fold([](auto counter, auto value)
		{
			return counter + 1;
		}, std::size_t(0), std::move(sequence));
	}

	template <typename S>
		requires IndexedBoundedSequence<S>
	constexpr std::size_t length (S sequence)
	{
		return s.length();
	}




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
			friend constexpr std::tuple<V, bounded_transform_sequencer> fold (C combiner, bounded_transform_sequencer elements)
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

	};


	/// Transforming sequencer for indexed bounded sequencer
	///
	/// This class is a an extension of the transforming sequencer. Not only does it handle bounded
	/// sequences but indexed ones. Therefore, this class provides an indexing operator and a length
	/// method.
	template <IndexedBoundedSequence S, Callable_<sequence_type_t<S>> C>
	class indexed_bounded_transform_sequencer : public bounded_transform_sequencer<S, C>
	{
		
		public:

			/// Attribute constructor
			///
			/// The \a sequence and the \a transformer are moved into the object.
			constexpr indexed_bounded_transform_sequencer (S elements, C transformer)
				: bounded_transform_sequencer(std::move(elements), std::move(transformer))
				noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_assignable<S>::value)
			{}

			/// Returns the length of the sequence
			constexpr std::size_t length () const
			{
				return elements.length();
			}

			/// Returns either the element at \a index or an empty optional container, if the \a index is
			/// out of bound
			constexpr optional<value_type> operator [] (std::size_t index) const
			{
				return fmap(transformer, elements[index]);
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

			/// Folding
			///
			/// Elements are folded by \a combiner over starting \a value. All initial elements for which
			/// \a combiner returns a true flag are considered and folding stops with the first element
			/// for which \a combiner returns a false flag. The returning tuple contains the value folded
			/// over all initial elements and a sequencer with the remaining elements.
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


	/// Transforming sequencer for indexed unbounded sequences
	///
	/// This class is a an extension of the transforming sequencer. Not only does it handle unbounded
	/// sequences but indexed ones. Therefore, this class provides an indexing operator.
	template <IndexedUnboundedSequence S, Callable_<sequence_type_t<S>> C>
	class indexed_unbounded_transform_sequencer
	{
	
		public:

			/// Attribute constructor
			///
			/// The \a elements and the \a transformer are moved into the constructed object.
			constexpr indexed_unbounded_transform_sequencer (S elements, C transformer)
				: elements(std::move(elements)), transformer(std::move(transformer))
				noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<C>::value)
			{}

			/// Returns the transformed element at \a index
			constexpr value_type operator [] (std::size_t index) const
			{
				return transformer(elements[i]);
			}
	};




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

			/// Decomposition
			///
			/// The next element for which the filter criterion returns true will be returned along with
			/// a sequencer for the remaining elements. If no elements can be found, an empty optional
			/// container will be returned.
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
			///
			/// All elements in the sequence \a elements for which the filter criterion returns true,
			/// are folded by \a combiner with \a value. The returning value will be folded over all
			/// accepted elements.
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
			///
			/// All initial elements in the sequence \a elements for which the filter criterion returns
			/// true, are folded by \a combiner with \a value. All initial accepted elements are
			/// considered in the folding process until the first accepted elements passed to \a combiner
			/// will return a false flag. The returning tuple will contain a value folded over all
			/// initial accepted elements for which \a combiner returned a true flag along with a
			/// sequencer for the remaining elements.
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
			///
			/// The next element satisfying the filter criterion along with a sequencer for the remaining
			/// elements are returned.
			constexpr std::tuple<value_type, unbounded_filter_sequencer> operator () () const
			{
				// TODO maybe possible with fold?
				auto decomposition = elements();
				while (not predictor(std::get<0>(decomposition)))
					decomposition = std::get<1>(decomposition)();
				auto remainings = unbounded_filter_sequencer(std::move(std::get<1>(decomposition)), predictor);
				return std::make_tuple(std::move(std::get<0>(decomposition)), std::move(remainings));
			}

			/// Folding
			///
			/// All initial elements which satisfy the filter criterion are folded by \a combiner with \a
			/// value. The folding stops as soon as \a combiner returns a false flag for any of the
			/// elements. The return tuple consists of the folded value and a sequencer for the remaining
			/// elements.
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



	/// Taking sequencer for bounded sequence
	///
	/// 
	template <BoundedSequence S, Callable<bool, sequence_type_t<S>> C>
	class bounded_take_sequencer
	{
	
		protected:

			S elements;
			C predictor;

		public:

			using value_type = sequence_type_t<S>;

			constexpr bounded_take_sequencer (S elements, C predictor)
				: elements(std::move(elements)), predictor(std::move(predictor))
				noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<C>::value)
			{}

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


	template <UnboundedSequence S, Callable<bool, sequence_type_t<S>> C>
	class unbounded_take_sequencer
	{
	
		private:

			S elements;
			C predictor;

		public:

			using value_type = sequence_type_t<S>;

			constexpr unbounded_take_sequencer (S elements, C predictor)
				: elements(std::move(elements)), predictor(std::move(predictor))
				noexcept(std::is_nothrow_move_constructible<S>::value and std::is_nothrow_move_constructible<C>::value)
			{}

			constexpr optional<std::tuple<value_type, unbounded_take_sequencer>> operator () () const
			{
				auto decomposition = elements();
				return make_optional(predictor(std::get<0>(decomposition)), [&predictor](auto element, auto elements)
				{
					auto remainings = unbounded_take_sequencer(std::move(elements), predictor);
					return std::make_tuple(std::move(element), std::move(remainings));
				}, std::move(std::get<0>(decomposition)), std::move(std::get<1>(decomposition)));
			}

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


	template <BoundedSequence S>
	class bounded_ntake_sequencer;

	template <IndexedBoundedSequence S>
	class indexed_bounded_ntake_sequencer;

	template <UnboundedSequence S>
	class unbounded_ntake_sequencer;

	template <IndexedUnboundedSequence S>
	class indexed_unbounded_ntake_sequencer;


	
	template <BoundedSequence S>
	class bounded_concat_sequencer

}

#endif

