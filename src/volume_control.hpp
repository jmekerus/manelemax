#pragma once

#include <expected>
#include <functional>

#include <endpointvolume.h>
#include <mmdeviceapi.h>

#include "win32_error.hpp"
#include "com_ptr.hpp"

namespace manelemax
{

class volume_control
{
public:
    static std::expected<volume_control, win32_com_error> make();

    std::expected<void, win32_com_error> set_muted(bool muted) const;
    std::expected<void, win32_com_error> set_volume(float vol) const;

    std::expected<void, win32_com_error> register_callback(
        std::function<void(float)>&& volume_changed      = nullptr,
        std::function<void(bool)>&&  muted_state_changed = nullptr
    );

private:
    volume_control() noexcept = default;

    GUID                                  guid_context_ {};
    com_ptr<IMMDeviceEnumerator>          mm_device_enumerator_ {nullptr};
    com_ptr<IMMDevice>                    mm_device_ {nullptr};
    com_ptr<IAudioEndpointVolume>         audio_endpoint_volume_ {nullptr};
    com_ptr<IAudioEndpointVolumeCallback> audio_endpoint_volume_cb_ {nullptr};
};

}  // namespace manelemax
