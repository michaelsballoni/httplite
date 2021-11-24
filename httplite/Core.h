#pragma once

#include "Includes.h"

namespace httplite
{
	typedef uint8_t byte;
	typedef std::vector<byte> buffer;

	class NetworkError : public std::runtime_error
	{
	public:
		NetworkError(const std::string& msg) : std::runtime_error(msg) {}
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
}
