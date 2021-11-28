#include "pch.h"
#include "Request.h"

namespace httplite
{
    std::string Request::GetTotalHeader() const
    {
        std::string headers;

        headers += Verb;
        headers += ' ';

        if (Path.empty())
        {
            headers += '/';
        }
        else
        {
            for (const auto& part : Path)
            {
                headers += "/" + UrlEncoded(part);
            }
        }

        if (!Query.empty())
        {
            headers += '?';
            bool anyYet = false;
            for (const auto& queryIt : Query)
            {
                if (anyYet)
                    headers += '&';
                anyYet = true;

                headers += UrlEncoded(queryIt.first) + "=" + UrlEncoded(queryIt.second);
            }
        }

        headers += " HTTP/1.0\r\n";

        for (const auto& headerIt : Headers)
            headers += headerIt.first + ": " + headerIt.second + "\r\n";

        if (Payload.has_value())
            headers += "Content-Length: " + num2str(Payload->Bytes.size()) + "\r\n";

        headers += "Connection: keep-alive\r\n";

        headers += "\r\n";

        return headers;
    }

	std::string Request::ReadHeader(const char* headerStart)
	{
		const char* headerEnd = strstr(headerStart, "\r\n");
		if (headerEnd == nullptr)
			return "Invalid request header";

		std::vector<std::string> responseLine = Split(std::string(headerStart, headerEnd), ' ');
		if (responseLine.size() < 3)
			return ("Invalid request line");

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

		Path = Split(UrlDecoded(path), '/');
		if (Path.empty())
			return ("Invalid request line");

		if (!query.empty())
		{
			for (const std::string& part : Split(query, '&'))
			{
				size_t equals = part.find('=');
				if (equals == std::string::npos)
					return ("Invalid request line");

				std::wstring name = UrlDecoded(part.substr(0, equals));
				std::wstring value = UrlDecoded(part.substr(equals + 1));
				
				if (Query.find(name) == Query.end())
					Query.insert({ name, value });
				else
					Query[name] += L"," + value;
			}
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
				return "Invalid request header";

			std::string headerName = line.substr(0, colonIndex);

			std::string headerValue;
			{
				std::string rawHeaderValue = line.substr(colonIndex + 1);
				char* headerValueStart = const_cast<char*>(rawHeaderValue.c_str());
				while (*headerValueStart && ::isspace(*headerValueStart))
					++headerValueStart;
				if (*headerValueStart == '\0')
					return "Invalid request header value";
				headerValue = headerValueStart;
			}

			if (Headers.find(headerName) != Headers.end())
				return "Invalid duplicate header";

			Headers.insert({ headerName, headerValue });

			headerStart = headerEnd + 2;
		}

		return std::string();
	}
}
