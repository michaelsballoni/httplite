#include "HttpClient.h"
#pragma comment(lib, "httplite")
using namespace httplite;

int wmain(int argc, wchar_t* argv[])
{
#ifndef _DEBUG
	try
#endif
	{
		if (argc < 5)
		{
			printf("Usage: httpclient <server> <port> <verb> <resource> [payload]\n");
			return 0;
		}

		std::string server = toNarrowStr(argv[1]);
		uint16_t port = uint16_t(_wtoi(argv[2]));
		std::string verb = toNarrowStr(argv[3]);
		std::wstring resource = argv[4];
		std::wstring body = argc >= 6 ? argv[5] : L"";

		httplite::SocketUse socketUsage; // WSAStart...

		HttpClient client(server, port, &pacify);

		Request request("HttpClient", &pacify);
		request.Verb = verb;
		if (request.Verb != "GET" && request.Verb != "POST")
		{
			printf("Verb must be GET or POST\n");
			return 1;
		}

		request.Path = Split(resource, '/');

		if (!body.empty())
		{
			request.Payload.emplace(body);
		}

		printf("Processing request:\n%s", request.GetTotalHeader().c_str());
		Response response = client.ProcessRequest(request);

		printf("Received response:\n%sPayload:\n%S\n", 
				response.GetTotalHeader().c_str(),
				response.Payload.has_value() ? response.Payload->ToString().c_str() : L"");
		return 0;
	}
#ifndef _DEBUG
	catch (const std::exception& exp)
	{
		printf("ERROR: %s\n", exp.what());
		return 1;
	}
#endif
}
