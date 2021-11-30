#include "pch.h"
#include "Response.h"
#include "Core.h"

namespace httplite
{
	std::uint16_t Response::GetStatusCode() const
	{
		size_t statusSpace = Status.find(' ');
		if (statusSpace == std::string::npos)
			throw NetworkError("Invalid response status");

		int code = ::atoi(Status.substr(0, statusSpace).c_str());
		if (code <= 0 || code > USHRT_MAX)
			throw NetworkError("Invalid status code");

		return static_cast<uint16_t>(code);
	}

	std::wstring Response::GetStatusDescription() const
	{
		size_t statusSpace = Status.find(' ');
		if (statusSpace == std::string::npos)
			throw NetworkError("Invalid response status");
		return UrlDecode(Status.substr(statusSpace + 1));
	}

	std::string Response::GetTotalHeader() const
	{
		return
			"HTTP/1.0 " + Status + "\r\n" +
			GetCommonHeader() +
			"\r\n";
	}

	std::string Response::ReadHeader(const char* headerStart)
	{
		const char* headerEnd = strstr(headerStart, "\r\n");
		if (headerEnd == nullptr)
			return "Invalid response header";

		const char* spaceAfterHttp = strstr(headerStart, " ");
		if (spaceAfterHttp == nullptr)
			return "HttpClient: Invalid response line";

		Status = std::string(spaceAfterHttp + 1, headerEnd);

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
				return "Invalid response header";

			std::string headerName = line.substr(0, colonIndex);

			std::string headerValue;
			{
				std::string rawHeaderValue = line.substr(colonIndex + 1);
				char* headerValueStart = const_cast<char*>(rawHeaderValue.c_str());
				while (*headerValueStart && ::isspace(*headerValueStart))
					++headerValueStart;
				if (*headerValueStart == '\0')
					return "Invalid response header value";
				headerValue = headerValueStart;
			}

			if (Headers.find(headerName) != Headers.end())
				return "Invalid duplicate response header";

			Headers.insert({ headerName, headerValue });

			headerStart = headerEnd + 2;
		}

		return std::string();
	}

	Response Response::CreateErrorResponse
	(
		const char* module, 
		uint16_t errorCode, 
		const std::string& errorMsg, 
		Pacifier pacifier
	)
	{
		pacifier
		(
			module, 
			"CreateErrorResponse", 
			("Error: " + std::to_string(errorCode) + " " + errorMsg).c_str()
		);
		Response response(module, pacifier);
		response.Status = std::to_string(errorCode) + " " + errorMsg;
		response.Headers.insert({ "Connection", "close" });
		return response;
	}
}
