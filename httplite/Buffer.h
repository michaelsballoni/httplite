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

		std::wstring ToString()
		{
			Bytes.push_back(0);
			std::string utf8 = reinterpret_cast<const char*>(Bytes.data());
			Bytes.pop_back();
			return toWideStr(utf8);
		}

		std::vector<uint8_t> Bytes;
	};
}
