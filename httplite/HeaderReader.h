#pragma once

#include "Core.h"

namespace httplite
{
	class HeaderReader
	{
	public:
		HeaderReader()
			: m_headersEnd(nullptr)
			, m_remainderStart(nullptr)
			, m_remainderCount(0)
		{}

		/// <summary>
		/// Call OnMoreData as data comes in, until it returns true, 
		/// or GetSize exceeds your threshold of pain of buffer memory usage.
		/// Then you can call GetHeaders and GetRemainder to tease out the goodies.
		/// </summary>
		/// <param name="data">pointer to new data</param>
		/// <param name="count">amount of new data</param>
		/// <returns>true if headers fully read</returns>
		bool OnMoreData(const uint8_t* data, const size_t count);

		size_t GetSize() const
		{
			return m_buffer.size();
		}

		const char* GetHeaders() const
		{
			return reinterpret_cast<const char*>(m_buffer.data());
		}

		const uint8_t* GetRemainder(size_t& count) const
		{
			count = m_remainderCount;
			return m_remainderStart;
		}

	private:
		buffer m_buffer;

		char* m_headersEnd;

		byte* m_remainderStart;
		size_t m_remainderCount;
	};
}
