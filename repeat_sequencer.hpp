/// @file repeat_sequencer.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_REPEAT_SEQUENCER_HPP__
#define __STDEXT_REPEAT_SEQUENCER_HPP__

namespace stdext
{

	/// Repeat sequencer
	template <typename T>
	class repeat_sequencer
	{
	
		private:
	
			T value;
	
		public:
	
			/// Moves \a value into the constructed object
			constexpr repeat_sequencer (T value)
				: value(std::move(value))
				noexcept(std::is_nothrow_move_constructible<T>::value)
			{}
	
			/// Decomposes the sequencer into a copy of the held value and a copy of itself
			constexpr std::tuple<T, repeat_sequencer> operator () () const
			{
				return std::make_tuple(value, *this);
			}
	
			/// Folds initial elements in \a sequencer with \a combiner over \a value
			template <typename V, Callable<std::tuple<V, bool>, V, T> C>
			friend constexpr std::tuple<V, repeat_sequencer> fold (C combiner, V value, repeat_sequencer sequencer)
			{
				auto keepOn = true;
				while (keepOn) std::tie(value, keepOn) = combiner(std::move(value), sequencer.value);
				return std::make_tuple(std::move(value), *this);
			}
	};
		

}

#endif

