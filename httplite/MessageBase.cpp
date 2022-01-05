#include "pch.h"

#include "MessageBase.h"
#include "HeaderReader.h"

namespace httplite
{
	bool MessageBase::IsConnectionClose() const
	{
		const auto& headerIt = Headers.find("Connection");
		return 
			headerIt == Headers.end() 
			|| 
			toLower(headerIt->second).find("keep-alive") == std::string::npos;
	}

	int MessageBase::GetContentLength() const
	{
		const auto& headerIt = Headers.find("Content-Length");
		if (headerIt == Headers.end())
			return -1;

		int contentLength = atoi(headerIt->second.c_str());
		return contentLength;
	}

	std::string MessageBase::GetCommonHeader() const
	{
		std::string header;

		for (const auto& headerIt : Headers)
			header += headerIt.first + ": " + headerIt.second + "\r\n";

		if (Headers.find("Content-Length") == Headers.end())
			header += "Content-Length: " + num2str(static_cast<double>(Payload.has_value() ? Payload->Bytes.size() : 0)) + "\r\n";

		if (Headers.find("Connection") == Headers.end()) // prefer keep-alive
			header += "Connection: keep-alive\r\n";

		return header;
	}

	std::string MessageBase::Recv(SOCKET theSocket)
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

		const char* totalHeader = reader.GetHeaders();
		std::string headerErrorMessage = ReadHeader(totalHeader);
		if (!headerErrorMessage.empty())
			return headerErrorMessage;

		size_t remainderSize = 0;
		const uint8_t* remainderBytes = reader.GetRemainder(remainderSize);

		bool isConnectionClose = IsConnectionClose();
		int64_t contentLength = GetContentLength();

		bool willReadPayload =
			(
				remainderSize > 0
				||
				contentLength > 0
				||
				(isConnectionClose && CanHavePayload())
			);
		if (!willReadPayload)
			return std::string();

		if (!Payload.has_value())
			Payload.emplace();
		std::vector<uint8_t>& requestPayloadBytes = Payload->Bytes;

		if (contentLength > 0)
			requestPayloadBytes.reserve(size_t(contentLength));

		if (remainderSize > 0)
		{
			requestPayloadBytes.resize(remainderSize);
			memcpy(requestPayloadBytes.data(), remainderBytes, remainderSize);
		}

		size_t recvYet = remainderSize;
		while (true)
		{
			if (contentLength > 0 && recvYet >= static_cast<size_t>(contentLength))
				break;

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

		return std::string();
	}

	std::string MessageBase::Send(SOCKET theSocket) const
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

		if (!Payload.has_value())
			return std::string();

		{
			const std::vector<uint8_t>& bytesToSendVec = Payload->Bytes;
			const char* bytesToSend = reinterpret_cast<const char*>(bytesToSendVec.data());
			if (bytesToSendVec.size() > INT_MAX)
				return "Response Too Large";
			int toSend = static_cast<int>(bytesToSendVec.size());
			int sentYet = 0;
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
