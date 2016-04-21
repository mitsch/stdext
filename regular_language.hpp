/// @file regular_language.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_REGULAR_LANGUAGE_HPP__
#define __STDEXT_REGULAR_LANGUAGE_HPP__

#include <stdext/array_view.hpp>
#include <stdext/integral_constant.hpp>
#include <stdext/integral_list.hpp>
#include <stdext/type_list.hpp>
#include <type_traits>

namespace stdext
{
	namespace regular_language
	{
	
		template <typename A, typename B, typename C, size_t ... D> struct _crosser;
		template <typename A, size_t ... B> struct _crosser<index_list<>, index_list<>, A, B ...> {using type = index_list<B ...>;};
		template <size_t A, size_t ... B, typename C, size_t ... D> struct _crosser<index_list<A, B ...>, index_list<>, C, D ...> : _crosser<index_list<B ...>, C, C, D ...> {};
		template <size_t A, size_t ... B, size_t C, size_t ... D, typename E, size_t ... F> struct _crosser<index_list<A, B ...>, index_list<C, D ...>, E, F ...> : _crosser<index_list<A, B ...>, index_list<D ...>, E, F ..., A, C> {};
		template <typename A, typename B> using crosser_t = typename _crosser<A, B, B>::type;



		template <bool A, typename B, typename ... C>
		struct set
		{
			using leafs = type_list<set<A, B, C ...>>;
		
			template <typename D> struct is_empty : true_type {};
			template <A D> struct is_empty<integral_constant<A, D>> : false_type {};
			template <A D, A E> struct is_empty<integral_list<A, D, E>> : boolean_type<D >= E> {};
			static constexpr auto epsilon = conjunction_v<is_empty<C> ...>;
		
			template <size_t E>
			struct transition_stack
			{
				using ins = index_list<E>;
				using outs = index_list<E>;
				using transitions = index_list<>;
				static constexpr auto count = E+1;
			};
			template <size_t E> using stack = transition_stack<E>;

			template <typename D, typename ... E> struct partitioner;
			template <typename D> struct partitioner<D> {using type = D;};
			template <typename D, typename E, typename ... F> struct partitioner<D, E, F ...>
				: partitioner<D::inserting<E>, F ...> {};

			using partitioning = partitioner<partition<integral_list<B>, boolean_constant<A>>, C ...>;
		};
		


		template <typename A, typename B>
		struct alternation
		{
			using leafs = A::leafs::concatenating<B::leafs>;
			static constexpr auto epsilon = A::epsilon or B::epsilon;

			template <size_t C>
			struct transition_stack
			{
				using _A = A::stack<C>;
				using _B = B::stack<_A::count>;
				using ins = _A::ins::inserting<_B::ins>;
				using outs = _A::outs::inserting<_B::outs>;
				using transitions = _A::transitions::concatenating<_B::transitions>;
				static constexpr auto count = _B::count;
			};
			template <size_t C> using stack = transition_stack<C>;
		};
		
		template <typename A, typename B>
		struct concatenation
		{
			using leafs = A::leafs::concatenating<B::leafs>;
			static constexpr auto epsilon = A::epsilon and B::epsilon;

			template <size_t C>
			struct transition_stack
			{
				using _A = A::stack<C>;
				using _B = B::stack<_A::count>;
				using ins = _A::ins::concatenating<std::conditional<A::epsilon, _B::ins, index_list<>>::type>;
				using outs = std::conditional<B::epsilon, _A::outs, index_list<>>::type::concatenating<_B::outs>;
				using transitions = _A::transitions::concatenating<_crosser<_A::ins, _B::outs>::type>::concatenating<_B::transitions>;
				static constexpr auto count = _B::count;
			};
			template <size_t C> using stack = transition_stack<C>;
		};
		
		template <typename A>
		struct closure
		{
			using leafs = A::leafs;
			static constexpr auto epsilon = true;

			template <size_t B>
			struct transition_stack
			{
				using _A = A::stack<B>;
				using ins = _A::ins;
				using outs = _A::outs;
				using transitions = _A::transitions::concatenating<_crosser<outs, ins>::type>;
				static constexpr auto count = _A::count;
			};
			template <size_t B> using stack = transition_stack<B>;
		};
		
		template <typename A>
		struct option
		{
			using leafs = A::leafs;
			static constexpr auto epsilon = true;
			template <size_t B> using stack = A::stack<B>;
		};


	

		template <typename A, typename B>
		struct partition;

		template <typename A, A ... B, bool C>
		struct partition<integral_list<A, B ...>, boolean_constant<C>>
		{
		
			template <bool D, typename E, typename F, typename G>
			struct inserter;



			template <A ... D, A E, A ... F, A G> requires G+1 < E
			struct inserter<false, integral_list<A, D...>, integral_list<A, E, F ...>, integral_constant<A, G>>
			{using type = partition<integral_list<A, D ..., G, G+1, E, F ...>, boolean_constant<C>>;};

