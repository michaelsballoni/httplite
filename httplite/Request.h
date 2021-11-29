#pragma once

#include "Message.h"

namespace httplite
{
	class Request : public Message
	{
	public:
		std::string Verb;
		std::vector<std::wstring> Path;
		std::unordered_map<std::wstring, std::wstring> Query;

		virtual std::string GetTotalHeader() const;
		virtual std::string ReadHeader(const char* headerStart);
	};
}
