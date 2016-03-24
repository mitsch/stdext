/// @file text.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0

#ifndef __STDEXT_TEXT_HPP__
#define __STDEXT_TEXT_HPP__

#include <stdext/allocator.hpp>
#include <stdext/text_view.hpp>
#include <stdext/array.hpp>

namespace stdext
{

	template <Allocator A>
	class text
	{
	
		private:

			array<char, A> bytes;

		public:

			constexpr void append (text_view appendix)
			{
				bytes.append(appendix.raw_view());
			}

			constexpr void append (const text & appendix)
			{
				bytes.append(appendix.raw_view());
			}
	
	};

}

#endif

