#include <optional>
#include <unordered_set>
#include <algorithm>

#include "auto_dj.hpp"
#include "volume_control.hpp"
#include "system_media_properties_notifier.hpp"
#include "keywords.hpp"
#include "string_utils.hpp"

namespace manelemax
{

struct auto_dj::impl
{
    impl() = default;

    struct volume_listener : public volume_control::listener
    {
        void on_volume_changed(const float vol) override
        {
            if (parent->force_volume)
            {
                parent->vol_ctrl->set_volume(parent->current_volume);
            }
        }

        void on_muted_state_changed(const bool muted) override
        {
            if (muted && parent->force_unmute)
            {
                parent->vol_ctrl->set_muted(false);
            }
        }

        impl* parent {nullptr};
    };

    struct media_listener : public system_media_properties_notifier::listener
    {
        void on_play(const system_media_properties_notifier::properties& media_props) override
        {
            parent->update_volume_settings(media_props);
        }

        void on_stop() override
        {
            parent->update_volume_settings(std::nullopt);
        }

        impl* parent {nullptr};
    };

    impl(const impl&)            = delete;
    impl& operator=(const impl&) = delete;
    impl(impl&&)                 = delete;
    impl& operator=(impl&&)      = delete;

    std::expected<void, win32_com_error> init()
    {
        if (auto new_vol_ctrl = manelemax::volume_control::make(); new_vol_ctrl.has_value())
        {
            vol_ctrl = *std::move(new_vol_ctrl);
        }
        else
        {
            return std::unexpected {new_vol_ctrl.error()};
        }

        vol_lsn.parent = this;
        if (const auto result = vol_ctrl->set_listener(&vol_lsn); !result.has_value())
        {
            return std::unexpected {result.error()};
        }

        media_lsn.parent = this;
        media_props_notifier.set_listener(&media_lsn);

        keywords_set.insert_range(g_keywords);

        update_volume_settings(media_props_notifier.get_media_props());

        return {};
    }

    void update_volume_settings(
        const std::optional<manelemax::system_media_properties_notifier::properties>& media_props
    )
    {
        if (!media_props.has_value())
        {
            force_unmute = false;
            force_volume = false;
            return;
        }

        if (!keyword_match(media_props->artist).empty() ||
            !keyword_match(media_props->title).empty())
        {
            current_volume = max_mode_volume;
            force_unmute   = true;
            force_volume   = true;
            vol_ctrl->set_muted(false);
            vol_ctrl->set_volume(current_volume);
        }
        else
        {
            current_volume = normal_mode_volume;
            force_unmute   = false;
            force_volume   = true;
            vol_ctrl->set_volume(current_volume);
        }
    }

    std::string keyword_match(const std::wstring& wstr) const
    {
        constexpr size_t max_words = 4;

        if (wstr.empty())
        {
            return "";
        }

        std::string str = stringutils::remove_ro_diacritics(wstr);

        const auto to_search = stringutils::all_word_aligned_substrings(
            stringutils::to_lower(stringutils::keep_alpha_and_spaces(str)),
            max_words
        );

        if (const auto it = std::ranges::find_if(
                to_search,
                [this](const std::string& s) { return keywords_set.contains(s); }
            );
            it != to_search.end())
        {
            return *it;
        }

        return "";
    }

    std::optional<volume_control>    vol_ctrl {std::nullopt};
    system_media_properties_notifier media_props_notifier {};

    static constexpr float normal_mode_volume {0.25f};
    static constexpr float max_mode_volume = {1.0f};

    bool  force_unmute {false};
    bool  force_volume {false};
    float current_volume {normal_mode_volume};

    volume_listener vol_lsn {};
    media_listener  media_lsn {};

    std::unordered_set<std::string_view> keywords_set;
};

std::expected<auto_dj, win32_com_error> auto_dj::make()
{
    auto_dj instance;
    instance.impl_ = std::make_unique<impl>();

    if (const auto result = instance.impl_->init(); !result.has_value())
    {
        return std::unexpected {result.error()};
    }

    return instance;
}

auto_dj::auto_dj(auto_dj&&)            = default;
auto_dj& auto_dj::operator=(auto_dj&&) = default;
auto_dj::~auto_dj()                    = default;

}  // namespace manelemax
