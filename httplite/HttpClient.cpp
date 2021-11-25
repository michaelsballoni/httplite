#include "pch.h"
#include "HttpClient.h"

namespace httplite
{
	HttpClient::HttpClient(const std::string& serverIpAddress, uint16_t port)
	{
		m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_socket == INVALID_SOCKET)
			throw NetworkError("HttpClient: socket call failed");

		sockaddr_in serverAddr;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = inet_addr(serverIpAddress.c_str());
		serverAddr.sin_port = htons(port);
		if (::connect(m_socket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
			throw NetworkError("HttpClient: connect failed");
	}

	HttpClient::~HttpClient()
	{
		if (m_socket != INVALID_SOCKET)
		{
			::closesocket(m_socket);
			m_socket = INVALID_SOCKET;
		}
	}

	Response HttpClient::ProcessRequest(const Request& request)
	{
		Response response;
		(void)request; // FORNOW
		return response;
	}
}

