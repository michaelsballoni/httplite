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
			Request request;
			request.Host = "www.michaelballoni.com";
			request.Verb = "GET";
			request.PathParts = std::vector<std::wstring>{ L"Music", L"index.html" };

			struct hostent* hostEntry = ::gethostbyname("www.michaelballoni.com");
			if (hostEntry == NULL)
				Assert::Fail();
			
			std::string hostIp = ::inet_ntoa(*((struct in_addr*)hostEntry->h_addr_list[0]));
			HttpClient client(hostIp, 80);
			
			Response response = client.ProcessRequest(request);
			Assert::AreEqual(200, int(response.Code));
			Assert::AreEqual(std::string("200"), response.Status);
			Assert::AreEqual(std::string("OK"), response.Description);
			Assert::AreEqual(std::string("bytes"), response.Headers["Accept-Ranges"]);
			std::wstring content = response.Payload.value().ToString();
			Assert::IsTrue(content.find(L"Computer Music") != std::wstring::npos);
		}
	};
}
