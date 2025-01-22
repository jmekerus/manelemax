#include <algorithm>

#include "systray_icon.h"
#include "resource.h"

namespace manelemax
{

// {9C578A2D-A01F-4C27-BF91-F0B7E0EE4BB6}
static constexpr GUID g_systray_icon_guid =
    {0x9c578a2d, 0xa01f, 0x4c27, {0xbf, 0x91, 0xf0, 0xb7, 0xe0, 0xee, 0x4b, 0xb6}};
static constexpr UINT g_systray_notif_msg     = WM_USER + 0x100;
static constexpr WORD g_context_menu_cmd_exit = 101;

std::expected<systray_icon, win32_error> systray_icon::make(const HINSTANCE hInstance)
{
    constexpr char             tooltip_text[]    = "ManeleMax";
    constexpr std::string_view window_class_name = "ManeleMax";

    bool cleanup_window_class = false;
    bool cleanup_notifyicon   = false;
    HWND hWnd                 = nullptr;
    auto nidata               = std::make_unique<NOTIFYICONDATAA>();

    const auto do_cleanup = [&] {
        if (cleanup_notifyicon)
        {
            ::Shell_NotifyIconA(NIM_DELETE, nidata.get());
        }
        if (hWnd)
        {
            ::DestroyWindow(hWnd);
        }
        if (cleanup_window_class)
        {
            ::UnregisterClassA(window_class_name.data(), hInstance);
        }
    };

    const WNDCLASS window_class {
        .lpfnWndProc   = &systray_icon::window_proc,
        .hInstance     = hInstance,
        .lpszClassName = window_class_name.data()
    };

    if (::RegisterClassA(&window_class) == 0)
    {
        do_cleanup();
        return std::unexpected {win32_error {"RegisterClassA", ::GetLastError()}};
    }
    cleanup_window_class = true;

    hWnd = ::CreateWindowExA(
        0,                         // dwExStyle
        window_class_name.data(),  // lpClassName
        nullptr,                   // lpWindowName
        0,                         // dwStyle
        0,                         // X
        0,                         // Y
        0,                         // nWidth
        0,                         // nHeight
        nullptr,                   // hWndParent
        nullptr,                   // hMenu
        hInstance,                 // hInstance
        nullptr                    // lpParam
    );
    if (!hWnd)
    {
        do_cleanup();
        return std::unexpected {win32_error {"CreateWindowExA", ::GetLastError()}};
    }

    HICON hIcon = ::LoadIconA(::GetModuleHandleA(nullptr), MAKEINTRESOURCEA(IDI_ICON1));
    if (!hIcon)
    {
        do_cleanup();
        return std::unexpected {win32_error {"LoadIconA", ::GetLastError()}};
    }

    nidata->cbSize           = sizeof(NOTIFYICONDATAA);
    nidata->hWnd             = hWnd;
    nidata->uFlags           = NIF_ICON | NIF_TIP | NIF_GUID | NIF_MESSAGE;
    nidata->hIcon            = hIcon;
    nidata->uVersion         = NOTIFYICON_VERSION_4;
    nidata->guidItem         = g_systray_icon_guid;
    nidata->uCallbackMessage = g_systray_notif_msg;
    std::copy_n(tooltip_text, sizeof(tooltip_text) / sizeof(char), nidata->szTip);

    // Try to remove the icon in case it was not removed during the last run
    ::Shell_NotifyIconA(NIM_DELETE, nidata.get());

    if (::Shell_NotifyIconA(NIM_ADD, nidata.get()) == FALSE)
    {
        do_cleanup();
        return std::unexpected {win32_error {"Shell_NotifyIconA", ::GetLastError()}};
    }
    cleanup_notifyicon = true;

    if (::Shell_NotifyIconA(NIM_SETVERSION, nidata.get()) == FALSE)
    {
        do_cleanup();
        return std::unexpected {win32_error {"Shell_NotifyIconA", ::GetLastError()}};
    }

    systray_icon instance;
    instance.nidata_ = std::move(nidata);
    instance.hWnd_   = hWnd;

    return instance;
}

systray_icon::~systray_icon()
{
    if (nidata_)
    {
        ::Shell_NotifyIconA(NIM_DELETE, nidata_.get());
    }
    if (hWnd_)
    {
        ::DestroyWindow(hWnd_);
    }
}

void systray_icon::process_messages()
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
}

LRESULT
systray_icon::window_proc(
    const HWND   hWnd,
    const UINT   uMsg,
    const WPARAM wParam,
    const LPARAM lParam
)
{
    switch (uMsg)
    {
        case WM_DESTROY:
        {
            ::PostQuitMessage(0);
            return 0;
        }
        case WM_COMMAND:
        {
            if (lParam == 0)
            {
                context_menu_cmd(hWnd, LOWORD(wParam));
                return 0;
            }
            break;
        }
        case g_systray_notif_msg:
        {
            switch (LOWORD(lParam))
            {
                case NIN_SELECT: [[fallthrough]];
                case NIN_KEYSELECT: [[fallthrough]];
                case WM_CONTEXTMENU:
                {
                    display_context_menu(hWnd);
                    ::PostMessage(hWnd, WM_NULL, 0, 0);
                    break;
                }
            }
            return 0;
        }
    }

    return ::DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

void systray_icon::context_menu_cmd(const HWND hWnd, const WORD cmd)
{
    switch (cmd)
    {
        case g_context_menu_cmd_exit:
        {
            ::PostMessageA(hWnd, WM_CLOSE, 0, 0);
        }
    }
}

void systray_icon::display_context_menu(const HWND hWnd)
{
    POINT pt;
    if (::GetCursorPos(&pt) == FALSE)
    {
        return;
    }

    HMENU hMenu = ::CreatePopupMenu();
    if (!hMenu)
    {
        return;
    }

    if (::InsertMenuA(hMenu, -1, MF_BYPOSITION | MF_STRING, g_context_menu_cmd_exit, "Exit") ==
        FALSE)
    {
        ::DestroyMenu(hMenu);
        return;
    }

    ::SetForegroundWindow(hWnd);

    if (::TrackPopupMenu(
            hMenu,
            TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN,
            pt.x,
            pt.y,
            0,
            hWnd,
            nullptr
        ) == FALSE)
    {
        ::DestroyMenu(hMenu);
        return;
    }
}

}  // namespace manelemax
