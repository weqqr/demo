#include <Demo/Common/Log.h>

#define DM_COLORED_OUTPUT 0

#if DM_COLORED_OUTPUT
#define DM_RED "\x1B[0;31m"
#define DM_GREEN "\x1B[0;32m"
#define DM_YELLOW "\x1B[0;33m"
#define DM_RESET "\x1B[0m"
#else
#define DM_RED ""
#define DM_GREEN ""
#define DM_YELLOW ""
#define DM_RESET ""
#endif

namespace Demo::Impl {
void log(LogLevel level, const char* format, const fmt::format_args& args)
{
    auto text = fmt::vformat(format, args);
    const char* level_text = "";

    switch (level) {
    case LogLevel::Debug:
        level_text = "DEBUG";
        break;
    case LogLevel::Info:
        level_text = DM_GREEN " INFO" DM_RESET;
        break;
    case LogLevel::Warning:
        level_text = DM_YELLOW " WARN" DM_RESET;
        break;
    case LogLevel::Error:
        level_text = DM_RED "ERROR" DM_RESET;
        break;
    }

    fmt::print("{}: {}\n", level_text, text);
}
}
