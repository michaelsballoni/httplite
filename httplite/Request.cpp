#include "pch.h"
#include "Request.h"

namespace httplite
{
    std::string Request::GetTotalHeader() const
    {
        std::string headers;

        headers += Verb;
        headers += ' ';
        if (PathParts.empty())
        {
            headers += '/';
        }
        else
        {
            for (const auto& part : PathParts)
            {
                headers += "/" + UrlEncoded(part);
            }
        }
        if (!QueryParams.empty())
        {
            headers += '?';
            bool anyYet = false;
            for (const auto& queryIt : QueryParams)
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
}
