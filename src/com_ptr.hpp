#pragma once

#include <memory>

#include "Unknwnbase.h"

namespace manelemax
{

namespace _internal
{
struct com_obj_deleter
{
    void operator()(IUnknown* obj) const
    {
        obj->Release();
    }
};

}  // namespace _internal

template<typename T>
using com_ptr = std::unique_ptr<T, _internal::com_obj_deleter>;

}  // namespace manelemax
