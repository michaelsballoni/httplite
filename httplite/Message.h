#pragma once

#include "Buffer.h"

namespace httplite
{
	typedef std::function<void(const char* module, const char* type, const char* area)> Pacifier;
	inline void pacify(const char* module, const char* type, const char* msg)
	{
		printf("%s: %s: %s\n", module, type, msg);
	}

	class Message
	{
	protected:
		Message(const char* module, const char* type, Pacifier pacifier);

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

	private:
		const char* ReturnErrorMsg(const char* area, const char* msg) const;

	private:
		const char* m_module;
		const char* m_type;
		Pacifier m_pacifier;
	};
}
