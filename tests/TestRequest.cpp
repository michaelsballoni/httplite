#include "pch.h"
#include "CppUnitTest.h"

#include "Request.h"
#pragma comment(lib, "httplite")

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace httplite
{
	TEST_CLASS(RequestTests)
	{
	public:
		TEST_METHOD(TestRequest)
		{
			Request request;
			request.Host = "michaelballoni.com";
			request.Verb = "GET";
			request.PathParts = std::vector<std::wstring>{ L"Music", L"index.html" };
			request.QueryParams =
				std::unordered_map<std::wstring, std::wstring>
				{ 
					{ L"foo", L"bar" }, 
					{ L"blet", L"mon key" }
				};
			request.Headers =
				std::unordered_map<std::string, std::string>
				{
					{ "some", "thing" },
					{ "totally", "different" }
				};
			request.Payload.emplace(L"this is a GET, payload only for length");

			std::string computedTotalHeader = request.GetTotalHeader();
			std::string expectedTotalHeader =
				"GET /Music/index.html?foo=bar&blet=mon%20key HTTP/1.0\r\n"
				"Host: michaelballoni.com\r\n"
				"some: thing\r\n"
				"totally: different\r\n"
				"Content-Length: 38\r\n"
				"Connection: keep-alive\r\n"
				"\r\n";
			Assert::AreEqual(expectedTotalHeader, computedTotalHeader);
		}
	};
}
