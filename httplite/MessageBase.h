#pragma once

#include "Buffer.h"

namespace httplite
{
	/// <summary>
	/// MessageBase is the base class to Request and Response
	/// Request and Response processing is very similar
	/// so the Recv and Send member functions do all the
	/// work, with only header processing defined by the
	/// derived types
	/// </summary>
	class MessageBase
	{
	public:
		virtual ~MessageBase() {} // for derived types

		// Data structures common to Request and Response
		std::unordered_map<std::string, std::string> Headers;
		std::optional<Buffer> Payload;

		// Header convenience methods
		bool IsConnectionClose() const;
		int GetContentLength() const;

		// Core message networking routines
		std::string Recv(SOCKET theSocket);
		std::string Send(SOCKET theSocket) const;

		// Request and Response need to define header behavior
		virtual std::string GetTotalHeader() const = 0;
		virtual std::string ReadHeader(const char* headerStart) = 0;

	protected:
		// Get the parts of the header that are common
		// between Request and Response
		std::string GetCommonHeader() const;
	};
}
