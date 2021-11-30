#include "pch.h"
#include "HttpServer.h"
#include "HeaderReader.h"

namespace httplite
{
	HttpServer::HttpServer(uint16_t port, RequestHandler function, Pacifier pacifier)
		: m_port(port)
		, m_function(function)
		, m_pacifier(pacifier)
		, m_keepRunning(true)
		, m_listenSocket(INVALID_SOCKET)
	{
	}

	HttpServer::~HttpServer()
	{
		StopServing();
	}

	void HttpServer::StartServing()
	{
		m_pacifier("HttpServer", "StartServing", "socket");
		m_listenSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_listenSocket == INVALID_SOCKET)
			throw NetworkError("HttpServer: Creating listener failed");

		m_pacifier("HttpServer", "StartServing", "bind");
		sockaddr_in serverAddr;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = INADDR_ANY;
		serverAddr.sin_port = htons(m_port);
		if (::bind(m_listenSocket, (const sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
			throw NetworkError("HttpServer: Binding listener to port failed");

		m_pacifier("HttpServer", "StartServing", "listen");
		if (::listen(m_listenSocket, 1000) == SOCKET_ERROR)
			throw NetworkError("HttpServer: Starting listening failed");

		m_listenThread = std::make_shared<std::thread>(&HttpServer::AcceptConnections, this);
		m_threadJoinThread = std::make_shared<std::thread>(&HttpServer::JoinThreads, this);
		m_pacifier("HttpServer", "StartServing", "All done.");
	}

	void HttpServer::StopServing()
	{
		if (!m_keepRunning)
			return;
		m_keepRunning = false;

		m_pacifier("HttpServer", "StopServing", "close(listener)");
		::closesocket(m_listenSocket);

		m_pacifier("HttpServer", "StopServing", "join(listener)");
		if (m_listenThread != nullptr && m_listenThread->joinable())
			m_listenThread->join();
		m_listenThread = nullptr;

		m_pacifier("HttpServer", "StopServing", "join(joiner)");
		if (m_threadJoinThread != nullptr && m_threadJoinThread->joinable())
			m_threadJoinThread->join();
		m_threadJoinThread = nullptr;

		m_pacifier("HttpServer", "StopServing", "close(clients)");
		{
			std::lock_guard<std::mutex> lock(m_clientsMutex);
			for (const auto& clientIt : m_clients)
			{
				::closesocket(clientIt.first);
				if (clientIt.second->joinable())
					clientIt.second->join();
			}
		}

		m_pacifier("HttpServer", "StopServing", "All done.");
	}

	void HttpServer::AcceptConnections()
	{
		while (true)
		{
			m_pacifier("HttpServer", "AcceptConnections", "accept");
			SOCKET clientSocket = ::accept(m_listenSocket, nullptr, nullptr);
			if (clientSocket == INVALID_SOCKET)
			{
				m_pacifier("HttpServer", "AcceptConnections", "accept...returning");
				return;
			}

			m_pacifier("HttpServer", "AcceptConnections", "accept...new connection");
			auto newThread =
				std::make_shared<std::thread>
				(
					&HttpServer::ServeClient,
					this,
					clientSocket
				);

			std::lock_guard<std::mutex> lock(m_clientsMutex);
			m_clients.insert({ clientSocket, newThread });
		}
	}

	void HttpServer::ServeClient(SOCKET clientSocket)
	{
		while (true)
		{
			Request request("HttpServer", m_pacifier);
			std::string messageReadError = request.Recv(clientSocket);
			if (messageReadError == "closed" || messageReadError == "Network Error")
				break;

			Response response("HttpServer", m_pacifier);
			if (!messageReadError.empty())
			{
				response = Response::CreateErrorResponse("HttpServer", 400, messageReadError, m_pacifier);
			}
			else
			{
				std::string functionErrorMessage;
				try
				{
					response = m_function(request, m_pacifier);
				}
				catch (const std::exception& exp)
				{
					functionErrorMessage = exp.what();
				}
				catch (...)
				{
					functionErrorMessage = "unknown";
				}
				if (!functionErrorMessage.empty())
				{
					response = Response::CreateErrorResponse("HttpServer", 500, functionErrorMessage, m_pacifier);
				}
			}

			std::string sendResponseError = response.Send(clientSocket);
			if (!sendResponseError.empty())
			{
				m_pacifier("HttpServer", "ServerClient", ("Send error: " + sendResponseError).c_str());
				break;
			}
		}

		if (m_threadJoinThread != nullptr)
		{
			std::lock_guard<std::mutex> lock(m_clientsMutex);
			m_threadsToJoin.push_back(m_clients[clientSocket]);
			m_clients.erase(clientSocket);
		}
	}

	void HttpServer::JoinThreads()
	{
		while (m_keepRunning)
		{
			{
				std::lock_guard<std::mutex> lock(m_clientsMutex);
				for (auto& threadToJoin : m_threadsToJoin)
					threadToJoin->join();
				m_threadsToJoin.clear();
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}
