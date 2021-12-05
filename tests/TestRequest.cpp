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
		TEST_METHOD(TestRequestGetHeader)
		{
			Request request;
			request.Verb = "GET";
			request.Path = std::vector<std::wstring>{ L"Music", L"index.html" };
			request.Query =
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
				"some: thing\r\n"
				"totally: different\r\n"
				"Content-Length: 38\r\n"
				"Connection: keep-alive\r\n"
				"\r\n";
			Assert::AreEqual(expectedTotalHeader, computedTotalHeader);
		}

		TEST_METHOD(TestRequestReadHeader)
		{
			std::string totalHeaderToRead =
				"GET /Music/index.html?foo=bar&blet=mon%20key HTTP/1.0\r\n"
				"some: thing\r\n"
				"totally: different\r\n"
				"Content-Length: 38\r\n"
				"Connection: keep-alive\r\n"
				"\r\n";

			Request request;
			std::string errorMessage = request.ReadHeader(totalHeaderToRead.c_str());
			Assert::IsTrue(errorMessage.empty());
			
			Assert::AreEqual(std::string("GET"), request.Verb);

			Assert::AreEqual(2, int(request.Path.size()));
			Assert::AreEqual(toWideStr("Music"), request.Path[0]);
			Assert::AreEqual(toWideStr("index.html"), request.Path[1]);

			Assert::AreEqual(2, int(request.Query.size()));
			Assert::AreEqual(toWideStr("bar"), request.Query[L"foo"]);
			Assert::AreEqual(toWideStr("mon key"), request.Query[L"blet"]);

			Assert::AreEqual(4, int(request.Headers.size()));
			Assert::AreEqual(std::string("thing"), request.Headers["some"]);
			Assert::AreEqual(std::string("different"), request.Headers["totally"]);
			Assert::AreEqual(std::string("38"), request.Headers["Content-Length"]);
			Assert::AreEqual(std::string("keep-alive"), request.Headers["Connection"]);
		}
	};
}