			template <A ... D, A E, A ... F, A G> requires G+1 == E
			struct inserter<false, integral_list<A, D ...>, integral_list<A, E, F ...>, integral_constant<A, G>>
			{using type = partition<integral_list<A, D ..., G, F...>, boolean_constant<C>>;};

			template <A ... D, A E, A ... F>
			struct inserter<false, integral_list<A, D ...>, integral_list<A, E, F ...>, integral_constant<A, E>>
			{using type = partition<integral_list<A, D ..., E, F ...>, boolean_constant<C>>;};

			template <A ... D, A E, A ... F, A G> requires G > E
			struct inserter<false, integral_list<A, D ...>, integral_list<A, E, F ...>, integral_constant<A, G>>
				: inserter<true, integral_list<A, D ..., E>, integral_list<A, F ...>, integral_constant<A, G>>
			{};

			template <A ... D, A E>
			struct inserter<false, integral_list<A, D ...>, integral_list<A>, integral_constant<A, E>>
			{using type = partition<integral_list<A, D ..., E, E+1>, boolean_constant<C>>;};



			template <A ... D, A E, A ... F, A G> requires G < E
			struct inserter<true, integral_list<A, D ...>, integral_list<A, E, F ...>, integral_constant<A, G>>
			{using type = partition<integral_list<A, D ..., E, F ...>, boolean_constant<C>>;};

			template <A ... D, A E, A F, A ... G> requires E+1 == F
			struct inserter<true, integral_list<A, D ...>, integral_list<A, E, F, G ...>, integral_constant<A, E>>
			{using type = partition<integral_list<A, D ..., G ...>, boolean_constant<C>>;};

			template <A ... D, A E, A F, A ... G> requires E+1 < F
			struct inserter<true, integral_list<A, D ...>, integral_list<A, E, F, G ...>, integral_constant<A, E>>
			{using type = partition<integral_list<A, D ..., E+1, F, G ...>, boolean_constant<C>>;};

			template <A ... D, A E>
			struct inserter<true, integral_list<A, D ...>, integral_list<A, E>, integral_constant<A, E>>
			{using type = partition<integral_list<A, D ..., E+1>, boolean_constant<C>>;};

			template <A ... D, A E, A ... F, A G> requires G > E
			struct inserter<true, integral_list<A, D ...>, integral_list<A, E, F ...>, integral_constant<A, G>>
				: inserter<false, integral_list<A, D ..., E>, integral_list<A, F ...>, integral_constant<A, G>>
			{};

			template <A ... D, A E>
			struct inserter<true, integral_list<A, D ...>, integral_list<A>, integral_constant<A, E>>
			{using type = partition<integral_list<A, D ...>, boolean_constant<C>>;};


			
			template <A ... D, A E, A ... F, A G> requires G < E
			struct inserter<false, integral_list<A, D ...>, integral_list<A, E, F ...>, integral_list<A, G>>
			{using type = partition<integral_list<A, D ..., G, E, F ...>, boolean_constant<sizeof...(D) % 2 == 1>>;};

			template <A ... D, A E, A ... F>
			struct inserter<false, integral_list<A, D ...>, integral_list<A, E, F ...>, integral_list<A, E>>
			{using type = partition<integral_list<A, D ..., F ...>, boolean_constant<sizeof...(D) % 2 == 1>>;};

			template <A ... D, A E, A ... F, A G> requires G > E
			struct inserter<false, integral_list<A, D ...>, integral_list<A, E, F ...>, integral_list<A, G>>
				: inserter<true, integral_list<A, D ...>, integral_list<A, F ...>, integral_list<A, G>>
			{};

			template <A ... D, A E, A ... F, A G> requires G <= E
			struct inserter<true, integral_list<A, D ...>, integral_list<A, E, F ...>, integral_list<A, G>>
			{using type = partition<integral_list<A, D ..., E, F ...>, boolean_constant<sizeof...(D) % 2 == 0>>;};

			template <A ... D, A E, A ... F, A G> requires G > E
			struct inserter<true, integral_list<A, D ...>, integral_list<A, E, F ...>, integral_list<A, G>>
				: inserter<false, integral_list<A, D ...>, integral_list<A, F ...>, integral_list<A, G>>
			{};

			template <A ... D, A E>
			struct inserter<true, integral_list<A, D ...>, integral_list<A>, integral_list<A, E>>
			{using type = partition<integral_list<A, D ...>, boolean_constant<sizeof...(D) % 2 == 0>>;};




			template <bool D, typename E, typename F, A G>
			struct inserter<D, integral_list<A, E ...>, integral_list<A, F ...>, integral_list<A, G, G>>
			{using type = partition<integral_list<A, E ..., F ...>, boolean_constant<C>>;};
		
