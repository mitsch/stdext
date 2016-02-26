/// @file iteration_sequencer.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_ITERATION_SEQUENCER_HPP__
#define __STDEXT_ITERATION_SEQUENCER_HPP__

namespace stdext
{

		/// Iteration sequence
		///
		/// Starting off with a value of type \a T, the (i + 1)-th element in the sequence will be the
		/// result of applying some function of type \a C with the i-th element.
		template <typename T, Callable<T, T> C>
		class iteration_sequence
		{
			
			private:
		
				C iterator;
				T value;
		
			public:
		
				/// Attribute constructor
				constexpr iteration_sequence (C iterator, T value)
					: iterator(std::move(iterator)), value(std::move(value))
					noexcept(std::is_nothrow_move_constructible<C>::value and std::is_nothrow_move_constructible<T>::value)
				{}
				
				/// Decomposition
				constexpr std::tuple<T, iteration_sequence> operator () () const
				{
					return std::make_tuple(value, iteration_sequence(iterator, iterator(value)));
				}
		
				/// Folding
				template <typename V, Callable<std::tuple<V, bool>, V, T> C>
				friend constexpr std::tuple<V, iteration_sequence> fold (C combiner, V value, iteration_sequence sequence)
				{
					auto keepOn = true;
					while (keepOn)
					{
						std::tie(value, keepOn) = combiner(std::move(value), sequence.value);
						if (keepOn) sequence.value = sequence.iterator(std::move(sequence.value));
					}
					return std::make_tuple(std::move(value), std::move(sequence));
				}
		};

}

#endif

