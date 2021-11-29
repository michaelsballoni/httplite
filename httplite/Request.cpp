#include "pch.h"
#include "Request.h"

namespace httplite
{
	std::string Request::GetTotalHeader() const
    {
        std::string header;

        header += Verb;
        header += ' ';

        if (!Path.empty())
        {
			for (const auto& part : Path)
				header += "/" + UrlEncode(part);
        }
        else
			header += '/';
		
        bool anyQueryYet = false;
        for (const auto& queryIt : Query)
        {
			if (anyQueryYet)
				header += '&';
			else
				header += '?';
			anyQueryYet = true;

            header += UrlEncode(queryIt.first) + "=" + UrlEncode(queryIt.second);
        }

        header += " HTTP/1.0\r\n";

		header += GetCommonHeader();
        header += "\r\n";
        
		return header;
    }

	std::string Request::ReadHeader(const char* headerStart)
	{
		const char* headerEnd = strstr(headerStart, "\r\n");
		if (headerEnd == nullptr)
			return "Invalid Request Header";

		std::vector<std::string> responseLine = Split(std::string(headerStart, headerEnd), ' ');
		if (responseLine.size() < 3)
			return "Invalid Request Line";

		Verb = responseLine[0];

		std::string path, query;
		{
			std::string pathAndQuery = responseLine[1];
			size_t queryStart = pathAndQuery.find('?');
			if (queryStart != std::string::npos)
			{
				path = pathAndQuery.substr(0, queryStart);
				query = pathAndQuery.substr(queryStart + 1);
			}
			else
				path = pathAndQuery;
		}

		for (const std::string& part : Split(path, '/'))
			Path.push_back(UrlDecode(part));

		for (const std::string& part : Split(query, '&'))
		{
			size_t equals = part.find('=');
			if (equals == std::string::npos)
				return "Invalid Query String";

			std::wstring name = UrlDecode(part.substr(0, equals));
			std::wstring value = UrlDecode(part.substr(equals + 1));
				
			if (Query.find(name) == Query.end())
				Query.insert({ name, value });
			else
				Query[name] += L"," + value;
		}

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
				return "Invalid Request Header";

			std::string headerName = line.substr(0, colonIndex);

			std::string headerValue;
			{
				std::string rawHeaderValue = line.substr(colonIndex + 1);
				char* headerValueStart = const_cast<char*>(rawHeaderValue.c_str());
				while (*headerValueStart && ::isspace(*headerValueStart))
					++headerValueStart;
				if (*headerValueStart == '\0')
					return "Invalid Request Header Value";
				headerValue = headerValueStart;
			}

			if (Headers.find(headerName) != Headers.end())
				return "Invalid Duplicate Header";

			Headers.insert({ headerName, headerValue });

			headerStart = headerEnd + 2;
		}

		return std::string();
	}

	bool Request::ShouldRecvPayload(size_t remainderSize) const
	{
		if (Verb == "GET")
			return false;

		int64_t contentLength = GetContentLength();
		return contentLength > 0 || remainderSize > 0;
	}
}