			template <A ... D, A E, A ... F, A G, A H> requires G < H and H < E
			struct inserter<false, integral_list<A, D ...>, integral_list<A, E, F ...>, integral_list<A, G, H>>
			{using type = partition<integral_list<A, D ..., G, H, E, F ...>, boolean_constant<C>>;};

			template <A ... D, A E, A ... F, A G> requires G < E
			struct inserter<false, integral_list<A, D ...>, integral_list<A, E, F ...>, integral_list<A, G, E>>
			{using type = partition<integral_list<A, D ..., G, F ...>, boolean_constant<C>>;};

			template <A ... D, A E, A ... F, A G, A H> requires G < E and E < H
			struct inserter<false, integral_list<A, D ...>, integral_list<A, E, F ...>, integral_list<A, G, H>>
				: inserter<true, integral_list<A, D ..., G>, integral_list<A, F ...>, integral_list<A, H>>
			{};

			template <A ... D, A E, A ... F, A G> requires E < G
			struct inserter<false, integral_list<A, D ...>, integral_list<A, E, F ...>, integral_list<A, E, G>>
				: inserter<true, integral_list<A, D ..., E>, integral_list<A, F ...>, integral_list<A, G>>
			{};

			template <A ... D, A E, A ... F, A G, A H> requires E < G and G < H
			struct inserter<false, integral_list<A, D ...>, integral_list<A, E, F ...>, integral_list<A, G, H>>
				: inserter<true, integral_list<A, D ..., E>, integral_list<A, F ...>, integral_list<A, G, H>>
			{};

			template <A ... D, A E, A F> requires E < F
			struct inserter<false, integral_list<A, D ...>, integral_list<A>, integral_list<A, E, F>>
			{using type = partition<integral_list<A, D ..., E, F>, boolean_constant<C>>;};

			template <A ... D, A E, A ... F, A G, A H> requires G < H and H <= E
			struct inserter<true, integral_list<A, D ...>, integral_list<A, E, F ...>, integral_list<A, G, H>>
			{using type = partition<integral_list<A, D ..., E, F ...>, boolean_constant<C>>;};

			template <A ... D, A E, A ... F, A G, A H> requires G < E and E < H
			struct inserter<true, integral_list<A, D ...>, integral_list<A, E, F ...>, integral_list<A, G, H>>
				: inserter<false, integral_list<A, D ..., E>, integral_list<A, F ...>, integral_list<A, H>>
			{};

			template <A ... D, A E, A ... F, A G> requires E < G
			struct inserter<true, integral_list<A, D ...>, integral_list<A, E, F ...>, integral_list<A, E, G>>
				: inserter<false, integral_list<A, D ..., E>, integral_list<A, F ...>, integral_list<A, G>>
			{};

			template <A ... D, A E, A ... F, A G, A H> requires E < G and G < H
			struct inserter<true, integral_list<A, D ...>, integral_list<A, E, F ...>, integral_list<A, G, H>>
				: inserter<false, integral_list<A, D ..., E>, integral_list<A, F ...>, integral_list<A, G, H>>
			{};

			template <A ... D, A E, A F> requires E < F
			struct inserter<true, integral_list<A, D ...>, integral_list<A>, integral_list<A, E, F>>
			{using type = partition<integral_list<A, D ...>, boolean_constant<C>>;};

			template <typename D> using inserting = inserter<C, integral_list<A>, integral_list<A, B ...>, D>::type;
		};



		template <typename A, A ... B, typename ... C>
		struct partition<integral_list<A, B ...>, type_list<C ...>>
		{
		
			template <typename A, typename B, typename C, bool D, typename E, typename F, typename G>
			struct inserter;

			template <A D, A ... E, typename F, typename ... G, A H, A ... I, typename J, typename K, typename L> requires H < D
			struct inserter<integral_list<A, D, E ...>, type_list<F, G ...>, integral_list<A, H, I ...>, false, J, K, L>
				: inserter<integral_list<A, D, E ...>, type_list<F, G ...>, integral_list<A, I ...>, true, J, K::appending<H>, K::appending<F>>
			{};

			template <A D, A ... E, typename F, typename ... G, A ... H, typename I, typename J, typename K>
			struct inserter<integral_list<A, D, E ...>, type_list<F, G ...>, integral_list<A, D, H ...>, false, I, J, K>
				: inserter<integral_list<A, E ...>, type_list<G ...>, integral_list<A, H ...>, true, I, J::appending<D>, K::appending<F>>
			{};

			template <A D, A ... E, typename F, typename ... G, A H, A ... I, typename J, typename K, typename L> requires H > D
			struct inserter<integral_list<A, D, E ...>, type_list<F, G ...>, integral_list<A, H, I ...>, false, J, K, L>
				: inserter<integral_list<A, E ...>, type_list<G ...>, integral_list<A, H, I ...>, false, J, K::appending<D>, L::appending<F>>
			{};

