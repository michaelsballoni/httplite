// This program shows how easy it is to use httplite
// to connect to a web server and perform an HTTP
// operation

// This is all that's needed to implement an HTTP client
#include "HttpClient.h"
#pragma comment(lib, "httplite")
using namespace httplite;

int wmain(int argc, wchar_t* argv[])
{
#ifndef _DEBUG
	try
#endif
	{
		// httplite splits up the URL you want to use
		// into its separate parts for easy assembly
		if (argc < 5)
		{
			printf("Usage: httpclient <server> <port> <verb> <resource> [payload]\n");
			return 0;
		}

		// Unpack the command line arguments
		std::string server = toNarrowStr(argv[1]);
		uint16_t port = uint16_t(_wtoi(argv[2]));
		std::string verb = toNarrowStr(argv[3]);
		std::wstring resource = argv[4];			   // can be
		std::wstring body = argc >= 6 ? argv[5] : L""; // Unicode, hence wmain

		// Handle WSAStartup...
		SocketUse socketUsage; 

		// Create our HTTP request, akin to .NET's HttpWebRequest,
		// with the verb...
		Request request;
		request.Verb = verb;
		if (request.Verb != "GET" && request.Verb != "POST")
		{
			printf("Verb must be GET or POST\n");
			return 1;
		}

		// The components of the path
		request.Path = Split(resource, '/');

		// The request payload for POST or PUT
		// In this case we pass in the string off 
		// the wmain command line, allowing for
		// Unicode characters
		// request.Payload automatically UTF-8
		// encodes the request body
		// and decodes it on the server (same code)
		if (!body.empty())
			request.Payload.emplace(body);

		// Create the HTTP client that will process
		// our request
		HttpClient client(server, port);

		// Do the deed, getting a Response object back
		printf("Processing request:\n%s", request.GetTotalHeader().c_str());
		Response response = client.ProcessRequest(request);

		// Report on the results of the HTTP operation
		// response.Payload->ToString() automatically
		// converts the response body into UTF-8
		printf("Received response:\n%sPayload:\n%S\n", 
				response.GetTotalHeader().c_str(),
				response.Payload.has_value() 
				? response.Payload->ToString().c_str() 
				: L"");
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
