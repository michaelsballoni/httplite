#include "pch.h"
#include "CppUnitTest.h"

#include "HttpClient.h"
#include "HttpServer.h"
#pragma comment(lib, "httplite")

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace httplite
{
	static Response HandleRequest(const Request& request)
	{
		Response response;
		if (request.Verb == "GET")
		{
			response.Payload.emplace(L"reversrever");
		}
		else if (request.Verb == "POST")
		{
			if (!request.Payload.has_value())
				throw NetworkError("POST request has no payload to reverse");
			std::wstring str = request.Payload->ToString();
			std::reverse(str.begin(), str.end());
			response.Payload.emplace(str);
		}
		else
			throw NetworkError("Unknown HTTP verb: " + request.Verb);
		return response;
	}

	TEST_CLASS(HttpTests)
	{
	public:
		TEST_METHOD(TestHttpConnectionServing)
		{
			HttpServer server(uint16_t(16384), &HandleRequest);
			server.StartServing();

			HttpClient client("127.0.0.1", uint16_t(16384));

			client.EnsureConnected();
			client.EnsureConnected();
			client.Disconnect();
			client.Disconnect();

			client.EnsureConnected();

			server.StopServing();
		}

		TEST_METHOD(TestHttpGet)
		{
			HttpServer server(uint16_t(16384), &HandleRequest);
			server.StartServing();

			{
				HttpClient client("127.0.0.1", uint16_t(16384));
				Request getRequest;
				Response getResponse = client.ProcessRequest(getRequest);
				Assert::IsTrue(getResponse.Payload.has_value());
				Assert::AreEqual(toWideStr("reversrever"), getResponse.Payload->ToString());
			}
		}

		TEST_METHOD(TestHttpPost)
		{
			HttpServer server(uint16_t(16384), &HandleRequest);
			server.StartServing();

			{
				HttpClient client("127.0.0.1", uint16_t(16384));
				Request postRequest;
				postRequest.Verb = "POST";
				postRequest.Payload.emplace(L"1234");
				Response postResponse = client.ProcessRequest(postRequest);
				Assert::IsTrue(postResponse.Payload.has_value());
				Assert::AreEqual(toWideStr("4321"), postResponse.Payload->ToString());
			}
		}
	};
}
