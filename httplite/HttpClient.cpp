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
			{
				std::string headers = request.GetTotalHeader();
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
				response.ReadHeader(headerStart);

				size_t remainderSize = 0;
				const uint8_t* remainderBytes = reader.GetRemainder(remainderSize);
				if (remainderSize > 0)
				{
					response.Payload.emplace(Buffer());
					response.Payload->Bytes.resize(remainderSize);
					memcpy(response.Payload->Bytes.data(), remainderBytes, remainderSize);
				}

				bool isConnectionClose = response.IsConnectionClose();
				int64_t contentLength = isConnectionClose ? -1 : response.ContentLength();

				if ((contentLength > 0 || isConnectionClose) && !response.Payload.has_value())
					response.Payload.emplace(Buffer());
				auto& payloadBytes = response.Payload->Bytes;
				if (contentLength > 0)
					payloadBytes.reserve(size_t(contentLength));

				size_t recvYet = remainderSize;
				int64_t totalToRecv = contentLength;
				while (isConnectionClose || recvYet < totalToRecv)
				{
					int recvd = ::recv(m_socket, reinterpret_cast<char*>(recvBuffer), sizeof(recvBuffer), 0);
					if (recvd == 0)
					{
						if (isConnectionClose)
							break;
						else
							throw NetworkError("HttpClient: Server closed connection while receiving response payload");
					}
					if (recvd < 0)
						throw NetworkError("HttpClient: Error receiving response payload");

					payloadBytes.resize(payloadBytes.size() + recvd);
					memcpy(payloadBytes.data() + payloadBytes.size() - recvd, recvBuffer, recvd);

					recvYet += recvd;
				}

				if (isConnectionClose)
					Disconnect();
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
