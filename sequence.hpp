/// @file sequence.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_SEQUENCE_HPP__
#define __STDEXT_SEQUENCE_HPP__

#include <stdext/sequence_concept.hpp>
#include <stdext/transform_sequencer.hpp>
#include <stdext/filter_sequencer.hpp>
#include <stdext/take_sequencer.hpp>
#include <stdext/drop_sequencer.hpp>
#include <stdext/token_sequencer.hpp>
#include <stdext/cycle_sequencer.hpp>

namespace stdext
{

	/// Transforms bounded \a sequence with \a transformer
	template <BoundedSequence S, Callable_<sequence_type_t<S>> C>
	constexpr bounded_transform_sequencer<S, C> transform (C transformer, S sequence)
	{
		return bounded_transform_sequencer<S, C>(std::move(transformer), std::move(sequence));
	}

	/// Transforms unbounded \a sequence with \a transformer
	template <UnboundedSequence S, Callable_<sequence_type_t<S>> C>
	constexpr unbounded_transform_sequencer<S, C> transform (C transformer, S sequence)
	{
		return unbounded_transform_sequencer<S, C>(std::move(transformer), std::move(sequence));
	}
	



	/// Filters bounded \a sequence with \a predictor
	template <BoundedSequence S, Callable<bool, sequence_type_t<S>> C>
	constexpr bounded_filter_sequencer<S, C> filter (C predictor, S sequence)
	{
		return bounded_filter_sequencer<S, C>(std::move(predictor), std::move(sequence));
	}

	/// Filters unbounded \a sequence with \a predictor
	template <UnboundedSequence S, Callable<bool, sequence_type_s<S>> C>
	constexpr unbounded_filter_sequencer<S, C> filter (C predictor, S sequence)
	{
		return unbounded_filter_sequencer<S, C>(std::move(predictor), std::move(sequence));
	}



	template <BoundedSequence S, Callable<bool, sequence_type_t<S>> C>
	constexpr bounded_take_sequencer<S, C> take (C predictor, S sequence)
	{
		return bounded_take_sequencer<S, C>(std::move(sequence), std::move(predictor));
	}

	template <BoundedSequence S>
	constexpr bounded_ntake_sequencer<S> take (std::size_t count, S sequence)
	{
		return bounded_ntake_sequencer<S>(std::move(sequence), count);
	}

	template <UnboundedSequence S, Callable<bool, sequence_type_t<S>> C>
	constexpr unbounded_take_sequencer<S, C> take (C predictor, S sequence)
	{
		return unbounded_take_sequencer<S, C>(std::move(sequence), std::move(predictor));
	}

	template <UnboundedSequence S>
	constexpr unbounded_ntake_sequencer<S> take (std::size_t count, S sequence)
	{
		return unbounded_ntake_sequencer<S>(std::move(sequence), count);
	}


	/// Drops elements from the 
	template <BoundedSequence S, Callable<bool, sequence_type_t<S>> C>
	constexpr bounded_drop_sequencer<S, C> drop (C predictor, S sequence)
	{
		return bounded_drop_sequencer<S, C>(std::move(sequence), std::move(predictor));
	}

	template <BoundedSequence S>
	constexpr bounded_ndrop_sequencer<S> drop (std::size_t count, S sequence)
	{
		return bounded_ndrop_sequencer<S>(std::move(sequence), count);
	}

	template <UnboundedSequence S, Callable<bool, sequence_type_t<S>> C>
	constexpr unbounded_drop_sequencer<S, C> drop (C predictor, S sequence)
	{
		return unbounded_drop_sequencer<S, C>(std::move(sequence), std::move(predictor));
	}

	template <UnboundedSequence S>
	constexpr unbounded_ndrop_sequencer<S> drop (std:size_t count, S sequence)
	{
		return unbounded_ndrop_sequencer<S>(std::move(sequence), count);
	}


	/// Groups elements in \a sequencer with \a matcher
	///
	/// @{
	///
	template <BoundedSequence S, Callable<bool, sequence_type_t<S>, sequence_type_t<S>> C>
	constexpr bounded_group_sequencer<S, C> group (C matcher, S sequence)
	{
		return bounded_group_sequencer<S, C>(std::move(matcher), std::move(sequene));
	}

	template <UnboundedSequence S, Callable<bool, sequence_type_t<S>, sequence_type_t<S>> >
	constexpr unbounded_group_sequencer<S, C> group (C matcher, S sequence)
	{
		return unbounded_group_sequencer<S, C>(std::move(matcher), std::move(sequence));
	}
	/// @}



	/// Tokenises elements in \a sequence with \a combiner and \a init being the starting token value
	///
	/// Tokenisation ...
	///
	/// @{
	///
	template <BoundedSequence S, typename T, Callable<std::tuple<T, bool>, T, sequence_type_t<S>> C>
	constexpr bounded_token_sequencer<S, T, C> tokenise (C combiner, T init, S sequence)
	{
		return bounded_token_sequencer<S, T, C>(std::move(combiner), std::move(init), optional<T>(), std::move(sequence));
	}

	template <UnboundedSequence S, typename T, Callable<std::tuple<T, bool>, T, sequence_type_t<S>> C>
	constexpr unbounded_token_sequencer<S, T, C> tokenise (C combiner, T init, S sequence)
	{
		return unbounded_token_sequencer<S, T, C>(std::move(combiner), std::move(init), optional<T>(), std::move(sequencer));
	}
	/// @}


	/// Makes \a sequence unboundable by cycling through it indefinitely
	///
	/// @{
	///
	
	template <BoundedSequence S>
	constexpr cycle_sequence<S> cycle (S sequence)
	{
		return cycle_sequence<S>(std::move(sequence));
	}

	/// @}



	/// Repeats \a value indefinitely
	template <typename T>
	constexpr repeat_sequencer<T> repeat (T value)
	{
		return repeat_sequencer<T>(std::move(value));
	}

}


#endif

