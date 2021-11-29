#pragma once

#include "Request.h"
#include "Response.h"

namespace httplite
{
	class HttpServer
	{
	public:
		HttpServer(uint16_t port, std::function<Response (const Request&)> function);
		~HttpServer();

		void StartServing();
		void StopServing(); // NOTE: can't restart after stopping

	private:
		void AcceptConnections();

	private:
		uint16_t m_port;
		std::function<Response(const Request&)> m_function;

		bool m_keepRunning;
		
		SOCKET m_listenSocket;
		std::shared_ptr<std::thread> m_listenThread;

		void ServeClient(SOCKET clientSocket);
		std::unordered_map<SOCKET, std::shared_ptr<std::thread>> m_clients;
		std::mutex m_clientsMutex;
	};
}
