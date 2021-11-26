#include "pch.h"
#include "HttpClient.h"

namespace httplite
{
	HttpClient::HttpClient(const std::string& serverIpAddress, uint16_t port)
		: m_socket(INVALID_SOCKET)
	{
		m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_socket == INVALID_SOCKET)
			throw NetworkError("HttpClient: creating client failed");

		sockaddr_in serverAddr;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = inet_addr(serverIpAddress.c_str());
		serverAddr.sin_port = htons(port);
		if (::connect(m_socket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
			throw NetworkError("HttpClient: connecting client failed");
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
		std::string headers;

		headers += request.Verb;
		headers += ' ';
		if (request.PathParts.empty())
		{
			headers += '/';
		}
		else
		{
			for (const auto& part : request.PathParts)
			{
				headers += "/" + UrlEncoded(part);
			}
		}
		if (!request.QueryParams.empty())
		{
			headers += '?';
			bool anyYet = false;
			for (const auto& queryIt : request.QueryParams)
			{
				if (anyYet)
					headers += '&';
				anyYet = true;

				headers += UrlEncoded(queryIt.first) + "=" + UrlEncoded(queryIt.second);
			}
		}
		headers += " HTTP/1.0\r\n";

		for (const auto& headerIt : request.Headers)
		{
			headers += headerIt.first + ": " + headerIt.second + "\r\n";
		}

		if (request.Payload.has_value())
			headers += "Content-Length: " + num2str(request.Payload->Bytes.size()) + "\r\n";

		headers += "Connection: keep-alive\r\n";

		headers += "\r\n";

		{
			const char* headersData = headers.c_str();
			int toSend = static_cast<int>(headers.length());
			int sentYet = 0;
			while (sentYet < toSend)
			{
				int newSent = ::send(m_socket, headersData + sentYet, toSend - sentYet, 0);
				if (newSent == 0)
					throw NetworkError("HttpClient: Server closed connection while sending request header");
				else if (newSent < 0)
					throw NetworkError("HttpClient: Error sending request header");
				else
					sentYet += newSent;
			}
		}
		
		if (request.Payload.has_value())
		{
			const std::vector<uint8_t>& bytesToSendVec = request.Payload->Bytes;
			const char* bytesToSend = reinterpret_cast<const char*>(bytesToSendVec.data());
			size_t toSend = bytesToSendVec.size();
			size_t sentYet = 0;
			while (sentYet < toSend)
			{
				int newSent = ::send(m_socket, bytesToSend + sentYet, toSend - sentYet, 0);
				if (newSent == 0)
					throw NetworkError("HttpClient: Server closed connection while sending request payload");
				else if (newSent < 0)
					throw NetworkError("HttpClient: Error sending request payload");
				else
					sentYet += newSent;
			}
		}

		(void)request; // FORNOW
		Response response;
		return response;
	}
}

