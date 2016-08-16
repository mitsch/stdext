/// @file set.hpp
/// @module integral
/// @library stdext
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT__INTEGRAL__SET
#define __STDEXT__INTEGRAL__SET

namespace stdext
{
namespace integral
{

	template <typename A, bool B, A ... C> struct set;

	
	template <typename A> using empty_set = set<A, false>;
	template <typename A> using full_set = set<A, true>;

	
	template <typename A> struct set_negation {};
	template <typename A, bool B, A ... C> struct set_negation<set<A, B, C ...>> {using type = set<A, not B, C ...>;};


	// TODO set_interaction, set_difference, set_from_constant, set_from_list, set_to_list
	

	// Element testing in a set
	template <typename, typename, typename ...> struct set_element {};
	template <typename A, A B, A ... C, A D> struct set_element<set<A, true, B, C ...>, constant<A, D>> :
		set_element<set<A, true, B, C ...>, constant<A , D>, D < B> {};
	template <typename A, A B, A ... C, A D> struct set_element<set<A, true, B, C ...>, constant<A, D>, true> : true_type {};
	template <typename A, A B, A ... C, A D> struct set_element<set<A, true, B, C ...>, constant<A, D>, false> :
		set_element<set<A, false, C ...>, constant<A, D>> {};
	template <typename A, A B, A ... C, A D> struct set_element<set<A, false, B, C ...>, constant<A, D>, true> : false_type {};
	template <typename A, A B, A ... C, A D> struct set_element<set<A, false, B, C ...>, constant<A, D>, false> :
		set_element<set<A, true, C ...>, constant<A, D>> {};
	template <typename A, bool B, A C> struct set_element<set<A, B>, constant<A, C>> : boolean_constant<B> {};

	
	template <typename, typename, typename ...> struct set_union {};
	template <typename A, bool B, A ... C, bool D, A ... E> struct set_union<set<A, B, C ...>, set<A, D, E ...>>
		: set_union<set<A, B, C ...>, set<A, D, E ...>, set<A, B or C>> {};
	template <typename A, A B, A ... C, A ... D, bool E, A ... F> struct set_union<set<A, true, B, C ...>, set<A, true, B, D ...>, set<A, E, F ...>>
		: set_union<set<A, false, C ...>, set<A, false, D ...>, set<A, E, F ..., B>> {};
	template <typename A, A B, A ... C, A ... D, bool E, A ... F> struct set_union<set<A, false, B, C ...>, set<A, true, B, D ...>, set<A, E, F ...>>
		: set_union<set<A, true, C ...>, set<A, false, D ...>, set<A, E, F ...>> {};
	template <typename A, A B, A ... C, A ... D, bool E, A ... F> struct set_union<set<A, true, B, C ...>, set<A, false, B, D ...>, set<A, E, F ...>>
		: set_union<set<A, false, C ...>, set<A, true, D ...>, set<A, E, F ...>> {};
	template <typename A, A B, A ... C, A ... D, bool E, A ... F> struct set_union<set<A, false, B, C ...>, set<A, false, B, D ...>, set<A, E, F ...>>
		: set_union<set<A, true, C ...>, set<A, true, D ...>, set<A, E, F ..., B>> {};
	template <typename A, bool B, A C, A ... D, bool E, A F, A ... G, bool H, A ... I> struct set_union<set<A, B, C, D ...>, set<A, E, F, G ...>, set<A, H, I ...>>
		: set_union<set<A, B, C, D ...>, set<A, E, F, G ...>, set<A, H, I ...>, C < F> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_union<set<A, true, B, C ...>, set<A, true, D, E ...>, set<A, F, G ...>, true>
		: set_union<set<A, false, C ...>, set<A, true, D, E ...>, set<A, F, G ...>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_union<set<A, true, B, C ...>, set<A, true, D, E ...>, set<A, F, G ...>, false>
		: set_union<set<A, true, B, C ...>, set<A, false, E ...>, set<A, F, G ...>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_union<set<A, false, B, C ...>, set<A, true, D, E ...>, set<A, F, G ...>, true>
		: set_union<set<A, true, C ...>, set<A, true, D, E ...>, set<A, F, G ...>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_union<set<A, false, B, C ...>, set<A, true, D, E ...>, set<A, F, G ...>, false>
		: set_union<set<A, false, B, C ...>, set<A, false, E ...>, set<A, F, G ..., D>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_union<set<A, true, B, C ...>, set<A, false, D, E ...>, set<A, F, G ...>, true>
		: set_union<set<A, false, C ...>, set<A, false, D, E ...>, set<A, F, G ..., B>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_union<set<A, true, B, C ...>, set<A, false, D, E ...>, set<A, F, G ...>, false>
		: set_union<set<A, true, B, C ...>, set<A, true, E ...>, set<A, F, G ...>> {};
	template <typename A, bool B, A C, A ... D, bool E, A ... F> struct set_union<set<A, B, C, D ...>, set<A, true>, set<A, E, F ...>>
		{using type = set<A, E, F ...>;};
	template <typename A, bool B, A C, A ... D, bool E, A ... F> struct set_union<set<A, B, C, D ...>, set<A, false>, set<A, E, F ...>>
		{using type = set<A, E, F ..., C, D ...>;};
	template <typename A, bool B, A C, A ... D, bool E, A ... F> struct set_union<set<A, true>, set<A, B, C, D ...>, set<A, E, F ...>>
		{using type = set<A, E, F ...>;};
	template <typename A, bool B, A C, A ... D, bool E, A ... F> struct set_union<set<A, false>, set<A, B, C, D ...>, set<A, E, F ...>>
		{using type = set<A, E, F ..., C, D ...>;};
	template <typename A, bool B, bool C, bool D, A ... E> struct set_union<set<A, B>, set<A, C>, set<A, D, E ...>>
		{using type = set<A, D, E ...>;};
	
	 
	
	

	template <typename, typename ...> struct set_constant {};
	template <typename A, A B> struct set_constant<constant<A, B>> : set_constant<constant<A, B>, B<B+1> {};
	template <typename A, A B> struct set_constant<constant<A, B>, true> {using type = set<A, false, B, B+1>;};
	template <typename A, A B> struct set_constant<constant<A, B>, false> {using type = set<A, false, B>;};



}
}

#endif

