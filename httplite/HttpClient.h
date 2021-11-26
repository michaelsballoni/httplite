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
		void EnsureConnected();
		void Disconnect();

	private:
		SOCKET m_socket;

		std::string m_serverIpAddress;
		uint16_t m_port;

		bool m_isConnected;
	};
}
