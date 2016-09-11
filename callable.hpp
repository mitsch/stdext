/// @file callable.hpp
/// @library stdext
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef STDEXT__CALLABLE__HPP
#define STDEXT__CALLABLE__HPP

#include <stdext/integral/constant.hpp>
#include <stdext/sfinae.hpp>

// TODO the header for std::declval is required
// TODO may require an invoke implementation (check std:invoke)


namespace stdext
{

	/// Return type deduction of a call
	///
	/// The return type of a call is deducted. The first argument takes the signature of the call, that is the callee type and
	/// the types of arguments. The second argument is reserved and won't be used directly. The deducted type will be
	/// declared in \a type. For a shortcut, use \a result_of_t. The trait is sfinae friendly, that is if the return type cannot be
	/// deducted for any reason, \a type will not be defined.
	///
	/// @code
	/// int foo (const char, int)
	/// {
	///   return 0;
	/// }
	///
	/// class Bar
	/// {
	///   public:
	///
	///   float bar (int, float)
	///   {
	///      return 0.5f; 
	///   }
	/// };
	///
	/// using first_type = result_of_t<foo(const char, int)>; // should be int
	/// using second_type = result_of_t<Bar::bar(int, float)>; // should be float
	/// @endcode
	///
	/// @{
	template <typename, typename = void> struct result_of {};
	template <typename A, typename ... B> struct result_of<A(B ...), void_t<decltype(std::declval<A>()(std::declval<B>() ...))>> {using type = decltype(std::declval<A>()(std::declval<B>() ...));};
	template <typename A> using result_of_t = typename result_of<A>::type;
	/// @}


	/// Validates a call expression disregarding th returning type
	///
	/// The call expression (in the first argument) will be validated. Whether the call expression is valid will be stored in
	/// \a value. Also depending on the validity of the call expression, the base class will be either \a true_constant or \a false_constant. A
	//// shortcut to obtain the boolean value directly, is to call \a is_callable_v.
	///
	/// @code
	/// int foo (char, short int);
	/// using is_first_callable = is_callable_v<foo(char, short int)>; // should be true
	/// using is_second_callable = is_callable_v<foo(char, float*)>; // should be false 
	/// @endcode
	///
	/// @{
	template <typename , typename = void> struct is_callable : false_constant {};
	template <typename A, typename ... B> struct is_callable<A(B...), void_t<result_of_t<A(B ...)>>> : true_constant {};
	template <typename A> constexpr auto is_callable_v = is_callable<A>::value;
	/// @}

	
	/// Validates a call expression considering the returning type as well
	///
	/// The call expression (in the first argument) will be validated. Whether the call expression is valid and the returning type matches
	/// the second argument, will be stored in \a value. Also depending on the validity of the call expression and the matching of the return
	/// type, the base class will be either \a true_constant or \a false_constant. A shortcut to obtain the boolean value directly is to call
	/// \a is_callale_resulting_v directly.
	///
	/// @code
	/// int foo (char, short int);
	/// using is_correct = is_callable_v<foo(char, short int), int>;
	/// using is_wrong = is_callable_v<foo(char, float*), void*>;
	/// @endcode
	///
	/// @{
	template <typename A, typename B, typename = B> struct is_callable_resulting : false_constant {};
	template <typename A, typename ... B, typename C> struct is_callable_resulting<A(B ...), result_of_t<A(B ...)>> : true_constant {};
	template <typename A, typename B> constexpr auto is_callable_resulting_v = is_callable_resulting<A, B>::value;
	/// @}

}

#endif
