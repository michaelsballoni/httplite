#pragma once

#include "Core.h"

namespace httplite
{
	/// <summary>
	/// Buffer manages the payloads sent to-and-from
	/// HTTP clients and servers
	/// </summary>
	struct Buffer
	{
		// The bytes of the payload
		std::vector<uint8_t> Bytes;

		Buffer() {}

		/// <summary>
		/// Create this buffer using a wstring
		/// </summary>
		/// <param name="str">wstring to initialize this payload with</param>
		Buffer(const std::wstring& str)
		{
			std::string utf8 = toNarrowStr(str);
			Bytes.resize(utf8.length());
			memcpy(Bytes.data(), utf8.c_str(), utf8.length());
		}

		/// <summary>
		/// Bytes->string conversion
		/// </summary>
		/// <returns>wstring decoded from payload bytes</returns>
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
