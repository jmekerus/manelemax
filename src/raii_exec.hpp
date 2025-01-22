#pragma once

#include <concepts>
#include <memory>
#include <type_traits>

namespace manelemax
{

template<typename F>
    requires(std::invocable<std::remove_reference_t<F>>)
class raii_exec
{
public:
    explicit raii_exec(F&& f) noexcept
        : func_ {std::move(f)}
    {
    }

    ~raii_exec()
    {
        func_();
    }

    raii_exec(const raii_exec& other)            = delete;
    raii_exec& operator=(const raii_exec& other) = delete;
    raii_exec(raii_exec&& other)                 = delete;
    raii_exec& operator=(raii_exec&& other)      = delete;

private:
    F func_;
};

}  // namespace manelemax
