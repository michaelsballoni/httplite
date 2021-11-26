#include "pch.h"
#include "CppUnitTest.h"

#include "Core.h"
#pragma comment(lib, "httplite")

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace httplite
{
	TEST_CLASS(CoreTests)
	{
	public:

		TEST_METHOD(TestCore)
		{
			{
				std::wstring from = L"";
				std::string to = "";
				std::string enc = httplite::UrlEncoded(from);
				Assert::AreEqual(to, enc);
				std::wstring dec = httplite::UrlDecoded(enc);
				Assert::AreEqual(from, dec);
			}

			{
				std::wstring from = L"f";
				std::string to = "f";
				std::string enc = httplite::UrlEncoded(from);
				Assert::AreEqual(to, enc);
				std::wstring dec = httplite::UrlDecoded(enc);
				Assert::AreEqual(from, dec);
			}

			{
				std::wstring from = L" ";
				std::string to = "%20";
				std::string enc = httplite::UrlEncoded(from);
				Assert::AreEqual(to, enc);
				std::wstring dec = httplite::UrlDecoded(enc);
				Assert::AreEqual(from, dec);
			}

			{
				std::wstring from = L"foo";
				std::string to = "foo";
				std::string enc = httplite::UrlEncoded(from);
				Assert::AreEqual(to, enc);
				std::wstring dec = httplite::UrlDecoded(enc);
				Assert::AreEqual(from, dec);
			}

			{
				std::wstring from = L"foo bar";
				std::string to = "foo%20bar";
				std::string enc = httplite::UrlEncoded(from);
				Assert::AreEqual(to, enc);
				std::wstring dec = httplite::UrlDecoded(enc);
				Assert::AreEqual(from, dec);
			}
		}
	};
}
