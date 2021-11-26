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

	struct Buffer
	{
		Buffer() {}
		Buffer(const std::wstring& str)
		{
			std::string utf8 = toNarrowStr(str);
			Bytes.resize(utf8.length());
			memcpy(Bytes.data(), utf8.c_str(), utf8.length());
		}

		std::wstring ToString()
		{
			Bytes.push_back(0);
			std::string utf8 = reinterpret_cast<const char*>(Bytes.data());
			Bytes.pop_back();
			return toWideStr(utf8);
		}

		std::vector<uint8_t> Bytes;
	};

	struct Request
	{
		std::string Verb;
		std::vector<std::wstring> PathParts;
		std::unordered_map<std::wstring, std::wstring> QueryParams;
		std::unordered_map<std::string, std::string> Headers;
		std::optional<Buffer> Payload;
	};

	struct Response
	{
		std::uint16_t Code; // 200, 500
		std::string Status; // Code + decimal part, 500.12
		std::wstring Description; // 200 OK
		std::unordered_map<std::string, std::wstring> Headers;
		std::optional<Buffer> Payload;
	};

	std::string UrlEncoded(const std::wstring& part);
	std::wstring UrlDecoded(const std::string& part);
}
