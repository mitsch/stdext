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


	/// Set of integral values computed at compile time
	///
	/// An integral set allows to handle integral values at compile time. No duplicates will be stored. No attribures will be stored
	/// Hence every object of an integral set will effectively be empty and can therefore can be optimised. At runtime, these objects
	/// won't be mutual. The object stores its elements in a comprised way. To instantiate any set object, see \a empty_set,
	/// \a full_set, \a set_from_constant as well \a set_from_list.
	template <typename A, bool B, A ... C> struct set;

	
	template <typename A> using empty_set = set<A, false>;
	template <typename A> using full_set = set<A, true>;


	/// Complement of set
	///
	/// A complement of a set contains all elements which are not contained by the set itself. The set (first and only argument) must
	/// have type \a stdext::integral::set. The resulting set will be declared by \a type. An abbreviation with the same result is
	/// \a stdext::integral::set_complement_t.
	///
	/// @code
	/// using supposed_empty = stdext::integral::set_complement<stdext::integral::full_set>::type;
	/// static_assert(std::is_same_v<supposed_empty, empty_set>, "The set mus be empty!");
	/// @endcode
	///
	///  @{
	template <typename A> struct set_complement {};
	template <typename A, bool B, A ... C> struct set_complement<set<A, B, C ...>> {using type = set<A, not B, C ...>;};
	/// @}

	/// Complement of set
	///
	/// This is an abbreviation for calling the resulting type of a set's complementary. The implementation will have the
	/// same beaviour as calling \a stdext::integral::set_complement<A>::type directly.
	template <typename A> using set_complement_t = typename set_complement<A>::type;




	/// Element set testing
	///
	/// A value (second argument) is tested on being an element in a set (first argument). The set (first argument) must have
	/// type \a stdext::integral::set and the value (second argument) must have type \a stdext::integral::constant. The resulting
	/// boolean value will be declared both by \a type and by the base class. Both will have some type \a stdext::integral::boolean_constant.
	/// An abbreviation with the same result is \a stdext::integral::set_element_t.
	///
	/// @code
	/// using some_values = stdext::integral::list<int, 0, 1, 2, 3, 4, 5>;
	/// using some_set = stdext::integral::set_from_list_t<some_values>;
	/// static_assert(stdext::integral::set_element<some_set, stdext::integral::constant<int, 0>>::value, "Zero should be contained!");
	/// using ten_testing = stdext::integral::set_element<some_set, stdext::integral::constant<int, 10>>::type;
	/// static_assert(not ten_testing::value, "Ten should not be contained!");
	/// @endcode
	///
	/// @{
	template <typename, typename, typename ...> struct set_element {};
	template <typename A, A B, A ... C, A D> struct set_element<set<A, true, B, C ...>, constant<A, D>> :
		set_element<set<A, true, B, C ...>, constant<A , D>, boolean_constant<D < B>> {};
	template <typename A, A B, A ... C, A D> struct set_element<set<A, true, B, C ...>, constant<A, D>, true_constant> : true_constant {};
	template <typename A, A B, A ... C, A D> struct set_element<set<A, true, B, C ...>, constant<A, D>, false_constant> :
		set_element<set<A, false, C ...>, constant<A, D>> {};
	template <typename A, A B, A ... C, A D> struct set_element<set<A, false, B, C ...>, constant<A, D>, true_constant> : false_constant {};
	template <typename A, A B, A ... C, A D> struct set_element<set<A, false, B, C ...>, constant<A, D>, false_constant> :
		set_element<set<A, true, C ...>, constant<A, D>> {};
	template <typename A, bool B, A C> struct set_element<set<A, B>, constant<A, C>> : boolean_constant<B> {};
	/// @}

	// TODO specialisation of set_element for boolean values

	/// Element set testing
	///
	/// This is an abbreviation for calling the resulting value of element testing on a set. The implementation will have the same
	/// behaviour as calling \a stdext::integral::set_element<A, B>::value directly.
	template <typename A, typename B> constexpr auto set_element_v = set_element<A, B>::value;


	

	/// Union of two sets
	///
	/// The union of two sets contains all elements which are contained in either of the two sets. The two sets must have some
	/// type \a stdext::integral::set. The resulting set will be declared by \a type. An abbreviation with the same result is
	/// \a stdext::integral::set_union_t.
	///
	/// @code
	/// using even_list = stdext::integral::list<unsigned int, 0, 2, 4, 6, 8>;
	/// using odd_list = stdext::integral::list<unsigned int, 1, 3, 5, 7, 9>;
	/// using even_set = stdext::integral::set_from_list_t<even_list>;
	/// using odd_set = stdext::integral::set_from_list_t<odd_list>;
	/// using from_zero_to_nine = stdext::integral::set_union<even_set, odd_set>::type;
	/// @endcode
	///
	/// @{
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
		: set_union<set<A, B, C, D ...>, set<A, E, F, G ...>, set<A, H, I ...>, boolean_constant<C < F>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_union<set<A, true, B, C ...>, set<A, true, D, E ...>, set<A, F, G ...>, true_constant>
		: set_union<set<A, false, C ...>, set<A, true, D, E ...>, set<A, F, G ...>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_union<set<A, true, B, C ...>, set<A, true, D, E ...>, set<A, F, G ...>, false_constant>
		: set_union<set<A, true, B, C ...>, set<A, false, E ...>, set<A, F, G ...>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_union<set<A, false, B, C ...>, set<A, true, D, E ...>, set<A, F, G ...>, true_constant>
		: set_union<set<A, true, C ...>, set<A, true, D, E ...>, set<A, F, G ...>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_union<set<A, false, B, C ...>, set<A, true, D, E ...>, set<A, F, G ...>, false_constant>
		: set_union<set<A, false, B, C ...>, set<A, false, E ...>, set<A, F, G ..., D>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_union<set<A, true, B, C ...>, set<A, false, D, E ...>, set<A, F, G ...>, true_constant>
		: set_union<set<A, false, C ...>, set<A, false, D, E ...>, set<A, F, G ..., B>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_union<set<A, true, B, C ...>, set<A, false, D, E ...>, set<A, F, G ...>, false_constant>
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
	/// @}

	// TODO specialisatio of set_union for boolean values

	/// Union of two sets
	///
	/// This is an abbreviation for calling the resulting type of the union of two sets. The implementation will have the
	/// same behaviour as calling \a stdext::integral::set_union<A, B>::type directly.
	template <typename A, typename B> using set_union_t = typename set_union<A, B>::type;




	/// Intersection of two sets
	///
	/// The result of an intersection of two sets is a set with all elements which are contained by these two sets. Both sets must have
	/// type \a stdext::integral::set. The resulting set will be declared by \a type. An abbreviation with the same result is
	/// \a stdext::integral::set_intersection_t.
	///
	/// @code
	/// using no_positives_list = stdext::integral::list<int, -5, -4, -3, -2, -1, 0>;
	/// using no_negatves_list = stdext::integral::list<int, 0, 1, 2, 3, 4, 5>;
	/// using no_positives_set = stdext::integral::set_from_list_t<no_positives_list>;
	/// using no_negatives_set = stdext::integral::set_from_list_t<no_negatives_list>;
	/// using no_negatives_positives = stdext::integral::set_intersection<no_positives_set, no_negatives_set>::type;
	/// @endcode
	///
	/// @{
	template <typename, typename, typename ...> struct set_intersection {};
	template <typename A, bool B, A ... C, bool D, A ... E> struct set_intersection<set<A, B, C ...>, set<A, D, E ...>>
		: set_intersection<set<A, B, C ...>, set<A, D, E ...>, set<A, B and D>> {};
	template <typename A, A B, A ... C, A ... D, bool E, A ... F> struct set_intersection<set<A, true, B, C ...>, set<A, true, B, D ...>, set<A, E, F ...>>
		: set_intersection<set<A, false, C ...>, set<A, false, D ...>, set<A, E, F ..., B>> {};
	template <typename A, A B, A ... C, A ... D, bool E, A ... F> struct set_intersection<set<A, false, B, C ...>, set<A, true, B, D ...>, set<A, E, F ...>>
		: set_intersection<set<A, false, B, C ...>, set<A, false, D ...>, set<A, E, F ...>> {};
	template <typename A, A B, A ... C, A ... D, bool E, A ... F> struct set_intersection<set<A, true, B, C ...>, set<A, false, B, D ...>, set<A, E, F ...>>
		: set_intersection<set<A, false, C ...>, set<A, false, B, D ...>, set<A, E, F ...>> {};
	template <typename A, A B, A ... C, A ... D, bool E, A ... F> struct set_intersection<set<A, false, B, C ...>, set<A, false, B, D ...>, set<A, E, F ...>>
		: set_intersection<set<A, true, C ...>, set<A, true, D ...>, set<A, E, F ..., B>> {};
	template <typename A, bool B, A C, A ... D, bool E, A F, A ... G, bool H, A ... I> struct set_intersection<set<A, B, C, D ...>, set<A, E, F, G ...>, set<A, H, I ...>>
		: set_intersection<set<A, B, C, D ...>, set<A, E, F, G ...>, set<A, H, I ...>, boolean_constant<C < F>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_intersection<set<A, true, B, C ...>, set<A, true, D, E ...>, set<A, F, G ...>, true_constant>
		: set_intersection<set<A, false, C ...>, set<A, true, D, E ...>, set<A, F, G ..., B>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_intersection<set<A, true, B, C ...>, set<A, true, D, E ...>, set<A, F, G ...>, false_constant>
		: set_intersection<set<A, true, B, C ...>, set<A, false, E ...>, set<A, F, G ..., D>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_intersection<set<A, true, B, C ...>, set<A, false, D, E ...>, set<A, F, G ...>, true_constant>
		: set_intersection<set<A, false, C ...>, set<A, false, D, E ...>, set<A, F, G ...>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_intersection<set<A, true, B, C ...>, set<A, false, D, E ...>, set<A, F, G ...>, false_constant>
		: set_intersection<set<A, true, B, C ...>, set<A, true, E ...>, set<A, F, G ..., D>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_intersection<set<A, false, B, C ...>, set<A, true, D, E ...>, set<A, F, G ...>, true_constant>
		: set_intersection<set<A, true, C ...>, set<A, true, D, E ...>, set<A, F, G ..., B>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_intersection<set<A, false, B, C ...>, set<A, true, D, E ...>, set<A, F, G ...>, false_constant>
		: set_intersection<set<A, false, B, C ...>, set<A, false, E ...>, set<A, F, G ...>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_intersection<set<A, false, B, C ...>, set<A, false, D, E ...>, set<A, F, G ...>, true_constant>
		: set_intersection<set<A, true, C ...>, set<A, false, D, E ...>, set<A, F, G ...>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_intersection<set<A, false, B, C ...>, set<A, false, D, E ...>, set<A, F, G ...>, false_constant>
		: set_intersection<set<A, false, B, C ...>, set<A, true, E ...>, set<A, F, G ...>> {};
	template <typename A, bool B, A C, A ... D, bool E, A ... F> struct set_intersection<set<A, B, C, D ...>, set<A, true>, set<A, E, F ...>>
		{using type = set<A, E, F ..., C, D ...>;};
	template <typename A, bool B, A C, A ... D, bool E, A ... F> struct set_intersection<set<A, true>, set<A, B, C, D ...>, set<A, E, F ...>>
		{using type = set<A, E, F ..., C, D ...>;};
	template <typename A, bool B, A C, A ... D, bool E, A ... F> struct set_intersection<set<A, B, C, D ...>, set<A, false>, set<A, E, F ...>>
		{using type = set<A, E, F ...>;};
	template <typename A, bool B, A C, A ... D, bool E, A ... F> struct set_intersection<set<A, false>, set<A, B, C, D ...>, set<A, E, F ...>>
		{using type = set<A, E, F ...>;};
	template <typename A, bool B, bool C, bool D, A ... E> struct set_intersection<set<A, B>, set<A, C>, set<A, D, E ...>>
		{using type = set<A, D, E ...>;};
	/// @}

	// TODO specialisation of set_intersection for boolean values

	/// Intersection of two sets
	///
	/// This is an abbeviation for calling the resulting type of building the intersection of two sets. The implementation will
	/// have the same behaviour as calling \a stdext::integral::set_intersection<A, B>::type directly.
	template <typename A, typename B> using set_intersection_t = typename set_intersection<A, B>::type;




	/// Set difference which is a set containing all elements of the first set (first argument) that are not elements of the second set (second argument)
	///
	/// The result of a set difference is a set which contains exactly all elements of the first set which are not elements of the second set.
	/// Both sets must have some type \a stdext::integral::set. The resulting set will be declared by \a type. An abbreviation with the same
	/// result is \a stdext::integral::set_difference_t.
	///
	/// @code
	/// using some_elements = stdext::integral::list<int, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9>;
	/// using some_other_elements = stdext::integral::list<int, 1, 3, 5, 7, 9>;
	/// using some_set = stdext::integral::set_from_list_t<some_elements>;
	/// using some_other_set = stdext::integral::set_from_list_t<some_other_elements>;
	/// using even_element_set = stdext::integral::set_difference<some_set, some_other_set>::type;
	/// @endcode
	///
	/// @{
	template <typename, typename, typename ...> struct set_difference {};
	template <typename A, bool B, A ... C, bool D, A ... E> struct set_difference<set<A, B, C ...>, set<A, D, E ...>>
		: set_difference<set<A, B, C ...>, set<A, D, E ...>, set<A, B and not D>> {};
	template <typename A, A B, A ... C, A ... D, bool E, A ... F> struct set_difference<set<A, true, B, C ...>, set<A, true, B, D ...>, set<A, E, F ...>>
		: set_difference<set<A, false, C ...>, set<A, false, D ...>, set<A, E, F ...>> {};
	template <typename A, A B, A ... C, A ... D, bool E, A ... F> struct set_difference<set<A, false, B, C ...>, set<A, true, B, D ...>, set<A, E, F ...>>
		: set_difference<set<A, false, B, C ...>, set<A, false, D ...>, set<A, E, F ...>> {};
	template <typename A, A B, A ... C, A ... D, bool E, A ... F> struct set_difference<set<A, true, B, C ...>, set<A, false, B, D ...>, set<A, E, F ...>>
		: set_difference<set<A, false, C ...>, set<A, false, B, D ...>, set<A, E, F ..., B>> {};
	template <typename A, A B, A ... C, A ... D, bool E, A ... F> struct set_difference<set<A, false, B, C ...>, set<A, false, B, D ...>, set<A, E, F ...>>
		: set_difference<set<A, true, C ...>, set<A, true, D ...>, set<A, E, F ...>> {};
	template <typename A, bool B, A C, A ... D, bool E, A F, A ... G, bool H, A ... I> struct set_difference<set<A, B, C, D ...>, set<A, E, F, G ...>, set<A, H, I ...>>
		: set_difference<set<A, B, C, D ...>, set<A, E, F, G ...>, set<A, H, I ...>, boolean_constant<C < F>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_difference<set<A, true, B, C ...>, set<A, true, D, E ...>, set<A, F, G ...>, true_constant>
		: set_difference<set<A, false, C ...>, set<A, true, D, E ...>, set<A, F, G ...>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_difference<set<A, true, B, C ...>, set<A, true, D, E ...>, set<A, F, G ...>, false_constant>
		: set_difference<set<A, true, B, C ...>, set<A, false, E ...>, set<A, F, G ...>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_difference<set<A, false, B, C ...>, set<A, true, D, E ...>, set<A, F, G ...>, true_constant>
		: set_difference<set<A, true, C ...>, set<A, true, D, E ...>, set<A, F, G ...>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_difference<set<A, false, B, C ...>, set<A, true, D, E ...>, set<A, F, G ...>, false_constant>
		: set_difference<set<A, false, B, C ...>, set<A, false, E ...>, set<A, F, G ...>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_difference<set<A, true, B, C ...>, set<A, false, D, E ...>, set<A, F, G ...>, true_constant>
		: set_difference<set<A, false, C ...>, set<A, false, D, E ...>, set<A, F, G ..., B>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_difference<set<A, true, B, C ...>, set<A, false, D, E ...>, set<A, F, G ...>, false_constant>
		: set_difference<set<A, true, B, C ...>, set<A, true, E ...>, set<A, F, G ..., D>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_difference<set<A, false, B, C ...>, set<A, false, D, E ...>, set<A, F, G ...>, true_constant>
		: set_difference<set<A, true, C ...>, set<A, false, D, E ...>, set<A, F, G ..., B>> {};
	template <typename A, A B, A ... C, A D, A ... E, bool F, A ... G> struct set_difference<set<A, false, B, C ...>, set<A, false, D, E ...>, set<A, F, G ...>, false_constant>
		: set_difference<set<A, false, B, C ...>, set<A, true, E ...>, set<A, F, G ..., B>> {};
	template <typename A, bool B, A C, A ... D, bool E, A ... F> struct set_difference<set<A, B, C, D ...>, set<A, true>, set<A, E, F ...>>
		{using type = set<A, E, F ...>;};
	template <typename A, bool B, A C, A ... D, bool E, A ... F> struct set_difference<set<A, B, C, D ...>, set<A, false>, set<A, E, F ...>>
		{using type = set<A, E, F ..., C, D ...>;};
	template <typename A, bool B, bool C, A ... D, bool E, A ... F> struct set_difference<set<A, B>, set<A, C, D ...>, set<A, E, F ...>>
		{using type = set<A, E, F ...>;};
	/// @}

	/// Set difference
	///
	/// This is an abbreviation for calling the resulting type of building the difference of two sets. The implementation will
	/// have the same behaviour as calling stdext::integral::set_difference<A, B>::type directly.
	template <typename A, typename B> using set_difference_t = typename set_difference<A, B>::type;


	// TODO specialisation of set_difference for boolean_values


	 
	/// Set instantiation with a single element
	///
	/// A set will be instantiated with a single element which is the first argument and must be have type \a stdext::integral::constant.
	/// The resulting set will be declared by \a type. An abbreviation with the same result is \a stdext::integral::set_from_constant_t.
	///
	/// @code
	/// using some_element = stdext::integral::constant<unsigned int, 0>;
	/// using and_the_set = stdext::integral::set_from_constant<some_element>::type;
	/// @endcode
	///
	/// @{
	template <typename, typename ...> struct set_from_constant {};
	template <typename A, A B> struct set_from_constant<constant<A, B>> : set_from_constant<constant<A, B>, B<B+1> {};
	template <typename A, A B> struct set_from_constant<constant<A, B>, true> {using type = set<A, false, B, B+1>;};
	template <typename A, A B> struct set_from_constant<constant<A, B>, false> {using type = set<A, false, B>;};
	/// @}


	/// Set instantiation with a single element
	///
	/// This is an abbreviation for calling the resulting type of building a set with a single element. The implementation will
	/// have the same behaviour as calling \a stdext::integral::set_from_constant<A>::type directly.
	template <typename A> using set_from_constant_t = set_from_constant<A>::type;



	/// Set instantiation with a list of elements
	///
	/// A set will be instantiated with a list of elements which must be have type \a stdext::integral::list. The list will be the first and only
	/// argument. The resulting type will be declared by \a type. An abbreviation with the same result is \a stdext::integral::set_from_list_t.
	///
	/// @code
	/// using some_elements = stdext::integral::list<unsigned int, 0, 0, 4, 3, 1, 29, 139, 0, 1>;
	/// using and_the_set = stdext::integral::set_from_list<some_elements>::type;
	/// using and_an_empty_set = stdext::integral::set_from_list<stdext::integral::empty_list<int>>::type;
	/// @endcode
	///
	/// @{
	template <typename, typename ...> struct set_from_list {};
	template <typename A, A ... B> struct set_from_list<list<A, B ...>> : set_from_list<typename list_uniq_t<typename list_sort_t<list<A, B ...>>>, list<A>> {};
	template <typename A> struct set_from_list<list<A>, list<A>> {using type = set<A, false>;};
	template <typename A, A B, A ... C> struct set_from_list<list<A, B, C ...>, list<A>> : set_from_list<list<A, C ...>, list<A, B>, B+1, B < B+1> {};
	template <typename A, A B, A ... C, A ... D> struct set_from_list<list<A, B, C ...>, list<A, D ...>, B, true>
		: set_from_list<list<A, C ...>, list<A, D ...>, B+1, B < B+1> {};
	template <typename A, A B, A ... C, A ... D, A E> struct set_from_list<list<A, B, C ...>, list<A, D ...>, E, true>
		: set_from_list<list<A, C ...>,  list<A, D ..., E, B>, B < B+1> {};
	template <typename A, A ... B, A C> struct set_from_list<list<A>, list<A, B ...>, C, true> {using type = set<A, B ..., C>;};
	template <typename A, A ... B, A C> struct set_from_list<list<A>, list<A, B ...>, C, false> {using type = set<A, B ...>;};
	/// @}


	// TODO specialisation of set_from_list for boolean values

	/// Set instantiation with a list of elements
	///
	/// This is an abbreviation for calling the resulting type of building a set with a list of elements. The implementation will have the same
	/// behaviour as calling \a stdext::integral::set_from_list<A>::type directly.
	template <typename A> using set_from_list_t = typename set_from_list<A>::type;



	/// Set to list convertion
	///
	/// The set (first and only argument) will be converted into a list with all elements in the set. The order of the elements in the list will
	/// be ascending. The resulting type will be declared as \a type. An abbreviation with the same result is \a stdext::integral::set_to_list_t.
	///
	/// @code
	/// using some_list = stdext::integral::list<int, 0, 0, 32, 1, 0, -123, -23, -23, 32, 10432>;
	/// using and_the_set = stdext::integral::set_from_list_t<some_list>;
	/// using uniqed_sorted_elements_in_list = stdext::integral::set_to_list<and_the_set>::type;
	/// @endcode
	///
	/// {
	template <typename, typename ...> struct set_to_list {};
	template <typename A, A B, A C, A ... D> struct set_to_list<set<A, false, B, C, D ...>>
		: set_to_list<set<A, true, C, D ...>, constant<A, B>, list<A>> {};
	template <typename A, A B> struct set_to_list<set<A, false, B>>
		: set_to_list<set<A, true>, constant<A, B+1>, list<A, B>, boolean_constant<B < B+1>> {};
	// TODO define min_value	
	template <typename A, A B, A ... C> struct set_to_list<set<A, true, B, C ...>>
		: set_to_list<set<A, true, B, C ...>, min_value<A>::type, list<A>> {};
	// TODO define min_value
	template <typename A> struct set_to_list<set<A, true>>
		: set_to_list<set<A, true>, constant<A, min_value<A>::value+1>, list<A, min_value<A>::value>, boolean_constant<min_value<A>::value < min_value<A>::value+1>> {};
	template <typename A, A B, A C, A D, A ... E, A ... F> struct set_to_list<set<A, true, B, C, D, E ...>, constant<A, B>, list<A, F ...>>
		: set_to_list<set<A, true, D, E ...>, constant<A, C>, list<A, F ...>> {};
	template <typename A, A B, A C, A ... D> struct set_to_list<set<A, true, B, C>, constant<A, B>, list<A, D ...>>
		: set_to_list<set<A, true>, constant<A, C+1>, list<A, D ..., C>, boolean_constant<C < C+1>> {};
	template <typename A, A B, A ... C> struct set_to_list<set<A, true, B>, constant<A, B>, list<A, C ...>> {using type = list<A, C ...>;};
	template <typename A, A B, A ... C, A D, A ... E> struct set_to_list<set<A, true, B, C ...>, constant<A, D>, list<A, E ...>>
		: set_to_list<set<A, true, B, C ...>, constant<A, D+1>, list<A, E ..., D>> {};
	template <typename A, A B, A ... C> struct set_to_list<set<A, true>, constant<A, B>, list<A, C ...>, true_constant>
		: set_to_list<set<A, true>, constant<A, B+1>, list<A, C ..., B>, boolean_constant<B < B+1>> {};
	template <typename A, A B, A ... C> struct set_to_list<set<A, true>, constant<A, B>, list<A, C ...>, false_constant> {using type = list<A, C ...>;};
	/// @}

	// TODO specialisation of set_to_list for boolean_values

	/// List instantiation with a set
	///
	/// This is an abbreviation for calling the resulting type of building a list with a set of elements. The implementation has the same
	/// behaviour as calling \a stdext::integral::set_to_list<A>::type directly.
	template <typename A> using set_to_list_t = typename set_to_list<A>::type;

}
}

#endif

