#include "pch.h"
#include "Message.h"
#include "HeaderReader.h"
#include "Request.h"

namespace httplite
{
	Message::Message(const char* module, const char* type, Pacifier pacifier)
		: m_module(module)
		, m_type(type)
		, m_pacifier(pacifier)
	{
	}

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

		if (Payload.has_value() && Headers.find("Content-Length") == Headers.end())
			header += "Content-Length: " + num2str(Payload->Bytes.size()) + "\r\n";

		if (Headers.find("Connection") == Headers.end())
			header += "Connection: keep-alive\r\n";

		return header;
	}

	const char* Message::ReturnErrorMsg(const char* area, const char* msg) const
	{
		m_pacifier(m_module, m_type, (std::string(area) + " errors: " + std::string(msg)).c_str());
		return msg;
	}

	std::string Message::Recv(SOCKET theSocket)
	{
		m_pacifier(m_module, m_type, "Recv");

		m_pacifier(m_module, m_type, "Recv: recv header");
		uint8_t recvBuffer[8 * 1024];
		const size_t maxHeaderSize = 1024 * 1024;
		HeaderReader reader;
		while (true)
		{
			int recvd = ::recv(theSocket, reinterpret_cast<char*>(recvBuffer), sizeof(recvBuffer), 0);
			if (recvd == 0)
				return ReturnErrorMsg("Recv: recv header", "closed");
			if (recvd < 0)
				return ReturnErrorMsg("Recv: recv header", "Network Error");
			if (reader.OnMoreData(recvBuffer, recvd))
				break;
			if (reader.GetSize() > maxHeaderSize)
				return ReturnErrorMsg("Recv: recv header", "Header Overflow");
		}

		m_pacifier(m_module, m_type, "Recv: ReadHeader");
		const char* totalHeader = reader.GetHeaders();
		std::string headerErrorMessage = ReadHeader(totalHeader);
		if (!headerErrorMessage.empty())
			return headerErrorMessage;

		m_pacifier(m_module, m_type, "Recv: GetRemainder");
		size_t remainderSize = 0;
		const uint8_t* remainderBytes = reader.GetRemainder(remainderSize);

		bool isConnectionClose = IsConnectionClose();
		int64_t contentLength = GetContentLength();

		bool isGetRequest =
			typeid(*this) == typeid(Request)
			&&
			static_cast<Request*>(this)->Verb == "GET";

		bool willReadPayload =
			!isGetRequest
			&&
			(
				remainderSize > 0
				||
				contentLength > 0
				||
				isConnectionClose
			);
		if (!willReadPayload)
		{
			m_pacifier(m_module, m_type, "Recv: will NOT ReadPayload, All done.");
			return std::string();
		}
		else
			m_pacifier(m_module, m_type, "Recv: will ReadPayload");

		Payload.emplace();
		std::vector<uint8_t>& requestPayloadBytes = Payload->Bytes;

		if (contentLength > 0)
			requestPayloadBytes.reserve(size_t(contentLength));

		if (remainderSize > 0)
		{
			Payload->Bytes.resize(remainderSize);
			memcpy(Payload->Bytes.data(), remainderBytes, remainderSize);
		}

		m_pacifier(m_module, m_type, "Recv: recv payload");
		size_t recvYet = remainderSize;
		while (true)
		{
			if (contentLength > 0 && recvYet >= contentLength)
				break;

			int recvd = ::recv(theSocket, reinterpret_cast<char*>(recvBuffer), sizeof(recvBuffer), 0);
			if (recvd <= 0)
			{
				if (isConnectionClose)
					return ReturnErrorMsg("Recv: recv payload", "closed");
				else
					return ReturnErrorMsg("Recv: recv payload", "Network Error");
			}

			requestPayloadBytes.resize(requestPayloadBytes.size() + recvd);
			memcpy(requestPayloadBytes.data() + requestPayloadBytes.size() - recvd, recvBuffer, recvd);

			recvYet += recvd;
		}

		m_pacifier(m_module, m_type, "Recv: All done.");
		return std::string();
	}

	std::string Message::Send(SOCKET theSocket) const
	{
		m_pacifier(m_module, m_type, "Send");
		{
			m_pacifier(m_module, m_type, "Send: send header");
			std::string headers = GetTotalHeader();
			const char* headersData = headers.c_str();
			int toSend = static_cast<int>(headers.length());
			int sentYet = 0;
			while (sentYet < toSend)
			{
				int newSent = ::send(theSocket, headersData + sentYet, toSend - sentYet, 0);
				if (newSent == 0)
					return ReturnErrorMsg("Send: send header", "Connection Closed");
				if (newSent < 0)
					return ReturnErrorMsg("Send: send header", "Network Error");
				sentYet += newSent;
			}
		}

		if (!Payload.has_value())
		{
			m_pacifier(m_module, m_type, "Send: no payload to send, All done.");
			return std::string();
		}
		else
			m_pacifier(m_module, m_type, "Send: send payload");

		{
			const std::vector<uint8_t>& bytesToSendVec = Payload->Bytes;
			const char* bytesToSend = reinterpret_cast<const char*>(bytesToSendVec.data());
			size_t toSend = bytesToSendVec.size();
			size_t sentYet = 0;
			while (sentYet < toSend)
			{
				int newSent = ::send(theSocket, bytesToSend + sentYet, toSend - sentYet, 0);
				if (newSent == 0)
					return ReturnErrorMsg("Send: send payload", "Connection Closed");
				if (newSent < 0)
					return ReturnErrorMsg("Send: send payload", "Network Error");
				sentYet += newSent;
			}
		}

		m_pacifier(m_module, m_type, "Send: All done.");
		return std::string();
	}
}
