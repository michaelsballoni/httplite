// Kind of a PCH, but a separate file so calling modules
// can include this in their PCH's
// None of the code in this library includes any of the below
// dependencies directly
// Instead, Core.h includes this, and most everybody includes Core.h
#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS // gethostbyname, etc.
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING // toWideString / toNarrowString

#define WIN32_LEAN_AND_MEAN 1
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include <assert.h>

#include <codecvt>
#include <cstdint>
#include <functional>
#include <locale> 
#include <memory> 
#include <mutex> 
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
