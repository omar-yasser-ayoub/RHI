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

    enum class ShaderStageFlags
    {
        Vertex = 1 << 0,
        Fragment = 1 << 1,
        Compute = 1 << 2,
    };

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
        uint32_t stride;
        uint32_t offset;
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