			template <A D, A ... E, typename F, typename G, typename H>
			struct inserter<integral_list<A>, type_list<>, integral_list<A, D, E ...>, false, F, G, H>
				: inserter<integral_list<A>, type_list<>, integral_list<A, E ...>, true, F, G, H>
			{};

			template <A D, A ... E, typename F, typename ... G, typename H, typename I, typename J>
			struct inserter<integral_list<A, D, E ...>, type_list<F, G ...>, integral_list<A>, false, H, I, J>
			{
				using keys = I::appending<D, E ...>;
				using values = J::appending<F, G ...>;
			};

			template <typename D, typename E, typename F>
			struct inserter<integral_list<A>, type_list<>, integral_list<A>, false, D, E, F>
			{
				using keys = E;
				using values = F;
			};

			template <typename D, typename E, typename F, typename G>
			struct inserter<integral_list<A>, type_list<D>, integral_list<A>, false, E, F, G>
			{
				using keys = F;
				using values = G::appending<D>;
			};

			template <A D, A ... E, typename F, typename ... G, A H, A ... I, typename J, typename K, typename L> requires H < D
			struct inserter<integral_list<A, D, E ...>, type_list<F, G ...>, integral_list<A, H, I ...>, true, J, K, L>
				: inserter<integral_list<A, D, E ...>, type_list<F, G ...>, integral_list<A, I ...>, false, J, K::appending<H>, L::appending<F::inserting<J>>>
			{};
		
			template <A D, A ... E, typename F, typename ... G, A ... H, typename I, typename J, typename K>
			struct inserter<integral_list<A, D, E ...>, type_list<F, G ...>, integral_list<A, D, H ...>, true, I, J, K>
				: inserter<integral_list<A, E ...>, type_list<G ...>, integral_list<A, H ...>, false, I, J::appending<D>, K::appending<F::inserting<I>>>
			{};

			template <A D, A ... E, typename F, typename ... G, A H, A ... I, typename J, typename K, typename L> requires H > D
			struct inserter<integral_list<A, D, E ...>, type_list<F, G ...>, integral_list<A, H, I ...>, true, J, K, L>
				: inserter<integral_list<A, E ...>, type_list<G ...>, integral_list<A, H, I ...>, true, J, K::appending<D>, L::appending<F::inserting<J>>>
			{};

			template <A D, A ... E, size_t F, typename G, typename H>
			struct inserter<integral_list<A>, type_list<>, integral_list<A, D, E ...>, true, index_constant<F>, G, H>
				: inserter<integral_list<A>, type_list<>, integral_list<A, E ...>, false, F, G::appending<D>, H::appending<index_list<F>>>
			{};

			template <A D, A ... E, typename  F, typename G, typename H>
			struct inserter<integral_list<A>, type_list<>, integral_list<A, D, E ...>, true, F, G, H>
				: inserter<integral_list<A>, type_list<>, integral_list<A, E ...>, false, F, G::appending<D>, H::appending<F>>
			{};

			template <A D, A ... E, typename F, typename ... G, typename H, typename I, typename J>
			struct inserter<integral_list<A, D, E ...>, type_list<F, G ...>, integral_list<A>, true, H, I, J>
				: inserter<integral_list<A, E ...>, type_list<G ...>, integral_list<A>, true, H, I::appending<D>, J::appending<F::inserting<H>>>
			{};

			template <size_t D, typename E, typename F>
			struct inserter<integral_list<A>, type_list<>, integral_list<A>, true, index_constant<D>, E, F>
			{
				using keys = E;
				using values = F::appending<index_list<D>>;
			};

			template <typename D, typename E, typename F>
			struct inserter<integral_list<A>, type_list<>, integral_list<A>, true, D, E, F>
			{
				using keys = E;
				using values = F::appending<D>;
			};

			template <typename D, typename E, typename F, typename G>
			struct inserter<integral_list<A>, type_list<D>, integral_list<A>, true, E, F, G>
			{
				using keys = F;
				using values = G::appending<D::inserting<E>>;
			};

			template <typename D, typename E>
			struct _inserter;

			template <bool D, A ... E, size_t F>
			struct _inserter<partition<integral_list<A, E ...>, boolean_constant<D>>, index_constant<F>>
			{
				using target = inserter<integral_list<A, B ...>, type_list<C ...>, integral_List<A, E ...>, D, index_list<F>, integral_list<A>, type_list<>>;
				using keys = typename target::keys;
				using values = typename target::values;
				using type = typename partition<keys, values>;
			};

			template <typename D, typename E> using inserting = typename _inserter<D, E>::type;
		};

	}
}

#endif

