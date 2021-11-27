#pragma once

#include "Buffer.h"

namespace httplite
{
	struct Request
	{
		std::string Verb;
		std::vector<std::wstring> PathParts;
		std::unordered_map<std::wstring, std::wstring> QueryParams;
		std::unordered_map<std::string, std::string> Headers;
		std::optional<Buffer> Payload;

		std::string GetTotalHeader() const;
	};
}