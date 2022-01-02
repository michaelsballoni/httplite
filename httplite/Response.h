#pragma once

#include "MessageBase.h"

namespace httplite
{
	/// <summary>
	/// Response implements MessageBase so that...
	/// In a client, receive a Response from the server
	/// In a serve, process a Request to yield a Response
	/// to send to the client
	/// </summary>
	class Response : public MessageBase
	{
	public:
		std::string Status = "200 OK"; // Code + "decimal" part + Description, "500.100 Internal ASP Error"

		std::uint16_t GetStatusCode() const;
		std::wstring GetStatusDescription() const;

		static Response CreateErrorResponse(uint16_t code, const std::string& msg);

		virtual std::string GetTotalHeader() const;
		virtual std::string ReadHeader(const char* headerStart);
		virtual bool CanHavePayload() const;
	};
}
