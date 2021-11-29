#pragma once

#include "Core.h"

#include "Request.h"
#include "Response.h"

namespace httplite
{
	class HttpClient
	{
	public:
		HttpClient(const std::string& serverIpAddress, uint16_t port);
		~HttpClient();

		void EnsureConnected();
		void Disconnect();

		Response ProcessRequest(const Request& request);

	private:
		SOCKET m_socket;

		std::string m_serverIpAddress;
		uint16_t m_port;

		bool m_isConnected;
	};
}
