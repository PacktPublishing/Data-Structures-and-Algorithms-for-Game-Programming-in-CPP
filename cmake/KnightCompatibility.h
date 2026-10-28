#pragma once

// The original Visual Studio projects use Microsoft's bounds-checked CRT
// functions. These small adapters preserve the call sites on Clang/GCC.
#include <algorithm>
#include <cfloat>
#include <cstring>

#if !defined(_WIN32)
#include <cerrno>
#include <cstdio>
#include <strings.h>

using errno_t = int;

#ifndef _TRUNCATE
#define _TRUNCATE (static_cast<std::size_t>(-1))
#endif

inline errno_t fopen_s(FILE** file, const char* name, const char* mode)
{
    if (file == nullptr) return EINVAL;
    *file = std::fopen(name, mode);
    return (*file != nullptr) ? 0 : errno;
}

template <std::size_t N, typename... Args>
inline int sprintf_s(char (&buffer)[N], const char* format, Args... args)
{
    return std::snprintf(buffer, N, format, args...);
}

template <typename... Args>
inline int sprintf_s(char* buffer, std::size_t size, const char* format, Args... args)
{
    return std::snprintf(buffer, size, format, args...);
}

template <std::size_t N>
inline errno_t strcpy_s(char (&destination)[N], const char* source)
{
    if (source == nullptr) return EINVAL;
    const std::size_t length = std::strlen(source);
    if (length >= N) {
        destination[0] = '\0';
        return ERANGE;
    }
    std::memcpy(destination, source, length + 1);
    return 0;
}

inline errno_t strcpy_s(char* destination, std::size_t size, const char* source)
{
    if (destination == nullptr || source == nullptr || size == 0) return EINVAL;
    const std::size_t length = std::strlen(source);
    if (length >= size) {
        destination[0] = '\0';
        return ERANGE;
    }
    std::memcpy(destination, source, length + 1);
    return 0;
}

inline errno_t strncpy_s(
    char* destination,
    std::size_t destinationSize,
    const char* source,
    std::size_t count)
{
    if (destination == nullptr || source == nullptr || destinationSize == 0) return EINVAL;
    const std::size_t sourceLength = std::strlen(source);
    const std::size_t copied = (count == _TRUNCATE)
        ? std::min(destinationSize - 1, sourceLength)
        : std::min(count, sourceLength);
    if (copied >= destinationSize) {
        destination[0] = '\0';
        return ERANGE;
    }
    std::memcpy(destination, source, copied);
    destination[copied] = '\0';
    return 0;
}

inline errno_t memcpy_s(
    void* destination,
    std::size_t destinationSize,
    const void* source,
    std::size_t count)
{
    if (destination == nullptr || source == nullptr) return EINVAL;
    if (count > destinationSize) return ERANGE;
    std::memcpy(destination, source, count);
    return 0;
}

#define _stricmp strcasecmp
#define _strcmpi strcasecmp
#endif

