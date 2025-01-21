#include <format>
#include <cstdlib>

#include <Windows.h>

#include "window.h"
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
    auto wnd = manelemax::window::make(hInstance);
    if (!wnd.has_value())
    {
        manelemax::display_win32_error(wnd.error());
        return EXIT_FAILURE;
    }

    wnd->set_visible(true);
    wnd->process_messages();

    return EXIT_SUCCESS;
}
