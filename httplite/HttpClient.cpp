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

	Response HttpClient::ProcessRequest(const Request& request)
	{
		EnsureConnected();

#ifndef _DEBUG
		try
#endif
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
					if (newSent < 0)
						throw NetworkError("HttpClient: Error sending request header");
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
					if (newSent < 0)
						throw NetworkError("HttpClient: Error sending request payload");
					sentYet += newSent;
				}
			}

			uint8_t recvBuffer[8 * 1024];
			const size_t maxHeaderSize = 1024 * 1024;
			HeaderReader reader;
			while (true)
			{
				int recvd = ::recv(m_socket, reinterpret_cast<char*>(recvBuffer), sizeof(recvBuffer), 0);
				if (recvd == 0)
					throw NetworkError("HttpClient: Server closed connection while receiving response header");
				if (recvd < 0)
					throw NetworkError("HttpClient: Error receiving response header");
				if (reader.OnMoreData(recvBuffer, recvd))
					break;
				if (reader.GetSize() > maxHeaderSize)
					throw NetworkError("HttpClient: Max response header size exceeded");
			}

			Response response;
			{
				const char* headerStart = reader.GetHeaders();
				const char* headerEnd = strstr(headerStart, "\r\n");
				if (headerEnd == nullptr)
					throw NetworkError("HttpClient: Invalid response header");

				const char* spaceAfterHttp = strstr(headerStart, " ");
				if (spaceAfterHttp == nullptr)
					throw NetworkError("HttpClient: Invalid response line");

				std::string statusLine(spaceAfterHttp, headerEnd);
				size_t statusSpace = statusLine.find(' ');
				if (statusSpace == std::string::npos)
					throw NetworkError("HttpClient: Invalid response status");

				std::string numberPart = statusLine.substr(0, statusSpace);
				std::string descriptionPart = statusLine.substr(statusSpace + 1);

				int code = ::atoi(numberPart.c_str());
				if (code <= 0)
					throw NetworkError("HttpClient: Invalid response code");
				response.Code = static_cast<uint16_t>(code);

				response.Status = numberPart;
				response.Description = descriptionPart;

				headerStart = headerEnd + 2;

				while (true)
				{
					headerEnd = strstr(headerStart, "\r\n");
					if (headerEnd == nullptr)
						break;

					std::string line(headerStart, headerEnd);
					if (line.empty())
						break;

					// FORNOW - Continue here

					headerStart = headerEnd + 2;
				}
			}
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
}

