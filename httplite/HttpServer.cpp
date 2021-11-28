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
		m_keepRunning = true;

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
		m_listenThread->join();

		{
			std::lock_guard<std::mutex> lock(m_clientsMutex);
			for (const auto& clientIt : m_clients)
			{
				::closesocket(clientIt.first);
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
			uint8_t recvBuffer[8 * 1024];
			const size_t maxHeaderSize = 1024 * 1024;
			HeaderReader reader;
			while (true)
			{
				int recvd = ::recv(clientSocket, reinterpret_cast<char*>(recvBuffer), sizeof(recvBuffer), 0);
				if (recvd <= 0)
					return;
				if (reader.OnMoreData(recvBuffer, recvd))
					break;
				if (reader.GetSize() > maxHeaderSize)
					return;
			}

			Request request;
			std::string errMsg = request.ReadHeader(reader.GetHeaders());
			// FORNOW - Return special special Response for error message

			// FORNOW - Process any posted payload

			Response response;
			std::string errorMessage;
			try
			{
				response = m_function(request);
			}
			catch (const std::exception& exp) 
			{
				errorMessage = exp.what();
			}
			catch (...)
			{
				errorMessage = "unknown";
			}
			if (!errorMessage.empty())
			{
				response = Response();
				// FORNOW - Return special Response for this
			}

			// FORNOW - send response
		}
	}
}
