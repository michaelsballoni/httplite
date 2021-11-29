#pragma once

#include "Core.h"

namespace httplite
{
	struct Buffer
	{
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

			std::string utf8
			(
				reinterpret_cast<const char*>(&Bytes.front()), 
				reinterpret_cast<const char*>(&Bytes.back())
			);
			return toWideStr(utf8);
		}

		std::vector<uint8_t> Bytes;
	};
}
