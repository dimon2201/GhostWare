#pragma once

#include "types.hpp"

namespace triton
{
	class cHandle
	{
	public:
		using index = types::u32;

	protected:
		index idx = 0;
		types::usize generation = 0;
	};
}