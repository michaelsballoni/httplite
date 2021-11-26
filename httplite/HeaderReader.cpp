#include "pch.h"
#include "HeaderReader.h"

namespace httplite
{
	bool HeaderReader::OnMoreData(const uint8_t* data, const size_t count)
	{
		// You cannot call this function again once headers have been returned
		if (m_headersEnd != nullptr)
		{
			assert(false);
			throw NetworkError("OnMoreData called after headers read");
		}

		// Add the data to our buffer, and add a null-terminator so we can...
		const size_t originalSize = m_buffer.size();
		m_buffer.resize(m_buffer.size() + count + 1);
		memcpy(m_buffer.data() + originalSize, data, count);
		m_buffer.back() = 0;

		// ...look in our buffer for the all-important \r\n\r\n with trusty strstr...
		m_headersEnd = const_cast<char*>(strstr((const char*)m_buffer.data(), "\r\n\r\n"));
		m_buffer.pop_back(); // strip our extra null byte now that we're done with it
		if (m_headersEnd == nullptr)
		{
			// ...buffer is incomplete
			return false;
		}
		else
		{
			// ...buffer is complete...seal it off
			m_headersEnd[0] = '\0';

			// Capture where the remainder starts and what's left, if any
			m_remainderStart = reinterpret_cast<uint8_t*>(m_headersEnd) + 4;
			m_remainderCount = m_buffer.size() - (m_remainderStart - m_buffer.data());
			if (m_remainderCount == 0) // don't just point any old place
				m_remainderStart = nullptr;
			return true;
		}
	}
}
