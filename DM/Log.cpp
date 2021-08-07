#include <DM/Log.h>
#include <fmt/format.h>

namespace DM::Impl {
void log(LogLevel level, const char* format, const fmt::format_args& args)
{
    auto text = fmt::vformat(format, args);
    const char* level_text = "";

    switch (level) {
    case LogLevel::Info:
        level_text = " INFO";
        break;
    case LogLevel::Warning:
        level_text = " WARN";
        break;
    case LogLevel::Error:
        level_text = "ERROR";
        break;
    case LogLevel::Fatal:
        level_text = "FATAL";
        break;
    }

    fmt::print("{}: {}\n", level_text, text);

    if (level == LogLevel::Fatal) {
#if NDEBUG
        exit(1);
#else
        __debugbreak();
#endif
    }
}
}
