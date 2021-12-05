#pragma once

#include "Request.h"
#include "Response.h"

namespace httplite
{
	/// <summary>
	/// Just define how to handle Requests by returning Responses
	/// and you're all set
	/// </summary>
	typedef std::function<Response(const Request& request)> RequestHandler;

	/// <summary>
	/// HttpServer implements a basic HTTP/1.0 web server
	/// It uses a thread-per-client, so it's not meant 
	/// for a heavily trafficked internet application
	/// </summary>
	class HttpServer
	{
	public:
		HttpServer(uint16_t port, RequestHandler function);
		~HttpServer();

		void StartServing();
		void StopServing(); // NOTE: can't restart after stopping

	private:
		void AcceptConnections();
		
		void ServeClient(SOCKET clientSocket);

		void JoinThreads();
		void DoJoinThreads();

	private:
		uint16_t m_port;
		RequestHandler m_function;

		bool m_keepRunning;
		
		SOCKET m_listenSocket;
		std::shared_ptr<std::thread> m_listenThread;

		std::unordered_map<SOCKET, std::shared_ptr<std::thread>> m_clients;
		std::vector<std::shared_ptr<std::thread>> m_threadsToJoin;
		std::shared_ptr<std::thread> m_threadJoinThread;
		std::mutex m_clientsMutex;
	};
}
