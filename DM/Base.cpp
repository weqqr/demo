// Silence MSVC's complaints about insecure, outdated functions
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <DM/Base.h>
#include <cstdio>
#include <cstdlib>

namespace DM::Impl {
[[noreturn]] static void _break()
{
#ifdef NDEBUG
    exit(1);
#else
    exit(1);
    __debugbreak();
#endif
}

void assert_handler(bool condition, const char* condition_text, const char* file, size_t line, const char* message)
{
    if (!condition) {
        fprintf(stderr, "Assertion failed: %s\n", message);
        fprintf(stderr, "%s:%llu: %s\n", file, line, condition_text);
        _break();
    }
}

[[noreturn]] void unreachable_handler(const char* file, size_t line)
{
    fprintf(stderr, "%s:%llu: Execution reached supposedly unreachable point\n", file, line);
    _break();
}

[[noreturn]] void panic_handler(const char* file, size_t line, const char* message)
{
    fprintf(stderr, "panic:\n");
    fprintf(stderr, "%s:%llu: %s\n", file, line, message);
    _break();
}
}
