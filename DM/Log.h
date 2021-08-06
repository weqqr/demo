#pragma once

#include <fmt/core.h>

namespace DM {
enum class LogLevel {
    Info,
    Warning,
    Fatal,
};

namespace Impl {
void log(LogLevel level, const char* format, const fmt::format_args& args);
}

template<typename... Args>
void log(const char* format, Args&&... args)
{
    auto fmt_args = fmt::make_format_args(std::forward<Args>(args)...);
    DM::Impl::log(LogLevel::Info, format, fmt_args);
}

template<typename... Args>
void log(LogLevel level, const char* format, Args&&... args)
{
    auto fmt_args = fmt::make_format_args(std::forward<Args>(args)...);
    DM::Impl::log(level, format, fmt_args);
}
}
