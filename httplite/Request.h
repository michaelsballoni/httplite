#pragma once

#include "Buffer.h"

namespace httplite
{
	struct Request
	{
		std::string Verb;
		std::vector<std::wstring> Path;
		std::unordered_map<std::wstring, std::wstring> Query;
		std::unordered_map<std::string, std::string> Headers;
		std::optional<Buffer> Payload;

		std::string GetTotalHeader() const;
		std::string ReadHeader(const char* headerStart);
	};
}
