#pragma once

#include <cstdint>
#include <vector>
#include <filesystem>
#include <string>

namespace rhi {

    enum class BindingType
    {
        UniformBuffer,
        StorageBuffer,
        SampledTexture,
        StorageTexture,
        Sampler,
    };

    enum class ShaderStageFlags : std::uint32_t
    {
        None     = 0,
        Vertex   = 1 << 0,
        Fragment = 1 << 1,
        Compute  = 1 << 2,
    };

    constexpr ShaderStageFlags operator|(ShaderStageFlags lhs, ShaderStageFlags rhs)
    {
        return static_cast<ShaderStageFlags>(
            static_cast<std::uint32_t>(lhs) |
            static_cast<std::uint32_t>(rhs));
    }

    constexpr ShaderStageFlags operator&(ShaderStageFlags lhs, ShaderStageFlags rhs)
    {
        return static_cast<ShaderStageFlags>(
            static_cast<std::uint32_t>(lhs) &
            static_cast<std::uint32_t>(rhs));
    }

    constexpr bool HasFlag(ShaderStageFlags value, ShaderStageFlags flag)
    {
        return (static_cast<std::uint32_t>(value) &
                static_cast<std::uint32_t>(flag)) != 0;
    }

    struct ShaderBinding
    {
        uint32_t set;
        uint32_t binding;
        BindingType type;
        uint32_t arraySize;
        ShaderStageFlags stage;
    };

    enum class VertexFormat
    {
        Float32,
        Float32x2,
        Float32x3,
        Float32x4,

        Int32,
        Int32x2,
        Int32x3,
        Int32x4,

        UInt32,
        UInt32x2,
        UInt32x3,
        UInt32x4,
    };

    struct VertexInputAttr
    {
        uint32_t location;
        VertexFormat format;
    };

    struct ShaderReflection
    {
        std::vector<ShaderBinding> bindings;
        std::vector<VertexInputAttr> vertexInputs;
    };

    struct ShaderModule
    {
        std::vector<uint8_t> bytecode;
        ShaderReflection reflection;
    };

    struct ShaderSource
    {
        std::filesystem::path path;
        ShaderStageFlags stage;
        std::string entryPoint;
    };

    struct ShaderModuleHandle
    {
        std::uint64_t id = 0xFFFFFFFF;
    };
}
