#include <format>
#include <cstdlib>

#include <Windows.h>
#include <objbase.h>

#include "auto_dj.hpp"
#include "systray_icon.hpp"
#include "win32_error.hpp"
#include "raii_exec.hpp"

namespace manelemax
{

template<typename T>
static void display_win32_error(const T& err)
{
    ::MessageBoxA(
        nullptr,
        std::format("{} failed with error 0x{:08x}", err.function, err.code).c_str(),
        "ManeleMax",
        MB_OK | MB_ICONERROR
    );
}

}  // namespace manelemax

int WINAPI WinMain(
    const HINSTANCE hInstance,
    const HINSTANCE /*hPrevInstance*/,
    const LPSTR /*lpCmdLine*/,
    const int /*nShowCmd*/
)
{
    manelemax::raii_exec co_uninitialize {[] { ::CoUninitialize(); }};

    if (const HRESULT result = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED); FAILED(result))
    {
        manelemax::display_win32_error(manelemax::win32_com_error {"CoInitializeEx", result});
    }

    auto auto_dj_obj = manelemax::auto_dj::make();
    if (!auto_dj_obj.has_value())
    {
        manelemax::display_win32_error(auto_dj_obj.error());
        return EXIT_FAILURE;
    }

    auto tray = manelemax::systray_icon::make(hInstance);
    if (!tray.has_value())
    {
        manelemax::display_win32_error(tray.error());
        return EXIT_FAILURE;
    }

    tray->set_current_match_fn([&auto_dj_obj] { return auto_dj_obj->current_match(); });
    tray->process_messages();

    return EXIT_SUCCESS;
}
