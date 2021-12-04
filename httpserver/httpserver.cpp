#include "HttpServer.h"
#pragma comment(lib, "httplite")
using namespace httplite;

#include <iostream> // getline

static Response HandleRequest(const Request& request, Pacifier pacifier)
{
	Response response("HandleRequest", pacifier);
	if (request.Verb == "GET")
	{
		printf("GET: reversrever\n");
		response.Payload.emplace(L"reversrever");
	}
	else if (request.Verb == "POST")
	{
		if (!request.Payload.has_value())
			throw NetworkError("POST request has no payload to reverse");
		std::wstring str = request.Payload->ToString();
		printf("POST: str to reverse: %S\n", str.c_str());
		std::reverse(str.begin(), str.end());
		printf("POST: reversed str: %S\n", str.c_str());
		response.Payload.emplace(str);
	}
	else
		throw httplite::NetworkError("Unknown HTTP verb: " + request.Verb);
	return response;
}

int main(int argc, char* argv[])
{
#ifndef _DEBUG
	try
#endif
	{
		if (argc < 2)
		{
			printf("Usage: httpserver <port>\n");
			return 0;
		}

		SocketUse socketUsage;

		uint16_t port = uint16_t(atoi(argv[1]));

		printf("Starting serving on port %d...\n", (int)port);
		HttpServer server(port, &HandleRequest, &pacify);
		server.StartServing();

		printf("Hit [Enter] to stop serving and close the program:\n");
		std::string line;
		std::getline(std::cin, line); // program spends its life here
		server.StopServing();
	}
#ifndef _DEBUG
	catch (const std::exception& exp)
	{
		printf("ERROR: %s\n", exp.what());
		return 1;
	}
#endif
	return 0;
}
