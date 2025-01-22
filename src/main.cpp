#include <format>
#include <cstdlib>

#include <Windows.h>

#include "systray_icon.h"
#include "win32_error.h"

namespace manelemax
{

static void display_win32_error(const win32_error& err)
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
    auto tray = manelemax::systray_icon::make(hInstance);
    if (!tray.has_value())
    {
        manelemax::display_win32_error(tray.error());
        return EXIT_FAILURE;
    }

    tray->process_messages();

    return EXIT_SUCCESS;
}
