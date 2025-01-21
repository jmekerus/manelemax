#pragma once

#include <expected>
#include <memory>

#include <Windows.h>

#include "win32_error.h"

namespace manelemax
{

class window
{
public:
    static std::expected<window, win32_error> make(HINSTANCE hInstance) noexcept;

    window(const window&)            = delete;
    window& operator=(const window&) = delete;

    window(window&& other) noexcept
    {
        *this = std::move(other);
    }

    window& operator=(window&& other) noexcept
    {
        if (this != &other)
        {
            hWnd_       = other.hWnd_;
            other.hWnd_ = nullptr;
        }
        return *this;
    }

    ~window() noexcept
    {
        destroy();
    }

    void set_visible(bool visible) const noexcept;
    void process_messages() noexcept;

    std::expected<void, win32_error> request_close() const noexcept;

private:
    void destroy() noexcept
    {
        if (hWnd_ != nullptr)
        {
            ::DestroyWindow(hWnd_);
            hWnd_ = nullptr;
        }
    }

    static LRESULT
    window_proc(const HWND hWnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam);

    window() noexcept = default;

    HWND hWnd_ {nullptr};
};

}  // namespace manelemax
