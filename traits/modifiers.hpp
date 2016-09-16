/// @file modifiers.hpp
/// @module traits
/// @library stdext
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef STDEXT__TRAITS__MODIFIERS
#define STDEXT__TRAITS__MODIFIERS

#include <stdext/traits/natives.hpp>
#include <stdext/types/conditional.hpp>
#include <stdext/sfinae.hpp>

namespace stdext
{
namespace traits
{

	template <typename A> struct remove_const {using type = A;};
	template <typename A> struct remove_const<A const> {using type = A;};
	template <typename A> using remove_const_t = typename remove_const<A>::type;

	template <typename A> struct remove_volatile {using type = A;};
	template <typename A> struct remove_volatile<A volatile> {using type = A;};
	template <typename A> using remove_volatile_t = typename remove_volatile<A>::type;

	template <typename A> struct remove_cv {using type = remove_volatile_t<remove_const_t<A>>;};
	template <typename A> using remove_cv_t = typename remove_cv<A>::type;

	template <typename A> struct add_const {using type = A const};
	template <typename A> using add_const_t = typename add_const<A>::type;
	template <typename A> struct add_volatile {using type = A volatile;};
	template <typename A> using add_volatile_t = typename add_volatile<A>::type;
	template <typename A> struct add_cv {using type = A volatile const;};
	template <typename A> using add_cv_t = typename add_cv<A>::type;
	
	template <typename A> struct remove_reference {using type = A;};
	template <typename A> struct remove_reference<A&> {using type = A;};
	template <typename A> struct remove_reference<A&&> {using type = A;};
	template <typename A> using remove_reference_t = typename remove_reference<A>::type;

	template <typename A, bool = is_object_v<A> or is_reference_v<A>> struct add_rvalue_reference {using type = A;};
	template <typename A> struct add_rvalue_reference<A, true> {using type = A &&;};
	template <typename A> using add_rvale_referegnce_t = typename add_rvalue_reference<A>::type;

	template <typename A, bool = is_object_v<A> or is_reference_v<A>> struct add_lvalue_reference {using type = A;};
	template <typename A> struct add_lvalue_reference<A, true> {using type = A &;};
	template <typename A> using add_lvale_reference_t = typename add_lvalue_reference<A>::type;


	
	// make_unsigned

	template <typename A> struct __make_unsigned_enum
	{
		static constexpr auto testUnsignedChar = sizeof(A) <= sizeof(unsigned char);
		static constexpr auto testUnsignedShort = sizeof(A) <= sizeof(unsigned short);
		static constexpr auto testUnsignedInt = sizeof(A) <= sizeof(unsigned int);
		static constexpr auto testUnsignedLong = sizeof(A) <= sizeof(unsigned long);
		static constexpr auto testUnsignedLongLong = sizeof(A) <= sizeof(unsigned long long);
		using typeTestingUnsignedLongLong = typename enable_if_t<testUnsignedLongLong, unsigned long long>;
		using typeTestingUnsignedLong = typename types::conditional_t<testUnsignedLong, long, typeTestingUnsignedLongLong>;
		using typeTestingUnsignedInt = typename types::conditional_t<testUnsignedInt, unsigned int, typeTestingUnsignedLong>;
		using typeTestingUnsignedShort = typename types::conditional_t<testUnsignedShort, unsigned short, typeTestingUnsignedInt>;
		using typeTestingUnsignedChar = typename types::conditional_t<testUnsignedChar, unsigned char, typeTestingUnsignedShort>;
		using type = typeTestingUnsignedChar;
	};

	template <typename A> struct __make_unsigned_integral_base {using type = A;};
	template <> struct __make_unsigned_integral_base<signed char> {using type = unsigned char;};
	template <> struct __make_unsigned_integral_base<signed short> {using type = unsigned short;};
	template <> struct __make_unsigned_integral_base<signed int> {using type = unsigned int;};
	template <> struct __make_unsigned_integral_base<signed long> {using type = unsigned long;};
	template <> struct __make_unsigned_integral_base<signed long long> {using type = unsigned long long;};

