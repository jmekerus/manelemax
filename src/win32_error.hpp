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

struct win32_com_error
{
    std::string_view function;
    HRESULT          code;
};

}  // namespace manelemax
