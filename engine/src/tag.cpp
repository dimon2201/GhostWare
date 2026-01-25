#include "tag.hpp"

using namespace types;

namespace triton
{
	cTag::cTag(cContext* context) : iObject(context)
	{
		FillZeros();
	}

	cTag::cTag(cContext* context, const std::string& text) : cTag(context)
	{
		CopyChars((const u8*)text.c_str(), text.size());
	}

	cTag::cTag(cContext* context, const u8* chars, usize charsByteSize) : cTag(context)
	{
		CopyChars(chars, charsByteSize);
	}

	boolean cTag::Compare(const std::string& text)
	{
		if (text.size() != _byteSize)
			return K_FALSE;

		for (usize i = 0; i < _byteSize; i++)
		{
			if (text[i] != _data[i])
				return K_FALSE;
		}

		return K_TRUE;
	}

	void cTag::FillZeros()
	{
		memset(&_data[0], 0, kMaxTagByteSize);
	}

	void cTag::CopyChars(const u8* chars, usize charsByteSize)
	{
		if (chars == nullptr || charsByteSize == 0 || charsByteSize >= kMaxTagByteSize)
			return;

		_byteSize = charsByteSize;
		memcpy(&_data[0], &chars[0], _byteSize);
	}
}