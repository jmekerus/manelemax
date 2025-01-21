#pragma once

#include <string_view>

#include <Windows.h>

namespace manelemax
{

struct win32_error
{
    std::string_view function;
    DWORD            code;
};

}  // namespace manelemax
