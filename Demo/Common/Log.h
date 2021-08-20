#pragma once

#include <fmt/core.h>

namespace Demo {
enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
};

namespace Impl {
void log(LogLevel level, const char* format, const fmt::format_args& args);
}

template<typename... Args>
void log(LogLevel level, const char* format, Args&&... args)
{
    auto fmt_args = fmt::make_format_args(std::forward<Args>(args)...);
    Demo::Impl::log(level, format, fmt_args);
}

template<typename... Args>
void debug(const char* format, Args&&... args)
{
    Demo::log(LogLevel::Debug, format, std::forward<Args>(args)...);
}

template<typename... Args>
void info(const char* format, Args&&... args)
{
    Demo::log(LogLevel::Info, format, std::forward<Args>(args)...);
}

template<typename... Args>
void warning(const char* format, Args&&... args)
{
    Demo::log(LogLevel::Warning, format, std::forward<Args>(args)...);
}

template<typename... Args>
void error(const char* format, Args&&... args)
{
    Demo::log(LogLevel::Error, format, std::forward<Args>(args)...);
}
}

