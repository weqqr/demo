#pragma once

namespace DM {
template<typename T>
T&& move(T& value)
{
    return static_cast<T&&>(value);
}

template<typename T>
void swap(T& lhs, T& rhs) noexcept
{
    T tmp(move(lhs));
    lhs = move(rhs);
    rhs = move(tmp);
}

#define ASSERT(condition, ...) ::DM::Impl::assert(condition, #condition, __FILE__, __LINE__, __VA_ARGS__)

#ifdef NDEBUG
#define DEBUG_ASSERT(condition, ...)
#else
#define DEBUG_ASSERT(condition, ...) ASSERT(condition, __VA_ARGS__)
#endif

#define UNREACHABLE() ::DM::Impl::unreachable(__FILE__, __LINE__)

#define PANIC(message) ::DM::Impl::panic(__FILE__, __LINE__, message)

namespace Impl {
void assert(bool condition, const char* condition_text, const char* file, size_t line, const char* message = "");
[[noreturn]] void unreachable(const char* file, size_t line);
[[noreturn]] void panic(const char* file, size_t line, const char* message);
}
}
