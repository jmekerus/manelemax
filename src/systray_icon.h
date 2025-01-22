#pragma once

#include <expected>
#include <memory>

#include <Windows.h>
#include <shellapi.h>

#include "win32_error.h"

namespace manelemax
{

class window;

class systray_icon
{
public:
    static std::expected<systray_icon, win32_error> make(HINSTANCE hInstance);

    ~systray_icon();

    systray_icon(const systray_icon&)            = delete;
    systray_icon& operator=(const systray_icon&) = delete;

    systray_icon(systray_icon&& other) noexcept
    {
        *this = std::move(other);
    }

    systray_icon& operator=(systray_icon&& other) noexcept
    {
        if (this != &other)
        {
            hWnd_       = other.hWnd_;
            other.hWnd_ = nullptr;
            nidata_     = std::move(other.nidata_);
        }
        return *this;
    }

    void process_messages();

private:
    systray_icon() noexcept = default;

    static LRESULT window_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static void    display_context_menu(HWND hWnd);
    static void    context_menu_cmd(HWND hWnd, WORD cmd);

    HWND                             hWnd_ {nullptr};
    std::unique_ptr<NOTIFYICONDATAA> nidata_ {nullptr};
};

}  // namespace manelemax
