#pragma once

#include "Message.h"

namespace httplite
{
	class Response : public Message
	{
	public:
		Response(const char* module, Pacifier pacifier = {})
			: Message(module, "response", pacifier)
		{}

		std::string Status = "200 OK"; // Code + "decimal" part + Description, "500.100 Internal ASP Error"

		std::uint16_t GetStatusCode() const;
		std::wstring GetStatusDescription() const;

		static Response CreateErrorResponse
		(
			const char* module,
			uint16_t errorCode,
			const std::string& errorMsg, 
			Pacifier pacifier
		);

		virtual std::string GetTotalHeader() const;
		virtual std::string ReadHeader(const char* headerStart);
	};
}
