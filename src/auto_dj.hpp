#pragma once

#include <memory>
#include <expected>
#include <string>

#include "win32_error.hpp"

namespace manelemax
{

class auto_dj
{
public:
    static std::expected<auto_dj, win32_com_error> make();

    auto_dj(auto_dj&&);
    auto_dj& operator=(auto_dj&&);

    ~auto_dj();

    std::string current_match() const;

private:
    struct impl;

    auto_dj() = default;

    std::unique_ptr<impl> impl_;
};

}  // namespace manelemax
