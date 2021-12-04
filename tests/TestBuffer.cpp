#include "pch.h"
#include "CppUnitTest.h"

#include "Buffer.h"
#pragma comment(lib, "httplite")

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace httplite
{
	TEST_CLASS(BufferTests)
	{
	public:
		TEST_METHOD(TestBuffer)
		{
			Buffer buffer(L"foobar");
			std::wstring to = buffer.ToString();
			Assert::AreEqual(toWideStr("foobar"), to);
		}
	};
}
