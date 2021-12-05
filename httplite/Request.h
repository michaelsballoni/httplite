#pragma once

#include "MessageBase.h"

namespace httplite
{
	/// <summary>
	/// Request implements MessageBase so that...
	/// In a client, populate a Request and get it processed
	/// to yield a Response for you to consume
	/// In a server, receive a Request and process it
	/// to yield a Response to return to the client
	/// </summary>
	class Request : public MessageBase
	{
	public:
		std::string Verb = "GET";
		std::vector<std::wstring> Path;
		std::unordered_map<std::wstring, std::wstring> Query;

		virtual std::string GetTotalHeader() const;
		virtual std::string ReadHeader(const char* headerStart);
	};
}
