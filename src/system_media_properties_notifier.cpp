#include <shared_mutex>
#include <atomic>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Control.h>

#include "system_media_properties_notifier.hpp"

namespace manelemax
{

using namespace winrt::Windows::Media::Control;

struct system_media_properties_notifier::impl
{
public:
    impl()
        : session_manager_ {GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get()}
        , current_session_ {session_manager_.GetCurrentSession()}
    {
        session_manager_.CurrentSessionChanged(
            [this](const auto& /*sender*/, const auto& /*args*/) {
                current_session_ = session_manager_.GetCurrentSession();
                on_session_changed();
            }
        );
        on_session_changed();
    }

    impl(const impl&)            = delete;
    impl& operator=(const impl&) = delete;
    impl(impl&&)                 = delete;
    impl& operator=(impl&&)      = delete;

    void on_session_changed()
    {
        if (current_session_)
        {
            current_session_.MediaPropertiesChanged(
                [this](const auto& /*sender*/, const auto& /*args*/) { update_media_properties(); }
            );

            current_session_.PlaybackInfoChanged(
                [this](const auto& /*sender*/, const auto& /*args*/) { update_playback_status(); }
            );

            update_media_properties();
            update_playback_status();
        }
        else
        {
            std::unique_lock lock {media_properties_mutex_};
            if (currently_playing_)
            {
                currently_playing_ = false;
                lock.unlock();

                auto lsn = lsn_.load();
                if (lsn)
                {
                    lsn_.load()->on_stop();
                }
            }
        }
    }

    void update_playback_status()
    {
        const auto media = current_session_.GetPlaybackInfo();

        const bool currently_playing =
            media.PlaybackStatus() ==
            GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing;

        std::unique_lock lock {media_properties_mutex_};

        const bool changed_currently_playing = currently_playing_ != currently_playing;
        const auto media_properties          = media_properties_;

        currently_playing_ = currently_playing;

        lock.unlock();

        if (!changed_currently_playing)
        {
            return;
        }

        auto lsn = lsn_.load();
        if (!lsn)
        {
            return;
        }

        if (currently_playing)
        {
            lsn->on_play(media_properties);
        }
        else
        {
            lsn->on_stop();
        }
    }

    void update_media_properties()
    {
        const auto media = current_session_.TryGetMediaPropertiesAsync().get();

        const properties new_media_props {
            .artist = media.Artist().c_str(),
            .title  = media.Title().c_str()
        };

        const bool currently_playing = [&] {
            std::unique_lock props_lock {media_properties_mutex_};
            media_properties_ = new_media_props;
            return currently_playing_;
        }();

        auto lsn = lsn_.load();
        if (!lsn)
        {
            return;
        }

        if (currently_playing)
        {
            lsn_.load()->on_play(new_media_props);
        }
    }

    GlobalSystemMediaTransportControlsSessionManager session_manager_;
    GlobalSystemMediaTransportControlsSession        current_session_;

    properties        media_properties_ {};
    bool              currently_playing_ {false};
    std::shared_mutex media_properties_mutex_ {};

    std::atomic<listener*> lsn_ {nullptr};
};

system_media_properties_notifier::~system_media_properties_notifier() = default;

system_media_properties_notifier::system_media_properties_notifier()
    : impl_ {std::make_unique<impl>()}
{
}

auto system_media_properties_notifier::get_media_props() const -> std::optional<properties>
{
    std::shared_lock lock {impl_->media_properties_mutex_};
    if (impl_->currently_playing_)
    {
        return impl_->media_properties_;
    }
    return std::nullopt;
}

void system_media_properties_notifier::set_listener(listener* const lsn)
{
    impl_->lsn_ = lsn;
}

}  // namespace manelemax
