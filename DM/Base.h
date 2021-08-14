#pragma once

namespace DM {
namespace Impl {
void assert_handler(bool condition, const char* condition_text, const char* file, size_t line, const char* message = "");
[[noreturn]] void unreachable_handler(const char* file, size_t line);
[[noreturn]] void panic_handler(const char* file, size_t line, const char* message);
}

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

template<typename T>
void dispose(T& t)
{
    T(move(t));
}

#define ASSERT(condition, ...) ::DM::Impl::assert_handler(condition, #condition, __FILE__, __LINE__, __VA_ARGS__)

#ifdef NDEBUG
#define DEBUG_ASSERT(condition, ...)
#else
#define DEBUG_ASSERT(condition, ...) ASSERT(condition, __VA_ARGS__)
#endif

#define UNREACHABLE() ::DM::Impl::unreachable_handler(__FILE__, __LINE__)

#define PANIC(message) ::DM::Impl::panic_handler(__FILE__, __LINE__, message)
}
