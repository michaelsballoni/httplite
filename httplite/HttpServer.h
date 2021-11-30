#pragma once

#include "Request.h"
#include "Response.h"

namespace httplite
{
	typedef std::function<Response(const Request& request, Pacifier pacifier)> RequestHandler;

	class HttpServer
	{
	public:
		HttpServer(uint16_t port, RequestHandler function, Pacifier pacifier);
		~HttpServer();

		void StartServing();
		void StopServing(); // NOTE: can't restart after stopping

	private:
		void AcceptConnections();
		void JoinThreads();

	private:
		uint16_t m_port;
		RequestHandler m_function;
		Pacifier m_pacifier;

		bool m_keepRunning;
		
		SOCKET m_listenSocket;
		std::shared_ptr<std::thread> m_listenThread;

		std::shared_ptr<std::thread> m_threadJoinThread;

		void ServeClient(SOCKET clientSocket);
		std::unordered_map<SOCKET, std::shared_ptr<std::thread>> m_clients;
		std::vector< std::shared_ptr<std::thread>> m_threadsToJoin;
		std::mutex m_clientsMutex;
	};
}
