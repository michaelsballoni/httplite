#include "pch.h"
#include "HttpClient.h"
#include "HeaderReader.h"

namespace httplite
{
	HttpClient::HttpClient(const std::string& server, uint16_t port, Pacifier pacifier)
		: m_socket(INVALID_SOCKET)
		, m_server(server)
		, m_port(port)
		, m_pacifier(pacifier)
		, m_isConnected(false)
	{
	}

	HttpClient::~HttpClient()
	{
		Disconnect();
	}

	void HttpClient::EnsureConnected()
	{
		if (m_isConnected)
			return;

		m_pacifier("HttpClient", "EnsureConnected", "socket");
		m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_socket == INVALID_SOCKET)
			throw NetworkError("HttpClient: Creating client failed");

		m_pacifier("HttpClient", "EnsureConnected", "gethostbyname");
		struct hostent* host = gethostbyname(m_server.c_str());
		if (host == nullptr)
			throw NetworkError("HttpClient: Resolving server address failed");

		m_pacifier("HttpClient", "EnsureConnected", "connect");
		sockaddr_in serverAddr;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr = *(struct in_addr*)(host->h_addr);
		serverAddr.sin_port = htons(m_port);
		if (::connect(m_socket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
			throw NetworkError("HttpClient: Connecting client failed");

		m_isConnected = true;
		m_pacifier("HttpClient", "EnsureConnected", "All done.");
	}

	void HttpClient::Disconnect()
	{
		if (!m_isConnected)
			return;

		m_pacifier("HttpClient", "Disconnect", "closesocket");
		if (m_socket != INVALID_SOCKET)
		{
			::closesocket(m_socket);
			m_socket = INVALID_SOCKET;
		}

		m_isConnected = false;
		m_pacifier("HttpClient", "Disconnect", "All done.");
	}

	Response HttpClient::ProcessRequest(const Request& request)
	{
		EnsureConnected();
#ifndef _DEBUG
		try
#endif
		{
			std::string sendMessageError = request.Send(m_socket);
			if (!sendMessageError.empty())
				throw NetworkError(sendMessageError);

			Response response("HttpClient", m_pacifier);
			std::string recvMessageError = response.Recv(m_socket);
			if (!recvMessageError.empty())
				throw NetworkError(recvMessageError);

			return response;
		}
#ifndef _DEBUG
		catch (...)
		{
			Disconnect();
			throw;
		}
#endif
	}
}
