#include "HttpClient.h"
#include "HttpServer.h"
#pragma comment(lib, "httplite")

#include <iostream>

static httplite::Response HandleRequest(const httplite::Request& request, httplite::Pacifier pacifier)
{
	httplite::Response response("HandleRequest", pacifier);
	if (request.Verb == "GET")
	{
		printf("GET: reversrever\n");
		response.Payload.emplace(L"reversrever");
	}
	else if (request.Verb == "POST")
	{
		if (!request.Payload.has_value())
			throw httplite::NetworkError("POST request has no payload to reverse");
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
		httplite::SocketUse socketUsage;

		if (argc <= 1)
		{
			printf("Client Usage: httplite client <server IP> <server port>\n");
			printf("Server Usage: httplite server <port>\n");
			return 0;
		}

		if (std::string(argv[1]) == "client")
		{
			if (argc < 4)
			{
				printf("Client Usage: httplite client <server IP> <server port>\n");
				return 0;
			}

			std::string serverIp = argv[2];
			uint16_t serverPort = uint16_t(atoi(argv[3]));

			httplite::HttpClient client(serverIp, serverPort, &httplite::pacify);
			while (true)
			{
				printf("\n> ");
				std::string line;
				std::getline(std::cin, line);
				if (line.empty())
					continue;
				if (line == "quit" || line == "exit")
					break;
				if (line == "help")
				{
					printf("Enter a request like \"POST / something\" to post \"something\" to the server\n");
					continue;
				}

				std::vector<std::string> strs = httplite::Split(line, ' ');
				if (strs.size() != 3)
				{
					printf("Requests must be \"verb path string-to-post\"\n");
					continue;
				}

				httplite::Request request("HttpClient", &httplite::pacify);
				request.Verb = strs[0];
				if (request.Verb != "GET" && request.Verb != "POST")
				{
					printf("Verb must be GET or POST\n");
					continue;
				}
				request.Path = httplite::Split(httplite::toWideStr(strs[1]), '/');
				if (request.Verb == "POST")
				{
					printf("String to reverse: %s\n", strs[2].c_str());
					request.Payload.emplace(httplite::toWideStr(strs[2]));
				}

				printf("Issuing query:\n%s", request.GetTotalHeader().c_str());
				httplite::Response response = client.ProcessRequest(request);

				printf("Received response:\n%sPayload:\n%S\n", 
					   response.GetTotalHeader().c_str(),
					   response.Payload.has_value() ? response.Payload->ToString().c_str() : L"");
			}
		}
		else if (std::string(argv[1]) == "server")
		{
			if (argc < 3)
			{
				printf("Server Usage: httplite server <port>\n");
				return 0;
			}

			uint16_t port = uint16_t(atoi(argv[2]));
			
			printf("Starting serving on port %d...\n", (int)port);
			httplite::HttpServer server(port, &HandleRequest, &httplite::pacify);
			server.StartServing();

			printf("Hit [Enter] to stop serving and close the program: ");
			std::string line;
			std::getline(std::cin, line);
			server.StopServing();
		}
		else
		{
			printf("Unrecognized command mode, must be client or server\n");
			return 1;
		}
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
