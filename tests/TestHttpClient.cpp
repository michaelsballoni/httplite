#include "pch.h"
#include "CppUnitTest.h"

#include "HttpClient.h"
#pragma comment(lib, "httplite")

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace httplite
{
	TEST_CLASS(HttpClientTests)
	{
	public:
		TEST_METHOD(TestHttpClient)
		{
			Request request("TestHttpClient", &pacify);
			request.Verb = "GET";
			request.Path = std::vector<std::wstring>{ L"Music", L"index.html" };
			request.Headers["Host"] = "www.michaelballoni.com";

			struct hostent* hostEntry = ::gethostbyname("www.michaelballoni.com");
			if (hostEntry == NULL)
				Assert::Fail();
			
			std::string hostIp = ::inet_ntoa(*((struct in_addr*)hostEntry->h_addr_list[0]));
			HttpClient client(hostIp, 80, &pacify);
			
			Response response = client.ProcessRequest(request);
			Assert::AreEqual(200, int(response.GetStatusCode()));
			Assert::AreEqual(std::string("200 OK"), response.Status);
			Assert::AreEqual(std::wstring(L"OK"), response.GetStatusDescription());
			Assert::AreEqual(std::string("bytes"), response.Headers["Accept-Ranges"]);
			std::wstring content = response.Payload.value().ToString();
			Assert::IsTrue(content.find(L"Computer Music") != std::wstring::npos);
		}
	};
}
