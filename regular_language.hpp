/// @file regular_language.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_REGULAR_LANGUAGE_HPP__
#define __STDEXT_REGULAR_LANGUAGE_HPP__

#include <stdext/integral_constant.hpp>
#include <stdext/integral_list.hpp>
#include <stdext/integral_set.hpp>
#include <stdext/integral_dictionary.hpp>
#include <stdext/type_list.hpp>
#include <type_traits>

namespace stdext
{
	namespace regular_language
	{

		template <typename A, typename B, typename C, size_t ... D> struct _crosser;
		template <typename A, typename B, size_t ... C> struct _crosser<index_list<>, A, B, C ...> : typer<index_list<C ...>> {};
		template <typename A, size_t ... B> struct _crosser<A, index_list<>, index_list<>, B ...> : typer<index_list<B ...>> {};
		template <size_t A, size_t ... B, size_t C, size_t ... D, size_t ... E> struct _crosser<index_list<A, B ...>, index_list<>, index_list<C, D ...>, E ...> : _crosser<index_list<B ...>, index_list<C, D ...>, index_list<D ...>, E ...> {};
		template <size_t A, size_t ... B, size_t C, size_t ... D, typename E, size_t ... F> struct _crosser<index_list<A, B ...>, index_list<C, D ...>, E, F ...> : _crosser<index_list<A, B ...>, index_list<D ...>, E, F ..., A, C> {};

		template <typename A, bool B, typename C, typename ... D> struct partitioner;
		template <typename A, bool B, A ... C, A D, typename ... E> struct partitioner<A, B, integral_set<A, false, C ...>, integral_constant<A, D>, E ...> : partitioner<A, B, typename integral_set<A, false, C ...>::template element_inserting<D>, E ...> {};
		template <typename A, bool B, A ... C, A D, A E, typename ... F> struct partitioner<A, B, integral_set<A, false, C ...>, integral_list<A, D, E>, F ...> : partitioner<A, B, typename integral_set<A, false, C ...>::template range_inserting<D, E+1>, F ...> {};
		template <typename A, bool B, A ... C> struct partitioner<A, B, integral_set<A, false, C ...>> {using type = integral_set<A, B, C ...>;};


		template <typename A> struct _is_empty : true_type {};
		template <typename A, A B> struct _is_empty<integral_constant<A, B>> : false_type {};
		template <typename A, A B, A C> struct _is_empty<integral_list<A, B, C>> : bool_constant<B >= C> {};


		template <bool A, typename B, typename ... C> struct set
		{
			template <size_t E> struct transition_stack
			{
				using ins = index_list<E>;
				using outs = index_list<E>;
				using transitions = index_list<>;
				static constexpr auto count = E+1;
			};

			template <size_t E> using stack = transition_stack<E>;
			using partitioning = typename partitioner<B, A, integral_set<B, false>, C ...>::type;
			using leafs = type_list<partitioning>;
			static constexpr auto epsilon = not A and conjunction_v<_is_empty<C> ...>;
			using value_type = B;
		};

		template <typename A, typename B> struct alternation
		{
			template <size_t C> struct transition_stack
			{
				using _A = typename A::template stack<C>;
				using _B = typename B::template stack<_A::count>;
				using ins = typename _A::ins::template inserting<typename _B::ins>;
				using outs = typename  _A::outs::template inserting<typename _B::outs>;
				using transitions = typename _A::transitions::template concatenating<typename _B::transitions>;
				static constexpr auto count = _B::count;
			};

			template <size_t C> using stack = transition_stack<C>;
			using leafs = typename A::leafs::template concatenating<typename B::leafs>;
			static constexpr auto epsilon = A::epsilon or B::epsilon;
			using value_type = typename std::common_type<typename A::value_type, typename B::value_type>::type;
		};

		template <typename A, typename B> struct concatenation
		{
			template <size_t C> struct transition_stack
			{
				using _A = typename A::template stack<C>;
				using _B = typename B::template stack<_A::count>;
				using ins = typename _A::ins::template concatenating<typename std::conditional<A::epsilon, typename _B::ins, index_list<>>::type>;
				using outs = typename std::conditional<B::epsilon, typename _A::outs, index_list<>>::type::template concatenating<typename _B::outs>;
				using transitions = typename _A::transitions::template concatenating<typename _crosser<typename _A::ins, typename _B::outs, typename _B::outs>::type>::template concatenating<typename _B::transitions>;
				static constexpr auto count = _B::count;
			};

			using leafs = typename A::leafs::template concatenating<typename B::leafs>;
			static constexpr auto epsilon = A::epsilon and B::epsilon;
			template <size_t C> using stack = transition_stack<C>;
			using value_type = typename std::common_type<typename A::value_type, typename B::value_type>::type;
		};

		template <typename A> struct closure
		{
			template <size_t B> struct transition_stack
			{
				using _A = typename A::template stack<B>;
				using ins = typename _A::ins;
				using outs = typename _A::outs;
				using transitions = typename _A::transitions::template concatenating<typename _crosser<outs, ins, ins>::type>;
				static constexpr auto count = _A::count;
			};

			template <size_t B> using stack = transition_stack<B>;
			using leafs = typename A::leafs;
			static constexpr auto epsilon = true;
			using value_type = typename A::value_type;
		};

