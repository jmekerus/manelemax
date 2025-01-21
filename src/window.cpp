#include "window.h"

namespace manelemax
{

std::expected<window, win32_error> window::make(HINSTANCE hInstance) noexcept
{
    constexpr std::string_view window_class_name = "ManeleMax";
    constexpr std::string_view window_name       = "ManeleMax";

    WNDCLASS window_class {
        .style         = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc   = &window::window_proc,
        .hInstance     = hInstance,
        .hCursor       = ::LoadCursor(nullptr, IDC_ARROW),
        .lpszClassName = window_class_name.data()
    };

    if (::RegisterClassA(&window_class) == 0)
    {
        return std::unexpected {win32_error {"RegisterClassA", ::GetLastError()}};
    }

    HWND hWnd = ::CreateWindowExA(
        0,                         // dwExStyle
        window_class_name.data(),  // lpClassName
        window_name.data(),        // lpWindowName
        WS_OVERLAPPEDWINDOW,       // dwStyle
        CW_USEDEFAULT,             // X
        CW_USEDEFAULT,             // Y
        CW_USEDEFAULT,             // nWidth
        CW_USEDEFAULT,             // nHeight
        nullptr,                   // hWndParent
        nullptr,                   // hMenu
        hInstance,                 // hInstance
        nullptr                    // lpParam
    );
    if (!hWnd)
    {
        return std::unexpected {win32_error {"CreateWindowExA", ::GetLastError()}};
    }

    window instance;
    instance.hWnd_ = hWnd;

    return instance;
}

void window::set_visible(bool visible) const noexcept
{
    if (hWnd_ != nullptr)
    {
        ::ShowWindow(hWnd_, visible ? SW_SHOW : SW_HIDE);
        ::UpdateWindow(hWnd_);
    }
}

void window::process_messages() noexcept
{
    if (hWnd_ == nullptr)
    {
        return;
    }

    MSG msg;
    while (::GetMessageA(&msg, hWnd_, 0, 0) == TRUE)
    {
        ::TranslateMessage(&msg);
        ::DispatchMessageA(&msg);
    }

    destroy();
}

std::expected<void, win32_error> window::request_close() const noexcept
{
    if (hWnd_ != nullptr && ::PostMessageA(hWnd_, WM_CLOSE, 0, 0) == FALSE)
    {
        return std::unexpected {win32_error {"PostMessageA", ::GetLastError()}};
    }

    return {};
}

LRESULT
window::window_proc(const HWND hWnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY:
        {
            ::PostQuitMessage(0);
            return 0;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            const HDC   hdc = ::BeginPaint(hWnd, &ps);
            if (!hdc)
            {
                return 0;
            }

            ::FillRect(hdc, &ps.rcPaint, HBRUSH(COLOR_WINDOW + 1));
            ::EndPaint(hWnd, &ps);

            return 0;
        }
    }

    return ::DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

}  // namespace manelemax
