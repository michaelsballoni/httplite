#include "pch.h"
#include "CppUnitTest.h"

#include "HeaderReader.h"
#pragma comment(lib, "httplite")

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace httplite
{
	TEST_CLASS(HeaderReaderTests)
	{
	public:
		
		TEST_METHOD(TestHeaderReader)
		{
			HeaderReader reader;
			Assert::IsFalse(reader.OnMoreData(reinterpret_cast<const byte*>("http some"), 9));
			Assert::IsFalse(reader.OnMoreData(reinterpret_cast<const byte*>("\r\n"), 2));
			Assert::IsFalse(reader.OnMoreData(reinterpret_cast<const byte*>("head1: foo\r\n"), 12));
			Assert::IsFalse(reader.OnMoreData(reinterpret_cast<const byte*>("head2: foo\r\n"), 12));
			Assert::IsTrue(reader.OnMoreData(reinterpret_cast<const byte*>("\r\n"), 2));
			Assert::AreEqual(std::string("http some\r\nhead1: foo\r\nhead2: foo"), std::string(reader.GetHeaders()));
			size_t remainderCount = 0;
			Assert::IsNull(reader.GetRemainder(remainderCount));
			Assert::AreEqual(size_t(0), remainderCount);
		}
	};
}
