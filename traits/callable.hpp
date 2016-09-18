/// @file callable.hpp
/// @module traits
/// @librar stdext
/// @author Michael Koch
/// @copyright CC BY 3.0

#include <stdext/integral/constant.hpp>
#include <stdext/sfinae.hpp>
#include <stdext/forward.hpp>
#include <stdext/types/declval.hpp>

#ifndef STDEXT__TRAITS__CALLABLE
#define STDEXT__TRAITS__CALLABLE

namespace stdext
{
namespace traits
{

	// TODO constant returns, that is single values!

	template <typename A, typename ... B> inline auto call (A&& callee, B&& ... arguments)
		noexcept(noexcept(forward<A>(callee)(forward<B>(arguments) ...)))
	{
		return forward<A>(callee)(forward<B>(arguments) ...);
	}
	template <typename A, typename B, typename C, typename ... D> inline auto call (A B::* pointerToCallee, C&& instance, D&& ... arguments)
		noexcept(noexcept((forward<C>(instance).*pointerToCallee)(forward<D>(arguments) ...)));	
	{
		return (forward<C>(instance).*pointerToCallee)(forward<D>(arguments) ...);
	}
	template <typename A, typename B, typename ... C> inline auto call (A&& pointerToCallee, B&& pointedInstance, C&& ... arguments)
		noexcept(noexcept(((*forward<B>(pointerToInstance)).*forward<A>(pointerToCallee))(forward<C>(arguments) ...)))	
	{
		return ((*forward<B>(pointerToInstance)).*forward<A>(pointerToCallee))(forward<C>(arguments) ...);
	}

	


	template <bool, typename, typename = void> struct __is_callable : integral::false_constant {};
	template <bool A, typename B, typename ... C> struct __is_callable<A, B(C ...), void_t<decltype(call(types::declval<B>(), types::declval<C>() ...)))>> : integral::boolean_constant<A or noexcept(call(types::declval<B>(), types::declval<C>() ...))> {};

	template <bool A, typename B, typename C, typename = C> struct __is_callable_resulting : integral::false_constant {};
	template <bool A, typename B, typename C, typename ... D> struct __is_callable_resulting<A, B(D ...), C, decltype(call(types::declval<B>(), types::declval<D>() ...))> : boolean_constant<A or noexcept(call(types::declval<B>(), types::declval<D>() ...))> {};

	template <typename, typename ...> struct is_callable;
	template <typename A> struct is_callable<A> : __is_callable<true, A> {};
	template <typename A, typename B> struct is_callable<A, B> : __is_callable_resulting<true, A, B> {};
	template <typename A, typename ... B> constexpr auto is_callable_v = is_callable<A, B ...>::value;

	template <typename, typename ...> struct is_nothrow_callable;
	template <typename A> struct is_nothrow_callable<A> : __is_callable<false, A> {};
	template <typename A, typename B> struct is_nothrow_callable<A, B> : __is_callable<false, A, B> {};
	template <typename A, typename ... B> constexpr auto is_nothrow_callable_v = is_nothrow_callable<A, B ...>::value;
	

	template <typename, typename = void> struct __result_of {};
	template <typename A, typename ... B> struct __result_of<A(B ...), void_t<decltype(call(types::declval<A>()(types::declval<B>() ...)))>>
	{
		using type = decltype(call(types::declval<A>(), types::declval<B>() ...));
	};
	template <typename A> struct result_of : __result_of<A> {};
	template <typename A> using result_of_t = typename result_of<A>::type;

}
}

#endif

