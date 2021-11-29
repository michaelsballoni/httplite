#include "pch.h"
#include "HttpClient.h"
#include "HeaderReader.h"

namespace httplite
{
	HttpClient::HttpClient(const std::string& serverIpAddress, uint16_t port)
		: m_socket(INVALID_SOCKET)
		, m_serverIpAddress(serverIpAddress)
		, m_port(port)
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

		m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_socket == INVALID_SOCKET)
			throw NetworkError("HttpClient: Creating client failed");

		sockaddr_in serverAddr;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = inet_addr(m_serverIpAddress.c_str());
		serverAddr.sin_port = htons(m_port);
		if (::connect(m_socket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
			throw NetworkError("HttpClient: Connecting client failed");

		m_isConnected = true;
	}

	void HttpClient::Disconnect()
	{
		if (!m_isConnected)
			return;

		if (m_socket != INVALID_SOCKET)
		{
			::closesocket(m_socket);
			m_socket = INVALID_SOCKET;
		}

		m_isConnected = false;
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

			Response response;
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
