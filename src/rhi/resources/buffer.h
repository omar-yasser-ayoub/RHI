#pragma once

#include <cstdint>

namespace rhi
{
    struct BufferHandle {
        std::uint64_t id = 0xFFFFFFFF;
    };

    enum class BufferUsage : std::uint32_t
    {
        None      = 0,
        Vertex    = 1 << 0,
        Index     = 1 << 1,
        Uniform   = 1 << 2,
        Storage   = 1 << 3,
        Indirect  = 1 << 4,
        CopySrc   = 1 << 5,
        CopyDst   = 1 << 6,
    };

    // Bitwise operators

    constexpr BufferUsage operator|(BufferUsage lhs, BufferUsage rhs)
    {
        return static_cast<BufferUsage>(
            static_cast<std::uint32_t>(lhs) |
            static_cast<std::uint32_t>(rhs));
    }

    constexpr BufferUsage operator&(BufferUsage lhs, BufferUsage rhs)
    {
        return static_cast<BufferUsage>(
            static_cast<std::uint32_t>(lhs) &
            static_cast<std::uint32_t>(rhs));
    }

    constexpr BufferUsage operator~(BufferUsage value)
    {
        return static_cast<BufferUsage>(
            ~static_cast<std::uint32_t>(value));
    }

    inline BufferUsage& operator|=(BufferUsage& lhs, BufferUsage rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    inline BufferUsage& operator&=(BufferUsage& lhs, BufferUsage rhs)
    {
        lhs = lhs & rhs;
        return lhs;
    }

    constexpr bool HasFlag(BufferUsage value, BufferUsage flag)
    {
        return (static_cast<std::uint32_t>(value) &
                static_cast<std::uint32_t>(flag)) != 0;
    }

    struct BufferDesc
    {
        std::uint64_t size = 0;
        BufferUsage usage = BufferUsage::None;
    };
}
