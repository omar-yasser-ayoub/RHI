#pragma once

#include "rhi/resources/window.h"

#include <cstdint>

namespace rhi
{
    struct SurfaceDesc
    {
        WindowHandle window{};
    };

    struct SurfaceHandle
    {
        std::uint64_t id = 0xFFFFFFFFFFFFFFFF;
        SurfaceDesc desc{};
    };
}