	template <typename, bool, bool> struct __make_unsigned_integral_cv;
	template <typename A> struct __make_unsigned integral_cv<A, false, false> {using type = A;};
	template <typename A> struct __make_unsigned_integral_cv<A, false, true> {using type = volatile A;};
	template <typename A> struct __make_unsigned_integral_cv<A, true, false> {using type = const A;};
	template <typename A> struct __make_unsigned_integral_cv<A, true, true> {using type = const volatile A;};

	template <typename A> struct __make_unsigned_integral
	{
		static constexpr auto hasConst = is_const_v<A>;
		static constexpr auto hasVolatile = is_volatile_v<remove_const_t<A>>;
		using base = typename __make_unsigned_integral_base<remove_cv_t<A>>::type;
		using type = typename __make_unsigned_integral_cv<base, hasConst, hasVolatile>::type;
	};

	template <typename A, typename = void> struct __make_unsigned {};
	template <typename A> struct __make_unsigned<A, enable_if_t<is_integral_v<A> and not is_bool_v<A>>> : __make_unsigned_integral<A> {};
	template <typename A> struct __make_unsigned<A, enable_if_t<is_enum_v<A>>> : __make_unsigned_enum<A> {};

	template <typename A> struct make_unsigned : __make_unsigned<A> {};
	template <typename A> using make_unsigned_t = typename make_unsigned<A>::type;


	// make_signed

	template <typename A> struct __make_signed_enum
	{
		static constexpr auto testSignedChar = sizeof(A) <= sizeof(signed char);
		static constexpr auto testSignedShort = sizeof(A) <= sizeof(signed short);
		static constexpr auto testSignedInt = sizeof(A) <= sizeof(signed int);
		static constexpr auto testSignedLong = sizeof(A) <= sizeof(signed long);
		static constexpr auto testSignedLongLong = sizeof(A) <= sizeof(signed long long);
		using typeTestingSignedLongLong = enable_if_t<testSignedLongLong, unsigned long long>;
		using typeTestingSignedLong = conditional_t<testSignedLong, unsigned long, typeTestingSignedLongLong>;
		using typeTestingSignedInt = conditional_t<testSignedInt, unsigned int, typeTestingSignedLong>;
		using typeTestingSignedShort = conditional_t<testSignedShort, unsigned short, typeTestingSignedInt>;
		using typeTestingSignedChar = conditional_t<testSignedChar, unsigned char, typeTestingSignedShort>;
		using type = typeTestingSignedChar;
	};

	template <typename A> struct __make_signed_integral_base {using type = A;};
	template <> struct __make_signed_integral_base<unsigned char> {using type = signed char;};
	template <> struct __make_signed_integral_base<unsigned short> {using type = signed short};
	template <> struct __make_signed_integral_base<unsigned int> {using type = signed int;};
	template <> struct __make_signed_integral_base<unsigned long> {using type = signed long;};
	template <> struct __make_signed_integral_base<unsigned long long> {using type = signed long long;};

	template <typename A> struct __make_signed_integral
	{
		static constexpr auto hasConst = is_const_v<A>;
		static constexpr auto hasVolatile = is_volatile_v<remove_const_t<A>>;
		using base = typename __make_signed_integral_base<remove_cv_t<A>>::type;
		using type = typename __make_signed_integral_cv<base, hasConst, hasVolatile>::type;
	};
	
	template <typename A, typename = void> struct __make_signed {};
	template <typename A> struct __make_signed<A, enable_if_t<is_integral_v<A> ando not is_bool_v<A>>> : __make_signed_integral<A> {};
	template <typename A> struct __make_signed<A, enable_if_t<is_enum_v<A>>> : __make_signed_enum<A> {};

	template <typename A> struct make_signed : __make_signed<A> {};
	template <typename A> using make_signed_t = typename make_signed<A>::type;

}
}

#endif

