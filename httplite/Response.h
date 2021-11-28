#pragma once

#include "Buffer.h"

namespace httplite
{
	struct Response
	{
		std::string Status; // Code + "decimal" part + Description, "500.100 Internal ASP Error"
		std::unordered_map<std::string, std::string> Headers;
		std::optional<Buffer> Payload;

		std::uint16_t GetStatusCode() const;
		std::wstring GetStatusDescription() const;

		bool IsConnectionClose() const;
		int64_t ContentLength() const;

		void ReadHeader(const char* headerStart);
	};
}
