// This program shows how easy it is to use httplite
// to create an HTTP server capable of handling
// basic requests
// The server is a simple HTTP/1.0 server with
// a thread per connection

// This is all that's needed to implement an HTTP server
#include "HttpServer.h"
#pragma comment(lib, "httplite")
using namespace httplite;

#include <iostream> // getline

// This function defines how the web server will response to requests
// This is a basic request handler, handling GET with a simple string
// and POST's by reversing and returning the posted string
// This type of function can take advantage of the simple 
// interface of the Request class to support rich request handling
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
		// Make sure the client POST'd something
		if (!request.Payload.has_value())
			throw NetworkError("POST request has no payload to reverse");

		// Get the POST'd string
		std::wstring str = request.Payload->ToString();
		printf("POST: str to reverse: %S\n", str.c_str());

		// Reverse it
		std::reverse(str.begin(), str.end());
		printf("POST: reversed str: %S\n", str.c_str());

		// Return it
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
		// You only need to provide the port number to serve
		if (argc < 2)
		{
			printf("Usage: httpserver <port>\n");
			return 0;
		}

		// WSAStartup...
		SocketUse socketUsage;

		// Unpack the command-line argument
		uint16_t port = uint16_t(atoi(argv[1]));

		// Start serving on the port, providing
		// the request handling function
		printf("Starting serving on port %d...\n", (int)port);
		HttpServer server(port, &HandleRequest, &pacify);
		server.StartServing();

		// Run until commanded to stop
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
