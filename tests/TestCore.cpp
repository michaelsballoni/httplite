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

		TEST_METHOD(TestSplit)
		{
			std::vector<std::string> strs;
			
			strs = Split("", ' ');
			Assert::AreEqual(0, int(strs.size()));

			strs = Split(" ", ' ');
			Assert::AreEqual(0, int(strs.size()));

			strs = Split("  ", ' ');
			Assert::AreEqual(0, int(strs.size()));

			strs = Split("foo", ' ');
			Assert::AreEqual(1, int(strs.size()));
			Assert::AreEqual(std::string("foo"), strs[0]);

			strs = Split("foo ", ' ');
			Assert::AreEqual(1, int(strs.size()));
			Assert::AreEqual(std::string("foo"), strs[0]);

			strs = Split(" foo ", ' ');
			Assert::AreEqual(1, int(strs.size()));
			Assert::AreEqual(std::string("foo"), strs[0]);

			strs = Split("foo bar", ' ');
			Assert::AreEqual(2, int(strs.size()));
			Assert::AreEqual(std::string("foo"), strs[0]);
			Assert::AreEqual(std::string("bar"), strs[1]);

			strs = Split("foo bar ", ' ');
			Assert::AreEqual(2, int(strs.size()));
			Assert::AreEqual(std::string("foo"), strs[0]);
			Assert::AreEqual(std::string("bar"), strs[1]);

			strs = Split("foo bar blet", ' ');
			Assert::AreEqual(3, int(strs.size()));
			Assert::AreEqual(std::string("foo"), strs[0]);
			Assert::AreEqual(std::string("bar"), strs[1]);
			Assert::AreEqual(std::string("blet"), strs[2]);
		}

		TEST_METHOD(TestUrlEnDecode)
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