		template <typename A> struct option
		{
			using leafs = typename A::leafs;
			static constexpr auto epsilon = true;
			template <size_t B> using stack = typename A::template stack<B>;
			using value_type = typename A::value_type;
		};



		template <typename A, typename B, typename C> struct dictionary_maker
		{using type = B;};
		template <typename A, typename B, size_t C, size_t ... D> struct dictionary_maker<A, B, index_list<C, D ...>>
			: dictionary_maker<A, typename B::template set_inserting_<typename A::template getting<C>, C>, index_list<D ...>>
		{};

		template <typename A, typename B> struct state_updater {using type = A;};
		template <typename A, typename B, typename C, bool> struct state_updater_ : state_updater<A, C> {};
		template <typename A, typename B, typename C> struct state_updater_<A, B, C, false> : state_updater<typename A::template appending<B>, C> {};
		template <typename A, size_t ... B, typename ... C> struct state_updater<A, type_list<index_list<B ...>, C ...>> : state_updater_<A, index_list<B ...>, type_list<C ...>, A::template element_testing<index_list<B ...>>> {};

		template <typename A, typename B> struct dictionary_mapper;
		template <typename A, typename B, typename C, typename ... D> struct dictionary_mapper<A, integral_dictionary<B, size_t, C, type_list<D ...>>>
		{
			using type = type_list<C, index_list<A::template index<D> ...>>;
		};

		template <typename A, typename B, typename C> struct next_state_transister : typer<C> {};
		template <size_t A, size_t B, size_t ... C, size_t ... D, typename E> struct next_state_transister<index_list<A, B, C ...>, index_list<A, D ...>, E>
			: next_state_transister<index_list<C ...>, index_list<A, D ...>, typename E::template inserting<index_constant<B>>>
		{};
		template <size_t A, size_t B, size_t ... C, size_t D, size_t ... E, typename F> struct next_state_transister<index_list<A, B, C ...>, index_list<D, E ...>, F>
			: std::conditional<
				A < D, next_state_transister<index_list<C ...>, index_list<D, E ...>, F>,
				next_state_transister<index_list<A, B, C ...>, index_list<E ...>, F>
			>::type
		{};



		template <typename A, typename B, typename C, typename D, typename E, typename F> struct transitioner;
		template <typename A, typename B, typename C, size_t D, typename E, typename F> struct transitioner<A, B, C, index_constant<D>, E, F>
		{
			using value_type = A;
			using leafs = B;
			using nfa_transitions = C;
			using seen = E;
			using stack = F;
			using current_states = typename seen::template getting<D>;
			using next_states = typename next_state_transister<nfa_transitions, current_states, index_list<>>::type;
			using partition = typename dictionary_maker<leafs, integral_dictionary<value_type, size_t, integral_list<value_type>, type_list<index_list<>>>, next_states>::type;
			using updated_seen = typename state_updater<seen, typename partition::values>::type;
			using updated_partition = typename dictionary_mapper<seen, partition>::type;
			using dfa_transitions = transitioner<value_type, leafs, nfa_transitions, index_constant<D+1>, updated_seen, typename stack::template appending<updated_partition>>;
			using transitions = typename dfa_transitions::transitions;
			using states = typename dfa_transitions::states;
		};
		template <typename A, typename B, typename C, typename ... D, typename E> struct transitioner<A, B, C, index_constant<sizeof...(D)>, type_list<D ...>, E>
		{
			using transitions = E;
			using states = type_list<D ...>;
		};

		struct _start_token {};

		template <typename A, typename B, typename C, typename D> struct start_transitioner
		{
			using value_type = A;
			using leafs = B;
			using nfa_transitions = C;
			using ins = D;
			using partition = typename dictionary_maker<leafs, integral_dictionary<value_type, size_t, integral_list<value_type>, type_list<index_list<>>>, ins>::type;
			using seen = typename state_updater<type_list<_start_token>, typename partition::values>::type;
			using updated_partition = typename dictionary_mapper<seen, partition>::type;
			using dfa_transitions = transitioner<value_type, leafs, nfa_transitions, index_constant<1>, seen, type_list<updated_partition>>;
			using transitions = typename dfa_transitions::transitions;
			using states = typename dfa_transitions::states;
		};

		template <typename A, typename B, bool ... C> struct accepter;
		template <typename A, size_t ... B, typename ... C, bool ... D> struct accepter<A, type_list<index_list<B ...>, C ...>, D ...>
			: accepter<A, type_list<C ...>, D ..., disjunction_v<bool_constant<A::template element_testing<B>> ...>>
		{};
		template <typename A, bool ... B> struct accepter<A, type_list<>, B ...>
		{
			using type = integral_list<bool, B ...>;
		};


		template <typename A> struct compiler
		{
			using root = A;
			using leafs = typename root::leafs;
			using root_stack = typename root::template stack<0>;
			using ins = typename root_stack::ins;
			using outs = typename root_stack::outs;
			using value_type = typename root::value_type;
			using nfa_transitions = typename root_stack::transitions;
			using dfa_transitions = start_transitioner<value_type, leafs, nfa_transitions, ins>;

			using transitions = typename dfa_transitions::transitions;
			using states = typename dfa_transitions::states;
			using acceptings = typename accepter<outs, typename states::tailing, root::epsilon>::type;
		};

	}
}

#endif
