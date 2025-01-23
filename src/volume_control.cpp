#include "volume_control.hpp"

namespace manelemax
{

class volume_control_callback : public IAudioEndpointVolumeCallback
{
public:
    volume_control_callback(const GUID& guid_context, volume_control::listener* const lsn)
        : guid_context_ {guid_context}
        , lsn_ {lsn}
    {
    }

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        return InterlockedIncrement(&ref_count_);
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        const ULONG ref = InterlockedDecrement(&ref_count_);
        if (ref == 0)
        {
            delete this;
        }
        return ref;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID** ppvInterface) override
    {
        if (IID_IUnknown == riid)
        {
            AddRef();
            *ppvInterface = (IUnknown*) this;
        }
        else if (__uuidof(IAudioEndpointVolumeCallback) == riid)
        {
            AddRef();
            *ppvInterface = (IAudioEndpointVolumeCallback*) this;
        }
        else
        {
            *ppvInterface = NULL;
            return E_NOINTERFACE;
        }
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) override
    {
        if (pNotify == NULL)
        {
            return E_INVALIDARG;
        }
        if (pNotify->guidEventContext == guid_context_)
        {
            return S_OK;
        }

        if (lsn_)
        {
            lsn_->on_volume_changed(pNotify->fMasterVolume);
            lsn_->on_muted_state_changed(pNotify->bMuted);
        }

        return S_OK;
    }

private:
    GUID                      guid_context_;
    volume_control::listener* lsn_;
    ULONG                     ref_count_ {1};
};

std::expected<volume_control, win32_com_error> volume_control::make()
{
    constexpr CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
    constexpr IID   IID_IMMDeviceEnumerator  = __uuidof(IMMDeviceEnumerator);
    constexpr IID   IID_IAudioEndpointVolume = __uuidof(IAudioEndpointVolume);

    HRESULT result;

    volume_control instance;

    result = CoCreateGuid(&instance.guid_context_);
    if (FAILED(result))
    {
        return std::unexpected {win32_com_error {"CoCreateGuid", result}};
    }

    IMMDeviceEnumerator* mm_device_enumerator;
    if (const HRESULT result = CoCreateInstance(
            CLSID_MMDeviceEnumerator,
            nullptr,
            CLSCTX_ALL,
            IID_IMMDeviceEnumerator,
            reinterpret_cast<void**>(&mm_device_enumerator)
        );
        FAILED(result))
    {
        return std::unexpected {win32_com_error {"CoCreateInstance", result}};
    }
    instance.mm_device_enumerator_.reset(mm_device_enumerator);

    IMMDevice* mm_device;
    if (const HRESULT result = instance.mm_device_enumerator_
                                   ->GetDefaultAudioEndpoint(eRender, eMultimedia, &mm_device);
        FAILED(result))
    {
        return std::unexpected {
            win32_com_error {"IMMDeviceEnumerator::GetDefaultAudioEndpoint", result}
        };
    }
    instance.mm_device_.reset(mm_device);

    IAudioEndpointVolume* audio_endpoint_volume;
    if (const HRESULT result = instance.mm_device_->Activate(
            IID_IAudioEndpointVolume,
            CLSCTX_ALL,
            nullptr,
            reinterpret_cast<void**>(&audio_endpoint_volume)
        );
        FAILED(result))
    {
        return std::unexpected {win32_com_error {"IMMDevice::Activate", result}};
    }
    instance.audio_endpoint_volume_.reset(audio_endpoint_volume);

    return instance;
}

std::expected<void, win32_com_error> volume_control::set_muted(bool muted) const
{
    if (const HRESULT result = audio_endpoint_volume_->SetMute(muted, &guid_context_);
        FAILED(result))
    {
        return std::unexpected {win32_com_error {"IAudioEndpointVolume::SetMute", result}};
    }
    return {};
}

std::expected<void, win32_com_error> volume_control::set_volume(float vol) const
{
    if (const HRESULT result =
            audio_endpoint_volume_->SetMasterVolumeLevelScalar(vol, &guid_context_);
        FAILED(result))
    {
        return std::unexpected {
            win32_com_error {"IAudioEndpointVolume::SetMasterVolumeLevelScalar", result}
        };
    }
    return {};
}

std::expected<void, win32_com_error> volume_control::set_listener(listener* const lsn)
{
    if (lsn == nullptr)
    {
        return {};
    }

    // Allow only once
    if (audio_endpoint_volume_cb_)
    {
        return {};
    }

    com_ptr<IAudioEndpointVolumeCallback> cb_obj {new volume_control_callback {guid_context_, lsn}};

    if (const HRESULT result = audio_endpoint_volume_->RegisterControlChangeNotify(cb_obj.get());
        FAILED(result))
    {
        return std::unexpected {
            win32_com_error {"IAudioEndpointVolume::RegisterControlChangeNotify", result}
        };
    }

    audio_endpoint_volume_cb_ = std::move(cb_obj);

    return {};
}

}  // namespace manelemax
