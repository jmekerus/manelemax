#include <format>
#include <cstdlib>

#include <Windows.h>
#include <objbase.h>

#include "systray_icon.hpp"
#include "win32_error.hpp"
#include "volume_control.hpp"
#include "raii_exec.hpp"
#include "system_media_properties_notifier.hpp"

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

    auto vol_ctrl = manelemax::volume_control::make();
    if (!vol_ctrl.has_value())
    {
        manelemax::display_win32_error(vol_ctrl.error());
        return EXIT_FAILURE;
    }

    constexpr float normal_mode_volume = 0.25f;
    constexpr float max_mode_volume    = 1.0f;

    struct volume_listener : public manelemax::volume_control::listener
    {
        void on_volume_changed(const float vol) override
        {
            if (force_volume)
            {
                vol_ctrl->set_volume(current_volume);
            }
        }

        void on_muted_state_changed(const bool muted) override
        {
            if (muted && force_unmute)
            {
                vol_ctrl->set_muted(false);
            }
        }

        manelemax::volume_control* vol_ctrl       = nullptr;
        bool                       force_unmute   = false;
        bool                       force_volume   = false;
        float                      current_volume = normal_mode_volume;
    };

    volume_listener vol_lsn;
    vol_lsn.vol_ctrl = std::addressof(*vol_ctrl);

    if (const auto result = vol_ctrl->set_listener(&vol_lsn); !result.has_value())
    {
        manelemax::display_win32_error(result.error());
        return EXIT_FAILURE;
    }

    const auto update_volume_settings =
        [&](const std::optional<manelemax::system_media_properties_notifier::properties>&
                media_props) {
            if (!media_props.has_value())
            {
                vol_lsn.force_unmute = false;
                vol_lsn.force_volume = false;
                return;
            }

            if (media_props->title.contains(L"Nicolae Guta"))
            {
                vol_lsn.current_volume = max_mode_volume;
                vol_lsn.force_unmute   = true;
                vol_lsn.force_volume   = true;
                vol_ctrl->set_muted(false);
                vol_ctrl->set_volume(vol_lsn.current_volume);
            }
            else
            {
                vol_lsn.current_volume = normal_mode_volume;
                vol_lsn.force_unmute   = false;
                vol_lsn.force_volume   = true;
                vol_ctrl->set_volume(vol_lsn.current_volume);
            }
        };

    struct media_listener : public manelemax::system_media_properties_notifier::listener
    {
        void on_play(const manelemax::system_media_properties_notifier::properties& media_props
        ) override
        {
            update_volume_settings(media_props);
        }

        void on_stop() override
        {
            update_volume_settings(std::nullopt);
        }

        std::function<
            void(const std::optional<manelemax::system_media_properties_notifier::properties>&)>
            update_volume_settings;
    };

    media_listener media_lsn;
    media_lsn.update_volume_settings = update_volume_settings;

    manelemax::system_media_properties_notifier media_props_notifier;
    media_props_notifier.set_listener(&media_lsn);
    update_volume_settings(media_props_notifier.get_media_props());

    auto tray = manelemax::systray_icon::make(hInstance);
    if (!tray.has_value())
    {
        manelemax::display_win32_error(tray.error());
        return EXIT_FAILURE;
    }

    tray->process_messages();

    return EXIT_SUCCESS;
}
