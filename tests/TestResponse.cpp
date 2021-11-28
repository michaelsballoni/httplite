#include "pch.h"
#include "CppUnitTest.h"

#include "Response.h"
#pragma comment(lib, "httplite")

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace httplite
{
	TEST_CLASS(ResponseTests)
	{
	public:
		TEST_METHOD(TestResponse)
		{
			const char* header =
				"HTTP/1.0 500.100 Interal ASP Error\r\n"
				"Content-Type: text/html\r\n"
				"Content-Length: 1234\r\n"
				"Connection: close\r\n"
				"\r\n";

			Response response;
			response.ReadHeader(header);

			Assert::AreEqual(uint16_t(500), response.GetStatusCode());
			Assert::AreEqual(std::string("500.100 Interal ASP Error"), response.Status);
			Assert::AreEqual(std::wstring(L"Interal ASP Error"), response.GetStatusDescription());
			Assert::AreEqual(std::string("text/html"), response.Headers["Content-Type"]);
			Assert::IsTrue(response.IsConnectionClose());
			Assert::AreEqual(1234, int(response.ContentLength()));
		}
	};
}
