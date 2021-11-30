#pragma once

#include "Message.h"

namespace httplite
{
	class Request : public Message
	{
	public:
		Request(const char* module, Pacifier pacifier = {})
			: Message(module, "request", pacifier)
		{}

		std::string Verb = "GET";
		std::vector<std::wstring> Path;
		std::unordered_map<std::wstring, std::wstring> Query;

		virtual std::string GetTotalHeader() const;
		virtual std::string ReadHeader(const char* headerStart);
	};
}
