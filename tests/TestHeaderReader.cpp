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
			// GET type, headers but no payload
			{
				HeaderReader reader;
				Assert::IsFalse(reader.OnMoreData(reinterpret_cast<const uint8_t*>("http some"), 9));
				Assert::IsFalse(reader.OnMoreData(reinterpret_cast<const uint8_t*>("\r\n"), 2));
				Assert::IsFalse(reader.OnMoreData(reinterpret_cast<const uint8_t*>("head1: foo\r\n"), 12));
				Assert::IsFalse(reader.OnMoreData(reinterpret_cast<const uint8_t*>("head2: foo\r\n"), 12));
				Assert::IsTrue(reader.OnMoreData(reinterpret_cast<const uint8_t*>("\r\n"), 2));
				Assert::AreEqual(std::string("http some\r\nhead1: foo\r\nhead2: foo"), std::string(reader.GetHeaders()));
				size_t remainderCount = 0;
				Assert::IsNull(reader.GetRemainder(remainderCount));
				Assert::AreEqual(size_t(0), remainderCount);
			}

			// POST type, headers with one byte of payload
			{
				HeaderReader reader;
				Assert::IsFalse(reader.OnMoreData(reinterpret_cast<const uint8_t*>("http some"), 9));
				Assert::IsFalse(reader.OnMoreData(reinterpret_cast<const uint8_t*>("\r\n"), 2));
				Assert::IsFalse(reader.OnMoreData(reinterpret_cast<const uint8_t*>("head1: foo\r\n"), 12));
				Assert::IsFalse(reader.OnMoreData(reinterpret_cast<const uint8_t*>("head2: foo\r\n"), 12));
				Assert::IsTrue(reader.OnMoreData(reinterpret_cast<const uint8_t*>("\r\nb"), 3));
				Assert::AreEqual(std::string("http some\r\nhead1: foo\r\nhead2: foo"), std::string(reader.GetHeaders()));
				size_t remainderCount = 0;
				const uint8_t* remainder = reader.GetRemainder(remainderCount);
				Assert::AreEqual(size_t(1), remainderCount);
				Assert::AreEqual('b', char(*remainder));
			}

			// POST type, headers with more than one byte of payload
			{
				HeaderReader reader;
				Assert::IsFalse(reader.OnMoreData(reinterpret_cast<const uint8_t*>("http some"), 9));
				Assert::IsFalse(reader.OnMoreData(reinterpret_cast<const uint8_t*>("\r\n"), 2));
				Assert::IsFalse(reader.OnMoreData(reinterpret_cast<const uint8_t*>("head1: foo\r\n"), 12));
				Assert::IsFalse(reader.OnMoreData(reinterpret_cast<const uint8_t*>("head2: foo\r\n"), 12));
				Assert::IsTrue(reader.OnMoreData(reinterpret_cast<const uint8_t*>("\r\nblet"), 6));
				Assert::AreEqual(std::string("http some\r\nhead1: foo\r\nhead2: foo"), std::string(reader.GetHeaders()));
				
				size_t remainderCount = 0;
				const uint8_t* remainder = reader.GetRemainder(remainderCount);
				Assert::AreEqual(size_t(4), remainderCount);

				std::vector<uint8_t> buffer;
				buffer.resize(remainderCount);
				memcpy(buffer.data(), remainder, buffer.size());
				buffer.push_back(0);
				
				Assert::AreEqual(std::string("blet"), std::string(reinterpret_cast<const char*>(buffer.data())));
			}
		}
	};
}
