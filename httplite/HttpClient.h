#pragma once

#include "Core.h"

#include "Request.h"
#include "Response.h"

namespace httplite
{
	class HttpClient
	{
	public:
		HttpClient(const std::string& server, uint16_t port, Pacifier pacifier);
		~HttpClient();

		void EnsureConnected();
		void Disconnect();

		Response ProcessRequest(const Request& request);

	private:
		SOCKET m_socket;

		std::string m_server;
		uint16_t m_port;

		Pacifier m_pacifier;

		bool m_isConnected;
	};
}
