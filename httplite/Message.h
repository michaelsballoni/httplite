#pragma once

#include "Buffer.h"

namespace httplite
{
	class Message
	{
	public:
		virtual ~Message() {}

		std::unordered_map<std::string, std::string> Headers;
		std::optional<Buffer> Payload;

		bool IsConnectionClose() const;
		int64_t GetContentLength() const;

		std::string Recv(SOCKET theSocket);
		std::string Send(SOCKET theSocket) const;

		virtual std::string GetTotalHeader() const = 0;
		virtual std::string ReadHeader(const char* headerStart) = 0;

	protected:
		std::string GetCommonHeader() const;
	};
}
