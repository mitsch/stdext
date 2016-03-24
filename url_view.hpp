/// @file url_view.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_URL_VIEW_HPP__
#define __STDEXT_URL_VIEW_HPP__

#include <stdext/array_view.hpp>
#include <stdext/optional.hpp>

namespace stdext
{

	class url_view
	{
	
		private:

			array_view<const char> scheme;
			array_view<
			array_view<const char> path;
			optional<array_view<const char>> query;
			optional<array_view<const char>> fragment;
	};

}

#endif

