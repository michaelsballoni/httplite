// All-important central #include of all
// libraries used in this library
// All classes include Core.h or
// other header(s) that include Core.h
// This module implements low-level 
// functions for working exceptions,
// sockets, and strings
#pragma once

#include "Includes.h"

namespace httplite
{
	/// <summary>
	/// Exception type used throughout this libray
	/// to stay distingusted from generic exceptions
	/// and capture socket errors
	/// </summary>
	class NetworkError : public std::runtime_error
	{
	public:
		NetworkError(const std::string& msg) 
			: std::runtime_error(GetErrorMessage(msg)) 
		{}

		static std::string GetErrorMessage(std::string msg)
		{
			int lastSocketError = WSAGetLastError();
			if (lastSocketError != 0)
				msg += " (" + std::to_string(lastSocketError) + ")";
			return msg;
		}
	};

	/// <summary>
	/// Create a SocketUse object in your (w)main function
	/// to automatically WSAStartup and WSACleanup
	/// </summary>
	class SocketUse
	{
	public:
		SocketUse()
		{
			WSADATA wsaData;
			if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
				throw NetworkError("WSAStartup failed");
		}

		~SocketUse()
		{
			WSACleanup();
		}
	};

	/// <summary>
	/// Turn a number into a string with
	/// no trailing zeroes
	/// </summary>
	/// <param name="num">number to convert into a string</param>
	/// <returns>string form of the number</returns>
	inline std::string num2str(double num)
	{
		std::stringstream ss;
		ss << num;
		return ss.str();
	}

	/// <summary>
	/// Convert a wstring into a UTF-8 string
	/// </summary>
	/// <param name="str">wstring to convert</param>
	/// <returns>UTF-8 encoded string</returns>
	inline std::string toNarrowStr(const std::wstring& str)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
		return converter.to_bytes(str);
	}

	/// <summary>
	/// Take a UTF-8 encoded string and convert it a wstring
	/// </summary>
	/// <param name="str">UTF-8 encoded string</param>
	/// <returns>converted wstring</returns>
	inline std::wstring toWideStr(const std::string& str)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(str);
	}

	/// <summary>
	/// Return a lowercase copy of a string
	/// </summary>
	/// <param name="str">string to make copy of</param>
	/// <returns>lowercase copy of input string</returns>
	inline std::string toLower(const std::string& str)
	{
		std::string retVal;
		retVal.reserve(str.size());
		for (auto c : str)
			retVal += static_cast<char>(tolower(c));
		return retVal;
	}

	/// <summary>
	/// Split a string into a vector of strings
	/// using a separator character
	/// </summary>
	/// <param name="str">string to split</param>
	/// <param name="separator">separator character</param>
	/// <returns>vector of strings that were split by the separator</returns>
	std::vector<std::string> Split(const std::string& str, char separator);

	/// <summary>
	/// Split a wstring into a vector of strings
	/// using a separator character
	/// </summary>
	/// <param name="str">string to split</param>
	/// <param name="separator">separator character</param>
	/// <returns>vector of strings that were split by the separator</returns>
	std::vector<std::wstring> Split(const std::wstring& str, wchar_t separator);

	std::wstring Join(const std::vector<std::wstring>& strs, const wchar_t* seperator);

	/// <summary>
	/// Are two vectors identical, same size, same contents?
	/// </summary>
	template<typename T>
	bool AreVecsEqual(const std::vector<T>& vec1, const std::vector<T>& vec2)
	{
		if (vec1.size() != vec2.size())
			return false;
		for (size_t v = 0; v < vec1.size(); ++v)
		{
			if (vec1[v] != vec2[v])
				return false;
		}
		return true;
	}
	/// <summary>
	/// Turn a wstring into a URL-encoded string
	/// </summary>
	/// <param name="part">wstring to convert</param>
	/// <returns>URL-encoded string</returns>
	std::string UrlEncode(const std::wstring& part);

	/// <summary>
	/// Convert a URL-encoded string into a full wstring
	/// </summary>
	/// <param name="part">URL-encoded string</param>
	/// <returns>decoded wstring</returns>
	std::wstring UrlDecode(const std::string& part);
}
