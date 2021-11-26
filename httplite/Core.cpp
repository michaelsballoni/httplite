#include "pch.h"
#include "Core.h"

std::string httplite::UrlEncoded(const std::wstring& part) 
{
    std::string retVal;
    char hexBuffer[10];
    for (size_t i = 0; i < part.length(); ++i) 
    {
        wchar_t c = part[i];
        if (c < 128 && (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~'))
        {
            retVal += char(c);
        }
        else 
        {
            sprintf_s<sizeof(hexBuffer)>(hexBuffer, "%X", c);
            if (c < 16)
                retVal += "%0";
            else
                retVal += "%";
            retVal += hexBuffer;
        }
    }
    return retVal;
}

std::wstring httplite::UrlDecoded(const std::string& part)
{
    std::wstring retVal;
    for (size_t i = 0; i < part.length(); ++i) 
    {
        if (part[i] != '%') 
        {
            retVal += part[i];
        }
        else 
        {
            int scanned = 0;
            if (sscanf_s(part.substr(i + 1, 2).c_str(), "%x", &scanned) == 1)
            {
                retVal += static_cast<char>(scanned);
                i += 2;
            }
        }
    }
    return retVal;
}
