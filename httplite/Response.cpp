#include "pch.h"
#include "Response.h"
#include "Core.h"

namespace httplite
{
	void Response::ReadHeader(const char* headerStart)
	{
		const char* headerEnd = strstr(headerStart, "\r\n");
		if (headerEnd == nullptr)
			throw NetworkError("HttpClient: Invalid response header");

		const char* spaceAfterHttp = strstr(headerStart, " ");
		if (spaceAfterHttp == nullptr)
			throw NetworkError("HttpClient: Invalid response line");

		std::string statusLine(spaceAfterHttp + 1, headerEnd);
		size_t statusSpace = statusLine.find(' ');
		if (statusSpace == std::string::npos)
			throw NetworkError("HttpClient: Invalid response status");

		std::string numberPart = statusLine.substr(0, statusSpace);
		std::string descriptionPart = statusLine.substr(statusSpace + 1);

		int code = ::atoi(numberPart.c_str());
		if (code <= 0)
			throw NetworkError("HttpClient: Invalid response code");
		Code = static_cast<uint16_t>(code);

		Status = numberPart;
		Description = descriptionPart;

		headerStart = headerEnd + 2;

		while (true)
		{
			headerEnd = strstr(headerStart, "\r\n");
			if (headerEnd == nullptr)
				break;

			std::string line(headerStart, headerEnd);
			if (line.empty())
				break;

			size_t colonIndex = line.find(':');
			if (colonIndex == std::string::npos)
				throw NetworkError("HttpClient: Invalid response header");

			std::string headerName = line.substr(0, colonIndex);

			std::string headerValue;
			{
				std::string rawHeaderValue = line.substr(colonIndex + 1);
				char* headerValueStart = const_cast<char*>(rawHeaderValue.c_str());
				while (*headerValueStart && ::isspace(*headerValueStart))
					++headerValueStart;
				if (*headerValueStart == '\0')
					throw NetworkError("HttpClient: Invalid response header value");
				headerValue = headerValueStart;
			}

			if (Headers.find(headerName) != Headers.end())
				throw NetworkError("HttpClient: Invalid duplicate response header");

			Headers.insert({ headerName, headerValue });

			headerStart = headerEnd + 2;
		}
	}

	bool Response::IsConnectionClose() const
	{
		for (const auto& header : Headers)
		{
			if 
			(
				toLower(header.first) == "connection" 
				&& 
				toLower(header.second).find("close") != std::string::npos
			)
			{
				return true;
			}
		}
		return false;
	}

	int64_t Response::ContentLength() const
	{
		for (const auto& header : Headers)
		{
			if (toLower(header.first) == "content-length")
			{
				int64_t contentLength = _atoi64(header.second.c_str());
				if (contentLength < 0)
					throw NetworkError("HttpClient: Invalid Content-Length header value");
				return contentLength;
			}
		}
		return -1;
	}
}
