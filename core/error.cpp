#include "error.h"
#include "fmt/core.h"
namespace pbrt
{

    void Error(const char *format, ...)
    {
        fmt::print("Error output: need implement");
    }

    void Info(const char *format, ...)
    {
        fmt::print("Info output: need implement");
    }

    void Warning(const char *format, ...)
    {
        fmt::print("Warning output: need implement");
    }
}