#include "pch.h"
#include "Message.h"
#include "HeaderReader.h"

namespace httplite
{
	bool Message::IsConnectionClose() const
	{
		for (const auto& header : Headers)
		{
			if
				(
					toLower(header.first) == "connection"
					&&
					toLower(header.second).find("keep-alive") != std::string::npos
					)
			{
				return false;
			}
		}
		return true;
	}

	int64_t Message::GetContentLength() const
	{
		for (const auto& header : Headers)
		{
			if (toLower(header.first) == "content-length")
			{
				int64_t contentLength = _atoi64(header.second.c_str());
				return contentLength;
			}
		}
		return -1;
	}

	std::string Message::GetCommonHeader() const
	{
		std::string header;

		for (const auto& headerIt : Headers)
			header += headerIt.first + ": " + headerIt.second + "\r\n";

		if (Payload.has_value())
			header += "Content-Length: " + num2str(Payload->Bytes.size()) + "\r\n";

		if (Headers.find("Connection") == Headers.end())
			header += "Connection: keep-alive\r\n";

		return header;
	}

	std::string Message::Recv(SOCKET theSocket)
	{
		uint8_t recvBuffer[8 * 1024];
		const size_t maxHeaderSize = 1024 * 1024;
		HeaderReader reader;
		while (true)
		{
			int recvd = ::recv(theSocket, reinterpret_cast<char*>(recvBuffer), sizeof(recvBuffer), 0);
			if (recvd == 0)
				return "closed";
			if (recvd < 0)
				return "Network Error";
			if (reader.OnMoreData(recvBuffer, recvd))
				break;
			if (reader.GetSize() > maxHeaderSize)
				return "Header Overflow";
		}

		std::string headerErrorMessage = ReadHeader(reader.GetHeaders());
		if (!headerErrorMessage.empty())
			return headerErrorMessage;

		size_t remainderSize = 0;
		const uint8_t* remainderBytes = reader.GetRemainder(remainderSize);

		bool isConnectionClose = IsConnectionClose();
		int64_t contentLength = isConnectionClose ? -1 : GetContentLength();

		if (contentLength > 0 || isConnectionClose || remainderSize > 0)
		{
			Payload.emplace();
			std::vector<uint8_t>& requestPayloadBytes = Payload->Bytes;

			if (contentLength > 0)
				requestPayloadBytes.reserve(size_t(contentLength));

			if (remainderSize > 0)
			{
				Payload->Bytes.resize(remainderSize);
				memcpy(Payload->Bytes.data(), remainderBytes, remainderSize);
			}

			size_t recvYet = remainderSize;
			while (isConnectionClose || recvYet < contentLength)
			{
				int recvd = ::recv(theSocket, reinterpret_cast<char*>(recvBuffer), sizeof(recvBuffer), 0);
				if (recvd <= 0)
				{
					if (isConnectionClose)
						return "closed";
					else
						return "Network Error";
				}

				requestPayloadBytes.resize(requestPayloadBytes.size() + recvd);
				memcpy(requestPayloadBytes.data() + requestPayloadBytes.size() - recvd, recvBuffer, recvd);

				recvYet += recvd;
			}
		}

		return std::string();
	}

	std::string Message::Send(SOCKET theSocket) const
	{
		{
			std::string headers = GetTotalHeader();
			const char* headersData = headers.c_str();
			int toSend = static_cast<int>(headers.length());
			int sentYet = 0;
			while (sentYet < toSend)
			{
				int newSent = ::send(theSocket, headersData + sentYet, toSend - sentYet, 0);
				if (newSent == 0)
					return "Connection Closed";
				if (newSent < 0)
					return "Network Error";
				sentYet += newSent;
			}
		}

		if (Payload.has_value())
		{
			const std::vector<uint8_t>& bytesToSendVec = Payload->Bytes;
			const char* bytesToSend = reinterpret_cast<const char*>(bytesToSendVec.data());
			size_t toSend = bytesToSendVec.size();
			size_t sentYet = 0;
			while (sentYet < toSend)
			{
				int newSent = ::send(theSocket, bytesToSend + sentYet, toSend - sentYet, 0);
				if (newSent == 0)
					return "Connection Closed";
				if (newSent < 0)
					return "Network Error";
				sentYet += newSent;
			}
		}

		return std::string();
	}
}
