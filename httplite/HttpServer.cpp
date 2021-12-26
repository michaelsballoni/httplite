#include "pch.h"
#include "HttpServer.h"
#include "HeaderReader.h"

namespace httplite
{
	HttpServer::HttpServer(uint16_t port, RequestHandler function)
		: m_port(port)
		, m_function(function)
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
		m_listenSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_listenSocket == INVALID_SOCKET)
			throw NetworkError("Creating listener failed");

		sockaddr_in serverAddr;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = INADDR_ANY;
		serverAddr.sin_port = htons(m_port);
		if (::bind(m_listenSocket, (const sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
			throw NetworkError("Binding listener to port failed");

		if (::listen(m_listenSocket, 1000) == SOCKET_ERROR)
			throw NetworkError("Starting listener failed");

		m_listenThread = std::make_shared<std::thread>(&HttpServer::AcceptConnections, this);
		m_threadJoinThread = std::make_shared<std::thread>(&HttpServer::JoinThreads, this);
	}

	void HttpServer::StopServing()
	{
		if (!m_keepRunning)
			return;
		m_keepRunning = false;

		::closesocket(m_listenSocket);
		m_listenSocket = INVALID_SOCKET;

		if (m_listenThread != nullptr && m_listenThread->joinable())
			m_listenThread->join();
		m_listenThread = nullptr;

		if (m_threadJoinThread != nullptr && m_threadJoinThread->joinable())
			m_threadJoinThread->join();
		m_threadJoinThread = nullptr;

		{
			std::lock_guard<std::mutex> lock(m_clientsMutex);
			for (const auto& clientIt : m_clients)
			{
				::closesocket(clientIt.first);
				if (clientIt.second->joinable())
					clientIt.second->join();
			}
			m_clients.clear();
		}

		DoJoinThreads();
	}

	void HttpServer::AcceptConnections()
	{
		while (true)
		{
			SOCKET clientSocket = ::accept(m_listenSocket, nullptr, nullptr);
			if (clientSocket == INVALID_SOCKET)
				return;

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
		// FORNOW - Use make_shared to create this straight off
		std::shared_ptr<std::unordered_map<std::wstring, std::wstring>> connectionVariables;
		while (m_keepRunning)
		{
			Request request;
			std::string messageReadError = request.Recv(clientSocket);
			if (messageReadError == "closed" || messageReadError == "Network Error")
				break;

			if (!messageReadError.empty())
			{
				Response errorResponse = Response::CreateErrorResponse(400, messageReadError);
				errorResponse.Send(clientSocket);
				break;
			}

			std::string functionErrorMessage;
			Response functionResponse;
			try
			{
				request.ConnectionVariables = connectionVariables;
				functionResponse = m_function(request);
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
				Response errorResponse = Response::CreateErrorResponse(500, functionErrorMessage);
				errorResponse.Send(clientSocket);
				break;
			}
		
			if (!m_keepRunning)
				break;

			std::string sendResponseError = functionResponse.Send(clientSocket);
			if (!sendResponseError.empty())
				break;
		}

		::closesocket(clientSocket);

		if (m_keepRunning) // otherwise StopServing deals with this
		{
			std::lock_guard<std::mutex> lock(m_clientsMutex);
			m_threadsToJoin.push_back(m_clients[clientSocket]); // order
			m_clients.erase(clientSocket);                      // matters
		}
	}

	void HttpServer::JoinThreads()
	{
		while (m_keepRunning)
		{
			DoJoinThreads();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	void HttpServer::DoJoinThreads()
	{
		std::lock_guard<std::mutex> lock(m_clientsMutex);
		for (auto& threadToJoin : m_threadsToJoin)
			threadToJoin->join();
		m_threadsToJoin.clear();
	}
}
