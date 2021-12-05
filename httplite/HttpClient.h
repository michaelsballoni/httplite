#pragma once

#include "Core.h"

#include "Request.h"
#include "Response.h"

namespace httplite
{
	/// <summary>
	/// HttpClient implements a basic HTTP/1.0 web client
	/// Simply tell it which server in the constructor
	/// then call ProcessRequest to do HTTP processing
	/// </summary>
	class HttpClient
	{
	public:
		HttpClient(const std::string& server, uint16_t port);
		~HttpClient();

		void EnsureConnected();
		void Disconnect();

		Response ProcessRequest(const Request& request);

	private:
		SOCKET m_socket;

		std::string m_server;
		uint16_t m_port;

		bool m_isConnected;
	};
}
