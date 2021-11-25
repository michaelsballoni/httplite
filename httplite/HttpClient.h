#pragma once

#include "Core.h"

namespace httplite
{
	class HttpClient
	{
	public:
		HttpClient(const std::string& serverIpAddress, uint16_t port);
		~HttpClient();

		Response ProcessRequest(const Request& request);

	private:
		SocketUse m_socketUse;
		SOCKET m_socket = INVALID_SOCKET;
	};
}