#include <format>
#include <cstdlib>

#include <Windows.h>
#include <objbase.h>

#include "systray_icon.hpp"
#include "win32_error.hpp"
#include "volume_control.hpp"
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

    if (const HRESULT result = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); FAILED(result))
    {
        manelemax::display_win32_error(manelemax::win32_com_error {"CoInitializeEx", result});
    }

    auto vol_ctrl = manelemax::volume_control::make();
    if (!vol_ctrl.has_value())
    {
        manelemax::display_win32_error(vol_ctrl.error());
        return EXIT_FAILURE;
    }

    vol_ctrl->register_callback(
        [&vol_ctrl](const float vol) { vol_ctrl->set_volume(1.0f); },
        [&vol_ctrl](const bool muted) { vol_ctrl->set_muted(false); }
    );

    vol_ctrl->set_volume(1.0f);
    vol_ctrl->set_muted(false);

    auto tray = manelemax::systray_icon::make(hInstance);
    if (!tray.has_value())
    {
        manelemax::display_win32_error(tray.error());
        return EXIT_FAILURE;
    }

    tray->process_messages();

    return EXIT_SUCCESS;
}
