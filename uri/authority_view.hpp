/// @file authority_view.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_URI_AUTHORITY_VIEW_HPP__
#define __STDEXT_URI_AUTHORITY_VIEW_HPP__

#include <stdext/array_view.hpp>
#include <stdext/optional.hpp>

namespace stdext
{

	template <typename C>
	class basic_uri_authority_view
	{
	
	private:

		optional<array_view<C>> userinfo;
		array_view<C> host;
		optional<array_view<C>> port;

		static constexpr bool is_digit (const C& character)
		{
			return character == C(0x30) or
			       character == C(0x31) or
			       character == C(0x32) or
			       character == C(0x33) or
			       character == C(0x34) or
			       character == C(0x35) or
			       character == C(0x36) or
			       character == C(0x37) or
			       character == C(0x38) or
			       character == C(0x39);
		}

	public:

		constexpr basic_uri_authority_view (optional<array_view<C>> userinfo, array_view<C> host, optional<array_view<C>> port)
			noexcept
			: userinfo(userinfo), host(host), port(port)
		{}

		constexpr optional<array_view<C>> get_userinfo () const
		{
			return userinfo;
		}

		constexpr array_view<C> get_host () const
		{
			return host;
		}

		constexpr optional<array_view<C>> get_port () const
		{
			return port;
		}

		static constexpr basic_uri_authority_view parse (array_view<C> data)
		{
			auto userinfoSplit = data.split_prefix([](const auto & element){return element != '@'});
			const auto userinfo = make_optional_if(not std::get<1>(userinfoSplit).empty(), std::get<0>(userinfoSplit));
			auto rest = std::get<1>(userinfoSplit).empty() ? std::get<0>(userinfoSplit) : std::get<1>(std::get<1>(userinfoSplit).split_prefix(1));
			const auto portNumberSplit = rest.split_suffix(is_digit);
			const auto hasTrailingColon = std::get<0>(portNumberSplit).has_suffix({C(0x3a)});
			const auto port = make_optional_if(hasTrailingColon, std::get<1>(portNumberSplit));
			const auto host = hasTrailingColon ? std::get<0>(std::get<0>(portNumberSplit).split_suffix(1)) : rest;
			return basic_uri_authority_view(userinfo, host, port);
		}

	};

}

#endif

