#pragma once

#define WIN32_LEAN_AND_MEAN 1
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include <assert.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
