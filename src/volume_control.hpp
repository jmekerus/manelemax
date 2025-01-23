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

    struct listener
    {
        virtual ~listener() = default;

        virtual void on_volume_changed(float vol)       = 0;
        virtual void on_muted_state_changed(bool muted) = 0;
    };

    std::expected<void, win32_com_error> set_listener(listener* lsn);

private:
    volume_control() noexcept = default;

    GUID                                  guid_context_ {};
    com_ptr<IMMDeviceEnumerator>          mm_device_enumerator_ {nullptr};
    com_ptr<IMMDevice>                    mm_device_ {nullptr};
    com_ptr<IAudioEndpointVolume>         audio_endpoint_volume_ {nullptr};
    com_ptr<IAudioEndpointVolumeCallback> audio_endpoint_volume_cb_ {nullptr};
};

}  // namespace manelemax
