#pragma once

#include "Message.h"

namespace httplite
{
	class Response : public Message
	{
	public:
		std::string Status = "200 OK"; // Code + "decimal" part + Description, "500.100 Internal ASP Error"

		std::uint16_t GetStatusCode() const;
		std::wstring GetStatusDescription() const;

		static Response CreateErrorResponse(uint16_t errorCode, const std::string& errorMsg);

		virtual std::string GetTotalHeader() const;
		virtual std::string ReadHeader(const char* headerStart);
		virtual bool ShouldRecvPayload(size_t remainderSize) const;
	};
}
