#pragma once

#include "Buffer.h"

namespace httplite
{
	struct Response
	{
		std::uint16_t Code = 0; // 500
		std::string Status; // Code + "decimal" part, "500.100"
		std::string Description; // "Internal ASP Error"
		std::unordered_map<std::string, std::string> Headers;
		std::optional<Buffer> Payload;

		bool IsConnectionClose() const;
		int64_t ContentLength() const;

		void ReadHeader(const char* headerStart);
	};
}
