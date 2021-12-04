#pragma once

#include "Core.h"

namespace httplite
{
	struct Buffer
	{
		std::vector<uint8_t> Bytes;

		Buffer() {}
		Buffer(const std::wstring& str)
		{
			std::string utf8 = toNarrowStr(str);
			Bytes.resize(utf8.length());
			memcpy(Bytes.data(), utf8.c_str(), utf8.length());
		}

		std::wstring ToString() const
		{
			if (Bytes.empty())
				return std::wstring();

			std::string utf8;
			utf8.resize(Bytes.size());
			memcpy(const_cast<char*>(utf8.c_str()), Bytes.data(), Bytes.size());
			return toWideStr(utf8);
		}
	};
}
