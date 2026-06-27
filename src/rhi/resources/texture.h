#pragma once

#include <cstdint>

namespace rhi
{
    enum class TextureDimension
    {
        Tex1D,
        Tex2D,
        Tex3D,
    };

    enum class TextureFormat
    {
        RGBA8Unorm,
        BGRA8Unorm,

        RGBA16Float,

        Depth24Stencil8,
        Depth32Float,
    };

    enum class TextureUsage : std::uint32_t
    {
        None          = 0,

        Sampled       = 1 << 0,
        Storage       = 1 << 1,

        RenderTarget  = 1 << 2,
        DepthStencil  = 1 << 3,

        CopySrc       = 1 << 4,
        CopyDst       = 1 << 5,
    };

    // Bitwise operators

    constexpr TextureUsage operator|(TextureUsage lhs, TextureUsage rhs)
    {
        return static_cast<TextureUsage>(
            static_cast<std::uint32_t>(lhs) |
            static_cast<std::uint32_t>(rhs));
    }

    constexpr TextureUsage operator&(TextureUsage lhs, TextureUsage rhs)
    {
        return static_cast<TextureUsage>(
            static_cast<std::uint32_t>(lhs) &
            static_cast<std::uint32_t>(rhs));
    }

    constexpr TextureUsage operator~(TextureUsage value)
    {
        return static_cast<TextureUsage>(
            ~static_cast<std::uint32_t>(value));
    }

    inline TextureUsage& operator|=(TextureUsage& lhs, TextureUsage rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    inline TextureUsage& operator&=(TextureUsage& lhs, TextureUsage rhs)
    {
        lhs = lhs & rhs;
        return lhs;
    }

    constexpr bool HasFlag(TextureUsage value, TextureUsage flag)
    {
        return (static_cast<std::uint32_t>(value) &
                static_cast<std::uint32_t>(flag)) != 0;
    }

    struct TextureDesc
    {
        TextureDimension dimension = TextureDimension::Tex2D;

        std::uint32_t width = 1;
        std::uint32_t height = 1;
        std::uint32_t depth = 1;

        std::uint32_t mipLevels = 1;
        std::uint32_t arrayLayers = 1;

        TextureFormat format = TextureFormat::RGBA8Unorm;

        TextureUsage usage = TextureUsage::None;
    };

    // struct TextureViewDesc
    // {
    // };
}
