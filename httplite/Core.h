#pragma once

#include "Includes.h"

namespace httplite
{
	typedef uint8_t byte;
	typedef std::vector<byte> buffer;

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

	struct Request
	{
		std::string Verb;
		std::vector<std::wstring> PathParts;
		std::unordered_map<std::wstring, std::wstring> QueryParams;
		std::unordered_map<std::wstring, std::wstring> Headers;
		std::optional<buffer> Payload;
	};

	struct Response
	{
		std::uint16_t Code; // 200, 500
		std::string Status; // Code + decimal part, 500.12
		std::wstring Description; // 200 OK
		std::unordered_map<std::wstring, std::wstring> Headers;
		std::optional<buffer> Payload;
	};

	std::string UrlEncoded(const std::wstring& part);
	std::wstring UrlDecoded(std::string part);
}
