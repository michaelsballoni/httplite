#include "pch.h"
#include "HttpServer.h"
#include "HeaderReader.h"

namespace httplite
{
	HttpServer::HttpServer(uint16_t port, std::function<Response(const Request&)> function)
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
			throw NetworkError("HttpServer: Creating listener failed");

		sockaddr_in serverAddr;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = INADDR_ANY;
		serverAddr.sin_port = htons(m_port);
		if (::bind(m_listenSocket, (const sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
			throw NetworkError("HttpServer: Binding listener to port failed");

		if (::listen(m_listenSocket, 1000) == SOCKET_ERROR)
			throw NetworkError("HttpServer: Starting listening failed");

		m_listenThread = std::make_shared<std::thread>(&HttpServer::AcceptConnections, this);
	}

	void HttpServer::StopServing()
	{
		if (!m_keepRunning)
			return;
		m_keepRunning = false;

		::closesocket(m_listenSocket);
		if (m_listenThread != nullptr && m_listenThread->joinable())
			m_listenThread->join();

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
		while (true)
		{
			Request request;
			std::string messageReadError = request.Recv(clientSocket);
			if (messageReadError == "closed" || messageReadError == "Network Error")
				return;

			Response response;
			if (!messageReadError.empty())
			{
				response = Response::CreateErrorResponse(400, messageReadError);
			}
			else
			{
				std::string functionErrorMessage;
				try
				{
					response = m_function(request);
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
					response = Response::CreateErrorResponse(500, functionErrorMessage);
				}
			}

			std::string sendResponseError = response.Send(clientSocket);
			(void)sendResponseError;
		}
	}
}
