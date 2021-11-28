#pragma once

#include "Includes.h"

namespace httplite
{
	class NetworkError : public std::runtime_error
	{
	public:
		NetworkError(const std::string& msg) 
			: std::runtime_error(GetErrorMessage(msg)) 
		{}

		static std::string GetErrorMessage(std::string msg)
		{
			int lastSocketError = WSAGetLastError();
			if (lastSocketError != 0)
				msg += " (" + std::to_string(lastSocketError) + ")";
			return msg;
		}
	};

	class SocketUse
	{
	public:
		SocketUse()
		{
			WSADATA wsaData;
			if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
				throw NetworkError("WSAStartup failed");
		}

		~SocketUse()
		{
			WSACleanup();
		}
	};

	inline std::string num2str(double num)
	{
		std::stringstream ss;
		ss << num;
		return ss.str();
	}

	inline std::string toNarrowStr(const std::wstring& str)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
		return converter.to_bytes(str);
	}

	inline std::wstring toWideStr(const std::string& str)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(str);
	}

	inline std::string toLower(const std::string& str)
	{
		std::string retVal;
		for (auto c : str)
			retVal += static_cast<char>(tolower(c));
		return retVal;
	}

	std::vector<std::string> Split(const std::string& str, char separator);
	std::vector<std::wstring> Split(const std::wstring& str, wchar_t separator);

	std::string UrlEncoded(const std::wstring& part);
	std::wstring UrlDecoded(const std::string& part);
}
