#pragma once

#include <optional>
#include <string>
#include <memory>

namespace manelemax
{

class system_media_properties_notifier
{
public:
    struct properties
    {
        std::wstring artist;
        std::wstring title;
    };

    struct listener
    {
        virtual ~listener() = default;

        virtual void on_play(const properties& media_props) = 0;
        virtual void on_stop()                              = 0;
    };

    system_media_properties_notifier();
    ~system_media_properties_notifier();

    std::optional<properties> get_media_props() const;
    void                      set_listener(listener* lsn);

private:
    struct impl;

    std::unique_ptr<impl> impl_;
};

}  // namespace manelemax
